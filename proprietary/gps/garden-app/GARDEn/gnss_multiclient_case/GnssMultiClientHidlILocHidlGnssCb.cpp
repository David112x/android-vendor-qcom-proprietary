/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#define LOG_NDEBUG 0
#define LOG_TAG "GARDEN_GMCC_ILocHidlGnssCb"

#include "GnssMultiClientHidlILocHidlGnssCb.h"
#include "GnssMultiClientCaseUtils.h"

namespace garden {

GnssMultiClientHidlILocHidlGnssCb::GnssMultiClientHidlILocHidlGnssCb(
        GnssMultiClientHidlILocHidlGnss* hidlGnss)
{
    mLocHidlGnss = hidlGnss;
}

GnssMultiClientHidlILocHidlGnssCb::~GnssMultiClientHidlILocHidlGnssCb() {}

Return<void> GnssMultiClientHidlILocHidlGnssCb::izatConfigCallback(
        const ::android::hardware::hidl_string& izatConfigContent)
{
    PRINTLN("izatConfigCallback()");
    PRINTLN("izat.conf content:\n%s", izatConfigContent.c_str());
    return Void();
}

} // namespace garden
