/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <memory>
#include <set>
#include <string>
#include <map> // for ConvertDeviceTypeName

#include "DriversFactory.h"
#include "Utils.h"
#include "DummyDriver.h"
#include "DebugLogger.h"

#ifdef _WINDOWS
    #include "WindowsPciDriver.h"
    #include "JTagDriver.h"
#else
    #include "UnixPciDriver.h"
    #include "UnixNetworkInterfaceMonitor.h"
    #include "UnixRtlDriver.h"
    #include "RtlSimEnumerator.h"

    // TODO: remove once DeviceManager callbacks are stored as members
    #include "Host.h"
    #include "DeviceManager.h"
#endif // _WINDOWS


using namespace std;

class DriverAPI;

namespace
{
    // local helper to create relevant driver instance according to the provided device type
    std::unique_ptr<DriverAPI> CreateDriver(DeviceType deviceType, const std::string& interfaceName)
    {
        switch (deviceType)
        {
#ifdef _WINDOWS
        case DeviceType::JTAG:
            return std::unique_ptr<JTagDriver>(new JTagDriver(interfaceName));
#endif // WINDOWS
        case DeviceType::DUMMY:
            return std::unique_ptr<DummyDriver>(new DummyDriver(interfaceName));
        case DeviceType::PCI:
#ifdef _WINDOWS
            return std::unique_ptr<WindowsDriverAPI>(new WindowsDriverAPI(interfaceName));
#else
            return std::unique_ptr<UnixPciDriver>(new UnixPciDriver(interfaceName));
        case DeviceType::RTL:
            return std::unique_ptr<UnixRtlDriver>(new UnixRtlDriver(interfaceName));
#endif // WINDOWS

        default:
            LOG_ERROR << "Got invalid device type. Return an empty driver" << std::endl;
            return std::unique_ptr<DriverAPI>(new DriverAPI(DeviceType::DUMMY, interfaceName));
        }
    }

#ifdef __linux
    std::vector<std::unique_ptr<DriverAPI>> TryCreateDriversForInterfaces(DeviceType deviceType, const std::set<std::string>& interfaceNames)
    {
        std::vector<std::unique_ptr<DriverAPI>> drivers;
        drivers.reserve(interfaceNames.size());

        for (const auto& interfaceName : interfaceNames)
        {
            // try create driver
            auto deviceDriver = CreateDriver(deviceType, interfaceName);
            if (deviceDriver && deviceDriver->IsValid()) // ready and can read
            {
                drivers.push_back(std::move(deviceDriver));
            }
        }

        return drivers;
    }

    void OnInterfacesAdded(int deviceTypeInt, const std::set<std::string>& interfacesAdded)
    {
        DeviceType deviceType = static_cast<DeviceType>(deviceTypeInt);
        std::vector<std::unique_ptr<DriverAPI>> drivers = TryCreateDriversForInterfaces(deviceType, interfacesAdded);

        Host::GetHost().GetDeviceManager().OnInterfacesAdded(drivers);
    }
    void OnInterfacesRemoved(int deviceTypeInt, const std::set<std::string>& interfacesRemoved)
    {
        DeviceType deviceType = static_cast<DeviceType>(deviceTypeInt);

        // convert interface to device names
        std::vector<std::string> deviceNames;
        deviceNames.reserve(interfacesRemoved.size());

        for (const auto& interfaceName : interfacesRemoved)
        {
            deviceNames.push_back(DriverAPI::BuildDeviceName(deviceType, interfaceName));
        }

        Host::GetHost().GetDeviceManager().OnInterfacesRemoved(deviceNames);
    }
#endif

    // TODO: This is a temporary solution till all Enumeration functions will return DriverAPI
    //       or separate variables for device type and interface name
    DeviceType ConvertDeviceTypeName(const std::string& deviceTypeName)
    {
        static const std::map<std::string, DeviceType> sc_deviceTypeTranslationMap =
        {
            { "PCI",  DeviceType::PCI },
            { "JTAG", DeviceType::JTAG },
            { "TEST", DeviceType::DUMMY },
            { "RTL",  DeviceType::RTL }
        };

        const auto it = sc_deviceTypeTranslationMap.find(deviceTypeName);
        if (it == sc_deviceTypeTranslationMap.cend())
        {
            LOG_ERROR << "Invalid device type name. Use dummy device type as default." << std::endl;
            return DeviceType::DUMMY;
        }

        return it->second;
    }
}

namespace DriversFactory {

    // Enumerate unmonitored interfaces
    set<string> GetNonMonitoredDrivers()
    {
        set<string> enumeratedDevices;

    #ifdef _WINDOWS
        set<string> pciDevices = WindowsDriverAPI::Enumerate();
        set<string> jtagDevices = JTagDriver::Enumerate();
        enumeratedDevices.insert(pciDevices.begin(), pciDevices.end());
        enumeratedDevices.insert(jtagDevices.begin(), jtagDevices.end());
    #elif RTL_SIMULATION
        set<string> rtlDevices = RtlSimEnumerator::Enumerate();
        enumeratedDevices.insert(rtlDevices.begin(), rtlDevices.end());
    #endif

    #ifdef _UseDummyDevice
        set<string> testDevices = DummyDriver::Enumerate();
        enumeratedDevices.insert(testDevices.begin(), testDevices.end());
    #endif // _UseTestDevice

        return enumeratedDevices;
    }

    // Enumerate monitored interfaces and create corresponding device drivers and start interface monitoring
    // \remarks Expected to be called once during initialization
    std::vector<std::unique_ptr<DriverAPI>> StartNetworkInterfaceMonitors()
    {
#ifdef __linux
        std::set<std::string> ifnames = UnixNetworkInterfaceMonitor::GetInstance().
            StartMonitoring(OnInterfacesAdded, OnInterfacesRemoved);

        return TryCreateDriversForInterfaces(DeviceType::PCI, ifnames);
#else
        return std::vector<std::unique_ptr<DriverAPI>>();
#endif
    }

    // Create device driver
    // \remarks Device type is currently extracted from the full device name
    unique_ptr<DriverAPI> OpenDriver(const string& deviceName)
    {
        vector<string> tokens = Utils::Split(deviceName, DEVICE_NAME_DELIMITER);

        // Device name consists of exactly 3 elements:
        // 1. Baseband Type (SPARROW, TALYN...)
        // 2. Transport Type (PCI, JTAG, Serial...)
        // 3. Interface name (wMp, wPci, wlan0, wigig0...)
        if (tokens.size() != 2)
        {
            LOG_ERROR << "Failed to open interface for " << deviceName << " - unknown device name pattern" << endl;
            return nullptr;
        }

        const auto deviceType = ConvertDeviceTypeName(tokens[0]);

        unique_ptr<DriverAPI> pDevice = CreateDriver(deviceType, tokens[1]);

        if (pDevice)
        {
            if (!pDevice->Open())
            {
                LOG_ERROR << "Failed to open interface " << tokens[1] << " over " << tokens[0] << endl;
            }
        }
        return pDevice;
    }

} // namespace DriversFactory
