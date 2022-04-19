/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

class DriverAPI;

namespace DriversFactory
{
    std::set<std::string> GetNonMonitoredDrivers();
    std::vector<std::unique_ptr<DriverAPI>> StartNetworkInterfaceMonitors();
    std::unique_ptr<DriverAPI> OpenDriver(const std::string& deviceName);
};
