/*Copyright (c) 2016 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * mct_shim_layer.h
*/

#ifndef __MCT_SHIM_LAYER_H__
#define __MCT_SHIM_LAYER_H__
#define MAX_DEV_NAME_SIZE 32
#define TOTAL_RAM_SIZE_512MB 536870912

#include "cam_ker_headers.h"
#include "mtype.h"
#include "mct_list.h"
#include "mm_camera_shim.h"
#include "mct_controller.h"
#include <linux/videodev2.h>


int mct_shimlayer_process_event(cam_shim_packet_t *packet);

cam_status_t mct_shimlayer_start_session(int session,
  mm_camera_shim_event_handler_func event_cb);

int mct_shimlayer_stop_session(int session);

#endif /* MCT_SHIM_LAYER_H */
