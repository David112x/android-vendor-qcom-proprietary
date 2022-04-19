/*
 * Copyright (c) 2010-2011, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 *
 *
 * @file Implementation of FastCV Sample native interface.
 *
 */

//==============================================================================
// Include Files
//==============================================================================
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include <android/log.h>
#include <cutils/log.h>
#include <time.h>
#include <fastcv.h>
#include <fastcv_.h>
#include <pthread.h>
#include "FastCVSample.h"
#include "FastCVSampleRenderer.h"
#include "FPSCounter.h"

//#define SCVE_PROFILE_ENABLE
//#define PROFILE_PRIO_HIGH
#include "scveProf.h"
#include "rpcmem.h"
#include <vendor/qti/hardware/scve/objecttracker/1.0/IObjectTracker.h>

#ifdef __arm__
#ifndef __WOA__
#include <sys/ioctl.h>
#include <sys/mman.h>
#endif /*ifndef __WOA__*/
#include <fcntl.h>
#endif

#define T2T_LOG_TAG    "Touch2Track"

using namespace android;
using namespace android::hardware;
using namespace vendor::qti::hardware::scve::objecttracker::V1_0;

//SCVE_PROFILE_PRIO_DEFINE(PROFILE_PRIO_HIGH, T2T_APP_TRACK);
//SCVE_PROFILE_PRIO_DEFINE(PROFILE_PRIO_HIGH, T2T_APP_REG_BOX);
//SCVE_PROFILE_PRIO_DEFINE(PROFILE_PRIO_HIGH, T2T_APP_REG_PT);

static int32_t bPrintInfoEveryCall = 0;

sp<IObjectTracker> objTracker = NULL;
sp<IObjectTracker_Callbacks> objTrackCallback = NULL;

//==============================================================================
// Declarations
//==============================================================================
#define DPRINTF(...)  __android_log_print(ANDROID_LOG_DEBUG,T2T_LOG_TAG,__VA_ARGS__)
#define IPRINTF(...)  __android_log_print(ANDROID_LOG_INFO,T2T_LOG_TAG,__VA_ARGS__)
#define EPRINTF(...)  __android_log_print(ANDROID_LOG_ERROR,T2T_LOG_TAG,__VA_ARGS__)
#define ASSERT(condition) LOG_ALWAYS_FATAL_IF(!(condition), T2T_LOG_TAG ": ASSERT FAILED at %s:%d", __FUNCTION__, __LINE__)

//--------------------------------------ANDROID_LOG_INFO----------------------------------------
/// @brief
///    Contains state information of the instance of the application.
//------------------------------------------------------------------------------
struct State
{
   //---------------------------------------------------------------------------
   /// Constructor for State object sets variables to default values.
   //---------------------------------------------------------------------------
   State()
   {
      timeFilteredMs = 5;
   }

   /// Camera preview FPS counter
   FPSCounter                 camFPSCounter;

   /// Filtered timing value for FastCV processing.
   float                      timeFilteredMs;
};

#define MAX_ROIS 5
#define TEMP_RESULT_SIZE 5

//#define CALL_SALMAP

struct TrackerJNIContext
{
   SCVE::Context              *pCtx;
   SCVE::ObjectTracker        *pTracker;
   SCVE::ImageDimension       sDim;

   uintptr_t                  nObjIndex[MAX_ROIS];
   uint32_t                   nActiveObjs;

   SCVE::ObjectTrackerResult  sResult[MAX_ROIS];

   pthread_mutex_t            hMutex;
   pthread_cond_t             hCond;

   SCVE::ObjectTrackerResult  sTempResult[TEMP_RESULT_SIZE][MAX_ROIS];
   bool                       bTempUsed[TEMP_RESULT_SIZE];

   int                        nTrackerMode;
   bool                       bUseAsync;

   SCVE::Image                sTrackImage;
   SCVE::Image                sRegImage;
   SCVE::Image                sSalMap;

   uint64_t                   prevTime;
   State                      state;

   TrackerJNIContext()
      : pCtx(NULL), pTracker(NULL), nActiveObjs(0), nTrackerMode(-1), bUseAsync(false),
        prevTime(0)
   {
      //sDim, sTrackImage and sSalMap should be zero-initialized through their constructors.

      memset(sResult,   0, sizeof(sResult));
      memset(nObjIndex, 0, sizeof(nObjIndex));
      memset(bTempUsed, 0, sizeof(bTempUsed));

      //initialize the mutex that guards the sResult[] array.
      pthread_mutex_init(&hMutex, NULL);
      pthread_cond_init(&hCond, NULL);
   }

   ~TrackerJNIContext()
   {
      pthread_mutex_destroy(&hMutex);
      pthread_cond_destroy (&hCond);
   }
};

struct TrackerJNIContext_hal
{
   //SCVE::Context              *pCtx;
   int64_t                    pTracker;
   ImageDimension_hal         sDim;

   uint64_t                   nObjIndex[MAX_ROIS];
   uint32_t                   nActiveObjs;

   ObjectTrackerResult_hal    sResult[MAX_ROIS];

