#include "JCpch.h"
#include "ConfigMgr.h"
#include "boost/filesystem.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"

ConfigMgr& ConfigMgr::Inst()
{
	static ConfigMgr configMgr;
	return configMgr;
}

ConfigMgr::ConfigMgr()
{
	boost::filesystem::path currentPath = boost::filesystem::current_path();
	boost::filesystem::path configPath = currentPath / "src/config.ini";
	JC_CORE_TRACE("Reading config file in: {}", configPath.string());
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(configPath.string(), pt);

	for(const auto& pair1 : pt)
	{
		std::string sectionName = pair1.first;
		boost::property_tree::ptree sectionData = pair1.second;

		std::map<std::string, std::string> sectionConfig;
		for(const auto& pair2 : sectionData)
		{
			std::string key = pair2.first;
			std::string value = pair2.second.get_value<std::string>();
			sectionConfig[key] = value;
		}
		
		SectionInfo si(sectionConfig);
		m_ConfigMap[sectionName] = si;
	}

	// For DEBUGING ...
	for( const auto& i : m_ConfigMap)
	{
		JC_CORE_TRACE("Section Name :{}", i.first);
		SectionInfo config = i.second;
		for( const auto& j : config.m_DataMap)
		{
			JC_CORE_TRACE("\t {} = {}", j.first, j.second);
		}
	}
}