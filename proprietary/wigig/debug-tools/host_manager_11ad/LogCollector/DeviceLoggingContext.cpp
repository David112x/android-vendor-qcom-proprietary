/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/



#include "DeviceLoggingContext.h"
#include "ChunkConsumer.h"
#include "Host.h"
#include "DeviceManager.h"
#include "Device.h"
using namespace std;

namespace log_collector
{
    DeviceLoggingContext::DeviceLoggingContext(const std::string& deviceName)
        : m_deviceName(deviceName)
    {
        shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
        if (!device)
        {
            LOG_ERROR << "Invalid device name provided " << m_deviceName << endl;
            return;
        }
        unique_lock<mutex> lock(device->m_mutex);
        const auto fwStateProvider = device->GetFwStateProvider();
        lock.unlock();
        m_lastFwVersion = fwStateProvider->GetFwVersion();
        m_lastFwTimestamp = fwStateProvider->GetFwTimestamp();
        m_lastuCodeTimestamp = fwStateProvider->GetUcodeTimestamp();
        LOG_DEBUG << "DeviceLoggingContext ctor: m_lastFwVersion: " << m_lastFwVersion
            << " m_lastFwTimestamp: " << m_lastFwTimestamp
            << " m_lastuCodeTimestamp: " << m_lastuCodeTimestamp << endl;
        m_fwLogCollector = make_shared<log_collector::LogReader>(m_deviceName, log_collector::CPU_TYPE_FW);
        m_uCodeLogCollector = make_shared<log_collector::LogReader>(m_deviceName, log_collector::CPU_TYPE_UCODE);
    }


    DeviceLoggingContext::~DeviceLoggingContext() = default;

    void DeviceLoggingContext::RestoreLoggingState()
    {
        LOG_DEBUG << "in RestoreLoggingState" << endl;
        m_fwLogCollector->RestoreLogging();

        m_uCodeLogCollector->RestoreLogging();

        if (!VerifyFwVersion())
        {
             LOG_DEBUG << "Fw was not ready yet, state will be updated in poll" << endl;
        }
    }

    bool DeviceLoggingContext::PrepareForLogging()
    {
        bool resFw = m_fwLogCollector->UpdateDeviceInfo();

        bool resuCode = m_uCodeLogCollector->UpdateDeviceInfo();

        return resFw && resuCode;
    }

    void DeviceLoggingContext::UnregisterPollerAndReportDeviceRemoved()
    {
        m_fwLogCollector->UnRegisterPoller();
        m_uCodeLogCollector->UnRegisterPoller();
        m_fwLogCollector->ReportDeviceRemoved();
        m_uCodeLogCollector->ReportDeviceRemoved();
    }


    bool DeviceLoggingContext::VerifyFwVersion()
    {
        shared_ptr<Device> device = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName);
        if (!device)
        {

            LOG_ERROR << "Invalid device name provided " << m_deviceName << endl;
            return false; // we want to prepare recording so the validity wil be updated.
        }
        unique_lock<mutex> lock(device->m_mutex);
        const FwStateProvider* fwStateProvider = device->GetFwStateProvider();
        lock.unlock();

        //if (currState != wil_fw_state::WIL_FW_STATE_ERROR_BEFORE_READY && currState != wil_fw_state::WIL_FW_STATE_READY)
        if (!fwStateProvider->IsInitialized())
        {
            LOG_DEBUG << " Fw not initialized FW state is: " << fwStateProvider->GetFwHealthState() << endl;
            return false;
        }

        FwVersion tmpFwVersion = fwStateProvider->GetFwVersion();
        FwTimestamp tmpFwTimestamp = fwStateProvider->GetFwTimestamp();
        FwTimestamp tmuCodeTimestamp = fwStateProvider->GetUcodeTimestamp();
        if (!(m_lastFwVersion == tmpFwVersion && m_lastFwTimestamp == tmpFwTimestamp && m_lastuCodeTimestamp == tmuCodeTimestamp))
        {
            LOG_INFO << "FW changed on " << m_deviceName << " from " << m_lastFwVersion << " to " << tmpFwVersion << endl;

            m_lastFwVersion = tmpFwVersion;
            m_lastFwTimestamp = tmpFwTimestamp;
            m_lastuCodeTimestamp = tmuCodeTimestamp;
        }

        return true;
    }
}
