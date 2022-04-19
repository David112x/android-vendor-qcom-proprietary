/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <locale> // for std::tolower
#include <string>

#include "LogCollectorService.h"
#include "Device.h"
#include "Host.h"
#include "PersistentConfiguration.h"
#include "DebugLogger.h"
#include "DeviceManager.h"
#include "FileSystemOsAbstraction.h"
#include "DeviceLoggingContext.h"

using namespace std;
using namespace log_collector;


LogCollectorService::~LogCollectorService() = default;

void LogCollectorService::OnDeviceDiscovered(const std::string& deviceName)
{
    /* 1. check if in map
     * 1.1 if not in map:
     * 1.1.1 add to map.
     * 1.1.2 start deferred collectors
     * 1.2 if in map:
     * 1.2.1 activate previous recordings
     * 1.2.2 add deferred recordings if needed
     * 1.2.3 report in new log file
     */
    OperationStatus os;
    auto deviceContext = m_DeviceLoggingContextMap.find(deviceName);
    if (deviceContext == m_DeviceLoggingContextMap.end())
    {
        LOG_DEBUG << "in OnDeviceDiscovered no context for: " << deviceName << endl;
        std::unique_ptr<DeviceLoggingContext> newDeviceLoggingContext(new DeviceLoggingContext(deviceName));
        if (m_deferredRecordingEnabled) // if deferred recording is active
        {

            os = newDeviceLoggingContext->GetFwLogCollector()->ActivateLogging(m_deferredLoggingType);
            if (!os)
            {
                LOG_ERROR << os.GetStatusMessage() << endl;
            }

            os = newDeviceLoggingContext->GetuCodeLogCollector()->ActivateLogging(m_deferredLoggingType);
            if (!os)
            {
                LOG_ERROR << os.GetStatusMessage() << endl;
            }
        }
        LOG_DEBUG << "in OnDeviceDiscovered insert new context: " << deviceName << endl;
        m_DeviceLoggingContextMap.insert(std::make_pair( deviceName, std::move(newDeviceLoggingContext)));

        return;
    }

    // device was rediscovered
    deviceContext->second->RestoreLoggingState();

    if (m_deferredRecordingEnabled) // if deferred recording is active
    {
        if (! (deviceContext->second->GetFwLogCollector()->IsLoggingValid(XmlRecorder) || deviceContext->second->GetFwLogCollector()->IsLoggingValid(TxtRecorder)))
        {
            // if no other recording exist
            os = deviceContext->second->GetFwLogCollector()->ActivateLogging(m_deferredLoggingType);
            if (!os)
            {
                LOG_ERROR << os.GetStatusMessage() << endl;
            }
        }
        if (!(deviceContext->second->GetuCodeLogCollector()->IsLoggingValid(XmlRecorder) || deviceContext->second->GetuCodeLogCollector()->IsLoggingValid(TxtRecorder)))
        {
            // if no other recording exist
            os = deviceContext->second->GetuCodeLogCollector()->ActivateLogging(m_deferredLoggingType);
            if (!os)
            {
                LOG_ERROR << os.GetStatusMessage() << endl;
            }
        }
    }
}


void LogCollectorService::OnDeviceRemoved(const std::string& deviceName)
{
    /* 1. if not in map report warning and return true!
     * 2. if in map:
     * 2.1 pause all recordings (consider stop all pollers)
     * 2.2 report to end of current log
     *
     */
    auto deviceContext = m_DeviceLoggingContextMap.find(deviceName);
    if (deviceContext == m_DeviceLoggingContextMap.end())
    {
        LOG_ERROR << "The device: " << deviceName << " was removed form device manager but was not found on LogCollectorService" << endl;
        return;
    }

    deviceContext->second->UnregisterPollerAndReportDeviceRemoved();
}


