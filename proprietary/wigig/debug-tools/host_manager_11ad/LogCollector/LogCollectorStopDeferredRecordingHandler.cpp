/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorStopDeferredRecordingHandler.h"
#include "Host.h"
#include "DeviceManager.h"
#include "LogCollectorService.h"

void LogCollectorStopDeferredRecordingHandler::HandleRequest(const LogCollectorStopDeferredRecordingRequest& jsonRequest, LogCollectorStopDeferredRecordingResponse& jsonResponse)
{
    (void)jsonRequest;
    (void)jsonResponse;
    Host::GetHost().GetLogCollectorService().DisableDeferredLogging();
}

