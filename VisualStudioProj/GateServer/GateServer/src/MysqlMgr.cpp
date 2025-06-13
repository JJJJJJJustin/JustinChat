#include "JCpch.h"
#include "MySqlMgr.h"

MySqlMgr::~MySqlMgr()
{
}

int MySqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& password)
{
	return m_Dao.RegUser(name, email, password);
}
