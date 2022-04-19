/*
 * Copyright (c) 2012-2013, 2015, 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOGV(...) do {} while (0)  //Comment this line to enable LOGV
#define LOGD(...) do {} while (0)  //Comment this line to enable LOGD
#include <jni.h>
#include <utils/RefBase.h>
#include <SurfaceComposerClient.h>
#include <Surface.h>
#include <ISurfaceComposer.h>
#include <DisplayInfo.h>
#include <PixelFormat.h>

#include <utils/Log.h>
#include <sys/stat.h>
#include <hidl/LegacySupport.h>
#include <hidl/HidlTransportSupport.h>

#include "common_log.h"
#include "gralloc_priv.h"

#include <tuple>
#include <vector>

extern "C" {

class SecDisp *g_pSd = NULL;
using namespace android;

#include "com_qualcomm_qti_services_secureui_SecureUIService.h"
#include "SecureDisplayInternals.h"

#define CHECK_DISP_INIT(disp) \
    if (!(disp)) { \
        LOGE("%s: Secure display is not initialized", __func__); \
        return FAILURE; \
    }

#define VALIDATE_INPUT(cond) \
    if ((!(cond))) { \
        LOGE("%s: invalid input", __func__); \
        return FAILURE; \
    }

SecDisp::SecDisp(uint32_t width, uint32_t height) {
    numAllocatedBufs = 0;
    mSurfaceComposer = NULL;
    mSurface = NULL;
    mControl = NULL;
    mNativeWindow = NULL;

    mDisplayWidth = width;
    mDisplayHeight = height;

    LOGD("SecDisp created with W x H = %d X %d\n",
         mDisplayWidth, mDisplayHeight);
}

int32_t SecDisp::init() {
    int32_t ret = FAILURE;
    SurfaceComposerClient::Transaction t;

    mSurfaceComposer = new SurfaceComposerClient();
    if (mSurfaceComposer->initCheck() != (status_t) OK) {
       LOGE("mSurfaceComposer->initCheck() failed");
       goto exit;
    }

    mControl = mSurfaceComposer->createSurface(String8("secure-ui"),
                                               mDisplayWidth,
                                               mDisplayHeight,
                                               PIXEL_FORMAT_OPAQUE);
    if ((mControl == NULL) || !(mControl->isValid())) {
        LOGE("failed to create surface control");
        goto exit;
    }

    t.setLayer(mControl, INT_MAX - 1);
    if (t.apply(true  /* synchronous */) != OK) {
        LOGE("setLayer failed");
        goto exit;
    }
    mSurface = mControl->getSurface();
    if (mSurface == NULL) {
       LOGE("getSurface failed");
       goto exit;
    }
    mNativeWindow = mSurface;

   ret = native_window_api_connect(mNativeWindow.get(),
                                   NATIVE_WINDOW_API_MEDIA);
   if (ret) {
       LOGE("native_window_api_connect failed %d", ret);
       goto exit;
   }

   LOGD("SecDisp init done\n");

exit:
    return ret;
}

int32_t SecDisp::setConfig(const Config& config) {
    mConfig.width = config.width;
    mConfig.height = config.height;
    mConfig.colorFormat = config.colorFormat;
    mConfig.usageFlags = config.usageFlags;
    mConfig.x = config.x;
    mConfig.y = config.y;

    return SUCCESS;
}


