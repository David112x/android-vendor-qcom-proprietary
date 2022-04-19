/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <array>
#include <vector>
#include <thread>
#include <string>
#include <set>
#include <mutex>
#include <functional> // for std::function

// forward declarations
struct nlmsghdr;

// callback for notification on added/removed network interface
// TODO: change int to DeviceType enumeration
typedef std::function<void(int deviceType, const std::set<std::string>&)> InterfaceChangedFunc;

// Singleton class to enumerate and monitor WIGIG network interfaces
class UnixNetworkInterfaceMonitor final
{
public:
    static UnixNetworkInterfaceMonitor& GetInstance();

    // Perform initial enumeration and start monitoring WIGIG interface changes
    std::set<std::string> StartMonitoring(InterfaceChangedFunc interfaceAddedFunc,
                                          InterfaceChangedFunc interfaceRemovedFunc);

    std::set<std::string> GetActiveInterfaces() const;

private:
    mutable std::mutex m_mutex;             // threads synchronization
    const int m_deviceType; //TODO: change to DeviceType enum
    int m_netlinkSocket;                    // file descriptor for the open netlink socket
    std::array<int, 2U> m_exitSockets;      // exit socket used for graceful shutdown
    std::vector<uint8_t> m_eventsBuffer;    // buffer for received events
    std::set<std::string> m_currentInterfaceNames;
    bool m_stopNetworkInterfaceMonitorThread;

    InterfaceChangedFunc m_interfaceAddedFunc;
    InterfaceChangedFunc m_interfaceRemovedFunc;

    UnixNetworkInterfaceMonitor(); // define Cstr to be private, part of Singleton pattern
    ~UnixNetworkInterfaceMonitor();

    std::set<std::string> InitializeCurrentInterfaceNames();
    void StopMonitorThread();
    void CloseSocketPair();
    bool SetupNetlinkSocket();
    void NetworkInterfaceMonitorThread();
    bool WaitForEventsBlocking();
    bool ReadAndHandleEvents();
    int ReadMessagesToBuffer();
    bool ParseAndHandleMessagesInBuffer(int count);
    bool ParseAndHandleSingleMessage(const struct nlmsghdr *nh, const char* messageName);
    void HandleSingleMessage(const char* interfaceName, bool isNewLink);

    std::thread m_monitorThread;
};
