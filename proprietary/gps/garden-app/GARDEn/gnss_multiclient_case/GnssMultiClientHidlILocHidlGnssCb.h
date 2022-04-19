/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#ifndef GNSS_MULTI_CLIENT_HIDL_ILOCHIDLGNSS_CB_H
#define GNSS_MULTI_CLIENT_HIDL_ILOCHIDLGNSS_CB_H
#include "GnssMultiClientCase.h"
#include "GnssCbBase.h"

#include <vendor/qti/gnss/3.0/ILocHidlGnss.h>
#include <vendor/qti/gnss/3.0/ILocHidlIzatConfig.h>

using android::hardware::Return;
using android::hardware::Void;

using ::vendor::qti::gnss::V3_0::ILocHidlGnss;
using ::vendor::qti::gnss::V3_0::ILocHidlIzatConfigCallback;

namespace garden {

class GnssMultiClientHidlILocHidlGnss;

class GnssMultiClientHidlILocHidlGnssCb :
        public ILocHidlIzatConfigCallback
{
public:
    GnssMultiClientHidlILocHidlGnssCb(GnssMultiClientHidlILocHidlGnss* hidlILocHidlGnss);
    ~GnssMultiClientHidlILocHidlGnssCb();

    Return<void> izatConfigCallback(const ::android::hardware::hidl_string& izatConfigContent);

private:
    GnssMultiClientHidlILocHidlGnss* mLocHidlGnss = nullptr;
};

} //namespace garden

#endif //GNSS_MULTI_CLIENT_HIDL_ILOCHIDLGNSS_CB_H
