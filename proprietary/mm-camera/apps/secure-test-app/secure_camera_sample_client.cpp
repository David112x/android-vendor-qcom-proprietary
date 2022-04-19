/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
Secure Camera Sample/Test Client app.
*********************************************************************/

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <binder/ProcessState.h>
#include <linux/msm_ion.h>
#include <utils/Log.h>
#include <ui/DisplayInfo.h>
#include <system/camera.h>
#include <camera/ICamera.h>
#include <media/mediarecorder.h>
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <cutils/memory.h>
#include <media/ICrypto.h>
#include "QSEEComAPI.h"
#include "common_log.h"

struct QSEECom_handle *l_QSEEComHandle = NULL;
int ion_fd = 0;

#define TEST_CAMERA
#define TEST_DATAFLOW


/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SECURE_CAMERA_SAMPLE_CLIENT: "
#ifdef LOG_NDDEBUG
#undef LOG_NDDEBUG
#endif
#define LOG_NDDEBUG 0 //Define to enable LOGD
#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG  0 //Define to enable LOGV

#if(1)
#define LOGD_PRINT(...) do { LOGD(__VA_ARGS__); printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE_PRINT(...) do { LOGE(__VA_ARGS__); printf(__VA_ARGS__); printf("\n"); } while(0)
#endif
#if(0)
#define LOGD_PRINT(...) do { } while(0)
#define LOGE_PRINT(...) do { } while(0)
#define LOGD(...) do { } while(0)
#define LOGE(...) do { } while(0)
#endif

// Messages IDs to use with the secure app
#define QSEE_TEST_CMD0_START_SEC_CAMERA     0
#define QSEE_TEST_CMD1_STOP_SEC_CAMERA      1
#define QSEE_TEST_CMD2_TAG_MEM              2
#define QSEE_TEST_CMD3_UNTAG_MEM            3
#define QSEE_TEST_CMD4_CAMERA_BUFFER        4
#define QSEE_TEST_CMD5_COPY_BUFFER          5

// Message structs to use with secure app
struct send_cmd {
  uint32_t cmd_id;
  uint32_t buffer;
  uint32_t size;
  uint32_t outBuffer;
};

struct send_cmd_rsp {
  int32_t status;
  uint32_t buffer;
  uint32_t size;
};

int32_t qsc_start_app(struct QSEECom_handle **l_QSEEComHandle,
                      const char *appname, int32_t buf_size)
{
  int32_t ret = 0;

  /* start the application */
  printf("Loading app %s from /system/etc/firmware\n", appname);
  ret = QSEECom_start_app(l_QSEEComHandle, "/system/etc/firmware",
                          appname, buf_size);
  if (ret) {
    LOGE_PRINT("Loading app -%s failed", appname);
  } else {
    LOGD("Loading app -%s succeded", appname);
    QSEECom_set_bandwidth(*l_QSEEComHandle, true);
  }

  return ret;
}

/**@brief:  Implement simple shutdown app
 * @param[in]   handle.
 * @return  zero on success or error count on failure.
 */
int32_t qsc_shutdown_app(struct QSEECom_handle **l_QSEEComHandle)
{
  int32_t ret = 0;

  LOGD("qsc_shutdown_app: start");
  QSEECom_set_bandwidth(*l_QSEEComHandle, false);
  /* shutdown the application */
  if (*l_QSEEComHandle != NULL) {
    ret = QSEECom_shutdown_app(l_QSEEComHandle);
    if (ret) {
      LOGE_PRINT("Shutdown app failed with ret = %d", ret);
    } else {
      LOGD("shutdown app: pass");
    }
  } else {
    LOGE_PRINT("cannot shutdown as the handle is NULL");
  }
  return ret;
}

int32_t issue_send_modified_cmd(struct QSEECom_handle *l_QSEEComHandle,
                                struct send_cmd *send_cmd,
                                struct QSEECom_ion_fd_info *ion_fd_info)
{
  int32_t ret = 0;
  int32_t req_len = 0;
  int32_t rsp_len = 0;
  struct send_cmd_rsp *msgrsp;    /* response data sent from QSEE */

  /* populate the data in shared buffer */

  memcpy(l_QSEEComHandle->ion_sbuffer, send_cmd, sizeof(struct send_cmd));

  req_len = sizeof(struct send_cmd);
  rsp_len = sizeof(struct send_cmd_rsp);

