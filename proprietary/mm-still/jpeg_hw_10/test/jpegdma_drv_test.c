/*************************************************************
* Copyright (c) 2015, 2019 Qualcomm Technologies, Inc.       *
* All Rights Reserved.                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc. *
*************************************************************/
#include <sys/mman.h>
#include <stddef.h>
#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif
#include <linux/ion.h>
#include <linux/videodev2.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>

#define JPEGDMA_DRV_NAME "msm_jpegdma"
#define JPEGDMA_V4L2_SYS_PATH "/sys/class/video4linux/"
#define JPEGDMA_DRV_MAX_NAME_SIZE 128
#define JPEGDMA_DRV_MAX_DEVICES 20
#define JPEGDMA_DRV_MAX_DOWNSCALE (16.0)

#define PAGE_SZ 4096

#define JPEGDMA_START_MEASURE \
  struct timeval start_time, mid_time, end_time;\
  gettimeofday(&start_time, NULL); \
  mid_time = start_time \

#define JPEGDMA_MIDDLE_TIME \
do { \
  gettimeofday(&end_time, NULL); \
  fprintf(stderr, "%s]%d Middle mtime  %lu ms \n",  __func__, __LINE__, \
  ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
  ((mid_time.tv_sec * 1000) + (mid_time.tv_usec / 1000))); \
  mid_time = end_time; \
} while (0)\

#define JPEGDMA_END_MEASURE \
do { \
  gettimeofday(&end_time, NULL); \
  fprintf(stderr, "End of measure Total %lu ms \n", \
  ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
  ((start_time.tv_sec * 1000) + (start_time.tv_usec / 1000))); \
} while (0) \

typedef struct  {
  struct ion_fd_data ion_info_fd;
  struct ion_allocation_data alloc;
  int fd;
  long size;
  int ion_fd;
  char *addr;
} img_buffer_t;

int write_file(void *addr, int size, char *filename, int cnt)
{
  int rc = 0, written_size;
  char name[64];
  int fd;

  if (!addr || ! size || !filename) {
    fprintf(stderr, "%s:%d write data to file\n", __func__, __LINE__);
  }

  memset(name, 0x00, sizeof(name));

  snprintf(name, sizeof(name), "%s_%d.yuv", filename, cnt);
  fprintf(stderr, "%s:%d write data to file %s\n", __func__, __LINE__, name);
  fd = open(name, O_RDWR | O_CREAT, 0777);
  if (fd < 0) {
    fprintf(stderr, "%s:%d failed: Cannot write data to file %s\n", __func__,
      __LINE__, filename);
    return -1;
  }

  written_size = write(fd, addr, size);
  if (size != written_size) {
    fprintf(stderr, "%s:%d failed: Cannot write data to file %s\n", __func__,
      __LINE__, filename);
  }

  close(fd);

  return 0;
}

int read_file(void *addr, img_buffer_t *buff, char *filename)
{
  int rc = 0;
  int fd;

  fprintf(stderr, "Opening the file %s %lu\n", filename,  buff->size);

  fd = open(filename, O_RDWR | O_SYNC);
  if (fd < 0) {
      fprintf(stderr, "Can not open the file %s ", strerror(errno));
      return -1;
  }

  rc = read(fd, addr, buff->size);
  if (rc < 0) {
    fprintf(stderr, "Can not read from the file %s ", strerror(errno));
  } else {
    fprintf(stderr, "%s:%d] bytes_read %d \n", __func__, __LINE__, rc);
  }

  if (rc != buff->size) {
    fprintf(stderr, "%s:%d] Sizes are not correct %d %lu\n",
      __func__, __LINE__, rc, buff->size);
    return -1;
  }
  close(fd);

  return 0;
}