   pthread_mutex_t            hMutex;
   pthread_cond_t             hCond;

   ObjectTrackerResult_hal    sTempResult[TEMP_RESULT_SIZE][MAX_ROIS];
   bool                       bTempUsed[TEMP_RESULT_SIZE];

   int                        nTrackerMode;
   bool                       bUseAsync;

   Image_hal                sTrackImage;
   Image_hal                sRegImage;
   Image_hal                sSalMap;

   uint64_t                   prevTime;
   State                      state;

   TrackerJNIContext_hal()
      : pTracker(0), nActiveObjs(0), nTrackerMode(-1), bUseAsync(false),
        prevTime(0)
   {
      //sDim, sTrackImage and sSalMap should be zero-initialized through their constructors.

      memset(sResult,   0, sizeof(sResult));
      memset(nObjIndex, 0, sizeof(nObjIndex));
      memset(bTempUsed, 0, sizeof(bTempUsed));

      //initialize the mutex that guards the sResult[] array.
      pthread_mutex_init(&hMutex, NULL);
      pthread_cond_init(&hCond, NULL);
   }

   ~TrackerJNIContext_hal()
   {
      pthread_mutex_destroy(&hMutex);
      pthread_cond_destroy (&hCond);
   }
};
//==============================================================================
// Function Declarations
//==============================================================================
#define FASTCVSAMPLE_CLASSNAME "com/qualcomm/qti/touch2track/FastCVSample"

static jfieldID fieldId_TrackerJNIContext = NULL;

static
TrackerJNIContext_hal* getNativeContext(JNIEnv * env, jobject obj)
{
   TrackerJNIContext_hal* retVal = NULL;

   if (fieldId_TrackerJNIContext == NULL)
   {
      jclass class_FastCVSample = env->FindClass(FASTCVSAMPLE_CLASSNAME);
      if (class_FastCVSample != NULL)
      {
         fieldId_TrackerJNIContext = env->GetFieldID(class_FastCVSample, "mNativeContext", "J");
      }
   }

   if (fieldId_TrackerJNIContext != NULL)
   {
      retVal = (TrackerJNIContext_hal*) env->GetLongField(obj, fieldId_TrackerJNIContext);

      if (retVal == NULL)
      {
         retVal = new TrackerJNIContext_hal;
         env->SetLongField(obj, fieldId_TrackerJNIContext, (jlong)retVal);
      }
   }

   return retVal;
}

static
void cleanupNativeContext(JNIEnv * env, jobject obj, TrackerJNIContext_hal* pNativeContext)
{
   if (pNativeContext == NULL)
      return;

   if (fieldId_TrackerJNIContext != NULL)
   {
      env->SetLongField(obj, fieldId_TrackerJNIContext, (jlong)NULL);
      delete pNativeContext;
   }
}

static
int getAvailableTempResultIndex(TrackerJNIContext_hal *pNativeContext)
{
   int i=TEMP_RESULT_SIZE;

   pthread_mutex_lock(&pNativeContext->hMutex);
   while (i >= TEMP_RESULT_SIZE)
   {
      for (i=0; i<TEMP_RESULT_SIZE; i++)
      {
         if (pNativeContext->bTempUsed[i] == false)
         {
            break;
         }
      }

      //If we don't find an empty slot, wait for signal from callback
      if (i >= TEMP_RESULT_SIZE)
      {
         pthread_cond_wait(&pNativeContext->hCond, &pNativeContext->hMutex);
      }
   }
   pthread_mutex_unlock(&pNativeContext->hMutex);

   return i;
}

static
uint64_t getTimeMicroSeconds()
{
   struct timespec t;
   t.tv_sec = t.tv_nsec = 0;

   clock_gettime(CLOCK_REALTIME, &t);
   return (uint64_t)t.tv_sec * 1000000ULL + t.tv_nsec / 1000L;
}

static
void CallSalMapFunctions(TrackerJNIContext_hal *pNativeContext)
{
#ifdef CALL_SALMAP
   //Just call to exercise SalMap function
   pNativeContext->pTracker->GetSaliencyMap(pNativeContext->nObjIndex, pNativeContext->nActiveObjs,
                                            &pNativeContext->sSalMap);
#endif
}

//static
//void TrackCallback ( SCVE::Status               status,
//                     SCVE::ObjectTrackerResult  *pResult,
//                     uint32_t                   nResultSize,
//                     void                       *pSessionUserData,
//                     void                       *pTaskUserData)
//{
//   TrackerJNIContext *pNativeContext = (TrackerJNIContext*) pSessionUserData;
//
//   //unsigned int i = ((uintptr_t)pTaskUserData - (uintptr_t)pNativeContext->bTempUsed) / sizeof(bool);
//   //DPRINTF("Recevied callback (%d) - status = %d", i, status);
//
//   uint64_t newTime = getTimeMicroSeconds();
//   if (pNativeContext->prevTime != 0)
//   {
//      uint64_t timeMs = (newTime - pNativeContext->prevTime) / 1000.f;
//      pNativeContext->state.timeFilteredMs = ((pNativeContext->state.timeFilteredMs * (29.f / 30.f)) +
//                                              (float)(timeMs / 30.f));
//   }
//   pNativeContext->prevTime = newTime;
//
//   pthread_mutex_lock(&pNativeContext->hMutex);
//   if (status == SCVE::SCVE_SUCCESS)
//   {
//      for (unsigned i = 0; i < pNativeContext->nActiveObjs; i++)
//      {
//         pNativeContext->sResult[i] = pResult[i];
//      }
//   }
//   *((bool*)pTaskUserData) = false; //bTempUsed to false
//
//   pthread_cond_signal(&pNativeContext->hCond);
//   pthread_mutex_unlock(&pNativeContext->hMutex);
//}

