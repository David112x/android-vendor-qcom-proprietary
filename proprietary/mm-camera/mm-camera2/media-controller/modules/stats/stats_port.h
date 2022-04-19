/* stats_port.h
 *
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "3AStatsDataTypes.h"

#ifndef __STATS_PORT_H__
#define __STATS_PORT_H__

#define STATS_MAX_FRAME_DELAY   5
#define STATS_PER_FRAME_CTRL    1
#define STATS_DELAY             2

typedef void (* set_pipeline_delay_func)(mct_port_t *port,
  mct_pipeline_session_data_t *session_data);

typedef enum {
  STATS_PORT_STATE_CREATED,
  STATS_PORT_STATE_RESERVED,
  STATS_PORT_STATE_LINKED,
  STATS_PORT_STATE_UNLINKED,
  STATS_PORT_STATE_UNRESERVED
} stats_port_state_t;

/** stats_port_private_t
 *    @sub_ports: stats module's sub-modules' sink-ports,
 *                used for internal link function
 **/

typedef struct {
  unsigned int         identity;
  cam_stream_type_t    stream_type;
  cam_streaming_mode_t streaming_mode;
  boolean              used_flag;
}stats_port_reserved_streams;

typedef struct {
  unsigned int cap_flag;
  mct_event_t  *event;
} stats_port_event_t;

typedef struct {
  boolean            has_chromatix_set;
  int32_t            stream_on_counter;
  stats_port_event_t evt[CAM_INTF_PARM_MAX];
  boolean            is_initialised[CAM_INTF_PARM_MAX];
  pthread_mutex_t    msg_q_lock[STATS_MAX_FRAME_DELAY];
  mct_queue_t        *msg_q[STATS_MAX_FRAME_DELAY];
} stats_port_setparm_ctrl_t;

typedef struct {
  int applying_delay;
  int meta_reporting_delay;
} stats_port_pipeline_delay;

/*First 3 members of this structure should be same for all the sub-modules of stats.
*Else we might run into wrong stream types*/
typedef struct _stats_port_private {
  unsigned int                reserved_id;
  cam_stream_type_t           stream_type;
  stats_port_state_t          state;
  stats_port_reserved_streams streams_info[MAX_NUM_STREAMS];
  unsigned int                video_stream_cnt;
  unsigned int                snapshot_stream_cnt;
  unsigned int                preview_stream_cnt;
  unsigned int                param_stream_cnt;
  uint32_t                    sof_id;
  mct_list_t                  *sub_ports;
  unsigned int                snap_stream_id;
  stats_port_setparm_ctrl_t   parm_ctrl;
  stats_port_pipeline_delay   delay;
  set_pipeline_delay_func     set_pipeline_delay;
  float                       max_sensor_fps;
  boolean                     skip_stats_mode;
  int32_t                     current_fps;
  int                         fake_trigger_id;
  boolean                     legacy_hal_cmd;
  mct_stream_info_t           preview_stream_info;
  int32_t                     preview_stream_width;
  int32_t                     preview_stream_height;
  int32_t                     hal_version;
  int                         skip_pattern;
  boolean                     preview_stream_on;
  int32_t                     bg_stats_buffer_size;
  int32_t                     be_stats_buffer_size;
  int32_t                     bg_aec_stats_buffer_size;
  int32_t                     bhist_stats_buffer_size;
  int32_t                     bg_config_buffer_size;
  int32_t                     be_config_buffer_size;
  int32_t                     bg_aec_config_buffer_size;
  BayerGridStatsType          bg_stats_debug_Data;
  BayerExposureStatsType      be_stats_debug_data;
  BayerGridStatsType          bg_aec_stats_debug_Data;
  BayerHistogramStatsType     hist_stats_debug_Data;
  BayerStatsConfigType        bg_config_debug_data;
  BayerStatsConfigType        be_config_debug_data;
  BayerStatsConfigType        bg_aec_config_debug_data;
  TuningDataInfoType          tuning_info_debug_data;
  uint32_t                    sessionStreamId;
  pthread_mutex_t             stats_buf_mutex; /* used to protect the stats counter
                                                * for circular buffer feature */
  boolean                     frame_capture_mode_in_progress;
  chromatix_3a_parms_wrapper  chromatix_wrapper;
  boolean                     is_stats_port_locks_initialized;
} stats_port_private_t;


/** stats_port_sub_link_t
 *    @id:   identity of current stream
 *    @peer: the peer module currently connected to
 *
 *  Structure used to link port with external peer
 **/
typedef struct {
  unsigned int id;
  mct_port_t   *peer;
} stats_port_sub_link_t;

typedef struct {
  unsigned int id;
  mct_list_t   *list;
} stats_port_int_link_t;

/** stats_port_caps_reserve_t
 *    @caps:        the capability of the port
 *    @stream_info: the information of current stream
 *
 *  Structure used to reserve port's capability
 **/
typedef struct {
  mct_port_caps_t   caps;
  mct_stream_info_t *stream_info;
} stats_port_caps_reserve_t;

boolean stats_port_init(mct_port_t *port, unsigned int identity,
  mct_list_t *sub_ports);
void    stats_port_deinit(mct_port_t *port);
boolean stats_port_check_port(mct_port_t *port, unsigned int identity);

void stats_port_set_log_level ();
#endif /* __STATS_PORT_H__ */
