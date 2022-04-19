/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <fstream>
#include <string>
#include "PersistentConfiguration.h"
#include "FileSystemOsAbstraction.h"

// *************************************************************************************************

JsonTree::JsonTree(const char* szName)
    : m_name(szName ? szName : "")
{
}

void JsonTree::Import(const Json::Value& jsonHostingContainer)
{
    const Json::Value& jsonContainer = m_name.empty() ? jsonHostingContainer : jsonHostingContainer[m_name];

    LOG_VERBOSE << "Importing json container"
        << " Name: " << (m_name.empty() ? "<empty>" : m_name.c_str())
        << " Content:\n" << jsonContainer
        << std::endl;

    if (Json::Value::nullSingleton() == jsonContainer)
    {
        LOG_WARNING << "No json container " << m_name << " found during json import" << std::endl;
        return;
    }

    for (auto& pParameter : m_children)
    {
        pParameter->Import(jsonContainer);
    }
}

void JsonTree::Export(Json::Value& jsonHostingContainer) const
{
    Json::Value jsonContainer;

    for (auto& pParameter : m_children)
    {
        pParameter->Export(jsonContainer);
    }

    if (m_name.empty())
    {
        jsonHostingContainer = jsonContainer;
    }
    else
    {
        jsonHostingContainer[m_name] = jsonContainer;
    }

    LOG_VERBOSE << "Exported json container"
        << " Name: " << m_name
        << " Content: \n" << jsonContainer
        << std::endl;
}

void JsonTree::AddChildNode(IJsonSerializable* pJsonValueMap)
{
    m_children.emplace_back(pJsonValueMap);
}

// *************************************************************************************************

ConfigurationSection::ConfigurationSection(const char* szName, JsonTree& configTreeRoot)
    : JsonTree(szName)
{
    configTreeRoot.AddChildNode(this);
}

// *************************************************************************************************

HostConfigurationSection::HostConfigurationSection(JsonTree& configTreeRoot)
    : ConfigurationSection("Host", configTreeRoot)
    , StringCommandPort("StringCommandPort", 12348, *this)
    , JsonCommandPort("JsonCommandPort", 12346, *this)
    , EventPort("EventPort", 12339, *this)
    , UdpDiscoveryInPort("UdpDiscoveryInPort", 12349, *this)
    , UdpDiscoveryOutPort("UdpDiscoveryOutPort", 12350, *this)
    , HttpPort("HttpPort", 3000, *this)
    , KeepAliveIntervalSec("KeepAliveIntervalSec", 1, *this)
    , PeriodicEnumeration("PeriodicEnumeration", true, *this)
{
}

// *************************************************************************************************

void HostConfigurationSection::ResetConfig()
{
    StringCommandPort = 12348;
    JsonCommandPort = 12346;
    EventPort = 12339;
    UdpDiscoveryInPort = 12349;
    UdpDiscoveryOutPort = 12350;
    HttpPort = 3000;
    KeepAliveIntervalSec = 1;
    PeriodicEnumeration = true;
}

LogConfigurationSection::LogConfigurationSection(JsonTree& configTreeRoot)
    : ConfigurationSection("LogCollector", configTreeRoot)
    , SuppressLogCollection("SupressLogCollection", false, *this)
    , PollingIntervalMs("PollingIntervalMs", 50, *this)
    , TargetLocation("TargetLocation", FileSystemOsAbstraction::GetOutputFilesLocation() + "Logs" + FileSystemOsAbstraction::GetDirectoryDelimiter(), *this)
    , ConversionFileLocation("ConversionFileLocation", FileSystemOsAbstraction::GetDefaultLogConversionFilesLocation(), *this)
    , MaxLogFileSize("MaxLogFileSize", 10, *this)
    , MaxNumOfLogFiles("MaxNumOfLogFiles", 10, *this)
{

}

