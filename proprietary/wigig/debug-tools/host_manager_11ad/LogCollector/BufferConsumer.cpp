/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/


#include "BufferConsumer.h"
#include "OperationStatus.h"
#include "ChunkConsumer.h"
#include "XmlRecorderChunkConsumer.h"
#include "TxtRecorderChunkConsumer.h"
#include "LogReader.h"
#include "RawPublisherChunkConsumer.h"
#include "FwStateProvider.h"

using namespace std;


namespace log_collector
{

    BufferConsumer::BufferConsumer(const std::string& deviceName, CpuType tracerType,
        unsigned logBufferContentSizeInDwords) :
        m_deviceName(deviceName), m_rptr(0), m_lastWptr(0), m_logBufferContentSizeInDwords(logBufferContentSizeInDwords), m_cpuType(tracerType)
    {
        m_chunkConsumers[XmlRecorder].reset(new XmlRecorderChunkConsumer(deviceName, tracerType));
        m_chunkConsumers[TxtRecorder].reset(new TxtRecorderChunkConsumer(deviceName, tracerType));
        m_chunkConsumers[RawPublisher].reset(new RawPublisherChunkConsumer(deviceName, tracerType));
    }

    BufferConsumer::~BufferConsumer() = default;

    bool BufferConsumer::Reset(const module_level_info* moduleLevelInfoArray, unsigned logBufferContentSizeInDwords, const FwStateProvider* fwStateProvider)
    {
        // Check if there were active recordings before. (we check only recordings since DmTools take care of its recordings)
        m_logBufferContentSizeInDwords = logBufferContentSizeInDwords;

        const bool xmlRecordingActive = IsActive(XmlRecorder);
        const bool txtRecordingActive = IsActive(TxtRecorder);


        // Reactivating the recordings if needed.
        OperationStatus os;
        if (xmlRecordingActive)
        {
            StopConsumer(XmlRecorder);
            os = ActivateChunkConsumer(XmlRecorder, moduleLevelInfoArray, fwStateProvider);
            if (!os)
            {
                LOG_ERROR << "Failed to reactivate XmlRecorder Logs: " << os.GetStatusMessage() << endl;

                return false; // no need to un-pause recording.
            }
        }
        if (txtRecordingActive)
        {
            StopConsumer(TxtRecorder);
            os = ActivateChunkConsumer(TxtRecorder, moduleLevelInfoArray, fwStateProvider);
            if (!os)
            {
                LOG_ERROR << "Failed to reactivate TxtRecorder Logs: " << os.GetStatusMessage() << endl;

                return false; // no need to un-pause recording.
            }
        }
        return true;
    }

    OperationStatus BufferConsumer::ActivateChunkConsumer(
        LoggingType loggingType, const module_level_info* moduleLevelInfoArray, const FwStateProvider* fwStateProvider)
    {
        return m_chunkConsumers[loggingType]->Activate(moduleLevelInfoArray, fwStateProvider);
    }

    void BufferConsumer::SetActiveFlag(LoggingType loggingType)
    {
        m_chunkConsumers[loggingType]->SetActiveFlag();
    }