  if (req_len & QSEECOM_ALIGN_MASK) req_len = QSEECOM_ALIGN(req_len);

  if (rsp_len & QSEECOM_ALIGN_MASK) rsp_len = QSEECOM_ALIGN(rsp_len);

  msgrsp = (struct send_cmd_rsp *)(l_QSEEComHandle->ion_sbuffer + req_len);

  /* send request from HLOS to QSEApp */
  ret = QSEECom_send_modified_cmd(l_QSEEComHandle,
                                  l_QSEEComHandle->ion_sbuffer,
                                  req_len,
                                  msgrsp,
                                  rsp_len,
                                  ion_fd_info);
  if (ret) {
    printf("QSEECom_send_modified_cmd %d failed with ret = %d\n", send_cmd->cmd_id, ret);
    return ret;
  }
  return msgrsp->status;
}

int32_t issue_send_cmd(struct QSEECom_handle *l_QSEEComHandle,
                       struct send_cmd *send_cmd)
{
  int32_t ret = 0;
  int32_t req_len = 0;
  int32_t rsp_len = 0;
  struct send_cmd_rsp *msgrsp;    /* response data sent from QSEE */

  /* populate the data in shared buffer */

  memcpy(l_QSEEComHandle->ion_sbuffer, send_cmd, sizeof(struct send_cmd));

  req_len = sizeof(struct send_cmd);
  rsp_len = sizeof(struct send_cmd_rsp);

  if (req_len & QSEECOM_ALIGN_MASK) req_len = QSEECOM_ALIGN(req_len);

  if (rsp_len & QSEECOM_ALIGN_MASK) rsp_len = QSEECOM_ALIGN(rsp_len);

  msgrsp = (struct send_cmd_rsp *)(l_QSEEComHandle->ion_sbuffer + req_len);
  /* send request from HLOS to QSEApp */
  ret = QSEECom_send_cmd(l_QSEEComHandle,
                         l_QSEEComHandle->ion_sbuffer,
                         req_len,
                         msgrsp,
                         rsp_len);
  if (ret) {
    LOGE_PRINT("QSEECom_send_cmd %d failed with ret = %d\n", send_cmd->cmd_id, ret);
    return ret;
  }
  return msgrsp->status;
}


using namespace android;


#ifdef TEST_CAMERA
class SampleCamera : public CameraListener {
public:
  SampleCamera(int camera, size_t frameLen);
  virtual ~SampleCamera();
  void open();
  void closecam();
  void start();
  void stop();

  // CameraListener methods
  virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr,
    camera_frame_metadata_t *metadata);
  virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);
  virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType,
    const sp<IMemory>& dataPtr);

private:
  void dumpFrame(int fd);
  void error(const char *msg);

  int mCameraNum;
  size_t mFrameLen;
  CameraParameters mParams;
  int mFrameCount;

  sp<Camera> mCamera;
  sp<Surface> mSurface;
  sp<SurfaceControl> mSurfaceControl;
  sp<SurfaceComposerClient> mSccClient;
};

SampleCamera::SampleCamera(int camera, size_t frameLen)
{
  mCameraNum = camera;
  mFrameCount = 0;
  mFrameLen = frameLen;
}

SampleCamera::~SampleCamera()
{
  stop();
  closecam();
}


void SampleCamera::open()
{
  printf("%s: Camera #%d\n", __func__, mCameraNum);
  String16 packageName("secure_camera_sample_client");

  mCamera = Camera::connect(mCameraNum, packageName, Camera::USE_CALLING_UID);
  if (mCamera.get() == NULL) {
    error("Cannot connect Camera");
  }

  mParams = mCamera->getParameters();
  mCamera->setListener(this);
  printf("%s: Camera #%d\n", __func__, mCameraNum);
}

