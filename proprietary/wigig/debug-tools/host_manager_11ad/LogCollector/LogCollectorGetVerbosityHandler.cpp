/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "LogCollectorGetVerbosityHandler.h"
#include "LogReader.h"
#include "Host.h"
#include "DeviceManager.h"
#include "PersistentConfiguration.h"
#include "LogCollectorService.h"

void LogCollectorGetVerbosityHandler::HandleRequest(const LogCollectorGetVerbosityRequest& jsonRequest, LogCollectorGetVerbosityResponse& jsonResponse)
{


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

    LOG_DEBUG << "Log Collector get verbosity from configuration for CPU type: " << cpuType << std::endl;


    // Get the verbosity from the persistent configuration
    const JsonValueBoxed<bool> fromDevice = jsonRequest.GetFromDevice();
    if (cpuType.GetState() == JsonValueState::JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("CpuType is wrong, it should be 'fw' or 'ucode'");
        return;
    }

    if (fromDevice.GetState() == JsonValueState::JSON_VALUE_MISSING || !fromDevice) //If missing we take verbosity from config (by default.)
    {
        if (cpuType == CPU_TYPE_FW)
        {
            jsonResponse.SetModuleVerbosity("Module0", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module0);
            jsonResponse.SetModuleVerbosity("Module1", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module1);
            jsonResponse.SetModuleVerbosity("Module2", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module2);
            jsonResponse.SetModuleVerbosity("Module3", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module3);
            jsonResponse.SetModuleVerbosity("Module4", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module4);
            jsonResponse.SetModuleVerbosity("Module5", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module5);
            jsonResponse.SetModuleVerbosity("Module6", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module6);
            jsonResponse.SetModuleVerbosity("Module7", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module7);
            jsonResponse.SetModuleVerbosity("Module8", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module8);
            jsonResponse.SetModuleVerbosity("Module9", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module9);
            jsonResponse.SetModuleVerbosity("Module10", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module10);
            jsonResponse.SetModuleVerbosity("Module11", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module11);
            jsonResponse.SetModuleVerbosity("Module12", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module12);
            jsonResponse.SetModuleVerbosity("Module13", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module13);
            jsonResponse.SetModuleVerbosity("Module14", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module14);
            jsonResponse.SetModuleVerbosity("Module15", Host::GetHost().GetConfiguration().FwModuleVerbosityConfiguration.Module15);
        }
        else
        {
            jsonResponse.SetModuleVerbosity("Module0", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module0);
            jsonResponse.SetModuleVerbosity("Module1", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module1);
            jsonResponse.SetModuleVerbosity("Module2", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module2);
            jsonResponse.SetModuleVerbosity("Module3", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module3);
            jsonResponse.SetModuleVerbosity("Module4", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module4);
            jsonResponse.SetModuleVerbosity("Module5", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module5);
            jsonResponse.SetModuleVerbosity("Module6", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module6);
            jsonResponse.SetModuleVerbosity("Module7", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module7);
            jsonResponse.SetModuleVerbosity("Module8", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module8);
            jsonResponse.SetModuleVerbosity("Module9", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module9);
            jsonResponse.SetModuleVerbosity("Module10", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module10);
            jsonResponse.SetModuleVerbosity("Module11", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module11);
            jsonResponse.SetModuleVerbosity("Module12", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module12);
            jsonResponse.SetModuleVerbosity("Module13", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module13);
            jsonResponse.SetModuleVerbosity("Module14", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module14);
            jsonResponse.SetModuleVerbosity("Module15", Host::GetHost().GetConfiguration().uCodeModuleVerbosityConfiguration.Module15);
        }
        return;
    }

    // Try get verbosity from device!
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

    std::vector<std::pair<std::string, std::string> > verbosityData;

    if (!Host::GetHost().GetLogCollectorService().GetModuleVerbosityMap(deviceName, cpuType, verbosityData))
    {
        jsonResponse.Fail("Failed to get module level info from device: " + deviceName);
        return;
    }
    for(const auto& kv: verbosityData)
    {
        jsonResponse.SetModuleVerbosity(kv.first, kv.second);
    }
}

