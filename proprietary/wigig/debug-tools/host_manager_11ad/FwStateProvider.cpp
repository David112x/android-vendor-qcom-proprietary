/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "FwStateProvider.h"
#include "Device.h"
#include "Host.h"
#include "EventsDefinitions.h"
#include "TaskManager.h"
#include "LogCollector/LogCollectorService.h"


using namespace std;

namespace
{
#ifdef RTL_SIMULATION
    const unsigned READ_POLLING_INTERVAL_MSEC = 60000U;
#else
    const unsigned READ_POLLING_INTERVAL_MSEC = 500U;
#endif
}

std::ostream& operator<<(std::ostream &os, wil_fw_state fwHealthState)
{
    switch (fwHealthState)
    {
    case wil_fw_state::WIL_FW_STATE_UNKNOWN:            return os << "Unknown";
    case wil_fw_state::WIL_FW_STATE_DOWN:               return os << "Down";
    case wil_fw_state::WIL_FW_STATE_READY:              return os << "Ready";
    case wil_fw_state::WIL_FW_STATE_ERROR_BEFORE_READY: return os << "SysAssert before Ready";
    case wil_fw_state::WIL_FW_STATE_ERROR:              return os << "SysAssert";
    case wil_fw_state::WIL_FW_STATE_UNRESPONSIVE:       return os << "Unresponsive";
    default: return os << "Unrecognized fw state " << static_cast<int>(fwHealthState);
    }
}

FwStateProvider::FwStateProvider(const Device& device, bool pollingRequired) :
    m_device(device),
    m_pollingRequired(pollingRequired),
    m_fwError(0U),
    m_uCodeError(0U),
    m_uCodeTimestampStartAddress(Device::sc_invalidAddress),
    m_rfcConnectedValue(0U),
    m_rfcEnabledValue(0U),
    m_fwHealthState(wil_fw_state::WIL_FW_STATE_UNKNOWN),
    m_fwInfoPollerTaskId(0U),
    m_fwUcodeErrorsPollerTaskId(0U)
{
    if (m_pollingRequired)
    {
        // init FW version
        m_device.ReadDeviceFwInfo(m_fwVersion, m_fwTimestamp, m_uCodeTimestamp, m_uCodeTimestampStartAddress);
        // init FW & uCode errors
        m_device.ReadFwUcodeErrors(m_fwError, m_uCodeError);

        m_fwInfoPollerTaskId = Host::GetHost().GetTaskScheduler().RegisterTask(
            std::bind(&FwStateProvider::PollDeviceFwInfo, this), std::chrono::milliseconds(READ_POLLING_INTERVAL_MSEC), true);
        m_fwUcodeErrorsPollerTaskId = Host::GetHost().GetTaskScheduler().RegisterTask(
            std::bind(&FwStateProvider::PollFwUcodeErrors, this), std::chrono::milliseconds(READ_POLLING_INTERVAL_MSEC), true);

        // note: do not poll for FW pointer table changes, supported only with the FW health state change event
    }
    // otherwise FW info and so errors are initialized with zeros and will be updated upon transition to WIL_FW_STATE_READY state
}

FwStateProvider::~FwStateProvider()
{
    if (m_pollingRequired)
    {
        Host::GetHost().GetTaskScheduler().UnregisterTask(m_fwInfoPollerTaskId);
        Host::GetHost().GetTaskScheduler().UnregisterTask(m_fwUcodeErrorsPollerTaskId);
    }
}

FwVersion FwStateProvider::GetFwVersion() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_fwVersion;
}
FwTimestamp FwStateProvider::GetFwTimestamp() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_fwTimestamp;
}
FwTimestamp FwStateProvider::GetUcodeTimestamp() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_uCodeTimestamp;
}
wil_fw_state FwStateProvider::GetFwHealthState() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_fwHealthState;
}

void FwStateProvider::ReadFwPointerTableInternal()
{
    uint32_t rfcConnectedValue = 0U, rfcEnabledValue = 0U;
    uint32_t uCodeTimestampStartAddress = Device::sc_invalidAddress;
    if (m_device.ReadFwPointerTable(rfcConnectedValue, rfcEnabledValue, uCodeTimestampStartAddress))
    {
        m_rfcConnectedValue = rfcConnectedValue;
        m_rfcEnabledValue = rfcEnabledValue;
        m_uCodeTimestampStartAddress = uCodeTimestampStartAddress;
    }
}

void FwStateProvider::PollDeviceFwInfo()
{
    lock_guard<mutex> lock(m_mutex);
    ReadDeviceFwInfoInternal();
}

// \remark assumes the lock is already acquired
void FwStateProvider::ReadDeviceFwInfoInternal()
{
    FwVersion fwVersion;
    FwTimestamp fwTimestamp;
    FwTimestamp uCodeTimestamp;

    if (m_device.ReadDeviceFwInfo(fwVersion, fwTimestamp, uCodeTimestamp, m_uCodeTimestampStartAddress)
        && !(m_fwVersion == fwVersion && m_fwTimestamp == fwTimestamp && m_uCodeTimestamp == uCodeTimestamp))
    {
        m_fwVersion = fwVersion;
        m_fwTimestamp = fwTimestamp;
        m_uCodeTimestamp = uCodeTimestamp;

        LOG_INFO << "On Device " << m_device.GetDeviceName() << ", detected FW ver. " << m_fwVersion
            << ", timestamp " << m_fwTimestamp << " (uCode timestamp " << m_uCodeTimestamp << ")" << endl;

        Host::GetHost().PushEvent(NewDeviceDiscoveredEvent(m_device.GetDeviceName(), m_fwVersion, m_fwTimestamp, m_uCodeTimestamp));
    }
}

