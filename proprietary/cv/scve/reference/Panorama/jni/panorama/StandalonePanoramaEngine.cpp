/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <android/log.h>
#include <jni.h>
#include "throw_exception.h"
#include "scvePanorama.hpp"
#include "rpcmem.h"
#include <vendor/qti/hardware/scve/panorama/1.0/IPanoramaTracking.h>
#include <vendor/qti/hardware/scve/panorama/1.0/IPanoramaStitching.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __arm__
#ifndef __WOA__
#include <sys/ioctl.h>
#include <sys/mman.h>
#endif /*ifndef __WOA__*/
#include <fcntl.h>
#endif

#define PACKAGE     "com/qualcomm/qti/panoramaui/hardware/"
#define CLASS       PACKAGE "StandalonePanoramaEngine"
#define LOG_TAG "PanoNative"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  ALOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace android;
using namespace android::hardware;
using namespace vendor::qti::hardware::scve::panorama::V1_0;

typedef struct {
    SCVE::Context *context;
    SCVE::PanoramaTracking *panoramaTracking;
    SCVE::PanoramaStitching *panoramaStitching;
    int previewNumChannels;
    SCVE::ColorFormat previewColorFormat;
    SCVE::ImageDimension previewDimensions;
    int direction;
} NativeContext;

typedef struct {
    int64_t Tracking_instance;
    int64_t Stitching_instance;
    int previewNumChannels;
    ColorFormat previewColorFormat;
    ImageDimension_hal previewDimensions;
    int direction;
} NativeContext_hal;

sp<IPanoramaTracking> panoTracker = NULL;
sp<IPanoramaStitching> panoStitch = NULL;
sp<IPanoramaTracking_Callbacks> ptrackCallBack = NULL;
sp<IPanoramaStitching_Callbacks> pstitchCallBack = NULL;

static jclass class_ByteBuffer;
static jmethodID method_ByteBuffer_allocateDirect_I_ByteBuffer;
static jfieldID field_StandalonePanoramaEngine_nativeContext_J;

static int rawORjpeg = 0;

static void cleanupNativeContext(NativeContext_hal *nativeContext)
{
    if (nativeContext->Stitching_instance) {
        panoStitch->destroyInstance(nativeContext->Stitching_instance);
        nativeContext->Stitching_instance = 0;
    }
    if (nativeContext->Tracking_instance) {
        panoTracker->destroyInstance(nativeContext->Tracking_instance);
        nativeContext->Tracking_instance = 0;
    }
}

/*
 * Method:    nClassInit
 * Signature: ()V
 */
static void JNICALL nClassInit(JNIEnv *env, jclass cls)
{
    field_StandalonePanoramaEngine_nativeContext_J = env->GetFieldID(cls, "nativeContext", "J");
    if (env->ExceptionCheck()) return;

    class_ByteBuffer = env->FindClass("java/nio/ByteBuffer");
    if (env->ExceptionCheck()) return;
    class_ByteBuffer = (jclass)env->NewGlobalRef(class_ByteBuffer); // N.B. must take global ref
    if (env->ExceptionCheck()) return;

    method_ByteBuffer_allocateDirect_I_ByteBuffer = env->GetStaticMethodID(class_ByteBuffer,
            "allocateDirect", "(I)Ljava/nio/ByteBuffer;");
    if (env->ExceptionCheck()) return;
}

/*
 * Method:    nInstanceInit
 * Signature: (IIIIF)Z
 */
