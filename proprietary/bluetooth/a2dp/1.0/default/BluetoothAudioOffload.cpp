/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/
#include "BluetoothAudioOffload.h"
#include <dlfcn.h>
#include <utils/Log.h>

namespace android {
namespace hardware {
namespace bluetooth {
namespace a2dp {
namespace V1_0 {
namespace implementation {

std::mutex sessionMutex;
BluetoothAudioOffload *btAudio = nullptr;

BluetoothAudioOffload::BluetoothAudioOffload()
    : deathRecipient(new BluetoothDeathRecipient(
                         std::bind(&BluetoothAudioOffload::cleanup, this))) { }

// Methods from ::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioOffload follow.
Return<::android::hardware::bluetooth::a2dp::V1_0::Status> BluetoothAudioOffload::startSession(
    const sp<::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost>& hostIf,
    const ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration& codecConfig) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    std::unique_lock<std::mutex> guard(sessionMutex);
    btAudio = new BluetoothAudioOffload();
    btAudio->deathRecipient->setHasDied(false);
    hostIf->linkToDeath(btAudio->deathRecipient, 0);
    btAudio->stream_cb_ = hostIf;
    btAudio->lib_handle = NULL;
    btAudio->hidl_callback_.hidl_start_req_cb = btAudio->start_stream_request_callback;
    btAudio->hidl_callback_.hidl_suspend_req_cb = btAudio->suspend_stream_request_callback;
    btAudio->hidl_callback_.hidl_stop_req_cb = btAudio->stop_stream_request_callback;

    if (btAudio->initialize_btlib(&btAudio->hidl_callback_) == 1) {
        ALOGI("%s: init BT IPC library successful", __PRETTY_FUNCTION__);
        btAudio->start_session(codecConfig);
    }

    unlink_cb_ = [hostIf](sp<BluetoothDeathRecipient>& death_recipient) {
        if (death_recipient->getHasDied()) {
           ALOGI("%s: skipping unlink call, service died", __PRETTY_FUNCTION__);
        } else {
           hostIf->unlinkToDeath(death_recipient);
        }
    };

    return Status::SUCCESS;
}

Return<void> BluetoothAudioOffload::streamStarted(
    ::android::hardware::bluetooth::a2dp::V1_0::Status status) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    if (btAudio != nullptr && btAudio->on_started) {
        ALOGV("%s: calling btAudio->on_started()", __PRETTY_FUNCTION__);
        btAudio->on_started(status);
    }
    return Void();
}

Return<void> BluetoothAudioOffload::streamSuspended(
    ::android::hardware::bluetooth::a2dp::V1_0::Status status) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    if (btAudio != nullptr && btAudio->on_suspended) {
        ALOGV("%s: calling btAudio->on_suspended()", __PRETTY_FUNCTION__);
        btAudio->on_suspended(status);
    }
    return Void();
}

Return<void> BluetoothAudioOffload::endSession() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    std::unique_lock<std::mutex> guard(sessionMutex);
    if (btAudio != nullptr) {
        unlink_cb_(btAudio->deathRecipient);
        deinitialize_btlib();
        delete btAudio;
        btAudio = nullptr;
    }
    ALOGD("%s: exit", __PRETTY_FUNCTION__);
    return Void();
}

void BluetoothAudioOffload::cleanup() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    std::unique_lock<std::mutex> guard(sessionMutex);
    if (btAudio != nullptr) {
        btAudio->deathRecipient->setHasDied(true);
        deinitialize_btlib();
        delete btAudio;
        btAudio = nullptr;
    }
    ALOGD("%s: exit", __PRETTY_FUNCTION__);
}

int BluetoothAudioOffload::initialize_btlib(hidl_callback_t *hidl_cb) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    btAudio->lib_handle = dlopen("libbthost_if.so", RTLD_NOW);
    if (!btAudio->lib_handle) {
        ALOGE("%s: failed to load libbthost_if.so: %s", __PRETTY_FUNCTION__, dlerror());
        return 0;
    }

    btAudio->lib_init =
        (bt_stack_init_t) dlsym(lib_handle, "bt_stack_init");
    btAudio->lib_deinit =
        (bt_stack_deinit_t) dlsym(lib_handle, "bt_stack_deinit");
    btAudio->on_started =
        (bt_stack_on_stream_started_t) dlsym(lib_handle, "bt_stack_on_stream_started");
    btAudio->on_suspended =
        (bt_stack_on_stream_suspended_t) dlsym(lib_handle, "bt_stack_on_stream_suspended");
    btAudio->on_stopped =
        (bt_stack_on_stream_stopped_t) dlsym(lib_handle, "bt_stack_on_stream_stopped");
    btAudio->start_session =
        (bt_stack_start_session_t) dlsym(lib_handle,"bt_stack_start_session");

    if (btAudio->lib_init) {
        ALOGI("%s: init BT IPC library with callbacks", __PRETTY_FUNCTION__);
        btAudio->lib_init(hidl_cb);
        return 1;
    }

    ALOGE("%s: failed to get symbols of BT IPC library", __PRETTY_FUNCTION__);
    return 0;
}

void BluetoothAudioOffload::deinitialize_btlib() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    if (btAudio != nullptr && btAudio->lib_deinit) {
        btAudio->lib_deinit(Status::SUCCESS);
        dlclose(btAudio->lib_handle);
        btAudio->lib_handle = nullptr;
    }
}

void BluetoothAudioOffload::start_stream_request_callback() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    if (btAudio != nullptr && btAudio->stream_cb_ != nullptr) {
        auto ret = btAudio->stream_cb_->startStream();
        if (!ret.isOk()) {
            ALOGE("%s: start_request_callback: client has died", __PRETTY_FUNCTION__);
        }
    } else {
        ALOGE("%s: btAudio is nullptr or BT IPC library not loaded", __PRETTY_FUNCTION__);
    }
}

void BluetoothAudioOffload::suspend_stream_request_callback() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    if (btAudio != nullptr && btAudio->stream_cb_ != nullptr) {
        auto ret = btAudio->stream_cb_->suspendStream();
        if (!ret.isOk()) {
            ALOGE("%s: suspend_request_callback: client has died", __PRETTY_FUNCTION__);
        }
    } else {
        ALOGE("%s: btAudio is nullptr or BT IPC library not loaded", __PRETTY_FUNCTION__);
    }
}

void BluetoothAudioOffload::stop_stream_request_callback() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    Status status = Status::SUCCESS;
    if (btAudio != nullptr && btAudio->stream_cb_ != nullptr) {
        btAudio->stream_cb_->stopStream();
        btAudio->on_stopped(status);
    } else {
        ALOGE("%s: btAudio is nullptr or BT IPC library not loaded", __PRETTY_FUNCTION__);
    }
}

IBluetoothAudioOffload* HIDL_FETCH_IBluetoothAudioOffload(const char* /* name */) {
    return new BluetoothAudioOffload();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace a2dp
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
