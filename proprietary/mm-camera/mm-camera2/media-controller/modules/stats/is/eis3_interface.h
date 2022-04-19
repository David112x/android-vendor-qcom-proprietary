/* eis3_interface.h
 *
 * Copyright (c) 2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __EIS3_INTERFACE_H__
#define __EIS3_INTERFACE_H__
#include "is_interface.h"


/** gyro_times_t:
 *    @first_gyro_ts: first gyro sample timestamps in a batch of gyro samples
 *    @last_gyro_ts: last gyro sample timestamps in a batch of gyro samples
 */
typedef struct {
  uint64_t first_gyro_ts;
  uint64_t last_gyro_ts;
} gyro_times_t;


/** eis3_input_t:
 *    @frame_id: frame id
 *    @frame_times: times associated frame (SOF, exposure, duration)
 *    @gyro_times: gyro sample timestamps
 *    @is_last_frame:  last frame indication
 */
typedef struct {
  uint32_t frame_id;
  frame_times_t frame_times;
  gyro_times_t gyro_times;
  int is_last_frame;
} eis3_input_t;


int eis3_initialize(void **eis3_handle, is_init_data_t *init_data);
int eis3_process(void *eis3_handle, eis3_input_t *eis3_input, is_output_type *is_output);
void eis3_deinitialize(void *eis3_handle);
void eis3_flush(void *eis3_handle);


#endif //__EIS3_INTERFACE_H__