static jboolean JNICALL nInstanceInit(JNIEnv *env, jobject obj, jint width, jint height,
        jint stride, jint colorFormat, jint projectionType, jfloat focalLength)
{
    if (env->GetLongField(obj, field_StandalonePanoramaEngine_nativeContext_J) != 0L) {
        throwException(env, RUNTIME_EXCEPTION, "Already initialized");
        return JNI_FALSE;
    }

    rpcmem_init();
    panoTracker = IPanoramaTracking::getService("panoTracking");
    ALOGI("Getting Service :: %p", panoTracker.get());
       
    if(panoTracker.get() == nullptr){
        ALOGE("Failed to get service instance");
        return -1;
    }

    panoStitch = IPanoramaStitching::getService("panoStitching");
    ALOGI("Getting Service :: %p", panoStitch.get());

    if(panoStitch.get() == nullptr){
        ALOGE("Failed to get service instance");
        return -1;
    }
    //NativeContext *nativeContext = new NativeContext();
    NativeContext_hal *nativeContext = new NativeContext_hal();
    SCVE::ImageDimension internalDim;
    ImageDimension_hal hal_internalDim;

    int32_t sts=-1;
    int64_t instance_id = 0;
    hidl_handle hal_pSessionUserData;
    hidl_handle halstitch_pSessionUserData;

    SCVE::TrackingConfiguration_t trackingConfig;
    SCVE::StitchingConfiguration_t stitchConfig;

    TrackingConfiguration_hal hal_trackingConfig;
    StitchingConfiguration_hal hal_stitchConfig;

    IPanoramaStitching::createInstance_cb _hidl_cb1([&sts,&instance_id](int32_t retval, int64_t id){
        ALOGI("In _hidl_cb");
        sts = retval;
        instance_id = id;
    });

    IPanoramaTracking::createInstance_cb _hidl_cb([&sts,&instance_id](int32_t retval, int64_t id){
        ALOGI("In _hidl_cb");
        sts = retval;
        instance_id = id;
    });

    Return<void> hidl_ret;

    switch (colorFormat) {
    case SCVE::SCVE_COLORFORMAT_YUV_NV21:
        nativeContext->previewNumChannels = 1;
        break;

    default:
        throwException(env, ILLEGAL_ARGUMENT_EXCEPTION, "Unsupported colorFormat '%d'",
                colorFormat);
        goto error;
    }

    nativeContext->direction  = SCVE::SCVE_PANORAMA_MOTION_UNKNOWN;

    nativeContext->previewDimensions.nWidth = width;
    nativeContext->previewDimensions.nHeight = height;
    nativeContext->previewDimensions.nStride = stride;
    nativeContext->previewDimensions.nStrides[0] = 0;
    nativeContext->previewDimensions.nStrides[1] = 0;
    nativeContext->previewDimensions.nStrides[2] = 0;
    nativeContext->previewDimensions.nPadLength[0] = 0;
    nativeContext->previewDimensions.nPadLength[1] = 0;

    nativeContext->previewColorFormat = (ColorFormat)colorFormat;

    //nativeContext->context = SCVE::Context::newInstance(SCVE::SCVE_MODE_DEFAULT);
    //if (!nativeContext->context) {
    //    throwException(env, RUNTIME_EXCEPTION, "Failed to create SCVE context");
    //    goto error;
    //}

    internalDim.nWidth = width;
    internalDim.nHeight = height;
    internalDim.nStride = stride;
    internalDim.nStrides[0] = 0;
    internalDim.nStrides[1] = 0;
    internalDim.nStrides[2] = 0;
    internalDim.nPadLength[0] = 0;
    internalDim.nPadLength[1] = 0;

    hal_internalDim.nWidth = width;
    hal_internalDim.nHeight = height;
    hal_internalDim.nStride = stride;
    hal_internalDim.nStrides[0] = 0;
    hal_internalDim.nStrides[1] = 0;
    hal_internalDim.nStrides[2] = 0;
    hal_internalDim.nPadLength[0] = 0;
    hal_internalDim.nPadLength[1] = 0;

    trackingConfig.iViewFinderDim = internalDim;
    trackingConfig.cFormat = (SCVE::ColorFormat)nativeContext->previewColorFormat;
    trackingConfig.fCameraFOV = 0;

    hal_trackingConfig.iViewFinderDim = hal_internalDim;
    hal_trackingConfig.cFormat = nativeContext->previewColorFormat;
    hal_trackingConfig.fCameraFOV = 0;

    //nativeContext->panoramaTracking = SCVE::PanoramaTracking::newInstance(nativeContext->context,
    //        trackingConfig);
    //if (!nativeContext->panoramaTracking) {
    //    throwException(env, RUNTIME_EXCEPTION, "Failed to create Panorama tracking instance");
    //    goto error;
    //}

    hal_pSessionUserData.setTo(NULL,true);
    ALOGI("before create tracking instance");
    hidl_ret = panoTracker->createInstance( (::vendor::qti::hardware::scve::panorama::V1_0::OperationMode)SCVE::SCVE_MODE_DEFAULT, hal_trackingConfig, ptrackCallBack, hal_pSessionUserData, 0, _hidl_cb);
    if(hidl_ret.isOk()){
    	ALOGI("ALL okay in hidl tracking return");
    }
    else
    	ALOGI("Some problem in hidl tracking return");

    nativeContext->Tracking_instance = instance_id;
    //SCVE::StitchingConfiguration_t stitchConfig;
    stitchConfig.iViewFinderDim = internalDim;
    // Hard code focalLength to 0 per cv request until they find a better solution.
    stitchConfig.fCameraFocalLength = 0;
    stitchConfig.fCameraPixelSize = 0;
    stitchConfig.iProjectionType = 0;

    hal_stitchConfig.iViewFinderDim = hal_internalDim;
    hal_stitchConfig.fCameraFocalLength = 0;
    hal_stitchConfig.fCameraPixelSize = 0;
    hal_stitchConfig.iProjectionType = 0;
    if(projectionType == 0 || projectionType == 1) {
        stitchConfig.iProjectionType = projectionType;
        hal_stitchConfig.iProjectionType = projectionType;
    }
    LOGD("init pano stitching, projection type: %d", stitchConfig.iProjectionType);

    //nativeContext->panoramaStitching = SCVE::PanoramaStitching::newInstance(nativeContext->context,
    //        stitchConfig);
    //if (!nativeContext->panoramaStitching) {
    //    throwException(env, RUNTIME_EXCEPTION, "Failed to create Panorama stitching instance");
    //    goto error;
    //}

    halstitch_pSessionUserData.setTo(NULL,true);
    ALOGI("before create tracking instance");
    hidl_ret = panoStitch->createInstance((::vendor::qti::hardware::scve::panorama::V1_0::OperationMode)SCVE::SCVE_MODE_DEFAULT, hal_stitchConfig, pstitchCallBack, halstitch_pSessionUserData, 0, _hidl_cb1);
    if(hidl_ret.isOk()){
    	ALOGI("ALL okay in hidl tracking return");
    }
    else
    	ALOGI("Some problem in hidl tracking return");

    nativeContext->Stitching_instance = instance_id;
    env->SetLongField(obj, field_StandalonePanoramaEngine_nativeContext_J, (jlong)nativeContext);

    return JNI_TRUE;

error:
    if (nativeContext) {
        cleanupNativeContext(nativeContext);
        delete nativeContext;
    }
    return JNI_FALSE;
}

