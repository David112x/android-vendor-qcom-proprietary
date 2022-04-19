/* is_process.c
 *
 * Copyright (c) 2013-2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "is.h"


/** is_process_is_initialize:
 *    @is_info: IS internal variables
 *    @stream_type: preview or video
 *
 * This function initializes IS.
 **/
static void is_process_is_initialize(is_info_t *is_info, int stream_type)
{
  int rc = 0;
  is_init_data_t is_init_data;

  is_init_data.sns_handle = NULL;
  is_init_data.frame_cfg.frame_fps = is_info->fps;
  is_init_data.frame_cfg.dis_frame_width = is_info->width[stream_type];
  is_init_data.frame_cfg.dis_frame_height = is_info->height[stream_type];
  is_init_data.frame_cfg.vfe_output_width = is_info->vfe_width[stream_type];
  is_init_data.frame_cfg.vfe_output_height = is_info->vfe_height[stream_type];
  is_init_data.frame_cfg.num_mesh_y = is_info->num_mesh_y;
  is_init_data.rs_cs_config.num_row_sum = is_info->num_row_sum;
  is_init_data.rs_cs_config.num_col_sum = is_info->num_col_sum;
  is_init_data.is_type = is_info->is_type[stream_type];
  is_init_data.sensor_mount_angle = is_info->sensor_mount_angle;
  is_init_data.camera_position = is_info->camera_position;
  is_init_data.dis_bias_correction = is_info->dis_bias_correction;
  STATS_MEMCPY(&is_init_data.is_chromatix_info, sizeof(is_chromatix_info_t),
    &is_info->is_chromatix_info, sizeof(is_chromatix_info_t));
  is_init_data.buffer_delay = is_info->buffer_delay;

  if (is_info->width[stream_type] == 0 || is_info->height[stream_type] == 0 ||
    is_info->vfe_width[stream_type] <= is_info->width[stream_type] ||
    is_info->vfe_height[stream_type] <= is_info->height[stream_type]) {
    IS_ERR("IS CRITICAL ERROR : IS did not get dimensions");
    rc = -1;
  }

  /* For now, DIS and EIS initialization need to succeed */
  STATS_MEMSET(&is_info->dis_context, 0, sizeof(dis_context_type));

  if (rc == 0 && is_info->is_type[stream_type] != IS_TYPE_DIS) {
    if (is_info->sns_handle == NULL) {
      is_info->sns_handle = sns_is_init(NULL);
      is_info->sns_lib_offset_set = 0;
      if (is_info->sns_handle != NULL) {
        is_info->sns_ref_cnt = 1;
        IS_HIGH("IS sensors library inited, sns_ref_cnt = %d", is_info->sns_ref_cnt);
      } else {
        IS_ERR("NULL sensors library handle");
        rc = -1;
      }
    } else {
      is_info->sns_ref_cnt = 2;
      IS_HIGH("Instance of IS sensors library already exist, sns_ref_cnt = %d", is_info->sns_ref_cnt);
    }
  }

  if ((is_info->is_type[stream_type] == IS_TYPE_DIS || is_info->dis_bias_correction) &&
    (rc == 0)) {
    rc = dis_initialize(&is_info->dis_context, &is_init_data);
  }

  if (rc == 0) {
    if (is_info->is_type[stream_type] == IS_TYPE_EIS_2_0) {
      STATS_MEMSET(&is_info->eis2_context, 0, sizeof(eis2_context_type));
      is_init_data.sns_handle = is_info->sns_handle;
      rc = eis2_initialize(&is_info->eis2_context, &is_init_data);
    } else if (is_info->is_type[stream_type] == IS_TYPE_EIS_3_0) {
      is_init_data.sns_handle = is_info->sns_handle;
      rc = eis3_initialize(&is_info->eis3_handle, &is_init_data);
      is_info->flush_mode = FALSE;
    }

    if (rc == 0) {
      is_info->is_inited[stream_type] = 1;
      IS_HIGH("IS inited");
    } else if (is_info->is_type[stream_type] == IS_TYPE_DIS ||
      is_info->dis_bias_correction) {
      dis_exit(&is_info->dis_context);
    }
  }

  if (rc != 0) {
    IS_ERR("IS initialization failed");
    /* Disable IS for the stream so we won't keep initializing and failing */
    is_info->is_enabled[stream_type] = 0;
    if (is_info->is_type[stream_type] != IS_TYPE_DIS) {
      is_info->sns_ref_cnt--;
      is_info->sns_ref_cnt = MAX(0, is_info->sns_ref_cnt);
      /* Release sensors library instance */
      if (is_info->sns_ref_cnt == 0) {
        sns_is_stop(is_info->sns_handle);
        is_info->sns_handle = NULL;
        IS_HIGH("IS sensors library released");
      }
    }
  }
}


