/*
 * Copyright (c) 2012-2013, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <jni.h>
/* Header for class com_qualcomm_qti_services_secureui_SecureUIService */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    init
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_init
  (JNIEnv *, jclass);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    terminate
 * Signature: ()
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_terminate
  (JNIEnv *, jclass);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    wait_for_message
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_waitForMessage
  (JNIEnv *, jclass, jbyteArray);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    getSource
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_getSource
  (JNIEnv *, jclass);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    send_response
 * Signature: (II[B)I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_sendResponse
  (JNIEnv *, jclass, jint, jint, jbyteArray);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    send_notification
 * Signature: (II[B])I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_sendNotification
  (JNIEnv *, jclass, jint, jint, jbyteArray);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    pauseExternal
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_pauseExternal
  (JNIEnv *, jclass, jint, jint);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    externalIsConnected
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_externalIsConnected
  (JNIEnv *, jclass);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    getdispprop
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_getdispprop
  (JNIEnv * env, jclass cls, jint, jint, jbyteArray input, jbyteArray remote);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    startdisp
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_startdisp
  (JNIEnv * env, jclass cls,jint, jint, jbyteArray input, jbyteArray remote);


/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    secuidequeue
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_secuidequeue
  (JNIEnv * env, jclass cls, jint, jint,jbyteArray input, jbyteArray remote);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    secuienqueue
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_secuienqueue
  (JNIEnv * env, jclass cls,jint, jint, jbyteArray input, jbyteArray remote);

/*
 * Class:     com_qualcomm_qti_services_secureui_SecureUIService
 * Method:    stopdisp 
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_qti_services_secureui_SecureUIService_stopdisp
  (JNIEnv * env, jclass cls,jint, jint, jbyteArray input, jbyteArray remote);

#ifdef __cplusplus
}
#endif
