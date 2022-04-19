/*************************************************************
* Copyright (c) 2016 Qualcomm Technologies, Inc.             *
* All Rights Reserved.                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc. *
*************************************************************/
#include <sys/mman.h>
#include <stddef.h>
#include <linux/msm_ion.h>
#include <linux/ion.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include "cam_types.h"
#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "img_queue.h"
#include "img_dbg.h"
#include "img_buffer.h"

/* Max down scale*/
#define JPEGDMA_DRV_MAX_DOWNSCALE (16.0)
/* Max color formats*/
#define JPEGDMA_MAX_COLOR_FMTS 4

#define JPEGDMA_START_MEASURE(start_time, mid_time) \
  gettimeofday(&start_time, NULL); \
  mid_time = start_time; \

#define JPEGDMA_MIDDLE_TIME(str, mid_time, end_time) \
do { \
  gettimeofday(&end_time, NULL); \
  fprintf(stderr, "Middle mtime, %s %lu ms \n", str, \
  ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
  ((mid_time.tv_sec * 1000) + (mid_time.tv_usec / 1000))); \
  mid_time = end_time; \
} while (0)\

#define JPEGDMA_END_MEASURE(str, start_time, end_time) \
do { \
  gettimeofday(&end_time, NULL); \
  fprintf(stderr, "End of measure Total, %s %lu ms \n", str, \
  ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
  ((start_time.tv_sec * 1000) + (start_time.tv_usec / 1000))); \
} while (0) \

/** jdma_test_format_t
 * @format: image format
 * @chroma_wt: chroma factor
 * @planeCnt: plane count
 * @h_divider: horizontal plane divider
 * @v_divider: vertical plane divider
**/
typedef struct {
  cam_format_t format;
  float chroma_wt;
  int planeCnt;
  int h_divider;
  int v_divider;
} jdma_test_format_t;

/** col_formats:
 *
 * Color format mapping from testapp to OMX
 **/
static const jdma_test_format_t col_formats[JPEGDMA_MAX_COLOR_FMTS] =
{
  { CAM_FORMAT_Y_ONLY, 1.0, 1, 1, 1},
  { CAM_FORMAT_YUV_420_NV12, 1.5, 2, 1, 2},
  { CAM_FORMAT_YUV_420_NV21, 1.5, 2, 1, 2},
  { CAM_FORMAT_YUV_420_YV12, 1.5, 2, 1, 4}
};

/** jdma_imglib_t
 * @ptr: handle to imglib library
 * @img_core_get_comp: function pointer for img_core_get_comp
**/
typedef struct {
  void *ptr;
  int (*img_core_get_comp) (img_comp_role_t role, char *name,
    img_core_ops_t *p_ops);
} jdma_imglib_t;

/** jdma_job_t
 * @ion_buff_out: input ion buffer
 * @ion_buff_cap: output ion buffer
 * @input_frame: input frame
 * @output_frame: output frame
 * @meta_frame: Meta frame
**/
typedef struct {
  img_mem_handle_t ion_buff_out;
  img_mem_handle_t ion_buff_cap;
  img_frame_t input_frame;
  img_frame_t output_frame;
  img_meta_t meta_frame;
} jdma_job_t;

/** jdma_test_clnt_input
 * @p_in_filename: pointer to input file
 * @p_out_filename: pointer to output file
 * @src_dim: source dimensions
 * @dest_dim: destination dimensions
 * @rect_crop: crop dimensions
 * @framerate: framerate used to calc HW speed
 * @fmt: image format
 * @burst_cnt: burst count
 * @threadid: Client thread id
 * @client_id: client id
**/
typedef struct {
  char *p_in_filename;
  char *p_out_filename;
  img_dim_t src_dim;
  img_dim_t dest_dim;
  img_rect_t rect_crop;
  uint32_t framerate;
  jdma_test_format_t fmt;
  uint32_t burst_cnt;
  pthread_t threadid;
  int32_t client_id;
} jdma_test_clnt_input;

/** jdma_test_input
 * @clnt_cnt: number of clients
 * @p_clnt_input: pointer to client input
**/
typedef struct {
  uint32_t clnt_cnt;
  jdma_test_clnt_input *p_clnt_input;
} jdma_test_input;

/** jdma_test_job_status
 * @jobDoneId: job done id
 * @cbEvent: job call back event
**/
typedef struct {
  int32_t jobDoneId;
  img_event_type cbEvent;
} jdma_test_job_status;