void LogConfigurationSection::ResetConfig()
{
    SuppressLogCollection = false;
    PollingIntervalMs = 50;
    TargetLocation = FileSystemOsAbstraction::GetOutputFilesLocation() + "Logs" + FileSystemOsAbstraction::GetDirectoryDelimiter();
    ConversionFileLocation = FileSystemOsAbstraction::GetDefaultLogConversionFilesLocation();
    MaxLogFileSize = 10;
    MaxNumOfLogFiles = 10;
}

ModuleVerbositySection::ModuleVerbositySection(JsonTree& configTreeRoot, const std::string &cpuType) :
    ConfigurationSection((cpuType + "ModuleVerbosity").c_str(), configTreeRoot)
    , Module0("Module0", "EWI", *this)
    , Module1("Module1", "EWI", *this)
    , Module2("Module2", "EWI", *this)
    , Module3("Module3", "EWI", *this)
    , Module4("Module4", "EWI", *this)
    , Module5("Module5", "EWI", *this)
    , Module6("Module6", "EWI", *this)
    , Module7("Module7", "EWI", *this)
    , Module8("Module8", "EWI", *this)
    , Module9("Module9", "EWI", *this)
    , Module10("Module10", "EWI", *this)
    , Module11("Module11", "EWI", *this)
    , Module12("Module12", "EWI", *this)
    , Module13("Module13", "EWI", *this)
    , Module14("Module14", "EWI", *this)
    , Module15("Module15", "EWI", *this)
{
}


bool ValidateVerboseVal(const std::string& cs)
{
    for (const auto & c : cs)
    {
        switch (toupper(c))
        {
        case 'V':
        case 'I':
        case 'E':
        case 'W':
            break;
        default:
            return false;
        }
    }
    return true;
}

OperationStatus ModuleVerbositySection::UpdateIthModule(int moduleNum, std::string newVal)
{

    if (!ValidateVerboseVal(newVal))
    {
        std::stringstream ss;
        ss << "Trying to set verbosity of module " << moduleNum << "to invalid val: " << newVal;
        LOG_INFO << ss.str() << std::endl;
        return OperationStatus(false, ss.str());
    }
    switch (moduleNum)
    {
    case 0:
        Module0 = newVal;
        break;
    case 1:
        Module1 = newVal;
        break;
    case 2:
        Module2 = newVal;
        break;
    case 3:
        Module3 = newVal;
        break;
    case 4:
        Module4 = newVal;
        break;
    case 5:
        Module5 = newVal;
        break;
    case 6:
        Module6 = newVal;
        break;
    case 7:
        Module7 = newVal;
        break;
    case 8:
        Module8 = newVal;
        break;
    case 9:
        Module9 = newVal;
        break;
    case 10:
        Module10 = newVal;
        break;
    case 11:
        Module11 = newVal;
        break;
    case 12:
        Module12 = newVal;
        break;
    case 13:
        Module13 = newVal;
        break;
    case 14:
        Module14 = newVal;
        break;
    case 15:
        Module15 = newVal;
        break;
    default:
        std::stringstream ss;
        ss << "Trying to set the verbosity level of invalid module num: " << moduleNum << "to: " << newVal;
        LOG_INFO << ss.str() << std::endl;
        return OperationStatus(false, ss.str());
    }
    return OperationStatus();
}

// *************************************************************************************************

void ModuleVerbositySection::ResetConfig()
{
    Module0 = "EWI";
    Module1 = "EWI";
    Module2 = "EWI";
    Module3 = "EWI";
    Module4 = "EWI";
    Module5 = "EWI";
    Module6 = "EWI";
    Module7 = "EWI";
    Module8 = "EWI";
    Module9 = "EWI";
    Module10 = "EWI";
    Module11 = "EWI";
    Module12 = "EWI";
    Module13 = "EWI";
    Module14 = "EWI";
    Module15 = "EWI";
}


// *************************************************************************************************

PmcConfigurationSection::PmcConfigurationSection(JsonTree& configTreeRoot)
    : ConfigurationSection("Pmc", configTreeRoot)
    , TargetLocation("TargetLocation", FileSystemOsAbstraction::GetDefaultPmcFilesLocation(), *this)
{
}


