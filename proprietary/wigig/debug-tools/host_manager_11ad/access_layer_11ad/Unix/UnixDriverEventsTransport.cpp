/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#include "UnixDriverEventsTransport.h"
#include "DriverAPI.h"
#include "EventsDefinitions.h"
#include "FwStateProvider.h"
#include "DebugLogger.h"
#include "Host.h"
#include "nl_services.h"

using namespace std;

namespace
{
    // === FW Health State Event structures for private usage, aligned with wil6210 driver === //
    enum class wil_nl_60g_generic_evt
    {
        NL_60G_GEN_EVT_FW_STATE = 0,
        NL_60G_GEN_EVT_AUTO_RADAR_DATA_READY,
    };

    struct wil_nl_60g_fw_state_event
    {
        wil_nl_60g_generic_evt evt_id;
        wil_fw_state fw_sts;
    };
}

// ======================================================================================= //

UnixDriverEventsTransport::UnixDriverEventsTransport() :
    m_nlState(nullptr),
    m_exitSockets{ { -1, -1 } }, // aggregate initialization
    m_stopRunningDriverControlEventsThread(false)
{}

UnixDriverEventsTransport::~UnixDriverEventsTransport()
{
    StopDriverControlEventsThread();
}

// Initialize driver transport and update the capability mask
// \returns True if we got these capabilities from the driver, False if manually initialized
bool UnixDriverEventsTransport::InitializeTransports(const std::string& shortInterfaceName, bitset<32>& driverCapabilities)
{
    driverCapabilities.reset(); // reset-all-bits initialization for case we fail to get it from the driver
    driverCapabilities.set(static_cast<size_t>(wil_nl_60g_driver_capa::NL_60G_DRIVER_CAPA_IOCTL_WRITE)); // no actual knowledge

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, m_exitSockets.data()) == -1) // shouldn't happen
    {
        LOG_ERROR << __FUNCTION__ << ": Failed to create cancellation socket pair, abort netlink interface initialization" << endl;
        return false;
    }

    nl_status res = nl_initialize(shortInterfaceName.c_str(), &m_nlState);
    if ( !(res == NL_SUCCESS && m_nlState) ) // can happen for old driver that doesn't support driver events
    {
        CloseSocketPair(); // release the exit sockets
                           // display warning instead of error to prevent SI test failure
        LOG_WARNING << "Failed to initialize netlink interface [nl_status=" << res << "], reason: " << nl_convert_status_to_string(res) << endl;
        return false;
    }

    // get capabilities from the driver
    uint32_t capabilityMask = 0U;
    const nl_status capRes = nl_get_capabilities(m_nlState, &capabilityMask);
    if (capRes == NL_SUCCESS)
    {
        driverCapabilities = bitset<32>(capabilityMask);
        return true;
    }

    // netlink is initialized, set WMI transport & IOCTL_WRITE bits anyway (backward compatibility with driver before NL_60G_GEN_GET_DRIVER_CAPA)
    // IOCTL_WRITE was set during init
    driverCapabilities.set(static_cast<size_t>(wil_nl_60g_driver_capa::NL_60G_DRIVER_CAPA_WMI));

    if (capRes == NL_FAILED_NOT_SUPPORTED_BY_DRIVER)
    {   // backward compatibility, treat as success
        LOG_WARNING << "wil6210 driver does not support GET_DRIVER_CAPA command. New driver version required" << endl;
        return true;
    }

    LOG_ERROR << "Failed to get driver capabilities. Error: " << nl_convert_status_to_string(capRes) << endl;
    return false;
}

// Register for driver events
void UnixDriverEventsTransport::RegisterForDriverEvents(const std::string& fullInterfaceName, FwStateProvider* fwStateProvider)
{
    if (!m_nlState)
    {
        // working with Debug mailbox, do nothing
        return;
    }

    // Creating a thread to wait on the netlink blocking poll, pass the interface name
    // To prevent miss of published events the thread is created before asking to send events to the user space
    m_waitOnSignalThread = thread(&UnixDriverEventsTransport::DriverControlEventsThread, this, fullInterfaceName, fwStateProvider);

    // Enable events transport on the driver side, i.e. ask to publish them to the user space
    if (nl_enable_driver_events_transport(m_nlState, true /*enable*/) != NL_SUCCESS)
    {
        // Do not halt the flow on error because of backward compatibility with previous Driver version before this feature
        LOG_WARNING << "Setup of commands and events transport is not supported on the driver side, driver upgrade required" << endl;
    }
}

// Send event to the driver
bool UnixDriverEventsTransport::SendDriverCommand(uint32_t id, const void *pInBuf, uint32_t inBufSize, void *pOutBuf, uint32_t outBufSize, DWORD* pLastError)
{
    (void)pLastError;   // not in use, netlink mechanism does not update it

    LOG_ASSERT(pInBuf); // just in case, pInBuf validity should be tested by the caller

    if (pOutBuf && outBufSize != sizeof(uint32_t))
    {   // when response required, the only supported response size is 32 bit
        LOG_WARNING << __FUNCTION__ << ": given output buffer size " << outBufSize << " does not match expected " << sizeof(uint32_t) << std::endl;
        return false;
    }

    nl_status res = nl_send_driver_command(m_nlState, id, inBufSize, pInBuf, reinterpret_cast<uint32_t*>(pOutBuf));

    if (res != NL_SUCCESS)
    {
        LOG_ERROR << "Failed to send driver command, [nl_status=" << res << "],error: " << nl_convert_status_to_string(res) << std::endl;
        return false;
    }

    return true;
}

