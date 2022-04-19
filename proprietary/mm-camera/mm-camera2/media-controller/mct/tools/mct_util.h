/*Copyright (c) 2015, 2018 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *mct_util.h
 *
 */

#ifndef MCT_UTIL_H
#define MCT_UTIL_H

#include <linux/media.h>

#include "mtype.h"
#include <pthread.h>
#include "cam_intf.h"

#ifndef MIN
#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) (((x)>(y)) ? (x) : (y))
#endif

#define SEC_TO_NS_FACTOR 1000000000

#define MAX_DEV_NAME_SIZE 32
#define MCT_THREAD_TIMEOUT 6 /*In seconds unit*/

typedef enum {
  COMPARE_LOWER,
  COMPARE_HIGHER,
  COMPARE_EQUAL
} mct_util_comparison_type_t;

int mct_util_calculate_lcm(int data1, int data2);

boolean mct_util_get_timeout(uint32_t timeout_interval,
  struct timespec *ts);

boolean mct_spawn_thread(void *(*t_func)(void *),
  void *args, const char *t_name, int t_state);

void mct_util_sort_fps_entries (
    cam_fps_range_t* fps_table, uint8_t num_entries);

int8_t mct_util_find_closest_fps (
  float src_fps, cam_fps_range_t* ref_table, uint8_t num_entries,
  mct_util_comparison_type_t compare_type);

boolean mct_util_check_matching_fps (
  cam_fps_range_t input_fps, cam_fps_range_t* ref_table,uint8_t num_entries);

int8_t mct_util_pick_lowest_fps(cam_fps_range_t *crucible,
  uint8_t num_entries);

int32_t mct_util_find_v4l2_subdev(char *node_name);

int mct_util_create_timer();

void mct_util_set_timer(struct timespec *timeToWait);

int mct_util_match_subdev_type(struct media_entity_desc entity,
  uint32_t gid, uint32_t type);
#endif
