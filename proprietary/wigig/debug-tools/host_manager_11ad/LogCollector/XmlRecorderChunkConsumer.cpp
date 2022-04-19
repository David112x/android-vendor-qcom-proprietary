/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "OperationStatus.h"
#include "Host.h"
#include "PersistentConfiguration.h"
#include "Device.h"
#include "FwStateProvider.h"
#include "XmlRecorderChunkConsumer.h"


using namespace std;

namespace log_collector
{

    std::ostream& operator<<(std::ostream& os, const RawLogObject& rawLogObject)
    {
        os << rawLogObject.m_module << "," << levels[rawLogObject.m_verbosityLevel] << "," << rawLogObject.m_stringOffset << ":";
        unsigned i;
        for (i = 0; i < rawLogObject.m_dwordNum; i++)
        {
            os << rawLogObject.m_dwords [i];
            if (i < rawLogObject.m_dwordNum - 1)
            {
                os << ",";
            }
        }

        // for backward compatibility - to enable parsing logs collected by new host_manager by old DmTools.
        // put '0' to pad up to 3 '0's (dwords)
        for (; i < 3; i++)
        {
            if (i > 0)
            {
                os << ",";
            }
            os << 0;
        }
        os << endl;
        return os;
    }



    RawLogObject::RawLogObject(unsigned module, unsigned verbosityLevel, unsigned stringOffset,
        unsigned dwordNum): m_module(module), m_verbosityLevel (verbosityLevel),
                                m_dwordNum(dwordNum),m_stringOffset (stringOffset)
    {
    }

    RawLogObject::operator basic_string<char>() const
    {
        stringstream ss;
        ss << *this;
        return ss.str();
    }


    XmlRecorderChunkConsumer::XmlRecorderChunkConsumer(const std::string& deviceName, CpuType tracerType)
        : ChunkConsumer(deviceName, tracerType)
        , m_logCollectorRecorder(deviceName, tracerType, RECORDING_TYPE_XML)
    {
    }

    XmlRecorderChunkConsumer::~XmlRecorderChunkConsumer()
    {
        Stop();
    }

    void XmlRecorderChunkConsumer::OnStartNewChunk(const TimeStamp& ts)
    {
        const OperationStatus os = m_logCollectorRecorder.HandleRecording();
        if(!os)
        {
            m_isActive = false;
            return ;
        }
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(Utils::GetCurrentLocalTimeXml(ts) + "<Content>");
    }

    void XmlRecorderChunkConsumer::ReportBufferOverrun(unsigned numMissedDwords)
    {
        stringstream missedDwords;
        missedDwords << BufferOverrunMessage << numMissedDwords << endl;
        if (!LogReader::Tracer3ppLogsFilterOn)
        {
            m_isActive =  m_logCollectorRecorder.WriteToOutputFile(missedDwords.str()); // DWORD == uint32
        }
    }

