/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/


#include "OperationStatus.h"
#include "RawPublisherChunkConsumer.h"
#include "DeviceManager.h"
#include "Host.h"
#include "PersistentConfiguration.h"
#include "FwStateProvider.h"
#include "RawLogLine.h"
#include "LogCollectorEvents.h"


using namespace std;

namespace log_collector
{


    RawPublisherChunkConsumer::RawPublisherChunkConsumer(const std::string& deviceName, CpuType tracerType)
        : ChunkConsumer(deviceName, tracerType)
    {
    }

    RawPublisherChunkConsumer::~RawPublisherChunkConsumer()
    {
        Stop();
    }

    void RawPublisherChunkConsumer::OnStartNewChunk(const TimeStamp& ts)
    {
        (void)ts;
        LOG_VERBOSE << __FUNCTION__ << ": "
            << " Log publishing for: " << m_deviceName
            << " for cpu: " << m_tracerCpuType
            << " started new buffer" << endl;
    }

    void RawPublisherChunkConsumer::ReportBufferOverrun(unsigned numMissedDwords)
    {
       m_rawLogLinesForEvent.emplace_back(RawLogLine::BUFFER_OVERRUN, numMissedDwords);
       LOG_DEBUG << __FUNCTION__ << ": "
           << " Log publishing for: " << m_deviceName
           << " for cpu: " << m_tracerCpuType
           << " Found buffer overrun - missed DWORDS: " << numMissedDwords << endl;
    }

    void RawPublisherChunkConsumer::ReportDeviceRestarted()
    {
        m_rawLogLinesForEvent.emplace_back(RawLogLine::DEVICE_WAS_RESTARTED);
        LOG_DEBUG << __FUNCTION__ << ": "
            << " Log publishing for: " << m_deviceName
            << " for cpu: " << m_tracerCpuType
            << " Device was restarted " << endl;
    }

    void RawPublisherChunkConsumer::ReportReadOverrun()
    {
        m_rawLogLinesForEvent.emplace_back(RawLogLine::RPTR_LARGER_THAN_WPTR);
        LOG_DEBUG << __FUNCTION__ << ": "
            << " Log publishing for: " << m_deviceName
            << " for cpu: " << m_tracerCpuType
            << " Read more lines than were created " << endl;
    }

    void RawPublisherChunkConsumer::ReportCorruptedEntry(unsigned signature)
    {
        m_rawLogLinesForEvent.emplace_back(RawLogLine::INVALID_SIGNATURE, 1);
        LOG_DEBUG << __FUNCTION__ << ": "
            << " Log publishing for: " << m_deviceName
            << " for cpu: " << m_tracerCpuType
            << " Got corrupted entry, signature: " << signature << endl;
    }

    void RawPublisherChunkConsumer::ReportAndUpdateFwVersion(const FwStateProvider* fwStateProvider,
        const module_level_info* moduleLevelInfoArray)
    {
        (void)fwStateProvider;
        (void)moduleLevelInfoArray;
    }


    void RawPublisherChunkConsumer::ConsumeMessage(const log_event* logEvent, unsigned rptr, unsigned bufferSize,
                                                 unsigned numOfDwords)
    {
        const union log_event *evtHead = &logEvent[rptr % bufferSize]; // h->evt is the log line payload.
        if (numOfDwords > (MaxParams * 2))
        {
            LOG_VERBOSE << __FUNCTION__ << ": "
                << " for device: " << m_deviceName
                << " for cpu: " << m_tracerCpuType
                << " invalid number of parameters provided number is: " << numOfDwords
                << " max val is: " << (MaxParams * 2) << endl;
            return;
        }

        std::vector<int> dwords; // parameters array (each log line can have at most three parameters). size need to be set to 3 due to log collector's old output format (each line has three params where dummy params has the value of 0)
        dwords.reserve(numOfDwords);
        for (unsigned i = 0; i < numOfDwords; i++)
        {
            dwords.push_back(logEvent[(rptr + i + 1) % bufferSize].dword);
        }
        unsigned stringOfset = evtHead->hdr.string_offset << 2; // string_ofst is in bytes.

        m_rawLogLinesForEvent.emplace_back(evtHead->hdr.module, evtHead->hdr.level, evtHead->hdr.signature, stringOfset, evtHead->hdr.is_string, dwords);
    }

void RawPublisherChunkConsumer::PublishLogs() const
{
    if ( !m_rawLogLinesForEvent.empty())
    {
        LOG_VERBOSE << __FUNCTION__ << ": "
        <<  " tracer is going to publish for: " << m_deviceName
        << " for cpu: " << m_tracerCpuType
        << " buffer size:" << m_rawLogLinesForEvent.size()
        << std::endl;
        // no problem to reset the buffer right after. rawLine objects are being copied.
        Host::GetHost().PushEvent(NewLogsEvent(m_deviceName, m_tracerCpuType, m_rawLogLinesForEvent));
    }
}


    void RawPublisherChunkConsumer::OnEndChunk()
    {
        PublishLogs();
        m_rawLogLinesForEvent.clear();
    }

    void RawPublisherChunkConsumer::Stop()
    {
        LOG_DEBUG << __FUNCTION__ << ": "
            <<  " Log publishing stopped for: " << m_deviceName
            << " for cpu: " << m_tracerCpuType << std::endl;
        m_isActive = false;
    }

    OperationStatus RawPublisherChunkConsumer::Activate(const module_level_info* moduleLevelInfoArray,
                                                        const FwStateProvider* fwStateProvider)
    {
        (void)moduleLevelInfoArray;
        (void)fwStateProvider;
        m_isActive = true;
        return OperationStatus();
    }

}
