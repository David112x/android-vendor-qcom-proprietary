/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <memory>
#include <vector>
#include <string>

#include "OperationStatus.h"
#include "ChunkConsumer.h"
#include "BufferConsumer.h"


class Device;

namespace log_collector
{

    class LogReader
    {
    public:
        static bool Tracer3ppLogsFilterOn;

        LogReader(const std::string& deviceName, CpuType tracerType);
        LogReader(const LogReader& lg) = delete;
        LogReader& operator=(const LogReader& lc) = delete;
        ~LogReader();

        // should be called periodically until valid.
        bool UpdateDeviceInfo();
        OperationStatus ActivateLogging(LoggingType loggingType);
        OperationStatus RestoreLogging();
        void DeactivateLogging(LoggingType loggingType);

        bool IsLoggingValid(LoggingType loggingType) const;
        bool IsLoggingValid() const;

        bool IsLogging(LoggingType loggingType) const;
        bool IsLogging() const;

        // configuration functions
        OperationStatus SetModuleVerbosity(const std::string& module, const std::string& level);

        bool GetModuleLevelInfo(std::vector<std::pair<std::string, std::string> > &verbosityData);

        bool SetPollingInterval(unsigned pollingInterval) const;

        // for old api
        bool SetModuleVerbosityFromAssignment(const std::string& assignment);

        void ReportDeviceRemoved();
        void RegisterPollerAndReportDeviceUp();

        void UnRegisterPoller();

    private:
        void RegisterPoller();
        OperationStatus ActivateLoggingInner(LoggingType loggingType);
        mutable std::mutex m_pollDestructionMutex;
        unsigned GetAhbStartAddress(BasebandType bb) const;
        unsigned GetLinkerStartAddress(BasebandType bb) const;
        bool ComputeLogBufferStartAddress();
        int GetBufferSizeInBytesById(int bufferSizeId) const;
        bool SetModuleVerbosityInner(const std::string& module, const std::string& level);
        bool SetMultipleModulesVerbosity(const std::string& modulesVerbosity);
        // configurations
        OperationStatus ApplyModuleVerbosity();

        bool GetModuleInfoFromDevice() const;
        // OS agnostic read log function
        bool ReadLogBuffer();

        void GetNextLogs(); // distribute next logs chunck (without log lines that were already read)

        void Poll();

        const std::string m_deviceName;
        const CpuType m_tracerCpuType;
        const std::string m_debugLogPrefix;

        unsigned m_pollerTaskId;
        //bool m_paused;

        // Indicates that the log buffer is initialized.
        bool m_valid;

        // Indicates whether the fw initialized or not.
        bool m_fwInit;
        // log buffer members
        std::unique_ptr<log_buffer> m_logHeader; // content of log buffer header
        int m_logAddress; // log buffer start address
        std::vector<unsigned char> m_logBufferContent; // log buffer content
        unsigned m_logBufferSizeInDwords;

        std::unique_ptr<BufferConsumer> m_bufferConsumer;
        int m_prevLogOffset;
    };

}


