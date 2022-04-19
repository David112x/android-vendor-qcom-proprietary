/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "BluetoothBATServiceJni"

#include "android_runtime/AndroidRuntime.h"
#include "com_android_bluetooth.h"
#include "hardware/bt_ba.h"
#include "utils/Log.h"

#include <string.h>

namespace android {
static jmethodID method_onBAStateChanged;
static jmethodID method_onEncKeyUpdateCallback;
static jmethodID method_onDivUpdateCallback;
static jmethodID method_onStreamIdUpdateCallback;

static const ba_transmitter_interface_t* sBATInterface = NULL;
static jobject mCallbacksObj = NULL;

static void ba_state_change_callback(ba_state_t state) {
  ALOGI("%s", __func__);
  CallbackEnv sCallbackEnv(__func__);
  if (!sCallbackEnv.valid()) return;

  sCallbackEnv->CallVoidMethod(mCallbacksObj, method_onBAStateChanged,(jint)state);
}

static void ba_enc_key_update_callback(uint8_t size, uint8_t* p_enc_key) {
  ALOGI("%s", __func__);
  CallbackEnv sCallbackEnv(__func__);
  if (!sCallbackEnv.valid()) return;

  ScopedLocalRef<jbyteArray> enc_key(
     sCallbackEnv.get(), sCallbackEnv->NewByteArray(size));
  if (!enc_key.get()) {
      ALOGE("Fail to get new jbyteArray for enc key, ba_enc_key_update_callback");
      return;
  }

  sCallbackEnv->SetByteArrayRegion(enc_key.get(), 0, size, (jbyte*)p_enc_key);

  sCallbackEnv->CallVoidMethod(mCallbacksObj, method_onEncKeyUpdateCallback,(jbyte)size,
                                                                            enc_key.get());
}

static void ba_div_update_callback(uint8_t size, uint8_t* p_div) {
  ALOGI("%s", __func__);
  CallbackEnv sCallbackEnv(__func__);
  if (!sCallbackEnv.valid()) return;

  ScopedLocalRef<jbyteArray> div(
     sCallbackEnv.get(), sCallbackEnv->NewByteArray(size));
  if (!div.get()) {
      ALOGE("Fail to get new jbyteArray for enc key, ba_div_update_callback");
      return;
  }

  sCallbackEnv->SetByteArrayRegion(div.get(), 0, size, (jbyte*)p_div);

  sCallbackEnv->CallVoidMethod(mCallbacksObj, method_onDivUpdateCallback, (jbyte)size, div.get());
}

static void ba_stream_id_update_callback(uint8_t stream_id) {
  ALOGI("%s", __func__);
  CallbackEnv sCallbackEnv(__func__);
  if (!sCallbackEnv.valid()) return;

  sCallbackEnv->CallVoidMethod(mCallbacksObj, method_onStreamIdUpdateCallback, (jbyte)stream_id);
}

static ba_transmitter_callbacks_t sBATCallbacks = {
    sizeof(sBATCallbacks),
    ba_state_change_callback,
    ba_enc_key_update_callback,
    ba_div_update_callback,
    ba_stream_id_update_callback,
};

static void classInitNative(JNIEnv* env, jclass clazz) {

  ALOGI("%s: Enter ", __func__);
  method_onBAStateChanged =
      env->GetMethodID(clazz, "onBATStateChanged", "(I)V");

  method_onEncKeyUpdateCallback =
      env->GetMethodID(clazz, "onEncKeyUpdate", "(B[B)V");

  method_onDivUpdateCallback =
      env->GetMethodID(clazz, "onDivUpdate", "(B[B)V");

  method_onStreamIdUpdateCallback =
      env->GetMethodID(clazz, "onStreamIdUpdate", "(B)V");

  ALOGI("%s: succeeds", __func__);
}

static void initNative(JNIEnv* env, jobject object) {
  ALOGI("%s: Enter ", __func__);
  const bt_interface_t* btInf = getBluetoothInterface();
  if (btInf == NULL) {
    ALOGE("Bluetooth module is not loaded");
    return;
  }

  if (sBATInterface != NULL) {
    ALOGW("Cleaning up BA Transmitter Interface before initializing...");
    sBATInterface->cleanup();
    sBATInterface = NULL;
  }

  if (mCallbacksObj != NULL) {
    ALOGW("Cleaning up BA Transmitter callback object");
    env->DeleteGlobalRef(mCallbacksObj);
    mCallbacksObj = NULL;
  }

  if ((mCallbacksObj = env->NewGlobalRef(object)) == NULL) {
    ALOGE("Failed to allocate Global Ref for BA Transmitter Callbacks");
    return;
  }

  sBATInterface =
      (ba_transmitter_interface_t*)btInf->get_profile_interface(BT_PROFILE_BAT_ID);
  if (sBATInterface == NULL) {
    ALOGE("Failed to get BA Transmitter Interface");
    return;
  }

  bt_status_t status = sBATInterface->init(&sBATCallbacks);
  if (status != BT_STATUS_SUCCESS) {
    ALOGE("Failed to initialize BA Transmitter, status: %d", status);
    sBATInterface = NULL;
    return;
  }
  ALOGI("%s: Success ", __func__);
}

static void cleanupNative(JNIEnv* env, jobject object) {
  const bt_interface_t* btInf = getBluetoothInterface();
  if (btInf == NULL) {
    ALOGE("Bluetooth module is not loaded");
    return;
  }

  if (sBATInterface != NULL) {
    sBATInterface->cleanup();
    sBATInterface = NULL;
  }

  if (mCallbacksObj != NULL) {
    env->DeleteGlobalRef(mCallbacksObj);
    mCallbacksObj = NULL;
  }
}

static void refreshEncKeyNative(JNIEnv* env, jobject object) {
    if (sBATInterface == NULL) {
      ALOGE("Failed to get BA Transmitter Interface in refreshEncKeyNative");
      return;
    }
    sBATInterface->refresh_enc_key();
}

static void setBAStateNative(JNIEnv* env, jobject object, jint state) {
    if (sBATInterface == NULL) {
      ALOGE("Failed to get BA Transmitter Interface in setBAStateNative ");
      return;
    }
    sBATInterface->set_state(state);
}

static void setVolNative(JNIEnv* env, jobject object, jint volLevel, jint maxVolLevel) {
    if (sBATInterface == NULL) {
      ALOGE("Failed to get BA Transmitter Interface in setVolNative ");
      return;
    }
    sBATInterface->set_vol(volLevel, maxVolLevel);
}

static JNINativeMethod sMethods[] = {
    {"classInitNative", "()V", (void*)classInitNative},
    {"initNative", "()V", (void*)initNative},
    {"refreshEncKeyNative", "()V", (void*)refreshEncKeyNative},
    {"cleanupNative", "()V", (void*)cleanupNative},
    {"setBAStateNative", "(I)V", (void*)setBAStateNative},
    {"setVolNative", "(II)V", (void*)setVolNative},
};

int register_com_android_bluetooth_ba(JNIEnv* env) {
  return jniRegisterNativeMethods(env,
                                  "com/android/bluetooth/ba/BATService",
                                  sMethods, NELEM(sMethods));
}
}
