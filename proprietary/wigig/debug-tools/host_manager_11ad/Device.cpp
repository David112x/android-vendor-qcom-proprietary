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

#include <map>

#include "Device.h"
#include "DriverAPI.h"
#include "FwStateProvider.h"
#include "EventsDefinitions.h"
#include "DebugLogger.h"
#include "DriversFactory.h"
#include "DeviceManager.h"
#include "TaskManager.h"
#include "FieldDescription.h"
#include "HostAndDeviceDataDefinitions.h"
#include "Utils.h"

using namespace std;

namespace
{
    enum FwInfoRegisters : DWORD
    {
        FW_VERSION_MAJOR_REGISTER = 0x880a2c,
        FW_VERSION_MINOR_REGISTER = 0x880a30,
        FW_VERSION_SUB_MINOR_REGISTER = 0x880a34,
        FW_VERSION_BUILD_REGISTER = 0x880a38,
        FW_TIMESTAMP_HOUR_REGISTER = 0x880a14,
        FW_TIMESTAMP_MINUTE_REGISTER = 0x880a18,
        FW_TIMESTAMP_SECOND_REGISTER = 0x880a1c,
        FW_TIMESTAMP_DAY_REGISTER = 0x880a20,
        FW_TIMESTAMP_MONTH_REGISTER = 0x880a24,
        FW_TIMESTAMP_YEAR_REGISTER = 0x880a28,
    };

    enum FwStateRegisters : DWORD
    {
        FW_ERROR_REGISTER_SPARROW = 0x91F020,
        UCODE_ERROR_REGISTER_SPARROW = 0x91F028,
        FW_ERROR_REGISTER_TALYN = 0xA37020,
        UCODE_ERROR_REGISTER_TALYN = 0xA37028,
    };

    enum FwPointerTableInfo : DWORD
    {
        DYNAMIC_ADDRESSING_PATTERN_ADDRESS = 0x880A80,
        DYNAMIC_ADDRESSING_PATTERN = 0xBACACAFE,
        POINTER_TABLE_ADDRESS = 0x880A84,
        RFC_CONNECTED_POINTER_OFFSET = 16,
        RFC_ENABLED_POINTER_OFFSET = 20,
        UCODE_TIMESTAMP_POINTER_OFFSET = 24
    };
}

// TODO this version should be removed once all Enumerations return Driver instead of interface name
//      (for this reason it's forgivable to "loss" device name and then rebuild it)
Device::Device(const string& deviceName) : Device(DriversFactory::OpenDriver(deviceName))
{}

Device::Device(std::unique_ptr<DriverAPI>&& deviceDriver) :
    m_basebandType(BASEBAND_TYPE_NONE),
    m_basebandRevision(BB_REV_UNKNOWN),
    m_driver(std::move(deviceDriver)),
    m_isSilent(false),
    m_deviceName(m_driver->BuildDeviceName())
{
    InitializeBasebandRevision();
    if (m_basebandType == BASEBAND_TYPE_TALYN)
    {
        LOG_INFO << "On Device " << m_deviceName << " Baseband is TALYN" << std::endl;
        FW_ERROR_REGISTER = FW_ERROR_REGISTER_TALYN;
        UCODE_ERROR_REGISTER = UCODE_ERROR_REGISTER_TALYN;
    }
    else
    {
        LOG_INFO << "On Device " << m_deviceName << " Baseband is SPARROW" << std::endl;
        FW_ERROR_REGISTER = FW_ERROR_REGISTER_SPARROW;
        UCODE_ERROR_REGISTER = UCODE_ERROR_REGISTER_SPARROW;
    }

    LOG_INFO << "On Device " << m_deviceName << " Baseband Revision: " << m_basebandRevision << std::endl;

    // should be done after initialization of error registers
    InitializeDriver();
}

// cannot be inline because incompleteness of FieldDescription
Device::~Device() = default;

std::string Device::GetInterfaceName(void) const
{
    return m_driver->GetInterfaceName();
}