/** is_process_is_deinitialize:
 *    @is_info: IS internal variables
 *    @stream_type: preview or video
 *
 * This function deinits IS.
 **/
static void is_process_is_deinitialize(is_info_t *is_info, int stream_type)
{
  if (is_info->is_type[stream_type] != IS_TYPE_DIS) {
    is_info->sns_ref_cnt--;
    is_info->sns_ref_cnt = MAX(0, is_info->sns_ref_cnt);
    if (is_info->sns_ref_cnt == 0) {
      sns_is_stop(is_info->sns_handle);
      is_info->sns_handle = NULL;
      IS_HIGH("IS sensors library released");
    }
  }

  if (is_info->is_type[stream_type] == IS_TYPE_EIS_2_0) {
    eis2_deinitialize(&is_info->eis2_context);
  } else if (is_info->is_type[stream_type] == IS_TYPE_EIS_3_0) {
    eis3_deinitialize(is_info->eis3_handle);
    is_info->flush_mode = FALSE;
    is_info->initial_sof_id = 0;
  }

  if (is_info->is_type[stream_type] == IS_TYPE_DIS || is_info->dis_bias_correction) {
    dis_exit(&is_info->dis_context);
  }
  is_info->is_inited[stream_type] = 0;
  if (!is_info->is_inited[IS_PREVIEW] && !is_info->is_inited[IS_VIDEO]) {
    is_info->gyro_frame_id = 0;
    is_info->rs_cs_frame_id = 0;
  }
  IS_HIGH("IS deinited for stream %d", stream_type);
}


/** is_process_is_flush:
 *    @is_info: IS internal variables
 *    @is_output: current is output used as input parameters for flush output
 *    @is_output_flush: flush output of the event processing
 *
 * This function generates the final matrices for the buffered gyro data.
 *
 * Returns number of outputs.
 **/
static int is_process_is_flush(is_info_t *is_info, is_output_type *is_output, is_output_type *is_output_flush)
{
  uint32_t i, j, k, last_frame_id;
  boolean run_is;

  /* Systems team recommends executing the EIS 3.0 algorithm with the remaining gyro
     data to flush the bufferred frames.  For now, we are going to skip running the
     algorithm and flush out the frames with the last correction matrix.  Thus, set
     run_is to FALSE. */
  run_is = FALSE;

  IS_HIGH("Last gyro_frame_id = %u, last update frame_id = %u",
    is_info->gyro_frame_id, is_output[IS_VIDEO].frame_id);

  last_frame_id = is_output[IS_VIDEO].frame_id;
  is_info->eis3_input.is_last_frame = 1;
  j = 0;

  if (run_is) {
    k = is_info->gyro_frame_id + 1;
  } else {
    k = last_frame_id + 1;
    /* Flush with identity matrix */
    for (i = 0; i < MAX_IS_MESH_Y + 1; i++) {
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 0] = 1.0 /
        (1 + 2 * is_info->is_chromatix_info.virtual_margin);
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 1] = 0.0;
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 2] = 0.0;
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 3] = 0.0;
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 4] = 1.0 /
        (1 + 2 * is_info->is_chromatix_info.virtual_margin);
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 5] = 0.0;
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 6] = 0.0;
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 7] = 0.0;
      is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 8] = 1.0;
    }
  }

  for (i = last_frame_id + 1; i <= is_info->gyro_frame_id; i++) {
    IS_HIGH("Generate output for frame id %u", i);
    if (j == IS_MAX_BUFFER_DELAY) {
      break;
    }
    if (run_is) {
      is_output_flush[j].x = is_output[IS_VIDEO].x;
      is_output_flush[j].y = is_output[IS_VIDEO].y;
      is_output_flush[j].frame_id = k;
      eis3_process(is_info->eis3_handle, &is_info->eis3_input, &is_output_flush[j]);
    } else {
      STATS_MEMCPY(&is_output_flush[j], sizeof(is_output_type), &is_output[IS_VIDEO], sizeof(is_output_type));
      is_output_flush[j].frame_id = k;
    }
    j++;
    k++;
  }
  is_info->eis3_input.is_last_frame = 0;
  return (j);
}