OperationStatus LogCollectorService::StartLogging(const std::string& deviceName, CpuType cpuType,
    LoggingType loggingType)
{
    OperationStatus os;
    if (!Host::GetHost().GetDeviceManager().DoesDeviceExist(deviceName))
    {
        return OperationStatus(false, "Device does not exist: " + deviceName);
    }

    auto DeviceLoggingContextIter = m_DeviceLoggingContextMap.find(deviceName);
    if (DeviceLoggingContextIter == m_DeviceLoggingContextMap.end())
    {
        return OperationStatus(false, "No Logging Context for : " + deviceName);
    }
    if (cpuType == CPU_TYPE_FW)
    {
        os = DeviceLoggingContextIter->second->GetFwLogCollector()->ActivateLogging(loggingType);
    }
    if (cpuType == CPU_TYPE_UCODE)
    {
        os = DeviceLoggingContextIter->second->GetuCodeLogCollector()->ActivateLogging(loggingType);
    }

    return os;
}

OperationStatus LogCollectorService::StopLogging(const std::string& deviceName, CpuType cpuType,
    LoggingType loggingType)
{
    OperationStatus os;
    if (!Host::GetHost().GetDeviceManager().DoesDeviceExist(deviceName))
    {
        return OperationStatus(false, "Device Name does not exist: " + deviceName);
    }

    auto DeviceLoggingContext = m_DeviceLoggingContextMap.find(deviceName);
    if (DeviceLoggingContext == m_DeviceLoggingContextMap.end())
    {
        return OperationStatus(false, "No Logging Context for this device: " + deviceName);
    }

    if (cpuType == CPU_TYPE_FW || cpuType == CPU_TYPE_BOTH)
    {
        DeviceLoggingContext->second->GetFwLogCollector()->DeactivateLogging(loggingType);
    }
    if (cpuType == CPU_TYPE_UCODE || cpuType == CPU_TYPE_BOTH)
    {
        DeviceLoggingContext->second->GetuCodeLogCollector()->DeactivateLogging(loggingType);
    }

    return os;
}

OperationStatus LogCollectorService::GetLogLoggingState(const std::string& deviceName, CpuType cpuType,
    bool& isLogging, LoggingType loggingType) const
{
    isLogging = false;
    OperationStatus os;
    if (!Host::GetHost().GetDeviceManager().DoesDeviceExist(deviceName))
    {
        return OperationStatus(false, "Device Name does not exist");
    }

    auto DeviceLoggingContext = m_DeviceLoggingContextMap.find(deviceName);
    if (DeviceLoggingContext == m_DeviceLoggingContextMap.end())
    {
        return OperationStatus(false, "No Logging Context for this device: " + deviceName);
    }

    switch (cpuType)
    {
    case CPU_TYPE_FW:
        isLogging = DeviceLoggingContext->second->GetFwLogCollector()->IsLoggingValid(loggingType);
        break;
    case CPU_TYPE_UCODE:
        isLogging = DeviceLoggingContext->second->GetuCodeLogCollector()->IsLoggingValid(loggingType);
        break;
    default:
        os = OperationStatus(false, "No such a cpu type");
        break;
    }

    return os;
}

void LogCollectorService::UpdateFwVersion(const std::string& deviceName)
{
    OperationStatus os;
    if (!Host::GetHost().GetDeviceManager().DoesDeviceExist(deviceName))
    {
        return;
    }

    auto DeviceLoggingContext = m_DeviceLoggingContextMap.find(deviceName);
    if (DeviceLoggingContext == m_DeviceLoggingContextMap.end())
    {
        return;
    }

    DeviceLoggingContext->second->VerifyFwVersion();

}

bool LogCollectorService::IsLogging()
{
    for(const auto & kv : m_DeviceLoggingContextMap)
    {
       for(const auto & lt : {RawPublisher, TxtRecorder, XmlRecorder})
       {
           if(kv.second->GetFwLogCollector()->IsLoggingValid(lt) || kv.second->GetuCodeLogCollector()->IsLoggingValid(lt))
           {
               return true;
           }
       }
    }
    return false;
}

