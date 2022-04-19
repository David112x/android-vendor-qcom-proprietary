/******************************************************************************
Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/
#pragma once

#include <vendor/qti/hardware/servicetracker/1.1/IServicetracker.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace servicetracker {
namespace V1_1 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Servicetracker : public IServicetracker {
    // Methods from ::vendor::qti::hardware::servicetracker::V1_0::IServicetracker follow.
    Return<void> startService(const ::vendor::qti::hardware::servicetracker::V1_0::ServiceData& serviceData) override;
    Return<void> bindService(const ::vendor::qti::hardware::servicetracker::V1_0::ServiceData& serviceData, const ::vendor::qti::hardware::servicetracker::V1_0::ClientData& clientData) override;
    Return<void> unbindService(const ::vendor::qti::hardware::servicetracker::V1_0::ServiceData& serviceData, const ::vendor::qti::hardware::servicetracker::V1_0::ClientData& clientData) override;
    Return<void> destroyService(const ::vendor::qti::hardware::servicetracker::V1_0::ServiceData& serviceData) override;
    Return<void> killProcess(int32_t pid) override;
    Return<void> getclientInfo(const hidl_string& clientName, getclientInfo_cb _hidl_cb) override;
    Return<void> getserviceInfo(const hidl_string& serviceName, getserviceInfo_cb _hidl_cb) override;
    Return<void> getServiceConnections(const hidl_string& serviceName, getServiceConnections_cb _hidl_cb) override;
    Return<void> getClientConnections(const hidl_string& clientName, getClientConnections_cb _hidl_cb) override;
    Return<void> getPid(const hidl_string& processName, getPid_cb _hidl_cb) override;
    Return<void> getPids(const hidl_vec<hidl_string>& serviceList, getPids_cb _hidl_cb) override;
    Return<void> isServiceB(const hidl_string& serviceName, isServiceB_cb _hidl_cb) override;
    Return<void> getServiceBCount(getServiceBCount_cb _hidl_cb) override;

    // Methods from ::vendor::qti::hardware::servicetracker::V1_1::IServicetracker follow.
    Return<void> getRunningServicePid(getRunningServicePid_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
 extern "C" IServicetracker* HIDL_FETCH_IServicetracker(const char* name);

}  // namespace implementation
}  // namespace V1_1
}  // namespace servicetracker
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