/*
 * Method:    nInstanceRelease
 * Signature: ()V
 */
static void JNICALL nInstanceRelease(JNIEnv *env, jobject obj)
{
    NativeContext_hal *nativeContext = (NativeContext_hal *)env->GetLongField(obj,
            field_StandalonePanoramaEngine_nativeContext_J);
    if (nativeContext == NULL) {
        throwException(env, RUNTIME_EXCEPTION, "Not initialized");
        return;
    }

    cleanupNativeContext(nativeContext);
    delete nativeContext;
    env->SetLongField(obj, field_StandalonePanoramaEngine_nativeContext_J, 0L);
    rpcmem_deinit();
}

/*
 * Method:    nProcessPreviewFrame
 * Signature: (Ljava/nio/ByteBuffer;[Z[I[I[I)I
 */
static jint JNICALL nProcessPreviewFrame(JNIEnv *env, jobject obj, jobject frame,
        jbooleanArray isKeyFrame, jintArray framePosition, jintArray movementSpeed,
        jintArray apiDirection)
{
    NativeContext_hal *nativeContext = (NativeContext_hal *)env->GetLongField(obj,
            field_StandalonePanoramaEngine_nativeContext_J);
    if (nativeContext == NULL) {
        throwException(env, RUNTIME_EXCEPTION, "Not initialized");
        return 0;
    }

    SCVE::Image image = SCVE::Image();
    Image_hal hal_image;
    int nSize = 0;

    //image.sFormat = nativeContext->previewColorFormat;
    //image.nChannels = nativeContext->previewNumChannels;
    //image.sDim = nativeContext->previewDimensions;
    image.pPtr = (uint8_t *)env->GetDirectBufferAddress(frame);
    nSize = (int)env->GetDirectBufferCapacity(frame);

    hal_image.sFormat = nativeContext->previewColorFormat;
    hal_image.nChannels = nativeContext->previewNumChannels;
    hal_image.sDim = nativeContext->previewDimensions;
    hal_image.nBytes = nSize;

    uint8_t* temp =  (uint8_t*)rpcmem_alloc(25,RPCMEM_DEFAULT_FLAGS,nSize);
    std::memcpy(temp, image.pPtr, nSize);

    native_handle_t* img_handle = native_handle_create(1,0);
    img_handle->numFds = 1;
    img_handle->data[0] =  rpcmem_to_fd((void*)temp);

    hal_image.pPtr.setTo(img_handle,true);

    if (env->ExceptionCheck()) return 0;

    SCVE::TrackingInput input;
    TrackingInput_hal hal_input;
    input.pFrame = &image;
    input.rotation = 0;
    input.flags = 0;

    hal_input.pFrame = hal_image;
    hal_input.rotation = 0;
    hal_input.flags = 0;

    //SCVE::TrackingOutput output;
    TrackingOutput_hal hal_output;
    int32_t status=-1;

    IPanoramaTracking::pTrackRun_Sync_cb _hidl_cb([&hal_output,&status](TrackingOutput_hal op, int32_t retval){
        ALOGI("In _hidl_cb track sync ");
        status = retval;
        hal_output = op;
       // ALOGI("In _hidl_cb track sync end assigned");
    });

    //SCVE::Status status = nativeContext->panoramaTracking->Run_Sync(input, output);.
    panoTracker->pTrackRun_Sync(hal_input, _hidl_cb);

    if (status == SCVE::SCVE_SUCCESS) {
        int32_t pos[2];
        pos[0] = hal_output.X;
        pos[1] = hal_output.Y;
        env->SetBooleanArrayRegion(isKeyFrame, 0, 1, (jboolean *)&hal_output.isKeyFrame);
        env->SetIntArrayRegion(framePosition, 0, 2, (jint *)pos);
        env->SetIntArrayRegion(movementSpeed, 0, 1, (jint *)&hal_output.speed);
        env->SetIntArrayRegion(apiDirection, 0, 1, (jint *)&hal_output.direction);
        nativeContext->direction = hal_output.direction;
    }

    return status;
}

