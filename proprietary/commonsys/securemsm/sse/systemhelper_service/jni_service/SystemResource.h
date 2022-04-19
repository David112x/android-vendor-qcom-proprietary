/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef SYSTEMRESOURCE_H
#define SYSTEMRESOURCE_H

#include <android/hardware/graphics/bufferqueue/1.0/IGraphicBufferProducer.h>
#include <vendor/qti/hardware/systemhelper/1.0/ISystemResource.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

using ::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::vendor::qti::hardware::systemhelper::V1_0::SystemResourceType;

namespace vendor {
namespace qti {
namespace hardware {
namespace systemhelper {
namespace V1_0 {
namespace implementation {

struct SystemResource : public ISystemResource {
    // Methods from ::vendor::qti::hardware::systemhelper::V1_0::ISystemResource follow.
    Return<void> acquire(SystemResourceType resource, acquire_cb _hidl_cb) override;
    Return<void> acquireSurface(uint32_t width, uint32_t height, acquireSurface_cb _hidl_cb) override;
    Return<int32_t> release(uint32_t resourceId) override;

private:
    bool isExist(SystemResourceType resource);
    uint32_t getResourceId(void);
    int32_t releaseSurface(uint32_t resourceId);
    int32_t releaseResource(uint32_t resourceId);

};

}  // namespace implementation
}  // namespace V1_0
}  // namespace systemhelper
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif
