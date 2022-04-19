/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <set>
#include "OperationStatus.h"
#include "LogCollectorDefinitions.h"
#include "ChunkConsumer.h"
#include "LogReader.h"
#include "DeviceLoggingContext.h"

namespace log_collector
{

    class LogCollectorService
    {
    public:
        LogCollectorService() : m_deferredRecordingEnabled(false), m_deferredLoggingType(XmlRecorder) {};
        ~LogCollectorService();

        void OnDeviceDiscovered(const std::string& deviceName);
        void OnDeviceRemoved(const std::string& deviceName);

        OperationStatus StartLogging(const std::string& deviceName, CpuType cpuType, LoggingType loggingType);
        OperationStatus StopLogging(const std::string& deviceName, CpuType cpuType, LoggingType loggingType);

        OperationStatus GetLogLoggingState(const std::string& deviceName, CpuType cpuType, bool& isLogging, LoggingType loggingType) const;
        void UpdateFwVersion(const std::string& deviceName);
        bool IsLogging();

        bool GetDeferredLoggingState() const;

        OperationStatus EnableDeferredLogging(LoggingType loggingType);
        void DisableDeferredLogging();
        OperationStatus SetLogCollectionModuleVerbosity(const std::string& deviceName, CpuType cpuType,
            const std::string& module, const std::string& level);

        bool GetModuleVerbosityMap(const std::string& deviceName, CpuType cpuType, std::vector<std::pair<std::string, std::string> > &verbosityData);
        // Old API
        std::string GetLogCollectionParameter(const std::string& deviceName, const std::string& cpuTypeName, const std::string& parameter);
        std::string GetLogCollectorConfigurationString(const std::string& deviceName, const std::string& cpuTypeName);
        bool SetLogCollectionParameter(const std::string& deviceName, const std::string& cpuTypeName, const std::string& parameterAssignment, std::string& errorMessage);

        // delete copy constructor and assignment operator
        // keep public for better error message
        // no move members will be declared implicitly
        LogCollectorService(const LogCollectorService&) = delete;
        LogCollectorService& operator=(const LogCollectorService&) = delete;
    private:
        std::unordered_map<std::string, std::unique_ptr<DeviceLoggingContext> > m_DeviceLoggingContextMap;

        bool m_deferredRecordingEnabled;
        LoggingType m_deferredLoggingType;

        // oldApi
        std::shared_ptr<LogReader> GetLogCollector(const std::string& deviceName, const std::string& cpuTypeName, std::string& errorMessage);
        std::string GetAllModulesVerbosityString(const std::string& deviceName, const std::string& cpuType);

    };

}