/*
static
void RegCallback   ( SCVE::Status  status,
                     uint32_t      nIndex,
                     void          *pSessionUserData,
                     void          *pTaskUserData )
{

}
*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT void
JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_init
(
   JNIEnv   *env,
   jobject  obj,
   jint     w,
   jint     h,
   jboolean yuvEnabled,
   jint     trackerMode,
   jboolean useAsync
)
{
   DPRINTF("%s begin\n", __FUNCTION__);

   //const int32_t PROP_VALUE_MAX = 512;

   //char cmdLog[PROP_VALUE_MAX];
   //const char* command = "getprop scve.t2t_apk.debug";
   //FILE* file = popen(command, "r");
   //if (file) {
   //   // read the property value from file
   //   if ( fgets (cmdLog , PROP_VALUE_MAX , file))
   //   {
   //      //make sure cmdLog is null terminated
   //      cmdLog[PROP_VALUE_MAX-1] = '\0';
   //      bPrintInfoEveryCall = atoi(cmdLog);
   //   }
   //   pclose(file);
   //}

    rpcmem_init();
    objTracker = IObjectTracker::getService("objectTracker");
    ALOGI("Getting Service :: %p", objTracker.get());

    if(objTracker.get() == nullptr){
        ALOGE("Failed to get service instance");
        return ;
    }
   //SCVE_PROFILE_PRIO_INIT(PROFILE_PRIO_HIGH, T2T_APP_TRACK);
   //SCVE_PROFILE_PRIO_INIT(PROFILE_PRIO_HIGH, T2T_APP_REG_BOX);
   //SCVE_PROFILE_PRIO_INIT(PROFILE_PRIO_HIGH, T2T_APP_REG_PT);

   TrackerJNIContext_hal *pNativeContext = getNativeContext(env, obj);

   if (!pNativeContext)
   {
      EPRINTF("Unable to allocate Native Context");
      return;
   }

   pNativeContext->sDim.nWidth = w;
   pNativeContext->sDim.nHeight = h;
   pNativeContext->sDim.nStride = 0;
   pNativeContext->sDim.nStrides[0] = w;
   pNativeContext->sDim.nStrides[2] = 0;
   pNativeContext->sDim.nPadLength[0] = 0;
   pNativeContext->sDim.nPadLength[1] = 0;

   if (yuvEnabled)
   {
      pNativeContext->sTrackImage.sFormat = ::vendor::qti::hardware::scve::objecttracker::V1_0::ColorFormat::SCVE_COLORFORMAT_YUV_NV21;
      pNativeContext->sRegImage.sFormat = ::vendor::qti::hardware::scve::objecttracker::V1_0::ColorFormat::SCVE_COLORFORMAT_YUV_NV21;
      pNativeContext->sDim.nStrides[1] = w;
   }
   else
   {
      pNativeContext->sTrackImage.sFormat = ::vendor::qti::hardware::scve::objecttracker::V1_0::ColorFormat::SCVE_COLORFORMAT_GREY_8BIT;
      pNativeContext->sRegImage.sFormat = ::vendor::qti::hardware::scve::objecttracker::V1_0::ColorFormat::SCVE_COLORFORMAT_GREY_8BIT;
   pNativeContext->sDim.nStrides[1] = 0;
   }

   pNativeContext->sTrackImage.sDim  = pNativeContext->sDim;
   pNativeContext->sRegImage.sDim  = pNativeContext->sDim;

#ifdef CALL_SALMAP
   pNativeContext->sSalMap.sDim = pNativeContext->sDim;
   pNativeContext->sSalMap.pPtr = (uint8_t *) malloc (w * h);
   if (pNativeContext->sSalMap.pPtr == NULL)
   {
      EPRINTF("Unable to allocate memory for SalMap buffer");
      cleanupNativeContext(env, obj, pNativeContext);
      return;
   }
#endif

   //reinitialize the object tracking if necessary
   if (pNativeContext->pTracker != 0)
   {
      uint32_t ret = objTracker->destroyInstance(pNativeContext->pTracker);
      pNativeContext->pTracker = 0;
      DPRINTF("reinitialized with ret = %d", ret);
   }

   //Cleanup Context
   //if (pNativeContext->pCtx != NULL)
   //{
   //   SCVE::Context::deleteInstance(pNativeContext->pCtx);
   //   pNativeContext->pCtx = NULL;
   //}

   pNativeContext->nTrackerMode = trackerMode;
   pNativeContext->bUseAsync    = (bool) useAsync;

   DPRINTF("Initializing in Mode %d, Use Async = %s", trackerMode, useAsync ? "True" : "False");

   OperationMode opMode = (OperationMode)0;
   /*
   if (trackerMode == 0)
      pNativeContext->pCtx = SCVE::Context::newInstance(SCVE::SCVE_MODE_DEFAULT);
   else if (trackerMode == 1)
      pNativeContext->pCtx = SCVE::Context::newInstance(SCVE::SCVE_MODE_PERFORMANCE);
   else if (trackerMode == 2)
      pNativeContext->pCtx = SCVE::Context::newInstance(SCVE::SCVE_MODE_CPU_OFFLOAD);
   else if (trackerMode == 3)
      pNativeContext->pCtx = SCVE::Context::newInstance(SCVE::SCVE_MODE_LOW_POWER);
   else if (trackerMode == 4)
      pNativeContext->pCtx = SCVE::Context::newInstance((SCVE::OperationMode)SCVE::SCVE_TEST_MODE_CPU);
   else if (trackerMode == 5)
      pNativeContext->pCtx = SCVE::Context::newInstance((SCVE::OperationMode)SCVE::SCVE_TEST_MODE_DSP);
   else
   {
      EPRINTF("Invalid 'trackerMode' in Init\n");
      cleanupNativeContext(env, obj, pNativeContext);
      return;
   }
   */

   if (trackerMode == 0)
      opMode = (vendor::qti::hardware::scve::objecttracker::V1_0::OperationMode)SCVE::SCVE_MODE_DEFAULT;
   else if (trackerMode == 1)
      opMode = (vendor::qti::hardware::scve::objecttracker::V1_0::OperationMode)SCVE::SCVE_MODE_PERFORMANCE;
   else if (trackerMode == 2)
      opMode = (vendor::qti::hardware::scve::objecttracker::V1_0::OperationMode)SCVE::SCVE_MODE_CPU_OFFLOAD;
   else if (trackerMode == 3)
      opMode = (vendor::qti::hardware::scve::objecttracker::V1_0::OperationMode)SCVE::SCVE_MODE_LOW_POWER;
   else if (trackerMode == 4)
      opMode = (vendor::qti::hardware::scve::objecttracker::V1_0::OperationMode)(SCVE::OperationMode)SCVE::SCVE_TEST_MODE_CPU;
   else if (trackerMode == 5)
      opMode = (vendor::qti::hardware::scve::objecttracker::V1_0::OperationMode)(SCVE::OperationMode)SCVE::SCVE_TEST_MODE_DSP;
   else
   {
      EPRINTF("Invalid 'trackerMode' in Init\n");
      cleanupNativeContext(env, obj, pNativeContext);
      return;
   }

    int32_t sts=-1;
    int64_t instance_id = 0;
    hidl_handle hal_pSessionUserData;
    Return<void> hidl_ret;

   IObjectTracker::createInstance_cb _hidl_cb1([&sts,&instance_id](int32_t retval, int64_t id){
        ALOGI("In _hidl_cb");
        sts = retval;
        instance_id = id;
    });

   //if (pNativeContext->bUseAsync)
   //{
   //   pNativeContext->pTracker = SCVE::ObjectTracker::newInstance(pNativeContext->pCtx,
   //         pNativeContext->sDim,
   //         pNativeContext->sTrackImage.sFormat,
   //         TrackCallback,
   //         /* No Async-Reg for now*/ NULL,
   //         SCVE::SCVE_OBJECTTRACKER_PRECISION_HIGH,
   //         (void*)pNativeContext);
   //}
   //else
   {
       hal_pSessionUserData.setTo(NULL,true);
       hidl_ret = objTracker->createInstance(opMode,
            pNativeContext->sDim,
            pNativeContext->sTrackImage.sFormat,
            objTrackCallback,
            (::vendor::qti::hardware::scve::objecttracker::V1_0::ObjectTracker_Precision)0,
            hal_pSessionUserData,
            0, _hidl_cb1);
    if(hidl_ret.isOk()){
       ALOGI("ALL okay in hidl tracking return");
    }
    else
       ALOGI("Some problem in hidl tracking return");

       pNativeContext->pTracker = instance_id;
   }

   if (pNativeContext->pTracker == 0)
   {
      EPRINTF("Unable to initialize Tracker");
      cleanupNativeContext(env, obj, pNativeContext);
      return;
   }

   DPRINTF("%s done\n", __FUNCTION__);

   return;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT void JNICALL