/*
 * Method:    nProcessKeyFrame
 * Signature: ([BII)I
 */
static jint JNICALL nProcessKeyFrame(JNIEnv *env, jobject obj, jbyteArray keyFrameJpegData,
        jint x, jint y, int exifAngle)
{
    NativeContext_hal *nativeContext = (NativeContext_hal *)env->GetLongField(obj,
            field_StandalonePanoramaEngine_nativeContext_J);
    if (nativeContext == NULL) {
        throwException(env, RUNTIME_EXCEPTION, "Not initialized");
        return 0;
    }

    rawORjpeg = 1;
    SCVE::KeyFrameJPEG_t jpeg;
    FrameJPEG_hal hal_jpeg;
    jpeg.pData = (uint8_t *)env->GetByteArrayElements(keyFrameJpegData, NULL);
    jpeg.size = (uint32_t)env->GetArrayLength(keyFrameJpegData);

    hal_jpeg.size = (uint32_t)env->GetArrayLength(keyFrameJpegData);
    uint8_t* temp =  (uint8_t*)rpcmem_alloc(25,RPCMEM_DEFAULT_FLAGS,hal_jpeg.size);
    std::memcpy(temp, jpeg.pData, hal_jpeg.size);

    native_handle_t* img_handle = native_handle_create(1,0);
    img_handle->numFds = 1;
    img_handle->data[0] =  rpcmem_to_fd((void*)temp);

    hal_jpeg.pData.setTo(img_handle);

    SCVE::KeyFrame_t keyFrame = SCVE::KeyFrame_t();
    KeyFrameJPEG_hal hal_keyFrame;

    keyFrame.type = SCVE::SCVE_PANORAMASTITCHING_KEYFRAME_JPEG;
    keyFrame.u.pKeyFrameJPEG = &jpeg;
    keyFrame.rotation = exifAngle % 360;

    hal_keyFrame.type = SCVE::SCVE_PANORAMASTITCHING_KEYFRAME_JPEG;
    hal_keyFrame.rotation = exifAngle % 360;	
    hal_keyFrame.pKeyFrameJPEG = hal_jpeg;

    ALOGI(" rotation of key frame is :: %d ",hal_keyFrame.rotation);

    // Extract orientation information using libexif built-in function
    // In case no Exif data available, use orientation

    SCVE::StitchingInput_t input;
    StitchingJPEGInput_hal hal_input;

    input.pKeyFrame = &keyFrame;
    input.X = x;
    input.Y = y;
    input.flags = 0;
    input.direction = nativeContext->direction;

    hal_input.pKeyFrame = hal_keyFrame;
    hal_input.X = x;
    hal_input.Y = y;
    hal_input.flags = 0;
    hal_input.direction = nativeContext->direction;

    //SCVE::StitchingOutput_t output;
    StitchingOutput_hal hal_output;

    //SCVE::Status status = nativeContext->panoramaStitching->Run_Sync(input, output);
    int32_t status=-1;

    IPanoramaStitching::panoJPEGStitchRun_Sync_cb _hidl_cb([&hal_output,&status](StitchingOutput_hal op, int32_t retval){
        ALOGI("In _hidl_cb jpeg stitch sync ");
        status = retval;
        hal_output = op;
       //std::memcpy(hal_output, op, sizeof(hal_output));
    });

    panoStitch->panoJPEGStitchRun_Sync(hal_input,_hidl_cb);

    env->ReleaseByteArrayElements(keyFrameJpegData, (jbyte *)keyFrame.u.pKeyFrameJPEG->pData,
            JNI_ABORT);
    return status;
}
static jint JNICALL nProcessKeyFrameNV21(JNIEnv *env, jobject obj, jobject frame, jint x, jint y)
{
    NativeContext_hal *nativeContext = (NativeContext_hal *)env->GetLongField(obj,
            field_StandalonePanoramaEngine_nativeContext_J);
    if (nativeContext == NULL) {
        throwException(env, RUNTIME_EXCEPTION, "Not initialized");
        return 0;
    }

    int nSize;
    rawORjpeg = 2;
    SCVE::Image image = SCVE::Image();
    Image_hal hal_image;

    image.pPtr = (uint8_t *)env->GetDirectBufferAddress(frame);
    nSize = (int)env->GetDirectBufferCapacity(frame);

    hal_image.sFormat = nativeContext->previewColorFormat;
    hal_image.nChannels = nativeContext->previewNumChannels;
    hal_image.sDim = nativeContext->previewDimensions;
    hal_image.nBytes = nSize;

    uint8_t* temp =  (uint8_t*)rpcmem_alloc(25,RPCMEM_DEFAULT_FLAGS,nSize);
    std::memcpy(temp, image.pPtr, nSize);

    native_handle_t* img_handle = native_handle_create(1,0);
    img_handle->numFds = 1;
    img_handle->data[0] =  rpcmem_to_fd((void*)temp);

    hal_image.pPtr.setTo(img_handle,true);

    if (env->ExceptionCheck()) return 0;
    SCVE::KeyFrame_t keyFrame = SCVE::KeyFrame_t();
    KeyFrameRaw_hal hal_keyFrame;

    keyFrame.type = SCVE::SCVE_PANORAMASTITCHING_KEYFRAME_RAW;
    keyFrame.u.pKeyFrameRaw = &image;

    hal_keyFrame.type = SCVE::SCVE_PANORAMASTITCHING_KEYFRAME_RAW;
    hal_keyFrame.rotation = 0;
    hal_keyFrame.pKeyFrameRaw = hal_image;

    SCVE::StitchingInput_t input;
    StitchingRawInput_hal hal_input;

    input.pKeyFrame = &keyFrame;
    input.X = x;
    input.Y = y;
    input.flags = 0;
    input.direction = nativeContext->direction;

    hal_input.pKeyFrame = hal_keyFrame;
    hal_input.X = x;
    hal_input.Y = y;
    hal_input.flags = 0;
    hal_input.direction = nativeContext->direction;
    //SCVE::StitchingOutput_t output;
    //SCVE::Status status = nativeContext->panoramaStitching->Run_Sync(input, output);

    StitchingOutput_hal hal_output;
    int32_t status=-1;

    IPanoramaStitching::panoRawStitchRun_Sync_cb _hidl_cb([&hal_output,&status](StitchingOutput_hal op, int32_t retval){
        ALOGI("In _hidl_cb raw stitch sync ");
        status = retval;
        hal_output = op;
    });

    panoStitch->panoRawStitchRun_Sync(hal_input,_hidl_cb);

    return status;
}

