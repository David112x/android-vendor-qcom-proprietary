/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h> // for opendir
#include <unistd.h> // for write
#include <poll.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <thread>
#include <string>
#include <set>
#include <mutex>
#include <algorithm>
#include <iterator>

#include "UnixNetworkInterfaceMonitor.h"
#include "FileSystemOsAbstraction.h"
#include "DebugLogger.h"
#include "Utils.h"

namespace
{
    const char* const PCI_END_POINT_FOLDER = "/sys/module/wil6210/drivers/pci:wil6210/";

    enum { NETLINK_MSG_BUFFER_SIZE = 32768 };

    std::string GetInterfaceNameFromEndPoint(const std::string& pciEndPoint)
    {
        // const definitions
        static const char* CURRENT_DIRECTORY = ".";
        static const char* PARENT_DIRECTORY = "..";

        std::stringstream interfaceNameContaingFolder; // path of a folder which contains the interface name of the specific EP
        interfaceNameContaingFolder << PCI_END_POINT_FOLDER << pciEndPoint << "/net";
        DIR* dp = opendir(interfaceNameContaingFolder.str().c_str());
        if (!dp)
        {
            return ""; // invalid
        }

        // For supporting multi vap (=virtual access point), we get all the interfaces from the same end point,
        // and choose the one without the '-' (which supposed to be the physical interface).
        // If there is more than one interface without '-', we choose the first one.
        // We assume that only virtual interfaces has '-' in their name.
        // TODO : This is kind of a hack until we fully support multi VIF
        dirent* de = nullptr;
        bool foundInterface = false;
        std::string interfaceName = ""; // init as invalid
        while ((de = readdir(dp)) != nullptr)
        {
            if ((strncmp(CURRENT_DIRECTORY, de->d_name, strlen(CURRENT_DIRECTORY)) == 0) || (strncmp(PARENT_DIRECTORY, de->d_name, strlen(PARENT_DIRECTORY)) == 0))
            { // The first two directories that returned from readdir are "." and "..", of course we ignore them
                continue;
            }

            if (strchr(de->d_name, '-') == nullptr)
            { // Interface without '-' means that it is physical interface and not virtual
                LOG_VERBOSE << "Interface name from the directory: " << interfaceNameContaingFolder.str() << " is: " << de->d_name << std::endl;
                interfaceName = de->d_name;
                break;
            }
            else if (!foundInterface)
            { // In case we can't find an interface name without '-', we take the first one available
                foundInterface = true;
                interfaceName = de->d_name;
            }
        }

        closedir(dp);
        return interfaceName;
    }

    // Returns the message name corresponding to given RTM type or nullptr when given type is not relevant
    const char* GetMessageNameFromRtmType(int type)
    {
        #define MESSAGE_NAME_FROM_RTM_TYPE(rtmType) case (rtmType): return #rtmType;

        // the only supported message types are RTM_NEWLINK and RTM_DELLINK
        switch (type)
        {
            MESSAGE_NAME_FROM_RTM_TYPE(RTM_NEWLINK);
            MESSAGE_NAME_FROM_RTM_TYPE(RTM_DELLINK);
        default:
            return nullptr;
        }
    }

    // Ensure this is a relevant interface by query for associated driver
    // Returns true when interface is relevant or relevance is unknown (will be tested by observer)
    // Method:
    // - check file (symbolic link to a folder) exists: /sys/class/net/_iname_/device/driver
    // - read link (will give something like ../../../../bus/pci/drivers/wil6210)
    // - take basename (e.g. wil6210)
    bool IsInterfaceRelevantOrRelevanceUnknown(const char* interfaceName)
    {
        static const char* sc_driverName = "wil6210";

        std::stringstream driverContaingFolder;
        driverContaingFolder << "/sys/class/net/" << interfaceName << "/device/driver";

        if (!FileSystemOsAbstraction::DoesFileExist(driverContaingFolder.str()))
        {
            // can happen upon rmmod when we get NEWLINK
            LOG_VERBOSE << __FUNCTION__ << ": Path '" << driverContaingFolder.str() << "' does not exist (possible for RTM_NEWLINK during rmmod)" << std::endl;
            return false;
        }

        // compare basename of readlink /sys/class/net/_iname_/device/driver with predefined driver name
        std::string driverPath;
        if (FileSystemOsAbstraction::ReadLink(driverContaingFolder.str(), driverPath))
        {
            size_t pos = driverPath.find_last_of('/') + 1U;
            if (pos < driverPath.size() && driverPath.substr(pos) != sc_driverName)
            {
                // ignore irrelevant interface
                LOG_DEBUG << __FUNCTION__ << ": Ignore irrelevant interface " << interfaceName << std::endl;
                return false;
            }
        }

        // relevant interface or relevance unknown, test in both case
        return true;
    }

