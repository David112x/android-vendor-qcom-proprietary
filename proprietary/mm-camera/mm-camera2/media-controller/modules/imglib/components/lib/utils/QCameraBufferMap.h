/**********************************************************************
*  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __QCAMERA_BUFFERMAP_H__
#define __QCAMERA_BUFFERMAP_H__

extern "C" {
#include "img_common.h"
#include "img_mem_ops.h"
#include "img_buffer.h"

}

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
//#ifndef _LE_CAMERA_
//#include <gui/Surface.h>
//#include <gui/SurfaceComposerClient.h>
//#endif
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <system/graphics.h>
#include <gralloc_priv.h>
#include <android/log.h>
#include <stdio.h>
#include <vector>
#include <pthread.h>
#include <map>
#include <ui/GraphicBuffer.h>
#include <qdMetaData.h>


//#include "mm_jpeg_ionbuf.h"
using namespace android;



typedef struct {
  uint8_t* addr;
  uint32_t length;
  int32_t fd;
} bufInfo_t;

class Gralloc1Utils {
  public:
    Gralloc1Utils();
    static Gralloc1Utils* GetInstance() {
        static Gralloc1Utils* instance = new Gralloc1Utils();
        return instance;
    }
    int Retain(private_handle_t *hnd);
    int Release(private_handle_t *hnd);
  private:
    const hw_module_t *module_;
    gralloc1_device_t* device_;
    GRALLOC1_PFN_RETAIN retain_;
    GRALLOC1_PFN_RELEASE release_;
};

class NativeBuffer : public ANativeWindowBuffer, public android::RefBase {
  public:
    void incStrong(const void* id) const { RefBase::incStrong(id); }
    void decStrong(const void* id) const { RefBase::decStrong(id); }

    NativeBuffer(int width_in, int height_in, int stride_in, int format_in,
                 int usage_in, native_handle_t *handle_in) :
      ANativeWindowBuffer(), RefBase() {
      width = width_in;
      height = height_in;
      stride = stride_in;
      format = format_in;
      usage = usage_in;
      handle = handle_in;
      common.incRef = incRef;
      common.decRef = decRef;
    }

  private:
    static void incRef(android_native_base_t* base) {
      NativeBuffer *self = static_cast<NativeBuffer*>(
        reinterpret_cast<ANativeWindowBuffer*>(base));
      self->incStrong(self);
    }
    static void decRef(android_native_base_t* base) {
      NativeBuffer *self = static_cast<NativeBuffer*>(
        reinterpret_cast<ANativeWindowBuffer*>(base));
      self->decStrong(self);
    }
  };

typedef struct {
  uint32_t index;
  uint32_t isInputBuffer;
  img_mem_handle_t meta_buffer;
  private_handle_t *privateHandle;
  sp<NativeBuffer> pNativeBuffer;
  EGLImageKHR hEGLImage;
  GLuint hColorTexture;
  Gralloc1Utils *gralloc_utils_;
  GLuint hRBO;  // Only for output buffers
  GLuint hFBO;  // Only for output buffers
}bufMapInfo;
/*========QCameraBufferMap=======================================
 * Class: QCameraBufferMap
 *
 * Description: This class provides functionality to map native buffers to graphics buffer handles
 *
 * Notes: none
 *====================================================*/
class QCameraBufferMap
{
public:
 /**
  * Function: QCameraBufferMap
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
  QCameraBufferMap();

 /**
  * Function: ~QCameraBufferMap
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
  ~QCameraBufferMap();

 //void Init(size_t size, int32_t flags, uint32_t inUsage,
  //int32_t bufferType, int32_t format, int32_t width, int32_t height);
 void Init();
 void DeInit();
 void FlushMaps();
 GLuint GetSourceTexture(uint32_t index, uint32_t fd, uint32_t size,
  int32_t update_format, int32_t width, int32_t height, int32_t stride,
  int32_t scanline);
 GLuint GetFrameBuffer(uint32_t index, uint32_t fd, uint32_t size,
   int32_t update_format, int32_t width, int32_t height, int32_t stride,
   int32_t scanline);

 private:
  int32_t mFlags;
  uint32_t mUsage;
  int32_t mBufferType;
  int32_t mFormat;
  #if 0
  size_t mSize;
  int32_t mWidth;
  int32_t mHeight;
  int32_t mStride;
  int32_t mScanline;
  #endif
  std::map<uint32_t ,bufMapInfo > mSourceTextureMap;
  std::map<uint32_t ,bufMapInfo > mFrameBufferMap;
};
#endif
