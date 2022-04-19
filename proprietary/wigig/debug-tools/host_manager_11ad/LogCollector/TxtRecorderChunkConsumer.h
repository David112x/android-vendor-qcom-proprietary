/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#pragma once

#include "OperationStatus.h"
#include "LogRecorder.h"
#include "ChunkConsumer.h"
#include "LogTxtParser.h"

namespace log_collector
{

    class TxtRecorderChunkConsumer final : public ChunkConsumer
    {
    public:
        TxtRecorderChunkConsumer(const std::string &deviceName, CpuType tracerType);
        ~TxtRecorderChunkConsumer();

        void OnStartNewChunk(const TimeStamp& ts) override;
        void ReportBufferOverrun(unsigned numMissedDwords) override;
        void ReportDeviceRestarted() override;
        void ReportReadOverrun() override;
        void ReportCorruptedEntry(unsigned signature) override;

        void ReportDeviceRemoved() override;
        void ReportDeviceDiscovered() override;
        void ReportTracerWasRestarted() override;
        void ReportAndUpdateFwVersion(const FwStateProvider* fwStateProvider, const module_level_info* moduleLevelInfoArray) override;
        void ConsumeMessage(const log_event* logEvent, unsigned rptr, unsigned bufferSize, unsigned numOfDwords) override;

        void OnEndChunk() override;

        // recording methods
        void Stop() override; // close files
        //virtual OperationStatus SplitRecordings() = 0; // close current file and start a new one.
        OperationStatus Activate(const module_level_info* moduleLevelInfoArray, const FwStateProvider* fwStateProvider) override;

    private:
        LogRecorder m_logCollectorRecorder;
        LogTxtParser m_logTxtParser;
        std::string m_timeStampStr;
        void SetLogFileHeader(const module_level_info* moduleLevelInfoArray, const FwStateProvider* fwStateProvider);
    };

}