bool LogCollectorService::GetDeferredLoggingState() const
{
    return m_deferredRecordingEnabled;
}


// The new method to start deferred recording/ publishing is to add the required chunk consumer type
// to the set m_deferredLoggingType and then every new device will get those consumers on creation!
OperationStatus LogCollectorService::EnableDeferredLogging(LoggingType loggingType)
{
    if (loggingType == RawPublisher)
    {
        return OperationStatus(false, "Deferred publishing is not supported"); // report exact missing file
    }
    if (loggingType == TxtRecorder)
    {
        // make sure conversion files exist in the configured location, make this common code to check conversion file validity.
        const string conversionFileDirectory = Host::GetHost().GetConfiguration().LogCollectorConfiguration.ConversionFileLocation;
        const bool validConversionFile =
            FileSystemOsAbstraction::DoesFileExist(conversionFileDirectory + "fw_image_trace_string_load.bin") &&
            FileSystemOsAbstraction::DoesFileExist(conversionFileDirectory + "ucode_image_trace_string_load.bin"); //move name to constant
        if (!validConversionFile)
        {
            return OperationStatus(false, "conversion file not found at: " + conversionFileDirectory); // report exact missing file
        }
    }
    m_deferredLoggingType = loggingType;
    m_deferredRecordingEnabled = true;
    return OperationStatus();
}

// To stop *future* deferred recording we remove the chunk consumers from the set
void LogCollectorService::DisableDeferredLogging()
{
    // No need to stop existing recordings only disable deferred mode for future recordings.
    m_deferredLoggingType = XmlRecorder; // default deferred recording is
    m_deferredRecordingEnabled = false;
}

OperationStatus LogCollectorService::SetLogCollectionModuleVerbosity(const std::string& deviceName, CpuType cpuType,
    const std::string& module, const std::string& level)
{
    OperationStatus os;
    if (!Host::GetHost().GetDeviceManager().DoesDeviceExist(deviceName))
    {
        return OperationStatus(false, "Device Name does not exist: " + deviceName);
    }
    //lock_guard<mutex> lock(device->m_mutex);
    auto DeviceLoggingContext = m_DeviceLoggingContextMap.find(deviceName);
    if (DeviceLoggingContext == m_DeviceLoggingContextMap.end())
    {
        return OperationStatus(false, "No Logging Context for this device: " + deviceName);
    }

    switch (cpuType)
    {
    case CPU_TYPE_FW:
        os = DeviceLoggingContext->second->GetFwLogCollector()->SetModuleVerbosity(module, level);
        break;
    case CPU_TYPE_UCODE:
        os = DeviceLoggingContext->second->GetuCodeLogCollector()->SetModuleVerbosity(module, level);
        break;
    default:
        os = OperationStatus(false, "No such a cpu type");
        break;
    }


    return os;
}

bool LogCollectorService::GetModuleVerbosityMap(const std::string& deviceName, CpuType cpuType,
    std::vector<std::pair<std::string, std::string> > &verbosityData)
{
    auto DeviceLoggingContext = m_DeviceLoggingContextMap.find(deviceName);
    if (DeviceLoggingContext == m_DeviceLoggingContextMap.end())
    {
        LOG_ERROR << "Device name not found: " << deviceName << endl;
         return false;
    }
    if (CPU_TYPE_FW == cpuType)
    {
        if (!DeviceLoggingContext->second->GetFwLogCollector()->GetModuleLevelInfo(verbosityData))
        {
            LOG_ERROR << "Failed to get FW module level info";
            return false;
        }
    }
    else
    {
        if (!DeviceLoggingContext->second->GetuCodeLogCollector()->GetModuleLevelInfo(verbosityData))
        {
            LOG_ERROR << "Failed to uCode get module level info";
            return false;
        }
    }
    return true;
}

