/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "JsonHandlerSDK.h"
#include "Utils.h"
#include "OperationStatus.h"

// *************************************************************************************************

// IJsonSerializable represents data structures that can be serialized to/de-serialized from json structures.
//
class IJsonSerializable
{
protected:
    virtual ~IJsonSerializable() = default;

public:
    virtual void Import(const Json::Value& jsonHostingContainer) = 0;
    virtual void Export(Json::Value& jsonHostingContainer) const = 0;
};

// *************************************************************************************************

// JsonTree holds a hierarchical json-compatible data structure. The tree hierarchy is established
// by constructing nesting objects of this class. Only
class JsonTree : public IJsonSerializable
{
public:

    void Import(const Json::Value& jsonHostingContainer) override;
    void Export(Json::Value& jsonHostingContainer) const override;

    void AddChildNode(IJsonSerializable* pJsonValueMap);

protected:

    // This class can only serve as a base class for real tree nodes. Derived classes
    // are expected to define properties that will be stored in the children container
    // for easy enumeration.
    // Non-empty name results adds a key when the content is exported. Otherwise json file
    // scope is supposed.
    explicit JsonTree(const char* szName = nullptr);

private:

    // Tree level name. Interpreted as a key for nested values and supposed to be empty if this node is
    // serialized as a whole json file.
    const std::string m_name;

    // Nested level json structures. Objects are supposed to be members of a derived class to provide
    // users with function-based API to access parameters (in opposite to string-based json API).
    // therefore children container contains raw pointers and does not manage their lifetime.
    std::vector<IJsonSerializable *> m_children;
};

// *************************************************************************************************

// Configuration parameter is a leaf node of a json tree, the type safety is achieved by a template
// parameter and only a limited set of types is supported by the json parser.If a non-supported type
// is used the code won't compile.
template<typename T>
class ConfigurationParameter final : public IJsonSerializable
{
public:

    ConfigurationParameter(const char* szName, T defaultValue, JsonTree& hostingContainer)
        : m_name(szName)
        , m_value(std::move(defaultValue))
    {
        hostingContainer.AddChildNode(this);
    }

    void Import(const Json::Value& jsonConfigurationSection) override
    {
        JsonValueBoxed<T> jsonValueBoxed = JsonValueExtractor::Parse<T>(jsonConfigurationSection, m_name.c_str());
        if (jsonValueBoxed.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
        {
            m_value = jsonValueBoxed.GetValue();
        }
    }

    void Export(Json::Value& jsonConfigurationSection) const override
    {
        jsonConfigurationSection[m_name.c_str()] = m_value;
    }

    const std::string& Name() const { return m_name; }

    // Get/set a value
    operator T() const { return m_value; }
    ConfigurationParameter<T>& operator=(const T& value) { m_value = value; return *this; }

private:
    const std::string m_name;
    T m_value;
};

// *************************************************************************************************

// Configuration section is a json tree that holds single parameters. Section is a child of a
// configuration file. Real configuration sections are supposed to derive from this class.
class ConfigurationSection : public JsonTree
{
public:
    virtual void ResetConfig() = 0;
protected:
    ConfigurationSection(const char* szName, JsonTree& configTreeRoot);
};

// *************************************************************************************************

class HostConfigurationSection final : public ConfigurationSection
{
public:
    explicit HostConfigurationSection(JsonTree& configTreeRoot);

    ConfigurationParameter<uint32_t> StringCommandPort;
    ConfigurationParameter<uint32_t> JsonCommandPort;
    ConfigurationParameter<uint32_t> EventPort;
    ConfigurationParameter<uint32_t> UdpDiscoveryInPort;
    ConfigurationParameter<uint32_t> UdpDiscoveryOutPort;
    ConfigurationParameter<uint32_t> HttpPort;
    ConfigurationParameter<uint32_t> KeepAliveIntervalSec;
    ConfigurationParameter<bool>     PeriodicEnumeration;
    void ResetConfig() override;
};

class LogConfigurationSection final : public ConfigurationSection
{
public:
    explicit LogConfigurationSection(JsonTree& configTreeRoot);
    ConfigurationParameter<bool> SuppressLogCollection;
    ConfigurationParameter<uint32_t> PollingIntervalMs;
    ConfigurationParameter<std::string> TargetLocation;
    ConfigurationParameter<std::string> ConversionFileLocation;
    ConfigurationParameter<uint32_t> MaxLogFileSize;
    ConfigurationParameter<uint32_t> MaxNumOfLogFiles;
    void ResetConfig() override;
};

class PmcConfigurationSection final : public ConfigurationSection
{
public:
    explicit PmcConfigurationSection(JsonTree& configTreeRoot);

    ConfigurationParameter<std::string> TargetLocation;
    void ResetConfig() override {};

};

class ModuleVerbositySection final : public ConfigurationSection
{
public:
    explicit ModuleVerbositySection(JsonTree& configTreeRoot, const std::string &cpuType);

    OperationStatus UpdateIthModule(int moduleNum, std::string newVal);
    ConfigurationParameter<std::string> Module0;
    ConfigurationParameter<std::string> Module1;
    ConfigurationParameter<std::string> Module2;
    ConfigurationParameter<std::string> Module3;
    ConfigurationParameter<std::string> Module4;
    ConfigurationParameter<std::string> Module5;
    ConfigurationParameter<std::string> Module6;
    ConfigurationParameter<std::string> Module7;
    ConfigurationParameter<std::string> Module8;
    ConfigurationParameter<std::string> Module9;
    ConfigurationParameter<std::string> Module10;
    ConfigurationParameter<std::string> Module11;
    ConfigurationParameter<std::string> Module12;
    ConfigurationParameter<std::string> Module13;
    ConfigurationParameter<std::string> Module14;
    ConfigurationParameter<std::string> Module15;

    void ResetConfig() override;
};

// *************************************************************************************************

// PersistentConfiguration is a root of the configuration tree. It holds configuration sections (which,
// in turn hold configuration parameters.
class PersistentConfiguration final : public JsonTree, public Immobile
{
public:
    explicit PersistentConfiguration(const std::string& filePath);
    ~PersistentConfiguration();

    // User may explicitly ask to save the configuration. This API is not mandatory as serialization is
    // automatically performed on application exit (up to crashes).
    void CommitChanges();
    void ResetConfig();

    HostConfigurationSection HostConfiguration;
    LogConfigurationSection LogCollectorConfiguration;
    ModuleVerbositySection FwModuleVerbosityConfiguration;
    ModuleVerbositySection uCodeModuleVerbosityConfiguration;
    PmcConfigurationSection PmcConfiguration;

private:

    const std::string m_filePath;
    Json::Value m_configRoot;

    void Load();
    void Save();

    static std::string BuildConfigFilePath(const std::string& filePath);
    static void CleanupOldFiles();

};