/** jdma_test_clnt_obj_t
 * @core_ops: imglib core ops
 * @comp: imglib component
 * @img_lib: imglib obj
 * @p_clnt_input: pointer to client input
 * @p_job_arr: pointer to job array
 * @threadid: call back handler thread id
 * @thread_exit: call back handler thread exit flag
 * @QJobDone: Job done queue
 * @jobs_start: Jobs start time
 * @jobs_end: Jobs end time
**/
typedef struct {
  img_core_ops_t core_ops;
  img_component_ops_t comp;
  jdma_imglib_t img_lib;
  jdma_test_clnt_input *p_clnt_input;
  jdma_job_t *p_job_arr;
  pthread_t threadid;
  int32_t thread_exit;
  img_queue_t QJobDone;
  struct timeval jobs_start;
  struct timeval jobs_end;
} jdma_test_clnt_obj_t;

/**
 * Function: write_file
 *
 * Description: write to output file
 *
 * Input parameters:
 *   p_addr - pointer to buffer address
 *   size - size of buffer
 *   p_filename - output file name
 *   cnt - output frame count
 *
 * Return values:
 *   0 on success, -1 on failure
 *
 * Notes: none
 **/
int write_file(void *p_addr, int size, char *p_filename, int cnt)
{
  int written_size;
  char name[256];
  int fd;

  if (!p_addr || !size || !p_filename) {
    fprintf(stderr, "%s:%d write data to file\n", __func__, __LINE__);
  }

  memset(name, 0x00, sizeof(name));

  snprintf(name, sizeof(name), "%s_%d.yuv", p_filename, cnt);
  fprintf(stderr, "%s:%d write data to file %s\n", __func__, __LINE__, name);
  fd = open(name, O_RDWR | O_CREAT, 0777);
  if (fd < 0) {
    fprintf(stderr, "%s:%d failed: Cannot write data to file %s\n", __func__,
      __LINE__, p_filename);
    return -1;
  }

  written_size = write(fd, p_addr, size);
  if (size != written_size) {
    fprintf(stderr, "%s:%d failed: Cannot write data to file %s\n", __func__,
      __LINE__, p_filename);
  }

  close(fd);

  return 0;
}

/**
 * Function: read_file
 *
 * Description: Read input file
 *
 * Input parameters:
 *   p_addr - pointer to buffer address
 *   p_buff - pointer to ion buffer struct
 *   p_filename - input file name
 *
 * Return values:
 *   0 on success, -1 on failure
 *
 * Notes: none
 **/
int read_file(void *p_addr, img_mem_handle_t *p_buff, char *p_filename)
{
  int rc = 0;
  int fd;

  fd = open(p_filename, O_RDWR | O_SYNC);
  if (fd < 0) {
      fprintf(stderr, "Can not open the file %s ", strerror(errno));
      return -1;
  }

  rc = read(fd, p_addr, p_buff->length);
  if (rc < 0) {
    fprintf(stderr, "Can not read from the file %s ", strerror(errno));
  }

  if (rc > (int)p_buff->length) {
    fprintf(stderr, "%s:%d] Sizes are not correct %d %d\n",
      __func__, __LINE__, rc, p_buff->length);
    return -1;
  }
  close(fd);

  return 0;
}

/**
 * Function: jdma_test_call_back_func
 *
 * Description: Thread function per client
 *
 * Input parameters:
 *   p_userdata - pointer to user data
 *   p_frame_bundle - pointer to frame bundle
 *   cb_event - call back event
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int jdma_test_call_back_func(void *p_userdata,
  img_frame_bundle_t *p_frame_bundle, img_event_type cb_event)
{
  int32_t client_id;
  jdma_test_clnt_obj_t *p_clnt_obj = (jdma_test_clnt_obj_t *)p_userdata;
  jdma_test_job_status *p_job_status = NULL;

  if (p_clnt_obj == NULL) {
    IDBG_ERROR("userdata NULL\n");
    return 0;
  }

  client_id = p_clnt_obj->p_clnt_input->client_id;

  p_job_status = malloc(sizeof(jdma_test_job_status));
  if (p_job_status == NULL) {
    IDBG_ERROR("client %d Memory alloc failed\n", client_id);
    return 0;
  }

  if ((p_clnt_obj->p_clnt_input->burst_cnt - 1) ==
    p_frame_bundle->p_output[0]->frame_id) {
    JPEGDMA_END_MEASURE("jobs done", p_clnt_obj->jobs_start,
      p_clnt_obj->jobs_end);
  }

  if (cb_event == QIMG_EVT_BUF_DONE) {
    IDBG_HIGH("client %d BUF DONE EVT RECEIVED for frame %d\n", client_id,
      p_frame_bundle->p_output[0]->frame_id);
  } else {
    IDBG_HIGH("client %d ERROR EVENT RECEIVED for frame %d\n", client_id,
      p_frame_bundle->p_output[0]->frame_id);
  }

  p_job_status->jobDoneId = p_frame_bundle->p_output[0]->frame_id;
  p_job_status->cbEvent = cb_event;

  img_q_enqueue(&p_clnt_obj->QJobDone, p_job_status);
  img_q_signal(&p_clnt_obj->QJobDone);

  return 0;
}

/**
 * Function: check_thread_exit
 *
 * Description: Thread exit check
 *
 * Input parameters:
 *   p_obj - pointer to client object
 *
 * Return values:
 *   0 if exit, 1 if not exit
 *
 * Notes: none
 **/