/** is_process_run_gyro_dependent_is:
 *    @is_info: IS internal variables
 *    @rs_cs_data: RS/CS stats
 *    @gyro_data: gyro data
 *    @is_output: output of the event processing
 *    @stream_type: preview or video
 *
 * This function initializes IS.
 **/
static void is_process_run_gyro_dependent_is(is_info_t *is_info,
  rs_cs_data_t *rs_cs_data, mct_event_gyro_data_t *gyro_data,
  is_output_type *is_output, int stream_type)
{
  frame_times_t frame_times;
  unsigned int i;

  frame_times.sof = gyro_data->sof_timestamp;
  frame_times.exposure_time = gyro_data->exposure_time;
  frame_times.frame_time = gyro_data->frame_time;
  IS_LOW("gyro_data.sof = %llu", frame_times.sof);

  if (is_info->is_type[stream_type] == IS_TYPE_EIS_2_0) {
    is_output->frame_id = is_info->gyro_frame_id;
    eis2_process(&is_info->eis2_context, &frame_times, is_output);
  } else if (is_info->is_type[stream_type] == IS_TYPE_EIS_3_0) {
    is_info->eis3_input.frame_times = frame_times;
    if (gyro_data->sample_len > 0) {
      is_info->eis3_input.gyro_times.first_gyro_ts = gyro_data->sample[0].timestamp;
      is_info->eis3_input.gyro_times.last_gyro_ts =
        gyro_data->sample[gyro_data->sample_len - 1].timestamp;
    }
    is_info->eis3_input.frame_id = is_info->gyro_frame_id;
    if (FALSE == is_info->flush_mode) {
      eis3_process(is_info->eis3_handle, &is_info->eis3_input, is_output);
    } else {
      /* PPROC expects DIS update events for PCRs already mapped to SoFs before flush
         Sending unity matrix for all those frames*/

      IS_LOW("Flush_mode %d,Sending unity matrix for frame id %d", is_info->flush_mode, is_info->gyro_frame_id);
      for (i = 0; i < MAX_IS_MESH_Y + 1; i++) {
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 0] = 1.0 /
          (1 + 2 * is_info->is_chromatix_info.virtual_margin);
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 1] = 0.0;
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 2] = 0.0;
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 3] = 0.0;
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 4] = 1.0 /
          (1 + 2 * is_info->is_chromatix_info.virtual_margin);
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 5] = 0.0;
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 6] = 0.0;
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 7] = 0.0;
        is_output[IS_VIDEO].transform_matrix[i*NUM_MATRIX_ELEMENTS + 8] = 1.0;
      }
      is_output->has_output = TRUE;
      is_output->frame_id = is_info->gyro_frame_id;
    }
  }

  if (is_info->is_type[stream_type] == IS_TYPE_GA_DIS || is_info->dis_bias_correction) {
    is_output->frame_id = is_info->gyro_frame_id;
    dis_process(&is_info->dis_context, rs_cs_data, &frame_times, is_output);
  }
}


/** is_process_stats_event:
 *    @stats_data: RS/CS stats
 *    @is_output: output of the event processing
 *
 * If DIS is the selected IS algorithm, this function runs the DIS algorithm.
 * If the selected IS algorithm depends on gyro data (EIS), this functions runs
 * the IS algorithm only if the frame's gyro data is available.  In other
 * words, two items are are needed to run IS on a frame, frame's RS/CS stats and
 * frame's gyro samples.  This is the function that runs the IS algorithm when
 * the frame's gyro data arrives before the frame's RS/CS stats.
 *
 * Returns TRUE if the IS algorithm ran, FALSE otherwise.
 **/