void* buffer_allocate(img_buffer_t *p_buffer)
{
  void *l_buffer = NULL;

  int lrc = 0;
  p_buffer->alloc.len = p_buffer->size;
  p_buffer->alloc.flags = 0;
  /* Make it page size aligned */
  p_buffer->alloc.len = (p_buffer->alloc.len + 4095) & (~4095);

#ifndef TARGET_ION_ABI_VERSION
  struct ion_handle_data lhandle_data;

   p_buffer->alloc.align = 4096;
   p_buffer->alloc.heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID);

  lrc = ioctl(p_buffer->ion_fd, ION_IOC_ALLOC, &p_buffer->alloc);
  if (lrc < 0) {
    fprintf(stderr, "%d Error %s  %lu\n", __LINE__, strerror(errno),
      p_buffer->alloc.len);
    goto ion_alloc_failed;
  }

  p_buffer->ion_info_fd.handle = p_buffer->alloc.handle;
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_SHARE, &p_buffer->ion_info_fd);
  if (lrc < 0) {
    fprintf(stderr, "%d Error %s \n", __LINE__, strerror(errno));
    goto ion_map_failed;
  }
  p_buffer->fd = p_buffer->ion_info_fd.fd;
#else
  int p_pmem_fd;
  p_buffer->alloc.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
  lrc = ion_alloc_fd(p_buffer->ion_fd, p_buffer->alloc.len,
    PAGE_SZ, p_buffer->alloc.heap_id_mask,
    p_buffer->alloc.flags, &p_pmem_fd);
  if (lrc < 0) {
    fprintf(stderr, "%d Error %s  %lu\n", __LINE__, strerror(errno),
      p_buffer->alloc.len);
    goto ion_alloc_failed;
  }

  p_buffer->ion_info_fd.fd = p_pmem_fd;

#endif
  l_buffer = mmap(NULL, p_buffer->alloc.len, PROT_READ | PROT_WRITE,
    MAP_SHARED, p_buffer->fd, 0);

  if (l_buffer == MAP_FAILED) {
    fprintf(stderr, "%d Error %s  \n", __LINE__, strerror(errno));
    goto ion_map_failed;
  }

  return l_buffer;

ion_map_failed:
#ifndef TARGET_ION_ABI_VERSION
  lhandle_data.handle = p_buffer->ion_info_fd.handle;
  ioctl(p_buffer->ion_fd, ION_IOC_FREE, &lhandle_data);
#else
  close(p_pmem_fd);
#endif
  return NULL;
ion_alloc_failed:
  return NULL;
}

int buffer_deallocate(img_buffer_t *p_buffer)
{
  int lrc = 0;
  int lsize = (p_buffer->size + 4095) & (~4095);

  struct ion_handle_data lhandle_data;
  lrc = munmap(p_buffer->addr, lsize);

  close(p_buffer->ion_info_fd.fd);

#ifndef TARGET_ION_ABI_VERSION
  lhandle_data.handle = p_buffer->ion_info_fd.handle;
  ioctl(p_buffer->ion_fd, ION_IOC_FREE, &lhandle_data);
#endif
  return lrc;
}

static int open_fd_device(char *p_fd_name, size_t size)
{
  char temp_name[JPEGDMA_DRV_MAX_NAME_SIZE];
  int temp_fd;
  int num_device;
  int cnt = JPEGDMA_DRV_MAX_DEVICES;
  int ret;

  if (size > sizeof(temp_name)) {
    size = sizeof(temp_name);
  }

  num_device = -1;
  while (cnt-- > 0) {
    num_device++;

    ret = snprintf(temp_name, sizeof(temp_name),
      JPEGDMA_V4L2_SYS_PATH"video%d/name", num_device);
    if (ret <= 0) {
      fprintf(stderr,"Snprintf fail %s", strerror(errno));
      return -1;
    }

    temp_fd = open(temp_name, O_RDONLY | O_NONBLOCK);
    if (temp_fd < 0) {
      if (errno == ENOENT) {
        fprintf(stderr, "Can not discover JPEG DMA\n");
        return -1;
      } else {
        fprintf(stderr,"%s %s Skip\n", strerror(errno), temp_name);
        continue;
      }
    }

    ret = read(temp_fd, temp_name, sizeof(JPEGDMA_DRV_NAME));
    close(temp_fd);
    if (ret <= 0) {
      temp_name[0] = 0;
      fprintf(stderr,"Can not read name for the device %s skip\n", strerror(errno));
      continue;
    }

    if ((size_t)ret < size) {
      temp_name[ret - 1] = 0;
    } else {
      temp_name[size - 1] = 0;
    }

    fprintf(stderr,"Check video device %s\n", temp_name);

    ret = strncmp((char *)temp_name, JPEGDMA_DRV_NAME, sizeof(JPEGDMA_DRV_NAME));
    if (!ret) {
      snprintf(temp_name, sizeof(temp_name), "/dev/video%d", num_device);
      fprintf(stderr, "Found Jpeg-dma device %s\n", temp_name);
      strlcpy(p_fd_name, temp_name, size);
      return 0;
    }
  }

  fprintf(stderr, "Exceed max dev number %d FD not found\n",
    JPEGDMA_DRV_MAX_DEVICES);

  return -1;
}

