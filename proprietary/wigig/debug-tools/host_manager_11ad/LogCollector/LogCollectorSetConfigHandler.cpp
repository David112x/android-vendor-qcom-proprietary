/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <sstream>

#include "LogCollectorSetConfigHandler.h"
#include "Host.h"
#include "PersistentConfiguration.h"
#include "DeviceManager.h"
#include "LogCollectorService.h"

using namespace std;

void LogCollectorSetConfigHandler::HandleRequest(const LogCollectorSetConfigRequest& jsonRequest, LogCollectorSetConfigResponse& jsonResponse)
{
    // This flow only update the persistent configuration and does not affect existing recording
    LOG_DEBUG << "Log Collector set configuration request " << std::endl;

    // check if any recording exist
    if (Host::GetHost().GetLogCollectorService().IsLogging())
    {
        jsonResponse.Fail("Set configuration is not supported when log collection is active. Please stop all log collection. ");
        return;
    }

    const JsonValueBoxed<uint32_t> getPollingIntervalMs = jsonRequest.GetPollingIntervalMs();
    if (getPollingIntervalMs.GetState() == JsonValueState::JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("'polling_interval_ms' field is malformed");
        return;
    }
    if (getPollingIntervalMs.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
    {
        LOG_DEBUG << "setting polling interval to: " << getPollingIntervalMs << endl;
        Host::GetHost().GetConfiguration().LogCollectorConfiguration.PollingIntervalMs = getPollingIntervalMs;
    }

    const JsonValueBoxed<uint32_t> getMaxSingleFileSizeMB = jsonRequest.GetMaxSingleFileSizeMb();
    if (getMaxSingleFileSizeMB.GetState() == JsonValueState::JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("'max_single_file_size_mb' field is malformed");
        return;
    }
    if (getMaxSingleFileSizeMB.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
    {
        Host::GetHost().GetConfiguration().LogCollectorConfiguration.MaxLogFileSize = getMaxSingleFileSizeMB;
    }

    const JsonValueBoxed<uint32_t> getMaxNumOfLogFiles = jsonRequest.GetMaxNumOfLogFiles();
    if (getMaxNumOfLogFiles.GetState() == JsonValueState::JSON_VALUE_MALFORMED)
    {
        jsonResponse.Fail("'max_number_of_log_files' field is malformed");
        return;
    }
    if (getMaxNumOfLogFiles.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
    {
        Host::GetHost().GetConfiguration().LogCollectorConfiguration.MaxNumOfLogFiles = getMaxNumOfLogFiles;
    }

    const JsonValueBoxed<std::string>& getConversionFilePath = jsonRequest.GetConversionFilePath();
    if (getConversionFilePath.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
    {
        LOG_DEBUG << "setting conversion path to: " << getConversionFilePath << endl;
        Host::GetHost().GetConfiguration().LogCollectorConfiguration.ConversionFileLocation = getConversionFilePath;
    }

    const JsonValueBoxed<std::string>& getLogFilesDirectoryPath = jsonRequest.GetLogFilesDirectoryPath();
    if (getLogFilesDirectoryPath.GetState() == JsonValueState::JSON_VALUE_PROVIDED)
    {
        LOG_DEBUG << "setting log output path to: " << getLogFilesDirectoryPath << endl;
        Host::GetHost().GetConfiguration().LogCollectorConfiguration.TargetLocation = getLogFilesDirectoryPath;
    }

    Host::GetHost().GetConfiguration().CommitChanges();

}

void LogCollectorReSetConfigHandler::HandleRequest(const LogCollectorReSetConfigRequest & jsonRequest, LogCollectorReSetConfigResponse & jsonResponse)
{
    (void)jsonRequest;
    (void)jsonResponse;
    Host::GetHost().GetConfiguration().ResetConfig();
    Host::GetHost().GetConfiguration().CommitChanges();
}