    std::string FormatInterfaceNames(const char* prefix, const std::set<std::string>& container)
    {
        std::stringstream ss;
        if (prefix)
        {
            ss << prefix;
        }

        if (container.empty())
        {
            ss << "none";
        }
        else if (container.size() == 1U)
        {
            ss << *container.cbegin();
        }
        else
        {
            std::copy(container.cbegin(), std::prev(container.cend()), std::ostream_iterator<std::string>(ss, ", "));
            ss << *std::prev(container.cend());
        }

        return ss.str();
    }
}

// === Monitor functionality === //
// Note:
// NL80211_CMD_NEW_INTERFACE & NL80211_CMD_DEL_INTERFACE events from "config" group of "nl80211" family are raised only in kernel 4.9 and later.
// These are events that dedicated for wifi interfaces, but we cannot assume that all customers have kernel version >= 4.9.
// Current method is to respond to more general network interface events and test their relevance based on corresponding
// interface name and associated host driver.

UnixNetworkInterfaceMonitor& UnixNetworkInterfaceMonitor::GetInstance()
{
    static UnixNetworkInterfaceMonitor monitor;
    return monitor;
}

UnixNetworkInterfaceMonitor::UnixNetworkInterfaceMonitor() :
    m_deviceType(0), // TODO: replace by DeviceType::PCI
    m_netlinkSocket(-1),
    m_exitSockets{ { -1, -1 } }, // aggregate initialization
    m_eventsBuffer(std::vector<uint8_t>(NETLINK_MSG_BUFFER_SIZE)),
    m_stopNetworkInterfaceMonitorThread(false)
{
}

UnixNetworkInterfaceMonitor::~UnixNetworkInterfaceMonitor()
{
    StopMonitorThread();
}

// Cancel blocking polling for incoming netlink events to allow graceful thread shutdown
void UnixNetworkInterfaceMonitor::StopMonitorThread()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Stop driver control event thread:
    m_stopNetworkInterfaceMonitorThread = true;

    // unblocks the monitor receive socket for termination
    // sockets pair serves as a pipe - a value written to one of its sockets, is also written to the second one
    // actual value written has no importance
    if (m_exitSockets[0] >= 0)
    {
        write(m_exitSockets[0], "T", 1);
    }

    if (m_monitorThread.joinable())
    {
        m_monitorThread.join();
    }

    // close netlink socket
    close(m_netlinkSocket);

    // release sockets from the sockets pair used to stop the monitor thread
    CloseSocketPair();
}

// Release sockets from the sockets pair, used to stop the monitor thread
void UnixNetworkInterfaceMonitor::CloseSocketPair()
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

bool UnixNetworkInterfaceMonitor::SetupNetlinkSocket()
{
    // open rtnetlink socket
    // use raw network protocol access since the netlink protocol does not distinguish between datagram and raw sockets
    m_netlinkSocket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (m_netlinkSocket < 0)
    {
        LOG_ERROR << __FUNCTION__ << ": Failed to create netlink socket. Error: " << strerror(errno) << std::endl;
        return false;
    }

    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_groups = RTMGRP_LINK;
    //addr.nl_pid = 0; // when set to zero, kernel will assign a unique pid

    if (bind(m_netlinkSocket, reinterpret_cast<struct sockaddr*>(&nladdr), sizeof(nladdr)) < 0)
    {
        LOG_ERROR << __FUNCTION__ << ": Failed to bind netlink socket. Error: " << strerror(errno) << std::endl;
        close(m_netlinkSocket);
        return false;
    }

    return true;
}

