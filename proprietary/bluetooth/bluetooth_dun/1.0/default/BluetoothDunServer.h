/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <vendor/qti/hardware/bluetooth_dun/1.0/IBluetoothDunServer.h>
#include <vendor/qti/hardware/bluetooth_dun/1.0/types.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <thread>

namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth_dun {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::wp;
using ::vendor::qti::hardware::bluetooth_dun::V1_0::IBluetoothDunServerResponse;
using ::vendor::qti::hardware::bluetooth_dun::V1_0::CtrlMsg;
using ::vendor::qti::hardware::bluetooth_dun::V1_0::Status;

class BluetoothDunServerDeathRecipient;

struct BluetoothDunServer : public IBluetoothDunServer {
    // Methods from ::vendor::qti::hardware::bluetooth_dun::V1_0::IBluetoothDunServer follow.
    Return<Status> initialize(const sp<IBluetoothDunServerResponse>& resCallback) override;
    Return<Status> sendCtrlMsg(CtrlMsg msgType) override;
    Return<Status> sendUplinkData(const hidl_vec<uint8_t>& packet) override;
    Return<Status> sendModemStatus(uint8_t status) override;
    Return<void> close_server() override;
    BluetoothDunServer();

    private:
        std::thread read_thread_;
        int startReadThread();
        void stopReadThread();
        void portBridgeReadThreadRoutine();
        ::android::sp<IBluetoothDunServerResponse> res_cb;
        ::android::sp<BluetoothDunServerDeathRecipient> death_recipient_;
        std::function<void(sp<BluetoothDunServerDeathRecipient>&)> unlink_cb_;
        int conn_sk;
};

extern "C" IBluetoothDunServer* HIDL_FETCH_IBluetoothDunServer(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth_dun
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