void FwStateProvider::PollFwUcodeErrors()
{
    lock_guard<mutex> lock(m_mutex);
    ReadFwUcodeErrorsInternal();
}

// \remark assumes the lock is already acquired
void FwStateProvider::ReadFwUcodeErrorsInternal()
{
    uint32_t fwError = 0;
    uint32_t uCodeError = 0;
    if (m_device.ReadFwUcodeErrors(fwError, uCodeError)
        && !(m_fwError == fwError && m_uCodeError == uCodeError))
    {
        m_fwError = fwError;
        m_uCodeError = uCodeError;

        if (m_fwError != 0U || m_uCodeError != 0U)
        {
            LOG_INFO << "On Device " << m_device.GetDeviceName() << ", detected Firmware error; assert codes: FW " << Hex<>(m_fwError) << ", uCode " << Hex<>(m_uCodeError) << endl;
        }

        Host::GetHost().PushEvent(FwUcodeStatusChangedEvent(m_device.GetDeviceName(), m_fwError, m_uCodeError));
    }
}

// Trivial implementation of a FSM for FW health state changes
// Note: FW can be considered as initialized when m_pollingRequired OR state is one of
//       {WIL_FW_STATE_READY, WIL_FW_STATE_SYSASSERT, WIL_FW_STATE_UNRESPONSIVE}
void FwStateProvider::OnFwHealthStateChanged(wil_fw_state fwHealthState)
{
    lock_guard<mutex> lock(m_mutex);

    if (m_pollingRequired)
    {
        // shouldn't happen
        LOG_ERROR << "On Device " << m_device.GetDeviceName() << ", received FW health state change notification when polling is turned On" << endl;
        return;
    }

    if (m_fwHealthState == fwHealthState)
    {
        // we're done
        LOG_VERBOSE << "On Device " << m_device.GetDeviceName() << ", received FW health state "
            << static_cast<int>(m_fwHealthState) << " (no change)" << endl;
        return;
    }


    switch (fwHealthState)
    {
        case (wil_fw_state::WIL_FW_STATE_READY):
        case (wil_fw_state::WIL_FW_STATE_ERROR_BEFORE_READY):
            // Note: In case of assert before ready it's still worth to try the device init sequence.
            // We may report bad FW info, but at least error code should be correct.
            HandleDeviceInitialization();
            break;

        case (wil_fw_state::WIL_FW_STATE_ERROR):
            if (m_fwHealthState == wil_fw_state::WIL_FW_STATE_UNKNOWN)
            {
                // we're after ready, device is initialized
                HandleDeviceInitialization();
            }
            else
            {
                ReadFwUcodeErrorsInternal();

                if (m_fwError == 0U && m_uCodeError == 0U)
                {
                    // meaning it's the 'unresponsive' state - override the state before pushing event
                    fwHealthState = wil_fw_state::WIL_FW_STATE_UNRESPONSIVE; //update the fwHealthState temporarily and continue
                    LOG_INFO << "On Device " << m_device.GetDeviceName() << ", detected unresponsive FW health state" << endl;
                }
            }
            break;

        case (wil_fw_state::WIL_FW_STATE_UNKNOWN):
        case (wil_fw_state::WIL_FW_STATE_DOWN):
            // do nothing
            break;

        default:
            // not an error, may happen if working with a newer driver version
            LOG_WARNING << "On Device " << m_device.GetDeviceName() << ", received unknown FW health state (" << static_cast<int>(fwHealthState)
                << "), consider upgrading host_manager_11ad" << endl;
            break;
    }
    const auto prevFwHealthState = m_fwHealthState;
    m_fwHealthState = fwHealthState;

    LOG_DEBUG << "On Device " << m_device.GetDeviceName() << ", FW health state changed from: "
        << prevFwHealthState << " to: " << m_fwHealthState << endl;

    Host::GetHost().PushEvent(FwHealthStateChangedEvent(m_device.GetDeviceName(),
        static_cast<int>(m_fwHealthState),
        static_cast<int>(prevFwHealthState)));
}

bool FwStateProvider::IsInitialized() const
{
    // If m_pollingRequired == true then we assume FW is initialized, otherwise check m_fwHealthState
    return (m_pollingRequired ||
        m_fwHealthState == wil_fw_state::WIL_FW_STATE_ERROR_BEFORE_READY ||
        m_fwHealthState == wil_fw_state::WIL_FW_STATE_READY ||
        m_fwHealthState == wil_fw_state::WIL_FW_STATE_ERROR ||
        m_fwHealthState == wil_fw_state::WIL_FW_STATE_UNRESPONSIVE);
}

void FwStateProvider::HandleDeviceInitialization()
{
    ReadFwPointerTableInternal();
    ReadDeviceFwInfoInternal();
    ReadFwUcodeErrorsInternal();
}