void SampleCamera::start()
{
  int width, height;
  // Determine sizes. These don't actually matter for RDI but apparently need
  // to be set anyway
  Vector<Size> sizes;
  sizes.clear();
  mParams.getSupportedPreviewSizes(sizes);
  Size previewSize = sizes.itemAt(0);
  width = previewSize.width;
  height = previewSize.height;
  sizes.clear();
  mParams.getSupportedPictureSizes(sizes);
  Size pictureSize = sizes.itemAt(0);
  sizes.clear();
  mParams.getSupportedVideoSizes(sizes);
  Size videoSize = sizes.itemAt(0);

  // Enable preview callbacks
  mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK);


  // Create preview surface. Again this isn't actually used for RDI but
  // without a surface we won't get preview callbacks.
  int surfWidth = 640, surfHeight = 480;
  mSccClient = new SurfaceComposerClient();
  if (mSccClient.get() == NULL) error("Cannot connect to surface composer");
  mSurfaceControl = mSccClient->createSurface(
    String8("secure_camera_sample_client"), surfWidth, surfHeight,
    HAL_PIXEL_FORMAT_YCrCb_420_SP, 0);
  if (mSurfaceControl.get() == NULL) error("Cannot create surface");
  mSurface = mSurfaceControl->getSurface();
  if (mSurface.get() == NULL) error("Cannot get surface");
  mSccClient->openGlobalTransaction();
  mSurfaceControl->setLayer(0x7fffffff);
  mSurfaceControl->setPosition(0, 0);
  mSurfaceControl->setSize(surfWidth, surfHeight);
  mSurfaceControl->show();
  mSccClient->closeGlobalTransaction();
  sp<IGraphicBufferProducer> gbp;
  gbp = mSurface->getIGraphicBufferProducer();

  // Set camera parameters
  mParams.set("rdi-mode", "enable");
  mParams.set("secure-mode", "enable");
  mParams.setPreviewSize(width, height);
  mParams.setPictureSize(pictureSize.width, pictureSize.height);
  mParams.setVideoSize(videoSize.width, videoSize.height);

  if (mCamera->setParameters(mParams.flatten()) != NO_ERROR) error("setParameters");
  if (mCamera->setPreviewTarget(gbp) != NO_ERROR) error("setPreviewTarget");
  if (mCamera->startPreview() != NO_ERROR) error("startPreview");
}

void SampleCamera::stop()
{
  mCamera->stopPreview();

  // Destroy (the unused) preview surface
  mSurface.clear();
  mSurfaceControl->clear();
  mSurfaceControl.clear();
  mSccClient->dispose();
  mSccClient.clear();

}

void SampleCamera::closecam()
{
  mCamera->disconnect();
  mCamera.clear();
}

void SampleCamera::notify(int32_t msgType, int32_t ext1, int32_t ext2)
{
#ifdef TEST_DATAFLOW
  if ((msgType & CAMERA_MSG_PREVIEW_FRAME) && (ext1 == CAMERA_FRAME_DATA_FD)) {
    int error;
    int bufferFD = dup(ext2);
    if (bufferFD < 0) {
      printf("postData: dup failed %d\n", bufferFD);
    }
    printf("postData: dup succeeded FD=%d\n", bufferFD);

    mFrameCount++;

    if (mFrameCount == 10) {
      dumpFrame(bufferFD);
    } else {
      // Send secure buffer to secure app
      struct send_cmd cmd;

      cmd.cmd_id = QSEE_TEST_CMD4_CAMERA_BUFFER;
      cmd.size = mFrameLen;
      struct QSEECom_ion_fd_info ion_fd_info;
      memset((void *)&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));
      ion_fd_info.data[0].fd = bufferFD;
      ion_fd_info.data[0].cmd_buf_offset = 4;
      error = issue_send_modified_cmd(l_QSEEComHandle, &cmd, &ion_fd_info);
      if (error != 0) {
        printf("QSEE_TEST_CMD4_CAMERA_BUFFER failed: %d\n", error);
      } else {
        printf("   Succeeded QSEE_TEST_CMD4_CAMERA_BUFFER\n");
      }
    }

    ::close(bufferFD);

  } else {
    printf("Unexpected postData type 0x%08X\n", msgType);
  }
#else
  printf("Unhandled postData type 0x%08X, size %u\n", msgType, dataPtr->size());
#endif

  if (msgType & CAMERA_MSG_ERROR) {
    error("CAMERA_MSG_ERROR");
  }
}

void SampleCamera::postDataTimestamp(nsecs_t /*timestamp*/,
  int32_t /*msgType*/, const sp<IMemory>& /*dataPtr*/)
{
  // Shouldn't get here
  printf("%s\n", __func__);
}

void SampleCamera::postData(int32_t msgType, const sp<IMemory>& dataPtr,
  camera_frame_metadata_t * /*metadata*/)
{
  return;
}


