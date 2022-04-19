/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorDeferredRecordingHandler.h"
#include "Host.h"
#include "DeviceManager.h"
#include "LogCollectorService.h"

void LogCollectorDeferredRecordingHandler::HandleRequest(const LogCollectorStartDeferredRecordingRequest& jsonRequest, LogCollectorStartDeferredRecordingResponse& jsonResponse)
{
    const JsonValueBoxed<log_collector::RecordingType> RecordingTypeBoxed = jsonRequest.GetRecordingTypeBoxed();
    log_collector::RecordingType recordingType;
    switch (RecordingTypeBoxed.GetState())
    {
    case JsonValueState::JSON_VALUE_MALFORMED:
        jsonResponse.Fail("RecordingType is wrong, it should be 'raw' or 'txt' or empty (default is raw)");
        return;
    case JsonValueState::JSON_VALUE_MISSING:
        recordingType = log_collector::RECORDING_TYPE_XML;
        break;
    default:
        // guaranty that value is provided.
        recordingType = RecordingTypeBoxed;
        break;
    }

    LOG_DEBUG << "Log Collector start deferred recording"
        << " recording type is: " << recordingType << std::endl;
    OperationStatus os;
    if(recordingType == log_collector::RECORDING_TYPE_XML || recordingType == log_collector::RECORDING_TYPE_BOTH)
    {
       os  = Host::GetHost().GetLogCollectorService().EnableDeferredLogging(log_collector::XmlRecorder);
    }
    OperationStatus os2;
    if (recordingType == log_collector::RECORDING_TYPE_TXT || recordingType == log_collector::RECORDING_TYPE_BOTH)
    {
        os2 = Host::GetHost().GetLogCollectorService().EnableDeferredLogging(log_collector::TxtRecorder);
    }
    os = OperationStatus::Merge(os, os2);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
    }
}

