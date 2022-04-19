/**********************************************************************
*  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __QCAMERA_PPEISCORE_H__
#define __QCAMERA_PPEISCORE_H__

extern "C" {
#include "img_common.h"
#include "img_meta.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include "img_queue.h"

}
#include "img_dbg.h"
#include "QCameraBufferMap.h"

void checkglError(const char* op);
void checkEglError(const char* op);

/**
*  MACROS and CONSTANTS
 **/
#define INPUT_FRAME_IDX                 0
#define REFERENCE_FRAME_IDX             1
#define OUTPUT_FRAME_IDX                0
#define MAX_FRAMES_FOR_DUMP             30
#define API_ERROR                       -1
#define FASTRPC_ERROR                   -2

typedef struct
{
  int    winW;
  int    winH;
  int    rBits;
  int    gBits;
  int    bBits;
  int    aBits;
  int    bitsPerPixel;
  int    depthBits;
  int    stencilBits;
  int    sampleBuffers;
  int    samples;
}Config;

/*
 * ppeis context Data
 */

typedef struct _ppeis_context_t{
  __GLsync*  glSyncs;
  void *user_data;
}ppeis_context_t;


/*========QCameraPPEisCore====================================================
 * Class: QCameraPPEisCore
 *
 * Description: This class represents a temple implementation
 *
 * Notes: none
 *==========================================================================*/
class QCameraPPEisCore
{
public:
 /**
  * Function: QCameraPPEisCore
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
  QCameraPPEisCore();

 /**
  * Function: ~QCameraPPEisCore
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
  ~QCameraPPEisCore();

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
bool Init();

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
 * Returns deinit state
 **/

bool IsDeinitStarted() {
  return bIsDeinitStarted;
}

/**
 * Sets deinit state
 **/

inline void setDeinitStart(bool bVal) {
  bIsDeinitStarted = bVal;
}

void Flush();

 /**
  * Function: processData
  *
  * Description: main processing body
  *
  * Arguments:
  *   @arg1: p_in_frame: input frame handler
  *   @arg2: in_frame_cnt: number of input frames
  *   @arg3: p_out_frame: output frame handler
  *   @arg4: out_frame_cnt: number of output frames
  *   @arg3: p_meta: frame meta data, containing ROI info
  *
  * Return values:
  *   IMG_SUCCESS or IMG_ERR_GENERAL
  *
  * Notes: none
  **/
  int8_t processData(img_frame_t *inFrame[], int inFrameCnt,
    img_frame_t *outFrame[], int outFrameCnt,
    img_meta_t* p_meta);

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
  inline void setFrameOps(img_frame_ops_t *pFrameOps) {mFrameOps = *pFrameOps;}

private:

 /** bufInfo_t
  *  @addr:    buf addr
  *  @length:  buf length
  *  @fd:      buf FD
  *
  *  Struct for registered buf list
  */
  typedef struct {
    uint8_t* addr;
    uint32_t length;
    int32_t fd;
  } bufInfo_t;

  /*
   * Thread context Data
   */
  typedef struct _ppeis_thread_data_t{
    EGLContext   context;
    EGLSurface   surface;
    EGLDisplay   display;
  }ppeis_thread_data_t;

  /*
   * Default context Data
   */
  EGLContext   m_defaultContext;
  EGLDisplay   m_defaultDisplay;
  EGLSurface   m_defaultSurface;

  EGLint      majorVersion;
  EGLint      minorVersion;
  EGLConfig   config;



/**
  * Function: registerBuffer
  *
  * Description: Register ION buffer to enable sharing buffer
  *   with the dsp via the smmu
  *
  * Input parameters:
  *   @buf: virtual ION buffer address
  *   @size: size of the buffer
  *   @fd:   FD of ION buffer
  *
  * Return values:
  *     None
  *
  * Notes: none
  **/
  void registerBuffer(void* buf, int size, int fd);

 /**
  * Frame ops
  */
  img_frame_ops_t mFrameOps;

 /**
  * Buffer count
  */
  int32_t mBufCnt;

 /**
  * flag to check if module is initialized and avoid multiple initialization
  */
  bool  mPPEisCoreInitialized;

  GLuint mShaderProgram;
  GLint mPosId;
  GLint mTexId;
  GLint mTransId;
  float *mVertices;
  float *mTexCoordinates;
  unsigned short *mIndices;
  GLuint mIndexVBOID;
  bool bIsDeinitStarted;
  QCameraBufferMap mBufferMap;

  bool CreateContext(EGLDisplay   display,
    EGLContext   sharedContext,
    EGLSurface   *surface,
    EGLContext   *context);

public:
  Config cfg;
  int (*callback)(void *user_data);
  img_queue_t callBackQ;
  pthread_t threadId;
  ppeis_thread_data_t *m_pThreadData;
  ppeis_context_t     *m_pPpeisContext;



};

#endif