void SampleCamera::dumpFrame(int fd)
{
  size_t size = mFrameLen;

  struct ion_allocation_data alloc;
  struct ion_fd_data ion_info_fd;
  int nonsec_fd = 0;
  int error;

  // Allocate non-secure buffer from ION
  memset(&alloc, 0, sizeof(alloc));
  alloc.len = size;
  alloc.align = 4096;
  alloc.heap_id_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
  alloc.flags = 0;
  error = ioctl(ion_fd, ION_IOC_ALLOC, &alloc);
  if (error != 0) {
    printf("%s: Non-secure ION allocation failed: %d\n", __func__, error);
    return;
  }

  // Get handle for buffer
  memset(&ion_info_fd, 0, sizeof(ion_info_fd));
  ion_info_fd.handle = alloc.handle;
  error = ioctl(ion_fd, ION_IOC_SHARE, &ion_info_fd);
  if (error != 0) {
    printf("%s: ION_IOC_SHARE failed: %d\n", __func__, error);
    return;
  }
  nonsec_fd = ion_info_fd.fd;

  // Map non-secure buffer to our address space
  void *nonsec_va = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, nonsec_fd, 0);
  if (nonsec_va == MAP_FAILED) {
    printf("%s: mmap() failed: %d\n", __func__, errno);
    return;
  }

  // Send copy request with both buffers
  struct send_cmd cmd;

  cmd.cmd_id = QSEE_TEST_CMD5_COPY_BUFFER;
  cmd.size = size;
  struct QSEECom_ion_fd_info ion_fd_info;
  memset((void *)&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));
  ion_fd_info.data[0].fd = fd;
  ion_fd_info.data[0].cmd_buf_offset = 4;
  ion_fd_info.data[1].fd = nonsec_fd;
  ion_fd_info.data[1].cmd_buf_offset = 12;
  error = issue_send_modified_cmd(l_QSEEComHandle, &cmd, &ion_fd_info);
  if (error != 0) {
    printf("%s: QSEE_TEST_CMD5_COPY_BUFFER failed: %d\n", __func__, error);
    return;
  } else {
    printf("   Succeeded QSEE_TEST_CMD5_COPY_BUFFER\n");
  }

  printf("%s: Writing image to /sdcard/securecamera.raw, %u bytes\n",
    __func__, (unsigned int) size);
  FILE *f = fopen("/sdcard/securecamera.raw", "wb");
  if (f == NULL) {
    printf("%s: fopen failed: %d\n", __func__, errno);
    return;
  }
  size_t n = fwrite(nonsec_va, size, 1, f);
  if (n != 1) {
    printf("%s: fwrite failed (%u): %d\n", __func__, (unsigned)n, errno);
    return;
  }
  fclose(f);

  // Unmap and free buffer
  munmap(nonsec_va, size);
  struct ion_handle_data handle_data;
  handle_data.handle = alloc.handle;
  error = ioctl(ion_fd, ION_IOC_FREE, &handle_data);
  if (error != 0) {
    printf("%s: ION_IOC_FREE failed: %d\n", __func__, error);
    return;
  }
  ::close(nonsec_fd);
}


void SampleCamera::error(const char *msg)
{
  printf("Camera error: %s", msg);
  exit(EXIT_FAILURE);
}


#endif


