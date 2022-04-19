/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <algorithm>
#include <cstring>
#include <fstream>

#include "Host.h"
#include "PersistentConfiguration.h"
#include "LogRecorder.h"
#include "LogCollectorStopRecordingHandler.h"
#include "LogReader.h"
#include "Device.h"
#include "FileSystemOsAbstraction.h"

using namespace std;

namespace log_collector
{

    LogRecorder::LogRecorder(const std::string& deviceName, CpuType tracerType, RecordingType recordingType) :
        m_debugLogPrefix(deviceName + "_" + CPU_TYPE_TO_STRING[tracerType] + " recorder:"),
        m_currentOutputFileSize(0),
        m_currentNumOfOutputFiles(0),
        m_logFilePrefix(deviceName + "_" + CPU_TYPE_TO_STRING[tracerType] + "_"),
        m_logFilesFolder(Host::GetHost().GetConfiguration().LogCollectorConfiguration.TargetLocation),
        m_logFileExtension((recordingType == RECORDING_TYPE_XML) ? ".log" : ".txt")
    {
        if (!FileSystemOsAbstraction::DoesFolderExist(m_logFilesFolder) && !FileSystemOsAbstraction::CreateFolder(m_logFilesFolder))
        {
            LOG_ERROR << m_debugLogPrefix << "Failed to create logs folder " << m_logFilesFolder << endl;
        }
    }

    OperationStatus LogRecorder::PrepareRecording()
    {
        m_currentOutputFileSize = 0;
        m_currentNumOfOutputFiles = GetNumOfLogFilesInFolder();

        CloseOutputFile();
        if (!CreateNewOutputFile())
        {
            stringstream debugSs(m_debugLogPrefix);
            debugSs << "Failed to create the first output file";
            LOG_ERROR << debugSs.str() << endl;
            return OperationStatus(false, debugSs.str());
        }

        LOG_DEBUG << m_debugLogPrefix << "Log collector preparation was successfully finished recorder type: " <<  endl;
        return OperationStatus();
    }

    OperationStatus LogRecorder::HandleRecording()
    {
        const size_t maxSingleFileSizeInByte = Host::GetHost().GetConfiguration().LogCollectorConfiguration.MaxLogFileSize * 1024 * 1024;
        // split output file if required
        if (maxSingleFileSizeInByte > 0 && m_currentOutputFileSize > maxSingleFileSizeInByte)
        {
            if (m_logFileExtension == ".log" && m_outputFile.is_open())
            {
                // in case that we are working on xml log file
                WriteToOutputFile("</Logs></LogFile>");
            }
            CloseOutputFile();
            RemoveOldFilesIfNeeded();
            if (!CreateNewOutputFile())
            {
                LOG_ERROR << m_debugLogPrefix << "failed to create a new output file" << endl;
                return OperationStatus(false, m_debugLogPrefix + "failed to create a new output file");
            }
        }
        return OperationStatus(true);
    }

    void LogRecorder::IncreaseCurrentOutputFileSize(size_t outputFileSize)
    {
        m_currentOutputFileSize += outputFileSize;
        LOG_VERBOSE << m_debugLogPrefix << "Current File Size = " << m_currentOutputFileSize << endl;
    }

    OperationStatus LogRecorder::SetLogFilesFolder(const std::string& newPath)
    {
        if (!FileSystemOsAbstraction::DoesFolderExist(newPath))
        {
            LOG_DEBUG <<  "Recording directory does not exist creating one at: " << newPath << endl;
            if (!FileSystemOsAbstraction::CreateFolder(newPath))
            {
                LOG_ERROR << m_debugLogPrefix << "Failed to create logs folder " << newPath << endl;
                return OperationStatus(false, "Failed to create logs folder " + newPath);
            }
        }
        m_logFilesFolder = newPath;
        if (newPath[newPath.length()-1] != '/')
        {
            m_logFilesFolder += "/";
        }

        return OperationStatus();
    }

