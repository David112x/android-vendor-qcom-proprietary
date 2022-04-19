/*****************************************************************************

============================
Copyright (c)  2004-2012,2017-2018  Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================

 File: qpVideo.cpp
 Description: Dpl implementation of video module

 Revision History
 ===============================================================================
 Date    |   Author's Name    |  BugID  |        Change Description
 ===============================================================================

 *******************************************************************************************/

/*-------------------------------------------------------------------

 -------------------------------------------------------------------*/

#include <qplog.h>
#include <qpVideo.h>
#ifndef MEDIA_CODEC
#include <VideoTransmitter.h>
#include <VideoReceiver.h>
//To check support for Camera HAL Version
#include <binder/IServiceManager.h>
#include <android/hardware/ICameraService.h>
#else
#include <VideoCodecDecoder.h>
#include <VideoCodecEncoder.h>
#endif
#include <VTPlayer.h>
#include <VTRecorder.h>


using namespace android;

/*===========================================================================

 FUNCTION checkForCamera2Support


 DESCRIPTION
 This api is used to check the supported camera HAL version.

 DEPENDENCIES
 None

 ARGUEMENTS IN
 None

 RETURN VALUE
 None

 SIDE EFFECTS
 None

 ===========================================================================*/

CAMERA_HAL_VERSION checkForCamera2Support()
{
  CAMERA_HAL_VERSION eHALVersion;
  //default mode is HAL3
  eHALVersion = CAMERA_HAL3;
#ifndef MEDIA_CODEC
  char cameraInterfaceType[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.qti.telephony.vt_cam_interface",
               cameraInterfaceType, "2");
  sp<IServiceManager> serviceman = defaultServiceManager();
  sp<IBinder> binderObject = serviceman->getService(String16("media.camera"));
  sp<hardware::ICameraService> serviceObject =
    interface_cast<hardware::ICameraService>(binderObject);
  binder::Status result;
  int32_t numCameras = 0;
  bool isCamera2Supported = false;

  if (serviceObject.get() != NULL) {
    result = serviceObject->getNumberOfCameras(
               hardware::ICameraService::CAMERA_TYPE_ALL, &numCameras);

    if (result.isOk()) {
      for (int32_t i = 0; i < numCameras; i++) {
        String16 cameraid = String16(String8::format("%d", i));
        result = serviceObject->supportsCameraApi(cameraid,
                 hardware::ICameraService::API_VERSION_2, &isCamera2Supported);

        if (!isCamera2Supported) {
          CRITICAL1("checkForCamera2Support Camera2 not supported");
          eHALVersion = CAMERA_HAL1;
        }
      }
    } else {
      CRITICAL1("checkForCamera2Support getNumberOfCameras failed");
    }
  } else {
    CRITICAL1("checkForCamera2Support getService failed");
  }

  if (eHALVersion == CAMERA_HAL3 && cameraInterfaceType[0] == '2') {
    CRITICAL1("checkForCamera2Support Camera2 enabled");
  } else {
    eHALVersion = CAMERA_HAL1;
    CRITICAL1("checkForCamera2Support Camera1 enabled");
  }

#endif
  return eHALVersion;
}


/**
 * \fn QP_IMPORT_C QPBOOL IsSavingStarted(void* clientData);
 *
 * \brief Internally used to check the state of the Video Saving.
 *
 * This function will return TRUE if the saving is already started else FALSE.
 *
 * \param clientData Video Data.\n
 * See \ref qpVideoData.
 *
 * \return \ref QPBOOL
 * \retval QP_TRUE
 * \retval QP_FALSE
 */

/********************************************************************************
 qpVideoInitialize
 *********************************************************************************/