Java_com_qualcomm_qti_touch2track_FastCVSample_cleanup
(
   JNIEnv * env,
   jobject obj
)
{
   DPRINTF("%s begin\n", __FUNCTION__);

   TrackerJNIContext_hal *pNativeContext = getNativeContext(env, obj);

   //delete tracker
   if (pNativeContext != NULL)
   {
      if (pNativeContext->pTracker != 0)
      {
         objTracker->destroyInstance(pNativeContext->pTracker);
         pNativeContext->pTracker = 0;
      }

      //Cleanup Context
      //if (pNativeContext->pCtx != NULL)
      //{
      //   SCVE::Context::deleteInstance(pNativeContext->pCtx);
      //   pNativeContext->pCtx = NULL;
      //}

#ifdef CALL_SALMAP
      if (pNativeContext->sSalMap.pPtr)
      {
         free(pNativeContext->sSalMap.pPtr);
      }
#endif

      cleanupNativeContext(env, obj, pNativeContext);
   }

   //SCVE_PROFILE_PRIO_SHOW_STATISTIC(PROFILE_PRIO_HIGH, T2T_APP_TRACK);
   if ( bPrintInfoEveryCall ) {
      //SCVE_PROFILE_PRIO_SHOW_STATISTIC(PROFILE_PRIO_HIGH, T2T_APP_REG_BOX);
      //SCVE_PROFILE_PRIO_SHOW_STATISTIC(PROFILE_PRIO_HIGH, T2T_APP_REG_PT);
   }
   DPRINTF("%s done\n", __FUNCTION__);
   return;
}

