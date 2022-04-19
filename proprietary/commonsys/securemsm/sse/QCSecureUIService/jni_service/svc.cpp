/*
 * Copyright (c) 2012-2013, 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define MAIN_C
#include <string.h>
#include <errno.h>
#include <jni.h>
#include <common_log.h>
#include <sys/un.h>
#include <unistd.h>
#include <vendor/display/config/1.0/IDisplayConfig.h>

#include <SurfaceComposerClient.h>
#include <Surface.h>
#include <ISurfaceComposer.h>

#define LST_SOCKET_NAME "suilst"
#define SVC_SOCKET_NAME "suisvc"

extern "C" {

using namespace android;
using vendor::display::config::V1_0::IDisplayConfig;

#include "com_qualcomm_qti_services_secureui_SecureUIService.h"
#include "SecureDisplayInternals.h"

static struct {
  char name[UNIX_PATH_MAX];
  size_t len;
  uint32_t used;
} remote_party = {};

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_init(
    JNIEnv * /*env*/, jclass /*cls*/) {

    const char *lst_name = LST_SOCKET_NAME;
    const char *svc_name = SVC_SOCKET_NAME;

    return svc_sock_init(svc_name, strlen(svc_name),
                         lst_name, strlen(lst_name), 0);
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_terminate(
    JNIEnv * /*env*/, jclass /*cls*/) {
    return svc_sock_terminate(SVC_SOCKET_NAME);
}

extern "C" jbyteArray
Java_com_qualcomm_qti_services_secureui_SecureUIService_waitForMessage(
    JNIEnv * env, jclass /*cls*/, jbyteArray input) {

    jbyteArray jarray = NULL;
    uint8_t mex[BYTES_IN_MEX] = {0};
    jbyte* arrayBytes = NULL;
    jbyte* arrayBytes_input = NULL;
    int rv = 0;
    secdisp_ip secdisp_input;

    remote_party.len = UNIX_PATH_MAX;
    rv = svc_sock_wait_for_message(mex, BYTES_IN_MEX,
                                   &secdisp_input,
                                   remote_party.name,
                                   &remote_party.len);
    if (rv) {
      LOGE("Error waiting for a message");
      goto exit;
    }

    remote_party.used = 0;
    LOGD("Message source: %s", (char*) (remote_party.name + 1));

    jarray = (env)->NewByteArray(BYTES_IN_MEX);
    if (jarray == NULL) {
      LOGE("Error creating jarray");
      goto exit;
    }

    arrayBytes = (env)->GetByteArrayElements(jarray, NULL);
    if (arrayBytes == NULL) {
      LOGE("Error getting arrayBytes");
      goto exit;
    }

    memcpy(arrayBytes, mex, BYTES_IN_MEX);

    arrayBytes_input = (env)->GetByteArrayElements(input, NULL);
    if (arrayBytes_input == NULL) {
      LOGE("Error getting arrayBytes");
      goto exit;
    }

    memcpy(arrayBytes_input, &secdisp_input, BYTES_IN_INPUT);

exit:
  if (arrayBytes) {
    (env)->ReleaseByteArrayElements(jarray, arrayBytes, 0);
  }

  if (arrayBytes_input) {
    (env)->ReleaseByteArrayElements(input, arrayBytes_input, 0);
  }

  return jarray;
}

extern "C" jbyteArray
Java_com_qualcomm_qti_services_secureui_SecureUIService_getSource(
    JNIEnv * env, jclass /*cls*/) {

    jbyteArray jarray = NULL;
    jbyte* arrayBytes = NULL;

    if (remote_party.used == 1) {
      LOGE("Message already consumed");
      goto exit;
    }

    jarray = (env)->NewByteArray(remote_party.len);
    if (jarray == NULL) {
      LOGE("Error creating jarray");
      goto exit;
    }

    arrayBytes = (env)->GetByteArrayElements(jarray, NULL);
    if (arrayBytes == NULL) {
      LOGE("Error getting arrayBytes");
      goto exit;
    }

    memcpy(arrayBytes, remote_party.name, remote_party.len);
    LOGD("source: %s", (char*) (remote_party.name + 1));

exit:
    if (arrayBytes) {
        (env)->ReleaseByteArrayElements(jarray, arrayBytes, 0);
    }

    return jarray;
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_sendResponse(
    JNIEnv * env, jclass /*cls*/, jint id,
    jint payload, jbyteArray remote) {

    int rv = 0;
    char name[UNIX_PATH_MAX] = {0};
    size_t len = 0;
    jbyte* arrayBytes = NULL;

    if (remote != NULL) {
        len = (env)->GetArrayLength(remote);
        if (len > UNIX_PATH_MAX) {
            LOGE("Array too long: %u vs %u max",
                 (unsigned int) len, UNIX_PATH_MAX);
            goto exit;
        }

        arrayBytes = (env)->GetByteArrayElements(remote, NULL);
        if (arrayBytes == NULL) {
            LOGE("Error getting arrayBytes");
            goto exit;
        }

        memcpy(name, arrayBytes, len);
    }

    LOGD("Sending to: %s", (char*) (name + 1));
    rv = svc_sock_send_response(id, payload, name, len);

exit:
    if (arrayBytes) {
        (env)->ReleaseByteArrayElements(remote, arrayBytes, 0);
    }

    return (rv);
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_sendNotification(
    JNIEnv * env, jclass /*cls*/, jint id,
    jint payload, jbyteArray remote) {

    int rv = 0;
    char name[UNIX_PATH_MAX] = {0};
    size_t len = 0;
    jbyte* arrayBytes = NULL;

    if (remote != NULL) {
      len = (env)->GetArrayLength(remote);
      if (len > UNIX_PATH_MAX) {
        LOGE("Array too long: %u vs %u max",
             (unsigned int) len, UNIX_PATH_MAX);
        goto exit;
      }

      arrayBytes = (env)->GetByteArrayElements(remote, NULL);
      if (arrayBytes == NULL) {
        LOGE("Error getting arrayBytes");
        goto exit;
      }

      memcpy(name, arrayBytes, len);
    }

    rv = svc_sock_send_notification(id, payload, name, len);

exit:
    if (arrayBytes) {
        (env)->ReleaseByteArrayElements(remote, arrayBytes, 0);
    }

    return (rv);
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_pauseExternal(
    JNIEnv * /*env*/, jclass /*cls*/,
    jint display, jint doPause) {

    int rv = 0;
    IDisplayConfig::DisplayType type;
    sp<IDisplayConfig> client = IDisplayConfig::getService();

    if (client == NULL) {
        ALOGE("failed to get IDisplayConfig service");
        return -1;
    }

    if (display == 1) {
        type = IDisplayConfig::DisplayType::DISPLAY_EXTERNAL;
        ALOGD("pauseExternal: HDMI: %d", doPause);
    } else if (display == 2) {
        type = IDisplayConfig::DisplayType::DISPLAY_VIRTUAL;
        ALOGD("pauseExternal: WFD: %d", doPause);
    } else {
        ALOGE("Unsupported external display: %d", display);
        return -1;
    }

    if (doPause) {
        rv = client->setSecondayDisplayStatus(type,
            IDisplayConfig::DisplayExternalStatus::EXTERNAL_PAUSE);
    } else {
        rv = client->setSecondayDisplayStatus(type,
            IDisplayConfig::DisplayExternalStatus::EXTERNAL_RESUME);
    }

    if (doPause) {
        usleep(200000); // sleep 200ms
    }

    ALOGD("setSecondaryDisplayStatus: %d", rv);
    return rv;
}

extern "C" jint
Java_com_qualcomm_qti_services_secureui_SecureUIService_externalIsConnected(
    JNIEnv *, jclass) {

    int ret = -1;
    int connected = -1;
    sp<IDisplayConfig> client = IDisplayConfig::getService();

    if (client == NULL)
        return ret;

    client->isDisplayConnected(IDisplayConfig::DisplayType::DISPLAY_EXTERNAL,
        [&](const auto &tmpError, const auto &tmpStatus) {
            if (tmpError) {
                return;
            }
            ret = tmpStatus;
        });

    if (ret > 0) {
        ALOGD("External connected");
        connected = 1;
    } else if (ret == 0) {
        ALOGD("External not connected");
        connected = 0;
    }

    return connected;
}

}