static boolean is_process_stats_event(is_stats_data_t *stats_data,
  is_output_type *is_output)
{
  int rc = TRUE;
  int i;
  struct timespec t_now;
  is_info_t *is_info = stats_data->is_info;

  if (!is_info->is_inited[IS_PREVIEW] && is_info->run_is[IS_PREVIEW] && is_info->is_enabled[IS_PREVIEW]) {
    IS_HIGH("Init IS for preview");
    is_process_is_initialize(is_info, IS_PREVIEW);
  }

  if (!is_info->is_inited[IS_VIDEO] && is_info->stream_on[IS_VIDEO] &&
    is_info->run_is[IS_VIDEO] && (is_info->is_type[IS_PREVIEW] != is_info->is_type[IS_VIDEO]) &&
    is_info->is_enabled[IS_VIDEO]) {
    IS_HIGH("Init IS for video");
    is_process_is_initialize(is_info, IS_VIDEO);
  }

  if (is_info->is_inited[IS_VIDEO] || is_info->is_inited[IS_PREVIEW]) {
    clock_gettime( CLOCK_REALTIME, &t_now );
    IS_LOW("RS(%u) & CS(%u) ready, time = %llu, ts = %llu, id = %u",
      stats_data->is_info->num_row_sum, stats_data->is_info->num_col_sum,
      ((int64_t)t_now.tv_sec * 1000 + t_now.tv_nsec/1000000),
      ((int64_t)stats_data->is_info->timestamp.tv_sec * 1000 +
                stats_data->is_info->timestamp.tv_usec/1000),
      stats_data->frame_id);
    is_output->frame_id = stats_data->frame_id;
    is_info->rs_cs_frame_id = stats_data->frame_id;

    /* We only need rs/cs stats in this case, adding checker here to save CPU */
    if (is_info->use_stats) {
      /* downsample RS stats */
      is_rs_stats_t *to_rs_stats = NULL;
      q3a_rs_stats_t *from_q3a_rs_stats = NULL;
      uint32_t i = 0, j = 0;
      to_rs_stats = &is_info->rs_cs_data.rs_stats;
      from_q3a_rs_stats = stats_data->yuv_rs_cs_data.p_q3a_rs_stats;
      /* there is case the stats event sent to algo, but the stats buffer is NULL
         whose purpose is only to triger the IS processing.So do the stats validiaty
         check here */
      if (from_q3a_rs_stats) {
        /* Convert 8x 1024 RS to 1 x 1024*/
        for(i = 0; i < from_q3a_rs_stats->num_v_regions; i++)
          for(j = 0; j < from_q3a_rs_stats->num_h_regions; j++)
            to_rs_stats->row_sum[i] += from_q3a_rs_stats->row_sum[j][i];

        to_rs_stats->num_row_sum = is_info->num_row_sum;

        /* copy CS stats */
        STATS_MEMCPY(&is_info->rs_cs_data.cs_stats, sizeof(is_info->rs_cs_data.cs_stats),
          stats_data->yuv_rs_cs_data.p_q3a_cs_stats, sizeof(q3a_cs_stats_t));
      }
    }
    for (i = 0; i < IS_MAX_STREAMS; i++) {
      if (is_info->is_inited[i]) {
        if (is_info->is_type[i] != IS_TYPE_DIS) {
          if (is_info->gyro_frame_id >= is_info->rs_cs_frame_id) {
            IS_LOW("Gyro is ready, can run IS, gyro_fid = %u, rs_cs_fid = %u",
              is_info->gyro_frame_id, is_info->rs_cs_frame_id);
            is_process_run_gyro_dependent_is(is_info, &is_info->rs_cs_data,
              &is_info->gyro_data, &is_output[i], i);
          } else {
            rc = FALSE;
            IS_LOW("Gyro not ready, can't run IS, gyro_fid = %u, rs_cs_fid = %u",
              is_info->gyro_frame_id, is_info->rs_cs_frame_id);
          }
        } else {
          frame_times_t frame_times;

          STATS_MEMSET(&frame_times, 0, sizeof(frame_times_t));
          dis_process(&is_info->dis_context, &is_info->rs_cs_data,
            &frame_times, &is_output[i]);
        }
      } else {
        /* IS for the stream is not inited which can mean:
           - Preview and video have the same is_type and hence only one instance
           of the algorithm may be running.  If this is the case and video stream
           is on, copy IS preview output to IS video output
           - The stream (video) is not on
           - No IS for the stream
         */
         if (is_info->is_type[IS_PREVIEW] == is_info->is_type[IS_VIDEO] &&
           is_info->stream_on[IS_VIDEO] && is_output[IS_PREVIEW].has_output) {
           STATS_MEMCPY(&is_output[IS_VIDEO], sizeof(is_output_type),
            &is_output[IS_PREVIEW], sizeof(is_output_type));
         }
      }
    }
  }

  return rc;
}