    void LogRecorder::WriteFwInfo()
    {
        WriteToOutputFile(m_fileHeader);
    }

    string LogRecorder::GetNextOutputFileFullName() const
    {
        std::stringstream ss;
        ss << m_logFilesFolder << m_logFilePrefix << Utils::GetCurrentLocalTimeForFileName()
            <<  m_logFileExtension;
        return ss.str();
    }

    bool LogRecorder::CreateNewOutputFile()
    {
        m_currentOutputFileName = GetNextOutputFileFullName();
        LOG_INFO << m_debugLogPrefix << "Creating output file: " << m_currentOutputFileName << endl;

        m_outputFile.open(m_currentOutputFileName.c_str());
        if (m_outputFile.fail())
        {
            LOG_ERROR << m_debugLogPrefix << "Error opening output file: " << m_currentOutputFileName << " Error: " << strerror(errno) << endl;
            return false;
        }

        m_currentNumOfOutputFiles++;
        m_outputFile << m_fileHeader;
        return true;
    }

    void LogRecorder::CloseOutputFile()
    {
        if (m_outputFile.is_open())
        {
            m_outputFile.close();
            LOG_INFO << m_debugLogPrefix << "Output file closed" << endl;
        }
        m_currentOutputFileSize = 0;
    }

    void LogRecorder::RemoveOldFilesIfNeeded()
    {
        uint32_t maxNumOfLogFiles = Host::GetHost().GetConfiguration().LogCollectorConfiguration.MaxNumOfLogFiles;
        if (0 == maxNumOfLogFiles)
        {
            // no need to remove old files
            return;
        }

        int numOfRemovedLogFiles = 0;
        vector<string> logFiles = GetLogFilesSorted();

        while (!logFiles.empty() &&
            m_currentNumOfOutputFiles >= maxNumOfLogFiles)
        {
            if (!RemoveOldestLogFileFromSortedVector(logFiles))
            {
                LOG_WARNING << m_debugLogPrefix << "Skipping old log files removal for tracer" << endl;
                break;
            }
            numOfRemovedLogFiles++;
            m_currentNumOfOutputFiles = GetNumOfLogFilesInFolder();
        }
        LOG_DEBUG << m_debugLogPrefix << "Removed " << numOfRemovedLogFiles << " oldest log files from the folder. Current tracer's log file size in folder: " << m_currentNumOfOutputFiles << endl;
    }

    bool LogRecorder::RemoveOldestLogFileFromSortedVector(vector<string>& logFiles) const
    {
        if (logFiles.empty())
        {
            return false;
        }
        LOG_INFO << m_debugLogPrefix << "Remove old log file: " << (*logFiles.begin()).c_str() << endl;
        if (std::remove((m_logFilesFolder + *logFiles.begin()).c_str()))
        {
            LOG_WARNING << m_debugLogPrefix << "Failed to remove old log file " << m_logFilesFolder << (*logFiles.begin()).c_str() << " error: " << std::strerror(errno) << endl;
            return false;
        }
        logFiles.erase(logFiles.begin());
        return true;
    }

    vector<string> LogRecorder::GetLogFilesSorted() const
    {
        vector<string> files = FileSystemOsAbstraction::GetFilesByPattern(m_logFilesFolder, m_logFilePrefix, m_logFileExtension);
        std::sort(files.begin(), files.end());
        return files;
    }

    long LogRecorder::GetNumOfLogFilesInFolder() const
    {
        return static_cast<long>(
            FileSystemOsAbstraction::GetFilesByPattern(m_logFilesFolder, m_logFilePrefix, m_logFileExtension).size());
    }


    bool LogRecorder::WriteToOutputFile(const string& logLine)
    {
        if (m_outputFile.is_open())
        {
            m_outputFile << logLine;
            m_currentOutputFileSize += logLine.size();
            return true;
        }
        LOG_ERROR << "The file: " + m_currentOutputFileName + " is not open! Can't print the line: " <<logLine << endl;
        return false;
    }
}
