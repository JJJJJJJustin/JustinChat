#include "JCpch.h"
#include "ConfigMgr.h"
#include "MysqlDAO.h"

// ------------------------------------------------------------
// --------------------- Mysql ���ӳ� -------------------------
// ------------------------------------------------------------
MySqlPool::MySqlPool(const std::string& url, const std::string& user, const std::string& password, const std::string& schema, uint32_t poolsize)
    :m_Url(url), m_User(user), m_Password(password), m_Schema(schema), m_Poolsize(poolsize)
{
    try {
        for (uint32_t i = 0; i < m_Poolsize; i++)
        {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect(m_Url, m_User, m_Password));
            con->setSchema(m_Schema);

            auto currentTime = std::chrono::system_clock::now().time_since_epoch();
            auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

            m_Connections.push(std::make_unique<MysqlConnection>(std::move(con), timestamp));
        }

        m_CheckThread = std::thread([this]() 
            {
                while(!b_Stop)
                {
                    CheckConnection();
                    std::this_thread::sleep_for(std::chrono::seconds(60));      // ��һ�δ�����ʲô���ã�ΪʲôҪ sleep_for 60s��
                }
            }
        );

        m_CheckThread.detach();
    }
    catch(sql::SQLException& ex)
    {
        JC_CORE_ERROR("MySQL pool init failed! (message: {})", ex.what());
    }

}

MySqlPool::~MySqlPool()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    Close();
    std::cout << "MySqlPool closed !" << std::endl;
    while(!m_Connections.empty())
    {
        m_Connections.pop();
    }
}

std::unique_ptr<MysqlConnection> MySqlPool::GetConnection()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Cond.wait(lock, [this]() 
        {
            if(b_Stop)
                return true;
            return !m_Connections.empty();
        }
    );

    if(b_Stop)
    {
        return nullptr;
    }
    std::unique_ptr<MysqlConnection> conn = std::move(m_Connections.front());
    m_Connections.pop();
    return conn;
}

void MySqlPool::ReturnConnection(std::unique_ptr<MysqlConnection> conn)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (b_Stop)
        return;

    m_Connections.push(std::move(conn));
    m_Cond.notify_one();
}

void MySqlPool::CheckConnection()
{
    // ������Ҫ���ʳ��е���Դ��������Ҫ��֤������ԭ����
    std::lock_guard<std::mutex> lock(m_Mutex);
    uint32_t poolsize = m_Connections.size();

    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    auto timestampNow = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

    for(uint32_t i = 0; i < poolsize; i++)
    {
        auto conn = std::move(m_Connections.front());
        m_Connections.pop();
        Defer defer([this, &conn]()
            {
                m_Connections.push(std::move(conn));
            }
        );

        // ÿ5���Ӳ��ܼ��һ�Σ������˻أ����¿�����һ��ѭ�������ľ�������ǣ�"SELECT 1"
        if(timestampNow - conn->LastOperTime > 300)
        {
            continue;
        }

        try
        {
            std::unique_ptr<sql::Statement> stmt(conn->Connection->createStatement());
            stmt->executeQuery("SELECT 1");             // ִ��һ���򵥵���䣬ͨ������Ҹ�Ч�ķ�ʽ�������ӻ�Ծ
            conn->LastOperTime = timestampNow;
            JC_CORE_INFO("A scheduled query is being executed (to keep the database connection active)...")
            JC_CORE_INFO("Scheduled query success! current time is {}", timestampNow);
        }
        catch(sql::SQLException& ex)
        {
            JC_CORE_ERROR("Error happened when keeping SQL connection alive! (message:{})", ex.what());

            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> newConn(driver->connect(m_Url, m_User, m_Password));
            newConn->setSchema(m_Schema);

            // ˢ�� conn �е���Ϣ�����������Ӳ����롢ˢ��ʱ�����
            conn->Connection.reset(newConn.release());
            conn->LastOperTime = timestampNow;
        }
    }
}

void MySqlPool::Close()
{
    b_Stop = true;
    m_Cond.notify_all();
}


// ------------------------------------------------------------
// --------------------- ��װ Mysql ���� ----------------------
// ------------------------------------------------------------
MySqlDAO::MySqlDAO()
{
    ConfigMgr& cfg = ConfigMgr::Inst();                 // ���������Ҫ��ȫ��ʹ�ã����Ա���Ҫ & ����
    const auto& host = cfg["MysqlLocalHost"]["Host"];
    const auto& port = cfg["MysqlLocalHost"]["Port"];
    const auto& user = cfg["MysqlLocalHost"]["User"];
    const auto& password = cfg["MysqlLocalHost"]["Password"];
    const auto& schema = cfg["MysqlLocalHost"]["Schema"];

    m_Pool = std::make_unique<MySqlPool>(host+':'+port, user, password, schema, uint32_t(5));
}

MySqlDAO::~MySqlDAO()
{
    // ��������ʹ�� std::unique_ptr ���� MySqlDAO �е� MySqlPool�������� MySqlD ������ʱ����ָ����Զ����� MySqlPool
    // ���� MySqlPool ����������ʹ���� Close()�������� MySqlDAO �����������в���Ҫ�ٴε��� Close()��
}

int MySqlDAO::RegUser(const std::string& name, const std::string& email, const std::string& password)
{
    std::unique_ptr<MysqlConnection> conn = m_Pool->GetConnection();

    if(conn == nullptr)
    {
        JC_CORE_ERROR("Failed to get the connection from MySql connection pool!");
        return false;
    }
    try
    {
        // ����һ��Ԥ�������
        std::unique_ptr<sql::PreparedStatement> stmt(conn->Connection->prepareStatement("CALL reg_user(?,?,?,@result)"));
        // ��ͨ�����Ϊ�洢���̵Ĳ�������Ӧֵ
        stmt->setString(1, name);
        stmt->setString(2, password);
        stmt->setString(3, email);
        // ִ�д洢����
        stmt->execute();

        // ���� prepatedStatement ��ֱ��֧��������������Ĺ��ܣ�������Ҫʹ�ûỰ��������������������ӻ�ȡ��Щֵ������ѡ����ʹ�ûỰ���� @result ���������ֵ��
        // ʹ�� createStatement ������ͨ�� SQL ���ִ����
        std::unique_ptr<sql::Statement> resultStmt(conn->Connection->createStatement());
        // ����洢�����������˻Ự����������������ʽ��ȡ���������ֵ������ʹ�ûỰ����ִ�в�ѯ���
        std::unique_ptr<sql::ResultSet> resultSet(resultStmt->executeQuery("SELECT @result AS result"));
        if(resultSet->next())
        {
            int resultInt = resultSet->getInt("result");
            JC_CORE_INFO("Result:{} (function{})", resultInt, __FUNCSIG__);

            m_Pool->ReturnConnection(std::move(conn));
            return resultInt;
        }
        m_Pool->ReturnConnection(std::move(conn));
        return -1;
    }
    catch (sql::SQLException& ex)
    {
        m_Pool->ReturnConnection(std::move(conn));
        JC_CORE_ERROR("(SQL exception message:{})", ex.what());
        JC_CORE_ERROR("(SQL error code:{})", ex.getErrorCode());
        JC_CORE_ERROR("(SQL state:{})", ex.getSQLState());
        return -1;
    }
}

