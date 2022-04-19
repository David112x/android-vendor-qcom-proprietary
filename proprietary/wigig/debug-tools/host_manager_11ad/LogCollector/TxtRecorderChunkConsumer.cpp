/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/


#include "OperationStatus.h"
#include "TxtRecorderChunkConsumer.h"
#include "DeviceManager.h"
#include "Host.h"
#include "PersistentConfiguration.h"
#include "Device.h"
#include "FwStateProvider.h"


using namespace std;

namespace log_collector
{
    TxtRecorderChunkConsumer::TxtRecorderChunkConsumer(const std::string& deviceName, CpuType tracerType)
        : ChunkConsumer(deviceName, tracerType)
        , m_logCollectorRecorder(deviceName, tracerType, RECORDING_TYPE_TXT)
    {
    }

    TxtRecorderChunkConsumer::~TxtRecorderChunkConsumer()
    {
        Stop();
    }

    void TxtRecorderChunkConsumer::OnStartNewChunk(const TimeStamp& ts)
    {
        m_timeStampStr = Utils::GetTimeString(ts);
        m_isActive =  m_logCollectorRecorder.HandleRecording();
    }

    void TxtRecorderChunkConsumer::ReportBufferOverrun(unsigned numMissedDwords)
    {
        stringstream missedDwordsMessage;
        missedDwordsMessage << BufferOverrunMessage << numMissedDwords << endl;
        if (!LogReader::Tracer3ppLogsFilterOn)
        {
            m_isActive = m_logCollectorRecorder.WriteToOutputFile(missedDwordsMessage.str()); // DWORD == uint32
        }
    }

    void TxtRecorderChunkConsumer::ReportDeviceRestarted()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(DeviceRestartedMessage);
    }

    void TxtRecorderChunkConsumer::ReportReadOverrun()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(ReadOverrunMessage);
    }

    void TxtRecorderChunkConsumer::ReportCorruptedEntry(unsigned signature)
    {
        stringstream ss;
        ss << CorruptedEntryMessage << signature << endl;
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(ss.str());
    }

    void TxtRecorderChunkConsumer::ReportDeviceRemoved()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(DeviceRemovedMessage);
        m_logCollectorRecorder.CloseOutputFile();
    }

    void TxtRecorderChunkConsumer::ReportDeviceDiscovered()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(DeviceDiscoveredMessage);
    }

    void TxtRecorderChunkConsumer::ReportTracerWasRestarted()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(TracerWasRestartedMessage);
    }

    void TxtRecorderChunkConsumer::ReportAndUpdateFwVersion(const FwStateProvider* fwStateProvider,
        const module_level_info* moduleLevelInfoArray)
    {
        SetLogFileHeader(moduleLevelInfoArray, fwStateProvider);
        OperationStatus os = m_logTxtParser.LogParserInit(string(Host::GetHost().GetConfiguration().LogCollectorConfiguration.ConversionFileLocation).c_str(), m_tracerCpuType, moduleLevelInfoArray);
        //LOG_DEBUG << " in: " << __FUNCTION__ << " m_validity  for LogParserInit is: " << os.IsSuccess() << " Message: " << os.GetStatusMessage() << endl;
        if (!os)
        {
            LOG_ERROR << __FUNCTION__ << ": Failed to init Log Parser: " << os.GetStatusMessage() << endl;
            m_isActive = false;
        }
        m_logCollectorRecorder.WriteFwInfo();
    }

    void TxtRecorderChunkConsumer::ConsumeMessage(const log_event* logEvent, unsigned rptr, unsigned bufferSize,
                                                unsigned numOfDwords)
    {
        const union log_event *evtHead = &logEvent[rptr % bufferSize]; // h->evt is the log line payload.
        if (numOfDwords > (MaxParams * 2))
        {
            stringstream ss;
            ss << "**** invalid number of parameters provided number is: " << numOfDwords << " max val is: " << (MaxParams * 2) << endl;
            m_isActive = m_logCollectorRecorder.WriteToOutputFile(ss.str());
            return ;
        }

        // TODOR: vector optimization
        vector<int> params(numOfDwords);
        for (unsigned i = 0; i < numOfDwords; i++)
        {
            params[i] = (logEvent[(rptr + i + 1) % bufferSize].dword);
        }
        unsigned stringOffset = evtHead->hdr.string_offset << 2;
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(m_logTxtParser.ParseLogMessage(stringOffset, params, evtHead->hdr.module, evtHead->hdr.level, m_timeStampStr));
    }

    void TxtRecorderChunkConsumer::OnEndChunk()
    {
    }

    void TxtRecorderChunkConsumer::Stop()
    {
        m_isActive = false;
        m_logCollectorRecorder.CloseOutputFile();
    }

    OperationStatus TxtRecorderChunkConsumer::Activate(const module_level_info* moduleLevelInfoArray,
                                                       const FwStateProvider* fwStateProvider)
    {
        SetLogFileHeader(moduleLevelInfoArray, fwStateProvider);
        OperationStatus os;
        os = m_logCollectorRecorder.SetLogFilesFolder(Host::GetHost().GetConfiguration().LogCollectorConfiguration.TargetLocation);
        if (!os)
        {
            m_isActive = false;
            return os;
        }
        os = m_logTxtParser.LogParserInit(string(Host::GetHost().GetConfiguration().LogCollectorConfiguration.ConversionFileLocation).c_str(), m_tracerCpuType, moduleLevelInfoArray);
        LOG_DEBUG << " in: " << __FUNCTION__ << " m_validity  for LogParserInit is: " << os.IsSuccess() << " Message: " << os.GetStatusMessage() << endl;
        if (!os)
        {
            m_isActive = false;
            return os;
        }
        os = m_logCollectorRecorder.PrepareRecording();
        LOG_DEBUG << " in: " << __FUNCTION__ << " m_validity for PrepareRecording is: " << os.IsSuccess() << " Message: " << os.GetStatusMessage() << endl;
        if (!os)
        {
            m_isActive = false;
            return os;
        }
        m_isActive = true;
        return os;
     }


    void TxtRecorderChunkConsumer::SetLogFileHeader(const module_level_info* moduleLevelInfoArray, const FwStateProvider* fwStateProvider)
    {
        std::ostringstream headerBuilder;

        const FwVersion fwVersion = fwStateProvider->GetFwVersion();
        const FwTimestamp fwTs = fwStateProvider->GetFwTimestamp();
        const FwTimestamp uCodeTs = fwStateProvider->GetUcodeTimestamp();

        headerBuilder << "Logs for FW version: " << fwVersion.m_major << "."
            << fwVersion.m_minor << "."
            << fwVersion.m_subMinor << "."
            << fwVersion.m_build << endl;

        headerBuilder << "FW Compilation Time: "
            << fwTs << endl;
        headerBuilder << "uCode Compilation Time: "
            << uCodeTs << endl;

        headerBuilder << " Third party mode: [" ;
        bool first = true;
        for (int i = 0; i < NUM_MODULES; i++)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                headerBuilder << ", ";
            }
            headerBuilder << static_cast<int>(moduleLevelInfoArray[i].third_party_mode);
        }
        headerBuilder << "]" << endl;
        m_logCollectorRecorder.SetLogFilesHeader(headerBuilder.str());
    }
}
