/*
* Copyright (c)  2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorGetStateHandler.h"
#include "LogReader.h"
#include "Host.h"
#include "DeviceManager.h"
#include "LogCollectorService.h"

void LogCollectorGetStateHandler::HandleRequest(const LogCollectorGetStateRequest& jsonRequest, LogCollectorGetStateResponse& jsonResponse)
{
    std::string deviceName = jsonRequest.GetDeviceName();
    if (deviceName.empty()) // key is missing or an empty string provided
    {
        const OperationStatus os = Host::GetHost().GetDeviceManager().GetDefaultDevice(deviceName);
        if (!os)
        {
            jsonResponse.Fail(os.GetStatusMessage());
            return;
        }
    }

    bool isRecordingRaw = false;
    bool isRecordingTxt = false;
    bool isPublishing = false;

    OperationStatus os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(deviceName, CPU_TYPE_FW, isRecordingRaw, XmlRecorder);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
    os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(deviceName, CPU_TYPE_FW, isRecordingTxt, TxtRecorder);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
    jsonResponse.SetIsRecording(isRecordingRaw || isRecordingTxt, CPU_TYPE_FW);

    os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(deviceName, CPU_TYPE_FW, isPublishing, RawPublisher);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
    jsonResponse.SetIsPublishing(isPublishing, CPU_TYPE_FW);

    os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(deviceName, CPU_TYPE_UCODE, isRecordingRaw, XmlRecorder);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
    os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(deviceName, CPU_TYPE_UCODE, isRecordingTxt, TxtRecorder);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
    jsonResponse.SetIsRecording(isRecordingRaw || isRecordingTxt, CPU_TYPE_UCODE);

    os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(deviceName, CPU_TYPE_UCODE, isPublishing, RawPublisher);
    if (!os)
    {
        jsonResponse.Fail(os.GetStatusMessage());
        return;
    }
    jsonResponse.SetIsPublishing(isPublishing, CPU_TYPE_UCODE);

    jsonResponse.SetIsDeferredRecordingActive(Host::GetHost().GetLogCollectorService().GetDeferredLoggingState());

}