/*
 * Method:    nCancelPanorama
 * Signature: ()I
 */
static jint JNICALL nCancelPanorama(JNIEnv *env, jobject obj)
{
    NativeContext_hal *nativeContext = (NativeContext_hal *)env->GetLongField(obj,
            field_StandalonePanoramaEngine_nativeContext_J);
    if (nativeContext == NULL) {
        throwException(env, RUNTIME_EXCEPTION, "Not initialized");
        return 0;
    }

    SCVE::TrackingInput trackInput;
    TrackingInput_hal hal_trackInput;
    trackInput.pFrame = NULL;
    trackInput.rotation = 0;
    trackInput.flags = SCVE::SCVE_PANORAMA_TASK_CANCEL;

    hal_trackInput.pFrame = {0};
    hal_trackInput.rotation = 0;
    hal_trackInput.flags = SCVE::SCVE_PANORAMA_TASK_CANCEL;

    SCVE::TrackingOutput trackOutput;// N.B. the optional parameters to PanoramaTracker::Run_Sync aren't optional
    TrackingOutput_hal hal_trackOutput;

    //SCVE::Status statusTracking = nativeContext->panoramaTracking->Run_Sync(trackInput, trackOutput);
    int32_t statusTracking=-1;

    IPanoramaTracking::pTrackRun_Sync_cb _hidl_cb([&hal_trackOutput,&statusTracking](TrackingOutput_hal op, int32_t retval){
        ALOGI("In _hidl_cb nCancelPanorama tracking sync");
        statusTracking = retval;
        hal_trackOutput = op;
    });

    //SCVE::Status status = nativeContext->panoramaTracking->Run_Sync(input, output);.
    panoTracker->pTrackRun_Sync(hal_trackInput, _hidl_cb);

    SCVE::StitchingInput_t stitchInput;
        stitchInput.X = 0;
        stitchInput.Y = 0;
        stitchInput.flags = SCVE::SCVE_PANORAMA_TASK_CANCEL;
        stitchInput.direction = SCVE::SCVE_PANORAMA_MOTION_UNKNOWN;
    //    SCVE::StitchingOutput_t stitchOutput;
    //SCVE::Status statusStitching = nativeContext->panoramaStitching->Run_Sync(stitchInput, stitchOutput);
    int32_t statusStitching=-1;
    if(rawORjpeg == 1){
      // jpeg image
        StitchingJPEGInput_hal hal_input;
        hal_input.X = 0;
        hal_input.Y = 0;
        hal_input.flags = SCVE::SCVE_PANORAMA_TASK_CANCEL;
        hal_input.direction = SCVE::SCVE_PANORAMA_MOTION_UNKNOWN;
        hal_input.pKeyFrame = {0};

        StitchingOutput_hal hal_output;
        //int32_t statusStitching=-1;

        IPanoramaStitching::panoJPEGStitchRun_Sync_cb _hidl_cb([&hal_output,&statusStitching](StitchingOutput_hal op, int32_t retval){
            ALOGI("In _hidl_cb jpeg nCancelPanorama stitch sync ");
            statusStitching = retval;
            hal_output = op;
            //std::memcpy(hal_output, op, sizeof(hal_output));
        });

        panoStitch->panoJPEGStitchRun_Sync(hal_input,_hidl_cb);

     }
     else if (rawORjpeg == 2){
       // Raw image
        StitchingRawInput_hal hal_input;
        hal_input.X = 0;
        hal_input.Y = 0;
        hal_input.flags = SCVE::SCVE_PANORAMA_TASK_CANCEL;
        hal_input.direction = SCVE::SCVE_PANORAMA_MOTION_UNKNOWN;
        hal_input.pKeyFrame = {0};

        StitchingOutput_hal hal_output;

        IPanoramaStitching::panoRawStitchRun_Sync_cb _hidl_cb([&hal_output,&statusStitching](StitchingOutput_hal op, int32_t retval){
            ALOGI("In _hidl_cb raw nCancelPanorama stitch sync");
            statusStitching = retval;
            hal_output = op;
        });
        panoStitch->panoRawStitchRun_Sync(hal_input,_hidl_cb);
     }
     else {
        ALOGI("Wrong CaLL");	
     }

    if (statusTracking != SCVE::SCVE_SUCCESS) return statusTracking;
    return statusStitching;
}

