/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014, 2016-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef LBS_API_V02_H
#define LBS_API_V02_H

#include <LocApiV02.h>
#include <LBSApiBase.h>
#include <LocationDataTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

//Empty class declaration so that IzatApiV02
//can be declared as a friend of LBSApiV02
namespace izat_core {
    class IzatApiV02;
}

using namespace loc_core;
using namespace izat_core;

namespace lbs_core {

class LBSApiV02 : public LocApiV02, public LBSApiBase {
    qmiLocPositionSrcEnumT_v02 mapSourceType(PositionSourceType sourceType);
    qmiLocPositionSrcProviderEnumT_v02 mapSourceProviderType(PositionSourceProviderType sourceProviderType);
    qmiLocReliabilityEnumT_v02 mapReliabilityValue(LocationReliability reliabilityValue);
    uint32_t mOpaqueId;
    LBSApiV02(LOC_API_ADAPTER_EVENT_MASK_T exMask,
              ContextBase* context = NULL);

protected:
    // use LocApiBase::destroy()
    virtual ~LBSApiV02();

public:
    static LBSApiV02* createLBSApiV02(LOC_API_ADAPTER_EVENT_MASK_T exMask,
                                      ContextBase* context = NULL);

    inline virtual void* getSibling() { return (LBSApiBase*)this; }
    inline virtual LocApiProxyBase* getLocApiProxy()
    {
        return mLocApiProxy;
    }

   /* event callback registered with the loc_api v02 interface */
    virtual void eventCb(locClientHandleType client_handle,
                         uint32_t loc_event_id,
                         locClientEventIndUnionType loc_event_payload);

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
    virtual void injectFeatureConfig(e_premium_mode sap_mode, e_premium_mode gtp_cell_mode,
            e_premium_mode gtp_wifi_mode, LocApiResponse *adapterResponse);
    virtual void injectSensorData(const SensorDataBundle &dataBundle);
    virtual void injectMotionData(const MotionData &motionData);
    virtual void injectPedometerData(const PedometerData &pedometerData);
    virtual void injectTimeData(const TimeData &data);
    virtual int injectSPIStatus(const SPIStatus &status);
    virtual void timeInfoInject(long curTimeMillis, int rawOffset, int dstOffset);
    virtual void batteryLevelInject(int batteryPct);
    virtual void injectFdclData(const FdclData& fdclData);
    virtual void requestFdclData(int32_t expireInDays);
    virtual void requestCellCSData();

private:
    void doSensorDataRequest(
            const qmiLocSensorReadyStatusStructT_v02 &fromRequest,
            SensorType sensorType);
    static void convertThreeAxisData(
            const SensorDataBundle &fromData,
            qmiLoc3AxisSensorSampleListStructT_v02 &toData);
    static void convertTemperatureData(
            const SensorDataBundle &fromData,
            qmiLocSensorTemperatureSampleListStructT_v02 &toData);
    qmiLocReliabilityEnumT_v02 getReliabilityEnumFromReliability(uint32_t reliability);
    qmiLocFdclAirInterfaceTypeEnumT_v02 getAirInterfaceTypeFromCellType(uint32_t cell_type);
};
}; //namespace lbs_core
#ifdef __cplusplus
}
#endif

#endif //LBS_API_V02_H
