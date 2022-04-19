/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
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
#include <vector>
#include <cstdint>
#include <bitset>

#include "Utils.h" // for DWORD
#include "OperationStatus.h"

#define BAUD_RATE_REGISTER 0x880050
#define DEVICE_NAME_DELIMITER '!'

enum class DeviceType
{
    PCI,
    JTAG,
    DUMMY,
    RTL,
};

// enumeration for driver capabilities, keep same type name defined in the driver
enum class wil_nl_60g_driver_capa
{
    NL_60G_DRIVER_CAPA_WMI = 0,     // WMI transport to/from the Operational mailbox
    NL_60G_DRIVER_CAPA_FW_STATE,    // notifications of FW health state changes
    NL_60G_DRIVER_CAPA_IOCTL_WRITE, // ioctl to write to the device address space
};

class FwStateProvider; // forward declaration

inline std::ostream& operator<<(std::ostream& os, const DeviceType deviceType)
{
    switch (deviceType)
    {
    case DeviceType::PCI:
        return os << "PCI";
    case DeviceType::JTAG:
        return os << "JTAG";
    case DeviceType::DUMMY:
        return os << "TEST"; // align with corresponding enumeration code
    case DeviceType::RTL:
        return os << "RTL";
    default:
        return os << "Unknown type";
    }
}

class DriverAPI
{
public:
    DriverAPI(DeviceType deviceType, std::string interfaceName, bool isMonitored = false):
        m_deviceType(deviceType),
        m_interfaceName(std::move(interfaceName)),
        m_capabilityMask(0U),
        m_isMonitored(isMonitored)
    {}

    virtual ~DriverAPI() = default;

    // Base access functions (to be implemented by specific device)
    virtual bool Read(DWORD /*address*/, DWORD& /*value*/) { return false;  }
    virtual bool ReadBlock(DWORD /*addr*/, DWORD /*blockSizeInDwords*/, std::vector<DWORD>& /*values*/) { return false; }
    virtual bool ReadBlock(DWORD /*addr*/, DWORD /*blockSizeInBytes*/, char * /*arrBlock*/) { return false; }
    virtual bool ReadRadarData(DWORD /*maxBlockSizeInDwords*/, std::vector<DWORD>& /*values*/) { return false; }
    virtual bool Write(DWORD /*address*/, DWORD /*value*/) { return false; }
    virtual bool WriteBlock(DWORD /*addr*/, std::vector<DWORD> /*values*/) { return false; };
    virtual bool WriteBlock(DWORD /*address*/, DWORD /*blockSizeInBytes*/, const char* /*valuesToWrite*/) { return false; }

    // PMC functions
    virtual bool AllocPmc(unsigned /*descSize*/, unsigned /*descNum*/, std::string& /*outMessage*/) { return false; }
    virtual bool DeallocPmc(bool /*bSuppressError*/, std::string& /*outMessage*/) { return false; }
    virtual bool CreatePmcFiles(unsigned /*refNumber*/, std::string& /*outMessage*/) { return false; }
    virtual bool FindPmcDataFile(unsigned /*refNumber*/, std::string& /*outMessage*/) { return false; }
    virtual bool FindPmcRingFile(unsigned /*refNumber*/, std::string& /*outMessage*/) { return false; }

    virtual bool IsOpen() { return false; }
    virtual bool Open() { return false;  }
    virtual bool DriverControl(uint32_t /*id*/, const void* /*inBuf*/, uint32_t /*inBufSize*/,
                               void* /*outBuf*/, uint32_t /*outBufSize*/, DWORD* /*pLastError*/ = nullptr) { return false; }
    virtual void Close() {}

    virtual bool GetDriverMode(int& /*currentState*/) { return false; }
    virtual bool SetDriverMode(int /*newState*/, int& /*oldState*/) { return false; }

    virtual bool InitializeTransports() { return true; } // default is true to force polling, just as legacy implementation did
    virtual void RegisterDriverControlEvents(FwStateProvider* /*fwStateProvider*/) { }

    virtual OperationStatus InterfaceReset() { return OperationStatus(false, "Interface Reset is not supported on the current platform"); }

    const std::string& GetInterfaceName() const { return m_interfaceName; }

    virtual bool IsValid()
    {
        DWORD value;
        if (!Read(BAUD_RATE_REGISTER, value))
        {
            return false;
        }
        return IsValidInternal();
    }

    uint32_t GetCapabilityMask() const { return static_cast<uint32_t>(m_capabilityMask.to_ulong()); }

    bool IsCapabilitySet(wil_nl_60g_driver_capa capability) const
    {
        return m_capabilityMask.test(static_cast<size_t>(capability));
    }

    bool IsMonitored() const { return m_isMonitored; }

    std::string BuildDeviceName() const
    {
        return BuildDeviceName(m_deviceType, m_interfaceName);
    }

    static std::string BuildDeviceName(DeviceType deviceType, const std::string& interfaceName)
    {
        std::stringstream ss;
        ss << deviceType << DEVICE_NAME_DELIMITER << interfaceName;
        return ss.str();
    }

protected:
    const DeviceType m_deviceType;
    const std::string m_interfaceName;
    std::bitset<32> m_capabilityMask; // driver capability mask according to wil_nl_60g_driver_capa enumeration
    const bool m_isMonitored;         // marks if related interface is monitored to be ignored during periodic enumeration

private:
    virtual bool IsValidInternal() { return true; }
};