//  ***************  TODO : For the regular tcp server. should be removed in the future. **********************
std::string LogCollectorService::GetAllModulesVerbosityString(const std::string& deviceName, const std::string& cpuType)
{
    stringstream responseMessage;
    if (!Host::GetHost().GetDeviceManager().DoesDeviceExist(deviceName))
    {
        responseMessage << "device name " << deviceName << " doesn't exist" << endl;
        return responseMessage.str();
    }
    //lock_guard<mutex> lock(device->m_mutex);
    auto DeviceLoggingContext = m_DeviceLoggingContextMap.find(deviceName);
    if (DeviceLoggingContext == m_DeviceLoggingContextMap.end())
    {
        responseMessage << "device " << deviceName << " has no recording context ";
        return responseMessage.str();
    }
    std::vector<std::pair<string, string> > verbosityData;
    if ("fw" == cpuType)
    {
        if(!DeviceLoggingContext->second->GetFwLogCollector()->GetModuleLevelInfo(verbosityData))
        {
            return "Failed to get module level info";
        }
    }
    else if ("ucode" == cpuType)
    {
        if (!DeviceLoggingContext->second->GetuCodeLogCollector()->GetModuleLevelInfo(verbosityData))
        {
            return "Failed to get module level info";
        }
    }
    else
    {
        responseMessage << "device " << deviceName << " invalid  cpu type: " << cpuType;
        return responseMessage.str();
    }

    for (const auto &kv : verbosityData)
    {
        responseMessage << kv.first << "=" /*configuration_parameter_value_delimiter*/ <<
            kv.second << ",";
    }
    string str = responseMessage.str();

    return str.erase(str.size() - 1, 1); // delete last delimiter character

}



//  ***************  TODO : For the regular tcp server. should be removed in the future. **********************
string LogCollectorService::GetLogCollectionParameter(const string& deviceName, const string& cpuTypeName, const string& parameter)
{
    if (POLLING_INTERVAL_MS == parameter)
    {
        stringstream ss;
        ss << int64_t(Host::GetHost().GetConfiguration().LogCollectorConfiguration.PollingIntervalMs);
        return ss.str();
    }
    if (OUTPUT_FILE_SUFFIX == parameter)
    {
        return string();
    }
    if (MODULES_VERBOSITY_LEVELS == parameter)
    {
        return GetAllModulesVerbosityString(deviceName, cpuTypeName);
    }
    // TODO*: might be removed.
    if (MAX_SINGLE_FILE_SIZE_MB == parameter)
    {
        stringstream ss;
        ss << int32_t(Host::GetHost().GetConfiguration().LogCollectorConfiguration.MaxLogFileSize);
        return ss.str();
    }
    if (MAX_NUM_OF_LOG_FILES == parameter)
    {
        stringstream ss;
        ss << int32_t(Host::GetHost().GetConfiguration().LogCollectorConfiguration.MaxNumOfLogFiles);
        return ss.str();
    }

    return "No such parameter";

}

//  ***************  TODO : For the regular tcp server. should be removed in the future. **********************
string LogCollectorService::GetLogCollectorConfigurationString(const string& deviceName, const string& cpuTypeName)
{
    LOG_ERROR << "in " << __FUNCTION__ << __LINE__ << endl;
    stringstream res;
    res << "device=" << deviceName << ",cpu=" << cpuTypeName
        << ",PollingIntervalMs=" << int64_t(Host::GetHost().GetConfiguration().LogCollectorConfiguration.PollingIntervalMs)
        << ",MaxSingleFileSizeMb=" << int32_t(Host::GetHost().GetConfiguration().LogCollectorConfiguration.MaxLogFileSize)
        << ",MaxNumberOfLogFiles=" << int32_t(Host::GetHost().GetConfiguration().LogCollectorConfiguration.MaxNumOfLogFiles)
        << ",OutputFileSuffix=" << "_"/* empty file name suffix*/
        << ",ModulesVerbosityLevels=" << GetAllModulesVerbosityString(deviceName, cpuTypeName) << endl;
    return res.str();
}