JNIEXPORT void
JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_update
(
   JNIEnv      *env,
   jobject     obj,
   jbyteArray  img,
   jintArray   jTrackingResultArray
)
{
   jbyte      *jimgData = NULL;
   jboolean   isCopy = 0;
   uint8_t    *renderBuffer;
   uint64_t   start_time;
   float      timeMs;

   // Get data from JNI
   jimgData = env->GetByteArrayElements(img, &isCopy);
   // Get pointer to output array from JNI
   jint *jTrackingResult = env->GetIntArrayElements(jTrackingResultArray, &isCopy);
   // Get Tracker's JNI context
   TrackerJNIContext_hal *pNativeContext = getNativeContext(env, obj);
   // Get an empty Temp Result index for current frame

   //SCVE::Status retStatus = SCVE::SCVE_ERROR_INTERNAL;
   int nSize = env->GetArrayLength(img);

   uint8_t* temp =  (uint8_t*)rpcmem_alloc(25,RPCMEM_DEFAULT_FLAGS,nSize);
   std::memcpy(temp, (uint8_t*) jimgData, nSize);

    native_handle_t* img_handle = native_handle_create(1,0);
    img_handle->numFds = 1;
    img_handle->data[0] =  rpcmem_to_fd((void*)temp);

   pNativeContext->sTrackImage.pPtr.setTo(img_handle,true);
   pNativeContext->sTrackImage.nBytes = nSize;

   //SCVE_PROFILE_PRIO_START(PROFILE_PRIO_HIGH, T2T_APP_TRACK);
   //if (pNativeContext->bUseAsync)
   //{
   //   //getAvailableTempResultIndex() will wait if there are no available slots to use.
   //   int nResIndex = getAvailableTempResultIndex(pNativeContext);
   //
   //   pNativeContext->bTempUsed[nResIndex] = true;
   //
   //   retStatus = pNativeContext->pTracker->TrackObjects_Async(&pNativeContext->sTrackImage,
   //                                                            pNativeContext->sTempResult[nResIndex],
   //                                                            pNativeContext->nActiveObjs,
   //     /*nBufferBlocked. It is important to pass 1 here -->*/ 1,
   //                                                            &pNativeContext->bTempUsed[nResIndex]);
   //
   //   ASSERT(retStatus != (int)SCVE::SCVE_OBJECTTRACKER_ERROR_TOO_MANY_REQUESTS);
   //
   //   if (retStatus != SCVE::SCVE_SUCCESS && retStatus != SCVE::SCVE_PENDING)
   //   {
   //      EPRINTF("Unable to queue Async Track request");
   //
   //      pthread_mutex_lock(&pNativeContext->hMutex);
   //      pNativeContext->bTempUsed[nResIndex] = false;
   //      pthread_mutex_unlock(&pNativeContext->hMutex);
   //
   //      if (jTrackingResult)
   //      {
   //         memset(jTrackingResult, 0, sizeof(int)*6*pNativeContext->nActiveObjs);
   //      }
   //   }
   //   else
   //   {
   //      if (jTrackingResult)
   //      {
   //         pthread_mutex_lock(&pNativeContext->hMutex);
   //         for (int i = 0; i < (int)pNativeContext->nActiveObjs; i++)
   //         {
   //            jTrackingResult[i * 6 + 0] = pNativeContext->sResult[i].sROI.nLocX;
   //            jTrackingResult[i * 6 + 1] = pNativeContext->sResult[i].sROI.nLocY;
   //            jTrackingResult[i * 6 + 2] = pNativeContext->sResult[i].sROI.nWidth;
   //            jTrackingResult[i * 6 + 3] = pNativeContext->sResult[i].sROI.nHeight;
   //            jTrackingResult[i * 6 + 4] = pNativeContext->sResult[i].nConfidence;
   //            jTrackingResult[i * 6 + 5] = 0;
   //         }
   //         pthread_mutex_unlock(&pNativeContext->hMutex);
   //      }
   //   }
   //}
   //else //Sync Call
   {
      start_time = getTimeMicroSeconds();

      hidl_vec<ObjectTrackerResult_hal> res_vec;
      res_vec.resize(pNativeContext->nActiveObjs);
      int32_t ret_sts = -1;

      IObjectTracker::trackObjects_sync_cb _hidl_cb([&res_vec,&ret_sts](hidl_vec<ObjectTrackerResult_hal> op, int32_t retval){
         ALOGI("In _hidl_cb track sync ");
         ret_sts = retval;
         res_vec = op;
         //ALOGI("In _hidl_cb track sync end assigned");
    });

      objTracker->trackObjects_sync(pNativeContext->sTrackImage,
                                                              pNativeContext->nActiveObjs, _hidl_cb);
      for(int i=0;i<pNativeContext->nActiveObjs;i++){
          pNativeContext->sResult[i].sROI.nLocX =   res_vec[i].sROI.nLocX;
          pNativeContext->sResult[i].sROI.nLocY =   res_vec[i].sROI.nLocY;
          pNativeContext->sResult[i].sROI.nWidth =  res_vec[i].sROI.nWidth;
          pNativeContext->sResult[i].sROI.nHeight = res_vec[i].sROI.nHeight;
          pNativeContext->sResult[i].nConfidence  = res_vec[i].nConfidence;
          pNativeContext->sResult[i].nObjectIndex = res_vec[i].nObjectIndex;
      }
      //Call from here only for Sync API. For Async, make the call in callback function.
      if (ret_sts == SCVE::SCVE_SUCCESS)
      {
         CallSalMapFunctions(pNativeContext);
      }

      timeMs = (getTimeMicroSeconds() - start_time) / 1000.f;
      pNativeContext->state.timeFilteredMs = ((pNativeContext->state.timeFilteredMs * (29.f / 30.f)) +
                                              (float)(timeMs / 30.f));

      if (jTrackingResult)
      {
         if (ret_sts != SCVE::SCVE_SUCCESS)
         {
            memset(jTrackingResult, 0, sizeof(int)*6*pNativeContext->nActiveObjs);
         }
         else
         {
            for (int i = 0; i < (int)pNativeContext->nActiveObjs; i++)
            {
               jTrackingResult[i * 6 + 0] = pNativeContext->sResult[i].sROI.nLocX;
               jTrackingResult[i * 6 + 1] = pNativeContext->sResult[i].sROI.nLocY;
               jTrackingResult[i * 6 + 2] = pNativeContext->sResult[i].sROI.nWidth;
               jTrackingResult[i * 6 + 3] = pNativeContext->sResult[i].sROI.nHeight;
               jTrackingResult[i * 6 + 4] = pNativeContext->sResult[i].nConfidence;
               jTrackingResult[i * 6 + 5] = 0;
            }
         }
      }
   }

   if ( !bPrintInfoEveryCall ) {
      //SCVE_PROFILE_PRIO_END(PROFILE_PRIO_HIGH, T2T_APP_TRACK);
   }
   else {
      // Print detailed debug information to logcat
      //SCVE_PROFILE_PRIO_END2(PROFILE_PRIO_HIGH, T2T_APP_TRACK);
      for (int i = 0; i < (int)pNativeContext->nActiveObjs; i++)
      {
         SCVE_LOG_INFO("Bounding rect for object%i= Left:%d, Top:%d, Width:%d, Height:%d, Confidence:%d",
               i, jTrackingResult[i * 6 + 0], jTrackingResult[i * 6 + 1],
               jTrackingResult[i * 6 + 2], jTrackingResult[i * 6 + 3], jTrackingResult[i * 6 + 4]);
      }
   }

   // Let JNI know we don't need data anymore. this is important!
   env->ReleaseByteArrayElements(img, jimgData, JNI_ABORT);
   env->ReleaseIntArrayElements(jTrackingResultArray, jTrackingResult, JNI_OK);
}

