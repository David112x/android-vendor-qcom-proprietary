/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#pragma once


#include <vector>
#include "OperationStatus.h"
#include "ChunkConsumer.h"
#include "RawLogLine.h"




namespace log_collector
{
    class RawPublisherChunkConsumer final : public ChunkConsumer
    {
    public:
        RawPublisherChunkConsumer(const std::string &deviceName, CpuType tracerType);
        ~RawPublisherChunkConsumer();

        // API methods.
        void OnStartNewChunk(const TimeStamp& ts) override;
        void ReportBufferOverrun(unsigned numMissedDwords) override;
        void ReportDeviceRestarted() override;
        void ReportReadOverrun() override;
        void ReportCorruptedEntry(unsigned signature) override;
        void ReportDeviceRemoved()override {};
        void ReportDeviceDiscovered() override {};
        void ReportTracerWasRestarted() override {};
        void ReportAndUpdateFwVersion(const FwStateProvider* fwStateProvider, const module_level_info* moduleLevelInfoArray) override;
        void ConsumeMessage(const log_event* logEvent, unsigned rptr, unsigned bufferSize, unsigned numOfDwords) override;
        void OnEndChunk() override;

        // recording methods
        void Stop() override; // close files
        //virtual OperationStatus SplitRecordings() = 0; // close current file and start a new one.
        OperationStatus Activate(const module_level_info* moduleLevelInfoArray, const FwStateProvider* fwStateProvider) override;

    private:

        std::vector<RawLogLine> m_rawLogLinesForEvent;
        void PublishLogs() const;
    };

}