int check_thread_exit(void *p_obj)
{
  jdma_test_clnt_obj_t *p_clnt_obj = (jdma_test_clnt_obj_t *)p_obj;

  if (p_clnt_obj->thread_exit) {
    return 0;
  }

  return 1;
}

/**
 * Function: jdma_test_cb_handler
 *
 * Description: Call back handler
 *
 * Input parameters:
 *   p_obj - pointer to client object
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void* jdma_test_cb_handler(void * p_obj)
{
  jdma_test_clnt_obj_t *p_clnt_obj = (jdma_test_clnt_obj_t *) p_obj;
  jdma_test_clnt_input *p_clnt_input = p_clnt_obj->p_clnt_input;
  int32_t burst_cnt = p_clnt_input->burst_cnt;
  jdma_job_t *p_jdma_job = NULL;
  int32_t cb_event;
  int32_t client_id = p_clnt_obj->p_clnt_input->client_id;
  int32_t job_done_id;

  jdma_test_job_status *p_job_status = NULL;

  while (burst_cnt) {
    img_q_wait_for_signal(&p_clnt_obj->QJobDone, check_thread_exit,
      p_clnt_obj);

    if (!check_thread_exit(p_clnt_obj)) {
      return NULL;
    }

    p_job_status = img_q_dequeue(&p_clnt_obj->QJobDone);
    if (p_job_status == NULL) {
      IDBG_ERROR("client %d Error no memory\n", client_id);
      return NULL;
    }

    job_done_id = p_job_status->jobDoneId;
    p_jdma_job = &p_clnt_obj->p_job_arr[job_done_id];
    cb_event = p_job_status->cbEvent;

    if (cb_event == QIMG_EVT_BUF_DONE) {
      if (p_clnt_input->p_out_filename) {
        write_file(p_jdma_job->ion_buff_cap.vaddr,
          p_jdma_job->ion_buff_cap.length,
          p_clnt_input->p_out_filename, job_done_id);
      }
    } else {
        p_clnt_obj->thread_exit = 1;
    }

    if (p_jdma_job->ion_buff_cap.vaddr){
      img_buffer_release(&p_jdma_job->ion_buff_cap);
    }

    if (p_jdma_job->ion_buff_out.vaddr){
      img_buffer_release(&p_jdma_job->ion_buff_out);
    }

    free(p_job_status);
    burst_cnt--;
  }
  return NULL;
}

/**
 * Function: print_usage
 *
 * Description: Prints test app usage
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void print_usage()
{
  fprintf(stderr, "Usage: program_name [options]\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -u num users (number of users - Mandatory)\n");
  fprintf(stderr, "  -i file (Path to the input file - Mandatory)\n");
  fprintf(stderr, "  -o file (Path to the output file - Mandatory)\n");
  fprintf(stderr, "  -w width (Input image width - Mandatory)\n");
  fprintf(stderr, "  -h height (Input image height - Mandatory)\n");
  fprintf(stderr, "  -s stride (Input image stride - Optional)\n");
  fprintf(stderr, "  -x width (Output image width - Mandatory)\n");
  fprintf(stderr, "  -y height(Output image height - Mandatory)\n");
  fprintf(stderr, "  -z stride (Output image stride - Optional)\n");
  fprintf(stderr, "  -f format (0 = Grey, 1 = NV12, 2 = NV21 - Mandatory)\n");
  fprintf(stderr, "  -a left (Crop left - Optional)\n");
  fprintf(stderr, "  -b top (Crop top - Optional)\n");
  fprintf(stderr, "  -c width (Crop width - Optional)\n");
  fprintf(stderr, "  -d height (Crop height - Optional)\n");
  fprintf(stderr, "  -n repeat_number (Repeat number of times - Optional)\n");
  fprintf(stderr, "  -p fps (Frames per second - Optional)\n");
  fprintf(stderr, "\n");
}

/**
 * Function: jdma_test_clnt_thread_func
 *
 * Description: Thread function per client
 *
 * Input parameters:
 *   p_input - pointer to input structure
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void* jdma_test_clnt_thread_func(void *p_input)
{
  int ion_fd;
  int32_t rc;
  int32_t buf_idx = 0;
  jdma_test_clnt_obj_t *p_clnt_obj  = NULL;
  img_core_ops_t *p_core_ops = NULL;
  img_component_ops_t *p_comp = NULL;
  jdma_job_t *p_curr_job = NULL;
  img_dim_t l_src_dim;
  img_dim_t l_dest_dim;
  pthread_t threadid;
  int32_t burst_cnt;
  float chroma_factor;
  int32_t client_id;
  int out_buf_len;
  int cap_buf_len;
  struct timeval setup_start_time, setup_mid_time, setup_end_time;
  float max_ds_factor;
  uint32_t temp_width = 0;
  uint32_t temp_height = 0;

  jdma_test_clnt_input *p_clnt_input = (jdma_test_clnt_input *) p_input;

  if (p_clnt_input == NULL) {
    IDBG_ERROR("Error NULL\n");
    return NULL;
  }

  client_id = p_clnt_input->client_id;

  ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if(ion_fd < 0) {
    IDBG_ERROR("client %d Error %s\n", client_id, strerror(errno));
    return NULL;
  }

  p_clnt_obj = malloc(sizeof(jdma_test_clnt_obj_t));
  if (p_clnt_obj == NULL) {
    IDBG_ERROR("client %d Error no memory\n", client_id);
    close(ion_fd);
    return NULL;
  }

  JPEGDMA_START_MEASURE(setup_start_time, setup_mid_time);
  // Open libmmcamera_imglib
  p_clnt_obj->img_lib.ptr = dlopen("libmmcamera_imglib.so", RTLD_NOW);
  if (!p_clnt_obj->img_lib.ptr) {
    IDBG_ERROR("client %d Error dlopen failed\n", client_id);
    goto dlopen_failed;
  }

  /* Get function pointer for imglib get component function */
  *(void **)&p_clnt_obj->img_lib.img_core_get_comp =
    dlsym(p_clnt_obj->img_lib.ptr, "img_core_get_comp");

  /* Validate function pointers */
  if (p_clnt_obj->img_lib.img_core_get_comp == NULL) {
    IDBG_ERROR("client %d Error symbol mapping failed\n", client_id);
    goto load_comp_failed;
  }

  JPEGDMA_MIDDLE_TIME("dlopen", setup_mid_time, setup_end_time);
  p_core_ops = &p_clnt_obj->core_ops;
  p_comp = &p_clnt_obj->comp;

  rc = p_clnt_obj->img_lib.img_core_get_comp(IMG_COMP_GEN_2D,
    "qti.jpegdma", p_core_ops);
  if (rc != IMG_SUCCESS) {
    goto load_comp_failed;
  }

  JPEGDMA_MIDDLE_TIME("get comp", setup_mid_time, setup_end_time);
  rc = IMG_COMP_LOAD(p_core_ops, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error load component fail\n", client_id);
    goto load_comp_failed;
  }
  JPEGDMA_MIDDLE_TIME("load comp", setup_mid_time, setup_end_time);
  rc = IMG_COMP_CREATE(p_core_ops, p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error create component fail\n", client_id);
    goto init_failed;
  }

  rc = IMG_COMP_INIT(p_comp, (void *)p_clnt_obj, jdma_test_call_back_func);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error init component fail\n", client_id);
    goto init_failed;
  }

  rc = IMG_COMP_GET_PARAM(p_comp, QIMG_PARAM_MAX_DS, (void *)&max_ds_factor);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error to get max dma downscale factor\n", client_id);
    goto comp_setup_failed;
  }

  if (p_clnt_input->rect_crop.size.width > 0) {
    temp_width = p_clnt_input->rect_crop.size.width;

    if ((temp_width + p_clnt_input->rect_crop.pos.x) >
      p_clnt_input->src_dim.width) {
      fprintf(stderr, "%s\n", "Invalid Crop width dimensions!");
      goto comp_setup_failed;
    }
  } else {
    temp_width = p_clnt_input->src_dim.width;
  }

  if (p_clnt_input->rect_crop.size.height > 0) {
    temp_height = p_clnt_input->rect_crop.size.height;

    if ((temp_height + p_clnt_input->rect_crop.pos.y) >
      p_clnt_input->src_dim.height) {
      fprintf(stderr, "%s\n", "Invalid Crop height dimensions!");
      goto comp_setup_failed;
    }
  } else {
    temp_height = p_clnt_input->src_dim.height;
  }

  if ((temp_width < p_clnt_input->dest_dim.width) ||
    (temp_height < p_clnt_input->dest_dim.height)) {
    fprintf(stderr, "%s\n", "Upscale not supported!");
    goto comp_setup_failed;
  }

  if ((((float)temp_width / (float)p_clnt_input->dest_dim.width) >
    max_ds_factor) ||
    (((float)temp_height / (float)p_clnt_input->dest_dim.height) >
    max_ds_factor)) {
    fprintf(stderr, "%s %f\n", "Error, max downscale ratio is",
      max_ds_factor);
    goto comp_setup_failed;
  }

  JPEGDMA_MIDDLE_TIME("create comp", setup_mid_time, setup_end_time);
  l_src_dim = p_clnt_input->src_dim;
  l_dest_dim = p_clnt_input->dest_dim;

  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_SRC_DIM, (void *)&l_src_dim);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error set src dim param fail\n", client_id);
    goto comp_setup_failed;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_DST_DIM, (void *)&l_dest_dim);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error set dest dim param fail\n", client_id);
    goto comp_setup_failed;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_YUV_FMT,
    (void *)&p_clnt_input->fmt.format);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error set yuv fmt param fail\n", client_id);
    goto comp_setup_failed;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_FRAME_RATE,
    (void *)&p_clnt_input->framerate);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error set framerate param fail\n", client_id);
    goto comp_setup_failed;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_CROP_DIM,
    (void *)&p_clnt_input->rect_crop);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error set crop param fail\n", client_id);
    goto comp_setup_failed;
  }
  JPEGDMA_MIDDLE_TIME("set params", setup_mid_time, setup_end_time);
  rc = IMG_COMP_START(p_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error comp start failed\n", client_id);
    goto comp_setup_failed;
  }
  JPEGDMA_MIDDLE_TIME("start comp", setup_mid_time, setup_end_time);
  JPEGDMA_END_MEASURE("setup time", setup_start_time, setup_end_time);

  p_clnt_obj->p_clnt_input = p_clnt_input;
  burst_cnt = p_clnt_input->burst_cnt;
  p_clnt_obj->p_job_arr = calloc(burst_cnt, sizeof(jdma_job_t));

  img_q_init(&p_clnt_obj->QJobDone, "QDMATestJobDone");

  rc = pthread_create(&p_clnt_obj->threadid, NULL, jdma_test_cb_handler,
    p_clnt_obj);
  if (rc != 0) {
    IDBG_ERROR("client %d Error in cb handler thread creation\n", client_id);
    goto thread_create_failed;
  }

  for (buf_idx = 0; buf_idx < burst_cnt; buf_idx++) {
    p_curr_job = &p_clnt_obj->p_job_arr[buf_idx];
    out_buf_len = (int)((float)(l_src_dim.stride *
      l_src_dim.height) * p_clnt_input->fmt.chroma_wt);
    rc = img_buffer_get(IMG_BUFFER_ION_IOMMU, -1, 0, out_buf_len,
      &p_curr_job->ion_buff_out);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("client %d Error alloc input buff\n", client_id);
      goto buf_send_failed;
    }

    rc = read_file(p_curr_job->ion_buff_out.vaddr,
      &p_curr_job->ion_buff_out, p_clnt_input->p_in_filename);
    if (rc < 0) {
      IDBG_ERROR("client %d Error %s %d\n", client_id,
        strerror(errno), rc);
      goto buf_send_failed;
    }

    cap_buf_len = (int)((float)(l_dest_dim.stride *
      l_dest_dim.height) * p_clnt_input->fmt.chroma_wt);
    rc = img_buffer_get(IMG_BUFFER_ION_IOMMU, -1, 0, cap_buf_len,
      &p_curr_job->ion_buff_cap);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("client %d Error alloc output buff\n", client_id);
      goto buf_send_failed;
    }
  }

  // Start send buffs
  for (buf_idx = 0; buf_idx < burst_cnt; buf_idx++) {
    int j;

    if (p_clnt_obj->thread_exit == 1) {
      goto client_exit;
    }

    if (buf_idx == 0) {
      JPEGDMA_START_MEASURE(p_clnt_obj->jobs_start, p_clnt_obj->jobs_end);
    }

    p_curr_job = &p_clnt_obj->p_job_arr[buf_idx];

    memset(&p_curr_job->input_frame, 0x00, sizeof(img_frame_t));
    p_curr_job->input_frame.idx = buf_idx;
    p_curr_job->input_frame.frame_id = buf_idx;
    p_curr_job->input_frame.frame_cnt = 1;
    p_curr_job->input_frame.info.width = p_clnt_input->src_dim.width;
    p_curr_job->input_frame.info.height = p_clnt_input->src_dim.height;
    p_curr_job->input_frame.info.num_planes = p_clnt_input->fmt.planeCnt;
    p_curr_job->input_frame.frame[0].plane_cnt = p_clnt_input->fmt.planeCnt;

    p_curr_job->input_frame.frame[0].plane[0].fd = p_curr_job->ion_buff_out.fd;
    p_curr_job->input_frame.frame[0].plane[0].length = (l_src_dim.stride *
      l_src_dim.height);
    p_curr_job->input_frame.frame[0].plane[0].stride = l_src_dim.stride;
    p_curr_job->input_frame.frame[0].plane[0].scanline = l_src_dim.height;
    p_curr_job->input_frame.frame[0].plane[0].addr =
      p_curr_job->ion_buff_out.vaddr;
    p_curr_job->input_frame.frame[0].plane[0].offset = 0;

    for (j = 1; j < p_clnt_input->fmt.planeCnt; j++) {
      p_curr_job->input_frame.frame[0].plane[j].fd =
        p_curr_job->ion_buff_out.fd;
      p_curr_job->input_frame.frame[0].plane[j].length = (l_src_dim.stride *
        l_src_dim.height / p_clnt_input->fmt.v_divider);
      p_curr_job->input_frame.frame[0].plane[j].stride = l_src_dim.stride;
      p_curr_job->input_frame.frame[0].plane[j].scanline =
        l_src_dim.height / p_clnt_input->fmt.v_divider;
      p_curr_job->input_frame.frame[0].plane[j].addr =
        (uint8_t *)p_curr_job->ion_buff_out.vaddr
        + p_curr_job->input_frame.frame[0].plane[j - 1].length;
      p_curr_job->input_frame.frame[0].plane[j].offset = 0;
    }

    rc = IMG_COMP_Q_BUF(p_comp, &p_curr_job->input_frame, IMG_IN);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("client %d Error Input QBuf failed\n", client_id);
      goto buf_send_failed;
    }

    memset(&p_curr_job->output_frame, 0x00, sizeof(img_frame_t));
    p_curr_job->output_frame.idx = buf_idx;
    p_curr_job->output_frame.frame_id = buf_idx;
    p_curr_job->output_frame.frame_cnt = 1;
    p_curr_job->output_frame.info.width = p_clnt_input->dest_dim.width;
    p_curr_job->output_frame.info.height = p_clnt_input->dest_dim.height;
    p_curr_job->output_frame.info.num_planes = p_clnt_input->fmt.planeCnt;
    p_curr_job->output_frame.frame[0].plane_cnt = p_clnt_input->fmt.planeCnt;

    p_curr_job->output_frame.frame[0].plane[0].fd = p_curr_job->ion_buff_cap.fd;
    p_curr_job->output_frame.frame[0].plane[0].stride = l_dest_dim.stride;
    p_curr_job->output_frame.frame[0].plane[0].scanline = l_dest_dim.height;
    p_curr_job->output_frame.frame[0].plane[0].length = (l_dest_dim.stride *
      l_dest_dim.height);
    p_curr_job->output_frame.frame[0].plane[0].addr =
      p_curr_job->ion_buff_cap.vaddr;
    p_curr_job->output_frame.frame[0].plane[0].offset = 0;

    for (j = 1; j < p_clnt_input->fmt.planeCnt; j++) {
      p_curr_job->output_frame.frame[0].plane[j].fd =
        p_curr_job->ion_buff_cap.fd;

      p_curr_job->output_frame.frame[0].plane[j].stride = l_dest_dim.stride;
      p_curr_job->output_frame.frame[0].plane[j].scanline =
        l_dest_dim.height / p_clnt_input->fmt.v_divider;
      p_curr_job->output_frame.frame[0].plane[j].length = (l_dest_dim.stride *
        l_dest_dim.height / p_clnt_input->fmt.v_divider);
      p_curr_job->output_frame.frame[0].plane[j].addr =
        (uint8_t *)p_curr_job->ion_buff_cap.vaddr +
        p_curr_job->output_frame.frame[0].plane[j - 1].length;
      p_curr_job->output_frame.frame[0].plane[j].offset = 0;
    }

    rc = IMG_COMP_Q_BUF(p_comp, &p_curr_job->output_frame, IMG_OUT);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("client %d Error Output QBuf failed\n", client_id);
      goto buf_send_failed;
    }

    memset(&p_curr_job->meta_frame, 0x0, sizeof(img_meta_t));
    p_curr_job->meta_frame.frame_id = buf_idx;
    p_curr_job->meta_frame.output_crop = p_clnt_input->rect_crop;

    rc = IMG_COMP_Q_META_BUF(p_comp, &p_curr_job->meta_frame);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("client %d Error meta buff enqueue failed\n", client_id);
      goto buf_send_failed;
    }
  }

  buf_idx--;
  goto client_exit;

