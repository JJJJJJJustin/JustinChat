#pragma once
#include "MysqlDAO.h"

class MySqlMgr : public Singleton<MySqlMgr>
{
	friend class Singleton<MySqlMgr>;
public:
	~MySqlMgr();

	int RegUser(const std::string& name, const std::string& email, const std::string& password);
private:
	MySqlMgr() = default;
private:
	MySqlDAO m_Dao;
};