JNIEXPORT int
JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_setROIext
(
   JNIEnv      *env,
   jobject     obj,
   jbyteArray  img,
   int         x,
   int         y,
   int         width,
   int         height,
   jintArray   jObjectHandleArray
   )
{
   jbyte *jimgData = NULL;
   jboolean          isCopy = 0;
   uint32_t *curCornerPtr = 0;
   uint64_t          time;
   float             timeMs;
   int               status = 0;

   TrackerJNIContext_hal *pNativeContext = getNativeContext(env, obj);

   if (pNativeContext->pTracker == 0)
   {
      DPRINTF("%s: Tracker not initialized \n", __FUNCTION__);
      return 1;
   }

   // Get data from JNI
   jimgData = env->GetByteArrayElements(img, &isCopy);

   jint *jObjectHandle = NULL;
   jObjectHandle = env->GetIntArrayElements(jObjectHandleArray, &isCopy);

   //set ROI to KLT trackers.
   //Rect<int> roi_(x,y,width,height);
   //fcvRectangleInt roi_;
   //set_rect(&roi_, x, y, width,height);
   SCVE::Rect ROI;
   ROI.nLocX = x;
   ROI.nLocY = y;
   ROI.nWidth = width;
   ROI.nHeight = height;

   Rect_hal hal_ROI;
   hal_ROI.nLocX = x;
   hal_ROI.nLocY = y;
   hal_ROI.nWidth = width;
   hal_ROI.nHeight = height;

   int nSize = env->GetArrayLength(img);

    uint8_t* temp =  (uint8_t*)rpcmem_alloc(25,RPCMEM_DEFAULT_FLAGS,nSize);
    std::memcpy(temp, (uint8_t*) jimgData, nSize);

    native_handle_t* img_handle = native_handle_create(1,0);
    img_handle->numFds = 1;
    img_handle->data[0] =  rpcmem_to_fd((void*)temp);

   pNativeContext->sRegImage.pPtr.setTo(img_handle,true);
   pNativeContext->sRegImage.nBytes = nSize;

   //pNativeContext->sRegImage.pPtr = (uint8_t *)jimgData;

   if (pNativeContext->nActiveObjs >= MAX_ROIS)
   {
      status = 1;
   }
   else
   {
      //register each object
      DPRINTF("%s Register start: %d %d %d \n", __FUNCTION__, pNativeContext->sRegImage.sDim.nWidth,
            pNativeContext->sRegImage.sDim.nHeight, pNativeContext->sRegImage.sDim.nStride);

      if ( bPrintInfoEveryCall ) {
         SCVE_LOG_INFO("Trying to register object%i by box= Left:%d, Top:%d, Width:%d, Height:%d",
               pNativeContext->nActiveObjs, ROI.nLocX, ROI.nLocY, ROI.nWidth, ROI.nHeight);
         SCVE_PROFILE_PRIO_START(PROFILE_PRIO_HIGH, T2T_APP_REG_BOX);
      }

      int32_t sts=-1;
      int64_t instance_id = 0;

      IObjectTracker::registerObject_sync_cb _hidl_cb([&instance_id,&sts](int32_t id, int64_t retval){
        ALOGI("In _hidl_cb");
        sts = retval;
        instance_id = id;
      });

//#ifdef CALL_SALMAP
//      if (SCVE::SCVE_SUCCESS != pNativeContext->pTracker->RegisterObject_Sync(&pNativeContext->sRegImage,
//               ROI,
//               &pNativeContext->nObjIndex[pNativeContext->nActiveObjs],
//               SCVE::SCVE_OBJECTTRACKER_REGFLAG_SAL_MAP))
//#else
       objTracker->registerObject_sync(pNativeContext->sRegImage,
                  hal_ROI, 0,
                  _hidl_cb);
       pNativeContext->nObjIndex[pNativeContext->nActiveObjs] = instance_id;

     if (SCVE::SCVE_SUCCESS != sts)
//#endif
      {
         status = 1;
         DPRINTF("%s Register failed\n", __FUNCTION__);
      }
      else
      {
         jObjectHandle[0] = pNativeContext->nObjIndex[pNativeContext->nActiveObjs];
         pNativeContext->nActiveObjs++;
         DPRINTF("%s Register success %lld\n", __FUNCTION__, jObjectHandle[0]);
      }

      if ( bPrintInfoEveryCall ) {
         //SCVE_PROFILE_PRIO_END2(PROFILE_PRIO_HIGH, T2T_APP_REG_BOX);
      }

   }

   // Let JNI know we don't need data anymore. this is important!
   env->ReleaseByteArrayElements(img, jimgData, JNI_ABORT);
   env->ReleaseIntArrayElements(jObjectHandleArray, jObjectHandle, JNI_OK);
   return status;
}

