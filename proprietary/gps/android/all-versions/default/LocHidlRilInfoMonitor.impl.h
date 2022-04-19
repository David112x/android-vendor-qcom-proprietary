/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

// Methods from ::vendor::qti::gnss::V1_0::IGnssRilInfoMonitor follow.
Return<void> LocHidlRilInfoMonitor::init() {
    FROM_HIDL_CLIENT();

    if (NULL == mLBSAdapter) {
        mLBSAdapter = LBSAdapter::get(0);
    }
    return Void();
}

Return<void> LocHidlRilInfoMonitor::cinfoInject(int32_t cid, int32_t lac,
        int32_t mnc, int32_t mcc, bool roaming) {

    FROM_HIDL_CLIENT();

    if (mLBSAdapter != NULL) {
        mLBSAdapter->cinfoInject(cid, lac, mnc, mcc, roaming);
    } else {
        LOC_LOGE("mLBSAdapter NULL");
    }
    return Void();
}

Return<void> LocHidlRilInfoMonitor::oosInform() {
    FROM_HIDL_CLIENT();

    if (mLBSAdapter) {
        mLBSAdapter->oosInform();
    } else {
        LOC_LOGE("mLBSAdapter NULL");
    }
    return Void();
}

Return<void> LocHidlRilInfoMonitor::niSuplInit(const hidl_string& str) {
    FROM_HIDL_CLIENT();

    if (mLBSAdapter) {
        mLBSAdapter->niSuplInit(str.c_str(), str.size());
    } else {
        LOC_LOGE("mLBSAdapter NULL");
    }

    return Void();
}

Return<void> LocHidlRilInfoMonitor::chargerStatusInject(int32_t status) {

    FROM_HIDL_CLIENT();

    if (mLBSAdapter) {
        mLBSAdapter->chargerStatusInject(status);
    } else {
        LOC_LOGE("mLBSAdapter NULL");
    }

    return Void();
}

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
