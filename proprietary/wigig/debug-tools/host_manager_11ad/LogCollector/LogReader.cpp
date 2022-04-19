/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <locale> // for std::tolower
#include <sstream>
#include <iostream>
#include <map>
#include <string>

#include "LogReader.h"
#include "Device.h"
#include "DriverAPI.h"
#include "Host.h"
#include "PersistentConfiguration.h"
#include "TaskManager.h"
#include "DebugLogger.h"
#include "LogCollectorDefinitions.h"
#include "DeviceManager.h"
#include "LogCollectorService.h"

using namespace std;

using namespace log_collector;

namespace
{

    //Old Api Only
    const string ConvertModuleVerbosityStructToString(const module_level_info& verbosityStruct)
    {
        stringstream verbosityStringStream;

        if (verbosityStruct.verbose_level_enable)
        {
            verbosityStringStream << "V";
        }
        if (verbosityStruct.info_level_enable)
        {
            verbosityStringStream << "I";
        }
        if (verbosityStruct.error_level_enable)
        {
            verbosityStringStream << "E";
        }
        if (verbosityStruct.warn_level_enable)
        {
            verbosityStringStream << "W";
        }

        return verbosityStringStream.str();
    }

    const char configuration_delimiter_string = ',';
}


bool LogReader::Tracer3ppLogsFilterOn = false;




LogReader::LogReader(const std::string& deviceName, CpuType tracerType) :
    m_deviceName(deviceName),
    m_tracerCpuType(tracerType),
    m_debugLogPrefix(m_deviceName + " " + CPU_TYPE_TO_STRING[tracerType] + " tracer:"),
    m_pollerTaskId(std::numeric_limits<uint32_t>::max()),
    m_valid(false),
    m_fwInit(false),
    m_logHeader(new log_buffer()),
    m_logAddress(Device::sc_invalidAddress),
    m_logBufferSizeInDwords(256),    // Minimal value, will be updated
    m_bufferConsumer(new BufferConsumer(m_deviceName, m_tracerCpuType, m_logBufferSizeInDwords)),
    m_prevLogOffset(-1)
{
}


LogReader::~LogReader()
{
    //TODO: remove after taskManager is fixed (Stop guaranty that all poller stopped ) .
    unique_lock<mutex> destructionLock(m_pollDestructionMutex);
    Host::GetHost().GetTaskScheduler().UnregisterTask(m_pollerTaskId);
    //TODO: remove after taskManager is fixed (Stop guaranty that all poller stopped ) .
    destructionLock.unlock();
}

void LogReader::Poll()
{
    //TODO: remove after taskManager is fixed (Stop guaranty that all poller stopped ) .
    lock_guard<mutex> destructionLock(m_pollDestructionMutex);
    if (!(Host::GetHost().GetDeviceManager().DoesDeviceExist(m_deviceName)) || !m_bufferConsumer->IsAnyChunkConsumerActive() )
    {
        return;
    }

    if (!m_valid)
    {
        //buffer start address is not set.
        LOG_DEBUG << m_debugLogPrefix << " not valid" << endl;
        if (!UpdateDeviceInfo())
        {
            LOG_DEBUG << m_debugLogPrefix << " UpdateDeviceInfo Failed " << endl;
            return;
        }
        // LogCollector was invalid, now it is valid and we need to check if the FW was changed for the device.
        //Host::GetHost().GetLogCollectorService().UpdateFwVersion(m_deviceName);
    }

    // TODO: should be changed when locking is in device
    shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
    if (!device)
    {
        LOG_ERROR << "Invalid device name provided " << m_deviceName << endl;
        return;
    }
    unique_lock<mutex> lock(device->m_mutex); // should lock, called only in init (after device was added to the collection in device manager)
    if (device->GetSilenceMode())
    {
        // In this case collection is not needed (device is silent or nor recording nor publishing)
        //TODOR: (PHASE2) may be pause the log collector.
        return;
    }

    // release the lock before going to publish
    const FwStateProvider* fwStateProvider = device->GetFwStateProvider();
    lock.unlock();
    if (!m_fwInit)
    {
        LOG_DEBUG << m_debugLogPrefix << " Fw was not ready" << endl;
        // does not mean it is not valid. validity is set by  ComputeLogBufferStartAddress()
        m_fwInit = fwStateProvider->IsInitialized();
        if (m_fwInit && GetModuleInfoFromDevice())
        {
            LOG_DEBUG << m_debugLogPrefix << " Fw is now ready writing to FW log file" << endl;
            m_bufferConsumer->ReportAndUpdateFwVersion(fwStateProvider, m_logHeader->module_level_info_array);
        }

    }
    GetNextLogs();
}

