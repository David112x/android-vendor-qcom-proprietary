/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "vpu_test.h"
#include "vpu_client.h"
#include <media/msm_media_info.h>

#define VPU_TEST_ALIGN_4K 4096



uint8_t *do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int *mapFd)
{
  void                  *ret; /* returned virtual address */
  int                    rc = 0;
  struct ion_handle_data handle_data;

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    CDBG_ERROR("ION allocation failed\n");
    goto ION_ALLOC_FAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    CDBG_ERROR("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  *mapFd = ion_info_fd->fd;
  ret = mmap(NULL, alloc->len, PROT_READ | PROT_WRITE, MAP_SHARED, *mapFd, 0);
  if (ret == MAP_FAILED) {
    CDBG_ERROR("ION_MMAP_FAILED: %s (%d)\n", strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  return ret;

ION_MAP_FAILED:
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
ION_ALLOC_FAILED:
  return NULL;
}


int32_t vpu_test_allocate_buffer(v4l2_frame_buf_t *fb,
  int32_t width, int32_t height, int32_t index)
{
  int32_t ion_fd, buf_fd;

  ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if (ion_fd < 0) {
    STD_ERR("ion device open failed");
    return -1;;
  }

  STD_LOG("allocating buf idx %d", index);

  memset(fb, 0x00, sizeof(v4l2_frame_buf_t));

  int32_t y_stride, y_scanline;
  int32_t uv_stride, uv_scanline;
  int32_t len;

  y_stride = VENUS_Y_STRIDE(COLOR_FMT_NV12, width);
  y_scanline = VENUS_Y_SCANLINES(COLOR_FMT_NV12, height);
  uv_stride = VENUS_UV_STRIDE(COLOR_FMT_NV12, width);
  uv_scanline = VENUS_UV_SCANLINES(COLOR_FMT_NV12, height);

  STD_LOG("yst=%d, ysc=%d, ust=%d, usc=%d", y_stride, y_scanline, uv_stride, uv_scanline);
  len = y_stride*y_scanline + uv_stride*uv_scanline;
  fb->ion_alloc[0].len = len;
  fb->ion_alloc[0].heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
  fb->ion_alloc[0].align = VPU_TEST_ALIGN_4K;
  fb->addr[0] = (unsigned long) do_mmap_ion(ion_fd,
    &(fb->ion_alloc[0]), &(fb->fd_data[0]), &buf_fd);
  if (!fb->addr[0]) {
    STD_ERR("ion mmap failed");
    close(ion_fd);
    return -1;
  }
  /* populate v4l2_buffer struct */
  fb->buffer.index = index;
  fb->buffer.field = V4L2_FIELD_NONE;
  fb->buffer.memory = V4L2_MEMORY_USERPTR;
  fb->buffer.length = 2; /* 2 planar */

  fb->buffer.m.planes = (struct v4l2_plane*)
    CAM_MALLOC(fb->buffer.length * sizeof(struct v4l2_plane));
  if(!fb->buffer.m.planes) {
    STD_ERR("CAM_MALLOC() failed");
    close(ion_fd);
    return -1;
  }

  /* data for plane 0 : Y */
  fb->buffer.m.planes[0].bytesused = y_stride*y_scanline;
  fb->buffer.m.planes[0].length = y_stride*y_scanline;
  fb->buffer.m.planes[0].m.fd = buf_fd;
  /* plane offset in the buffer */
  fb->buffer.m.planes[0].reserved[0] = 0;

  /* data for plane 1 : CRCB */
  fb->buffer.m.planes[1].bytesused = uv_stride*uv_scanline;
  fb->buffer.m.planes[1].length = uv_stride*uv_scanline;
  fb->buffer.m.planes[1].m.fd = buf_fd;
  /* plane offset in the buffer */
  fb->buffer.m.planes[1].reserved[0] = width*height;

  close(ion_fd);
  return 0;
}

int32_t vpu_test_create_buffers(test_case_t *tc)
{
  int32_t i;
  for (i=0; i < tc->buf_count; i++) {
    vpu_test_allocate_buffer(&(tc->buf[i]), tc->width, tc->height, i);
  }
  return 0;
}

int32_t vpu_test_read_file_to_buffer(test_case_t *tc,
  char *filename, int32_t idx)
{
  int32_t fd;
  uint32_t read_len;
  /* open input image file */
  fd = open(filename, O_RDONLY);
  if (fd < 0) {
    STD_ERR("input file open failed: %s", filename);
    return -1;
  }

  /* Read from input file */
  read_len = read(fd, (void *)tc->buf[idx].addr[0],
    tc->buf[idx].ion_alloc[0].len);
  if (read_len != tc->buf[idx].ion_alloc[0].len) {
    STD_ERR("read input image failed read_len=%d, buf_len=%d",
      read_len, tc->buf[idx].ion_alloc[0].len);
    close(fd);
    return -1;
  }
  close(fd);
  return 0;
}

int32_t vpu_test_init(char *filename, test_case_t *tc)
{
  FILE *fp;
  char type[256], value[256];
  char line_buf[BUFSIZ];

  STD_LOG("test-file : %s", filename);
  fp = fopen(filename, "r");
  if (fp == NULL) {
    STD_ERR("cannot open file: %s", filename);
    return -1;
  }

  memset(tc, 0x00, sizeof(test_case_t));

  while(fgets(line_buf, sizeof(line_buf), fp)) {
    if (sscanf(line_buf, "%s %s", type, value) != 2) {
      STD_ERR("invalid line in test file");
      return -1;
    }
    VPU_LOW("type=%s, val=%s", type, value);
    if (!strncmp(type, "input_path", 256)) {
      strlcpy(tc->input_path, value, 256);
    } else if (!strncmp(type, "width", 256)) {
      tc->width = atoi(value);
    } else if (!strncmp(type, "height", 256)) {
      tc->height = atoi(value);
    }
  }

  STD_LOG("input_path = %s", tc->input_path);
  STD_LOG("width = %d", tc->width);
  STD_LOG("height = %d", tc->height);

  char inputfile[256];
  snprintf(inputfile, 256, "%s/f0-%dx%d.yuv",
    tc->input_path, tc->width, tc->height);

  tc->buf_count = 2;
  vpu_test_create_buffers(tc);
  vpu_test_read_file_to_buffer(tc, inputfile, 0);

  STD_LOG("test init done.");

  return 0;
}

int32_t vpu_test_write_output_to_file(test_case_t *tc, char *filename,
  int32_t idx)
{
  int32_t fd;
  /* open output image file */
  fd = open(filename, O_CREAT|O_RDWR, 0777);
  if (fd < 0) {
    STD_ERR("output file open failed: %s", filename);
    return -1;
  }
  /* write buffer to output file */
  write(fd, (const void *)tc->buf[idx].addr[0],
    tc->buf[idx].ion_alloc[0].len);
  close(fd);
  return 0;
}

void vpu_client_cb(vpu_client_event_t event, void *arg, void *userdata __unused)
{
  struct v4l2_buffer *buffer = (struct v4l2_buffer *) arg;
  STD_LOG("client event received %d, buf_idx=%d", event, buffer->index);
}

int32_t vpu_test_run(test_case_t *tc)
{
  vpu_client_t vc = vpu_client_create();
  int i;
  struct v4l2_format format;

  format.fmt.pix_mp.width = tc->width;
  format.fmt.pix_mp.height = tc->height;
  format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
  format.fmt.pix_mp.num_planes = 2;

  for (i=0; i < 2; i++) {
    format.fmt.pix_mp.plane_fmt[i].bytesperline = tc->width;
  }

  vpu_client_init_session(vc, vpu_client_cb, NULL);
  vpu_client_set_format(vc, format);
  vpu_client_init_streaming(vc, 2);
  vpu_client_stream_on(vc);

  vpu_client_sched_frame_for_processing(vc, &(tc->buf[0].buffer), &(tc->buf[1].buffer));
  vpu_client_stream_off(vc);
  vpu_client_destroy(vc);

  char outfilepath[256];
  snprintf(outfilepath, 256, "%s/out-f0-%dx%d.yuv",
    tc->input_path, tc->width, tc->height);
  vpu_test_write_output_to_file(tc, outfilepath, 1);
  return 0;
}

int main(int argc, char *argv[])
{
  test_case_t tc;
  STD_LOG("-- VPU/TNR test app --");

  if(argc < 2) {
    STD_LOG("usage: %s <test case file>", argv[0]);
    return 0;
  }
  vpu_test_init(argv[1], &tc);

  vpu_test_run(&tc);
  STD_LOG("FINISHED");
  return 0;
}