buf_send_failed:
  p_clnt_obj->thread_exit = 1;
  img_q_signal(&p_clnt_obj->QJobDone);

client_exit:
  pthread_join(p_clnt_obj->threadid, NULL);

  img_q_flush_and_destroy(&p_clnt_obj->QJobDone);

  do {
    p_curr_job = &p_clnt_obj->p_job_arr[buf_idx];

    if (p_curr_job->ion_buff_cap.vaddr){
      img_buffer_release(&p_curr_job->ion_buff_cap);
    }

    if (p_curr_job->ion_buff_out.vaddr){
      img_buffer_release(&p_curr_job->ion_buff_out);
    }

    buf_idx--;

  } while (buf_idx > 0);

thread_create_failed:

  img_q_deinit(&p_clnt_obj->QJobDone);
  free(p_clnt_obj->p_job_arr);

  rc = IMG_COMP_ABORT(p_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error abort component fail\n", client_id);
  }

comp_setup_failed:

  rc = IMG_COMP_DEINIT(p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error deinit component fail\n", client_id);
  }

init_failed:
  rc = IMG_COMP_UNLOAD(p_core_ops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("client %d Error unload component fail\n", client_id);
  }

load_comp_failed:
  dlclose(p_clnt_obj->img_lib.ptr);

dlopen_failed:
  free(p_clnt_obj);
  close(ion_fd);
  return NULL;
}

