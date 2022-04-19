/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <memory>
#include <array>
#include <string>
#include "OperationStatus.h"
#include "Utils.h"
#include "LogCollectorDefinitions.h"

class Device;
class FwStateProvider;

namespace log_collector
{
    class ChunkConsumer;

    class BufferConsumer
    {
    public:
        BufferConsumer(const std::string &deviceName, CpuType tracerType,
             unsigned logBufferContentSizeInDwords);
        ~BufferConsumer();
        BufferConsumer(const BufferConsumer & lcc) = delete;
        BufferConsumer & operator=(const BufferConsumer & lcc) = delete;

        bool Reset(const module_level_info* moduleLevelInfoArray, unsigned logBufferContentSizeInDwords, const FwStateProvider* fwStateProvider);
        void UpdateBufferSize(unsigned logBufferContentSizeInDwords) { m_logBufferContentSizeInDwords = logBufferContentSizeInDwords; };

        OperationStatus ActivateChunkConsumer(LoggingType loggingType, const module_level_info* moduleLevelInfoArray, const FwStateProvider* fwStateProvider);
        void SetActiveFlag(LoggingType loggingType);

        void ConsumeBuffer(log_buffer* logBuffer);

        void StopConsumer(LoggingType loggingType); //Stopping only ChunkConsumers of type: loggingType

        bool IsAnyChunkConsumerActive() const;
        bool IsActive(LoggingType loggingType) const;

        void ReportDeviceRemoved();
        void ReportDeviceUp();

        // This method is used as a callback during poll if fw state changed to Ready.
        void ReportAndUpdateFwVersion(const FwStateProvider* fwStateProvider, const module_level_info* moduleLevelInfoArray);

        void ResetRWPointers(); //Reset Read pointers and last write pointer

    private:


        void ConsumeMessage(const log_event* logBufferBase);
        void StartNewBuffer(const TimeStamp& ts);
        void ReportBufferOverrun(unsigned numMissedDwords);
        void ReportDeviceRestarted();
        void ReportReadOverrun();
        void ReportCorruptedEntry(unsigned signature);
        void EndBuffer();

        std::array<std::unique_ptr<ChunkConsumer>, MaxConsumers> m_chunkConsumers;
        std::string m_deviceName;
        unsigned m_rptr; // last read address -
        unsigned m_lastWptr; // last write ptr address (used for detecting buffer overflow)
        unsigned m_logBufferContentSizeInDwords;
        CpuType m_cpuType;
    };

}