bool LogReader::UpdateDeviceInfo()
{
    shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
    if (!device)
    {
        LOG_ERROR << "Invalid device name provided " << m_deviceName << endl;
        m_valid = false;
        return m_valid;
    }
    unique_lock<mutex> lock(device->m_mutex); // should lock, called only in init (after device was added to the collection in device manager)
    const FwStateProvider* fwStateProvider = device->GetFwStateProvider();
    lock.unlock();
    if (!fwStateProvider->IsInitialized())
    {
        LOG_DEBUG << m_debugLogPrefix << " Fw not ready, FW state is: " << fwStateProvider->GetFwHealthState() << endl;
        // does not mean it is not valid. validity is set by  ComputeLogBufferStartAddress()
        m_fwInit = false;
    }
    else
    {
        LOG_DEBUG << m_debugLogPrefix << " Fw ready, FW state is: " << fwStateProvider->GetFwHealthState() << endl;
        m_fwInit = true;
    }

    bool res = ComputeLogBufferStartAddress() && GetModuleInfoFromDevice();

    if (!res)
    {
        LOG_DEBUG << m_debugLogPrefix << " Failed to initialize Log buffer and Module info." << endl;
        m_valid = false;
        return m_valid; // Error in device should not start/continue recording
    }

    if (!m_bufferConsumer->Reset(m_logHeader->module_level_info_array, m_logBufferSizeInDwords, fwStateProvider))
    {
        m_valid = false;
        return m_valid;
    }

    m_valid = true;
    return m_valid;
}


void LogReader::GetNextLogs()
{
    // **** NOTE!!! LOCKS are only in the inner functions so pay attention not to dead LOCK
    // **** by LOCKING from high level functions.
    if (!ReadLogBuffer())
    {
        return;
    }

    // Prepare a header pointing to log buffer top
    log_buffer *h = (log_buffer*)m_logBufferContent.data(); // h contains all the log buffer (including the log buffer header)

    m_bufferConsumer->ConsumeBuffer(h);
}

bool LogReader::SetPollingInterval(unsigned pollingInterval) const
{
    LOG_INFO << m_debugLogPrefix << "SetPollingInterval to: " << pollingInterval << endl;
    if (IsLoggingValid())
    {
        return Host::GetHost().GetTaskScheduler().ChangePollingInterval(m_pollerTaskId, std::chrono::milliseconds(pollingInterval));
    }
    return true;
}

OperationStatus LogReader::ApplyModuleVerbosity()
{
    LOG_DEBUG << m_debugLogPrefix << "Apply module verbosity" << endl;
    if (!ComputeLogBufferStartAddress())
    {
        return OperationStatus(false, "Cannot compute log buffer start/size");
    }

    // TODO: should be changed when locking is in device
    shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
    if (!device)
    {
        LOG_ERROR << "Device is no longer valid " << m_deviceName << endl;
        return  OperationStatus(false, "Device is no longer valid " + m_deviceName);
    }
    unique_lock<mutex> lock(device->m_mutex); // should lock, called only in init (after device was added to the collection in device manager)
    bool writeblockRes = device->GetDriver()->WriteBlock(m_logAddress + sizeof(m_logHeader->write_ptr), sizeof(m_logHeader->module_level_info_array), reinterpret_cast<char*>(m_logHeader->module_level_info_array));
    lock.unlock();


    if (!writeblockRes)
    {
        stringstream st;
        st << m_debugLogPrefix << " Failed to write module verbosity structure for "
            << "(address " << Address(m_logAddress + sizeof(m_logHeader->write_ptr))
            << ", size of data: " << sizeof(m_logHeader->module_level_info_array) << ")";
        LOG_ERROR << __FUNCTION__ << ", " << st.str() << endl;
        return OperationStatus(false, st.str());
    }
    else
    {
        LOG_VERBOSE << m_debugLogPrefix << "Module verbosity for " << m_tracerCpuType
            << "was set" << endl;
    }
    return OperationStatus(true);
}

