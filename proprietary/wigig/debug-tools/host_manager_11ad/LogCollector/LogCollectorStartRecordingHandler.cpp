/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <string>
#include "LogCollectorStartRecordingHandler.h"
#include "LogCollectorDefinitions.h"
#include "Host.h"
#include "DeviceManager.h"
#include "LogCollectorService.h"
#include "PersistentConfiguration.h"

using namespace std;

namespace
{
    bool CheckCpuType(const JsonValueBoxed<CpuType> &cpuTypeBoxed, CpuType cpuType)
    {
        return cpuTypeBoxed.GetState() == JsonValueState::JSON_VALUE_MISSING || cpuTypeBoxed == cpuType;
    }
}


void LogCollectorStartRecordingHandler::HandleRequest(const LogCollectorStartRecordingRequest& jsonRequest, LogCollectorStartRecordingResponse& jsonResponse)
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
    if (cpuTypeBoxed.GetState() == JsonValueState::JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("CpuType is wrong, it should be 'fw' or 'ucode' or empty (for both cpus).");
        return;
    }
    OperationStatus os;
    const JsonValueBoxed<string> RecordingTypeBoxed = jsonRequest.GetRecordingTypeBoxed();

    //should not happen if working with shell_11ad but for completeness:
    if (RecordingTypeBoxed.GetState() == JsonValueState::JSON_VALUE_PROVIDED &&
        (RecordingTypeBoxed.GetValue() != "txt" && RecordingTypeBoxed.GetValue() != "raw" && RecordingTypeBoxed.GetValue() != "both"))
    {
        jsonResponse.Fail("Invalid recording type provided: " + RecordingTypeBoxed.GetValue() + " Recording type should be txt/raw");
        return;
    }


    LOG_DEBUG << "Log Collector start recording for Device: " << deviceName
        << " with CPU type: " << cpuTypeBoxed
        << " recording type is: " << RecordingTypeBoxed << std::endl;

    if (RecordingTypeBoxed.GetState() == JsonValueState::JSON_VALUE_MISSING ||
        (RecordingTypeBoxed.GetValue() == "raw" || RecordingTypeBoxed.GetValue() == "both"))
    {
        if (CheckCpuType(cpuTypeBoxed, CPU_TYPE_FW))
        {
            os = Host::GetHost().GetLogCollectorService().StartLogging(deviceName, CPU_TYPE_FW, XmlRecorder);
            if (!os)
            {
                jsonResponse.Fail(os.GetStatusMessage());
                return;
            }
        }
        if (CheckCpuType(cpuTypeBoxed, CPU_TYPE_UCODE))
        {
            os = Host::GetHost().GetLogCollectorService().StartLogging(deviceName, CPU_TYPE_UCODE, XmlRecorder);
            if (!os)
            {
                jsonResponse.Fail(os.GetStatusMessage());
                return;
            }
        }
    }

    if (RecordingTypeBoxed.GetState() == JsonValueState::JSON_VALUE_PROVIDED &&
        (RecordingTypeBoxed.GetValue() == "txt" || RecordingTypeBoxed.GetValue() == "both"))
    {
        if (CheckCpuType(cpuTypeBoxed, CPU_TYPE_FW))
        {
            os = Host::GetHost().GetLogCollectorService().StartLogging(deviceName, CPU_TYPE_FW, TxtRecorder);
            if (!os)
            {
                jsonResponse.Fail(os.GetStatusMessage());
                return;
            }
        }
        if (CheckCpuType(cpuTypeBoxed, CPU_TYPE_UCODE))
        {
            os = Host::GetHost().GetLogCollectorService().StartLogging(deviceName, CPU_TYPE_UCODE, TxtRecorder);
            if (!os)
            {
                jsonResponse.Fail(os.GetStatusMessage());
                return;
            }
        }
    }

    jsonResponse.SetRecordingPath(Host::GetHost().GetConfiguration().LogCollectorConfiguration.TargetLocation);
}

