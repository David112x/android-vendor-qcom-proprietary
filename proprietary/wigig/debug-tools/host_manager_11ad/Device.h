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
#include <vector>
#include <memory>
#include <mutex>
#include <map>

#include "LogCollector/LogReader.h"
#include "HostManagerDefinitions.h"
#include "Utils.h"

struct FwVersion;
struct FwTimestamp;
struct StringNameValuePair;
class DriverAPI;
class FieldDescription;
class FwStateProvider;

class Device
{
public:
    explicit Device(const std::string& deviceName);

    explicit Device(std::unique_ptr<DriverAPI>&& deviceDriver);

    // cannot be inline because incompleteness of FieldDescription and other types
    ~Device();

    bool IsValid() const { return m_driver != nullptr; }

    const std::string& GetDeviceName() const { return m_deviceName; }

    std::string GetInterfaceName() const;

    // ************************** [END] Device API **********************//

    BasebandType GetBasebandType() const { return m_basebandType; }

    BasebandRevision GetBasebandRevision() const { return m_basebandRevision; }

    bool GetSilenceMode() const { return m_isSilent; }

    void SetSilenceMode(bool silentMode) { m_isSilent = silentMode; }

    DriverAPI* GetDriver() const { return m_driver.get(); }

    const FwStateProvider* GetFwStateProvider() const { return m_fwStateProvider.get(); }

    //TODO: make private after blocking simultaneous driver actions
    mutable std::mutex m_mutex; // threads synchronization

    // \remark When uCode timestamp address is zero, an invalidated timestamp will be returned (backward compatibility)
    bool ReadDeviceFwInfo(FwVersion& fwVersion, FwTimestamp& fwTimestamp,
                          FwTimestamp& uCodeTimestamp, uint32_t uCodeTimestampStartAddress) const;
    bool ReadFwUcodeErrors(uint32_t& fwError, uint32_t& uCodeError) const;
    bool ReadFwPointerTable(uint32_t& rfcConnectedValue, uint32_t& rfcEnabledValue, uint32_t& uCodeTimestampStartAddress) const;


    // ************************ Custom Regs *************************//
    bool AddCustomRegister(const std::string& name, uint32_t address, uint32_t firstBit, uint32_t lastBit);

    bool RemoveCustomRegister(const std::string& name);

    // Read values for all defined custom registers
    // Returns true when read operation succeeded for all registers
    bool ReadCustomRegisters(std::vector<StringNameValuePair>& customRegisters) const;
    // *********************** [END] Custom Regs *******************//

    // invalid address constant to be used during memory access
    static const uint32_t sc_invalidAddress = 0U;

private:
    BasebandType m_basebandType;
    BasebandRevision m_basebandRevision;
    std::unique_ptr<DriverAPI> m_driver;
    std::unique_ptr<FwStateProvider> m_fwStateProvider;
    bool m_isSilent;
    std::string m_deviceName;

    void InitializeBasebandRevision();

    // Custom registers
    std::map<std::string, std::unique_ptr<FieldDescription>> m_customRegistersMap;

    void InitializeDriver();

    DWORD FW_ERROR_REGISTER;
    DWORD UCODE_ERROR_REGISTER;
};