std::set<std::string> UnixNetworkInterfaceMonitor::StartMonitoring(InterfaceChangedFunc interfaceAddedFunc,
                                                                   InterfaceChangedFunc interfaceRemovedFunc)
{
    if ( !(interfaceAddedFunc && interfaceRemovedFunc) ) // shouldn't happen
    {
        LOG_ERROR << __FUNCTION__ << ": Cannot start monitoring with invalid callbacks" << std::endl;
        return std::set<std::string>();
    }

    // this lock will prevent monitor, started below, from reporting changes before initial enumeration state
    std::lock_guard<std::mutex> lock(m_mutex);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, m_exitSockets.data()) == -1) // shouldn't happen
    {
        LOG_ERROR << __FUNCTION__ << ": Failed to create cancellation socket pair, abort netlink socket initialization" << std::endl;
        return std::set<std::string>();
    }

    if (!SetupNetlinkSocket()) // shouldn't happen
    {
        CloseSocketPair();
        return std::set<std::string>();
    }

    m_interfaceAddedFunc = std::move(interfaceAddedFunc);
    m_interfaceRemovedFunc = std::move(interfaceRemovedFunc);

    // create thread to wait on the netlink blocking poll
    // Thread should be created before the first enumeration to prevent race
    m_monitorThread = std::thread(&UnixNetworkInterfaceMonitor::NetworkInterfaceMonitorThread, this);

    // first enumeration
    return InitializeCurrentInterfaceNames();
}

void UnixNetworkInterfaceMonitor::NetworkInterfaceMonitorThread()
{
    while (!m_stopNetworkInterfaceMonitorThread)
    {
        const bool dataReady = WaitForEventsBlocking();

        if (m_stopNetworkInterfaceMonitorThread)
        {
            // asked to quit
            break;
        }

        if (!dataReady)
        {
            // error was already displayed
            continue;
        }

        ReadAndHandleEvents();
    }

    LOG_INFO << "NetworkInterfaceMonitorThread was asked to quit" << std::endl;
}

// Poll for incoming events and return when data is available for read or asked to quit (write to a cancellation socket)
// Returns true when data is ready to read
bool UnixNetworkInterfaceMonitor::WaitForEventsBlocking()
{
    // m_exitSockets[1] is a file descriptor for one of the sockets from the cancellation sockets pair
    // sockets pair serves as a pipe - a value written to one of its sockets, is also written to the second one
    // no lock since socket file descriptors are not changed during run, also prevent blocking of StopMonitorThread
    struct pollfd fds[2];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = m_netlinkSocket;
    fds[0].events |= POLLIN;
    fds[1].fd = m_exitSockets[1];
    fds[1].events |= POLLIN;

    int count = poll(fds, 2, -1); // infinite timeout

    if (count < 0) // shouldn't happen
    {
        LOG_ERROR << __FUNCTION__ << ": failed to poll for event, error: " << strerror(errno) << std::endl;
        return false;
    }
    // no need to check for zero, timeout is infinite

    if (fds[0].revents & POLLIN) // data may be read without blocking
    {
        return true;
    }

    LOG_DEBUG << __FUNCTION__ << ": poll for events was interrupted" << std::endl;
    return false;
}

bool UnixNetworkInterfaceMonitor::ReadAndHandleEvents()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto prevInterfaceNames = m_currentInterfaceNames;

    ParseAndHandleMessagesInBuffer(ReadMessagesToBuffer());

    decltype(m_currentInterfaceNames) interfacesAdded, interfacesRemoved;
    // updated \ original => added
    std::set_difference(m_currentInterfaceNames.cbegin(), m_currentInterfaceNames.cend(),
                        prevInterfaceNames.cbegin(), prevInterfaceNames.cend(),
                        std::inserter(interfacesAdded, interfacesAdded.begin()));
    // original \ updated => removed
    std::set_difference(prevInterfaceNames.cbegin(), prevInterfaceNames.cend(),
                        m_currentInterfaceNames.cbegin(), m_currentInterfaceNames.cend(),
                        std::inserter(interfacesRemoved, interfacesRemoved.begin()));

    // report interface changes, if any
    // assume callbacks are valid, checked before monitor thread creation
    if (!interfacesAdded.empty())
    {
        LOG_INFO << FormatInterfaceNames("Added new interface/s: ", interfacesAdded) << std::endl;
        m_interfaceAddedFunc(m_deviceType, interfacesAdded);
    }
    if (!interfacesRemoved.empty())
    {
        LOG_INFO << FormatInterfaceNames("Removed existing interface/s: ", interfacesRemoved) << std::endl;
        m_interfaceRemovedFunc(m_deviceType, interfacesRemoved);
    }

    return true;
}