void Device::InitializeBasebandRevision()
{
    LOG_DEBUG << "Reading BB revision" << std::endl;

    const uint32_t RGF_USER_JTAG_DEV_ID = 0x00880b34; // Device ID (USER.JTAG.USER_USER_JTAG_1.dft_idcode_dev_id)

    const uint32_t JTAG_DEV_ID_SPARROW = 0x2632072f;
    const uint32_t JTAG_DEV_ID_TALYN_MA = 0x7e0e1;
    const uint32_t JTAG_DEV_ID_TALYN_MBC = 0x1007e0e1;

    // Revision ID allows to differentiate between Sparrow_M_B0 and Sparrow_M_D0
    const uint32_t RGF_USER_REVISION_ID = (0x88afe4);
    const uint32_t RGF_USER_REVISION_ID_MASK = (3);
    const uint32_t REVISION_ID_SPARROW_B0 = (0x0);
    const uint32_t REVISION_ID_SPARROW_D0 = (0x3);

    // Debug ID allows to differentiate between Talyn_M_B0 and Talyn_M_C0
    const uint32_t RGF_POWER_DEBUG_ID = (0x883f6c); /* PHY_RX_RGF.POWER_DEBUG.power_dbg_dout0 */
    const uint32_t DEBUG_ID_TALYN_B0 = (0x0);
    const uint32_t DEBUG_ID_TALYN_C0 = (0x1);

    m_basebandType = BASEBAND_TYPE_NONE;
    m_basebandRevision = BB_REV_UNKNOWN;

    DWORD jtagId = Utils::REGISTER_DEFAULT_VALUE;

    if (!m_driver->Read(RGF_USER_JTAG_DEV_ID, jtagId))
    {
        LOG_ERROR << "Cannot determine device revision. Failed to read JTAG ID: "
            << Address(RGF_USER_JTAG_DEV_ID) << std::endl;
        return;
    }

    LOG_DEBUG << "Obtained JTAG ID: " << AddressValue(RGF_USER_JTAG_DEV_ID, jtagId) << std::endl;

    switch (jtagId)
    {
    case JTAG_DEV_ID_SPARROW:
        m_basebandType = BASEBAND_TYPE_SPARROW;

        DWORD chipRevision;
        if (!m_driver->Read(RGF_USER_REVISION_ID, chipRevision))
        {
            LOG_ERROR << "Cannot determine device revision. Failed to read RGF_USER_REVISION_ID: "
                << Address(RGF_USER_REVISION_ID) << std::endl;
        }
        chipRevision &= RGF_USER_REVISION_ID_MASK;

        switch (chipRevision)
        {
        case REVISION_ID_SPARROW_D0:
            m_basebandRevision = SPR_D0;
            break;
        case REVISION_ID_SPARROW_B0:
            m_basebandRevision = SPR_B0;
            break;
        default:
            LOG_ERROR << "Unsupported SPR chip revision: " << AddressValue(RGF_USER_REVISION_ID, chipRevision) << std::endl;
            break;
        }
        break;

    case JTAG_DEV_ID_TALYN_MA:
        m_basebandType = BASEBAND_TYPE_TALYN;
        m_basebandRevision = TLN_M_A0;
        break;

    case JTAG_DEV_ID_TALYN_MBC:
        m_basebandType = BASEBAND_TYPE_TALYN;

        DWORD debugRevision;
        if (!m_driver->Read(RGF_POWER_DEBUG_ID, debugRevision))
        {
            LOG_ERROR << "Cannot determine device revision. Failed to read RGF_POWER_DEBUG_ID: "
                << Address(RGF_POWER_DEBUG_ID) << std::endl;
            return;
        }

        switch (debugRevision)
        {
        case DEBUG_ID_TALYN_B0:
            m_basebandRevision = TLN_M_B0;
            break;
        case DEBUG_ID_TALYN_C0:
            m_basebandRevision = TLN_M_C0;
            break;
        default:
            LOG_ERROR << "Unsupported TLN chip revision: " << AddressValue(RGF_POWER_DEBUG_ID, debugRevision) << std::endl;
            break;
        }
        break;

    default:
        LOG_ERROR << "Unsupported JTAG ID: " << AddressValue(RGF_USER_JTAG_DEV_ID, jtagId) << std::endl;
        break;
    }

    LOG_INFO << "Baseband type: " << m_basebandType << " Baseband revision: " << m_basebandRevision << std::endl;
}

void Device::InitializeDriver()
{
    // always create FwStateProvider to prevent potential null pointer access
    // if driver is not ready, it will be created without polling
    const bool isPollingRequired = m_driver->InitializeTransports()
        && !m_driver->IsCapabilitySet(wil_nl_60g_driver_capa::NL_60G_DRIVER_CAPA_FW_STATE);

    m_fwStateProvider.reset(new FwStateProvider(*this, isPollingRequired));

    // internally tests if driver is ready
    m_driver->RegisterDriverControlEvents(m_fwStateProvider.get());

    const bool isOperationalMailbox = m_driver->IsCapabilitySet(wil_nl_60g_driver_capa::NL_60G_DRIVER_CAPA_WMI);
    LOG_INFO << "Device " << m_deviceName << " is using " << (isOperationalMailbox ? "Operational" : "Debug") << " mailbox" << endl;
}

