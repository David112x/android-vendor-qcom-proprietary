/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <sstream>

#include "LogCollectorSetVerbosityHandler.h"
#include "Host.h"
#include "OperationStatus.h"
#include "DeviceManager.h"
#include "PersistentConfiguration.h"
#include "LogCollectorService.h"


OperationStatus UpdateVerbosityInConfig(CpuType cpuType, int moduleNum, const std::string &verbosity )
{
    switch (cpuType)
    {
    case CPU_TYPE_FW:
        return Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.UpdateIthModule(moduleNum, verbosity);
    case CPU_TYPE_UCODE:
        return Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.UpdateIthModule(moduleNum, verbosity);
    default:
        return OperationStatus(false, "UpdateVerbosityInConfig fail, No such CPU type");
    }
}

void LogCollectorSetVerbosityHandler::HandleRequest(const LogCollectorSetVerbosityRequest& jsonRequest, LogCollectorSetVerbosityResponse& jsonResponse)
{
    /* 1. check device
     * 2. check cpu
     * 2.1 apply directly
     * 2.2 check persist
     * 2.2.1 apply persist
     */

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

    const JsonValueBoxed<CpuType> cpuType = jsonRequest.GetCpuType();
    if (cpuType.GetState() == JsonValueState::JSON_VALUE_MISSING)
    {
        jsonResponse.Fail("CpuType field is missing");
        return;
    }
    if (cpuType.GetState() == JsonValueState::JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("CpuType is wrong, it should be 'fw' or 'ucode'");
        return;
    }

    bool persist = false;
    const JsonValueBoxed<bool> persistBoxed = jsonRequest.GetPersist();

    if (persistBoxed.GetState() == JsonValueState::JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("Persist param has to be boolean");
        return;
    }
    if (persistBoxed.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
    {
        persist = persistBoxed.GetValue();
    }

    LOG_DEBUG << "Log Collector set verbosity request for Device: " << deviceName << " with CPU type: " << cpuType << std::endl;

    JsonValueBoxed<std::string> defaultVerbosityBoxed = jsonRequest.GetDefaultVerbosity();
    OperationStatus os;

    for (int i = 0; i < log_collector::NUM_MODULES; i++)
    {
        const JsonValueBoxed<std::string> moduleVerbosityBoxed = jsonRequest.GetModuleVerbosity(log_collector::module_names[i]);

        if (moduleVerbosityBoxed.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
        {
            os = Host::GetHost().GetLogCollectorService().SetLogCollectionModuleVerbosity(deviceName, cpuType, log_collector::module_names[i], moduleVerbosityBoxed);
            if (!os)
            {
                jsonResponse.Fail(os.GetStatusMessage());
                return;
            }
            if (persist)
            {
                os = UpdateVerbosityInConfig(cpuType, i, moduleVerbosityBoxed);
                if (!os)
                {
                    jsonResponse.Fail(os.GetStatusMessage());
                    return;
                }
            }
        }
        else if (defaultVerbosityBoxed.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
        {
            os = Host::GetHost().GetLogCollectorService().SetLogCollectionModuleVerbosity(deviceName, cpuType, log_collector::module_names[i], defaultVerbosityBoxed);
            if (!os)
            {
                jsonResponse.Fail(os.GetStatusMessage());
                return;
            }
            if (persist)
            {
                os = UpdateVerbosityInConfig(cpuType, i, defaultVerbosityBoxed);
                if (!os)
                {
                    jsonResponse.Fail(os.GetStatusMessage());
                    return;
                }
            }
        }
    }
    if (persist)
    {
        Host::GetHost().GetConfiguration().CommitChanges();
    }
}
