/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _11AD_PCI_UNIX_DRIVER_EVENTS_HANDLER_H_
#define _11AD_PCI_UNIX_DRIVER_EVENTS_HANDLER_H_

#include "OperationStatus.h"
#include "Utils.h"

#include <array>
#include <string>
#include <thread>
#include <bitset>

// forward declarations
struct nl_state_t;
struct driver_event_report;
class FwStateProvider;

// Internal class for operations related to send/receive of Driver Events
class UnixDriverEventsTransport final
{
public:
    UnixDriverEventsTransport();

    ~UnixDriverEventsTransport();

    // Initialize driver transport and the capability mask
    // \returns True if we got these capabilities from the driver, False if manually initialized
    bool InitializeTransports(const std::string& shortInterfaceName, std::bitset<32>& driverCapabilities);

    // Register for driver events
    void RegisterForDriverEvents(const std::string& fullInterfaceName, FwStateProvider* fwStateProvider);

    // Send event to the driver
    bool SendDriverCommand(uint32_t Id, const void *inBuf, uint32_t inBufSize, void *outBuf, uint32_t outBufSize, DWORD* pLastError);

    // Reset FW through generic drive command
    // \remark May fail when:
    // - Working with Debug mailbox
    // - Driver in use does not support NL_60G_GEN_FW_RESET generic command
    // - Not WMI_ONLY FW in use
    OperationStatus TryFwReset();

    // make it non-copyable
    UnixDriverEventsTransport(const UnixDriverEventsTransport& rhs) = delete;
    UnixDriverEventsTransport& operator=(const UnixDriverEventsTransport& rhs) = delete;

private:
    void DriverControlEventsThread(const std::string& fullInterfaceName, FwStateProvider* fwStateProvider); // main loop of polling for incoming driver events (blocking, no busy wait)
    bool HandleDriverEvent(FwStateProvider* fwStateProvider, const driver_event_report& driverEventReport, const std::string& fullInterfaceName);
    void StopDriverControlEventsThread();   // cancel blocking polling for incoming events to allow graceful thread shutdown
    void CloseSocketPair();                 // release sockets from the sockets pair used to stop the driver events thread

    nl_state_t* m_nlState;                  // representation of the netlink state, kind of device descriptor
    std::array<int,2U> m_exitSockets;       // sockets pair to allow cancellation of blocking polling for incoming events
    bool m_stopRunningDriverControlEventsThread;
    std::thread m_waitOnSignalThread;       // polling thread
};

#endif // _11AD_PCI_UNIX_DRIVER_EVENTS_HANDLER_H_
