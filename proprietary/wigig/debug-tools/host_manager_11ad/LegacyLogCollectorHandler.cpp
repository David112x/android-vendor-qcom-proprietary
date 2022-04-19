/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*
* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sstream>

#include "LegacyLogCollectorHandler.h"
#include "Host.h"
#include "LogCollector/LogCollectorDefinitions.h"
#include "OperationStatus.h"
#include "LogCollector/LogCollectorService.h"

const char LegacyLogCollectorHandler::LIST_DELIMITER = ',';
const char LegacyLogCollectorHandler::DEVICE_RESULT_STRUCT_OPENER = '{';
const char LegacyLogCollectorHandler::DEVICE_RESULT_STRUCT_CLOSER = '}';
const char* const LegacyLogCollectorHandler::UNKNOWN_DEVICE = "unknown_device";
const char* const LegacyLogCollectorHandler::SUCCESS = "SUCCESS";
const char* const LegacyLogCollectorHandler::TRUE_STR = "True"; // TODO: move message formatting to higher level
const char* const LegacyLogCollectorHandler::FALSE_STR = "False"; // TODO: move message formatting to higher level
const char* const LegacyLogCollectorHandler::START_COLLECTION = "start_collection";
const char* const LegacyLogCollectorHandler::STOP_COLLECTION = "stop_collection";
const char* const LegacyLogCollectorHandler::IS_COLLECTING = "is_collecting";
const char* const LegacyLogCollectorHandler::GET_CONFIGURATION = "get_configuration";
const char* const LegacyLogCollectorHandler::GET_PARAMETER = "get_parameter";
const char* const LegacyLogCollectorHandler::SET_PARAMETER = "set_parameter";

using namespace std;
using namespace log_collector;

bool LegacyLogCollectorHandler::HandleTcpCommand(const string& operation, const vector<string>& parameterList, string& statusMessage) const
{
    if (g_LogConfig.ShouldPrint(LOG_SEV_DEBUG))
    {
        stringstream ss;
        for (const auto &p : parameterList)
        {
            ss << p << "; ";
        }
        LOG_DEBUG << "Legacy log collector operation: " << operation << ", Parameters: " << ss.str() << endl;
    }

    auto it = m_handlers.find(operation);
    if (m_handlers.end() == it)
    {
        statusMessage = "No handle exists for command " + operation + ". Use help command for more information.";
        return false;
    }

    return it->second(*this, parameterList, statusMessage);
}

bool LegacyLogCollectorHandler::CheckArgumentValidity(const std::vector<std::string> arguments,
    unsigned numberOfExpectedArguments, std::string& statusMessage) const
{
    if (arguments.size() != numberOfExpectedArguments)
    {
        stringstream ss;
        ss << "Expected " << numberOfExpectedArguments << " parameters but got " << arguments.size() << endl;
        statusMessage = ss.str();
        return false;
    }
    return true;
}

bool LegacyLogCollectorHandler::StartCollection(const std::vector<std::string>& parameters, std::string& statusMessage) const
{
    // 2 parameters: device , cpu
    if (!CheckArgumentValidity(parameters, 2, statusMessage))
    {
        return false;
    }
    const string& deviceName = parameters[0];
    bool isPublishing = false;
    OperationStatus os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(
        deviceName, CPU_TYPE_FW, isPublishing, RawPublisher);
    if (!os)
    {
        statusMessage = os.GetStatusMessage();
        return false;
    }
    if (!isPublishing)
    {
        os = Host::GetHost().GetLogCollectorService().StartLogging(deviceName, CPU_TYPE_FW, RawPublisher);
        if (!os)
        {
            statusMessage = os.GetStatusMessage();
            return false;
        }
    }
    else
    {
        statusMessage = "Fw logs are already being collected; ";
        LOG_INFO << statusMessage << endl;
    }

    os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(
        deviceName, CPU_TYPE_UCODE, isPublishing, RawPublisher);
    if (!os)
    {
        statusMessage = os.GetStatusMessage();
        return false;
    }
    if (!isPublishing)
    {
        os = Host::GetHost().GetLogCollectorService().StartLogging(deviceName, CPU_TYPE_UCODE, RawPublisher);
        if (!os)
        {
            statusMessage = os.GetStatusMessage();
            return false;
        }
    }
    else
    {
        statusMessage += "uCode logs are already being collected;";
        LOG_INFO << statusMessage << endl;
    }

    return true;
}

