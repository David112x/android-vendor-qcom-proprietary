/***************************************************************************
* Copyright (c) 2017-2019 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#ifndef __QCAMERA_BOKEH_H__
#define __QCAMERA_BOKEH_H__

extern "C" {
#include "img_common.h"
#include "img_meta.h"
#include "img_queue.h"
}

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglextQCOM.h>

#include "img_dbg.h"
#include "QCameraBufferMap.h"
#include "cam_intf.h"

#ifdef ENABLE_QC_BOKEH
#include "videobokeh_wrapper.h"
#endif

#include "Effects.h"
#include "Texture.h"

using namespace android;

#define BOKEH_DDM_AUX_WIDTH 640
#define BOKEH_DDM_AUX_HEIGHT 480
#define BOKEH_DDM_TARGET_WIDTH 320
#define BOKEH_DDM_TARGET_HEIGHT 240
#define BOKEH_OUTPUT_WIDTH_ALIGN 64
#define BOKEH_OUTPUT_HEIGHT_ALIGN 2

typedef struct {
  uint32_t width;
  uint32_t height;
} slave_bokeh_dim;

typedef struct
{
  int winW;
  int winH;
  int rBits;
  int gBits;
  int bBits;
  int aBits;
  int bitsPerPixel;
  int depthBits;
  int stencilBits;
  int sampleBuffers;
  int samples;
} Config;

typedef struct {
  uint32_t frameId;
  uint8_t* slaveDepthAddr;
  int slaveDepthStride;
  uint8_t* masterDepthAddr;
  int masterDepthStride;
  uint8_t* outputBuffer;
  int outputWidth;
  int outputHeight;
  float slaveFocalLengthRatio;
  float masterFocalLengthRatio;
  int focusROIX;
  int focusROIY;
  float blurIntensity;
  GLuint textureId;
  EGLDisplay display;
  EGLContext context;
  EGLSurface surface;
} ddm_algo_job_t;

typedef struct {
  uint32_t master_width;
  uint32_t master_height;
  uint32_t slave_width;
  uint32_t slave_height;
} bokeh_dim;

typedef struct {
  img_frame_t **pFrame;
  void *pBokehObj;
} ddmblur_algo_init_t;


/*============================================================================
 * Class: QCameraBokeh
 *
 * Description: This class wraps the Depth Map and Blur algorithms
 *              used to create Real-Time Bokeh
 *
 * Notes: none
 *==========================================================================*/
class QCameraBokeh
{
public:
  /**
   * Function: QCameraBokeh
   *
   * Description: constructor of class
   *
   * Arguments:
   *   none
   *
   * Return values:
   *     none
   *
   * Notes: none
   **/
  QCameraBokeh();

  /**
   * Function: ~QCameraBokeh
   *
   * Description: destructor of class
   *
   * Arguments:
   *   none
   *
   * Return values:
   *     none
   *
   * Notes: none
   **/
  ~QCameraBokeh();

  /**
   * Function: Init
   *
   * Description: Initialization for the class
   *
   * Arguments:
   *   none
   *
   * Return values:
   *     True/False: Status of  initialization
   *
   * Notes: none
   **/
  bool Init(img_multicam_init_params_t *p_params);

  /**
   * Function: DeInit
   *
   * Description: Deinitialization for the class
   *
   * Arguments:
   *   none
   *
   * Return values:
   *     True/False: Status of  initialization
   *
   * Notes: none
   **/
  bool DeInit();

  /**
   * Function: QCameraBokeh::Abort
   *
   * Description: Abort DDM processing
   *
   * Arguments:
   *   none
   *
   * Return values:
   *     True/False: Status of DDM abort
   *
   * Notes: none
   **/
  bool Abort();

  /**
   * Function: QCameraBokeh::Flush
   *
   * Description: Flush function of class
   *
   * Arguments:
   *
   * Return values:
   *     none
   *
   * Notes: none
   **/
  void Flush();

  /**
   * Function: BlurInit
   *
   * Description: Function to init blur wrapper
   *
   * Arguments:
   *	@inputWidth: Input frame width
   *	@inputHeight: Input frame height
   *	@p_obj: pointer to user data
   *
   * Return values:
   *   0 on success, -ve on failure
   *
   * Notes: none
   **/
  static int32_t BlurInit(uint32_t inputWidth,
    uint32_t inputHeight, void *p_obj);

  /**
   * Function: DDMInit
   *
   * Description: Function to init DDM library wrapper
   *
   * Arguments:
  *    @bokeh_dim: Master/Slave frame dimensions
   *   @p_userdata: pointer to user data
   *
   * Return values:
   *   0 on success, -ve on failure
   *
   * Notes: none
   **/
  static int32_t DDMInit(bokeh_dim *bokeh_dim,
    void *p_obj);

  /**
   * Function: DDMBlurInitThread
   *
   * Description: DDM and blur wrapper init thread
   * to do initialize asynchronously. Until initialization
   * is done, input master frames will be copied to output
   *
   * Arguments:
   *   @data: input data
   *
   * Return values:
   *   None
   *
   * Notes: none
   **/
  static void* DDMBlurInitThread(void *arg);

