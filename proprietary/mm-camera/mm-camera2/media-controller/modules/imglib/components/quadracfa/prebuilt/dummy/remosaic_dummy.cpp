/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <stdint.h>
extern "C" {
  #include "libremosaiclib.h"
}

#if defined (_ANDROID_)
#include <android/log.h>
#define  QUADDUMMY_LOG_TAG    "quadracfa_dummy"
#define QUADDUMMY_DEBUG_TEST 1
#define QUADDUMMY_LOGE(...) \
  do { if (QUADDUMMY_DEBUG_TEST) __android_log_print(ANDROID_LOG_ERROR, \
  QUADDUMMY_LOG_TAG, __VA_ARGS__); } while (0)
#define QUADDUMMY_LOGI(...) \
  do { if (QUADDUMMY_DEBUG_TEST) __android_log_print(ANDROID_LOG_INFO, \
  QUADDUMMY_LOG_TAG, __VA_ARGS__); } while (0)
#else
#define QUADDUMMY_LOGE(...) \
  do { if (QUADDUMMY_DEBUG_TEST) fprintf(stderr, __VA_ARGS__); } while (0)
#define QUADDUMMY_LOGI(...) \
  do { if (QUADDUMMY_DEBUG_TEST) fprintf(stderr, __VA_ARGS__); } while (0)
#endif

#define VAR_UNUSED(x) (void)(x)

#ifdef USE_API1_REMOSAIC_LIB_DUMMY
void Run4PixelProc(unsigned char* m_pInputBuffer,
  unsigned short* m_pOutputBuffer, int m_iWidth, int m_iHeight,
  int wb_grgain, int wb_rgain, int wb_bgain, int wb_gbgain, int analog_gain,
  int pedestal,int byr_order,
  int* xtalk_gain_map, int mode, int *lib_status,
  int *port0, int *port1, int *port2, int *port3)
{
  VAR_UNUSED(m_pInputBuffer);
  VAR_UNUSED(m_pOutputBuffer);
  VAR_UNUSED(m_iWidth);
  VAR_UNUSED(m_iHeight);
  VAR_UNUSED(wb_grgain);
  VAR_UNUSED(wb_rgain);
  VAR_UNUSED(wb_bgain);
  VAR_UNUSED(wb_gbgain);
  VAR_UNUSED(analog_gain);
  VAR_UNUSED(pedestal);
  VAR_UNUSED(byr_order);
  VAR_UNUSED(xtalk_gain_map);
  VAR_UNUSED(mode);
  VAR_UNUSED(lib_status);
  VAR_UNUSED(port0);
  VAR_UNUSED(port1);
  VAR_UNUSED(port2);
  VAR_UNUSED(port3);
  QUADDUMMY_LOGE("inside dummy Run4PixelProc 1");
  return;
}

void remosaic_init(int32_t img_w, int32_t img_h,
            e_remosaic_bayer_order bayer_order, int32_t pedestal)
{
  VAR_UNUSED(img_w);
  VAR_UNUSED(img_h);
  VAR_UNUSED(bayer_order);
  VAR_UNUSED(pedestal);
  QUADDUMMY_LOGE("inside dummy remosaic_init 1");
  return;
}

int32_t remosaic_gainmap_gen(void* eep_buf_addr, size_t eep_buf_size)
{
  VAR_UNUSED(eep_buf_addr);
  VAR_UNUSED(eep_buf_size);
  QUADDUMMY_LOGE("inside dummy remosaic_gainmap_gen 1");
  return 0;
}

void remosaic_process_param_set(struct st_remosaic_param* p_param)
{
  VAR_UNUSED(p_param);
  QUADDUMMY_LOGE("inside dummy remosaic_process_param_set 1");
  return;
}

int32_t remosaic_process(int32_t src_buf_fd, size_t src_buf_size,
                    int32_t dst_buf_fd, size_t dst_buf_size)
{
  VAR_UNUSED(src_buf_fd);
  VAR_UNUSED(src_buf_size);
  VAR_UNUSED(dst_buf_fd);
  VAR_UNUSED(dst_buf_size);
  QUADDUMMY_LOGE("inside dummy remosaic_process 1");
  return 0;
}

void remosaic_deinit()
{
  QUADDUMMY_LOGE("inside dummy remosaic_deinit 1");
  return ;
}

#endif

#ifdef USE_API2_REMOSAIC_LIB_DUMMY
void Run4PixelProc(unsigned char* m_pInputBuffer,
  unsigned short* m_pOutputBuffer, int m_iWidth, int m_iHeight,
  int wb_grgain, int wb_rgain, int wb_bgain, int wb_gbgain, int analog_gain,
  int* port)
{
  QUADDUMMY_LOGE("inside dummy Run4PixelProc 2");
  return;
}
#endif