void print_usage()
{
  fprintf(stderr, "Usage: program_name [options]\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -i file (Path to the input file - Mandatory)\n");
  fprintf(stderr, "  -o file (Path to the output file - Mandatory)\n");
  fprintf(stderr, "  -w width (Input image width - Mandatory)\n");
  fprintf(stderr, "  -h height (Input image height - Mandatory)\n");
  fprintf(stderr, "  -s stride (Input image stride - Optional)\n");
  fprintf(stderr, "  -x width (Output image width - Mandatory)\n");
  fprintf(stderr, "  -y height(Output image height - Mandatory)\n");
  fprintf(stderr, "  -z stride (Output image stride - Optional)\n");
  fprintf(stderr, "  -f format (0 = Grey, 1 = NV12, 2 = NV21 - Mandatory)\n");
  fprintf(stderr, "  -a step (Zoom step x - Optional)\n");
  fprintf(stderr, "  -b step (Zoom step y - Optional)\n");
  fprintf(stderr, "  -n repeat_number (Repeat number of times - Optional)\n");
  fprintf(stderr, "  -p fps (Frames per second - Optional)\n");
  fprintf(stderr, "\n");
}

int main(int argc, char* argv[])
{
  int fd_face_detect, ion_fd;
  struct v4l2_capability cap;
  struct v4l2_format fmt_out;
  struct v4l2_format fmt_cap;

  struct v4l2_requestbuffers req_bufs;
  struct v4l2_buffer buff1_out, buff2_out;
  struct v4l2_buffer buff1_cap, buff2_cap;
  struct v4l2_buffer dqbuff1_out, dqbuff2_out;
  struct v4l2_buffer dqbuff1_cap, dqbuff2_cap;
  void *alloc1_out = NULL, *alloc2_out = NULL;
  void *alloc1_cap = NULL, *alloc2_cap = NULL;
  img_buffer_t buffer1_out, buffer2_out;
  img_buffer_t buffer1_cap, buffer2_cap;
  struct v4l2_control control;
  struct v4l2_crop crop;
  char dma_name[JPEGDMA_DRV_MAX_NAME_SIZE];

  enum v4l2_buf_type buf_type;
  int c, ret, i, cnt, val;
  int in_width, in_height, in_stride, out_width, out_height, out_stride;
  int a, zoom_step_x, zoom_step_y, img_cnt, framerate;
  unsigned int pixel_format;
  char *input_image, *out_image;
  struct v4l2_streamparm param;

  fprintf(stderr, "=======================================================\n");
  fprintf(stderr, " Qualcomm MSM JPEGDMA test \n");
  fprintf(stderr, "=======================================================\n");

  /* Set default values */
  input_image = NULL;
  out_image = NULL;

  zoom_step_x = 0;
  zoom_step_y = 0;
  in_width = 0;
  in_height = 0;
  in_stride = 0;
  out_width = 0;
  out_height = 0;
  out_stride = 0;
  framerate = 30;
  img_cnt = 0;
  pixel_format = V4L2_PIX_FMT_GREY;
  cnt = 1;

  while ((c = getopt(argc, argv, "i:o:w:h:n:x:y:s:z:a:b:f:p:c")) != -1) {
    switch (c) {
    case 'i':
      input_image = optarg;
      fprintf(stderr, "Input image path %s\n", input_image);
      break;
    case 'o':
      out_image = optarg;
      fprintf(stderr, "out image path %s\n", out_image);
      break;
    case 'w':
      in_width = atoi(optarg);
      fprintf(stderr, "Input image width %d\n", in_width);
      break;
    case 'h':
      in_height = atoi(optarg);
      fprintf(stderr, "Input image height %d\n", in_height);
      break;
    case 's':
      in_stride = atoi(optarg);
      fprintf(stderr, "Input image stride %d\n", in_stride);
      break;
    case 'x':
      out_width = atoi(optarg);
      fprintf(stderr, "Output image width %d\n", out_width);
      break;
    case 'y':
      out_height = atoi(optarg);
      fprintf(stderr, "Output image height %d\n", out_height);
      break;
    case 'z':
      out_stride = atoi(optarg);
      fprintf(stderr, "Output image stride %d\n", out_stride);
      break;
    case 'n':
      cnt = atoi(optarg);
      fprintf(stderr,"Number of iterations %d\n", cnt);
      break;
    case 'a':
      zoom_step_x = atoi(optarg);
      fprintf(stderr,"Zoom step x %d\n", zoom_step_x);
      break;
    case 'b':
      zoom_step_y = atoi(optarg);
      fprintf(stderr,"Zoom step y %d\n", zoom_step_y);
      break;
    case 'p':
      framerate = atoi(optarg);
      fprintf(stderr,"Framerate %d\n", framerate);
      break;
    case 'f':
      val = atoi(optarg);
      switch (val){
      case 0:
        pixel_format = V4L2_PIX_FMT_GREY;
        fprintf(stderr,"Pixel format monochrome\n");
        break;
      case 1:
        pixel_format = V4L2_PIX_FMT_NV12;
        fprintf(stderr,"Pixel format nv12\n");
        break;
      case 2:
        pixel_format = V4L2_PIX_FMT_NV21;
        fprintf(stderr,"Pixel format nv21\n");
        break;
      default:
        print_usage();
        return -1;
      }
      break;
    default:
      print_usage();
      return -1;
    }
  }

  if (!input_image || (in_width <= 0) || (in_height <= 0)) {
    print_usage();
    return -1;
  }

  if ((out_width <= 0) || (out_height <= 0)) {
    out_width = in_width;
    out_height = in_height;
  }

  if (in_stride < in_width)
    in_stride = in_width;

  if (out_stride < out_width)
    out_stride = out_width;

  if ((in_width & 1) || (in_height & 1) ||
      (out_width & 1) || (out_height & 1)) {
    fprintf(stderr, "%-25s\n", "Image sizes must be even!");
    return 1;
  }

  if ((in_width < out_width) ||
      (in_height < out_height)) {
    fprintf(stderr, "%-25s\n", "Upscale not supported!");
    return 1;
  }

  if (((float)(in_height / out_height)) > JPEGDMA_DRV_MAX_DOWNSCALE) {
    fprintf(stderr, "%-25s %f\n", "Error, max downscale ratio is",
        JPEGDMA_DRV_MAX_DOWNSCALE);
    return 1;
  }

#ifndef TARGET_ION_ABI_VERSION
  ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if(ion_fd < 0) {
    fprintf(stderr, "%d Error %s\n", __LINE__, strerror(errno));
    return -1;
  }
#else
  ion_fd = ion_open();
  if(ion_fd < 0) {
    fprintf(stderr, "%d Error %s\n", __LINE__, strerror(errno));
    return -1;
  }
#endif
  memset(dma_name, 0x00, sizeof(dma_name));
  open_fd_device(dma_name, sizeof(dma_name));

  fd_face_detect = open(dma_name, O_RDONLY | O_SYNC);
  if(fd_face_detect < 0) {
    fprintf(stderr, "%d Error %s %s\n", __LINE__, strerror(errno), dma_name);
    return -1;
  }

  memset(&cap, 0x00, sizeof(cap));
  ret = ioctl(fd_face_detect, VIDIOC_QUERYCAP, &cap);
  if (ret < 0) {
    return 0;
  }
  fprintf(stderr, "ret %d VIDIOC_QUERYCAP drv %s  cap %d\n",
    ret, cap.driver, cap.capabilities);

  memset(&fmt_out, 0x00, sizeof(fmt_out));
  fmt_out.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  fmt_out.fmt.pix.width = in_width;
  fmt_out.fmt.pix.height = in_height;
  fmt_out.fmt.pix.pixelformat = pixel_format;
  fmt_out.fmt.pix.bytesperline = in_stride;
  ret = ioctl(fd_face_detect, VIDIOC_S_FMT, &fmt_out);
  if (ret < 0) {
    fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_S_FMT OUT %dx%d   colorspace %d\n",
    ret, fmt_out.fmt.pix.width, fmt_out.fmt.pix.height,
    fmt_out.fmt.pix.pixelformat);

  buffer1_out.size = fmt_out.fmt.pix.sizeimage;
  buffer1_out.ion_fd = ion_fd;
  fprintf(stderr, "Buffer size %ld\n", buffer1_out.size);
  alloc1_out = buffer_allocate(&buffer1_out);
  if (!alloc1_out) {
    fprintf(stderr, "ERROR allocating buffer \n");
    getchar();
    goto out;
  }

  ret = read_file(alloc1_out, &buffer1_out, input_image);
  if (ret < 0) {
    fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }

  memset(&fmt_cap, 0x00, sizeof(fmt_cap));
  fmt_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt_cap.fmt.pix.width = out_width;
  fmt_cap.fmt.pix.height = out_height;
  fmt_cap.fmt.pix.pixelformat = pixel_format;
  fmt_cap.fmt.pix.bytesperline = out_stride;
  ret = ioctl(fd_face_detect, VIDIOC_S_FMT, &fmt_cap);
  if (ret < 0) {
    fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_S_FMT CAP %dx%d   colorspace %d\n",
    ret, fmt_cap.fmt.pix.width, fmt_cap.fmt.pix.height,
    fmt_cap.fmt.pix.pixelformat);

  buffer1_cap.size = fmt_cap.fmt.pix.sizeimage;
  buffer1_cap.ion_fd = ion_fd;
  fprintf(stderr, "Buffer size %ld\n", buffer1_cap.size);
  alloc1_cap = buffer_allocate(&buffer1_cap);
  if (!alloc1_cap) {
    fprintf(stderr, "ERROR allocating buffer \n");
    getchar();
    goto out;
  }

  memset(&req_bufs, 0x00, sizeof(req_bufs));
  req_bufs.count = 2;
  req_bufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  req_bufs.memory = V4L2_MEMORY_USERPTR;
  ret = ioctl(fd_face_detect, VIDIOC_REQBUFS, &req_bufs);
  if (ret < 0) {
    fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_REQBUFS OUT count %d type %d memory %d \n",
    ret, req_bufs.count, req_bufs.type, req_bufs.memory);

  memset(&req_bufs, 0x00, sizeof(req_bufs));
  req_bufs.count = 2;
  req_bufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req_bufs.memory = V4L2_MEMORY_USERPTR;
  ret = ioctl(fd_face_detect, VIDIOC_REQBUFS, &req_bufs);
  if (ret < 0) {
    fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_REQBUFS CAP count %d type %d memory %d \n",
    ret, req_bufs.count, req_bufs.type, req_bufs.memory);

  memset(&buff1_out, 0x00, sizeof(buff1_out));
  buff1_out.index = 0;
  buff1_out.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  buff1_out.memory = V4L2_MEMORY_USERPTR;
  buff1_out.m.userptr = buffer1_out.fd;
  buff1_out.length = fmt_out.fmt.pix.sizeimage;
  ret = ioctl(fd_face_detect, VIDIOC_QBUF, &buff1_out);
  if (ret < 0) {
    fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
    getchar();
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_QBUF index %u type %d memory %d \n",
    ret, buff1_out.index, buff1_out.type, buff1_out.type);

  buffer2_out.size = fmt_out.fmt.pix.sizeimage;
  buffer2_out.ion_fd = ion_fd;
  alloc2_out = buffer_allocate(&buffer2_out);
  if (!alloc2_out) {
    fprintf(stderr, "ERROR allocating buffer \n");
    getchar();
    goto out;
  }
  ret = read_file(alloc2_out, &buffer2_out, input_image);
  if (ret < 0) {
    goto out;
  }

  memset(&buff2_out, 0x00, sizeof(buff2_out));
  buff2_out.index = 1;
  buff2_out.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  buff2_out.memory = V4L2_MEMORY_USERPTR;
  buff2_out.m.userptr = buffer2_out.fd;
  buff2_out.length = fmt_out.fmt.pix.sizeimage;
  ret = ioctl(fd_face_detect, VIDIOC_QBUF, &buff2_out);
  if (ret < 0) {
    fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
    getchar();
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_QBUF index %d type %d memory %d \n",
    ret, buff2_out.index, buff2_out.type, buff2_out.memory);

  memset(&buff1_cap, 0x00, sizeof(buff1_cap));
  buff1_cap.index = 0;
  buff1_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buff1_cap.memory = V4L2_MEMORY_USERPTR;
  buff1_cap.m.userptr = buffer1_cap.fd;
  buff1_cap.length = fmt_cap.fmt.pix.sizeimage;
  ret = ioctl(fd_face_detect, VIDIOC_QBUF, &buff1_cap);
  if (ret < 0) {
    fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
    getchar();
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_QBUF index %u type %d memory %d \n",
    ret, buff1_cap.index, buff1_cap.type, buff1_cap.type);

  buffer2_cap.size = fmt_cap.fmt.pix.sizeimage;
  buffer2_cap.ion_fd = ion_fd;
  alloc2_cap = buffer_allocate(&buffer2_cap);
  if (!alloc2_cap) {
    fprintf(stderr, "ERROR allocating buffer \n");
    getchar();
    goto out;
  }

  memset(&buff2_cap, 0x00, sizeof(buff2_cap));
  buff2_cap.index = 1;
  buff2_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buff2_cap.memory = V4L2_MEMORY_USERPTR;
  buff2_cap.m.userptr = buffer2_cap.fd;
  buff2_cap.length = fmt_cap.fmt.pix.sizeimage;
  ret = ioctl(fd_face_detect, VIDIOC_QBUF, &buff2_cap);
  if (ret < 0) {
    fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
    getchar();
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_QBUF index %d type %d memory %d \n",
    ret, buff2_cap.index, buff2_cap.type, buff2_cap.memory);

  /* Set parm for dynamic FPS */
  memset(&param, 0x00, sizeof(param));
  param.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  param.parm.output.timeperframe.numerator = 1;
  param.parm.output.timeperframe.denominator = framerate;
  ret = ioctl(fd_face_detect, VIDIOC_S_PARM, &param);
  if (ret < 0) {
    fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "VIDIOC_S_CTRL VIDIOC_S_PARM %d\n",
    param.parm.output.timeperframe.denominator);

  /* Only input crop is supported */
  memset(&crop, 0x00, sizeof(crop));
  crop.c.top = 0;
  crop.c.left = 0;
  crop.c.width = in_width;
  crop.c.height = in_height;

  if (zoom_step_x || zoom_step_y){
    crop.c.top += zoom_step_y / 2;
    crop.c.left += zoom_step_x / 2;
    crop.c.width -= zoom_step_x;
    crop.c.height -= zoom_step_y;
  }
  crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ret = ioctl(fd_face_detect, VIDIOC_S_CROP, &crop);
  if (ret < 0) {
    fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_S_CROP %dx%d %dx%d\n",
    ret, crop.c.top, crop.c.left, crop.c.width, crop.c.height);

  buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ret = ioctl(fd_face_detect, VIDIOC_STREAMON, &buf_type);
  if (ret < 0) {
    fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_STREAMON index %d type \n",
    ret, buf_type);

  buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = ioctl(fd_face_detect, VIDIOC_STREAMON, &buf_type);
  if (ret < 0) {
    fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_STREAMON index %d type \n",
    ret, buf_type);

  JPEGDMA_START_MEASURE;
  while (cnt-- > 0) {

    dqbuff1_out.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = ioctl(fd_face_detect, VIDIOC_DQBUF, &dqbuff1_out);
    if (ret < 0) {
      fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
      goto out;
    }
    JPEGDMA_MIDDLE_TIME;
    fprintf(stderr, "ret %d VIDIOC_DQBUF index %d type %d memory %d \n",
      ret, dqbuff1_out.index, dqbuff1_out.type, dqbuff1_out.memory);

    ret = ioctl(fd_face_detect, VIDIOC_QBUF, &buff1_out);
    buff1_out.sequence++;
    if (ret < 0) {
      fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
      goto out;
    }
    fprintf(stderr, "ret %d VIDIOC_QBUF ion_fd %lu index %u type %d seq %d \n",
      ret, buff1_out.m.userptr, buff1_out.index, buff1_out.type,
      buff1_out.sequence);

    dqbuff1_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd_face_detect, VIDIOC_DQBUF, &dqbuff1_cap);
    if (ret < 0) {
      fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
      goto out;
    }
    if (out_image) {
      write_file(alloc1_cap, fmt_cap.fmt.pix.sizeimage, out_image, img_cnt++);
    }

    fprintf(stderr, "ret %d VIDIOC_DQBUF index %d type %d memory %d \n",
      ret, dqbuff1_cap.index, dqbuff1_cap.type, dqbuff1_cap.memory);

    ret = ioctl(fd_face_detect, VIDIOC_QBUF, &buff1_cap);
    buff1_cap.sequence++;
    if (ret < 0) {
      fprintf(stderr, "%d Error %s %d \n", __LINE__, strerror(errno), ret);
      goto out;
    }
    fprintf(stderr, "ret %d VIDIOC_QBUF ion_fd %lu index %u type %d seq %d \n",
      ret, buff1_cap.m.userptr, buff1_cap.index, buff1_cap.type,
      buff1_cap.sequence);

    dqbuff2_out.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = ioctl(fd_face_detect, VIDIOC_DQBUF, &dqbuff2_out);
    if (ret < 0) {
      fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
      goto out;
    }
    JPEGDMA_MIDDLE_TIME;

    fprintf(stderr, "ret %d VIDIOC_DQBUF index %d type %d memory %d \n",
      ret, dqbuff2_out.index, dqbuff2_out.type, dqbuff2_out.memory);

    buff2_out.sequence++;
    ret = ioctl(fd_face_detect, VIDIOC_QBUF, &buff2_out);
    if (ret < 0) {
      fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
      goto out;
    }
    fprintf(stderr, "ret %d VIDIOC_QBUF index %lu type %d memory %d \n",
      ret, buff2_out.m.userptr, buff2_out.index, buff2_out.type);


    dqbuff2_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd_face_detect, VIDIOC_DQBUF, &dqbuff2_cap);
    if (ret < 0) {
      fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
      goto out;
    }

    if (out_image) {
      write_file(alloc2_cap, fmt_cap.fmt.pix.sizeimage, out_image, img_cnt++);
    }

    fprintf(stderr, "ret %d VIDIOC_DQBUF index %d type %d memory %d \n",
      ret, dqbuff2_cap.index, dqbuff2_cap.type, dqbuff2_cap.memory);

    buff2_cap.sequence++;
    ret = ioctl(fd_face_detect, VIDIOC_QBUF, &buff2_cap);
    if (ret < 0) {
      fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
      goto out;
    }
    fprintf(stderr, "ret %d VIDIOC_QBUF index %lu type %d memory %d \n",
      ret, buff2_cap.m.userptr, buff2_cap.index, buff2_cap.type);
  }
  JPEGDMA_END_MEASURE;

  buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ret = ioctl(fd_face_detect, VIDIOC_STREAMOFF, &buf_type);
  if (ret < 0) {
    fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_STREAMOFF type %d \n",
    ret, buf_type);

  buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = ioctl(fd_face_detect, VIDIOC_STREAMOFF, &buf_type);
  if (ret < 0) {
    fprintf(stderr, "Error %d %s %d \n", __LINE__, strerror(errno), ret);
    goto out;
  }
  fprintf(stderr, "ret %d VIDIOC_STREAMOFF type %d \n",
    ret, buf_type);

out:
  if (alloc1_cap)
    buffer_deallocate(&buffer1_cap);

  if (alloc2_cap)
    buffer_deallocate(&buffer2_cap);

  if (alloc1_out)
    buffer_deallocate(&buffer1_out);

  if (alloc2_out)
    buffer_deallocate(&buffer2_out);

  close(fd_face_detect);
#ifndef TARGET_ION_ABI_VERSION
  close(ion_fd);
#else
  ion_close(ion_fd);
#endif
  return 0;
}