/*
 * Method:    nCompletePanorama
 * Signature: ([Ljava/nio/ByteBuffer;)I
 */
static jint JNICALL nCompletePanorama(JNIEnv *env, jobject obj, jobjectArray panoramaJpegData)
{
    NativeContext_hal *nativeContext = (NativeContext_hal *)env->GetLongField(obj,
            field_StandalonePanoramaEngine_nativeContext_J);
    if (nativeContext == NULL) {
        throwException(env, RUNTIME_EXCEPTION, "Not initialized");
        return 0;
    }
    SCVE::TrackingInput trackInput;
    TrackingInput_hal hal_trackInput;

    trackInput.pFrame = NULL;
    trackInput.rotation = 0;
    trackInput.flags = SCVE::SCVE_PANORAMA_FRAME_ENDOFSEQUENCE;

    hal_trackInput.pFrame = {0};
    hal_trackInput.rotation = 0;
    hal_trackInput.flags = SCVE::SCVE_PANORAMA_FRAME_ENDOFSEQUENCE;

    //SCVE::TrackingOutput trackOutput;// N.B. the optional parameters to PanoramaTracker::Run_Sync aren't optional
    //SCVE::Status statusTracking = nativeContext->panoramaTracking->Run_Sync(trackInput, trackOutput);
    TrackingOutput_hal hal_trackOutput;

    int32_t statusTracking=-1;

    IPanoramaTracking::pTrackRun_Sync_cb _hidl_cb([&hal_trackOutput,&statusTracking](TrackingOutput_hal op, int32_t retval){
        ALOGI("In _hidl_cb nCompletePanorama track sync ");
        statusTracking = retval;
        hal_trackOutput = op;
    });

    //SCVE::Status status = nativeContext->panoramaTracking->Run_Sync(input, output);.
    panoTracker->pTrackRun_Sync(hal_trackInput, _hidl_cb);

    SCVE::StitchingInput_t stitchInput;
    stitchInput.X = 0;
    stitchInput.Y = 0;
    stitchInput.flags = SCVE::SCVE_PANORAMA_FRAME_ENDOFSEQUENCE;
    stitchInput.direction = SCVE::SCVE_PANORAMA_MOTION_UNKNOWN;
    //SCVE::StitchingOutput_t stitchOutput;
    //SCVE::Status statusStitching = nativeContext->panoramaStitching->Run_Sync(stitchInput, stitchOutput);
    int32_t statusStitching=-1;
    if(rawORjpeg == 1){
       // jpeg image
        StitchingJPEGInput_hal hal_input;
        hal_input.X = 0;
        hal_input.Y = 0;
        hal_input.flags = SCVE::SCVE_PANORAMA_FRAME_ENDOFSEQUENCE;
        hal_input.direction = SCVE::SCVE_PANORAMA_MOTION_UNKNOWN;
        hal_input.pKeyFrame = {0};

        StitchingOutput_hal hal_output;

        //int32_t statusStitching=-1;
        IPanoramaStitching::panoJPEGStitchRun_Sync_cb _hidl_cb([&hal_output,&statusStitching](StitchingOutput_hal op, int32_t retval){
            ALOGI("In _hidl_cb jpeg nCompletePanorama stitch sync ");
            statusStitching = retval;
            hal_output = op;
            //std::memcpy(hal_output, op, sizeof(hal_output));
        });

        panoStitch->panoJPEGStitchRun_Sync(hal_input,_hidl_cb);
     }
     else if (rawORjpeg == 2){
           // Raw image
        StitchingRawInput_hal hal_input;
        hal_input.X = 0;
        hal_input.Y = 0;
        hal_input.flags = SCVE::SCVE_PANORAMA_FRAME_ENDOFSEQUENCE;
        hal_input.direction = SCVE::SCVE_PANORAMA_MOTION_UNKNOWN;
        hal_input.pKeyFrame = {0};

        StitchingOutput_hal hal_output;

        IPanoramaStitching::panoRawStitchRun_Sync_cb _hidl_cb([&hal_output,&statusStitching](StitchingOutput_hal op, int32_t retval){
            ALOGI("In _hidl_cb raw nCompletePanorama stitch sync ");
            statusStitching = retval;
            hal_output = op;
        });
        panoStitch->panoRawStitchRun_Sync(hal_input,_hidl_cb);
      }
     else {
        ALOGI("Wrong CaLL");	
      }

    if (statusTracking != SCVE::SCVE_SUCCESS) return statusTracking;
    if (statusStitching != SCVE::SCVE_SUCCESS) return statusStitching;

    uint32_t size = 0;
    uint32_t val = -1;
    hidl_handle dst;
    IPanoramaStitching::getPanoResultSize_cb _hidl_cb1([&size,&val](uint32_t nSize, uint32_t ret){
         size = nSize;
         val = ret;
    });

    panoStitch->getPanoResultSize(_hidl_cb1);
    if(size == 0) return val;

    //const native_handle_t* ref = dst.getNativeHandle();
    //uint8_t* temp_img = (uint8_t*)mmap(NULL,size,PROT_READ|PROT_WRITE, MAP_SHARED, ref->data[0],0);
    uint8_t* pDst =  (uint8_t*)rpcmem_alloc(25,RPCMEM_DEFAULT_FLAGS,size);
    native_handle_t* img_handle = native_handle_create(1,0);
    img_handle->numFds = 1;
    img_handle->data[0] =  rpcmem_to_fd((void*)pDst);

    dst.setTo(img_handle,true);
    val = panoStitch->getPanoResult(dst,size);

    //statusStitching = nativeContext->panoramaStitching->GetPanoramaResult(NULL, &size);
    //if (statusStitching != SCVE::SCVE_SUCCESS) return statusStitching;

    jobject byteBuffer = env->CallStaticObjectMethod(class_ByteBuffer,
            method_ByteBuffer_allocateDirect_I_ByteBuffer, (jint)size);
    if (env->ExceptionCheck()) return 0;
    if (byteBuffer == NULL) {
        throwException(env, RUNTIME_EXCEPTION, "Failed allocating a direct ByteBuffer");
        return 0;
    }

    uint8_t *jpegData = (uint8_t *)env->GetDirectBufferAddress(byteBuffer);
    //statusStitching = nativeContext->panoramaStitching->GetPanoramaResult(jpegData, &size);
    //if (statusStitching != SCVE::SCVE_SUCCESS) return statusStitching;

    std::memcpy(jpegData, pDst, size);
    env->SetObjectArrayElement(panoramaJpegData, 0, byteBuffer);
    if (env->ExceptionCheck()) return 0;

    return SCVE::SCVE_SUCCESS;
}

///////////////////////////////////////// JNI INTERFACE ////////////////////////////////////////////

static const JNINativeMethod methods[] = {
    { "nClassInit", "()V", (void *)nClassInit },
    { "nInstanceInit", "(IIIIIF)Z", (void *)nInstanceInit },
    { "nInstanceRelease", "()V", (void *)nInstanceRelease },
    { "nProcessPreviewFrame", "(Ljava/nio/ByteBuffer;[Z[I[I[I)I", (void *)nProcessPreviewFrame },
    { "nProcessKeyFrame", "([BIII)I", (void *)nProcessKeyFrame },
    { "nProcessKeyFrameNV21", "(Ljava/nio/ByteBuffer;II)I", (void *)nProcessKeyFrameNV21 },
    { "nCancelPanorama", "()I", (void *)nCancelPanorama },
    { "nCompletePanorama", "([Ljava/nio/ByteBuffer;)I", (void *)nCompletePanorama },
};

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    jclass cls = env->FindClass(CLASS);
    if (env->ExceptionCheck()) return -1;
    env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(methods[0]));
    if (env->ExceptionCheck()) return -1;

    return JNI_VERSION_1_4;
}
