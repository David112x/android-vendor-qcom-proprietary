/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorStopRecordingHandler.h"
#include "Host.h"
#include "DeviceManager.h"
#include "LogCollectorService.h"
#include "PersistentConfiguration.h"

void LogCollectorStopRecordingHandler::HandleRequest(const LogCollectorStopRecordingRequest& jsonRequest, LogCollectorStopRecordingResponse& jsonResponse)
{
    std::string deviceName = jsonRequest.GetDeviceName();
    if (deviceName.empty()) // key is missing or an empty string provided
    {
        OperationStatus os = Host::GetHost().GetDeviceManager().GetDefaultDevice(deviceName);
        if (!os)
        {
            jsonResponse.Fail(os.GetStatusMessage());
            return;
        }
    }
    const JsonValueBoxed<CpuType> cpuTypeBoxed = jsonRequest.GetCpuType();
    CpuType cpuType;
    switch (cpuTypeBoxed.GetState())
    {
    case JsonValueState::JSON_VALUE_MALFORMED:
        jsonResponse.Fail("CpuType is wrong, it should be 'fw' or 'ucode' or empty");
        return;
    case JsonValueState::JSON_VALUE_MISSING:
        cpuType = CPU_TYPE_BOTH;
        break;
    default:
        // guaranty that value is provided.
        cpuType = cpuTypeBoxed;
        break;
    }

    LOG_DEBUG << "Log Collector stop recording for Device: " << deviceName << " with CPU type: " << cpuTypeBoxed << std::endl;

    OperationStatus os1 = Host::GetHost().GetLogCollectorService().StopLogging(deviceName, cpuType, XmlRecorder);
    OperationStatus os2 = Host::GetHost().GetLogCollectorService().StopLogging(deviceName, cpuType, TxtRecorder);

    OperationStatus os = OperationStatus::Merge(os1, os2);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
    jsonResponse.SetRecordingPath(Host::GetHost().GetConfiguration().LogCollectorConfiguration.TargetLocation);
}

