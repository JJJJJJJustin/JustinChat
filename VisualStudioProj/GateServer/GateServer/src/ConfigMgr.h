#pragma once

struct SectionInfo
{
public:
	SectionInfo() = default;
	SectionInfo(std::map<std::string, std::string> map)
		:m_DataMap(map){}

	~SectionInfo() { m_DataMap.clear(); }

	SectionInfo(const SectionInfo& src)
	{
		this->m_DataMap = src.m_DataMap;
	}

	SectionInfo& operator=(const SectionInfo& src)
	{
		if (&src == this)
			return *this;
	
		this->m_DataMap = src.m_DataMap;
	}

	std::string operator[](const std::string& key)
	{
		if (m_DataMap.find(key) == m_DataMap.end())
			return "";

		return m_DataMap[key];
	}
public:
	std::map<std::string, std::string> m_DataMap;
};


class ConfigMgr
{
public:
	ConfigMgr();
	~ConfigMgr() { m_ConfigMap.clear(); }

	ConfigMgr(const ConfigMgr& src) { this->m_ConfigMap = src.m_ConfigMap; }

	ConfigMgr& operator= (const ConfigMgr& src)
	{
		if (this == &src)
			return *this;

		this->m_ConfigMap = src.m_ConfigMap;
	}

	SectionInfo operator[](const std::string& key)
	{
		if (m_ConfigMap.find(key) == m_ConfigMap.end())
		{
			return SectionInfo();
		}
		return m_ConfigMap[key];
	}


private:
	std::map<std::string, SectionInfo> m_ConfigMap;
};