/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#ifndef GNSS_MULTI_CLIENT_HIDL_IGNSS_H
#define GNSS_MULTI_CLIENT_HIDL_IGNSS_H

#include "IGnssAPI.h"
#include <LocationAPI.h>
#include <GnssMultiClientHidlIGnssCb.h>

#include <android/hardware/gnss/2.0/IGnss.h>

#include "IGardenCase.h"

using android::OK;
using android::sp;
using android::wp;
using android::status_t;

using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;
using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;


using android::hardware::gnss::V2_0::IGnss;

using android::hardware::gnss::V1_1::IGnssCallback;
using android::hardware::gnss::V1_1::IGnssConfiguration;
using android::hardware::gnss::V1_1::IGnssMeasurement;
using ::android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControl;

using IGnssConfiguration_V2_0 = android::hardware::gnss::V2_0::IGnssConfiguration;

using IGnssVisibilityControl = android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControl;

using IAGnssRil_V2_0 = android::hardware::gnss::V2_0::IAGnssRil;
using IGnssMeasurement_V1_0 = android::hardware::gnss::V1_0::IGnssMeasurement;
using IGnssMeasurement_V1_1 = android::hardware::gnss::V1_1::IGnssMeasurement;
using IGnssMeasurement_V2_0 = android::hardware::gnss::V2_0::IGnssMeasurement;
using IAGnssRil_V2_0 = android::hardware::gnss::V2_0::IAGnssRil;
using IAGnss_V2_0 = android::hardware::gnss::V2_0::IAGnss;
using IGnssDebug_V2_0 = android::hardware::gnss::V2_0::IGnssDebug;
using IGnssBatching_V1_0 = android::hardware::gnss::V1_0::IGnssBatching;
using IGnssBatching_V2_0 = android::hardware::gnss::V2_0::IGnssBatching;
using IAGnssCallback_2_0 = android::hardware::gnss::V2_0::IAGnssCallback;
using IGnssCallback_2_0 = android::hardware::gnss::V2_0::IGnssCallback;

using android::hardware::gnss::measurement_corrections::V1_0::IMeasurementCorrections;
using android::hardware::gnss::measurement_corrections::V1_0::MeasurementCorrections;
using android::hardware::gnss::V1_0::IGnssNi;
using android::hardware::gnss::V2_0::ElapsedRealtimeFlags;
using android::hardware::gnss::V2_0::GnssConstellationType;
using android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControl;
namespace garden {

/* Data structures for HIDL service interaction */
struct GnssDeathRecipient : virtual public hidl_death_recipient {
    virtual void serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) override;
    virtual ~GnssDeathRecipient() {};
};

/* GNSS MULTI CLIENT HIDL CLIENT FOR IGNSS INTERFACE */
class GnssMultiClientHidlIGnss {

    friend class GnssMultiClientHidlIGnssCb;

private:
    static GnssMultiClientHidlIGnss* sInstance;
    GnssMultiClientHidlIGnss() {
        mHidlIGnssCb = new GnssMultiClientHidlIGnssCb(this);
    };

public:
    static GnssMultiClientHidlIGnss& get() {
        if (nullptr == sInstance) {
            sInstance = new GnssMultiClientHidlIGnss();
        }
        return *sInstance;
    }

public:
    GARDEN_RESULT menuTest();

private:
    /* Individual test cases triggered from menu test */
    GARDEN_RESULT createHidlClient();

    GARDEN_RESULT IGnss_1_0_start();
    GARDEN_RESULT IGnss_1_0_stop();
    GARDEN_RESULT IGnss_1_1_setPositionMode_1_1();
    GARDEN_RESULT IGnss_1_1_injectBestLocation();

    GARDEN_RESULT IGnssMeasurement_1_0_close();
    GARDEN_RESULT IGnssMeasurement_1_1_setCallback_1_1();
    GARDEN_RESULT IGnssConfiguration_1_1_setBlacklist();

    GARDEN_RESULT IGnssConfiguration_2_0_setEsExtensionSec();
    GARDEN_RESULT GnssVisibilityControl_setCallback();
    GARDEN_RESULT GnssVisibilityControl_enableNfwLocationAccess(bool enable);

    GARDEN_RESULT IGnss_setCallback();
    GARDEN_RESULT hidlClientCleanup();

    GARDEN_RESULT IGnss_setCallback_2_0();
    GARDEN_RESULT IGnss_injectBestLocation_2_0();
    GARDEN_RESULT GnssMeasurement_2_0_setCallback_2_0();
    GARDEN_RESULT GnssBatching_init_2_0();
    GARDEN_RESULT GnssDebug_getDebugData_2_0();
    GARDEN_RESULT AGnss_2_0_setCallback_2_0();
    GARDEN_RESULT AGnss_2_0_dataConnOpen_2_0();
    GARDEN_RESULT AGnss_2_0_setServer_2_0();
    GARDEN_RESULT AGnss_2_0_dataConnClosed_2_0();
public:
    sp<IGnss> mGnssIface = nullptr;
    sp<IGnssConfiguration> mGnssConfigurationIface = nullptr;
    sp<IGnssMeasurement_V1_1> mGnssMeasurementIface_1_1 = nullptr;
    sp<GnssDeathRecipient> mGnssDeathRecipient = nullptr;

    sp<IGnssConfiguration_V2_0> mGnssConfig_2_0 = nullptr;
    sp<IGnssVisibilityControl> mGnssVisibilityControl = nullptr;
    sp<IAGnss_V2_0> mAGnss_2_0 = nullptr;
    sp<IAGnssRil_V2_0> mAGnssRil_2_0 = nullptr;
    sp<IGnssDebug_V2_0> mGnssDebug_2_0 = nullptr;
    sp<IGnssBatching_V2_0> mGnssBatching_2_0 = nullptr;
    sp<IGnssMeasurement_V2_0> mGnssMeasurementIface_2_0 = nullptr;
    sp<IMeasurementCorrections> mMeasurementCorrections = nullptr;
    sp<GnssMultiClientHidlIGnssCb> mHidlIGnssCb = nullptr;
};

} //namespace garden


#endif //GNSS_MULTI_CLIENT_HIDL_IGNSS_H
