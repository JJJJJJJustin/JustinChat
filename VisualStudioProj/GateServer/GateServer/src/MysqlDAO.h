#pragma once
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>

// --------------------------------------------------------------
// --------------------- Mysql ������ ---------------------------
// ��װ�� sql::Connection ָ�� �� LastOperTime����ʱ��������Ϣ��
// --------------------------------------------------------------
class MysqlConnection
{
public:
	MysqlConnection(std::unique_ptr<sql::Connection> connection, uint64_t lastOperTime)
		:Connection(std::move(connection)), LastOperTime(lastOperTime) {};
public:
	std::unique_ptr<sql::Connection> Connection;
	uint64_t LastOperTime;
};

// ------------------------------------------------------------
// --------------------- Mysql ���ӳ� -------------------------
// ------------------------------------------------------------
class MySqlPool
{
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& password, const std::string& schema, uint32_t poolsize);
	~MySqlPool();

	std::unique_ptr<MysqlConnection> GetConnection();
	void ReturnConnection(std::unique_ptr<MysqlConnection> conn);
private:
	void CheckConnection();
	void Close();
private:
	std::string m_Url;
	std::string m_User;
	std::string m_Password;
	std::string m_Schema;
	uint32_t m_Poolsize;

	std::atomic<bool> b_Stop;
	std::condition_variable m_Cond;
	std::mutex m_Mutex;
	std::thread m_CheckThread;

	std::queue<std::unique_ptr<MysqlConnection>> m_Connections;
};

// ------------------------------------------------------------
// --------------------- User ��Ϣ�ṹ�� ----------------------
// ------------------------------------------------------------
//struct UserInfo
//{
//	std::string Name;
//	std::string Password;
//	std::string Email;
//	uint32_t UId;
//};

// ------------------------------------------------------------
// --------------------- ��װ Mysql ���� ----------------------
// ------------------------------------------------------------
class MySqlDAO
{
public:
	MySqlDAO();
	~MySqlDAO();

	int RegUser(const std::string& name, const std::string& email, const std::string& password);
	// check email
	// update password
	// check password
private:
	std::unique_ptr<MySqlPool> m_Pool;
};