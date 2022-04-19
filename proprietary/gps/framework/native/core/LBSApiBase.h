/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014, 2016-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef LBS_API_BASE_H
#define LBS_API_BASE_H

#include <WiperData.h>
#include <SlimSensor.h>
#include <LocApiBase.h>
#include "wwan_db_receiver.h"
#include "wwan_db_provider.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_ADAPTERS 10

using namespace loc_core;
using namespace izat_manager;

namespace lbs_core {

typedef enum {
    DISABLED,
    BASIC,
    PREMIUM,
    THIRDPARTY,
    MODEM_DEFAULT
}e_premium_mode;

class LBSAdapterBase;

class LBSApiBase {
    LBSAdapterBase *mLBSAdapters[MAX_ADAPTERS];
protected:
    LocApiProxyBase* mLocApiProxy;
public:
    LBSApiBase(LocApiProxyBase* locApiProxy);
    inline virtual ~LBSApiBase() {}

    void addAdapter(LBSAdapterBase *adapter);
    void removeAdapter(LBSAdapterBase *adapter);

    void requestWps(const OdcpiRequest &request);
    void requestWifiApData(const WifiApDataRequest &request);
    void requestTimeZoneInfo();
    void requestSrnData(const GnssSrnDataReq &gnssSrnDataReq);
    void requestFdclDataEvent();
    void reportFdclStatusEvent(bool success, const char* errorString, uint32_t errorLength);
    void reportFdclInfoEvent(FdclInfo& fdclInfo, uint8_t status, UlpLocation& location);
    void requestCellCSDataEvent();
    void reportCellCSInfoEvent(CellCSInfo& cellCSInfo, uint8_t status);

    virtual void cinfoInject(int cid, int lac, int mnc,
                            int mcc, bool roaming);
    virtual void oosInform();
    virtual void niSuplInit(char* supl_init, int length);
    virtual void chargerStatusInject(int status);
    virtual void wifiAttachmentStatusInject(
        const WifiSupplicantInfo &wifiSupplicantInfo);
    virtual void wifiEnabledStatusInject(int status);
    virtual void injectWifiApInfo(const WifiApInfo &wifiApInfo);
    virtual void injectBtClassicDevScanData(const BtDeviceInfo &btDevInfo);
    virtual void injectBtLeDevScanData(const BtDeviceInfo &btDevInfo);
    virtual void setWifiWaitTimeoutValue(int timeout);
    virtual int shutdown();
    virtual void injectFeatureConfig(e_premium_mode sap_mode,
                                     e_premium_mode gtp_cell_mode,
                                     e_premium_mode gtp_wifi_mode,
                                     LocApiResponse *adapterResponse);
    virtual void requestSensorData(SensorRequest &request);
    virtual void requestPedometerData(PedometerRequest &request);
    virtual void requestMotionData(SensorRequest &request);
    virtual void requestTimeData(TimeRequest &timeRequest);
    virtual void requestSPIStatus(SensorRequest &request);
    virtual void injectSensorData(const SensorDataBundle &dataBundle);
    virtual void injectMotionData(const MotionData &motionData);
    virtual void injectPedometerData(const PedometerData &pedometerData);
    virtual void injectTimeData(const TimeData &data);
    virtual int injectSPIStatus(const SPIStatus &status);
    virtual void timeInfoInject(long curTimeMillis, int rawOffset,
                               int dstOffset);
    virtual void batteryLevelInject(int batteryPct);
    virtual void injectFdclData(const FdclData& fdclData);
    virtual void requestFdclData(int32_t expireInDays);
    virtual void requestCellCSData();
};
}; //namespace lbs_core
#ifdef __cplusplus
}
#endif
#endif //LBS_API_BASE_H