int UnixNetworkInterfaceMonitor::ReadMessagesToBuffer()
{
    struct iovec iov = { m_eventsBuffer.data(), m_eventsBuffer.size() };
    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    struct msghdr hdr = { &addr, sizeof(addr), &iov, 1, NULL, 0, 0 };

    int count = recvmsg(m_netlinkSocket, &hdr, 0);
    if (count <= 0)
    {
        LOG_ERROR << __FUNCTION__ << ": Failed to read from netlink socket. Error: " << strerror(errno) << std::endl;
        return count;
    }

    if (addr.nl_pid != 0)
    {
        // ignore non-kernel messages
        LOG_VERBOSE << __FUNCTION__ << ": Ignored non-kernel message" << std::endl;
        return -1;
    }

    LOG_VERBOSE << __FUNCTION__ << ": Received netlink message, length " << count << " Bytes" << std::endl;

    return count;
}

bool UnixNetworkInterfaceMonitor::ParseAndHandleMessagesInBuffer(int count)
{
    if (count <= 0)
    {
        // error was reported, if any
        return false;
    }

    bool allParsed = true;

    // single read from socket may return more than one message
    for (struct nlmsghdr* nh = reinterpret_cast<struct nlmsghdr *>(m_eventsBuffer.data());
        NLMSG_OK(nh, static_cast<unsigned>(count)) && nh->nlmsg_type != NLMSG_DONE;
        nh = NLMSG_NEXT(nh, count))
    {
        // check message relevance
        const char* messageName = GetMessageNameFromRtmType(nh->nlmsg_type);
        if (!messageName)
        {
            LOG_VERBOSE << __FUNCTION__ << ": Unsupported message type " << nh->nlmsg_type << std::endl;
            continue;
        }

        // parse and handle single message
        // do not stop after parse, may have more relevant messages in the buffer (although it's not likely)
        allParsed = ParseAndHandleSingleMessage(nh, messageName) && allParsed;
    }

    return allParsed;
}

bool UnixNetworkInterfaceMonitor::ParseAndHandleSingleMessage(const struct nlmsghdr *nh, const char* messageName)
{
    const struct ifinfomsg* ifi = static_cast<struct ifinfomsg*>(NLMSG_DATA(nh));

    if (nh->nlmsg_len < NLMSG_LENGTH(sizeof(*ifi))) // shouldn't happen
    {
        LOG_ERROR << __FUNCTION__ << ": Got invalid " << messageName << ". Expected length " << NLMSG_LENGTH(sizeof(*ifi))
            << ", actual " << nh->nlmsg_len << std::endl;
        return false;
    }

    if ((ifi->ifi_flags & IFF_LOOPBACK) != 0)
    {
        // ignore loopback interface
        LOG_VERBOSE << __FUNCTION__ << ": Ignore " << messageName << " received for loopback interface" << std::endl;
        return false;
    }

    int len = IFLA_PAYLOAD(nh); // cannot be const, reassigned during iteration
    for (struct rtattr* rta = IFLA_RTA(ifi); RTA_OK(rta, len); rta = RTA_NEXT(rta, len))
    {
        if (rta->rta_type != IFLA_IFNAME)
        {
            // ignore irrelevant attributes
            continue;
        }

        // Notes:
        // 1. During RTM_DELLINK handling the network device can be already removed and so its
        // interface name may be not available through the if_indextoname(ifi->ifi_index, m_ifname) call.
        // Thus we always fetch the interface name directly from IFLA_IFNAME netlink message attribute.
        // 2. If needed, the interface state (up/down) can be tested by (ifi->ifi_flags & IFF_LOWER_UP)
        // 3. The only supported message types are RTM_NEWLINK and RTM_DELLINK
        // 4. rmmod may cause to issue NEWLINK message before DELLINK
        // 5. 'ifconfig wlan0 down' may cause to multiple RTM_NEWLINK messages

        const char* interfaceName = static_cast<const char*>(RTA_DATA(rta));
        const bool isNewLink = nh->nlmsg_type == RTM_NEWLINK ? true : false;

        LOG_VERBOSE << __FUNCTION__ << ": received " << messageName << " message for interface " << interfaceName
            << " (index " << ifi->ifi_index << ")" << std::endl;

        HandleSingleMessage(interfaceName, isNewLink);

        // other attributes are not required, may return
        return true;
    }

    LOG_ERROR << __FUNCTION__ << ": Failed to find IFLA_IFNAME attribute for message " << messageName << std::endl;
    return false;
}

