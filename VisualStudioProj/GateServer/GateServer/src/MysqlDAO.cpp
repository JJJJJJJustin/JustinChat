#include "JCpch.h"
#include "ConfigMgr.h"
#include "MysqlDAO.h"

// ------------------------------------------------------------
// --------------------- Mysql 连接池 -------------------------
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
                    std::this_thread::sleep_for(std::chrono::seconds(60));      // 这一段代码起到什么作用？为什么要 sleep_for 60s？
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
    // 由于需要访问池中的资源，所以需要保证操作的原子性
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

        // 每5分钟才能检查一次，否则退回，重新开启下一次循环。检查的具体操作是："SELECT 1"
        if(timestampNow - conn->LastOperTime > 300)
        {
            continue;
        }

        try
        {
            std::unique_ptr<sql::Statement> stmt(conn->Connection->createStatement());
            stmt->executeQuery("SELECT 1");             // 执行一个简单的语句，通过简便且高效的方式保持连接活跃
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

            // 刷新 conn 中的信息（建立新连接并传入、刷新时间戳）
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
// --------------------- 封装 Mysql 操作 ----------------------
// ------------------------------------------------------------
MySqlDAO::MySqlDAO()
{
    ConfigMgr& cfg = ConfigMgr::Inst();                 // 这个变量需要在全局使用，所以必须要 & 接收
    const auto& host = cfg["MysqlLocalHost"]["Host"];
    const auto& port = cfg["MysqlLocalHost"]["Port"];
    const auto& user = cfg["MysqlLocalHost"]["User"];
    const auto& password = cfg["MysqlLocalHost"]["Password"];
    const auto& schema = cfg["MysqlLocalHost"]["Schema"];

    m_Pool = std::make_unique<MySqlPool>(host+':'+port, user, password, schema, uint32_t(5));
}

MySqlDAO::~MySqlDAO()
{
    // 由于我们使用 std::unique_ptr 管理 MySqlDAO 中的 MySqlPool，所以在 MySqlD 被销毁时智能指针会自动销毁 MySqlPool
    // 并且 MySqlPool 的析构函数使用了 Close()，所以在 MySqlDAO 的析构函数中不需要再次调用 Close()。
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
        // 创建一个预处理语句
        std::unique_ptr<sql::PreparedStatement> stmt(conn->Connection->prepareStatement("CALL reg_user(?,?,?,@result)"));
        // 并通过语句为存储过程的参数填充对应值
        stmt->setString(1, name);
        stmt->setString(2, password);
        stmt->setString(3, email);
        // 执行存储过程
        stmt->execute();

        // 由于 prepatedStatement 不直接支持输出参数这样的功能，我们需要使用会话变量或者其他方法来间接获取这些值（这里选择了使用会话变量 @result 来传递输出值）
        // 使用 createStatement 创建普通的 SQL 语句执行器
        std::unique_ptr<sql::Statement> resultStmt(conn->Connection->createStatement());
        // 如果存储过程中设置了会话变量，或者其他方式获取输出参数的值，可以使用会话变量执行查询结果
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