bool LegacyLogCollectorHandler::StopCollection(const std::vector<std::string>& parameters, std::string& statusMessage) const
{
    // 2 parameters: device list, cpu list
    if (!CheckArgumentValidity(parameters, 2, statusMessage))
    {
        return false;
    }
    const string& deviceName = parameters[0];
    bool isPublishing = false;
    OperationStatus os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(
        deviceName, CPU_TYPE_FW, isPublishing, RawPublisher);
    if (!os)
    {
        statusMessage = os.GetStatusMessage();
        return false;
    }
    if (isPublishing)
    {
        os = Host::GetHost().GetLogCollectorService().StopLogging(deviceName, CPU_TYPE_FW, RawPublisher);
        if (!os)
        {
            statusMessage = os.GetStatusMessage();
            return false;
        }
    }
    else
    {
        statusMessage = "Fw logs are not being collected; ";
        LOG_INFO << statusMessage << endl;
    }

    os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(
        deviceName, CPU_TYPE_UCODE, isPublishing,RawPublisher);
    if (!os)
    {
        statusMessage = os.GetStatusMessage();
        return false;
    }
    if (isPublishing)
    {
        os = Host::GetHost().GetLogCollectorService().StopLogging(deviceName, CPU_TYPE_UCODE, RawPublisher);
        if (!os)
        {
            statusMessage = os.GetStatusMessage();
            return false;
        }
    }
    else
    {
        statusMessage += "uCode logs are not being collected;";
        LOG_INFO << statusMessage << endl;
    }
    return true;

}

bool LegacyLogCollectorHandler::IsCollecting(const std::vector<std::string>& parameters, std::string& statusMessage) const
{
    // 2 parameters: device list, cpu list
    if (!CheckArgumentValidity(parameters, 2, statusMessage))
    {
        return false;
    }
    const string& deviceName = parameters[0];
    bool isPublishingFw = false;
    OperationStatus os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(
        deviceName, CPU_TYPE_FW, isPublishingFw,RawPublisher);
    if (!os)
    {
        statusMessage = os.GetStatusMessage();
        return false;
    }

    bool isPublishinguCode = false;
    os = Host::GetHost().GetLogCollectorService().GetLogLoggingState(
        deviceName, CPU_TYPE_UCODE, isPublishinguCode, RawPublisher);
    if (!os)
    {
        statusMessage = os.GetStatusMessage();
        return false;
    }


    statusMessage = (isPublishingFw || isPublishinguCode) ? TRUE_STR : FALSE_STR;
    return true;
}


bool LegacyLogCollectorHandler::GetConfiguration(const std::vector<std::string>& parameters, std::string& statusMessage) const
{
    // 2 parameters: device list, cpu list
    if (!CheckArgumentValidity(parameters, 2, statusMessage))
    {
        return false;
    }
    // currently parameters[0] must be only one device and parameter[1] must be one cpu
    statusMessage = Host::GetHost().GetLogCollectorService().GetLogCollectorConfigurationString(parameters[0], parameters[1]);
    return true;
}

bool LegacyLogCollectorHandler::GetParameter(const std::vector<std::string>& parameters, std::string& statusMessage) const
{
    // 3 parameters: device list, cpu list, parameter to be retrieved
    if (!CheckArgumentValidity(parameters, 3, statusMessage))
    {
        return false;
    }

    // currently parameters[0] must be only one device and parameter[1] must be one cpu
    statusMessage = Host::GetHost().GetLogCollectorService().GetLogCollectionParameter(parameters[0], parameters[1], parameters[2]);
    return true;
}

bool LegacyLogCollectorHandler::SetParameter(const std::vector<std::string>& parameters, std::string& statusMessage) const
{
    // 3 parameters: device list, cpu list, parameter to be set and its new value (e.g. polling_interval_ms=300)
    if (!CheckArgumentValidity(parameters, 3, statusMessage))
    {
        return false;
    }

    // currently parameters[0] must be only one device and parameter[1] must be one cpu
    return Host::GetHost().GetLogCollectorService().SetLogCollectionParameter(parameters[0], parameters[1], parameters[2], statusMessage);
}
