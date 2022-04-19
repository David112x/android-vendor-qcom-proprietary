/* abcc_algo.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ABCC_ALGO_H__
#define __ABCC_ALGO_H__

#include "modules.h"

#define PIX_IND(v,w) ((v.x)*w+(v.y))

#define PIX_CMP(p1, p2) \
  ((p1.x == p2.x) && (p1.y == p2.y))

typedef struct {
  int x;
  int y;
} abcc_pix_t;

typedef struct {
  pix_t pixel_index;
  int kernel_index;
  int skip_index[2];
}abcc_entry_t;

typedef struct {
  pix_t center;
  pix_t neighbor;
  int single_defective_pix;
}abcc_proc_entry_t;

typedef struct {
  pix_t center;
  pix_t neighbor;
  int kernel_index;
  int skip_index[2];
  int single_defective_pix;
}abcc_single_pix_entry_t;

typedef struct {
  abcc_entry_t *entry;
  int actual_count;
  int count;
}abcc_list_t;

typedef struct {
  abcc_proc_entry_t *entry;
  int actual_count;
  int count;
}abcc_proc_list_t;

typedef struct {
  abcc_single_pix_entry_t *entry;
  int actual_count;
  int count;
}abcc_single_pix_list_t;

typedef struct {
  abcc_list_t lut_list;
  abcc_proc_list_t processed_list;
  abcc_list_t final_lut_list;
  abcc_single_pix_list_t single_pix_list;
}abcc_lut_t;

/* the structure of defect table is as follows
  checked? | corrected? |  present/future update
  (1 = yes, 0 = no)   (1 = present, 0 = future) */
typedef struct {
  int checked;
  int corrected;
  int present_update;
} abcc_corr_tab_entry_t;

typedef struct {
  abcc_corr_tab_entry_t *val;
  int count;
} abcc_corr_table_t;

typedef struct {
  /* input */
  defective_pix_array_t defect_pixels;
  int sensor_width;
  int sensor_height;
  int forced_correction;
  /* output */
  defective_pix_array_t unprocessed;
  abcc_lut_t lut;
  /* intermediate */
  abcc_corr_table_t corr_table;
} abcc_algo_t;

void abcc_algo_deinit(abcc_algo_t *info);

boolean abcc_algo_init(abcc_algo_t *info);

boolean abcc_algo_process(abcc_algo_t *info, int maxNoEntries);

#endif /* __ABCC_ALGO_H__ */