// Main loop of polling for incoming driver events
void UnixDriverEventsTransport::DriverControlEventsThread(const std::string& fullInterfaceName, FwStateProvider* fwStateProvider)
{
    struct driver_event_report driverEvent;

    while (!m_stopRunningDriverControlEventsThread)
    {
        memset(&driverEvent, 0, sizeof(driverEvent));

        // blocking call to fetch the next event
        nl_status res = nl_get_driver_event(m_nlState, m_exitSockets[1] /* cancellation socket file descriptor */, &driverEvent);

        if (m_stopRunningDriverControlEventsThread) // asked to quit
        {
            // Disable commands and events transport on the Driver side
            // Ignore the possible error because of backward compatibility with previous Driver version before this feature
            nl_enable_driver_events_transport(m_nlState, false /*disable*/);
            break;
        }

        if (res != NL_SUCCESS)
        {
            LOG_ERROR << "On Device " << fullInterfaceName << ", failed to get driver event [nl_status=" << res << "],error: " << nl_convert_status_to_string(res) << std::endl;
            continue;
        }

        if (driverEvent.buf_len > DRIVER_MSG_MAX_LEN)
        {
            // shouldn't happen
            LOG_ERROR << "On Device " << fullInterfaceName << ", got invalid driver event, buffer length is " << driverEvent.buf_len << std::endl;
            continue;
        }

        // handle driver event or push it to connected clients
        if (!HandleDriverEvent(fwStateProvider, driverEvent, fullInterfaceName))
        {
            Host::GetHost().PushEvent(DriverEvent(fullInterfaceName, static_cast<int>(driverEvent.evt_type),
                driverEvent.evt_id, driverEvent.listener_id, driverEvent.buf_len, driverEvent.buf));
        }
    }

    LOG_INFO << "DriverControlEventsThread for device " << fullInterfaceName << " was asked to quit." << std::endl;
}

// \returns True if event was handled
bool UnixDriverEventsTransport::HandleDriverEvent(FwStateProvider* fwStateProvider, const driver_event_report& driverEventReport, const std::string& fullInterfaceName)
{
    if (driverEventReport.evt_type != DRIVER_EVENT_DRIVER_GENERIC_EVENT)
    {
        return false;
    }

    const auto payload = reinterpret_cast<const wil_nl_60g_fw_state_event*>(driverEventReport.buf);

    if (payload->evt_id == wil_nl_60g_generic_evt::NL_60G_GEN_EVT_FW_STATE)
    {
        if (driverEventReport.buf_len != sizeof(wil_nl_60g_fw_state_event))
        {
            // shouldn't happen
            LOG_ERROR << "Got invalid FW_STS_EVENT, expected " << sizeof(wil_nl_60g_fw_state_event)
                << "Bytes of payload, got " << driverEventReport.buf_len << endl;
            return true;
        }

        fwStateProvider->OnFwHealthStateChanged(payload->fw_sts);
        return true;
    }
    else if (payload->evt_id == wil_nl_60g_generic_evt::NL_60G_GEN_EVT_AUTO_RADAR_DATA_READY)
    {
        // ignore, mark handled
        return true;
    }
    // handle other generic driver event...

    // Not an error, may happen if working with a newer driver version
    LOG_WARNING << "On Device " << fullInterfaceName << ", received unsupported driver generic event (type=" << static_cast<uint32_t>(payload->evt_id)
        << "), consider upgrading host_manager_11ad" << endl;
    return false; // was not handled...
}

// Cancel blocking polling for incoming events to allow graceful thread shutdown
void UnixDriverEventsTransport::StopDriverControlEventsThread()
{
    // Stop driver control event thread:
    m_stopRunningDriverControlEventsThread = true;

    // unblocks the monitor receive socket for termination
    // sockets pair serves as a pipe - a value written to one of its sockets, is also written to the second one
    // actual value written has no importance
    if (m_exitSockets[0] >= 0)
    {
        write(m_exitSockets[0], "T", 1);
    }

    if (m_waitOnSignalThread.joinable())
    {
        m_waitOnSignalThread.join();
    }

    // release netlink state
    nl_release(m_nlState);

    // release sockets from the sockets pair used to stop the driver events thread
    CloseSocketPair();
}

// Release sockets from the sockets pair used to stop the driver events thread
void UnixDriverEventsTransport::CloseSocketPair()
{
    if (m_exitSockets[0] >= 0)
    {
        close(m_exitSockets[0]);
        m_exitSockets[0] = -1;
    }
    if (m_exitSockets[1] >= 0)
    {
        close(m_exitSockets[1]);
        m_exitSockets[1] = -1;
    }
}

// Reset FW through generic drive command
// \remark May fail when:
// - Working with Debug mailbox
// - Driver in use does not support NL_60G_GEN_FW_RESET generic command
// - Not WMI_ONLY FW in use
OperationStatus UnixDriverEventsTransport::TryFwReset()
{
    LOG_DEBUG << __FUNCTION__ << endl;

    if (!m_nlState)
    {
        return OperationStatus(false, "FW Reset is not supported for Debug mailbox");
    }

    const nl_status resetRes = nl_fw_reset(m_nlState);
    if (resetRes == NL_SUCCESS)
    {
        return OperationStatus();
    }

    LOG_DEBUG << __FUNCTION__ << ": nl_status is " << static_cast<int>(resetRes) << endl;

    // Provide a specific error description when possible
    if (resetRes == NL_FAILED_NOT_SUPPORTED_BY_DRIVER)
    {
        return OperationStatus(false, "Driver does not support FW RESET command. New driver version required");
    }
    if (resetRes == NL_FAILED_NOT_SUPPORTED_BY_FW)
    {
        return OperationStatus(false, "FW RESET command is supported for WMI_ONLY FW");
    }

    return OperationStatus(false, nl_convert_status_to_string(resetRes));
}