bool ConvertModuleVerbosityStringToStruct(const string& verbosityString, module_level_info& verbosityStruct)
{
    module_level_info verbosityStructInternal;
    for (const auto& c : verbosityString)
    {
        switch (c)
        {
        case 'V':
            verbosityStructInternal.verbose_level_enable = 1;
            break;
        case 'I':
            verbosityStructInternal.info_level_enable = 1;
            break;
        case 'E':
            verbosityStructInternal.error_level_enable = 1;
            break;
        case 'W':
            verbosityStructInternal.warn_level_enable = 1;
            break;
        default:
            return false;
        }
    }
    verbosityStruct = verbosityStructInternal;
    return true;
}

bool LogReader::SetModuleVerbosityInner(const string& module, const string& level)
{
    for (int i = 0; i < NUM_MODULES; i++)
    {
        if (module_names[i] == module)
        {
            module_level_info newValue;
            if (!ConvertModuleVerbosityStringToStruct(level, newValue))
            {
                return false;
            }

            LOG_DEBUG << "setting module verbosity for module: " << module << "to: " << level << endl;
            m_logHeader->module_level_info_array[i] = newValue;
            return true;
        }
    }
    LOG_ERROR << "Failed to set verbosity " << level << " to module " << module << endl;
    return false;
}


bool LogReader::SetMultipleModulesVerbosity(const string& modulesVerbosity)
{
    // split in case there are multiple modules in the same line, i.e: SYSTEM=VIEW,DRIVER=VIEW
    stringstream ss(modulesVerbosity);
    string moduleLevel;
    while (std::getline(ss, moduleLevel, configuration_delimiter_string))
    {
        size_t delimeterPos = moduleLevel.find('=');
        if (delimeterPos != string::npos && (moduleLevel.length() > delimeterPos)) // ">" and not ">=" for skipping '='
        {
            SetModuleVerbosityInner(moduleLevel.substr(0, delimeterPos), moduleLevel.substr(delimeterPos + 1));
        }
    }

    return ApplyModuleVerbosity();
}

bool LogReader::SetModuleVerbosityFromAssignment(const std::string& assignment)
{
    std::size_t found = assignment.find("=");
    if (std::string::npos != found)
    {
        return SetMultipleModulesVerbosity(assignment.substr(found + 1));
    }
    return false;
}


void LogReader::ReportDeviceRemoved()
{
    m_valid = false;
    m_bufferConsumer->ResetRWPointers();
    m_bufferConsumer->ReportDeviceRemoved();
}

void LogReader::RegisterPollerAndReportDeviceUp()
{
    RegisterPoller();
    m_bufferConsumer->ReportDeviceUp();
}

void LogReader::RegisterPoller()
{

    m_pollerTaskId = Host::GetHost().GetTaskScheduler().RegisterTask(
        std::bind(&LogReader::Poll, this), static_cast<std::chrono::milliseconds>(int64_t(Host::GetHost().GetConfiguration().LogCollectorConfiguration.PollingIntervalMs)), true);
    LOG_DEBUG << __FUNCTION__ << ": poller id is: " << m_pollerTaskId << endl;
}

void LogReader::UnRegisterPoller()
{
    Host::GetHost().GetTaskScheduler().UnregisterTask(m_pollerTaskId);
}


OperationStatus LogReader::SetModuleVerbosity(const std::string& module, const std::string& level)
{
    bool os = SetModuleVerbosityInner(module, level);
    if (os)
    {
        os = ApplyModuleVerbosity();
    }
    return OperationStatus(os);
}

bool LogReader::GetModuleLevelInfo(std::vector<std::pair<std::string, std::string> > &verbosityData)
{
    if (!GetModuleInfoFromDevice())
    {
        return false;
    }
    for (int i = 0; i < NUM_MODULES; i++)
    {
        verbosityData.emplace_back(make_pair( module_names[i], ConvertModuleVerbosityStructToString(m_logHeader->module_level_info_array[i])));
    }
    return true;
}

OperationStatus LogReader::ActivateLogging(LoggingType loggingType)
{
    // Updating Fw state and device info is done in this flow - on activation.
    // therefore no other actions should be taken before.

    if (IsLoggingValid(loggingType))
    {
        return OperationStatus(true, "Logs are already being recorded");
    }

    return ActivateLoggingInner(loggingType);
}