JNIEXPORT int
JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_setROIbyPointext
(
   JNIEnv      *env,
   jobject     obj,
   jbyteArray  img,
   int         x,
   int         y,
   int         newRoiFlag,
   jintArray   jObjectHandlePointArray
   )
{
   jbyte *jimgData = NULL;
   jboolean          isCopy = 0;
   uint32_t *curCornerPtr = 0;
   //uint8_t*          renderBuffer;
   uint64_t          time;
   float             timeMs;
   int               status = 0;

   TrackerJNIContext_hal *pNativeContext = getNativeContext(env, obj);

   if (pNativeContext->pTracker == NULL)
   {
      DPRINTF("%s: Tracker not initialized\n", __FUNCTION__);
      return 2;
   }

   if (newRoiFlag == 1)
      DPRINTF("First time object is being registered by point\n");

   DPRINTF("%s: %d,%d\n", __FUNCTION__, x, y);

   // Get data from JNI
   jimgData = env->GetByteArrayElements(img, &isCopy);

   jint *jObjectHandlePoint = NULL;
   jObjectHandlePoint = env->GetIntArrayElements(jObjectHandlePointArray, &isCopy);

   SCVE::Point Point;
   Point.nLocX = x;
   Point.nLocY = y;

   Point_hal hal_Point;
   hal_Point.nLocX = x;
   hal_Point.nLocY = y;

   int nSize = env->GetArrayLength(img);

    uint8_t* temp =  (uint8_t*)rpcmem_alloc(25,RPCMEM_DEFAULT_FLAGS,nSize);
    std::memcpy(temp, (uint8_t*) jimgData, nSize);

    native_handle_t* img_handle = native_handle_create(1,0);
    img_handle->numFds = 1;
    img_handle->data[0] =  rpcmem_to_fd((void*)temp);

   pNativeContext->sRegImage.pPtr.setTo(img_handle,true);
   pNativeContext->sRegImage.nBytes = nSize;
   //pNativeContext->sRegImage.pPtr = (uint8_t *)jimgData;

   DPRINTF("%s Reset start.\n", __FUNCTION__);
   DPRINTF("%s nActiveObjs: %d\n", __FUNCTION__, pNativeContext->nActiveObjs);
   //unregister existing objects first if nActiveObjs equal MAX_ROIS (i.e. maximum allowable objects)
   if (pNativeContext->nActiveObjs == MAX_ROIS)
   {
      status = 2;
   }
   else
   {
      //register each object
      DPRINTF("%s Register start: %d %d %d.\n", __FUNCTION__, pNativeContext->sRegImage.sDim.nWidth,
            pNativeContext->sRegImage.sDim.nHeight, pNativeContext->sRegImage.sDim.nStride);

#ifdef CALL_SALMAP
      newRoiFlag |= SCVE::SCVE_OBJECTTRACKER_REGFLAG_SAL_MAP;
#endif

      if ( bPrintInfoEveryCall ) {
         SCVE_LOG_INFO("Trying to register object%i by point (%d, %d)",
               pNativeContext->nActiveObjs, x, y);
         SCVE_PROFILE_PRIO_START(PROFILE_PRIO_HIGH, T2T_APP_REG_PT);
      }

      int32_t sts=-1;
      int64_t instance_id = 0;

      IObjectTracker::registerObjectByPoint_sync_cb _hidl_cb([&instance_id,&sts](int32_t id, int64_t retval){
        ALOGI("In _hidl_cb");
        sts = retval;
        instance_id = id;
      });

      objTracker->registerObjectByPoint_sync(pNativeContext->sRegImage, hal_Point, newRoiFlag, _hidl_cb);

      pNativeContext->nObjIndex[pNativeContext->nActiveObjs] = (uintptr_t)instance_id;

      if (SCVE::SCVE_SUCCESS == sts)
      {
         jObjectHandlePoint[0] = pNativeContext->nObjIndex[pNativeContext->nActiveObjs];
         DPRINTF("%s jObjectHandlePoint[0] = %d\n", __FUNCTION__, jObjectHandlePoint[0]);
         pNativeContext->nActiveObjs++;
      }
      else if (SCVE::SCVE_PENDING == sts)
      {
         status = 1;
         pNativeContext->nObjIndex[pNativeContext->nActiveObjs] = 0;
         DPRINTF("%s Register pending\n", __FUNCTION__);
      }
      else
      {
         status = 2;
         pNativeContext->nObjIndex[pNativeContext->nActiveObjs] = 0;
         EPRINTF("%s Register failed\n", __FUNCTION__);
      }

      if ( bPrintInfoEveryCall && sts != SCVE::SCVE_PENDING ) {
         //SCVE_PROFILE_PRIO_END2(PROFILE_PRIO_HIGH, T2T_APP_REG_PT);
      }

   }

   // Let JNI know we don't need data anymore. this is important!
   env->ReleaseByteArrayElements(img, jimgData, JNI_ABORT);
   env->ReleaseIntArrayElements(jObjectHandlePointArray, jObjectHandlePoint, JNI_OK);
   return status;
}

