/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

#define LOG_TAG "BluetoothVendorSocketJni"

#include "com_android_bluetooth.h"
#include <hardware/vendor_socket.h>
#include "utils/Log.h"
#include "android_runtime/AndroidRuntime.h"


namespace android {

static btvendor_socket_interface_t *sBluetoothVendorSocketInterface = NULL;
static jobject mCallbacksObj = NULL;

static void classInitNative(JNIEnv* env, jclass clazz) {

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

    if ( (sBluetoothVendorSocketInterface = (btvendor_socket_interface_t *)
          btInf->get_profile_interface(BT_PROFILE_VENDOR_SOCKET_ID)) == NULL) {
        ALOGE("Failed to get Bluetooth Vendor Interface");
        return;
    }

    if ( (status = sBluetoothVendorSocketInterface->init(NULL))
                 != BT_STATUS_SUCCESS) {
        ALOGE("Failed to initialize Bluetooth Vendor, status: %d", status);
        sBluetoothVendorSocketInterface = NULL;
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

    if (sBluetoothVendorSocketInterface !=NULL) {
        ALOGW("Cleaning up Bluetooth Vendor Interface...");
        sBluetoothVendorSocketInterface->cleanup();
        sBluetoothVendorSocketInterface = NULL;
    }

    if (mCallbacksObj != NULL) {
        ALOGW("Cleaning up Bluetooth Vendor callback object");
        env->DeleteGlobalRef(mCallbacksObj);
        mCallbacksObj = NULL;
    }

}


static int getSocketOptNative(JNIEnv *env, jobject obj, jint type, jint channel, jint optionName,
                                        jbyteArray optionVal) {
    ALOGV("%s:",__FUNCTION__);

    jbyte *option_val = NULL;
    int option_len;
    bt_status_t status;

    if (!sBluetoothVendorSocketInterface) return -1;

    option_val = env->GetByteArrayElements(optionVal, NULL);
    if (option_val == NULL) {
        ALOGE("getSocketOptNative :jniThrowIOException ");
        jniThrowIOException(env, EINVAL);
        return -1;
    }

    if ( (status = sBluetoothVendorSocketInterface->get_sock_opt((btsock_type_t)type, channel,
         (btsock_option_type_t) optionName, (void *) option_val, &option_len)) !=
                                                           BT_STATUS_SUCCESS) {
        ALOGE("get_sock_opt failed: %d", status);
        goto Fail;
    }
    env->ReleaseByteArrayElements(optionVal, option_val, 0);

    return option_len;
Fail:
    env->ReleaseByteArrayElements(optionVal, option_val, 0);
    return -1;
}

static int setSocketOptNative(JNIEnv *env, jobject obj, jint type, jint channel, jint optionName,
                                        jbyteArray optionVal, jint optionLen) {
    ALOGV("%s:",__FUNCTION__);

    jbyte *option_val = NULL;
    bt_status_t status;

    if (!sBluetoothVendorSocketInterface) return -1;

    option_val = env->GetByteArrayElements(optionVal, NULL);
    if (option_val == NULL) {
        ALOGE("setSocketOptNative:jniThrowIOException ");
        jniThrowIOException(env, EINVAL);
        return -1;
    }

    if ( (status = sBluetoothVendorSocketInterface->set_sock_opt((btsock_type_t)type, channel,
         (btsock_option_type_t) optionName, (void *) option_val, optionLen)) !=
                                                         BT_STATUS_SUCCESS) {
        ALOGE("set_sock_opt failed: %d", status);
        goto Fail;
    }
    env->ReleaseByteArrayElements(optionVal, option_val, 0);

    return 0;
Fail:
    env->ReleaseByteArrayElements(optionVal, option_val, 0);
    return -1;
}


static JNINativeMethod sMethods[] = {
    {"classInitNative", "()V", (void *) classInitNative},
    {"initNative", "()V", (void *) initNative},
    {"cleanupNative", "()V", (void *) cleanupNative},
    {"getSocketOptNative", "(III[B)I", (void*) getSocketOptNative},
    {"setSocketOptNative", "(III[BI)I", (void*) setSocketOptNative}
};

int register_com_android_bluetooth_btservice_vendor_socket(JNIEnv* env)
{
    ALOGE("%s:",__FUNCTION__);
    return jniRegisterNativeMethods(env, "com/android/bluetooth/btservice/VendorSocket",
                                    sMethods, NELEM(sMethods));
}

} /* namespace android */