OperationStatus LogReader::ActivateLoggingInner(LoggingType loggingType)
{

    // TODO: should be changed when locking is in device
    shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
    if (!device)
    {
        LOG_ERROR << "Device is no longer valid " << m_deviceName << endl;
        m_valid = false;
        return  OperationStatus(false, "Device is no longer valid " + m_deviceName);
    }
    unique_lock<mutex> lock(device->m_mutex); // should lock, called only in init (after device was added to the collection in device manager)
    const auto fwStateProvider = device->GetFwStateProvider();
    lock.unlock();

    if (!fwStateProvider->IsInitialized())
    {
        LOG_DEBUG << m_debugLogPrefix << " Fw not ready, FW state is: " << fwStateProvider->GetFwHealthState() << endl;
        // does not mean it is not valid. validity is set by  ComputeLogBufferStartAddress()
        m_fwInit = false;
    }
    else
    {
        LOG_DEBUG << m_debugLogPrefix << " Fw ready, FW state is: " << fwStateProvider->GetFwHealthState() << endl;
        m_fwInit = true;
    }
    // update polling interval
    if (!IsLoggingValid())
    {
        // if there were no consumers before the poller was not registered so need to register now.
        RegisterPoller();
    }
    if (!SetPollingInterval(Host::GetHost().GetConfiguration().LogCollectorConfiguration.PollingIntervalMs))
    {
        return  OperationStatus(false, "Failed to set polling interval, Log collection aborted.");
    }


    bool res = ComputeLogBufferStartAddress() && GetModuleInfoFromDevice();

    if (!res)
    {
        LOG_DEBUG << m_debugLogPrefix << " Failed to initialize Log buffer and Module info." << endl;
        m_valid = false;
        // If Log buffer is not initialized yet, we set the active flag anyway and will try to validateon polling
        m_bufferConsumer->SetActiveFlag(loggingType);
        return OperationStatus(false, " Failed to initialize Log buffer and Module info."); // Error in device should not start/continue recording
    }

    m_bufferConsumer->UpdateBufferSize(m_logBufferSizeInDwords);
    m_valid = true;
    LOG_DEBUG << "******************************before ActivateChunkConsumer in ActivateLogging *********************" << endl;
    return m_bufferConsumer->ActivateChunkConsumer(loggingType, m_logHeader->module_level_info_array, fwStateProvider);

}


OperationStatus LogReader::RestoreLogging()
{
    // should be used only on restoring session - device was already runnig
    LOG_DEBUG << m_debugLogPrefix << " in RestoreLogging" << endl;
    OperationStatus osXml;
    OperationStatus osTxt;

    if (IsLogging())
    {
        LOG_DEBUG << m_debugLogPrefix << " isLogging!" << endl;
        //activate inner
        if(IsLogging(XmlRecorder))
        {
            osXml = ActivateLoggingInner(XmlRecorder);
        }
        if(IsLogging(TxtRecorder))
        {
            osTxt = ActivateLoggingInner(TxtRecorder);
        }
        OperationStatus os = OperationStatus::Merge(osXml, osTxt);
        if (!os)
        {
            return os;
        }

        LOG_DEBUG << "Resuming FW logging for: " << m_deviceName << endl;
    }
    return OperationStatus();
}

void LogReader::DeactivateLogging(LoggingType loggingType)
{
    m_bufferConsumer->StopConsumer(loggingType);
    LOG_DEBUG << m_debugLogPrefix << loggingType << " Log Consumer deactivated" << endl;

    if (!IsLoggingValid())
    {
        // if there are no more consumers now  need to UnRegister.
        UnRegisterPoller();
        m_bufferConsumer->ResetRWPointers();
        LOG_DEBUG << m_debugLogPrefix << "Read and Last Write pointers were reset" << endl;
    }

}

bool LogReader::IsLoggingValid(LoggingType loggingType) const
{
    return m_valid && IsLogging(loggingType);
}


bool LogReader::IsLoggingValid() const
{
    // Checking if at least one of the consumers is logging
    return  m_valid && IsLogging();
}

bool LogReader::IsLogging(LoggingType loggingType) const
{
    return  m_bufferConsumer->IsActive(loggingType);
}


bool LogReader::IsLogging() const
{
    // Checking if at least one of the consumers is logging
    return IsLogging(XmlRecorder) || IsLogging(TxtRecorder) || IsLogging(RawPublisher);
}



