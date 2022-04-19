/**********************************************************************
*  Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "QCameraBufferMap.h"



// Gralloc1Utils

Gralloc1Utils::Gralloc1Utils() {
  int ret = 0;
  retain_ = NULL;
  release_ = NULL;

  ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module_);
  if (ret != 0) {
    IDBG_ERROR("hw_get_module FAILED ret %d", ret);
    return;
  }
  // Check the gralloc API version for gralloc1
  if (((module_->module_api_version >> 8) & 0xFF) != 1) {
    IDBG_ERROR(" unsupported gralloc version 0x%X",
      module_->module_api_version);
    return;
  }
  ret = gralloc1_open(module_, &device_);
  if (ret != 0) {
    IDBG_ERROR("gralloc1_open FAILED ret %d", ret);
  } else {
    retain_ = reinterpret_cast<GRALLOC1_PFN_RETAIN>(device_->getFunction(
      device_, GRALLOC1_FUNCTION_RETAIN));
    release_ = reinterpret_cast<GRALLOC1_PFN_RELEASE>(device_->getFunction(
      device_, GRALLOC1_FUNCTION_RELEASE));
  }
}

int Gralloc1Utils::Retain(private_handle_t *hnd) {
  if (!hnd || !device_ || !retain_) {
    IDBG_ERROR("Retain :hnd NULL or device_ NULL or retain_ NULL");
    return -1;
  }
  return retain_(device_, (buffer_handle_t) hnd);
}

int Gralloc1Utils::Release(private_handle_t *hnd) {
  if (!hnd  || !device_ || !release_) {
    IDBG_ERROR("Release :hnd NULL or device_ NULL or release_ NULL");
    return -1;
  }
  return release_(device_, (buffer_handle_t) hnd);
}

// EGLImage extension function pointers
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC                     p_glEGLImageTargetTexture2DOES = NULL;
PFNEGLCREATEIMAGEKHRPROC                                p_eglCreateImageKHR            = NULL;
PFNEGLDESTROYIMAGEKHRPROC                               p_eglDestroyImageKHR           = NULL;
PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC p_glEGLImageTargetRenderbufferStorageOES = NULL;

#if 1
void checkglError(const char* op)
{
    GLint error = glGetError();
    if(error) {
        IDBG_ERROR("after %s() glError (0x%x)", op, error);
    } else {
        IDBG_LOW("No GL error");
    }
}

void checkEglError(const char* op)
{
    GLint error = eglGetError();
    if (error != 0x3000) {
      IDBG_ERROR("after %s() eglError (0x%x)", op, error);
    } else {
      IDBG_LOW("No eGL error");
    }
}
#endif

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
QCameraBufferMap::QCameraBufferMap()
{

}

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
QCameraBufferMap::~QCameraBufferMap()
{

}

void QCameraBufferMap::Init()
{
  //IDBG_LOW("E");
  mFlags = 0x8 | 0x40000000;
  mUsage = GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_RENDER;
  mBufferType = BUFFER_TYPE_UI;
  mFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS;

  p_glEGLImageTargetTexture2DOES = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
  p_eglCreateImageKHR  = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
  p_eglDestroyImageKHR = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
  p_glEGLImageTargetRenderbufferStorageOES = reinterpret_cast<PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC>(eglGetProcAddress("glEGLImageTargetRenderbufferStorageOES"));
}

void QCameraBufferMap::DeInit()
{
  IDBG_LOW("E");
  mFlags = 0;
  mUsage = 0;
  mBufferType = 0;
  mFormat = 0;

  FlushMaps();
  p_glEGLImageTargetTexture2DOES = NULL;
  p_eglCreateImageKHR  = NULL;
  p_eglDestroyImageKHR = NULL;
  p_glEGLImageTargetRenderbufferStorageOES = NULL;

}

void QCameraBufferMap::FlushMaps()
{
  IDBG_LOW("Flushing Source Texture Map");
  std::map<uint32_t ,bufMapInfo>::iterator itr = mSourceTextureMap.begin();
  while (itr != mSourceTextureMap.end()) {
    bufMapInfo info = itr->second;
    IDBG_LOW("Flushing element for idx %d ", info.index);
    glDeleteTextures(1, &info.hColorTexture);
    if(info.hEGLImage) {
      p_eglDestroyImageKHR(eglGetCurrentDisplay(), info.hEGLImage);
      checkEglError("Destroy image");
    }
    //Free metadata buffer
    img_buffer_release(&info.meta_buffer);

    //Free handle
    info.gralloc_utils_->Release(info.privateHandle);

    itr = mSourceTextureMap.erase(itr);

  }
  IDBG_LOW("Flushing Frame Buffer Map");
  itr = mFrameBufferMap.begin();
  while (itr != mFrameBufferMap.end()) {
    bufMapInfo info = itr->second;

    IDBG_MED("Flushing element for idx %d ", info.index);
    glDeleteFramebuffers(1, &info.hFBO);
    glDeleteRenderbuffers(1, &info.hRBO);

    if(info.hEGLImage){
      p_eglDestroyImageKHR(eglGetCurrentDisplay(), info.hEGLImage);
      checkEglError("Destroy image");
    }
    //Free metadata buffer
    img_buffer_release(&info.meta_buffer);

    //Free handle
    info.gralloc_utils_->Release(info.privateHandle);

    itr = mFrameBufferMap.erase(itr);

  }
}

GLuint QCameraBufferMap::GetSourceTexture(uint32_t index, uint32_t fd,
  uint32_t size, int32_t update_format, int32_t width, int32_t height,
  int32_t stride, int32_t scanline)
{
  IDBG_LOW("E");
  int rc = 0;
  GLint error = 0;
  std::map<uint32_t ,bufMapInfo> ::iterator it;
  EGLint vEGLAttribs[] = {EGL_WIDTH,  width, EGL_HEIGHT, height, EGL_NONE};
  bufMapInfo mapInfo;

  it = mSourceTextureMap.find(index);
  if (it != mSourceTextureMap.end()) {
    IDBG_LOW("Map found!");
    bufMapInfo info = it->second;

    IDBG_LOW("index %d tex %d!", index, info.hColorTexture);

    return info.hColorTexture;
  } else {

  IDBG_LOW("Map miss! Need to add mapping!");

    //bufMapInfo *mapInfo = (bufMapInfo *)malloc(sizeof(bufMapInfo));

    memset(&mapInfo, 0, sizeof(bufMapInfo));
    mapInfo.gralloc_utils_ = Gralloc1Utils::GetInstance();

    mapInfo.index = index;
#ifndef TARGET_ION_ABI_VERSION
    rc = img_buffer_get(IMG_BUFFER_ION_IOMMU, ION_HEAP(ION_IOMMU_HEAP_ID), 0, ROUND_UP_PAGESIZE(sizeof(MetaData_t)),
      &mapInfo.meta_buffer);
#else
    rc = img_buffer_get(IMG_BUFFER_ION_IOMMU, ION_HEAP(ION_SYSTEM_HEAP_ID), 0, ROUND_UP_PAGESIZE(sizeof(MetaData_t)),
      &mapInfo.meta_buffer);
#endif
    if (IMG_ERROR(rc)) {
      IDBG_ERROR(" cannot alloc meta buffer");
      return (GLuint)0;
    }

    MetaData_t *p_meta_data = (MetaData_t *)mapInfo.meta_buffer.vaddr;

#ifdef TARGET_USES_GRALLOC1

  int priv_flags = private_handle_t::PRIV_FLAGS_USES_ION |
    private_handle_t::PRIV_FLAGS_NON_CPU_WRITER |
    private_handle_t::PRIV_FLAGS_CAMERA_WRITE |
    private_handle_t::PRIV_FLAGS_HW_TEXTURE;
    int dup_fd = dup(fd);
    int dup_meta_buffer_fd = dup(mapInfo.meta_buffer.fd);

  gralloc1_producer_usage_t pu = (gralloc1_producer_usage_t)
    (GRALLOC1_PRODUCER_USAGE_CPU_READ | GRALLOC1_PRODUCER_USAGE_CAMERA);
  gralloc1_consumer_usage_t cu = (gralloc1_consumer_usage_t)
    (GRALLOC1_CONSUMER_USAGE_CPU_READ | GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE);

#ifdef TARGET_USES_GRALLOC
  mapInfo.privateHandle = new private_handle_t(dup_fd, dup_meta_buffer_fd,
    priv_flags, width, height, width, height,
    update_format, mBufferType, size);
#else
  mapInfo.privateHandle = new private_handle_t(dup_fd, dup_meta_buffer_fd,
    priv_flags, width, height, width, height,
    update_format, mBufferType, size, pu, cu );
#endif

#else
    mapInfo.privateHandle = new private_handle_t(fd, size, mFlags,
      mBufferType, update_format, width, height,
      mapInfo.meta_buffer.fd, 0, (uint64_t)p_meta_data);
#endif

    if (mapInfo.privateHandle == NULL)
    {
      IDBG_ERROR("create private_handle_t failed!!");
      goto private_handle_failed;
    }

    mapInfo.gralloc_utils_->Retain(mapInfo.privateHandle);

    BufferDim_t dim;
    dim.sliceWidth  = stride;
    dim.sliceHeight = scanline;
    int ret = setMetaData(mapInfo.privateHandle, UPDATE_BUFFER_GEOMETRY, &dim);
    if (ret < 0) {
      IDBG_LOW("setMetaData(UPDATE_BUFFER_GEOMETRY) failed %d", ret);
      goto eglcreateimage_failed;
    }
    IDBG_LOW(" wxh %dX%d strXsca %dX%d ret %d", width, height, dim.sliceWidth, dim.sliceHeight, ret);

    ColorSpace_t colorSpace = ITU_R_601_FR;
    ret = setMetaData(mapInfo.privateHandle, UPDATE_COLOR_SPACE, &colorSpace);
    if (ret < 0) {
      IDBG_LOW("setMetaData(UPDATE_COLOR_SPACE) failed %d", ret);
      goto eglcreateimage_failed;
    }

    mapInfo.pNativeBuffer = new NativeBuffer(width, height, stride, update_format,
      mUsage, mapInfo.privateHandle);
    if (mapInfo.pNativeBuffer == NULL)
    {
        IDBG_ERROR("GraphicBuffer Creation Failed \n");
        goto eglcreateimage_failed;
    }

    EGLClientBuffer pNativeBuffer = (EGLClientBuffer)static_cast<ANativeWindowBuffer*>
      (mapInfo.pNativeBuffer.get());

    mapInfo.hEGLImage = p_eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, pNativeBuffer, vEGLAttribs);
    if (!mapInfo.hEGLImage)
    {
        IDBG_ERROR("EGLImage creation failed");
        goto eglcreateimage_failed;
    }

    glGenTextures(1, &mapInfo.hColorTexture);
    error = glGetError();
    if(error != GL_NO_ERROR) {
      IDBG_ERROR("after glGenTextures eglError (0x%x)", error);
      goto glgeterror_failed;
    }
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mapInfo.hColorTexture);
    checkglError("glBindTexture");
    p_glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, mapInfo.hEGLImage);
    checkEglError("p_glEGLImageTargetTexture2DOES");
    checkglError("p_glEGLImageTargetTexture2DOES");
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkglError("glTexParameteri");
    mapInfo.isInputBuffer = TRUE;
    mapInfo.hRBO = 0;
    mapInfo.hFBO = 0;
    //Insert into the map
    IDBG_LOW("Inserting index %d tex %d!", index, mapInfo.hColorTexture);
    mSourceTextureMap[index] = mapInfo;
    return mapInfo.hColorTexture;
  }

  IDBG_LOW("X");
glgeterror_failed:
  if(mapInfo.hColorTexture) {
    glDeleteTextures(1, &mapInfo.hColorTexture);
  }
  if(mapInfo.hEGLImage) {
    p_eglDestroyImageKHR(eglGetCurrentDisplay(), mapInfo.hEGLImage);
    checkEglError("Destroy image");
   }

eglcreateimage_failed:
  delete mapInfo.privateHandle;

private_handle_failed:
  img_buffer_release(&mapInfo.meta_buffer);
  return (GLuint)0;
}

GLuint QCameraBufferMap::GetFrameBuffer(uint32_t index, uint32_t fd,
  uint32_t size, int32_t update_format, int32_t width, int32_t height,
  int32_t stride, int32_t scanline)
{
  IDBG_LOW("E");
  int rc = 0;
  GLint error = 0;
  std::map<uint32_t ,bufMapInfo> ::iterator it;
  EGLint vEGLAttribs[] = {EGL_WIDTH,  width, EGL_HEIGHT, height, EGL_NONE};
  bufMapInfo mapInfo;

  it = mFrameBufferMap.find(index);
  if (it != mFrameBufferMap.end()) {
    IDBG_LOW("Map found!");
    bufMapInfo info = it->second;

    IDBG_LOW("index %d fbo %d!", index, info.hFBO);

    return info.hFBO;
  } else {

    IDBG_LOW("Map miss! Need to add mapping!");

    memset(&mapInfo, 0, sizeof(bufMapInfo));
    mapInfo.gralloc_utils_ = Gralloc1Utils::GetInstance();

    mapInfo.index = index;
#ifndef TARGET_ION_ABI_VERSION
    rc = img_buffer_get(IMG_BUFFER_ION_IOMMU, ION_HEAP(ION_IOMMU_HEAP_ID), 0, ROUND_UP_PAGESIZE(sizeof(MetaData_t)),
      &mapInfo.meta_buffer);
#else
   rc = img_buffer_get(IMG_BUFFER_ION_IOMMU, ION_HEAP(ION_SYSTEM_HEAP_ID), 0, ROUND_UP_PAGESIZE(sizeof(MetaData_t)),
     &mapInfo.meta_buffer);
#endif
    if (IMG_ERROR(rc)) {
      IDBG_ERROR(" cannot alloc meta buffer");
      return (GLuint)0;
    }

    MetaData_t *p_meta_data = (MetaData_t *)mapInfo.meta_buffer.vaddr;

#ifdef TARGET_USES_GRALLOC1
    int priv_flags = private_handle_t::PRIV_FLAGS_USES_ION |
      private_handle_t::PRIV_FLAGS_NON_CPU_WRITER |
      private_handle_t::PRIV_FLAGS_CAMERA_WRITE |
      private_handle_t::PRIV_FLAGS_HW_TEXTURE;
      int dup_fd = dup(fd);
      int dup_meta_buffer_fd = dup(mapInfo.meta_buffer.fd);

  gralloc1_producer_usage_t pu = (gralloc1_producer_usage_t)
    (GRALLOC1_PRODUCER_USAGE_CPU_READ | GRALLOC1_PRODUCER_USAGE_CAMERA);
  gralloc1_consumer_usage_t cu = (gralloc1_consumer_usage_t)
    (GRALLOC1_CONSUMER_USAGE_CPU_READ | GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE);

#ifdef TARGET_USES_GRALLOC
  mapInfo.privateHandle = new private_handle_t(dup_fd, dup_meta_buffer_fd,
    priv_flags, width, height, width, height,
    update_format, mBufferType, size);
#else
  mapInfo.privateHandle = new private_handle_t(dup_fd, dup_meta_buffer_fd,
    priv_flags, width, height, width, height,
    update_format, mBufferType, size, pu, cu );
#endif
#else
    mapInfo.privateHandle = new private_handle_t(fd, size, mFlags,
      mBufferType, update_format, width, height, mapInfo.meta_buffer.fd,
      0, (uint64_t)p_meta_data);
#endif

    if (mapInfo.privateHandle == NULL)
    {
      IDBG_ERROR("create private_handle_t failed!!");
      goto private_handle_failed;
    }

    mapInfo.gralloc_utils_->Retain(mapInfo.privateHandle);

    BufferDim_t dim;
    dim.sliceWidth  = stride;
    dim.sliceHeight = scanline;
    int ret = setMetaData(mapInfo.privateHandle, UPDATE_BUFFER_GEOMETRY, &dim);
    if (ret < 0) {
      IDBG_ERROR("setMetaData(UPDATE_BUFFER_GEOMETRY) failed %d", ret);
      goto eglcreateimage_failed;
    }
    IDBG_LOW(" wxh %dX%d strXsca %dX%d ret %d", width, height, dim.sliceWidth, dim.sliceHeight, ret);

    ColorSpace_t colorSpace = ITU_R_601_FR;
    ret = setMetaData(mapInfo.privateHandle, UPDATE_COLOR_SPACE, &colorSpace);
    if (ret < 0) {
      IDBG_ERROR("setMetaData(UPDATE_COLOR_SPACE) failed %d", ret);
      goto eglcreateimage_failed;
    }

     mapInfo.pNativeBuffer = new NativeBuffer(width, height, stride, update_format,
       mUsage, mapInfo.privateHandle);
    if (mapInfo.pNativeBuffer == NULL)
    {
      IDBG_ERROR("GraphicBuffer Creation Failed \n");
      goto eglcreateimage_failed;
    }

    EGLClientBuffer pNativeBuffer = (EGLClientBuffer)static_cast<ANativeWindowBuffer*>
         (mapInfo.pNativeBuffer.get());
    mapInfo.hEGLImage = p_eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, pNativeBuffer, vEGLAttribs);
    if (!mapInfo.hEGLImage)
    {
      IDBG_ERROR("EGLImage creation failed");
      goto eglcreateimage_failed;
    }

    glGenFramebuffers(1, &mapInfo.hFBO);
    error = glGetError();
    if(error != GL_NO_ERROR) {
      IDBG_ERROR("after glGenFramebuffers eglError (0x%x)", error);
      goto glgeterror_failed;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, mapInfo.hFBO);
    checkglError("glBindFramebuffer");
    glGenRenderbuffers(1, &mapInfo.hRBO);
    checkglError("glGenRenderbuffers");
    glBindRenderbuffer(GL_RENDERBUFFER, mapInfo.hRBO);
    checkglError("glBindRenderbuffer");
    p_glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, (GLeglImageOES)mapInfo.hEGLImage);
    checkglError("p_glEGLImageTargetRenderbufferStorageOES");
    checkEglError("p_glEGLImageTargetRenderbufferStorageOES");
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mapInfo.hRBO);
    checkglError("glFramebufferRenderbuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkglError("glBindFramebuffer");
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    checkglError("glBindRenderbuffer");

    mapInfo.isInputBuffer = FALSE;
    mapInfo.hColorTexture = 0;
    //Insert into the map
    IDBG_LOW("Inserting index %d fbo %d!", index, mapInfo.hFBO);
    mFrameBufferMap[index] = mapInfo;

    return mapInfo.hFBO;
  }

  IDBG_LOW("X");
glgeterror_failed:
  if(mapInfo.hColorTexture) {
    glDeleteTextures(1, &mapInfo.hColorTexture);
  }
  if(mapInfo.hEGLImage) {
    p_eglDestroyImageKHR(eglGetCurrentDisplay(), mapInfo.hEGLImage);
    checkEglError("Destroy image");
  }

eglcreateimage_failed:
  delete mapInfo.privateHandle;

private_handle_failed:
  img_buffer_release(&mapInfo.meta_buffer);
  return (GLuint)0;
}


