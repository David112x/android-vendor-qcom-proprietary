/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

void Run4PixelProc(unsigned char* m_pInputBuffer,
  unsigned short* m_pOutputBuffer, int m_iWidth, int m_iHeight,
  int wb_grgain, int wb_rgain, int wb_bgain, int wb_gbgain, int analog_gain,
  int pedestal, int byr_order,
  int* xtalk_gain_map, int mode, int *lib_status,
  int *port0, int *port1, int *port2, int *port3);

enum e_remosaic_bayer_order{
    BAYER_GRBG = 0,
    BAYER_RGGB = 1,
    BAYER_BGGR = 2,
    BAYER_GBRG = 3,
};

struct st_remosaic_param {
    int16_t wb_r_gain;
    int16_t wb_gr_gain;
    int16_t wb_gb_gain;
    int16_t wb_b_gain;
    int16_t analog_gain;
};

enum {
    RET_OK = 0,
    RET_NG = -1,
};

void remosaic_init(int32_t img_w, int32_t img_h,
            e_remosaic_bayer_order bayer_order, int32_t pedestal);
int32_t remosaic_gainmap_gen(void* eep_buf_addr, size_t eep_buf_size);
void remosaic_process_param_set(struct st_remosaic_param* p_param);
int32_t remosaic_process(int32_t src_buf_fd, size_t src_buf_size,
                    int32_t dst_buf_fd, size_t dst_buf_size);
void remosaic_deinit();
