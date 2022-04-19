/* Copyright (c) 2014-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef VT_JNI_INTERFACE_H
#define VT_JNI_INTERFACE_H

#include <jni.h>
#include <cutils/log.h>
#include <utils/StrongPointer.h>
#include <utils/RefBase.h>
#include <android/native_window_jni.h>

jclass gClassImsMedia;

typedef void (*IMS_EVENT_NOTIFY_CALLBACK2)(int, int);
typedef void (*ImsMediaRegisterCbFuncPtr2)(IMS_EVENT_NOTIFY_CALLBACK2);
typedef int (*ImsMediaInitFuncPtr)(void);
typedef short (*ImsMediaFrameFuncPtr)(unsigned short *, int);
typedef int (*ImsMediaSetSurfFuncPtr)(JNIEnv *, jobject);
typedef void (*ImsMediaVoidUshortFuncPtr)(unsigned short);
typedef void (*ImsMediaVoidUshortUintFuncPtr)(unsigned short, unsigned int);
typedef int (*ImsMediaDeinitFuncPtr)(void);
typedef int (*ImsMediaIntVoidFuncPtr)(void);
typedef short (*ImsMediaShortVoidFuncPtr)(void);
typedef uint32_t (*ImsMediaUintVoidFuncPtr)(void);
typedef int (*ImsMediaIntIntFuncPtr)(int);
typedef unsigned long (*ImsMediaDataUsageFuncPtr)(unsigned short, int); //deprecated
typedef unsigned long (*ImsMediaDataUsageFuncPtr3)(unsigned short, int, int);
typedef ANativeWindow* (*ImsMediaGetRecordingSurfaceFuncPtr)(JNIEnv *);
typedef int (*ImsMediaSetVideoImageBufferFuncPtr)(int *, int, int, int);
typedef int (*ImsMediaSetDisplayParametersFuncPtr)(int, int);

struct ImsMediaApis {
    ImsMediaRegisterCbFuncPtr2 registerAppEventCallback2;
    ImsMediaInitFuncPtr initImsThinClient;
    ImsMediaFrameFuncPtr frameToEncode;
    ImsMediaSetSurfFuncPtr setFarEndSurface;
    ImsMediaVoidUshortFuncPtr setDeviceOrientation;
    ImsMediaVoidUshortFuncPtr setCameraFacing;
    ImsMediaVoidUshortUintFuncPtr setCameraInfo;
    ImsMediaDeinitFuncPtr deInitImsThinClient;
    ImsMediaIntVoidFuncPtr getNegotiatedFPS;
    ImsMediaIntVoidFuncPtr getNegotiatedHeight;
    ImsMediaIntVoidFuncPtr getNegotiatedWidth;
    ImsMediaShortVoidFuncPtr getUIOrientationMode;
    ImsMediaIntVoidFuncPtr getPeerHeight;
    ImsMediaIntVoidFuncPtr getPeerWidth;
    ImsMediaShortVoidFuncPtr getVideoQualityIndication;
    ImsMediaDataUsageFuncPtr getRtpDataUsage2; //deprecated
    ImsMediaDataUsageFuncPtr3 getRtpDataUsage3;
    ImsMediaIntIntFuncPtr requestRtpDataUsage2;
    ImsMediaGetRecordingSurfaceFuncPtr getRecordingSurface;
    ImsMediaUintVoidFuncPtr getRecorderFrameRate;
    ImsMediaSetVideoImageBufferFuncPtr setVideoImageBuffer;
    ImsMediaSetDisplayParametersFuncPtr setDisplayParameters;
};

#endif