bool LogReader::GetModuleInfoFromDevice() const
{
    // TODO: should be changed when locking is in device
    shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
    if (!device)
    {
        LOG_ERROR << "Device is no longer valid " << m_deviceName << endl;
        return  OperationStatus(false, "Device is no longer valid " + m_deviceName);
    }
    unique_lock<mutex> lock(device->m_mutex); // should lock, called only in init (after device was added to the collection in device manager)
    // retrieve BB type (for getting ahb start address)
    bool readBlockRes = device->GetDriver()->ReadBlock(
        m_logAddress + sizeof(m_logHeader->write_ptr),
        sizeof(m_logHeader->module_level_info_array),
        reinterpret_cast<char*>(m_logHeader->module_level_info_array));

    lock.unlock();

    if (!readBlockRes)
    {
        LOG_ERROR << "Debug Log Prefix: " << m_debugLogPrefix
            << " Failed to read module verbosity structure for " << m_tracerCpuType
            << " Address: " << Address(m_logAddress + sizeof(m_logHeader->write_ptr))
            << " Size: " << sizeof(m_logHeader->module_level_info_array)
            << endl;
        return false;
    }
    for (int i = 0; i < NUM_MODULES; i++)
    {
        LOG_DEBUG << "module_level_info_array[" << i << "] = " << m_logHeader->module_level_info_array[i] << endl;
    };
    return true;
}

bool LogReader::ReadLogBuffer()
{

    // TODOR: currently called every iteration should be called only if device was restarted.
    // Update FW & uCode log addresses add put it in m_logAddress
    if (!m_fwInit && !ComputeLogBufferStartAddress())
    {
        // Log buffer is not yet initialized, not an error
        return false;
    }

    // TODO: should be changed when locking is in device
    shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
    if (!device)
    {
        LOG_ERROR << "Device is no longer valid " << m_deviceName << endl;
        return  OperationStatus(false, "Device is no longer valid " + m_deviceName);
    }
    unique_lock<mutex> lock(device->m_mutex); // should lock, called only in init (after device was added to the collection in device manager)
    // retrieve BB type (for getting ahb start address)
    // Read the actual log
    bool readBlockRes = device->GetDriver()->ReadBlock(
        m_logAddress,
        static_cast<DWORD>(m_logBufferContent.size()),
        reinterpret_cast<char*>(m_logBufferContent.data()));

    lock.unlock();

    return readBlockRes;
}

unsigned LogReader::GetAhbStartAddress(BasebandType bb) const
{
    // Get the relevant start address in AHB.
    if (CPU_TYPE_FW == m_tracerCpuType)
    {
        auto it = baseband_to_peripheral_memory_start_address_ahb.find(bb);
        if (baseband_to_peripheral_memory_start_address_ahb.end() == it)
        {
            LOG_ERROR << m_debugLogPrefix << "Log collector doesn't support baseband type (type code is " << bb << ")" << endl;
            return false;
        }
        return it->second;
    }
    else
    {
        auto it = baseband_to_ucode_dccm_start_address_ahb.find(bb);
        if (baseband_to_ucode_dccm_start_address_ahb.end() == it)
        {
            LOG_ERROR << m_debugLogPrefix << "Log collector doesn't support baseband type (type code is " << bb << ")" << endl;
            return false;
        }
        return it->second;
    }
}

unsigned LogReader::GetLinkerStartAddress(BasebandType bb) const
{
    if (CPU_TYPE_FW == m_tracerCpuType)
    {
        auto it = baseband_to_peripheral_memory_start_address_linker.find(bb);
        if (baseband_to_peripheral_memory_start_address_linker.end() == it)
        {
            LOG_ERROR << m_debugLogPrefix << "Log collector doesn't support baseband type (type code is " << bb << ")" << endl;
            return false;
        }
        return it->second;
    }
    else
    {
        auto it = baseband_to_ucode_dccm_start_address_linker.find(bb);
        if (baseband_to_ucode_dccm_start_address_linker.end() == it)
        {
            LOG_ERROR << m_debugLogPrefix << "Log collector doesn't support baseband type (type code is " << bb << ")" << endl;
            return false;
        }
        return it->second;
    }
}

