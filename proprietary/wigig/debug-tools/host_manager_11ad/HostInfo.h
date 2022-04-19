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

#pragma once

#include <string>
#include <mutex>
#include <set>
#include <bitset>

#include "Utils.h"
#include "HostDefinitions.h"

class HostInfo
{

public:

    /*
    HostInfo
    Initializes all hostInfo members (ips and alias)
    */
    HostInfo();

    /*
    GetIps
    Returns host's Ips (private and broadcast)
    @param: none
    @return: host's ips
    */
    const HostIps& GetIps() const;

    /*
    GetHostName
    Returns host name
    @param: none
    @return: host's alias if exists, otherwise empty string
    */
    std::string GetHostName();

    /*
    GetConnectedUsers
    Returns a set of all the users that are connected to this host
    @param: none
    @return: set<string> of all connected users
    */
    std::set<std::string> GetConnectedUsers() const;

    /*
    AddNewConnectedUser
    Adds a user to the connected user's list
    @param: string - a new user (currently the user's DmTools's IP. TODO: change to the user's personal host's name or user's DmTools username)
    @return: none
    */
    void AddNewConnectedUser(const std::string& user);

    /*
    RemoveConnectedUser
    Removes a user from the connected user's list
    @param: string - a user (currently the user's DmTools's IP. TODO: change to the user's personal host's name or user's DmTools username)
    @return: none
    */
    void RemoveConnectedUser(const std::string& user);

    static std::string GetOSName() { return s_osName; }

    /*
    GetHostCapabilities
    Get the capabilities of the host
    @param: none
    @return: bit mask of the capabilities
    */
    uint32_t GetHostCapabilities() const { return static_cast<uint32_t>(m_capabilityMask.to_ulong()); }

private:
    HostIps m_ips; // host's network details // assumption: each host has only one IP address for Ethernet interfaces
    std::set<std::string> m_connectedUsers; // list of users IPs that have a connection to the commandsTcpServer // TODO: change to the user's personal host's name or user's DmTools username
    const static std::string s_osName;  // current OS name
    mutable std::mutex m_connectedUsersLock;
    std::bitset<32> m_capabilityMask;   // host capability mask according to CAPABILITIES enumeration

    void SetHostCapabilities();

    // create all directories required for host manager
    void CreateHostDirectories();
};
