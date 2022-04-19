/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/
#ifndef ANDROID_HARDWARE_BLUETOOTH_A2DP_V1_0_BLUETOOTHAUDIOOFFLOAD_H
#define ANDROID_HARDWARE_BLUETOOTH_A2DP_V1_0_BLUETOOTHAUDIOOFFLOAD_H

#include <android/hardware/bluetooth/a2dp/1.0/IBluetoothAudioOffload.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android {
namespace hardware {
namespace bluetooth {
namespace a2dp {
namespace V1_0 {
namespace implementation {

using ::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioOffload;
using ::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost;
using ::android::hardware::bluetooth::a2dp::V1_0::Status;
using ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hardware::hidl_death_recipient;
using ::android::wp;

using CodecCfg_cb = std::function<void(Status status, CodecConfiguration& cfg)>;

typedef void (*start_stream_request_t)(void);
typedef void (*suspend_stream_request_t)(void);
typedef void (*stop_stream_request_t)(void);
typedef void (*check_a2dp_ready_request_t)(void);
typedef void (*get_codec_config_callback_t)(void);
typedef void (*get_sink_latency_request_t)(void);

typedef struct {
    start_stream_request_t hidl_start_req_cb;
    suspend_stream_request_t hidl_suspend_req_cb;
    stop_stream_request_t hidl_stop_req_cb;
    check_a2dp_ready_request_t hidl_check_a2dp_ready_req_cb;
    get_codec_config_callback_t hidl_get_codec_cfg_req_cb;
    get_sink_latency_request_t hidl_get_sink_latency_req_cb;
} hidl_callback_t;

typedef void (*bt_stack_init_t)(hidl_callback_t *callbacks);
typedef void (*bt_stack_deinit_t)(Status status);
typedef void (*bt_stack_on_stream_started_t)(Status status);
typedef void (*bt_stack_on_stream_suspended_t)(Status status);
typedef void (*bt_stack_on_stream_stopped_t)(Status status);
typedef void (*bt_stack_start_session_t)(const CodecConfiguration &cfg);
typedef void (*bt_stack_on_check_a2dp_ready_t)(Status status);
typedef void (*bt_stack_on_get_sink_latency_t)(uint16_t latency);

typedef struct {
    bt_stack_init_t bt_stack_init;
    bt_stack_deinit_t bt_stack_deinit;
    bt_stack_on_stream_started_t bt_stack_on_stream_started;
    bt_stack_on_stream_suspended_t bt_stack_on_stream_suspended;
    bt_stack_on_stream_stopped_t bt_stack_on_stream_stopped;
} bt_lib_stream_control_t;

using OnClientDead = std::function<void(void)>;

struct BluetoothDeathRecipient : hidl_death_recipient {
    BluetoothDeathRecipient(const OnClientDead& clientDead) {
      mOnClientDead = clientDead;
    }
    virtual void serviceDied(
        uint64_t /*cookie*/,
        const wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
      mOnClientDead();
      has_died_ = true;
    }
    OnClientDead mOnClientDead;
    bool getHasDied() const { return has_died_; }
    void setHasDied(bool has_died) { has_died_ = has_died; }

  private:
    bool has_died_;
};

class BluetoothAudioOffload : public IBluetoothAudioOffload {
  public:
    // Methods from ::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioOffload follow.
    BluetoothAudioOffload();
    Return<::android::hardware::bluetooth::a2dp::V1_0::Status> startSession(
        const sp<::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost>& hostIf,
        const ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration& codecConfig) override;
    Return<void> streamStarted(::android::hardware::bluetooth::a2dp::V1_0::Status status) override;
    Return<void> streamSuspended(::android::hardware::bluetooth::a2dp::V1_0::Status status) override;
    Return<void> endSession() override;

    static void get_codec_config_callback();
    static void start_stream_request_callback();
    static void suspend_stream_request_callback();
    static void stop_stream_request_callback();
    static void check_a2dp_ready_request_callback();
    void cleanup();

    ::android::sp<IBluetoothAudioHost> stream_cb_;
  private:
    ::android::sp<BluetoothDeathRecipient> deathRecipient;
    std::function<void(sp<BluetoothDeathRecipient>&)> unlink_cb_;
    hidl_callback_t hidl_callback_;
    void *lib_handle;
    bt_lib_stream_control_t *bt_lib_control;
    bt_stack_init_t lib_init;
    bt_stack_deinit_t lib_deinit;
    bt_stack_on_stream_started_t on_started;
    bt_stack_on_stream_suspended_t on_suspended;
    bt_stack_on_stream_stopped_t on_stopped;
    bt_stack_start_session_t start_session;

#ifdef BHK
    bt_stack_on_check_a2dp_ready_t on_check_a2dp_ready;
    bt_stack_on_get_sink_latency_t on_get_sink_latency;
#endif
    int initialize_btlib(hidl_callback_t *cb);
    void deinitialize_btlib();
    // Methods from ::android::hidl::base::V1_0::IBase follow.
};

extern "C" IBluetoothAudioOffload* HIDL_FETCH_IBluetoothAudioOffload(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace a2dp
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_BLUETOOTH_A2DP_V1_0_BLUETOOTHAUDIOOFFLOAD_H