JNIEXPORT void
JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_unregisterROI
(
   JNIEnv      *env,
   jobject     obj,
   int         index
)
{
   bool flag = false;

   TrackerJNIContext_hal* pNativeContext = getNativeContext(env, obj);

   if (pNativeContext->pTracker == 0)
      return;

   for (unsigned i = 0; i < pNativeContext->nActiveObjs; i++)
   {
      if (pNativeContext->nObjIndex[i] == (uint32_t)index)
      {
         objTracker->unRegisterObject(pNativeContext->nObjIndex[i]);
         pNativeContext->nActiveObjs--;
         flag = true;
      }
      if (flag == true)
      {
         pNativeContext->nObjIndex[i] = pNativeContext->nObjIndex[i + 1];
      }
   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT jfloat JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_getCameraFPS
(
   JNIEnv   *env,
   jobject  obj
)
{
   TrackerJNIContext_hal *pNativeContext = getNativeContext(env,obj);
   return pNativeContext->state.camFPSCounter.GetFilteredFPS();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_cameraFrameTick
(
   JNIEnv   *env,
   jobject  obj
)
{
   TrackerJNIContext_hal *pNativeContext = getNativeContext(env,obj);
   pNativeContext->state.camFPSCounter.FrameTick();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT jfloat JNICALL
Java_com_qualcomm_qti_touch2track_FastCVSample_getFastCVProcessTime
(
   JNIEnv   *env,
   jobject  obj
)
{
   TrackerJNIContext_hal *pNativeContext = getNativeContext(env,obj);
   return (jfloat)pNativeContext->state.timeFilteredMs;
}

