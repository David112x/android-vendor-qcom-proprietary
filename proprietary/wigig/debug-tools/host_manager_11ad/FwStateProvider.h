/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <mutex>
#include "FwInfoDefinitions.h"

// Enumeration of FW health states as defined by wil6210 driver
enum class wil_fw_state
{
    WIL_FW_STATE_UNKNOWN = 0,           // uninitialized (also may mark debug_fw)
    WIL_FW_STATE_DOWN,                  // FW not loaded or not ready yet
    WIL_FW_STATE_READY,                 // FW is ready
    WIL_FW_STATE_ERROR_BEFORE_READY,    // Detected FW error before FW sent ready indication
    WIL_FW_STATE_ERROR,                 // Detected FW error after FW sent ready indication
    WIL_FW_STATE_UNRESPONSIVE           // FW operation error, does not exist in driver enumeration
};

std::ostream& operator<<(std::ostream &os, wil_fw_state fwHealthState);

class Device; // forward declaration

class FwStateProvider final
{
public:
    FwStateProvider(const Device& device, bool pollingRequired);
    ~FwStateProvider();

    FwVersion GetFwVersion() const;
    FwTimestamp GetFwTimestamp() const;
    FwTimestamp GetUcodeTimestamp() const;
    wil_fw_state GetFwHealthState() const;

    void OnFwHealthStateChanged(wil_fw_state fwHealthState);
    bool IsInitialized() const;

private:
    mutable std::mutex m_mutex; // threads synchronization
    const Device& m_device;
    const bool m_pollingRequired;

    FwVersion m_fwVersion;
    FwTimestamp m_fwTimestamp;
    FwTimestamp m_uCodeTimestamp;
    uint32_t m_fwError;
    uint32_t m_uCodeError;
    uint32_t m_uCodeTimestampStartAddress;
    uint32_t m_rfcConnectedValue; // zero is a valid value, uninitialized will be marked as MAX_UINT
    uint32_t m_rfcEnabledValue;
    wil_fw_state m_fwHealthState;

    unsigned m_fwInfoPollerTaskId;
    unsigned m_fwUcodeErrorsPollerTaskId;

    void HandleDeviceInitialization();
    void ReadFwPointerTableInternal();
    void PollDeviceFwInfo();
    void ReadDeviceFwInfoInternal();
    void PollFwUcodeErrors();
    void ReadFwUcodeErrorsInternal();
};