/** is_process_gyro_stats_event:
 *    @gyro_stats_data: gyro data
 *    @is_output: output of the event processing
 *
 * If the selected IS algorithm depends on gyro data (EIS), this functions runs
 * the IS algorithm only if the frame's stats data is available.  In other
 * words, two items are are needed to run IS on a frame, frame's RS/CS stats and
 * frame's gyro samples.  This is the function that runs the IS algorithm when
 * the frame's RS/CS stats arrives before the frame's gyro data.
 *
 * Returns TRUE if the IS algorithm ran, FALSE otherwise.
 **/
static boolean is_process_gyro_stats_event(is_gyro_data_t *gyro_stats_data,
  is_output_type *is_output)
{
  int rc = TRUE;
  unsigned int i;
  is_info_t *is_info = gyro_stats_data->is_info;
  mct_event_gyro_data_t *gyro_data = &gyro_stats_data->gyro_data;

  is_info->gyro_frame_id = gyro_stats_data->frame_id;

  if (!is_info->sns_lib_offset_set && is_info->sns_handle != NULL) {
    set_sns_apps_offset(is_info->sns_handle, gyro_data->sample[0].timestamp);
    is_info->sns_lib_offset_set = 1;
  }

  IS_LOW("num_samples %d", gyro_data->sample_len);
  /* Update the gyro buffer */
  for (i = 0; i < gyro_data->sample_len; i++) {
    IS_LOW("Poking in gyro sample, %llu, %d, %d, %d",
      gyro_data->sample[i].timestamp, gyro_data->sample[i].value[0],
      gyro_data->sample[i].value[1], gyro_data->sample[i].value[2]);
    poke_gyro_sample(is_info->sns_handle, gyro_data->sample[i].timestamp,
      gyro_data->sample[i].value[0],
      gyro_data->sample[i].value[1],
      gyro_data->sample[i].value[2]);
  }

  for (i = 0; i < IS_MAX_STREAMS; i++) {
    if (is_info->is_inited[i]) {
      if (is_info->gyro_frame_id <= is_info->rs_cs_frame_id || !is_info->use_stats) {
        IS_LOW("Frame is ready, can run IS, gyro_fid = %u, rs_cs_fid = %u",
          is_info->gyro_frame_id, is_info->rs_cs_frame_id);
        is_process_run_gyro_dependent_is(is_info, &is_info->rs_cs_data,
          &gyro_stats_data->gyro_data, &is_output[i], i);
      } else {
        rc = FALSE;
        /* Cache gyro data for when the frame is ready */
        STATS_MEMCPY(&is_info->gyro_data, sizeof(mct_event_gyro_data_t),
          &gyro_stats_data->gyro_data, sizeof(mct_event_gyro_data_t));
        IS_LOW("Frame not ready, can't run IS, gyro_fid = %u, rs_cs_fid = %u",
          is_info->gyro_frame_id, is_info->rs_cs_frame_id);
      }
    } else {
      /* IS for the stream is not inited which can mean:
         - Preview and video have the same is_type and hence only one instance
         of the algorithm may be running.  If this is the case and video stream
         is on, copy IS preview output to IS video output
         - The stream (video) is not on
         - No IS for the stream
       */
      if (is_info->is_type[IS_PREVIEW] == is_info->is_type[IS_VIDEO] &&
        is_info->stream_on[IS_VIDEO] && is_output[IS_PREVIEW].has_output) {
        STATS_MEMCPY(&is_output[IS_VIDEO], sizeof(is_output_type),
          &is_output[IS_PREVIEW], sizeof(is_output_type));
      }
    }
  }

  return rc;
}


/** is_process_no_gyro_data:
 *    @sof_id: SOF frame id
 *    @is_output: output of the event processing
 *
 * Returns TRUE.
 **/