// Provided mask is assumed to be in range [0,31]
bool Device::AddCustomRegister(const string& name, uint32_t address, uint32_t firstBit, uint32_t lastBit)
{
    if (m_customRegistersMap.find(name) != m_customRegistersMap.end())
    {
        // Register already exists
        return false;
    }

    m_customRegistersMap[name].reset(new FieldDescription(address, firstBit, lastBit));
    return true;
}

bool Device::RemoveCustomRegister(const string& name)
{
    if (m_customRegistersMap.find(name) == m_customRegistersMap.end())
    {
        // Register already does not exist
        return false;
    }

    m_customRegistersMap.erase(name);

    return true;
}

// Read values for all defined custom registers
// Returns true when read operation succeeded for all registers
bool Device::ReadCustomRegisters(std::vector<StringNameValuePair>& customRegisters) const
{
    customRegisters.clear(); // capacity unchanged
    customRegisters.reserve(m_customRegistersMap.size());

    bool ok = true;
    for (const auto& reg : m_customRegistersMap)
    {
        DWORD value = Utils::REGISTER_DEFAULT_VALUE;
        ok &= m_driver->Read(reg.second->GetAddress(), value);
        std::ostringstream oss;
        oss << reg.second->MaskValue(value);

        customRegisters.push_back({reg.first, oss.str()});
    }

    return ok;
}

// Internal service for fetching the FW version and compile times
bool Device::ReadDeviceFwInfo(FwVersion& fwVersion, FwTimestamp& fwTimestamp,
                              FwTimestamp& uCodeTimestamp, uint32_t uCodeTimestampStartAddress) const
{
    lock_guard<mutex> lock(m_mutex);
    if (GetSilenceMode())
    {
        return false;
    }

    // FW version
    bool readOk = m_driver->Read(FW_VERSION_MAJOR_REGISTER, fwVersion.m_major);
    readOk &= m_driver->Read(FW_VERSION_MINOR_REGISTER, fwVersion.m_minor);
    readOk &= m_driver->Read(FW_VERSION_SUB_MINOR_REGISTER, fwVersion.m_subMinor);
    readOk &= m_driver->Read(FW_VERSION_BUILD_REGISTER, fwVersion.m_build);

    // FW compile time
    readOk &= m_driver->Read(FW_TIMESTAMP_HOUR_REGISTER, fwTimestamp.m_hour);
    readOk &= m_driver->Read(FW_TIMESTAMP_MINUTE_REGISTER, fwTimestamp.m_min);
    readOk &= m_driver->Read(FW_TIMESTAMP_SECOND_REGISTER, fwTimestamp.m_sec);
    readOk &= m_driver->Read(FW_TIMESTAMP_DAY_REGISTER, fwTimestamp.m_day);
    readOk &= m_driver->Read(FW_TIMESTAMP_MONTH_REGISTER, fwTimestamp.m_month);
    readOk &= m_driver->Read(FW_TIMESTAMP_YEAR_REGISTER, fwTimestamp.m_year);

    // uCode compile times
    if (uCodeTimestampStartAddress == sc_invalidAddress)
    {
        uCodeTimestamp = FwTimestamp::CreateInvalidTimestamp();
    }
    else
    {
        // uCode timestamp is stored as 6 Bytes starting from uCodeTimestampStartAddress
        DWORD value;
        readOk &= m_driver->Read(uCodeTimestampStartAddress, value);
        if (value == 0U)
        {
            // when uCode timestamp is not properly updated (can happen for WMI_ONLY FW) it should be ignored
            // newer WMI_ONLY FW can update the timestamp, so cannot decided base on FW flavor
            uCodeTimestamp = FwTimestamp::CreateInvalidTimestamp();
        }
        else
        {
            uCodeTimestamp.m_sec = FieldDescription::MaskValue(value, 0, 7);
            uCodeTimestamp.m_min = FieldDescription::MaskValue(value, 8, 15);
            uCodeTimestamp.m_hour = FieldDescription::MaskValue(value, 16, 23);
            uCodeTimestamp.m_day = FieldDescription::MaskValue(value, 24, 31);

            readOk &= m_driver->Read(uCodeTimestampStartAddress + sizeof(uint32_t), value);
            uCodeTimestamp.m_month = FieldDescription::MaskValue(value, 0, 7);
            uCodeTimestamp.m_year = FieldDescription::MaskValue(value, 8, 15);
        }
    }

    if (!readOk)
    {
        LOG_ERROR << "Failed to read FW info for device " << GetDeviceName() << endl;
    }

    return readOk;
}