void *qpVideoInitialize(QP_VIDEO_CALLBACK tVideoCallBack,
                        void* pUserData, QPE_VIDEO_DEV eVDev,
                        QP_VIDEO_CONFIG* pCodecConfig)
{
  VTRecorder* pVideoTx = NULL;
  VTPlayer* pVideoRx = NULL;
  QPE_VIDEO_ERROR eStatus;

  if (VIDEO_PLAYER == eVDev) {
#ifdef MEDIA_CODEC
    ERROR1("qpVideo::qpVideoinitialize for player (MediaCodec)");
    pVideoRx = new VideoCodecDecoder();
#else
    ERROR1("qpVideo::qpVideoinitialize for player (IOMX)");
    pVideoRx = new VideoReceiver();
#endif

    if (pVideoRx) {
      eStatus = pVideoRx->Init(tVideoCallBack, pUserData,  eVDev, pCodecConfig);

      if (VIDEO_ERROR_OK != eStatus) {
        ERROR1("qpVideoInitialize: Player Init/Register Failed");
        delete pVideoRx;
        pVideoRx = NULL;
      }
    }

    return pVideoRx;
  } else if (VIDEO_RECORDER == eVDev) {
#ifdef MEDIA_CODEC
    ERROR2("qpVideo::qpVideoinitialize for recorder (MediaCodec), hal %d",
           pCodecConfig->eHALVersion);
    pVideoTx = new VideoCodecEncoder();
#else
    ERROR2("qpVideo::qpVideoinitialize for recorder (IOMX), hal %d",
           pCodecConfig->eHALVersion);
    pVideoTx = new VideoTransmitter();
#endif

    if (pVideoTx) {
      eStatus = pVideoTx->Init(tVideoCallBack, pUserData,  eVDev, pCodecConfig);

      if (VIDEO_ERROR_OK != eStatus) {
        ERROR1("qpVideoInitialize: Recorder Init/Register Failed");
        delete pVideoTx;
        pVideoTx = NULL;
      }
    }

    return pVideoTx;
  }

  return NULL;
}

void qpVideoUninitialize(VideoCodecBase *tVideoDescriptor, QPE_VIDEO_DEV eVDev)
{
  if (tVideoDescriptor == NULL) {
    return;
  } else if (VIDEO_PLAYER == eVDev) {
#ifdef MEDIA_CODEC
    VideoCodecDecoder* pVideoRx = (VideoCodecDecoder*)tVideoDescriptor;
    ERROR1("qpVideo::qpVideoUninitialize for player (MediaCodec)");
#else
    VideoReceiver* pVideoRx = (VideoReceiver*)tVideoDescriptor;
    ERROR1("qpVideo::qpVideoUninitialize for player (IOMX)");
#endif
    pVideoRx->DeInit();
  } else if (VIDEO_RECORDER == eVDev) {
#ifdef MEDIA_CODEC
    VideoCodecEncoder* pVideoTx = (VideoCodecEncoder*)tVideoDescriptor;
    ERROR1("qpVideo::qpVideoUninitialize for recorder (MediaCodec)");
#else
    VideoTransmitter* pVideoTx = (VideoTransmitter*)tVideoDescriptor;
    ERROR1("qpVideo::qpVideoUninitialize for recorder (IOMX)");
#endif
    pVideoTx->DeInit();
    delete pVideoTx;
  }
}

void qpVideoDeleteObject(VideoCodecBase *tVideoDescriptor, QPE_VIDEO_DEV eVDev)
{
  if (tVideoDescriptor == NULL) {
    return;
  } else if (VIDEO_PLAYER == eVDev) {
#ifdef MEDIA_CODEC
    VideoCodecDecoder* pVideoRx = (VideoCodecDecoder*)tVideoDescriptor;
    ERROR1("qpVideo::qpVideoDeleteObject for player (MediaCodec)");
#else
    VideoReceiver* pVideoRx = (VideoReceiver*)tVideoDescriptor;
    ERROR1("qpVideo::qpVideoDeleteObject for player (IOMX)");
#endif
    delete pVideoRx;
  } else if (VIDEO_RECORDER == eVDev) {
#ifdef MEDIA_CODEC
    VideoCodecEncoder* pVideoTx = (VideoCodecEncoder*)tVideoDescriptor;
    ERROR1("qpVideo::qpVideoDeleteObject for recorder (MediaCodec)");
#else
    VideoTransmitter* pVideoTx = (VideoTransmitter*)tVideoDescriptor;
    ERROR1("qpVideo::qpVideoDeleteObject for recorder (IOMX)");
#endif
    delete pVideoTx;
  }
}