int32_t SecDisp::allocateNativeBuffers(uint32_t buffersForHeap) {

    /* 2 buffers to guarantee triple-buffering */
    const uint32_t numRequestedBufs = 2 + buffersForHeap;
    status_t err = 0;

    err = native_window_set_scaling_mode(mNativeWindow.get(),
                                         NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    if (err != OK) {
        LOGE("native_window_set_scaling_mode failed");
        return FAILURE;
    }

    SurfaceComposerClient::Transaction t;
    t.setPosition(mControl, mConfig.x, mConfig.y);
    t.setSize(mControl, mConfig.width, mConfig.height);
    t.setFlags(mControl, layer_state_t::eLayerOpaque,
               layer_state_t::eLayerOpaque);
    err = t.apply(true /* synchronous */);
    if (err != OK) {
        LOGE("native_window_set_buffers_geometry failed: %d", err);
        return FAILURE;
    }

    err = native_window_set_buffers_format(mNativeWindow.get(),
                                           mConfig.colorFormat);
    if (err != OK) {
        LOGE("native_window_set_buffers_format failed: %d", err);
        return FAILURE;
    }

    err = native_window_set_buffers_dimensions(mNativeWindow.get(),
                                               mConfig.width,
                                               mConfig.height);
    if (err != OK) {
        LOGE("native_window_set_buffers_dimentions failed: %d", err);
        return FAILURE;
    }

    // Make sure to check whether protected buffers are requested.
    if (mConfig.usageFlags & GRALLOC1_PRODUCER_USAGE_PROTECTED ) {
        // Verify that the ANativeWindow sends images directly to
        // SurfaceFlinger.
        int queuesToNativeWindow = 0;
        err = mNativeWindow->query(mNativeWindow.get(),
                                   NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER,
                                   &queuesToNativeWindow);
        if (err != 0) {
            LOGE("error authenticating native window: %d", err);
            return FAILURE;
        }

        if (queuesToNativeWindow != 1) {
            LOGE("native window could not be authenticated");
            return FAILURE;
        }
    }

    err = native_window_set_usage(mNativeWindow.get(),
                                  mConfig.usageFlags);
    if (err != 0) {
        LOGE("native_window_set_usage failed: %s (%d)",
             strerror(-err), -err);
        return FAILURE;
    }

    int minUndequeuedBufs = 0;
    err = mNativeWindow->query(mNativeWindow.get(),
                               NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
                               &minUndequeuedBufs);
    if (err != 0) {
        LOGE("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS query failed: %s (%d)",
             strerror(-err), -err);
        return FAILURE;
    }

    LOGD("minUndequeuedBufs = %d", minUndequeuedBufs);
    numAllocatedBufs = numRequestedBufs + minUndequeuedBufs;

    err = native_window_set_buffer_count(mNativeWindow.get(),
                                         numAllocatedBufs);
    if (err != 0) {
        LOGE("native_window_set_buffer_count failed: %s (%d)",
             strerror(-err), -err);
        return FAILURE;
    }

    LOGD("Requested %d buffers", numAllocatedBufs);
    return SUCCESS;
}


int32_t SecDisp::dequeue(ANativeWindowBuffer* &dispBuf)
{
    ANativeWindowBuffer* buf;
    int err = 0;

    err = native_window_dequeue_buffer_and_wait(mNativeWindow.get(), &buf);
    if (err != 0) {
        LOGE("buffer dequeue failed w/ error 0x%08x", err);
        return FAILURE;
    }

    dispBuf = buf;
    return SUCCESS;
}

int32_t SecDisp::enqueue(ANativeWindowBuffer* dispBuf, bool render) {
    int err = 0;
    const char *operation;

    operation = (render) ? "queue" : "cancel";
    LOGD("%s: buffer [%p]", operation, dispBuf);

    if (render) {
        err = mNativeWindow->queueBuffer(mNativeWindow.get(), dispBuf, -1);
    } else {
        err = mNativeWindow->cancelBuffer(mNativeWindow.get(), dispBuf, -1);
    }

    if (err != 0) {
        LOGE("buffer %s failed w/ error 0x%08x", operation, err);
        return FAILURE;
    }

    return SUCCESS;
}

SecDisp::~SecDisp() {
    // allow the pipes to be removed before the buffers are freed
    if (mSurfaceComposer && mControl &&
        (mSurfaceComposer.get() != NULL) &&
        (mControl.get() != NULL)) {
        SurfaceComposerClient::Transaction t;
        t.hide(mControl);
        auto err = t.apply(false /* asynchronous */);
        if (err != OK) {
            LOGE("%s: SurfaceComposerClient transaction failed (error %d)", __func__, err);
        }
    }
    if (mNativeWindow) {
       native_window_api_disconnect(mNativeWindow.get(), NATIVE_WINDOW_API_MEDIA);
    }
    if (mSurface && (mSurface.get() != NULL)) {
        mSurface.clear();
        mSurface = NULL;
    }
    if (mControl && (mControl.get() != NULL)) {
        mControl.clear();
        mControl = NULL;
    }
    if (mSurfaceComposer && (mSurfaceComposer.get() != NULL)) {
        mSurfaceComposer->dispose();
        mSurfaceComposer.clear();
        mSurfaceComposer = NULL;
    }
}

int32_t sec_ui_get_disp_properties(uint32_t u32Height, uint32_t u32Width) {
    int32_t ret = FAILURE;

    if (!g_pSd) {
        g_pSd = new SecDisp(u32Width, u32Height);
        if (!g_pSd) {
            goto exit;
        }
        if (g_pSd->init() != SUCCESS) {
            delete g_pSd;
            g_pSd = NULL;
            goto exit;
        }
    }
    ret = SUCCESS;

exit:
    return ret;
}

int32_t sec_ui_start_disp(bool bDisableOverlay, uint32_t colorFormat,
                          uint32_t u32Height, uint32_t u32Width,
                          uint32_t u32HPos, uint32_t u32VPos,
                          uint16_t *u16NumBuffers) {

    SecDisp::Config config;
    CHECK_DISP_INIT(g_pSd);
    VALIDATE_INPUT((u16NumBuffers != NULL));

    if ((u32Height + u32VPos > g_pSd->mDisplayHeight) ||
        (u32Width + u32HPos) > g_pSd->mDisplayWidth ) {
        LOGE("%s: bad parameters: u32Height = %d, u32VPos = %d, "
             "g_pSd->mDisplayHeight = %d, u32Width =%d, u32HPos = %d, "
             "g_pSd->mDisplayWidth = %d", u32Height, u32VPos,
             g_pSd->mDisplayHeight, u32Width, u32HPos,
             g_pSd->mDisplayWidth);
        return FAILURE;
    }

    // Usage flags for securing the buffer
    config.usageFlags = GRALLOC1_PRODUCER_USAGE_PROTECTED;
    config.usageFlags |= GRALLOC1_PRODUCER_USAGE_PRIVATE_UNCACHED;
    config.usageFlags |= GRALLOC_USAGE_PRIVATE_SECURE_DISPLAY;

    if (bDisableOverlay) {
        if ((u32Height != g_pSd->mDisplayHeight) ||
            (u32Width != g_pSd->mDisplayWidth) ||
            (u32VPos != 0) ||
            (u32HPos != 0)) {
            LOGE("%s: Overlay disable only supported in full screen mode", __func__);
            return FAILURE;
        }
    }

    config.height = u32Height;
    config.width = u32Width;
    config.x = u32HPos;
    config.y = u32VPos;
    config.colorFormat = colorFormat;

    if (g_pSd->setConfig(config) != 0)
        return FAILURE;

    if (g_pSd->allocateNativeBuffers(*u16NumBuffers) != 0)
        return FAILURE;

    if(g_pSd->numAllocatedBufs == 0) return FAILURE;

    *u16NumBuffers = g_pSd->numAllocatedBufs;
    return SUCCESS;
}

int32_t sec_ui_dequeue(uintptr_t *bufHandle, uint32_t *ionFd,
                       uint32_t *u32Stride, uint32_t *u32PixelWidth) {

    ANativeWindowBuffer* dispBuf;

    CHECK_DISP_INIT(g_pSd);
    VALIDATE_INPUT(((bufHandle != NULL) && (ionFd != NULL) &&
                    (u32Stride != NULL) && (u32PixelWidth != NULL)));

    if(g_pSd->dequeue(dispBuf) != 0) {
        return FAILURE;
    }

    *bufHandle = (uintptr_t) dispBuf;
    *ionFd = (uint32_t) (dispBuf->handle->data[0]);
    *u32Stride = dispBuf->stride;
    *u32PixelWidth = (uint32_t) bytesPerPixel(g_pSd->mConfig.colorFormat);

    return SUCCESS;
}

int32_t sec_ui_enqueue(uintptr_t bufHandle, uint8_t bToRender) {
    CHECK_DISP_INIT(g_pSd);
    return (g_pSd->enqueue((ANativeWindowBuffer*) bufHandle, bToRender));
}

int32_t sec_ui_stop_disp() {
    CHECK_DISP_INIT(g_pSd);
    delete(g_pSd);
    g_pSd = NULL;
    return SUCCESS;
}

class JavaArrayDeleter {
public:
    JavaArrayDeleter(JNIEnv * env, jbyteArray array): env_(env), array_(array) {}
    void operator()(jbyte* ptr) {
        if (ptr) env_->ReleaseByteArrayElements(array_, ptr, 0);
    }
private:
    JNIEnv * env_;
    jbyteArray array_;
};

using JavaByteArrayPtr = std::unique_ptr<jbyte, JavaArrayDeleter>;

static std::tuple<int, std::string, secdisp_ip> unpackJavaParams(
    JNIEnv * env, jbyteArray input, jbyteArray remote) {

    std::tuple<int, std::string, secdisp_ip> result;
    int &ret = std::get<0>(result);
    std::string &remote_name = std::get<1>(result);
    secdisp_ip &input_struct = std::get<2>(result);
    ret = -1;

    if (!remote) {
        LOGE("no remote name given");
        return result;
    }

    if (!input) {
        LOGE("no input given");
        return result;
    }

    {
        auto len = (env)->GetArrayLength(remote);
        if (!len) {
            LOGE("remote name lenght cannot be 0");
            return result;
        }

        auto bytes = JavaByteArrayPtr((env)->GetByteArrayElements(remote, 0), {env, remote});
        if (!bytes) {
            LOGE("failed to get remote name bytes");
            return result;
        }

        remote_name.assign(bytes.get(), bytes.get() + len);
    }

    {
        auto len = (env)->GetArrayLength(input);
        if (len != sizeof(secdisp_ip)) {
            LOGE("input bytearray has invalid size %d (expected %zu)", len, sizeof(secdisp_ip));
            return result;
        }
        auto bytes = JavaByteArrayPtr((env)->GetByteArrayElements(input, 0), {env, input});
        if (!bytes) {
            LOGE("failed to get input bytearray");
            return result;
        }
        memcpy(&input_struct, bytes.get(), sizeof(secdisp_ip));
    }

    ret = 0;
    return result;
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_getdispprop(
    JNIEnv * env, jclass /*cls*/, jint id, jint payload,
    jbyteArray input, jbyteArray remote) {

    int ret = 0;
    std::string remote_name;
    secdisp_ip ip = {};
    secdisp_op op = {};

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) {
       op.status = ret;
       return svc_sock_send_response_getdisp(id, payload,
                                             &op, NULL, 0);
    }

    op.status = sec_ui_get_disp_properties(ip.u32Height, ip.u32Width);
    return svc_sock_send_response_getdisp(id, payload,
                                          &op, remote_name.c_str(),
                                          remote_name.length());
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_startdisp(
    JNIEnv * env, jclass /*cls*/,jint id, jint payload,
    jbyteArray input, jbyteArray remote) {

    int ret = 0;
    std::string remote_name;
    secdisp_ip ip = {};
    secdisp_op op = {};

    op.u16NumBuffers = 0;
    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) {
       op.status = ret;
       return svc_sock_send_response_getdisp(id, payload,
                                          &op, NULL, 0);
    }

    op.status = sec_ui_start_disp((bool) ip.bDisableOverlay,
                                  (uint32_t) ip.colorFormat,
                                  (uint32_t) ip.u32Height,
                                  (uint32_t) ip.u32Width,
                                  (uint32_t) ip.u32HPos,
                                  (uint32_t) ip.u32VPos,
                                  &op.u16NumBuffers);

    return svc_sock_send_response_getdisp(id, payload,
                                          &op, remote_name.c_str(),
                                          remote_name.length());
}

static android::hardware::hidl_handle wrapFd(int fd) {
    if (fd >= 0) {
        auto *handle = native_handle_create(1, 0);
        if (NULL != handle) {
            handle->data[0] = fd;
            android::hardware::hidl_handle hidl_handle;
            hidl_handle.setTo(handle, true /*take ownership*/);
            return hidl_handle;
        }
    }
    return {};
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_secuidequeue(
    JNIEnv * env, jclass /*cls*/,jint id, jint payload,
    jbyteArray input, jbyteArray remote) {

    int32_t ret = 0;
    secdisp_ip ip = {};
    secdisp_op op = {};
    std::string remote_name;
    uint32_t ionFd;

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) {
       op.status = ret;
       return svc_sock_send_response_getdisp(id, payload,
                                             &op, NULL, 0);
    }

    op.status = sec_ui_dequeue((uintptr_t *)&op.bufHandle,
                               &ionFd, &op.u32Stride,
                               &op.u32PixelWidth);

    if(ip.bToRender && !op.status) {
        op.status = 0xf;
        auto native_handle = wrapFd(ionFd);
        op.fd = native_handle;
    }

    return svc_sock_send_response_getdisp(id, payload, &op,
                                          remote_name.c_str(),
                                          remote_name.length());
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_secuienqueue(
    JNIEnv * env, jclass /*cls*/,jint id, jint payload,
    jbyteArray input, jbyteArray remote) {

    int32_t ret = 0;
    std::string remote_name;
    secdisp_ip ip = {};
    secdisp_op op = {};

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) {
       op.status = ret;
       return svc_sock_send_response_getdisp(id, payload,
                                             &op, NULL, 0);
    }

    op.status = sec_ui_enqueue((uintptr_t) ip.bufHandle,
                               (uint8_t) ip.bToRender);

    return svc_sock_send_response_getdisp(id, payload, &op,
                                          remote_name.c_str(),
                                          remote_name.length());
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_stopdisp(
    JNIEnv * env, jclass /*cls*/,jint id, jint payload,
    jbyteArray input, jbyteArray remote) {

    int32_t ret = 0;
    std::string remote_name;
    secdisp_ip ip = {};
    secdisp_op op = {};

    std::tie(ret, remote_name, ip) = unpackJavaParams(env, input, remote);
    if (ret) {
       op.status = ret;
       return svc_sock_send_response_getdisp(id, payload,
                                             &op, NULL, 0);
    }

    op.status = sec_ui_stop_disp();
    return svc_sock_send_response_getdisp(id, payload, &op,
                                          remote_name.c_str(),
                                          remote_name.length());
}

}