    void XmlRecorderChunkConsumer::ReportDeviceRestarted()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(DeviceRestartedMessage);
    }

    void XmlRecorderChunkConsumer::ReportReadOverrun()
    {
         m_logCollectorRecorder.WriteToOutputFile(ReadOverrunMessage);
         m_isActive = m_logCollectorRecorder.WriteToOutputFile("</Content></Log_Content>");
    }

    void XmlRecorderChunkConsumer::ReportCorruptedEntry(unsigned signature)
    {
        stringstream ss;
        ss << "**** Got corrupted entry, signature: " << signature << endl;
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(ss.str());
    }

    void XmlRecorderChunkConsumer::ReportDeviceRemoved()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(DeviceRemovedMessage);
        m_isActive = m_isActive && m_logCollectorRecorder.WriteToOutputFile("</Logs></LogFile>");

        m_logCollectorRecorder.CloseOutputFile();

    }

    void XmlRecorderChunkConsumer::ReportDeviceDiscovered()
    {
         m_isActive = m_logCollectorRecorder.WriteToOutputFile(DeviceDiscoveredMessage);
    }

    void XmlRecorderChunkConsumer::ReportTracerWasRestarted()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile(TracerWasRestartedMessage);
    }

    void XmlRecorderChunkConsumer::ReportAndUpdateFwVersion(const FwStateProvider* fwStateProvider,
        const module_level_info* moduleLevelInfoArray)
    {
        SetLogFileHeader(moduleLevelInfoArray, fwStateProvider);
        m_logCollectorRecorder.WriteToOutputFile(CreateFwVerTime(fwStateProvider));
    }

    void XmlRecorderChunkConsumer::ConsumeMessage(const log_event* logEvent, unsigned rptr, unsigned bufferSize, unsigned numOfDwords)
    {
        const union log_event *evtHead = &logEvent[rptr % bufferSize]; // h->evt is the log line payload.
        const unsigned stringOffset = evtHead->hdr.string_offset << 2;

        RawLogObject newRawLogObject(evtHead->hdr.module, evtHead->hdr.level, stringOffset, numOfDwords);
        if (numOfDwords > (MaxParams * 2))
        {
            stringstream ss;
            ss << "**** invalid number of parameters provided number is: " << numOfDwords << " max val is: " << (MaxParams * 2) << endl;
            m_isActive = m_logCollectorRecorder.WriteToOutputFile(ss.str());
            return;
        }

        newRawLogObject.m_dwords.reserve(numOfDwords);// parameters array (each log line can have at most three parameters). size need to be set to 3 due to log collector's old output format (each line has three params where dummy params has the value of 0)
        for (unsigned i = 0; i < numOfDwords; i++)
        {
            newRawLogObject.m_dwords.push_back(logEvent[(rptr + i + 1) % bufferSize].dword);
        }

        m_isActive = m_logCollectorRecorder.WriteToOutputFile(newRawLogObject);
    }

    void XmlRecorderChunkConsumer::OnEndChunk()
    {
        m_isActive = m_logCollectorRecorder.WriteToOutputFile("</Content></Log_Content>");
    }

    void XmlRecorderChunkConsumer::Stop()
    {
        if (m_isActive)
        {
            m_logCollectorRecorder.WriteToOutputFile("</Logs></LogFile>");
        }
        m_isActive = false;
        m_logCollectorRecorder.CloseOutputFile();
    }

    OperationStatus XmlRecorderChunkConsumer::Activate(const module_level_info* moduleLevelInfoArray,
                                                       const FwStateProvider* fwStateProvider)
    {
        SetLogFileHeader(moduleLevelInfoArray, fwStateProvider);
        OperationStatus os = m_logCollectorRecorder.SetLogFilesFolder(Host::GetHost().GetConfiguration().LogCollectorConfiguration.TargetLocation);
        if (!os)
        {

            m_isActive = false;
            return os;
        }

        os = m_logCollectorRecorder.PrepareRecording();
        m_isActive = os.IsSuccess();
        return os;
    }

    void XmlRecorderChunkConsumer::SetLogFileHeader(const module_level_info* moduleLevelInfoArray,
                                                    const FwStateProvider* fwStateProvider)
    {
        std::ostringstream headerBuilder;
        headerBuilder << "<LogFile>"
            << CreateFwVerTime(fwStateProvider)
            << "<Third_Party_Flags>";
        // make a list of the Third_party_flag for each module.
        for (int i = 0; i < NUM_MODULES; i++)
        {
            headerBuilder << "<Flag><value>" << static_cast<int>(moduleLevelInfoArray[i].third_party_mode) << "</value></Flag>";
        }
        headerBuilder << "</Third_Party_Flags>"
            << "<Logs>";

        m_logCollectorRecorder.SetLogFilesHeader(headerBuilder.str());
    }


    const std::string XmlRecorderChunkConsumer::CreateFwVerTime(const FwStateProvider* fwStateProvider)
    {
        std::ostringstream headerBuilder;
        const auto fwVersion = fwStateProvider->GetFwVersion();
        const auto fwTs = fwStateProvider->GetFwTimestamp();
        const auto uCodeTs = fwStateProvider->GetUcodeTimestamp();
        headerBuilder << "<FW_Ver>"
            << fwVersion
            << "</FW_Ver>"
            << "<FwCompilation_Time>"
            << fwTs
            << "</FwCompilation_Time>"
            << "<uCodeCompilation_Time>"
            << uCodeTs
            << "</uCodeCompilation_Time>";
        return headerBuilder.str();

    }
}