void UnixNetworkInterfaceMonitor::HandleSingleMessage(const char* interfaceName, bool isNewLink)
{
    // cannot check interface relevance during rmmod for both RTM_NEWLINK and RTM_DELLINK events
    // therefore ignore RTM_NEWLINK in this case and mark interface as removed upon RTM_DELLINK without testing relevance
    if (isNewLink && !IsInterfaceRelevantOrRelevanceUnknown(interfaceName))
    {
        LOG_DEBUG << "RTM_NEWLINK was ignored for for interface " << interfaceName << std::endl;
        return;
    }

    if (isNewLink)
    {
        m_currentInterfaceNames.emplace(interfaceName);
    }
    else
    {
        m_currentInterfaceNames.erase(interfaceName);
    }
}

// === Enumeration functionality === //
// Method:
// - check folder exists : /sys/module/wil6210/drivers/pci:wil6210/ (symbolic link to ../../../bus/pci/drivers/wil6210 folder)
// - ls and check for file having ":" (only PCI endpoint has ":" in its name, e.g. 0000:02:00.0)
//  - if not found, there is no wigig interface, return
// - get interface name from endpoint
//  - check folder exists : /sys/module/wil6210/drivers/pci:wil6210/_pciEndPoint_/net (e.g. /sys/module/wil6210/drivers/pci:wil6210/0000:02:00.0/net)
//    (this folder should contain '_iname_' folder for specific endpoint, e.g. /sys/module/wil6210/drivers/pci:wil6210/0000:02:00.0/net/wlan0)
//  - ls and find the first folder having NO "-" in the name, i.e.a physical interface => this is the iname
//  - if physical interface not found, have first one found(having "-" in the name) => this is the iname
//
// \remarks Should be called after monitor thread creation to prevent miss of interface changes.
//          Assumes lock is already acquired.
std::set<std::string> UnixNetworkInterfaceMonitor::InitializeCurrentInterfaceNames()
{
    m_currentInterfaceNames.clear();

    DIR* dp = opendir(PCI_END_POINT_FOLDER);
    if (!dp)
    {
        LOG_VERBOSE << "Failed to open PCI EP directories" << std::endl;
        return m_currentInterfaceNames;
    }

    dirent* de = nullptr;
    while ((de = readdir(dp)) != nullptr) // iterate through directory content and search for End Point folders
    {
        const std::string potentialPciEndPoint(de->d_name);
        if (potentialPciEndPoint.find(":") != std::string::npos) // only PCI end point has ":" in its name
        {
            // Get interface name from End Point
            std::string networkInterfaceName = GetInterfaceNameFromEndPoint(potentialPciEndPoint);
            if (!networkInterfaceName.empty())
            {
                m_currentInterfaceNames.insert(std::move(networkInterfaceName));
            }
            else
            {
                LOG_VERBOSE << "Failed to find interface name for EP: " << potentialPciEndPoint << std::endl;
            }
        }
    }

    closedir(dp);

    LOG_INFO << FormatInterfaceNames("Discovered WIGIG interface/s: ", m_currentInterfaceNames) << std::endl;

    return m_currentInterfaceNames;
}

std::set<std::string> UnixNetworkInterfaceMonitor::GetActiveInterfaces() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentInterfaceNames;
}