int main()
{
  struct send_cmd cmd;
  int32_t ret;

  ProcessState::self()->startThreadPool();

  ion_fd = open("/dev/ion", O_RDONLY);

  ret = qsc_start_app(&l_QSEEComHandle, "secure_camera_sample", 1024);
  if (ret) {
    printf("Start app: fail %d\n", ret);
    return ret;
  } else {
    printf("Start app: pass\n");
  }

#ifdef TEST_CAMERA
  // Open camera
  sp<SampleCamera> camera;
  // Front camera. RDI buffers are 1288x728, 10 bits per pixel
  camera = new SampleCamera(1, ((1288 * 728) * 10) / 8);
  if (camera.get() == NULL) {
    printf("Camera init failed\n");
    exit(EXIT_FAILURE);
  }
#endif

  // Allocate work buffer
  struct ion_allocation_data alloc;
  struct ion_fd_data ion_info_fd;
  int data_fd = 0;
  int error;
  const unsigned int buf_size = 10 * 1024 * 1024;

  // Allocate secure buffer from ION
  memset(&alloc, 0, sizeof(alloc));
  alloc.len = buf_size; // must be 1MB aligned
  alloc.align = 0x100000;
  alloc.flags = ION_SECURE; // | ION_FLAG_CACHED if cached
  alloc.heap_id_mask = ION_HEAP(ION_CP_MM_HEAP_ID);
  error = ioctl(ion_fd, ION_IOC_ALLOC, &alloc);
  if (error != 0) {
    printf("ION allocation failed: %d\n", error);
    exit(EXIT_FAILURE);
  }

  // Get handle for buffer
  memset(&ion_info_fd, 0, sizeof(ion_info_fd));
  ion_info_fd.handle = alloc.handle;
  error = ioctl(ion_fd, ION_IOC_SHARE, &ion_info_fd);
  if (error != 0) {
    printf("ION_IOC_SHARE failed: %d\n", error);
    exit(EXIT_FAILURE);
  }
  data_fd = ion_info_fd.fd;

  printf("Work buffer fd %d\n", data_fd);

  // Send work bufferbuffer to secure app
  cmd.cmd_id = QSEE_TEST_CMD2_TAG_MEM;
  cmd.size = buf_size;
  struct QSEECom_ion_fd_info ion_fd_info;
  memset((void *)&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));
  ion_fd_info.data[0].fd = data_fd;
  ion_fd_info.data[0].cmd_buf_offset = 4;
  error = issue_send_modified_cmd(l_QSEEComHandle, &cmd, &ion_fd_info);
  if (error != 0) {
    printf("QSEE_TEST_CMD2_TAG_MEM failed: %d\n", error);
    exit(EXIT_FAILURE);
  } else {
    printf("   Succeeded QSEE_TEST_CMD2_TAG_MEM\n");
  }

  // Start secure camera mode in secure app
  cmd.cmd_id = QSEE_TEST_CMD0_START_SEC_CAMERA;
  ret = issue_send_cmd(l_QSEEComHandle, &cmd);
  if (ret != 0) {
    printf("   Failed QSEE_TEST_CMD0_START_SEC_CAMERA: %d\n", ret);
    exit(EXIT_FAILURE);
  }
  printf("   Succeeded QSEE_TEST_CMD0_START_SEC_CAMERA (%d)\n", ret);

#ifdef TEST_CAMERA
  printf("Starting secure camera test\n");

  // Start camera data flow
  camera->open();
  camera->start();
#endif

  // Wait for enter
  printf("Press enter\n");

  char dummybuffer[16];
  fgets(dummybuffer, 15, stdin);

#ifdef TEST_CAMERA
  // Stop camera data flow
  camera->stop();
#endif
  // Stop secure camera in app
  cmd.cmd_id = QSEE_TEST_CMD1_STOP_SEC_CAMERA;
  ret = issue_send_cmd(l_QSEEComHandle, &cmd);
  if (ret < 0) {
    printf("   Failed QSEE_TEST_CMD1_STOP_SEC_CAMERA: %d\n", ret);
  } else {
    printf("   Succeeded QSEE_TEST_CMD1_STOP_SEC_CAMERA\n");
  }

  // Stop using work buffer in secure app
  cmd.cmd_id = QSEE_TEST_CMD3_UNTAG_MEM;
  ret = issue_send_cmd(l_QSEEComHandle, &cmd);
  if (ret < 0) {
    printf("   QSEE_TEST_CMD3_UNTAG_MEM: %d\n", ret);
  } else {
    printf("   Succeeded QSEE_TEST_CMD3_UNTAG_MEM\n");
  }

  // Free work buffer
  struct ion_handle_data handle_data;
  handle_data.handle = alloc.handle;
  error = ioctl(ion_fd, ION_IOC_FREE, &handle_data);
  if (error != 0) {
    printf("ION_IOC_FREE failed: %d\n", error);
    exit(EXIT_FAILURE);
  }
  close(data_fd);

#ifdef TEST_CAMERA
  camera->closecam();
#endif

  close(ion_fd);
  ion_fd = 0;

  ret = qsc_shutdown_app(&l_QSEEComHandle);
  if (ret) {
    printf("   Failed to shutdown app: %d\n", ret);
  }
  printf("shutdown: pass\n");

  return 0;
}