  /**
   * Function: Execute
   *
   * Description: main processing body
   *
   * Arguments:
   *   @p_aInFrame: input frame handler
   *   @aInFrameCnt: number of input frames
   *   @p_aOutFrame: output frame handler
   *   @aOutFrameCnt: number of output frames
   *   @p_aMeta: frame meta data
   *   @aMetaCount: number of metadata
   *
   * Return values:
   *   IMG_SUCCESS or IMG_ERR_GENERAL
   *
   * Notes: none
   **/
  int32_t Execute(img_frame_t *p_aInFrame[],
    int aInFrameCnt,
    img_frame_t  *p_aOutFrame[],
    int aOutFrameCnt,
    img_meta_t *p_aMeta[],
    int aMetaCount);

  /**
   * Function: AlgoThreadCanWait
   *
   * Description: Function to check if thread can wait
   *
   * Arguments:
   *   @p_userdata: pointer to user data
   *
   * Return values:
   *   true/false
   *
   * Notes: none
   **/
  static int AlgoThreadCanWait(void *p_userdata);

  /**
   * Function: AlgoThreadloop
   *
   * Description: Algo thread loop used for async calls
   *
   * Arguments:
   *   @data: input data
   *
   * Return values:
   *   None
   *
   * Notes: none
   **/
  static void* AlgoThreadloop(void* data);

  /**
   * Function: FillAlgoOutputInfo
   *
   * Description: Fill OTP Calibration Data info by from imglib meta
   * structure
   *
   * Arguments:
   *   @calib_data: OTP calibration data
   *
   * Return values:
   *     none
   *
   * Notes: none
   **/
  void fillOTPCalibrationData(cam_related_system_calibration_data_t * calib_data);

  /**
   * Function: fillROICropData
   *
   * Description: Fill ROI and Crop data from the current stream to
   * structure
   *
   * Arguments:
   *   @p_aMeta: metadata of input frame
   *   @buf: buffer to be filled with roi/crop data
   *
   * Return values:
   *     none
   *
   * Notes: none
   **/
  void fillROICropData(img_meta_t *p_aMeta, char* buf);

 /**
  * Function: setFrameOps
  *
  * Description: set frame ops
  *
  * Arguments:
  *   @pFrameOps: pointer to frame ops
  *
  * Return values:
  *   none
  *
  * Notes: none
  **/
  inline void setFrameOps(img_frame_ops_t *pFrameOps) {m_frameOps = *pFrameOps;}

 /**
  * Function: flushmaps
  *
  * Description: flush buffer maps allocated
  *
  * Arguments:
  *   @p_aMeta: metadata of input frame
  *   @m_masterIndex: master index value
  * Return values:
  *   none
  *
  * Notes: none
  **/
  inline void flushmaps (img_meta_t* pMeta[], int m_masterIndex) {
    if ((pMeta[m_masterIndex]->hal_verison == CAM_HAL_V3) &&
        (pMeta[m_masterIndex]->before_cpp == FALSE))
    {
     m_bufferMap.FlushMaps();
    }
  }

private:

  /**
   * Frame ops
   **/
  img_frame_ops_t m_frameOps;

  GLuint m_blurTextureId;
  GLuint m_rgbOutTextureId;
  EGLDisplay m_display;
  EGLContext m_context;
  EGLSurface m_surface;
  EGLContext m_contextDepth;
  uint8_t* m_blurTextureData;
  uint32_t m_width;
  uint32_t m_height;
  static pthread_mutex_t m_lock;
  QCameraBufferMap m_bufferMap;
  uint8_t m_bokehInitialized;
  Effects* m_pBokehEffects;
  img_meta_t* m_pSlaveMeta;
  img_meta_t* m_pMasterMeta;
  uint32_t m_slaveIndex;
  uint32_t m_masterIndex;
  int m_blurSizeWidth;
  int m_blurSizeHeight;
  float m_focalRatio;
  size_t m_blurSize;
  char m_pCalibrationData[2048];
  char m_pMasterROICrop[1024];
  char m_pSlaveROICrop[1024];
  int m_goodROISrcX;
  int m_goodROISrcY;
  int m_goodROISrcWidth;
  int m_goodROISrcHeight;
  uint32_t isBayerMono;
  uint32_t optmasterswap;
  float lux_index;
  uint32_t m_frameNum;
  uint32_t m_masterSensorWidth;
  uint32_t m_masterSensorHeight;
  uint32_t m_tracingEnabled;
  Texture* m_pInputColorMap;
  Texture* m_pInputBlurRadiusMap;
  uint32_t m_isDepthAlgoDisabled;
  uint32_t m_isRTBAlgoDummy;
  bool enableHVX;
  int m_dumpNow;
  int m_dumpCount;
  int m_dumpingEnabled;
  uint32_t m_depthStatus;
  int32_t m_wrapperInitStatus;
  bool m_wrapperInitdone;
  uint64_t algoframe_timestamp;

  /**
   *  Thread id for thread to calculate DDM
   **/
  pthread_t m_threadId;

  pthread_t m_initthreadId;

  /**
   *  Thread lock
   **/
  pthread_mutex_t m_threadLock;

  /**
   *  Thread condition
   **/
  pthread_cond_t m_threadCond;

  /**
   *  Flag to indicate if thread created
   **/
  bool m_threadCreated;

  /**
   *  Flag to indicate if thread created
   **/
  bool m_InitthreadCreated;

  /**
   *  Flag to trigger thread exit
   **/
  bool m_threadExit;

  /**
   *  DDM compute job queue
   **/
  img_queue_t m_jobQueue;

  bool m_DDMObjInitialized;

#ifdef ENABLE_QC_BOKEH
  qrcp::VideoBokehWrapper *m_pBokehWrapper;
#endif

};

#endif
