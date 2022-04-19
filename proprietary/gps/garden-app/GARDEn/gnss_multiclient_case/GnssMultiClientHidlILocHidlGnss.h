/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#ifndef GNSS_MULTI_CLIENT_HIDL_ILOCHIDLGNSS_H
#define GNSS_MULTI_CLIENT_HIDL_ILOCHIDLGNSS_H

#include "IGnssAPI.h"
#include <LocationAPI.h>
#include <GnssMultiClientHidlILocHidlGnssCb.h>

#include <vendor/qti/gnss/3.0/ILocHidlGnss.h>

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

using ::vendor::qti::gnss::V3_0::ILocHidlGnss;
using ::vendor::qti::gnss::V3_0::ILocHidlIzatConfig;

namespace garden {

#define GNSS_VENDOR_SERVICE_INSTANCE "gnss_vendor"

/* Data structures for HIDL service interaction */
struct LocHidlGnssDeathRecipient : virtual public hidl_death_recipient {
    virtual void serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) override;
    virtual ~LocHidlGnssDeathRecipient() {};
};

/* GNSS MULTI CLIENT HIDL CLIENT FOR IGNSS INTERFACE */
class GnssMultiClientHidlILocHidlGnss {

    friend class GnssMultiClientHidlILocHidlGnssCb;

private:
    static GnssMultiClientHidlILocHidlGnss* sInstance;
    GnssMultiClientHidlILocHidlGnss() {
        mLocHidlGnssCb = new GnssMultiClientHidlILocHidlGnssCb(this);
    };

public:
    static GnssMultiClientHidlILocHidlGnss& get() {
        if (nullptr == sInstance) {
            sInstance = new GnssMultiClientHidlILocHidlGnss();
        }
        return *sInstance;
    }

public:
    GARDEN_RESULT menuTest();

private:
    /* Individual test cases triggered from menu test */
    GARDEN_RESULT createHidlClient();

    GARDEN_RESULT ILocHidlIzatConfig_1_0_init();
    GARDEN_RESULT ILocHidlIzatConfig_1_0_readConfig();

public:
    sp<ILocHidlGnss> mLocHidlGnssIface = nullptr;
    sp<ILocHidlIzatConfig> mLocHidlGnssExtensionIzatConfig = nullptr;
    sp<LocHidlGnssDeathRecipient> mLocHidlGnssDeathRecipient = nullptr;

    sp<GnssMultiClientHidlILocHidlGnssCb> mLocHidlGnssCb = nullptr;
};

} //namespace garden


#endif //GNSS_MULTI_CLIENT_HIDL_ILOCHIDLGNSS_H