bool Device::ReadFwUcodeErrors(uint32_t& fwError, uint32_t& uCodeError) const
{
    lock_guard<mutex> lock(m_mutex);
    if (GetSilenceMode())
    {
        return false;
    }

    bool readOk = m_driver->Read(FW_ERROR_REGISTER, reinterpret_cast<DWORD&>(fwError));
    readOk &= m_driver->Read(UCODE_ERROR_REGISTER, reinterpret_cast<DWORD&>(uCodeError));
    if (!readOk)
    {
        LOG_ERROR << "Failed to read FW state for device " << GetDeviceName() << endl;
    }

    return readOk;
}

// Read FW pointer table and initialize relevant values/addresses
// Pointer table is defined by the following structure:
// typedef struct
// {
//    U32 version;
//    U32 operational_mbox_base_address;
//    U32 debug_mbox_base_address;
//    U32 dashboard_base_address;
// #if (PRODUCTION_RF==TRUE)
//    U32 rf_production_api_base_address;
// #endif
//    U32 p_rfc_connected;                // from ver 1, Linker addr. of FW.g_rfc_connected_indx_vec which is rf_connected, FW DCCM
//    U32 p_rfc_enabled;                  // from ver 1, Linker addr. of FW.g_marlon_r_if.marlon_r_if_class.m_rfc_enabled, FW DCCM
//    ucode_timestamp_s *ucode_timestamp; // from ver 2, Linker addr. of g_shared_mem_ipc.ucode_timestamp, Peripheral Memory
//                                        // (UCODE.g_shared_mem_ipc.shared_mem_ipc_s.shared_mem_ipc_s.ucode_timestamp.ucode_timestamp_s, 6 Bytes)
// } fw_pointer_table_s;
bool Device::ReadFwPointerTable(uint32_t& rfcConnectedValue, uint32_t& rfcEnabledValue, uint32_t& uCodeTimestampStartAddress) const
{
    // initialize out params to defaults, will be overriden when possible
    rfcConnectedValue = rfcEnabledValue = 0U;
    uCodeTimestampStartAddress = sc_invalidAddress;

    lock_guard<mutex> lock(m_mutex);
    if (GetSilenceMode())
    {
        return false;
    }

    DWORD pattern = 0U;
    bool readOk = m_driver->Read(DYNAMIC_ADDRESSING_PATTERN_ADDRESS, pattern);
    if (!readOk)
    {
        LOG_ERROR << "On Device " << m_deviceName << ", failed to read dynamic addressing pattern" << endl;
        return false;
    }
    if (pattern != DYNAMIC_ADDRESSING_PATTERN)
    {
        // old FW before dynamic addressing
        LOG_DEBUG << "On Device " << m_deviceName << ", there is no FW pointer table" << endl;
        return false;
    }

    DWORD pointerTableBaseAddress = sc_invalidAddress;
    readOk &= m_driver->Read(POINTER_TABLE_ADDRESS, pointerTableBaseAddress);
    DWORD tableVersion = 0U;
    readOk &= m_driver->Read(pointerTableBaseAddress, tableVersion);
    if (!readOk)
    {
        LOG_ERROR << "On Device " << m_deviceName << ", failed to read FW pointer table version" << endl;
        return false;
    }

    LOG_DEBUG << "On Device " << m_deviceName << ", FW pointer table version is " << Hex<2>(tableVersion) << endl;

    if (tableVersion >= 1U)
    {
        // read rfc_connected & rfc_enabled
        DWORD rfcConnectedAddress = sc_invalidAddress;
        readOk &= m_driver->Read(pointerTableBaseAddress + RFC_CONNECTED_POINTER_OFFSET, rfcConnectedAddress);
        if (rfcConnectedAddress != sc_invalidAddress) // may be zero in 10.1, this case stay with default value
        {
            readOk &= m_driver->Read(rfcConnectedAddress, reinterpret_cast<DWORD&>(rfcConnectedValue));
        }
        DWORD rfcEnabledAddress = sc_invalidAddress;
        readOk &= m_driver->Read(pointerTableBaseAddress + RFC_ENABLED_POINTER_OFFSET, rfcEnabledAddress);
        if (rfcEnabledAddress != sc_invalidAddress) // may be zero in 10.1, this case stay with default value
        {
            readOk &= m_driver->Read(rfcEnabledAddress, reinterpret_cast<DWORD&>(rfcEnabledValue));
        }
    }

    if (tableVersion >= 2U)
    {
        // read uCode timestamp address
        readOk &= m_driver->Read(pointerTableBaseAddress + UCODE_TIMESTAMP_POINTER_OFFSET, reinterpret_cast<DWORD&>(uCodeTimestampStartAddress));
    }

    if (!readOk)
    {
        LOG_ERROR << "On Device " << m_deviceName << ", failed to read FW pointer table" << endl;
    }

    return readOk;
}
