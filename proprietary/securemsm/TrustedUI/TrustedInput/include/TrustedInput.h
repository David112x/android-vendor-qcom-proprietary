/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef TRUSTEDINPUT_H
#define TRUSTEDINPUT_H

#include <fcntl.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <memory>
#include <string>
#include <thread>
#include <vendor/qti/hardware/trustedui/1.0/ITrustedInput.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace trustedui {
namespace V1_0 {
namespace implementation {

using ::vendor::qti::hardware::trustedui::V1_0::ITrustedInputCallback;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct TrustedInput : public ITrustedInput {
    // Methods from ::vendor::qti::hardware::trustedui::V1_0::ITrustedInput follow.
    Return<void> init(const sp<ITrustedInputCallback>& cb, init_cb _hidl_cb) override;
    Return<Response> terminate() override;
    Return<Response> getInput(int32_t timeout) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    std::shared_ptr<std::thread> mThread = nullptr;
    int32_t mAbortFd = -1;
    sp<ITrustedInputCallback> mCB;
    uint8_t stSession = false;
    int mControlFd = -1;
    int mIrqFd = -1;
    int32_t mTouchControllerId = -1;
    std::string mControlFile;
    std::string mIrqFile;

    void _waitForInputThreadFunc(int32_t _timeout);
    int32_t _stStartSession(void);
    int32_t _stTerminateSession(void);
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace trustedui
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif
