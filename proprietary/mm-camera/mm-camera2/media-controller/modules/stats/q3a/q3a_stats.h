/* q3a_stats.h
 *
 * Copyright (c) 2013-2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __Q3A_STATS_H__
#define __Q3A_STATS_H__
#include "q3a_stats_hw.h"
#include "camera_dbg.h"
#include "q3a_common_types.h"

#define MAX_EXP_BRACKETING_LENGTH 32

#undef  MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/** q3a_operation_mode_t
 *
 *  This enumeration defines the operation modes for the module.
 */
typedef enum {
  Q3A_OPERATION_MODE_NONE,
  Q3A_OPERATION_MODE_INIT,
  Q3A_OPERATION_MODE_PREVIEW,
  Q3A_OPERATION_MODE_SNAPSHOT,
  Q3A_OPERATION_MODE_CAMCORDER,
  Q3A_OPERATION_MODE_INVALID,
} q3a_operation_mode_t;


/** stats_yuv_stats_t
 *    @q3a_aec_stats:   AEC stats from the ISP
 *    @q3a_af_stats:    AF stats from the ISP
 *    @q3a_awb_stats:   AWB stats from the ISP
 *    @histogram:       histogram stats from the ISP
 *    @q3a_cs_stats:    CS stats from the ISP
 *    @q3a_rs_stats:    RS stats from the ISP
 *    @q3a_ihist_stats: IHIST stats from the ISP
 *
 *  This structure is used to pass YUV stats from the ISP to the libraries
 *  for processing.
 */
typedef struct {
  q3a_aec_stats_t*  p_q3a_aec_stats;
  q3a_af_stats_t*   p_q3a_af_stats;
  q3a_awb_stats_t*  p_q3a_awb_stats;
  uint32_t*         p_histogram;
  q3a_cs_stats_t*   p_q3a_cs_stats;
  q3a_rs_stats_t*   p_q3a_rs_stats;
  q3a_ihist_stats_t* p_q3a_ihist_stats;
} stats_yuv_stats_t;

/** stats_bayer_stats_t
 *    @q3a_bg_stats:    Bayer Grid Stats from the ISP
 *    @q3a_bf_stats:    Bayer Focus Stats from the ISP
 *    @q3a_bhist_stats: Bayer Histogram Stats from the ISP
 *
 *  This structure is used to pass BAYER stats from the ISP to the libraries
 *  for processing.
 */
typedef struct {
  q3a_bg_stats_t      *p_q3a_bg_stats;
  q3a_bg_stats_t      *p_q3a_bg_aec_stats;
  q3a_bf_stats_t      *p_q3a_bf_stats;
  q3a_be_stats_t      *p_q3a_be_stats;
  q3a_hdr_be_stats_t  *p_q3a_hdr_be_stats;
  q3a_bhist_stats_t   *p_q3a_bhist_stats;
} stats_bayer_stats_t;

/** Defines the 3HDR statistic  data structure
*
* @stats_hdr3_hist_data: 3HDR statistic sub data structures
* @hdr3_sensor_type:     Indicate 3HDR sensor type
*/
typedef struct {
  stats_hdr3_hist_data_type stats_hdr3_hist_data;
  uint32 hdr3_sensor_type;
} stats_3hdr_stats_t;

/** stats_t
 *    @stats_type_mask: This mask shows what types of stats are passed by ISP
 *    @frame_id:        The ID of the frame for which the stats are calculated
 *    @time_stamp:      The timestamp of the message.
 *    @yuv_stats:       The YUV stats from the ISP.
 *    @bayer_stats      The BAYER stats from the ISP.
 *    @hdr3_stats       The 3HDR stats from sensor RDI.
 *    @ack_data         The data used to ACK done use of STATS
 *                      data to ISP
 *    @isp_stream_type: stats streaming mode from ISP -
 *                      online or offline
 *  This structure is used to pass the calculated stats from the
 *  ISP to the libraries for processing. It packs additional
 *  information to determine which frame are these stats for.
 */
typedef struct {
  stats_type_t        stats_type_mask;
  uint32_t            frame_id;
  time_stamp_t        time_stamp;
  time_stamp_t        sof_timestamp;
  uint8_t             camera_id;

  stats_yuv_stats_t   yuv_stats;
  stats_bayer_stats_t bayer_stats;
  stats_3hdr_stats_t  hdr3_stats;
  void *              ack_data;
  q3a_stats_stream_type isp_stream_type;
} stats_t;

/** af_stats_t
 *    @stats_type_mask: What is the type of the stats. For this structure
 *                      they will be either STATS_BF or STATS_AF. This will
 *                      tell us what union member to access.
 *    @frame_id:        The ID of the frame for which these stats are
 *                      calculated.
 *    @time_stamp:      The timestamp of the message.
 *    @isFake:      If the stats is fake or not
 *    @camera_id:       The corresponding camera id of these stats
 *    @q3a_af_stats:    If the stats are of type STATS_AF, this member will
 *                      contain the calculated AF stats passed from the ISP.
 *    @q3a_bf_stats:    If the stats are of type STATS_BF, this member will
 *                      contain the calculated BF stats passed from the ISP.
 *
 *  This structure is used to pass the AF/BF stats from ISP to the AF library
 *  for processing.
 */
typedef struct {
  stats_type_t   stats_type_mask;
  uint32_t       frame_id;
  time_stamp_t   time_stamp;
  time_stamp_t   sof_timestamp;
  boolean        isFake;
  uint8_t        camera_id;

  union {
    q3a_af_stats_t* p_q3a_af_stats;
    q3a_bf_stats_t* p_q3a_bf_stats[MAX_BF_KERNEL_NUM];
  } u;

  void *         ack_data;
} stats_af_t;

#endif /* __Q3A_STATS_H__ */
