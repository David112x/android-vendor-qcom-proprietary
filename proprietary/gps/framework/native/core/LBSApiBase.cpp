/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014, 2016-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#define LOG_NDEBUG 0
#define LOG_TAG "LocSvc_LBSApiBase"
#include <LBSApiBase.h>
#include <LBSAdapterBase.h>
#include <loc_pla.h>

using namespace loc_core;

namespace lbs_core {

#define TO_ALL_LBSADAPTERS(call) TO_ALL_ADAPTERS(mLBSAdapters, (call))
#define TO_1ST_HANDLING_LBSADAPTERS(call) TO_1ST_HANDLING_ADAPTER(mLBSAdapters, (call))

LBSApiBase::LBSApiBase(LocApiProxyBase *locApiProxy) :
    mLocApiProxy(locApiProxy)
{
    memset(mLBSAdapters, 0, sizeof(mLBSAdapters));
    LOC_LOGd("LBSApiBase created. :lbsApiBase%p\n", this);
}

void LBSApiBase::addAdapter(LBSAdapterBase* adapter)
{
    for (int i = 0; i < MAX_ADAPTERS && mLBSAdapters[i] != adapter; i++) {
        if (mLBSAdapters[i] == NULL) {
            mLBSAdapters[i] = adapter;
            LOC_LOGD("%s:%d]: Adapter added: %p\n", __func__, __LINE__, adapter);
            break;
        }
    }
}

void LBSApiBase::removeAdapter(LBSAdapterBase* adapter)
{
    for (int i = 0;
         i < MAX_ADAPTERS && NULL != mLBSAdapters[i];
         i++) {
        if (mLBSAdapters[i] == adapter) {
            mLBSAdapters[i] = NULL;

            // shift the rest of the adapters up so that the pointers
            // in the array do not have holes.  This should be more
            // performant, because the array maintenance is much much
            // less frequent than event handlings, which need to linear
            // search all the adapters
            int j = i;
            while (++i < MAX_ADAPTERS && mLBSAdapters[i] != NULL);

            // i would be MAX_ADAPTERS or point to a NULL
            i--;
            // i now should point to a none NULL adapter within valid
            // range although i could be equal to j, but it won't hurt.
            // No need to check it, as it gains nothing.
            mLBSAdapters[j] = mLBSAdapters[i];
            // this makes sure that we exit the for loop
            mLBSAdapters[i] = NULL;
        }
    }
}

void LBSApiBase::requestWps(const OdcpiRequest &request)
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestWps(request));
}

void LBSApiBase::requestWifiApData(const WifiApDataRequest &request)
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestWifiApData(request));
}

void LBSApiBase::requestSrnData(const GnssSrnDataReq &gnssSrnDataReq)
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestSrnData(gnssSrnDataReq));
}

void LBSApiBase::cinfoInject(int /*cid*/, int /*lac*/, int /*mnc*/,
                int /*mcc*/, bool /*roaming*/)
DEFAULT_IMPL()

void  LBSApiBase::oosInform()
DEFAULT_IMPL()

void  LBSApiBase::niSuplInit(char* /*supl_init*/, int /*length*/)
DEFAULT_IMPL()

void  LBSApiBase::chargerStatusInject(int /*status*/)
DEFAULT_IMPL()

void  LBSApiBase::wifiAttachmentStatusInject(
    const WifiSupplicantInfo& /*wifiSupplicantInfo*/)
DEFAULT_IMPL()

void  LBSApiBase::wifiEnabledStatusInject(int /*status*/)
DEFAULT_IMPL()

void  LBSApiBase::injectWifiApInfo(const WifiApInfo& /*wifiApInfo*/)
DEFAULT_IMPL()

void  LBSApiBase::injectBtClassicDevScanData(const BtDeviceInfo& /*btDevInfo*/)
DEFAULT_IMPL()

void  LBSApiBase::injectBtLeDevScanData(const BtDeviceInfo& /*btDevInfo*/)
DEFAULT_IMPL()

void  LBSApiBase::setWifiWaitTimeoutValue(int /*timeout*/)
DEFAULT_IMPL()

int LBSApiBase::shutdown()
DEFAULT_IMPL(0)

void LBSApiBase::requestSensorData(SensorRequest &request)
{
     TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestSensorData(request));
}

void LBSApiBase::requestPedometerData(PedometerRequest &request)
{
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestPedometerData(request));
}

void LBSApiBase::requestMotionData(SensorRequest &request)
{
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestMotionData(request));
}

void LBSApiBase::requestTimeData(TimeRequest &timeRequest)
{
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestTimeData(timeRequest));
}

void LBSApiBase::requestSPIStatus(SensorRequest &request)
{
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestSPIStatus(request));
}

void LBSApiBase::injectSensorData(const SensorDataBundle& /*dataBundle*/)
DEFAULT_IMPL()

void LBSApiBase::injectMotionData(const MotionData& /*motionData*/)
DEFAULT_IMPL()

void LBSApiBase::injectPedometerData(const PedometerData& /*pedometerData*/)
DEFAULT_IMPL()

void LBSApiBase::injectTimeData(const TimeData& /*data*/)
DEFAULT_IMPL()

int LBSApiBase::injectSPIStatus(const SPIStatus& /*status*/)
DEFAULT_IMPL(0)

void LBSApiBase::injectFeatureConfig(e_premium_mode /*sap_mode*/,
    e_premium_mode /*gtp_cell_mode*/, e_premium_mode /*gtp_wifi_mode*/,
    LocApiResponse * /*adapterResponse*/)
DEFAULT_IMPL()

void LBSApiBase::requestTimeZoneInfo()
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestTimeZoneInfo());
}

void LBSApiBase :: timeInfoInject(long /*curTimeMillis*/, int /*rawOffset*/, int /*dstOffset*/)
DEFAULT_IMPL()

void  LBSApiBase::batteryLevelInject(int batteryPct)
DEFAULT_IMPL()

void LBSApiBase::requestFdclDataEvent()
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestFdclDataEvent());
}

void LBSApiBase::reportFdclStatusEvent(bool success, const char* errorString, uint32_t errorLength)
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->reportFdclStatusEvent(success, errorString,
                                                                       errorLength));
}

void LBSApiBase::reportFdclInfoEvent(FdclInfo& fdclInfo, uint8_t status, UlpLocation& location)
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->reportFdclInfoEvent(fdclInfo, status, location));
}

void LBSApiBase::injectFdclData(const FdclData& fdclData)
DEFAULT_IMPL()

void LBSApiBase::requestFdclData(int32_t expireInDays)
DEFAULT_IMPL()

void LBSApiBase::requestCellCSData()
DEFAULT_IMPL()

void LBSApiBase::requestCellCSDataEvent()
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestCellCSDataEvent());
}

void LBSApiBase::reportCellCSInfoEvent(CellCSInfo& cellCSInfo, uint8_t status)
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->reportCellCSInfoEvent(cellCSInfo, status));
}

}; //namespace lbs_core
