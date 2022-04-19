/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#define LOG_TAG "BluetoothVendorHfJni"

#include "com_android_bluetooth.h"
#include <hardware/vendor_hf.h>
#include "utils/Log.h"
#include "android_runtime/AndroidRuntime.h"

namespace android {
static btvendor_hf_interface_t *sBluetoothVendorHfInterface = NULL;
static jobject mCallbacksObj = NULL;
static jmethodID method_onSWB;
static jmethodID method_onTwsBatteryStateCallback;

static jbyteArray marshall_bda(RawAddress* bd_addr) {
  CallbackEnv sCallbackEnv(__func__);
  if (!sCallbackEnv.valid()) return nullptr;

  jbyteArray addr = sCallbackEnv->NewByteArray(sizeof(RawAddress));
  if (!addr) {
    ALOGE("Fail to new jbyteArray bd addr");
    return nullptr;
  }
  sCallbackEnv->SetByteArrayRegion(addr, 0, sizeof(RawAddress),
                                   (jbyte*)bd_addr);
  return addr;
}

static void SwbCallback(uint16_t swb_config, RawAddress* bd_addr) {

    ALOGI("%s", __FUNCTION__);
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid()) return;

    ScopedLocalRef<jbyteArray> addr(sCallbackEnv.get(), marshall_bda(bd_addr));
    if (addr.get() == nullptr) return;
    sCallbackEnv->CallVoidMethod(mCallbacksObj, method_onSWB, swb_config,
                                 addr.get());
}

void bt_twsp_battery_status_callback(char* at_string, RawAddress* bd_addr) {
    ALOGD("TwsBatteryStateCallback :%s, BD_ADDR: %s",
          at_string, bd_addr->ToString().c_str());
    CallbackEnv sCallbackEnv(__func__);
    if (!sCallbackEnv.valid() || !mCallbacksObj) return;

    ScopedLocalRef<jbyteArray> addr(sCallbackEnv.get(), marshall_bda(bd_addr));
    if (addr.get() == nullptr) return;

    char null_str[] = "";
    if (!sCallbackEnv.isValidUtf(at_string)) {
      android_errorWriteLog(0x534e4554, "109838537");
      ALOGE("%s: at_string is not a valid UTF string.", __func__);
      at_string = null_str;
    }

    ScopedLocalRef<jstring> js_at_string(sCallbackEnv.get(),
                                         sCallbackEnv->NewStringUTF(at_string));

    sCallbackEnv->CallVoidMethod(mCallbacksObj, method_onTwsBatteryStateCallback,
                                 js_at_string.get(), addr.get());
}

static btvendor_hf_callbacks_t sBluetoothVendorHfCallbacks = {
    sizeof(sBluetoothVendorHfCallbacks),
    SwbCallback,
    bt_twsp_battery_status_callback
};

static void classInitNative(JNIEnv* env, jclass clazz) {

    method_onSWB = env->GetMethodID(clazz, "onSWB", "(I[B)V");
    method_onTwsBatteryStateCallback =
         env->GetMethodID(clazz, "onTwsBatteryStateCallback",
                                 "(Ljava/lang/String;[B)V");
    ALOGI("%s: succeeds", __FUNCTION__);
}

static void initNative(JNIEnv *env, jobject object) {
    const bt_interface_t* btInf;
    bt_status_t status;

    if ( (btInf = getBluetoothInterface()) == NULL) {
        ALOGE("Bluetooth module is not loaded");
        return;
    }

    if (mCallbacksObj != NULL) {
        ALOGW("Cleaning up Bluetooth Vendor callback object");
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }

    if ( (sBluetoothVendorHfInterface = (btvendor_hf_interface_t *)
          btInf->get_profile_interface(BT_PROFILE_VENDOR_HF_ID)) == NULL) {
        ALOGE("Failed to get Bluetooth Vendor Interface");
        return;
    }

    if ( (status = sBluetoothVendorHfInterface->init(&sBluetoothVendorHfCallbacks))
                 != BT_STATUS_SUCCESS) {
        ALOGE("Failed to initialize Bluetooth Vendor, status: %d", status);
        sBluetoothVendorHfInterface = NULL;
        return;
    }
    mCallbacksObj = env->NewGlobalRef(object);
}

static void cleanupNative(JNIEnv *env, jobject object) {
    const bt_interface_t* btInf;

    if ( (btInf = getBluetoothInterface()) == NULL) {
        ALOGE("Bluetooth module is not loaded");
        return;
    }

    if (sBluetoothVendorHfInterface !=NULL) {
        ALOGW("Cleaning up Bluetooth Vendor Interface...");
        sBluetoothVendorHfInterface->cleanup();
        sBluetoothVendorHfInterface = NULL;
    }

    if (mCallbacksObj != NULL) {
        ALOGW("Cleaning up Bluetooth Vendor callback object");
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }

}

/* native interface */
static jint enableSwbNative(JNIEnv* env, jobject thiz, jboolean enable)
{
    if (sBluetoothVendorHfInterface == NULL) {
        ALOGE("No Interface initialized");
        return JNI_FALSE;
    }

    int ret = sBluetoothVendorHfInterface->enable_swb(enable);

    if (ret != 0) {
        ALOGE("%s: Failure", __func__);
        return JNI_FALSE;
    } else {
        ALOGV("%s: Success :%d", __func__, enable);
    }

    return JNI_TRUE;
}

static JNINativeMethod sMethods[] = {
    {"classInitNative", "()V", (void *) classInitNative},
    {"initNative", "()V", (void *) initNative},
    {"cleanupNative", "()V", (void *) cleanupNative},
    { "enableSwbNative", "(Z)I", (void*)enableSwbNative},
};

int register_com_android_bluetooth_hfp_vendorhfservice(JNIEnv* env)
{
    ALOGE("%s:",__FUNCTION__);
    return jniRegisterNativeMethods(env, "com/android/bluetooth/hfp/vendorhfservice",
                                    sMethods, NELEM(sMethods));
}

} /* namespace android */
