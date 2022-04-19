/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once
#include <string>
#include <vector>
#include <fstream>

#include "OperationStatus.h"
#include "LogCollectorDefinitions.h"


class Device;

namespace log_collector
{
    class LogRecorder
    {
    public:
        LogRecorder(
            const std::string &deviceName, CpuType tracerType, RecordingType recordingType);
        // recording methods
        bool WriteToOutputFile(const std::string& logLine);
        OperationStatus HandleRecording();
        OperationStatus PrepareRecording();
        void CloseOutputFile();
        void IncreaseCurrentOutputFileSize(size_t outputFileSize);
        const std::string& GetRecordingFolder() const { return m_logFilesFolder; }

        OperationStatus SetLogFilesFolder(const std::string& newPath);
        void SetLogFilesHeader(const std::string & newHeader) { m_fileHeader = newHeader; };
        void WriteFwInfo();
    private:
        std::string m_debugLogPrefix;

        // output file methods
        std::string GetNextOutputFileFullName() const;
        bool CreateNewOutputFile();

        void RemoveOldFilesIfNeeded();
        long GetNumOfLogFilesInFolder() const;
        std::vector<std::string> GetLogFilesSorted() const;
        bool RemoveOldestLogFileFromSortedVector(std::vector<std::string>& logFiles) const;

        // output file
        std::string m_currentOutputFileName;
        std::ofstream m_outputFile;
        size_t m_currentOutputFileSize;
        size_t m_currentNumOfOutputFiles;
        const std::string m_logFilePrefix;
        std::string m_logFilesFolder;
        std::string m_logFileExtension;
        std::string m_fileHeader;
    };
}