    void BufferConsumer::ConsumeBuffer(log_buffer* logBuffer)
    {
        const unsigned wptr = logBuffer->write_ptr;
        if (m_lastWptr ==0)
        {
            LOG_DEBUG << "On tracer: "<< m_cpuType << "@" << m_deviceName << " first write ptr is: " << wptr << endl;
            if (wptr > m_logBufferContentSizeInDwords)
            {
                LOG_DEBUG << "On tracer: " << m_cpuType << "@" << m_deviceName << "first write ptr is bigger than buffer size :" << m_logBufferContentSizeInDwords << endl;
            }
        }
        if (wptr == m_rptr)
        {
            // Nothing to read.
            return;
        }

        if (wptr > m_rptr && (wptr == m_lastWptr))
        {
            LOG_DEBUG << "Interface is probably down, wptr > m_rptr && wptr == m_lastWptr: wptr ==" << m_lastWptr << ", m_rptr == " << m_rptr  << endl;
            return;
        }
        TimeStamp ts = Utils::GetCurrentLocalTime();
        StartNewBuffer(ts);

        if (wptr < m_lastWptr) // device was restarted
        {
            // previously was just retun 0
            LOG_WARNING << "device was restarted (wptr < m_lastWptr: wptr == " << wptr << ", m_rptr == " << m_rptr << ", m_lastWptr: " << m_lastWptr << ")" << endl;
            m_lastWptr = wptr;
            ReportDeviceRestarted();
            //m_rptr = max (0, wptr-m_logBufferContentSizeInDwords)
            if (wptr <= m_logBufferContentSizeInDwords)
            {
                m_rptr = 0;
            }
            else
            {
                m_rptr = wptr - m_logBufferContentSizeInDwords;// no read more that one buffer
            }
            LOG_WARNING << "pointers were reset (  m_rptr == wptr - (wptr % m_logBufferSizeInDwords) == " << m_rptr << ", m_lastWptr == wptr : " << m_lastWptr << ")" << endl;
        }

        if (wptr < m_rptr) // read more lines than were created
        {
            LOG_WARNING << "read more lines than were created (wptr < m_rptr: wptr == " << wptr << ", m_rptr == " << m_rptr << ", m_lastWptr: " << m_lastWptr << ")" << endl;
            ReportReadOverrun();
            m_lastWptr = wptr;
            m_rptr = wptr; // we might miss few messages here.

            LOG_WARNING << "pointers were reset (  m_rptr == wptr  == " << m_rptr << ", m_lastWptr == wptr : " << m_lastWptr << ")" << endl;
            return; // no need to continue.
        }

        bool corruptedEntryExpected = false;
        if (wptr > m_logBufferContentSizeInDwords + m_rptr)
        {
            // overflow; try to parse last wrap
            corruptedEntryExpected = true; // set to true since buffer overrun occurred but not yet handled, corrupted lines expected.
            stringstream missedDwords;
            missedDwords << wptr - m_rptr - m_logBufferContentSizeInDwords;
            if (!LogReader::Tracer3ppLogsFilterOn && m_rptr > 0) // we check m_rptr> 0 so that we wont print buffer overrun message on start recording.
            {
                ReportBufferOverrun(wptr - m_rptr - m_logBufferContentSizeInDwords);
            }
            m_rptr = wptr - (m_logBufferContentSizeInDwords);
        }

        LOG_VERBOSE << "  wptr = " << wptr << ", rptr = " << m_rptr << endl;

        for (; wptr > m_rptr && (wptr != m_lastWptr); m_rptr++)
        {
            union log_event *evt = &logBuffer->evt[m_rptr % m_logBufferContentSizeInDwords]; // h->evt is the log line payload.
            if (evt->hdr.signature != 5)
            {
                if (!corruptedEntryExpected)
                {
                    if (!LogReader::Tracer3ppLogsFilterOn)
                    {
                        ReportCorruptedEntry(evt->hdr.signature);
                    }
                }
                continue;
            }
            corruptedEntryExpected = false; // at this point corrupted entries are not expected, next invalid signature should be reported.
            ConsumeMessage(logBuffer->evt);
        }
        m_lastWptr = wptr;
        if (m_rptr > wptr)
        {
            //stringstream beMessage;
            LOG_WARNING << "At the end of the buffer **m_rptr > wptr**,  wptr = " << wptr << ", m_rptr = " << m_rptr << " setting m_rptr = wptr" << endl;
            m_rptr = wptr;
        }
        EndBuffer();
    }

    void BufferConsumer::StopConsumer(LoggingType loggingType)
    {
        m_chunkConsumers[loggingType]->Stop();
    }

    bool BufferConsumer::IsAnyChunkConsumerActive() const
    {
        for (const auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                return true;
            }
        }
        return false;
    }

    bool BufferConsumer::IsActive(LoggingType loggingType) const
    {
        return m_chunkConsumers[loggingType]->IsActive();
    }

    void BufferConsumer::ReportDeviceRemoved()
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ReportDeviceRemoved();
            }
        }
    }

    void BufferConsumer::ReportDeviceUp()
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ReportDeviceDiscovered();
            }
        }
    }


    void BufferConsumer::ReportAndUpdateFwVersion(const FwStateProvider* fwStateProvider, const module_level_info* moduleLevelInfoArray)
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ReportAndUpdateFwVersion(fwStateProvider, moduleLevelInfoArray);
            }
        }
    }

    void BufferConsumer::ResetRWPointers()
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ReportTracerWasRestarted();
            }
        }
        m_rptr = 0;
        m_lastWptr = 0;
    }

    void BufferConsumer::ConsumeMessage(const log_event* logBufferBase)
    {
        // We do this anyway because we know that we have at least one consumer.
        const union log_event *evtHead = &logBufferBase[m_rptr % m_logBufferContentSizeInDwords]; // h->evt is the log line payload.
        const unsigned numOfDwords = 4 * evtHead->hdr.dword_num_msb + evtHead->hdr.dword_num_lsb; // Part of the same DWORD
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ConsumeMessage(logBufferBase, m_rptr, m_logBufferContentSizeInDwords, numOfDwords);
            }
        }
        m_rptr += numOfDwords;
    }

    void BufferConsumer::StartNewBuffer(const TimeStamp& ts)
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->OnStartNewChunk(ts);
            }
        }
    }

    void BufferConsumer::ReportBufferOverrun(unsigned numMissedDwords)
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ReportBufferOverrun(numMissedDwords);
            }
        }
    }

    void BufferConsumer::ReportDeviceRestarted()
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ReportDeviceRestarted();
            }
        }
    }

    void BufferConsumer::ReportReadOverrun()
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ReportReadOverrun();
            }
        }
    }

    void BufferConsumer::ReportCorruptedEntry(unsigned signature)
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->ReportCorruptedEntry(signature);
            }
        }
    }

    void BufferConsumer::EndBuffer()
    {
        for (auto &cc : m_chunkConsumers)
        {
            if (cc->IsActive())
            {
                cc->OnEndChunk();
            }
        }
    }

}