//  ***************  TODO : For the regular tcp server. should be removed in the future. **********************
std::shared_ptr<LogReader> LogCollectorService::GetLogCollector(const string& deviceName, const string& cpuTypeName, string& errorMessage)
{
    static const string fw = "fw";
    static const string ucode = "ucode";

    stringstream errorMessageSs;

    auto DeviceLoggingContext = m_DeviceLoggingContextMap.find(deviceName);
    if (DeviceLoggingContext == m_DeviceLoggingContextMap.end())
    {
        errorMessageSs << "No Logging Context for this device: " << deviceName << endl;
        return nullptr;
    }


    // get tracer
    shared_ptr<LogReader> tracer;
    if (fw == cpuTypeName)
    {
        tracer = DeviceLoggingContext->second->GetFwLogCollector();
    }
    else if (ucode == cpuTypeName)
    {
        tracer = DeviceLoggingContext->second->GetuCodeLogCollector();
    }
    else
    {
        errorMessageSs << "device " << deviceName << " has no tracer named " << cpuTypeName << "; ";
        errorMessage = errorMessageSs.str();
        return nullptr;
    }

    // check tracer is valid
    if (!tracer)
    {
        errorMessageSs << "device " << deviceName << " has no active tracer for " << cpuTypeName << "; ";
        errorMessage = errorMessageSs.str();
        return nullptr;
    }

    return tracer;
}

bool TryConvertStringToInteger(const string& parameterAssignment, int& i)
{
    std::size_t found = parameterAssignment.find("=");
    if (std::string::npos != found)
    {
        const std::string interval(parameterAssignment.substr(found + 1));
        i = atoi(interval.c_str());
        return !(0 == i && interval != "0");
    }
    LOG_ERROR << "String Separator (=) not found in parameterAssignment: " << parameterAssignment << endl;
    return false;
}


//  ***************  TODO : For the regular tcp server. should be removed in the future. **********************
bool LogCollectorService::SetLogCollectionParameter(const string& deviceName, const string& cpuTypeName, const string& parameterAssignment, string& errorMessage)
{

    // This function is called from the TCP connection with DmTools.
    // therefore it should affect the device directly and not the configuration.

    // from dmtools can only set few of the parameters :
    // Polling interval, verbosity

    if (std::string::npos != parameterAssignment.find(POLLING_INTERVAL_MS))
    {
        int pollingInterval;
        if (!TryConvertStringToInteger(parameterAssignment, pollingInterval) )
        {
            LOG_ERROR << "Failed to convert polling interval: "<< parameterAssignment << endl;

            return false;
        }

        Host::GetHost().GetConfiguration().LogCollectorConfiguration.PollingIntervalMs = pollingInterval;
        for (const auto & kv : m_DeviceLoggingContextMap)
        {
            bool res = kv.second->GetFwLogCollector()->SetPollingInterval(pollingInterval);
            res &= kv.second->GetuCodeLogCollector()->SetPollingInterval(pollingInterval);
            if (!res)
            {
                return false;
            }


            if (!kv.second->PrepareForLogging())
            {
                LOG_ERROR << "Failed to Prepare recording. Configuration might be invalid. Fix configuration and start recording again." << endl;
                return false;
            }
        }

        return true;
    }
    if (std::string::npos != parameterAssignment.find(MODULES_VERBOSITY_LEVELS))
    {
        auto tracer = GetLogCollector(deviceName, cpuTypeName, errorMessage);

        if (!tracer)
        {
            return false;
        }

        return tracer->SetModuleVerbosityFromAssignment(parameterAssignment);
    }
    errorMessage = "Invalid parameterAssignment " + parameterAssignment;
    return false;
}

