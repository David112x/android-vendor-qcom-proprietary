/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <string>
#include <vector>

#include "FwInfoDefinitions.h"

/*
 * Definitions required to provide Host and its Devices description to the on-target-UI application through HTTP server
 */

struct StringNameValuePair
{
    std::string m_name;
    std::string m_value;
};

struct DeviceData
{
    std::string m_deviceName;
    bool m_associated = false;
    int m_signalStrength = 0;
    int m_fwAssert = 0;
    int m_uCodeAssert = 0;
    int m_mcs = 0;
    int m_channel = 0;
    FwVersion m_fwVersion = {};
    std::string m_bootloaderVersion;
    std::string m_mode;
    FwTimestamp m_compilationTime = {};
    std::string m_hwType;
    std::string m_hwTemp;
    std::string m_rfType;
    std::string m_rfTemp;
    std::string m_boardFile;
    std::vector<int> m_rf;
    std::vector<StringNameValuePair> m_fixedRegisters;
    std::vector<StringNameValuePair> m_customRegisters;
};

struct HostData
{
    std::string m_hostManagerVersion; //should it be a structure like FwVersion??
    std::string m_hostName;
    std::string m_hostIP;
    std::vector<DeviceData> m_devices;
};