static boolean is_process_no_gyro_data(is_no_gyro_data_t *no_gyro_data, is_output_type *is_output)
{
  if (no_gyro_data->sof_id >= no_gyro_data->is_info->initial_sof_id - 1 + no_gyro_data->is_info->buffer_delay) {
    is_output[IS_VIDEO].frame_id = no_gyro_data->sof_id - no_gyro_data->is_info->buffer_delay + 1;
    is_output[IS_VIDEO].has_output = 1;
    is_output[IS_PREVIEW].has_output = 0;
  }
  return TRUE;
}


/** is_process:
 *    @param: input event parameters
 *    @output: output of the event processing
 *
 * This function is the top level event handler.
 **/
boolean is_process(is_process_parameter_t *param, is_process_output_t *output)
{
  int rc = TRUE;

  switch (param->type) {
  case IS_PROCESS_RS_CS_STATS:
    IS_LOW("IS_PROCESS_RS_CS_STATS, fid = %u", param->u.stats_data.frame_id);
    output->type = IS_PROCESS_OUTPUT_RS_CS_STATS;
    rc = is_process_stats_event(&param->u.stats_data, output->is_output);
    break;

  case IS_PROCESS_GYRO_STATS:
    IS_LOW("IS_PROCESS_GYRO_STATS, fid = %u", param->u.gyro_data.frame_id);
    rc = is_process_gyro_stats_event(&param->u.gyro_data, output->is_output);
    output->type = IS_PROCESS_OUTPUT_GYRO_STATS;
    break;

  case IS_PROCESS_FLUSH_MODE: {
    is_info_t *is_info = param->u.flush_mode.is_info;

    output->type = IS_PROCESS_OUTPUT_FLUSH_MODE;
    output->num_output = 0;
    if (param->u.flush_mode.flush_mode == TRUE) {
      if (is_info->is_inited[IS_VIDEO]) {
        if (is_info->is_type[IS_VIDEO] == IS_TYPE_EIS_3_0) {
          output->num_output = is_process_is_flush(is_info, output->is_output, output->is_output_flush);
          eis3_flush(is_info->eis3_handle);
        }
      }
      is_info->flush_mode = TRUE;
    } else {
      is_info->flush_mode = FALSE;
    }
  }
    break;

  case IS_PROCESS_STREAM_EVENT: {
    is_info_t *is_info = param->u.stream_event_data.is_info;

    IS_LOW("IS_PROCESS_STREAM_EVENT, s = %d",
      param->u.stream_event_data.stream_event);
    output->type = IS_PROCESS_OUTPUT_STREAM_EVENT;
    output->is_stream_event = param->u.stream_event_data.stream_event;
    switch (param->u.stream_event_data.stream_event) {
      case IS_PREVIEW_STREAM_ON:
        if (!is_info->use_stats && is_info->run_is[IS_PREVIEW] && is_info->is_enabled[IS_PREVIEW]) {
          is_process_is_initialize(is_info, IS_PREVIEW);
        }
        break;

      case IS_VIDEO_STREAM_ON:
        if (!is_info->use_stats && is_info->run_is[IS_VIDEO] && is_info->is_enabled[IS_VIDEO] &&
          (is_info->is_type[IS_PREVIEW] != is_info->is_type[IS_VIDEO])) {
          is_process_is_initialize(is_info, IS_VIDEO);
        }
        break;

      case IS_PREVIEW_STREAM_OFF:
        if (is_info->is_inited[IS_PREVIEW]) {
          is_process_is_deinitialize(is_info, IS_PREVIEW);
        }
        break;

      case IS_VIDEO_STREAM_OFF:
        if (is_info->is_inited[IS_VIDEO]) {
          if ((is_info->is_type[IS_VIDEO] == IS_TYPE_EIS_3_0) && (is_info->flush_mode == FALSE)) {
            output->num_output = 0;
            output->num_output = is_process_is_flush(is_info, output->is_output, output->is_output_flush);
          }
          is_process_is_deinitialize(is_info, IS_VIDEO);
        }
        break;

      default: ;
    }
  }
    break;

  case IS_PROCESS_NO_GYRO_DATA:
    output->type = IS_PROCESS_OUTPUT_NO_GYRO_DATA;
    rc = is_process_no_gyro_data(&param->u.no_gyro_data, output->is_output);
    break;

  default:
    break;
  }

  return rc;
} /* is_process */