/**
 * Function: jdma_test_get_input
 *
 * Description: Function to get input
 *
 * Input parameters:
 *   argc - argument count
 *   argv - argument strings
 *   p_input - pointer to input structure
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int8_t jdma_test_get_input(int argc, char *argv[], jdma_test_input *p_input)
{
  int c;
  int idx = 0;
  uint32_t clnt_idx = 0;
  uint32_t temp_width = 0;
  uint32_t temp_height = 0;
  while ((c = getopt(argc, argv, "u:i:o:w:h:n:x:y:s:z:a:b:c:d:f:p")) != -1) {
    if (p_input->clnt_cnt == 0 && c != 'u') {
      fprintf(stderr, "%s\n", "Please input number of users "
        "as first argument\n");
      print_usage();
      return -1;
    }

    switch (c) {
    case 'u': {
      clnt_idx = 0;
      p_input->clnt_cnt = atoi(optarg);

      p_input->p_clnt_input = calloc(p_input->clnt_cnt,
        sizeof(jdma_test_clnt_input));
      if (!(p_input->p_clnt_input)) {
        return -1;
      }

      for (clnt_idx = 0; clnt_idx < p_input->clnt_cnt; clnt_idx++) {
        p_input->p_clnt_input[clnt_idx].framerate = 30;
        p_input->p_clnt_input[clnt_idx].burst_cnt = 1;
        p_input->p_clnt_input[clnt_idx].client_id = clnt_idx;
      }

      break;
    }
    case 'i': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].p_in_filename = argv[idx];
        fprintf(stderr, "%s%d%s%s\n", "client ", clnt_idx,
          " Input file path ", p_input->p_clnt_input[clnt_idx].p_in_filename);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", "insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'o': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].p_out_filename = argv[idx];
        fprintf(stderr, "%s%d%s%s\n", "client ", clnt_idx,
          " output file path ", p_input->p_clnt_input[clnt_idx].p_out_filename);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'w': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].src_dim.width = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " Input width ", p_input->p_clnt_input[clnt_idx].src_dim.width);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'h': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].src_dim.height = atoi(argv[idx]);
        p_input->p_clnt_input[clnt_idx].src_dim.scanline = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " Input height ", p_input->p_clnt_input[clnt_idx].src_dim.height);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 's': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].src_dim.stride = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " Input stride ", p_input->p_clnt_input[clnt_idx].src_dim.stride);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'x': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].dest_dim.width = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " output width ", p_input->p_clnt_input[clnt_idx].dest_dim.width);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'y': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].dest_dim.height = atoi(argv[idx]);
        p_input->p_clnt_input[clnt_idx].dest_dim.scanline = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " output height ", p_input->p_clnt_input[clnt_idx].dest_dim.height);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'z': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].dest_dim.stride = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " output stride ", p_input->p_clnt_input[clnt_idx].dest_dim.stride);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'n': {
      uint32_t clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].burst_cnt = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " burst count ", p_input->p_clnt_input[clnt_idx].burst_cnt);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'a': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].rect_crop.pos.x = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " crop left ", p_input->p_clnt_input[clnt_idx].rect_crop.pos.x);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'b': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].rect_crop.pos.y = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx,
          " crop top ", p_input->p_clnt_input[clnt_idx].rect_crop.pos.y);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'c': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].rect_crop.size.width =
          atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx, " crop width ",
          p_input->p_clnt_input[clnt_idx].rect_crop.size.width);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'd': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].rect_crop.size.height =
          atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx, " crop height ",
          p_input->p_clnt_input[clnt_idx].rect_crop.size.height);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'p': {
      clnt_idx = 0;

      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        p_input->p_clnt_input[clnt_idx].framerate = atoi(argv[idx]);
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx, " framerate ",
          p_input->p_clnt_input[clnt_idx].framerate);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    case 'f': {
      uint32_t val;
      clnt_idx = 0;
      for (idx = optind - 1; idx < argc; idx++) {
        if (argv[idx][0] == '-') {
          idx++;
          break;
        }
        val = atoi(argv[idx]);
        p_input->p_clnt_input[clnt_idx].fmt = col_formats[val];
        fprintf(stderr, "%s%d%s%d\n", "client ", clnt_idx, " Pixel format ",
          p_input->p_clnt_input[clnt_idx].fmt.format);
        clnt_idx++;
      }

      if (clnt_idx < p_input->clnt_cnt) {
        fprintf(stderr, "%s\n", " insufficient input for all clients\n");
        goto error;
      }
      optind = idx - 1;
      break;
    }
    default:
      print_usage();
      goto error;
    }
  }

  for (clnt_idx = 0; clnt_idx < p_input->clnt_cnt; clnt_idx++) {
    if (!p_input->p_clnt_input[clnt_idx].p_in_filename ||
      !p_input->p_clnt_input[clnt_idx].p_out_filename) {
      fprintf(stderr, "%s\n", " file names null ");
      print_usage();
      goto error;
    }

    if ((p_input->p_clnt_input[clnt_idx].src_dim.width == 0) ||
      (p_input->p_clnt_input[clnt_idx].src_dim.height == 0)) {
      fprintf(stderr, "%s\n", " src Width/hight is zero ");
      print_usage();
      goto error;
    }

    if ((p_input->p_clnt_input[clnt_idx].dest_dim.width == 0) ||
      (p_input->p_clnt_input[clnt_idx].dest_dim.height == 0)) {
      fprintf(stderr, "%s\n", " dest Width/hight is zero ");
      print_usage();
      goto error;
    }

    if (p_input->p_clnt_input[clnt_idx].src_dim.stride == 0) {
      p_input->p_clnt_input[clnt_idx].src_dim.stride =
        p_input->p_clnt_input[clnt_idx].src_dim.width;
    }

    if (p_input->p_clnt_input[clnt_idx].dest_dim.stride == 0) {
      p_input->p_clnt_input[clnt_idx].dest_dim.stride =
        p_input->p_clnt_input[clnt_idx].dest_dim.width;
    }

    if ((p_input->p_clnt_input[clnt_idx].src_dim.stride % 8) ||
      (p_input->p_clnt_input[clnt_idx].dest_dim.stride % 8)) {
      fprintf(stderr, "%s\n", "Stride not padded to 8");
      goto error;
    }

    if ((p_input->p_clnt_input[clnt_idx].src_dim.width & 1) ||
      (p_input->p_clnt_input[clnt_idx].src_dim.height & 1) ||
      (p_input->p_clnt_input[clnt_idx].dest_dim.width & 1) ||
      (p_input->p_clnt_input[clnt_idx].dest_dim.height & 1) ||
      (p_input->p_clnt_input[clnt_idx].rect_crop.size.width & 1) ||
      (p_input->p_clnt_input[clnt_idx].rect_crop.size.height & 1)) {
      fprintf(stderr, "%s\n", "Image sizes must be even!");
      goto error;
    }
  }

  if(p_input->clnt_cnt == 0) {
    fprintf(stderr, "%s\n", " clnt cnt zero ");
    print_usage();
  }

  return 0;

error:
  if(p_input->p_clnt_input)
    free(p_input->p_clnt_input);
  return -1;
}

/**
 * Function: main
 *
 * Description: main jpeg dma test app routine
 *
 * Input parameters:
 *   argc - argument count
 *   argv - argument strings
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int main(int argc, char* argv[])
{
  int8_t ret = 0;
  uint32_t i = 0;
  uint32_t thread_cnt = 0;
  jdma_test_input test_input;

  fprintf(stderr, "=======================================================\n");
  fprintf(stderr, " Qualcomm Technologies Inc MSM JPEGDMA test \n");
  fprintf(stderr, "=======================================================\n");

  memset(&test_input, 0x0, sizeof(jdma_test_input));

  ret = jdma_test_get_input(argc, argv, &test_input);
  if (ret < 0) {
    fprintf(stderr, "Error invalid input\n");
    return -1;
  }

  for (i = 0; i < test_input.clnt_cnt; i++) {

    ret = pthread_create(&test_input.p_clnt_input[i].threadid, NULL,
      jdma_test_clnt_thread_func, &test_input.p_clnt_input[i]);
    if (ret != 0) {
       fprintf(stderr, "Error in thread creation\n");
       break;
    }
  }

  thread_cnt = i;
  for (i = 0; i < thread_cnt; i++) {
    pthread_join(test_input.p_clnt_input[i].threadid, NULL);
  }

  if (test_input.p_clnt_input) {
    free(test_input.p_clnt_input);
  }

  return 0;
}