PersistentConfiguration::PersistentConfiguration(const std::string& filePath)
    : JsonTree("")
    , HostConfiguration(*this)
    , LogCollectorConfiguration(*this)
    , FwModuleVerbosityConfiguration(*this, "Fw")
    , uCodeModuleVerbosityConfiguration(*this, "uCode")
    , PmcConfiguration(*this)
    , m_filePath(BuildConfigFilePath(filePath))
{
    Load(); // Overwrite default configuration with persistent configuration
    Save(); // Align persistent configuration with the up-to-date content (remove redundant/add new fields)

    CleanupOldFiles();
}

PersistentConfiguration::~PersistentConfiguration()
{
    Save();
}

void PersistentConfiguration::CommitChanges()
{
    Save();
}

void PersistentConfiguration::ResetConfig()
{
    FwModuleVerbosityConfiguration.ResetConfig();
    uCodeModuleVerbosityConfiguration.ResetConfig();
    LogCollectorConfiguration.ResetConfig();
    HostConfiguration.ResetConfig();
    // TODO: add reset PMC config
}

void PersistentConfiguration::Load()
{
    Json::Reader jsonReader;

    std::ifstream configFile(m_filePath);
    if (!configFile && ENOENT == errno)
    {
        // Configuration file does not exist
        LOG_INFO << "Configuration file does not exist at " << m_filePath << std::endl;
        return;
    }

    if (!configFile)
    {
        // Configuration will hold default hard-coded values
        LOG_ERROR << "Failed to load configuration file."
            << " Path: " << m_filePath
            << " Error: " << strerror(errno) << std::endl;
        return;
    }

    if (!jsonReader.parse(configFile, m_configRoot, true))
    {
        LOG_ERROR << "Failed to parse configuration file."
            << " Path: " << m_filePath
            << " Error: " << jsonReader.getFormattedErrorMessages()
            << std::endl;
        return;
    }

    LOG_INFO << "Loaded persistent configuration from: " << m_filePath << std::endl;
    LOG_VERBOSE << "Persistent configuration file content:\n" << m_configRoot << std::endl;

    Import(m_configRoot);
}

void PersistentConfiguration::Save()
{
    std::ofstream configFile(m_filePath);
    if (!configFile)
    {
        // Configuration will hold default hard-coded values
        LOG_ERROR << "Failed to export configuration to file."
            << " Path: " << m_filePath
            << " Error: " << strerror(errno) << std::endl;
        return;
    }

    m_configRoot.clear();
    Export(m_configRoot);

    Json::StyledStreamWriter writer;
    writer.write(configFile, m_configRoot);

    LOG_INFO << "Saved configuration file at " << m_filePath << std::endl;
    LOG_VERBOSE << "Persistent configuration file content:\n" << m_configRoot << std::endl;
}

std::string PersistentConfiguration::BuildConfigFilePath(const std::string& filePath)
{
    if (filePath.empty())
    {
        std::stringstream configFileBuilder;
        configFileBuilder << FileSystemOsAbstraction::GetConfigurationFilesLocation() << "host_manager_11ad.config";
        return configFileBuilder.str();
    }

    return filePath;
}

void PersistentConfiguration::CleanupOldFiles()
{
    std::stringstream ss;
    ss << FileSystemOsAbstraction::GetConfigurationFilesLocation() << "host_alias";
    const std::string oldAliasFile(ss.str());

    if (!FileSystemOsAbstraction::DoesFileExist(oldAliasFile))
    {
        LOG_DEBUG << "Backward compatibility cleanup: No old alias file found at: " << oldAliasFile << std::endl;
        return;
    }

    if (FileSystemOsAbstraction::RemoveFile(oldAliasFile))
    {
        LOG_INFO << "Backward compatibility cleanup: Deleted old alias file at: " << oldAliasFile << std::endl;
    }
    else
    {
        LOG_ERROR << "Backward compatibility cleanup: Cannot delete old alias file at: " << oldAliasFile << std::endl;
    }
}