//TODO: check if can be called once and be kept as member.
bool LogReader::ComputeLogBufferStartAddress()
{
    // TODO: should be changed when locking is in device
    shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
    if (!device)
    {
        LOG_ERROR << "Device is no longer valid " << m_deviceName << endl;
        return  false;
    }
    unique_lock<mutex> lock(device->m_mutex); // should lock, called only in init (after device was added to the collection in device manager)
    // retrieve BB type (for getting ahb start address)
    const BasebandType bb = device->GetBasebandType();

    lock.unlock();

    const unsigned ahbStartAddress = GetAhbStartAddress(bb);
    const unsigned linkerStartAddress = GetLinkerStartAddress(bb);

    // calculate the difference between the buffer start address as driver exposes it to the buffer start address as fw/ucode exposes it
    const unsigned ahbToLinkerDelta = ahbStartAddress - linkerStartAddress;

    // retrieve fw/ucode offset from peripheral_memory_start_linker_address
    const DWORD addressToReadLogOffset = logTracerTypeToLogOffsetAddress.at(m_tracerCpuType);
    DWORD logBufferInfoDword = 0;

    LOG_VERBOSE << m_debugLogPrefix << "Log Buffer Pointer"
        << " AHB: " << Address(ahbStartAddress)
        << " Linker: " << Address(linkerStartAddress)
        << " Delta: " << Address(ahbToLinkerDelta)
        << " Const Log Buffer Pointer: " << Address(addressToReadLogOffset)
        << endl;

    if (Host::GetHost().GetDeviceManager().Read(m_deviceName, addressToReadLogOffset, logBufferInfoDword) != dmosSuccess)
    {
        LOG_ERROR << m_debugLogPrefix << "Log collector failed to read log offset address" << endl;
        return false;
    }

    /* calculate first address of fw/ucode log buffer
                          +--------------------------+----------------+
     logBufferInfoDword = |3bits of buffer size index|29bits of offset|
                          +--------------------------+----------------+
    */

    const int logOffset = logBufferInfoDword & 0x1FFFFFFF; // we take only the lower 29 bit

    if (logOffset != m_prevLogOffset)
    {
        LOG_INFO << m_debugLogPrefix << "Log buffer offset changed from " << Address(m_prevLogOffset) << " to " << Address(logOffset) << endl;
        m_prevLogOffset = logOffset;
    }

    if (Device::sc_invalidAddress == logOffset)
    {
        // This may happen when the log buffer is not initialized yet
        LOG_DEBUG << m_debugLogPrefix << "The log buffer is not initialized yet" << endl;
        return false;
    }

    m_logAddress = ahbToLinkerDelta + logOffset; // calculate the log buffer start address.

    //we want to determine the buffer size:
    const int bufferSizeId = (logBufferInfoDword & 0xE0000000) >> 29; /* The three upper bits are the index in the size table */
                                                                      /* explained in GetBufferSizeById */
    const int bufferSizeInBytes = GetBufferSizeInBytesById(bufferSizeId);
    m_logBufferSizeInDwords = bufferSizeInBytes / sizeof(DWORD);

    // update buffer size accordingly
    m_logBufferContent.resize(sizeof(log_buffer) + bufferSizeInBytes);

    LOG_VERBOSE << m_debugLogPrefix << "Log Buffer "
        << " Linker Address: " << Address(logOffset)
        << " AHB Address: " << Address(ahbStartAddress)
        << " Log Address: " << Address(m_logAddress)
        << " Size ID: " << bufferSizeId
        << " Size: " << m_logBufferSizeInDwords << " DW"
        << endl;

    return true;
}

int LogReader::GetBufferSizeInBytesById(int bufferSizeId) const
{
    /*
    * The 3 MSBs of the value in REG_FW_USAGE_1 (for fw) and REG_FW_USAGE_2 (for uCode) are used to determine the size
    * of the their log buffer (respectively). The number created with these 3 MSBs is an index in the following sizes table:
    * 0 - default (4KB for FW, 1KB for uCode) - for backward compatibility
    * 1 - 1K
    * 2 - 2K
    * 3 - 4K
    * 4 - 8K
    * 5 - 16K
    * 6 - 32K
    * 7 - 64K
    */
    const int bufferSizeMultiplier[] = { 0, 1, 2, 4, 8, 16, 32, 64 };
    int result;
    const int KB = 1024;

    if (bufferSizeId < 0 || bufferSizeId > 7)
    {
        LOG_ERROR << "Could not get buffer size from logs header roll back to default size (CPU_TYPE_FW = 1024, CPU_TYPE_UCODE = 256)" << endl;
        bufferSizeId = 0;
    }

    if (bufferSizeId == 0)
    { // default values, for backward compatibility
        result = (m_tracerCpuType == CPU_TYPE_FW) ? 4 * KB : 1 * KB;
    }
    else
    {
        result = bufferSizeMultiplier[bufferSizeId] * KB; // buffer size id is calculated by 3 bits only, so its range is 0-7. Therefore can't be out of range.
    }

    return result;
}

