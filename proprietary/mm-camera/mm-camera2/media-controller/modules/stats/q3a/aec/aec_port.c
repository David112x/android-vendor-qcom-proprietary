/* aec_port.c
 *
 * Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "aec_module.h"
#include "aec_port.h"
#include "aec_biz.h"
#include "aec_ext.h"
#include "q3a_port.h"
#include <pthread.h>
#include "modules.h"
#include "stats_event.h"
#include <math.h>
#include "sensor_lib.h"
#include "stats_util.h"
#include "stats_chromatix_wrapper.h"

#define AEC_UNDEF -1
#undef  LOG_TAG
#define LOG_TAG "AEC_PORT"

/* Define 3A debug info sw version number here. */
#define MAJOR_NUM             (0x05)
#define MINOR_NUM             (0x00)
#define PATCH_NUM             (0x00)
#define FEATURE_DESIGNATOR    (0x00)

#define LOG10_1P03               0.012837224705f
#define MAX_INTEGRATION_MARGIN   50L
#define Q8                       256L
#define Q10                      1024L

#define MIN_FLASH_CURRENT     10


/*Forward Declarations*/
static boolean aec_port_link_to_peer(mct_port_t *port, mct_event_t *event);
static boolean aec_port_unlink_from_peer(mct_port_t *port, mct_event_t *event);
static boolean aec_port_local_unlink(mct_port_t *port, mct_event_t *event);
static boolean aec_port_remote_unlink(mct_port_t *port);
static boolean aec_port_handle_role_switch(mct_port_t *port,mct_event_t *event);
static boolean aec_port_set_role_switch(aec_port_private_t *private);
static boolean aec_port_reset_dual_cam_info(aec_port_private_t *port);
static boolean aec_port_can_stats_be_configured(aec_port_private_t *port);
static boolean aec_port_update_aux_sync_and_update(aec_port_private_t *port);
static boolean aec_is_aux_sync_enabled(aec_port_private_t *port);
static boolean aec_is_aux_update_enabled(aec_port_private_t *port);
static boolean aec_port_update_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl);
static boolean aec_port_save_peer_aec_state(aec_port_private_t *port, uint8_t state);
static uint8_t aec_port_get_aec_state(aec_port_private_t *aec_port);
static boolean aec_port_send_bus_message_enabled(aec_port_private_t *aec_port,mct_bus_msg_type_t bus_msg_type);
static boolean aec_port_send_update_message_enabled(aec_port_private_t *aec_port);
static boolean aec_port_is_algo_active(aec_port_private_t *aec_port);
static boolean aec_port_forward_bus_message_if_linked(mct_port_t *mct_port,mct_event_t event);
static float aec_port_get_dual_cam_multiplier_unadjusted(aec_dual_cam_info* dual_cam_info);
static float aec_port_get_dual_cam_multiplier(aec_dual_cam_info* dual_cam_info,
                                              aec_port_peer_aec_update* master_update_info);
static void aec_port_interpolate_aec_update_from_peer(aec_port_private_t*       aec_port,
                                                      aec_port_peer_aec_update* master_update_info,
                                                      stats_update_t*           slave_stats_update);
static void aec_port_handle_peer_aec_update(mct_port_t* mct_port,
                                            aec_port_peer_aec_update* aec_update);
static void aec_port_handle_peer_event(mct_port_t* mct_port, mct_event_t* event);
static void aec_port_handle_peer_config(mct_port_t* mct_port, aec_port_peer_config* config);
static boolean aec_port_intra_event(mct_port_t *port, mct_event_t *event);
static boolean aec_port_proc_get_aec_data(mct_port_t * port,stats_get_data_t * stats_get_data);
static void aec_port_handle_aec_update(mct_port_t *port);

/* determin if AEC is currently using the divert stats */
boolean aec_port_using_HDR_divert_stats(aec_port_private_t *private)
{
  if (!private) {
    return FALSE;
  }

  return (private->video_hdr != 0 ||
     private->snapshot_hdr == AEC_SENSOR_HDR_IN_SENSOR);
}

void aec_port_reset_output_index(aec_port_private_t *private)
{
  private->state_update.sof_output_index = 0xFF; //invalid value
  private->state_update.cb_output_index  = 0xFF; //invalid value
  AEC_LOW("Reset output index");
}

static boolean aec_port_cal_manual_mode_exp_param(
   aec_port_private_t *private,
   float real_gain, uint32_t linecount, float exp_time,
   float *out_real_gain, float *out_sensor_gain, uint32_t *out_linecount, float *out_exp_time)
{
  if (!private || !out_real_gain || !out_sensor_gain || !out_linecount || !out_exp_time) {
    AEC_ERR("invalid parameter");
    return FALSE;
  }

  if (private->manual.is_exp_time_valid &&
      private->manual.is_gain_valid) {
    /* Fully manual */
    *out_sensor_gain = *out_real_gain = private->manual.gain;
    *out_linecount = (1.0 * private->manual.exp_time *
                      private->sensor_info.pixel_clock /
                      private->sensor_info.pixel_clock_per_line);
    *out_exp_time = private->manual.exp_time;
  } else if (private->manual.is_exp_time_valid &&
             !private->manual.is_gain_valid) {
    /* Manual exposure time, calculate gain using sensitivity*/
    *out_linecount = (1.0 * private->manual.exp_time *
                      private->sensor_info.pixel_clock /
                      private->sensor_info.pixel_clock_per_line);
    *out_sensor_gain = *out_real_gain =
      (real_gain * linecount) / *out_linecount;
    *out_exp_time = private->manual.exp_time;
  } else if (!private->manual.is_exp_time_valid &&
              private->manual.is_gain_valid) {
    /* Manual gain/iso, calculate linecount using sensitivity*/
    *out_sensor_gain = *out_real_gain = private->manual.gain;
    *out_linecount = (real_gain * linecount)/ *out_real_gain;
    *out_exp_time =  (real_gain * exp_time) / *out_real_gain;
  }
  if (*out_linecount < 1) {
    *out_linecount = 1;
  }

  return TRUE;
}

/** aec_port_create_msg:
 *    @msg_type:   Type to be set by the msg
 *    @param_type: Parameter type
 *    @private: Contains port private structure containing common info.
 *
 * Create msg intended to be queue to the algorithm thread. Allocate memory,
 *  assign a type and set common parameters to the header of the msg.
 *
 * Return: Address of the allocated msg
 **/
q3a_thread_aecawb_msg_t* aec_port_create_msg(
  q3a_thread_aecawb_msg_type_t msg_type,
  int param_type, aec_port_private_t *private)
{
  q3a_thread_aecawb_msg_t *aec_msg = (q3a_thread_aecawb_msg_t *)
    malloc(sizeof(q3a_thread_aecawb_msg_t));

  if (aec_msg == NULL) {
    return NULL;
  }
  STATS_MEMSET(aec_msg, 0 , sizeof(q3a_thread_aecawb_msg_t));

  aec_msg->type = msg_type;
  aec_msg->camera_id = private->camera_id;
  if (msg_type == MSG_AEC_SET || msg_type == MSG_AEC_SEND_EVENT) {
    aec_msg->u.aec_set_parm.type = param_type;
    aec_msg->u.aec_set_parm.camera_id = private->camera_id;
  } else if (msg_type == MSG_AEC_GET) {
    aec_msg->u.aec_get_parm.type = param_type;
    aec_msg->u.aec_get_parm.camera_id = private->camera_id;
  }
  return aec_msg;
}

boolean aec_port_dummy_set_param(aec_set_parameter_t *param,
  aec_output_data_t *output, uint8_t num_out, void *aec_obj)
{
  (void)param;
  (void)output;
  (void)aec_obj;
  (void)num_out;
  AEC_ERR("Error: Uninitialized interface been use");
  return FALSE;
}

boolean aec_port_dummy_get_param(aec_get_parameter_t *param,
  void *aec_obj)
{
  (void)param;
  (void)aec_obj;
  AEC_ERR("Error: Uninitialized interface been use");
  return FALSE;
}

boolean aec_port_dummy_process(stats_t *stats, void *aec_obj,
  aec_output_data_t *output, uint8_t num_out)
{
  (void)stats;
  (void)aec_obj;
  (void)output;
  (void)num_out;
  AEC_ERR("Error: Uninitialized interface been use");
  return FALSE;
}
void *aec_port_dummy_init(void *lib)
{
  (void)lib;
  AEC_ERR("Error: Uninitialized interface been use");
  return NULL;
}

void aec_port_dummy_destroy(void *aec)
{
  (void)aec;
  AEC_ERR("Error: Uninitialized interface been use");
  return;
}

float aec_port_dummy_map_iso_to_real_gain(void *aec_obj, uint32_t iso,
  uint8_t camera_id)
{
  (void)aec_obj;
  (void)iso;
  (void)camera_id;
  AEC_ERR("Error: Uninitialized interface been use");
  return 0; /* Returning gain equal to zero as an error */
}

/* This function prepares the input containing the XML keys to retrieve the
 * data from XML. Retrieved fixed settings are stored in internal structure
 * and it will be used to update the AEC event.*/
static void aec_port_load_fixed_settings_from_xml(
  aec_port_private_t *private) {

  char *nodeName = "AEC";
  const uint32 aec_max_fixed_settings = 10;
  q3a_port_xml_values_t xml[aec_max_fixed_settings];

  /* Add the xml key and initialized to invalid value.*/
  xml[0].key = "LuxIndex";
  xml[0].value = &private->fixed_settings.LuxIndex;
  private->fixed_settings.LuxIndex = Q3A_FIXED_INVALID_VALUE;

  xml[1].key = "Linecount";
  xml[1].value = &private->fixed_settings.Linecount;
  private->fixed_settings.Linecount = Q3A_FIXED_INVALID_VALUE;

  xml[2].key = "RealGain";
  xml[2].value = &private->fixed_settings.RealGain;
  private->fixed_settings.RealGain = Q3A_FIXED_INVALID_VALUE;

  xml[3].key = "SensorGain";
  xml[3].value = &private->fixed_settings.SensorGain;
  private->fixed_settings.SensorGain = Q3A_FIXED_INVALID_VALUE;

  xml[4].key = "DRCTotalGain";
  xml[4].value = &private->fixed_settings.DRCTotalGain;
  private->fixed_settings.DRCTotalGain = Q3A_FIXED_INVALID_VALUE;

  xml[5].key = "DRCColorGain";
  xml[5].value = &private->fixed_settings.DRCColorGain;
  private->fixed_settings.DRCColorGain = Q3A_FIXED_INVALID_VALUE;

  xml[6].key = "DRCRatioGTM";
  xml[6].value = &private->fixed_settings.DRCRatioGTM;
  private->fixed_settings.DRCRatioGTM = Q3A_FIXED_INVALID_VALUE;

  xml[7].key = "DRCRatioLTM";
  xml[7].value = &private->fixed_settings.DRCRatioLTM;
  private->fixed_settings.DRCRatioLTM = Q3A_FIXED_INVALID_VALUE;

  xml[8].key = "DRCRatioLA";
  xml[8].value = &private->fixed_settings.DRCRatioLA;
  private->fixed_settings.DRCRatioLA = Q3A_FIXED_INVALID_VALUE;

  xml[9].key = "DRCRatioGamma";
  xml[9].value = &private->fixed_settings.DRCRatioGamma;
  private->fixed_settings.DRCRatioGamma = Q3A_FIXED_INVALID_VALUE;

  /* Get the values from xml */
  private->fixed_settings.fixed_settings_enable = q3a_port_load_values_from_xml(
    nodeName, xml, aec_max_fixed_settings);

  AEC_HIGH("AEC fixed value setting enabled = %d",
    private->fixed_settings.fixed_settings_enable);
}

/* This function updates the AEC event with fixed settings.*/
static void aec_port_update_fixed_settings_in_output(
  aec_port_private_t *private, aec_output_data_t *output) {

  if (private && output) {
    aec_update_t *aec_output = &output->stats_update.aec_update;

    /* Update only valid values */
    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.LuxIndex) {
      aec_output->lux_idx = private->fixed_settings.LuxIndex;
      aec_output->exp_index = (int32)private->fixed_settings.LuxIndex;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.Linecount) {
      aec_output->linecount = (uint32)private->fixed_settings.Linecount;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.RealGain) {
      aec_output->real_gain = private->fixed_settings.RealGain;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.SensorGain) {
      aec_output->sensor_gain = private->fixed_settings.SensorGain;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.DRCTotalGain) {
      aec_output->total_drc_gain = private->fixed_settings.DRCTotalGain;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.DRCColorGain) {
      aec_output->color_drc_gain = private->fixed_settings.DRCColorGain;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.DRCRatioGTM) {
      aec_output->gtm_ratio = private->fixed_settings.DRCRatioGTM;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.DRCRatioLTM) {
      aec_output->ltm_ratio = private->fixed_settings.DRCRatioLTM;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.DRCRatioLA) {
     aec_output->la_ratio = private->fixed_settings.DRCRatioLA;
    }

    if (Q3A_FIXED_INVALID_VALUE != private->fixed_settings.DRCRatioGamma) {
      aec_output->gamma_ratio = private->fixed_settings.DRCRatioGamma;
    }

    /* Always set the below values */
    aec_output->settled = TRUE;
  }
}

static boolean aec_port_get_led_fd_enable(aec_port_private_t *private)
{
  boolean fd_enable = FALSE;
  /* led FD feature is enabled when UI FD is enabled and led FD chromatix parameter is TRUE */
  if (private->led_fd_settings.is_led_fd_enable_from_tuning) {
    fd_enable = TRUE;
  } else {
    fd_enable = FALSE;
  }
  return fd_enable;
}

/** aec_port_wait_for_led_fd_info:
 *  @private:         Passing port private data
 *  @return:
 *  wait_for_fd:      Returning wait status in getting fd roi.
 *                    FALSE: led fd feature is disabled or
 *                           got fd roi
 *                    TRUE: Waiting to get FD roi.
 *  @description:
 *  This function checks led fd feature is enabled/disabled.
 *  If enbale: Check led_fd_use_delay and wait for MIN(tuned frame count, capability.max_frames)
 *             to get fd roi.
 *             Send valid fd roi to core using AEC_SET_PARAM_LED_FD_COMPLETED independent of
 *             led_fd_use_delay flag status.
 *             led_fd_use_delay will be Set(low light)/reset(bright light) by core based on
 *             lighting conditions.
 *    disable: no need to wait. return false.
 **/

static boolean aec_port_wait_for_led_fd_info(aec_port_private_t *private)
{
  boolean wait_for_fd = FALSE;
  boolean send_roi_flag = FALSE;
  aec_led_fd_settings_t *led_fd_settings = &private->led_fd_settings;

  /* If led FD feature is disabled return FALSE */
  if (FALSE == aec_port_get_led_fd_enable(private)) {
    return wait_for_fd;
  }
  /*led_fd_use_delay: Core sets this Flag to TRUE only when FD in flash off case is not reliable
                    and FD should detect the faces after preflash, else it will be false.
                    False indicates, no need to wait for FD to detect further ROI, send the
                    present ROI to core.*/
  if (led_fd_settings->led_fd_use_delay) {
    /* Change the state to Est for FD and start waiting for ROI */
    private->est_state = AEC_EST_DONE_FOR_FD;
    /* handling fd info is not yet done so stay in same est state */
    wait_for_fd = TRUE;
    if (led_fd_settings->led_fd_cur_frame_count == 0) {
      /* Reset the current ROI to ensure that we are always updated with latest ROI in LED */
      memset(&private->fd_roi, 0, sizeof(aec_proc_roi_info_t));
    } else {
      /* If max count is reached, send the current ROI info as it is to core */
        if (led_fd_settings->led_fd_cur_frame_count >=
          MIN(led_fd_settings->led_fd_tuned_frame_count, led_fd_settings->led_fd_capability.max_frames)) {
          send_roi_flag = TRUE;
        }
    }
    /* Update the frame count */
    led_fd_settings->led_fd_cur_frame_count++;
  } else {
    /* Send the current ROI. No need to change the state */
    send_roi_flag = TRUE;
  }

  AEC_LOW("LED FD: SOF=%d, led_fd_use_delay=%d, estSt=%d, roi_count=%d, frame_count=%d,"
    " led_fd_tuned_frame_count=%d, led_fd_avg_frames=%d, led_fd_max_frames=%d send_roi_flag=%d,",
    private->cur_sof_id, led_fd_settings->led_fd_use_delay, private->est_state,
    private->fd_roi.num_roi,
    led_fd_settings->led_fd_cur_frame_count,
    led_fd_settings->led_fd_tuned_frame_count,
    led_fd_settings->led_fd_capability.avg_frames,
    led_fd_settings->led_fd_capability.max_frames,
    send_roi_flag);

  if (send_roi_flag) {
    q3a_thread_aecawb_msg_t *aec_msg = NULL;
    aec_msg = aec_port_create_msg(MSG_AEC_SET, AEC_SET_PARAM_LED_FD_COMPLETED,
      private);

    if (aec_msg) {
      /* Copy original face coordinates, do the transform in aec_biz.c */
      memcpy(&aec_msg->u.aec_set_parm.u.fd_roi, &private->fd_roi, sizeof(aec_proc_roi_info_t));
      if (FALSE == q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg)) {
        AEC_ERR(" FD ROI enqueue msg Failed! msg: %p", aec_msg);
      }
    } else {
      AEC_ERR(" Port create msg Failed! msg: %p", aec_msg);
    }
    led_fd_settings->led_fd_cur_frame_count = 0;
    send_roi_flag = FALSE;
    wait_for_fd = FALSE;
  }

  return wait_for_fd;
}

static void aec_port_update_aec_flash_state(
  mct_port_t *port, aec_output_data_t *output) {

  if (output && port) {
    aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
    aec_led_est_state_t tmp_state = private->est_state;

    /* HAL uses flash_needed flag to determine if prepare snapshot
     * will be called, hence AEC will turn LED on when the call comes
     * As a result, AEC_EST_NO_LED_DONE is not used (still kept below) */
    if (output->stats_update.aec_update.led_state == Q3A_LED_OFF &&
      ((private->est_state == AEC_EST_START) || (private->est_state == AEC_EST_DONE_FOR_FD))) {
    /* send AEC_SET_PARAM_LED_FD_COMPLETED to core if led fd feature is enabled and
     * go to AEC_EST_DONE_FOR_AF state in low light
     * or AEC_EST_DONE state in bright light */
      if (FALSE == aec_port_wait_for_led_fd_info(private)) {
        if (private->aec_precap_for_af == TRUE) {
          private->est_state = AEC_EST_DONE_FOR_AF;
          private->aec_precap_for_af = FALSE;
        } else {
          private->est_state = AEC_EST_DONE;
        }
      }
      private->aec_precap_start = FALSE;
    } else if (output->stats_update.aec_update.led_state == Q3A_LED_LOW) {
      private->est_state = AEC_EST_START;
      if (private->aec_precap_for_af != TRUE) {
        private->aec_precap_start = TRUE;
      }
    } else if (output->stats_update.aec_update.prep_snap_no_led == TRUE) {
      /* If core sends status as, no led needed then reset the AF flag. */
      private->est_state = AEC_EST_NO_LED_DONE;
      private->aec_precap_for_af = FALSE;
      private->aec_precap_start = FALSE;
    } else if (output->force_prep_snap_done) {
      private->force_prep_snap_done = TRUE;
    } else {
      private->est_state = AEC_EST_OFF;
    }

    /* No LED CASE, reset the precapture flag */
    if (output->stats_update.aec_update.led_state == Q3A_LED_OFF &&
        private->est_state == AEC_EST_OFF &&
        output->stats_update.aec_update.flash_needed == FALSE) {
      private->aec_precap_start = FALSE;
    }

    if (tmp_state != private->est_state) {
      AEC_LOW("AEC EST state change: Old=%d New=%d", tmp_state, private->est_state);
    }

    output->stats_update.aec_update.est_state = private->est_state;
  }
}

static boolean aec_port_is_aec_locked(aec_port_private_t *private)
{
  if(private->locked_from_hal || private->locked_from_algo) {
    return TRUE;
  }

  return FALSE;
}

/** aec_port_print_log
 *
 **/
void aec_port_print_log(aec_output_data_t *output, char *event_name,
  aec_port_private_t *private, int8 output_index)
{
  if (output) {
    aec_update_t *aec = &output->stats_update.aec_update;
    AEC_HIGH("AEUPD: %10s:cam-role=%d,cam-mode:%d,SOF=%03d,Stat=%03d,outSOF=%03d,outFrameID=%03d,outInd=%d,"
      "capt=%d,Update=%d,TL=%03d,CL=%03d,"
      "EI=%03d,LI=%03.3f, Gains: SG:RG:LC=(%02.3f:%02.3f:%04d), ET=%01.3f,settled=%d,"
      "HDR(Long:G=%02.3f,LC=%04d,Short:G=%02.3f,LC=%04d),"
      "estSt=%d,aecSt=%d,ledSt=%d,preCap=%d,flashSnap=%d,UseFDdelay=%d,"
      "LLS=%d,LLC=%d,flashMode=%d,gamma=%d,nr=%d, "
      "DRC_Gains: (TG:CG=%02.3f:%02.3f), DRC_Ratios(%1.2f,%1.2f,%1.2f,%1.2f)"
      "IR(M:%d B%%:%03.3f)",
      event_name, private->dual_cam_info.cam_role, private->dual_cam_info.mode,
      private->cur_sof_id, private->cur_stats_id, aec->sof_id,
      aec->frame_id, output_index, private->still.capture_sof, private->aec_update_flag,
      aec->target_luma,aec->cur_luma, aec->exp_index, aec->lux_idx, aec->sensor_gain,
      aec->real_gain, aec->linecount, aec->exp_time, aec->settled, aec->l_real_gain,
      aec->l_linecount, aec->s_real_gain, aec->s_linecount, aec->est_state, private->aec_state,
      aec->led_state, private->aec_precap_start, output->snap.is_flash_snapshot,
      private->led_fd_settings.led_fd_use_delay,
      private->low_light_shutter_flag, aec->low_light_capture_update_flag, aec->flash_hal,
      aec->gamma_flag, aec->nr_flag, aec->total_drc_gain, aec->color_drc_gain, aec->gtm_ratio,
      aec->ltm_ratio, aec->la_ratio, aec->gamma_ratio,
      aec->ir_config.ir_needed, aec->ir_config.ir_led_brightness);
  }
}

/** aec_port_print_bus
 *
 */
static inline void aec_port_print_bus(const char* trigger, aec_port_private_t *private)
{
  /* Print all bus info in one message */
  AEC_LOW("BSUPD:%15s SOF=%d,cam-role=%d,cam-mode:%d,streamTy=%d,zsl_cap=%d,"
    "AEINFO:ET=%f,RG=%f iso=%d,flashNeed=%d,settled=%d,bright=%f,metering: %d,"
    "IMME:aeSt=%d,tr=%d,trId=%d,ledM=%d,onOffM=%d,"
    "AEC:expCm=%d,lock=%d,speed=%f", trigger,
    private->cur_sof_id, private->dual_cam_info.cam_role, private->dual_cam_info.mode,
    private->stream_type,private->in_zsl_capture,// Other info
    private->aec_info.exp_time, private->aec_info.real_gain, private->aec_info.iso_value,
    private->aec_info.flash_needed, private->aec_info.settled, private->aec_info.brightness,
    private->aec_info.metering_mode, // AE_INFO end
    private->aec_state, private->aec_trigger.trigger,
    private->aec_trigger.trigger_id, private->led_mode,
    private->aec_on_off_mode,// IMM end
    private->exp_comp, aec_port_is_aec_locked(private),
    private->conv_speed // AE end
    );
}

/** aec_port_print_manual
 *
 **/
static inline void aec_port_print_manual(aec_port_private_t *private,
  char *event_name, aec_manual_update_t *output)
{
  AEC_HIGH("AEUPD: %15s: SOF=%03d,update=%d,G=%02.3f,LC=%04d,LI=%03.3f,iso=%d",
    event_name, private->cur_sof_id, private->aec_update_flag,
    output->sensor_gain, output->linecount, output->lux_idx,
    output->exif_iso);
}

/**
 * aec_port_load_dummy_default_func
 *
 * @aec_object: structure with function pointers to be assign
 *
 * Return: TRUE on success
 **/
boolean aec_port_load_dummy_default_func(aec_object_t *aec_object)
{
  boolean rc = FALSE;
  if (aec_object) {
    aec_object->set_parameters = aec_port_dummy_set_param;
    aec_object->get_parameters = aec_port_dummy_get_param;
    aec_object->process = aec_port_dummy_process;
    aec_object->init = aec_port_dummy_init;
    aec_object->deinit = aec_port_dummy_destroy;
    aec_object->iso_to_real_gain = aec_port_dummy_map_iso_to_real_gain;
    rc = TRUE;
  }
  return rc;
}

/** aec_port_load_function
 *
 *    @aec_object: structure with function pointers to be assign
 *
 * Return: Handler to AEC interface library
 **/
void* aec_port_load_function(aec_object_t *aec_object)
{
  if (!aec_object) {
    return FALSE;
  }

  return aec_biz_load_function(aec_object);
}


/** aec_port_unload_function
 *
 *    @private: Port private structure
 *
 *  Free resources allocated by aec_port_load_function
 *
 * Return: void
 **/
void aec_port_unload_function(aec_port_private_t *private)
{
  if (!private) {
    return;
  }

  aec_biz_unload_function(&private->aec_object, private->aec_iface_lib);
  aec_port_load_dummy_default_func(&private->aec_object);
  private->aec_iface_lib = NULL;
}

/** aec_port_query_capabilities:
 *    @port: aec's port
 *
 *  Provide session data information for algo library set-up.
 **/
boolean aec_port_query_capabilities(mct_port_t *port,
  mct_pipeline_stats_cap_t *stats_cap)
{
  aec_port_private_t *private = NULL;
  Q3a_version_t aec_version;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "aec_sink")) {
    return FALSE;
  }
  private = port->port_private;
  if (!private) {
    return FALSE;
  }
  if (NULL == private->aec_object.get_version) {
    /* This is an optional function, not an error */
    AEC_ERR("Get AEC version function not implemented");
    return TRUE;
  }

   /* For now, dictate the overall 3A version base on AEC algo version*/
  /* TODO: Add version for every algorithm */
  if (!private->aec_object.get_version(private->aec_object.aec, &aec_version,
      private->camera_id)){
    AEC_ERR("Fail to get AEC version");
    return FALSE;
  }
  stats_cap->q3a_version.major_version =
    aec_version.major_version;
  stats_cap->q3a_version.minor_version =
    aec_version.minor_version;
  stats_cap->q3a_version.patch_version =
    aec_version.patch_version;
  stats_cap->q3a_version.new_feature_des =
    aec_version.new_feature_des;

  return TRUE;
}

/** aec_port_set_session_data:
 *    @port: aec's sink port to be initialized
 *    @q3a_lib_info: Q3A session data information
 *    @sessionid: session identity
 *
 *  Provide session data information for algo library set-up.
 **/
boolean aec_port_set_session_data(mct_port_t *port, void *q3a_lib_info,
  mct_pipeline_session_data_t *session_data, unsigned int *sessionid)
{
  aec_port_private_t *private = NULL;
  mct_pipeline_session_data_q3a_t *q3a_session_data = NULL;
  Q3a_version_t aec_version;
  boolean rc = FALSE;
  unsigned int session_id = GET_SESSION_ID(*sessionid);
  if (!port || !port->port_private || strcmp(MCT_OBJECT_NAME(port), "aec_sink") ||
    !q3a_lib_info) {
    return rc;
  }

  q3a_session_data = (mct_pipeline_session_data_q3a_t *)q3a_lib_info;

  AEC_HIGH("aec_libptr %p session_id %d lens_type %d",
    q3a_session_data->aec_libptr, session_id, session_data->lens_type);

  private = port->port_private;

  /* Read delay information from pipeline session data */
  private->max_pipeline_delay =
    session_data->max_pipeline_frame_applying_delay + session_data->max_pipeline_meta_reporting_delay;
  AEC_HIGH("Max Pipeline delay %d",private->max_pipeline_delay);

  /* Query to verify if extension use is required and if using default algo */
  private->aec_extension_use =
    aec_port_ext_is_extension_required(q3a_session_data->aec_libptr,
      session_data->position, &private->use_default_algo);
  if (FALSE == private->aec_extension_use) {
    AEC_HIGH("Load AEC interface functions");
    private->aec_iface_lib = aec_port_load_function(&private->aec_object);
  } else { /* Use extension */
    AEC_HIGH("Load AEC EXTENSION interface functions");
    private->aec_iface_lib = aec_port_ext_load_function(&private->aec_object,
      q3a_session_data->aec_libptr, session_data->position, private->use_default_algo);
  }

  /* Verify that all basic fields were populated */
  if (!(private->aec_iface_lib && private->aec_object.init &&
    private->aec_object.deinit &&
    private->aec_object.set_parameters &&
    private->aec_object.get_parameters &&
    private->aec_object.process &&
    private->aec_object.get_version)) {
    AEC_ERR("Error: setting algo iface functions");
    /* Resetting default interface to clear things */
    if (FALSE == private->aec_extension_use) {
      aec_port_unload_function(private);
    } else {
      aec_port_ext_unload_function(private);
    }
    return FALSE;
  }

  private->aec_object.aec =
    private->aec_object.init(private->aec_iface_lib);
  rc = private->aec_object.aec ? TRUE : FALSE;
  if (FALSE == rc) {
    AEC_ERR("Error: fail to init AEC algo");
    return rc;
  }

  AEC_LOW("AEC OTP data: [%d %d %d %d %d %d]",
      session_data->dual_data.brightness_ratio,
      session_data->dual_data.ref_aux_linecount,
      session_data->dual_data.ref_aux_gain,
      session_data->dual_data.ref_master_linecount,
      session_data->dual_data.ref_master_gain,
      session_data->dual_data.ref_master_color_temperature);

  /* It's possible that the peer has already passed its own OTP data to this session */
  if (private->dual_cam_info.otp_data_valid) {
    AEC_LOW("AECDualCam: Valid OTP data is already present - skipping");
  } else if (session_data->is_valid_dualcalib &&
             session_data->dual_data.ref_aux_linecount > 0 &&
             session_data->dual_data.ref_master_linecount > 0) {
    uint16_t slave_linecount    = session_data->dual_data.ref_aux_linecount;
    uint16_t master_linecount   = session_data->dual_data.ref_master_linecount;
    float    slave_gain         = (float)session_data->dual_data.ref_aux_gain / (float)Q10;
    float    master_gain        = (float)session_data->dual_data.ref_master_gain / (float)Q10;
    float    brightness_ratio   = (float)session_data->dual_data.brightness_ratio / (float)Q10;
    float    slave_sensitivity  = ((float)slave_linecount * slave_gain);
    float    master_sensitivity = ((float)master_linecount * master_gain);

    if ((master_sensitivity * brightness_ratio) > 0.0f) {
      private->dual_cam_info.exp_multiplier_otp =
        slave_sensitivity / (master_sensitivity * brightness_ratio);
      private->dual_cam_info.tuned_color_temp = session_data->dual_data.ref_master_color_temperature;
      AEC_LOW("AECDualCam: AEC OTP calibration data present. Exposure multiplier: %f", private->dual_cam_info.exp_multiplier_otp);
      private->dual_cam_info.otp_data_valid = TRUE;
    }
  }
  if (!private->dual_cam_info.otp_data_valid) {
    private->dual_cam_info.tuned_color_temp = 6500;
    /* This is not a hard failure - not all sensors implement this */
    AEC_LOW("AECDualCam: AEC OTP calibration data is not present");
  }

  /* Save lens type, to be provided to algo later */
  rc = q3a_port_map_sensor_format(&private->sensor_info.sensor_type,
    session_data->sensor_format);
  if (!rc) {
    AEC_ERR("Fail to map sensor_format");
    return rc;
  }
  rc = q3a_port_map_lens_type(&private->sensor_info.lens_type,
    session_data->lens_type);
  if (!rc) {
    AEC_ERR("Fail to map lens_format");
    return rc;
  }

  if (private->aec_extension_use) {
    rc = aec_port_ext_update_func_table(private);
    if (rc && private->func_tbl.ext_init) {
      stats_ext_return_type ret = STATS_EXT_HANDLING_FAILURE;
      ret = private->func_tbl.ext_init(port, session_id);
      if (ret != STATS_EXT_HANDLING_FAILURE) {
        rc = TRUE;
      }
    }
  }

  /* Print 3A version */
  rc = private->aec_object.get_version(private->aec_object.aec, &aec_version,
    private->camera_id);
  if (rc) {
    AEC_HIGH("3A VERSION --> %d.%d.%d.r%d",
      aec_version.major_version,
      aec_version.minor_version,
      aec_version.patch_version,
      aec_version.new_feature_des);
  } else {
    AEC_ERR("Fail to get_version");
  }

  AEC_HIGH("aec = %p", private->aec_object.aec);
  return rc;
}

/** aec_set_skip_stats:
 *    @private: internal port structure
 *    @skip_stats_start: Stat id to start skip logic
 *    @skip_count: Number of stats to skip
 *
 * Setup all skip stats related variables
 *
 * Return: nothing
 **/
static void aec_set_skip_stats(aec_port_private_t *private,
  uint32_t skip_stats_start, uint8_t skip_count)
{
  AEC_LOW("start: %d,skip_count: %d", skip_stats_start, skip_count);
  private->aec_skip.skip_stats_start = skip_stats_start;
  private->aec_skip.skip_count = skip_count;
}

/** aec_set_skip_stats:
 *    @private: internal port structure
 *
 * Verify if stats skip is required.
 *
 * Return: TRUE if AEC stat skip is required
 **/
static boolean is_aec_stats_skip_required(aec_port_private_t *private,
  uint32 stats_id)
{
  boolean rc = FALSE;
  aec_skip_t *skip = &private->aec_skip;
  AEC_LOW("sof: %d, start: %d, skip: %d, stats_id: %d",
    private->cur_sof_id, skip->skip_stats_start, skip->skip_count, stats_id);

  /* if SOF is not recived yet skip stats */
  if (stats_id > private->cur_sof_id) {
    rc = TRUE;
  } else if (skip->skip_count && stats_id >= skip->skip_stats_start) {
    skip->skip_count--;
    rc = TRUE;
  } else if (private->aec_auto_mode == AEC_MANUAL) /* skip stats in Manual mode */
    rc = TRUE;

  /* if it is in fast aec mode and AWB is runing, skip sending the stats to aec core. */
  if(private->fast_aec_data.enable &&
    private->fast_aec_data.state == Q3A_FAST_AEC_STATE_AWB_RUNNING)
    rc = TRUE;

  return rc;
}

static boolean aec_port_is_aec_searching(aec_output_data_t *output)
{
  if(output->stats_update.aec_update.settled) {
    return FALSE;
  }
  return TRUE;
}

static boolean aec_port_is_precapture_start(aec_port_private_t *private)
{
  if(private->aec_precap_start == TRUE) {
    return TRUE;
  }

  return FALSE;
}

static boolean aec_port_is_converged(aec_output_data_t *output,
  boolean *low_light)
{
  if(output->stats_update.aec_update.settled) {
    if(!output->stats_update.aec_update.flash_needed) {
      *low_light = FALSE;
    } else {
      *low_light = TRUE;
    }
    return TRUE;
  }

  return FALSE;
}

void aec_send_bus_message(mct_port_t *port,
  mct_bus_msg_type_t bus_msg_type, void* payload, int size, int sof_id)
{
  aec_port_private_t *aec_port = (aec_port_private_t *)(port->port_private);
  mct_event_t        event;
  mct_bus_msg_t      bus_msg;
  mct_event_t        slave_event;

  /* Slave will not send its own bus messages except for IMMEDIATE  */
  if(!aec_port_send_bus_message_enabled(aec_port,bus_msg_type)) {
    return;
  }

  STATS_MEMSET(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = GET_SESSION_ID(aec_port->reserved_id);
  bus_msg.type = bus_msg_type;
  bus_msg.msg = payload;
  bus_msg.size = size;

  /* pack into an mct_event object*/
  memset(&event, 0, sizeof(mct_event_t));
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = aec_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.current_frame_id = sof_id;
  event.u.module_event.module_event_data = (void *)(&bus_msg);

  /* Save a copy in case slave needs it. Certain events are errnously modified
     by other components in the stream after they are sent. */
  slave_event = event;
  MCT_PORT_EVENT_FUNC(port)(port, &event);
  aec_port_forward_bus_message_if_linked(port,slave_event);
  return;
}

/*
 * common utility function to send module event
 */
static boolean aec_port_send_module_event(mct_port_t *port,
  mct_event_module_type_t module_event_type, void* event_data)
{
  aec_port_private_t *aec_port = (aec_port_private_t *)(port->port_private);
  mct_event_t        event;
  STATS_MEMSET(&event, 0, sizeof(mct_event_t));

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = aec_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = module_event_type;
  event.u.module_event.current_frame_id = aec_port->cur_sof_id;
  event.u.module_event.module_event_data = event_data;

  MCT_PORT_EVENT_FUNC(port)(port, &event);

  return TRUE;
}

/*
 * Get the max or average number of frames from fd library.
 * max or avg frames will be used to wait fd roi from fd lib during face AEC
 *
 */
static boolean aec_port_get_ledfd_fd_capability(mct_port_t *port)
{
  boolean rc = TRUE;
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
  faceproc_capbility_aec_t  *fd_capability = &private->led_fd_settings.led_fd_capability;

  rc = aec_port_send_module_event(port,
    MCT_EVENT_MODULE_QUERY_LEDFD_FD_CAPABILITY, (void *)fd_capability);

  AEC_LOW("led fd: fd capability avg.frames=%d max.frames=%d",
    fd_capability->avg_frames, fd_capability->max_frames);

  return rc;
}

/*
 * Get the max supported current for flash from sensor which will
 * get the info from the PMIC module.
 *
 * If the feature is disabled in PMIC, the sensor module will return
 * full current.
 *
 * And the call to sensor won't fail.
 *
 */
static uint32 aec_port_get_max_flash_current(mct_port_t *port)
{
  int32 max_current = 0;

  boolean rc = aec_port_send_module_event(port,
    MCT_EVENT_MODULE_QUERY_CURRENT, (void *)&max_current);

  AEC_LOW("max flash current %d", max_current);
  if (max_current < MIN_FLASH_CURRENT) {
    max_current = MIN_FLASH_CURRENT;
    AEC_LOW("set minimum flash current to %d", max_current);
  }
  return max_current;
}

/*
 * Entry point for the derating feature(available from chromatix 310).
 * The function will get the max current information from sensor and
 * send it to the algorithm.
 *
 */
static void aec_port_send_max_flash_current_to_algo(mct_port_t *port)
{
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x0310)
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  uint32_t max_current = aec_port_get_max_flash_current(port);

  q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
    AEC_SET_PARAM_MAX_FLASH_CURRENT, private);

  if (aec_msg == NULL) {
    AEC_ERR("Failed to create AEC_SET_PARAM_MAX_FLASH_CURRENT aec msg");
    return;
  }

  aec_msg->u.aec_set_parm.u.max_flash_current = max_current;
  aec_msg->sync_flag = TRUE;

  boolean rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);

  if (!rc) {
    AEC_ERR("Failed to enque msg");
  }
  free(aec_msg);
#else
  (void *)port;
#endif
  return;
}

/** aec_port_set_aec_mode:
 *  @aec_meta_mode: OFF/AUTO/SCENE_MODE- Main 3a switch
 *  @aec_on_off_mode: AEC OFF/ON switch
 **/
static void aec_port_set_aec_mode(aec_port_private_t *private) {
  uint8_t aec_meta_mode = private->aec_meta_mode;
  uint8_t aec_on_off_mode = private->aec_on_off_mode;
  aec_auto_mode_t prev_aec_auto_mode = private->aec_auto_mode;
  switch(aec_meta_mode){
    case CAM_CONTROL_OFF:
      private->aec_auto_mode = AEC_MANUAL;
      break;
    case CAM_CONTROL_AUTO:
      if(aec_on_off_mode)
        private->aec_auto_mode = AEC_AUTO;
      else
        private->aec_auto_mode = AEC_MANUAL;
      break;
    case CAM_CONTROL_USE_SCENE_MODE:
      private->aec_auto_mode = AEC_AUTO;
      break;
    default:{
      private->aec_auto_mode = AEC_AUTO;
    }
  }
  if (prev_aec_auto_mode != private->aec_auto_mode) {
    aec_state_update_data_t state_update;
    STATS_MEMSET(&state_update, 0, sizeof(state_update));
    state_update.type = AEC_PORT_STATE_MODE_UPDATE;
    state_update.u.trigger_new_mode = TRUE;
    aec_port_update_aec_state(private, &state_update);
  }
}

/** aec_port_set_bestshot_mode:
 *    @aec_mode:     scene mode to be set
 *    @mode: scene mode coming from HAL
 *
 * Set the bestshot mode for algo.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean aec_port_set_bestshot_mode(
  aec_bestshot_mode_type_t *aec_mode, cam_scene_mode_type mode)
{
  boolean rc = TRUE;
  *aec_mode = AEC_BESTSHOT_OFF;
  AEC_LOW("Set scene mode: %d", mode);

  /* We need to translate Android scene mode to the one
   * AEC algorithm understands.
   **/
  switch (mode) {
  case CAM_SCENE_MODE_OFF: {
    *aec_mode = AEC_BESTSHOT_OFF;
  }
    break;

  case CAM_SCENE_MODE_AUTO: {
    *aec_mode = AEC_BESTSHOT_AUTO;
  }
    break;

  case CAM_SCENE_MODE_LANDSCAPE: {
    *aec_mode = AEC_BESTSHOT_LANDSCAPE;
  }
    break;

  case CAM_SCENE_MODE_SNOW: {
    *aec_mode = AEC_BESTSHOT_SNOW;
  }
    break;

  case CAM_SCENE_MODE_BEACH: {
    *aec_mode = AEC_BESTSHOT_BEACH;
  }
    break;

  case CAM_SCENE_MODE_SUNSET: {
    *aec_mode = AEC_BESTSHOT_SUNSET;
  }
    break;

  case CAM_SCENE_MODE_NIGHT: {
    *aec_mode = AEC_BESTSHOT_NIGHT;
  }
    break;

  case CAM_SCENE_MODE_PORTRAIT: {
    *aec_mode = AEC_BESTSHOT_PORTRAIT;
  }
    break;

  case CAM_SCENE_MODE_BACKLIGHT: {
    *aec_mode = AEC_BESTSHOT_BACKLIGHT;
  }
    break;

  case CAM_SCENE_MODE_SPORTS: {
    *aec_mode = AEC_BESTSHOT_SPORTS;
  }
    break;

  case CAM_SCENE_MODE_ANTISHAKE: {
    *aec_mode = AEC_BESTSHOT_ANTISHAKE;
  }
    break;

  case CAM_SCENE_MODE_FLOWERS: {
    *aec_mode = AEC_BESTSHOT_FLOWERS;
  }
    break;

  case CAM_SCENE_MODE_CANDLELIGHT: {
    *aec_mode = AEC_BESTSHOT_CANDLELIGHT;
  }
    break;

  case CAM_SCENE_MODE_FIREWORKS: {
    *aec_mode = AEC_BESTSHOT_FIREWORKS;
  }
    break;

  case CAM_SCENE_MODE_PARTY: {
    *aec_mode = AEC_BESTSHOT_PARTY;
  }
    break;

  case CAM_SCENE_MODE_NIGHT_PORTRAIT: {
    *aec_mode = AEC_BESTSHOT_NIGHT_PORTRAIT;
  }
    break;

  case CAM_SCENE_MODE_THEATRE: {
    *aec_mode = AEC_BESTSHOT_THEATRE;
  }
    break;

  case CAM_SCENE_MODE_ACTION: {
    *aec_mode = AEC_BESTSHOT_ACTION;
  }
    break;

  case CAM_SCENE_MODE_AR: {
    *aec_mode = AEC_BESTSHOT_AR;
  }
    break;
  case CAM_SCENE_MODE_FACE_PRIORITY: {
    *aec_mode = AEC_BESTSHOT_FACE_PRIORITY;
  }
    break;
  case CAM_SCENE_MODE_BARCODE: {
    *aec_mode = AEC_BESTSHOT_BARCODE;
  }
    break;
  case CAM_SCENE_MODE_HDR: {
    *aec_mode = AEC_BESTSHOT_HDR;
  }
    break;
  default: {
    rc = FALSE;
  }
    break;
  }

  return rc;
} /* aec_port_set_bestshot_mode */

/** aec_send_batch_bus_message:
 *    @port:   TODO
 *    @output: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
void aec_send_batch_bus_message(mct_port_t *port, uint32_t urgent_sof_id,
  uint32_t regular_sof_id)
{
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  mct_bus_msg_aec_immediate_t aec_urgent;
  mct_bus_msg_aec_t aec_bus_msg;
  cam_3a_params_t *aec_meta_info;

  aec_urgent.aec_state = aec_port_get_aec_state(private);

  /* if the aec_reset_precap_start_flag is true, we need to send the CAM_AE_STATE_PRECAPTURE
   * to HAL per HAL3 spec, but we are not in AEC preflash sequence, so after we send out the
   * CAM_AE_STATE_PRECAPTURE state to HAL, here we reset the aec_precap_start flag, so the
   * state machine could move the state to converged. otherwise, the state will be stuck at
   * CAM_AE_STATE_PRECAPTURE due to aec_precap_start flag
   */
  if (aec_urgent.aec_state == CAM_AE_STATE_PRECAPTURE && private->aec_reset_precap_start_flag) {
    private->aec_reset_precap_start_flag = false;
    private->aec_precap_start = false;
  }

  aec_urgent.aec_trigger = private->aec_trigger;
  aec_urgent.led_mode = private->led_mode;
  aec_urgent.aec_mode =  private->aec_on_off_mode;
  aec_urgent.touch_ev_status = private->touch_ev_status;
  aec_urgent.lux_index = private->lux_idx;
  aec_urgent.lcd_flash_settled = private->lcd_flash_settled;

  aec_bus_msg.exp_comp =  private->exp_comp;
  aec_bus_msg.ae_lock   = aec_port_is_aec_locked(private);
  /* HAL 3 manual mode */
  if (private->aec_auto_mode == AEC_MANUAL) {
    /* Reporting ROI weight as 0 in manual mode */
    STATS_MEMSET(&aec_bus_msg.aec_roi, 0, sizeof(aec_bus_msg.aec_roi));
    aec_bus_msg.aec_roi_valid = TRUE;
  } else {
    aec_bus_msg.aec_roi = private->aec_roi;
    aec_bus_msg.aec_roi_valid = TRUE;
  }
  aec_bus_msg.fps.min_fps = private->fps.min_fps/256.0;
  aec_bus_msg.fps.max_fps = private->fps.max_fps/256.0;
  /* TODO make similar state in HAL and AEC CORE for lls */
  /* here 4 represent LOW_LIGHT_SNAPHOT */
  aec_bus_msg.lls_flag = (private->low_light_shutter_flag == 4) ? CAM_LOW_LIGHT_ON : CAM_LOW_LIGHT_OFF;
  aec_bus_msg.conv_speed = private->conv_speed;

  /* Print all bus messages info */
  aec_port_print_bus("SOF", private);

  aec_send_bus_message(port, MCT_BUS_MSG_AE_INFO,
    &private->aec_info, sizeof(cam_3a_params_t), private->cur_sof_id);

  aec_send_bus_message(port, MCT_BUS_MSG_AEC_IMMEDIATE,
    (void*)&aec_urgent, sizeof(mct_bus_msg_aec_immediate_t), urgent_sof_id);

  aec_send_bus_message(port, MCT_BUS_MSG_AEC,
    (void*)&aec_bus_msg, sizeof(mct_bus_msg_aec_t), regular_sof_id);
}

static cam_metering_mode_t aec_port_get_hal_metering_mode(
  aec_auto_exposure_mode_t in) {
  cam_metering_mode_t out = CAM_METERING_MODE_UNKNOWN;

  switch (in) {
      default:
          out = CAM_METERING_MODE_UNKNOWN;
          break;
      case AEC_METERING_FRAME_AVERAGE:
          out = CAM_METERING_MODE_AVERAGE;
          break;
      case AEC_METERING_CENTER_WEIGHTED:
      case AEC_METERING_CENTER_WEIGHTED_ADV:
          out = CAM_METERING_MODE_CENTER_WEIGHTED_AVERAGE;
          break;
      case AEC_METERING_SPOT_METERING:
      case AEC_METERING_SPOT_METERING_ADV:
          out = CAM_METERING_MODE_SPOT;
          break;
      case AEC_METERING_SMART_METERING:
          out = CAM_METERING_MODE_MULTI_SPOT;
          break;
      case AEC_METERING_USER_METERING:
          out = CAM_METERING_MODE_PATTERN;
          break;
  }

  return out;
}

/** aec_port_pack_exif_info:
 *    @port:   TODO
 *    @output: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
void aec_port_pack_exif_info(mct_port_t *port,
  aec_output_data_t *output)
{
  aec_port_private_t *private;

  if (!output || !port) {
    AEC_ERR("input error");
    return;
  }

  private = (aec_port_private_t *)(port->port_private);
  private->aec_info.exp_time = output->stats_update.aec_update.exp_time;
  private->aec_info.real_gain = output->stats_update.aec_update.real_gain;
  private->aec_info.iso_value = output->stats_update.aec_update.exif_iso;
  private->aec_info.flash_needed = output->stats_update.aec_update.flash_needed;
  private->aec_info.settled = output->stats_update.aec_update.settled;
  private->aec_info.metering_mode =
    aec_port_get_hal_metering_mode(output->metering_type);
  private->aec_info.exposure_program = output->snap.exp_program;
  private->aec_info.exposure_mode = output->snap.exp_mode;
  if(private->aec_info.exposure_mode <= 2)
    private->aec_info.scenetype = 0x1;
  else
   private->aec_info.scenetype = 0xFFFF;
  private->aec_info.brightness = output->stats_update.aec_update.Bv;
}


/** aec_port_send_exif_debug_data:
 *    @port:   TODO
 *    @stats_update_t: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static void aec_port_send_exif_debug_data(mct_port_t *port)
{
  mct_event_t          event;
  mct_bus_msg_t        bus_msg;
  cam_ae_exif_debug_t  *aec_info;
  aec_port_private_t   *private;
  int                  size;

  if (!port) {
    AEC_ERR("input error");
    return;
  }
  private = (aec_port_private_t *)(port->port_private);
  if (private == NULL) {
    return;
  }

  /* Send exif data if data size is valid */
  if (!private->aec_debug_data_size) {
    AEC_LOW("aec_port: Debug data not available");
    return;
  }
  aec_info = (cam_ae_exif_debug_t *)malloc(sizeof(cam_ae_exif_debug_t));
  if (!aec_info) {
    AEC_ERR("Failure allocating memory for debug data");
    return;
  }

  STATS_MEMSET(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = GET_SESSION_ID(private->reserved_id);
  bus_msg.type = MCT_BUS_MSG_AE_EXIF_DEBUG_INFO;
  bus_msg.msg = (void *)aec_info;
  size = (int)sizeof(cam_ae_exif_debug_t);
  bus_msg.size = size;
  STATS_MEMSET(aec_info, 0, size);
  aec_info->aec_debug_data_size = private->aec_debug_data_size;
  aec_info->sw_version_number = (uint64_t)(((uint64_t)MAJOR_NUM & 0xFFFF) |
    (((uint64_t)MINOR_NUM & 0xFFFF) << 16) |
    (((uint64_t)PATCH_NUM & 0xFFFF) << 32) |
    (((uint64_t)FEATURE_DESIGNATOR & 0xFFFF) << 48));
  AEC_LOW("aec_debug_data_size: %d, sw version number: %llx",
   private->aec_debug_data_size, aec_info->sw_version_number);
  STATS_MEMCPY(aec_info->aec_private_debug_data, sizeof(aec_info->aec_private_debug_data),
    private->aec_debug_data_array, private->aec_debug_data_size);
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
  if (aec_info) {
    free(aec_info);
  }
}


/** aec_port_send_aec_info_to_metadata
 *  update aec info which required by eztuning
 **/

void aec_port_send_aec_info_to_metadata(mct_port_t *port,
  aec_output_data_t *output)
{
  mct_event_t        event;
  mct_bus_msg_t      bus_msg;
  aec_ez_tune_t      aec_info;
  aec_port_private_t *private;
  int                size;

  if (!output || !port) {
    AEC_ERR("input error");
    return;
  }

  private = (aec_port_private_t *)(port->port_private);
  STATS_MEMSET(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = GET_SESSION_ID(private->reserved_id);
  bus_msg.type = MCT_BUS_MSG_AE_EZTUNING_INFO;
  bus_msg.msg = (void *)&aec_info;
  size = (int)sizeof(aec_ez_tune_t);
  bus_msg.size = size;

  STATS_MEMCPY(&aec_info, sizeof(aec_ez_tune_t), &output->eztune, size);

  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);

  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

void aec_port_configure_stats(aec_output_data_t *output,
  mct_port_t *port)
{
  aec_port_private_t *private = NULL;
  mct_event_t        event;
  aec_config_t       aec_config;

  private = (aec_port_private_t *)(port->port_private);

  aec_config = output->config;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AEC_CONFIG_UPDATE;
  event.u.module_event.module_event_data = (void *)(&aec_config);

  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

/** aec_port_adjust_for_fixed_fps
 *
 **/
static void aec_port_adjust_for_fixed_fps(aec_output_data_t *output, aec_port_private_t *private)
{
  if ((NULL != private) && (NULL != output))
  {
    if ((private->fps.min_fps > 0) && (private->fps.max_fps > 0) && (private->fps.min_fps == private->fps.max_fps)) {
      float factor = 1.00f;
      float fixed_fps_frame_duration = (float)(Q8) / (float)(private->fps.min_fps);

      /* If the current exposure time exceeds the fixed fps frame duration, the current stats update
      is adjusted so that the line count is capped to avoid dropping the frame rate. Also, the gain
      is increased accordingly to maintain the sensitivity */
      if (output->stats_update.aec_update.exp_time > fixed_fps_frame_duration) {
        /* The line count corresponding to the fixed fps frame duration is computed here*/
        uint32_t line_count = ((float)fixed_fps_frame_duration / (float)output->stats_update.aec_update.exp_time) *
                              (float)(output->stats_update.aec_update.linecount);

        /* The sensor driver adds an offset (around ~20) to the line count before applying the value
        in the registers and prior to the calculation of the frame duration. This causes the frame
        duration to vary slightly from the desired value. This margin value is subtracted here to
        ensure that the frame duration is exactly at the value desired.
        IMPORTANT: This margin value has been selected conservatively based on the current sensor
        drivers being used. It needs to be updated if a new sensor driver has a bigger margin value */
        line_count = line_count - MAX_INTEGRATION_MARGIN;

        factor = (float)line_count / (float)output->stats_update.aec_update.linecount;
      }

      /* Offset the gain, line count and exposure time by this factor */
      output->stats_update.aec_update.real_gain = output->stats_update.aec_update.real_gain / factor;
      output->stats_update.aec_update.linecount = output->stats_update.aec_update.linecount * factor;
      output->stats_update.aec_update.exp_time  = fixed_fps_frame_duration;
      output->stats_update.aec_update.exif_iso  = output->stats_update.aec_update.exif_iso / factor;

      AEC_LOW("Exposure adjust for Fixed FPS: RG:%f LC:%d ET:%f ISO:%d FPS: max = %d min = %d",
        output->stats_update.aec_update.real_gain,
        output->stats_update.aec_update.linecount,
        output->stats_update.aec_update.exp_time,
        output->stats_update.aec_update.exif_iso,
        private->fps.max_fps,
        private->fps.min_fps);
    }
  }

  else
  {
    AEC_ERR("Null pointer check failed!!! output = %p private = %p", output, private);
  }
}

/** aec_port_send_event:
 *    @port:         TODO
 *    @evt_type:     TODO
 *    @sub_evt_type: TODO
 *    @data:         TODO
 *
 * TODO description
 *
 * Return nothing
 **/
void aec_port_send_event(mct_port_t *port, int evt_type,
  int sub_evt_type, void *data, uint32_t sof_id)
{
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  mct_event_t        event;

  MCT_OBJECT_LOCK(port);

  /*
   * in traditional non-ZSL mode while the stream_type is CAM_STREAM_TYPE_SNAPSHOT
   * AEC shall not send AEC update outside.
   * In HAL3, framework could configure stream_type to SNAPSHOT mode but it's not
   * non-ZSL mode, in this mode, AEC still need to sent out the update
   * That's the reason put (private->stream_type == CAM_STREAM_TYPE_SNAPSHOT &&
   * !private->still.is_capture_intent) here
   */
  if (private->aec_auto_mode == AEC_MANUAL &&
      private->aec_update_flag == TRUE) {
    private->aec_update_flag = FALSE;
  } else if (private->aec_update_flag == FALSE ||
      (private->in_zsl_capture == TRUE && private->in_longshot_mode == 0) ||
      (private->stream_type == CAM_STREAM_TYPE_SNAPSHOT &&
      !private->still.is_capture_intent)) {
    AEC_LOW("No AEC update event to send");
    MCT_OBJECT_UNLOCK(port);
    return;
  } else {
    private->aec_update_flag = FALSE;
  }
  MCT_OBJECT_UNLOCK(port);
  /* Pack into an mct_event object */
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = evt_type;
  event.u.module_event.current_frame_id = sof_id;
  event.u.module_event.type = sub_evt_type;
  event.u.module_event.module_event_data = data;

  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

/** aec_port_update_roi
 *
**/
static void aec_port_update_roi(aec_port_private_t *private,
  aec_interested_region_t roi)
{
  if (roi.enable) {
    private->aec_roi.rect.left = roi.r[0].x;
    private->aec_roi.rect.top = roi.r[0].y;
    private->aec_roi.rect.width = roi.r[0].dx;
    private->aec_roi.rect.height = roi.r[0].dy;
    private->aec_roi.weight = roi.weight;
  } else {
    private->aec_roi.rect.left = 0;
    private->aec_roi.rect.top = 0;
    private->aec_roi.rect.width = private->sensor_info.sensor_res_width;
    private->aec_roi.rect.height = private->sensor_info.sensor_res_height;
    private->aec_roi.weight = 0;
  }
}

/** aec_port_pack_update:
 *    @port:   TODO
 *    @output: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
void aec_port_pack_update(mct_port_t *port, aec_output_data_t *output,
  uint8_t aec_output_index)
{
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  aec_output_index %= AEC_OUTPUT_ARRAY_MAX_SIZE;
  output->stats_update.flag = STATS_UPDATE_AEC;

  if (output->snap.is_flash_snapshot) {
    /* Save flash snapshot data */
    private->still.is_flash_snap_data = TRUE;
    private->still.flash_sensor_gain = output->snap.sensor_gain;
    private->still.flash_real_gain = output->snap.real_gain;
    private->still.flash_line_cnt = output->snap.line_count;
    private->still.flash_lux_index = output->snap.lux_index;
    private->still.flash_exif_iso = output->snap.exif_iso;
    private->still.flash_drc_gains = output->snap.drc_gains;
    private->still.flash_exp_time = output->snap.exp_time;
    private->still.flash_hdr_gains = output->snap.hdr_gains;
    private->still.dual_led_setting = output->stats_update.aec_update.dual_led_setting;
  }
  if (private->still.is_capture_intent) {
    /* Update with still capture intent data only for Auto/ON LED mode */
    if (private->still.is_flash_snap_data && private->led_mode != LED_MODE_OFF) {
      output->stats_update.aec_update.sensor_gain = private->still.flash_sensor_gain;
      output->stats_update.aec_update.real_gain   = private->still.flash_real_gain;
      output->stats_update.aec_update.s_real_gain = private->still.flash_hdr_gains.s_gain;
      output->stats_update.aec_update.l_real_gain = private->still.flash_hdr_gains.l_gain;

      output->stats_update.aec_update.linecount   = private->still.flash_line_cnt;
      output->stats_update.aec_update.s_linecount = private->still.flash_hdr_gains.s_linecount;
      output->stats_update.aec_update.l_linecount = private->still.flash_hdr_gains.l_linecount;

      output->stats_update.aec_update.hdr_sensitivity_ratio =
        private->still.flash_hdr_gains.hdr_sensitivity_ratio;
      output->stats_update.aec_update.hdr_exp_time_ratio =
        private->still.flash_hdr_gains.hdr_exp_time_ratio;

      output->stats_update.aec_update.lux_idx   = private->still.flash_lux_index;
      output->stats_update.aec_update.exif_iso  = private->still.flash_exif_iso;
      output->stats_update.aec_update.exp_time = private->still.flash_exp_time;
      /* drc gains */
      output->stats_update.aec_update.total_drc_gain =
        private->still.flash_drc_gains.total_drc_gain;
      output->stats_update.aec_update.color_drc_gain =
        private->still.flash_drc_gains.color_drc_gain;
      output->stats_update.aec_update.gtm_ratio =
        private->still.flash_drc_gains.gtm_ratio;
      output->stats_update.aec_update.ltm_ratio =
        private->still.flash_drc_gains.ltm_ratio;
      output->stats_update.aec_update.la_ratio =
        private->still.flash_drc_gains.la_ratio;
      output->stats_update.aec_update.gamma_ratio =
        private->still.flash_drc_gains.gamma_ratio;
      output->stats_update.aec_update.dual_led_setting = private->still.dual_led_setting;
      private->still.is_flash_snap_data = FALSE;
    } else {
      if (private->aec_auto_mode == AEC_AUTO) {
        output->stats_update.aec_update.sensor_gain = output->snap.sensor_gain;
          output->stats_update.aec_update.real_gain = output->snap.real_gain;
        output->stats_update.aec_update.s_real_gain = output->snap.hdr_gains.s_gain;
        output->stats_update.aec_update.l_real_gain = output->snap.hdr_gains.l_gain;

        output->stats_update.aec_update.linecount = output->snap.line_count;
        output->stats_update.aec_update.s_linecount = output->snap.hdr_gains.s_linecount;
        output->stats_update.aec_update.l_linecount = output->snap.hdr_gains.l_linecount;

        output->stats_update.aec_update.hdr_sensitivity_ratio =
          output->snap.hdr_gains.hdr_sensitivity_ratio;
        output->stats_update.aec_update.hdr_exp_time_ratio =
          output->snap.hdr_gains.hdr_exp_time_ratio;

        output->stats_update.aec_update.exif_iso = output->snap.exif_iso;
        output->stats_update.aec_update.lux_idx  = output->snap.lux_index;
        output->stats_update.aec_update.exp_time = output->snap.exp_time;
        output->stats_update.aec_update.total_drc_gain =
          output->snap.drc_gains.total_drc_gain;
        output->stats_update.aec_update.color_drc_gain =
          output->snap.drc_gains.color_drc_gain;
        output->stats_update.aec_update.gtm_ratio =
          output->snap.drc_gains.gtm_ratio;
        output->stats_update.aec_update.ltm_ratio =
          output->snap.drc_gains.ltm_ratio;
        output->stats_update.aec_update.la_ratio =
          output->snap.drc_gains.la_ratio;
        output->stats_update.aec_update.gamma_ratio =
          output->snap.drc_gains.gamma_ratio;

        /* Overwrite Snapshot parameters with queried exposure values from algo for initial frames */
        if (private->cur_sof_id <= private->max_pipeline_delay) {
          aec_port_handle_aec_update(port);
          output->stats_update.aec_update.sensor_gain = private->aec_get_data.sensor_gain[0];;
          output->stats_update.aec_update.real_gain = private->aec_get_data.real_gain[0];
          output->stats_update.aec_update.linecount = private->aec_get_data.linecount[0];
          output->stats_update.aec_update.exp_time = private->aec_get_data.exp_time;
        }
      } else {
        /* Handle partial auto mode for HAL3 snapshot */
        float real_gain = output->snap.real_gain;
        uint32_t linecount = output->snap.line_count;
        float exp_time = output->snap.exp_time;

        float out_real_gain = 0;
        float out_sensor_gain = 0;
        uint32_t out_linecount = 0;
        float out_exp_time = 0;

        /* HAL 1 manual 3A applies only to non-ZSL snapshot */
        /* In Manual AEC, ADRC is disabled*/
        aec_port_cal_manual_mode_exp_param(private, real_gain, linecount, exp_time,
          &out_real_gain, &out_sensor_gain, &out_linecount, &out_exp_time);

        output->stats_update.aec_update.sensor_gain = out_sensor_gain;
        output->stats_update.aec_update.real_gain = out_real_gain;
        output->stats_update.aec_update.s_real_gain = 0;
        output->stats_update.aec_update.l_real_gain = 0;

        output->stats_update.aec_update.linecount = out_linecount;
        output->stats_update.aec_update.s_linecount = 0;
        output->stats_update.aec_update.l_linecount = 0;

        output->stats_update.aec_update.hdr_sensitivity_ratio = 0;
        output->stats_update.aec_update.hdr_exp_time_ratio = 0;

        output->stats_update.aec_update.exif_iso = output->snap.exif_iso;
        output->stats_update.aec_update.lux_idx  = output->snap.lux_index;
        output->stats_update.aec_update.exp_time = out_exp_time;
      }
    }
  }

  if (private->fast_aec_data.enable &&
    output->stats_update.aec_update.sof_id >= private->fast_aec_forced_cnt) {
    AEC_HIGH("Fast-AEC: force settled to run AWB, fid %d",
      output->stats_update.aec_update.sof_id);
    output->stats_update.aec_update.settled = TRUE;
  }

  output->stats_update.aec_update.dual_exp_multiplier =
    1.0f / aec_port_get_dual_cam_multiplier_unadjusted(&private->dual_cam_info);
  private->touch_ev_status = output->stats_update.aec_update.touch_ev_status;
  private->low_light_shutter_flag =
    output->stats_update.aec_update.low_light_shutter_flag;
  output->stats_update.aec_update.est_state = private->est_state;
  output->stats_update.aec_update.led_mode = private->led_mode;

  private->locked_from_algo = output->locked_from_algo;
  private->core_aec_locked = output->aec_locked;
  private->conv_speed = output->conv_speed;
  private->led_fd_settings.led_fd_use_delay = output->led_fd_use_delay;
  private->lux_idx = output->stats_update.aec_update.lux_idx;
  private->lcd_flash_settled = output->lcd_flash_settled;

  /*update shdr params in aec stats update */
  STATS_MEMCPY(&output->stats_update.aec_update.vhdr_update, sizeof(aec_vhdr_update_t),
    &output->vhdr_update, sizeof(aec_vhdr_update_t));

  /* Save algo update */
  private->state_update.type = AEC_PORT_STATE_ALGO_UPDATE;
  STATS_MEMCPY(&private->state_update.u.output[aec_output_index],
    sizeof(aec_output_data_t), output, sizeof(aec_output_data_t));
  private->state_update.u.output[aec_output_index].aec_custom_param =
    output->aec_custom_param;

  /* Keep this aec state update call for legacy not needed in HAL3 */
  aec_port_update_aec_state(private, &private->state_update);

  /* Save the debug data in private data struct to be sent out later */
  private->aec_debug_data_size = output->aec_debug_data_size;
  if (output->aec_debug_data_size) {
    STATS_MEMCPY(private->aec_debug_data_array, sizeof(private->aec_debug_data_array),
      output->aec_debug_data_array, output->aec_debug_data_size);
  }
}

void aec_port_update_aec_state(aec_port_private_t *private,
  aec_state_update_data_t *aec_update_state)
{
  pthread_mutex_lock(&private->update_state_lock);
  uint8_t last_state        = private->aec_state;
  boolean low_light         = FALSE;
  aec_output_data_t *output = NULL;
  if (aec_update_state->type == AEC_PORT_STATE_MODE_UPDATE) {
    if (aec_update_state->u.trigger_new_mode) {
      if (private->aec_auto_mode != AEC_MANUAL)
        private->aec_state = CAM_AE_STATE_SEARCHING;
      else
        private->aec_state = CAM_AE_STATE_INACTIVE;
    }
  } else if (aec_update_state->type == AEC_PORT_STATE_ALGO_UPDATE) {

    uint8_t output_index = private->state_update.cb_output_index;
    output_index %= AEC_OUTPUT_ARRAY_MAX_SIZE;
    output = &aec_update_state->u.output[output_index];

    /* state transition logic */
    switch(private->aec_state) {
    case CAM_AE_STATE_INACTIVE: {
      if (aec_port_is_aec_locked(private)) {
        private->aec_state = CAM_AE_STATE_LOCKED;
      } else if (private->aec_auto_mode == AEC_MANUAL) {
        // no change
      } else if (aec_port_is_aec_searching(output)) {
        private->aec_state = CAM_AE_STATE_SEARCHING;
      } else {
        //no change
      }
      if(aec_port_is_precapture_start(private)) {
        private->aec_state = CAM_AE_STATE_PRECAPTURE;
      }
    }
      break;

    case CAM_AE_STATE_SEARCHING: {
      if (aec_port_is_aec_locked(private)) {
        private->aec_state = CAM_AE_STATE_LOCKED;
      } else if (aec_port_is_converged(output, &low_light)) {
        if (low_light == TRUE) {
          private->aec_state = CAM_AE_STATE_FLASH_REQUIRED;
        } else {
          private->aec_state = CAM_AE_STATE_CONVERGED;
        }
      } else {
        //no change
      }
      if (aec_port_is_precapture_start(private)) {
        private->aec_state = CAM_AE_STATE_PRECAPTURE;
      }
    }
      break;

    case CAM_AE_STATE_CONVERGED: {
      if (aec_port_is_aec_locked(private)) {
        private->aec_state = CAM_AE_STATE_LOCKED;
      } else {
        if (aec_port_is_converged(output, &low_light)) {
          if (low_light == TRUE) {
            private->aec_state = CAM_AE_STATE_FLASH_REQUIRED;
          } else {
            private->aec_state = CAM_AE_STATE_CONVERGED;
          }
        } else {
          private->aec_state = CAM_AE_STATE_SEARCHING;
        }
      }

      if (aec_port_is_precapture_start(private)) {
        private->aec_state = CAM_AE_STATE_PRECAPTURE;
      }
    }
      break;

    case CAM_AE_STATE_LOCKED: {
      if ((FALSE == aec_port_is_aec_locked(private)) &&
          (FALSE == private->core_aec_locked)) {
        if (aec_port_is_converged(output, &low_light)) {
          if (low_light == TRUE) {
            private->aec_state = CAM_AE_STATE_FLASH_REQUIRED;
          } else {
            private->aec_state = CAM_AE_STATE_CONVERGED;
          }
        } else {
          private->aec_state = CAM_AE_STATE_SEARCHING;
        }
      }
    }
      break;

    case CAM_AE_STATE_FLASH_REQUIRED: {
      if (aec_port_is_aec_locked(private)) {
        private->aec_state = CAM_AE_STATE_LOCKED;
      } else {
        if (aec_port_is_converged(output, &low_light)) {
          if (low_light == TRUE) {
            private->aec_state = CAM_AE_STATE_FLASH_REQUIRED;
          } else {
            private->aec_state = CAM_AE_STATE_CONVERGED;
          }
        } else {
          private->aec_state = CAM_AE_STATE_SEARCHING;
        }
      }
      if (aec_port_is_precapture_start(private)) {
        private->aec_state = CAM_AE_STATE_PRECAPTURE;
      }
    }
      break;

    case CAM_AE_STATE_PRECAPTURE: {
      if (aec_port_is_precapture_start(private)) {
        AEC_LOW("Still in Precatpure!!");
      } else if (aec_port_is_aec_locked(private)) {
        private->aec_state = CAM_AE_STATE_LOCKED;
      } else if (aec_port_is_converged(output, &low_light)) {
        if (low_light == TRUE) {
          private->aec_state = CAM_AE_STATE_FLASH_REQUIRED;
        } else {
          private->aec_state = CAM_AE_STATE_CONVERGED;
        }
      } else {
        /* Optimization: During LED estimation and Prepare snapshot is done
           led is turned off and it takes 4~5 frames to converge. Uncomment
           below to optimize this delay. But this doesn't follow State machine
           table*/
        if (private->force_prep_snap_done == TRUE) {
          if (output->stats_update.aec_update.flash_needed) {
            private->aec_state = CAM_AE_STATE_FLASH_REQUIRED;
          } else {
            private->aec_state = CAM_AE_STATE_CONVERGED;
          }
        }
      }
      private->force_prep_snap_done = FALSE;
    }
      break;

    default: {
      AEC_ERR("Error, AEC last state is unknown: %d",
        private->aec_last_state);
    }
      break;
    }
  } else {
    AEC_ERR("Invalid state transition type");
  }

  if (private->aec_state != last_state) {
    AEC_LOW("AE state transition Old=%d New=%d aec_precap_start=%d",
      last_state, private->aec_state, private->aec_precap_start);
  }

  private->aec_last_state = last_state;
error_update_aec_state:
  pthread_mutex_unlock(&private->update_state_lock);
}

/** aec_port_forward_update_event_if_linked:
  *    @mct_port:   MCT port object
  *    @aec_update: AEC update info
  *
  * Forward the AEC update event to the slave session if: dual camera is in use, the
  * cameras are linked, and this method is called from the master session
  *
  * Return: boolean value indicating success or failure
  */
static boolean aec_port_forward_update_event_if_linked(
  mct_port_t* mct_port,
  aec_output_data_t *output)
{
  aec_port_private_t* aec_port = (aec_port_private_t *)(mct_port->port_private);
  boolean result = true;

  /* Only forward the AEC update if we are the master camera, and the peer ID is set,
     indicating dual camera is in use */
  if (aec_port->dual_cam_info.is_aux_sync_enabled) {
    /* Populate the peer info with master's AEC update and anti-banding state */
    aec_port_peer_aec_update        aec_peer_update;
    aec_peer_update.output         = (*output);
    aec_peer_update.awb_color_temp = aec_port->awb_color_temp;

    /* Forward the AEC update info to the slave session */
    result = stats_util_post_intramode_event(mct_port,
                                             aec_port->dual_cam_info.intra_peer_id,
                                             MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT,
                                             (void*)&aec_peer_update);
  }

  if (!result) {
    AEC_ERR("Error! failed to forward the AEC Update event to the slave (id=%d)",
      aec_port->dual_cam_info.intra_peer_id);
  }

  return result;
}

void aec_port_send_aec_update(mct_port_t *port, aec_port_private_t
  *private, char *trigger_name, uint8_t output_index, uint32_t aec_update_frame_id)
{
  private->still.is_capture_intent = FALSE;
  output_index %= AEC_OUTPUT_ARRAY_MAX_SIZE;

  if(!aec_port_send_update_message_enabled(private)) {
    return;
  }

  /* check if the gains are valid */
  if (private->state_update.u.output[output_index].stats_update.aec_update.sensor_gain == 0 ||
      private->state_update.u.output[output_index].stats_update.aec_update.linecount == 0) {
    AEC_HIGH("WARNING: Sending invalid AEC Update: Skipped...");
    return;
  }

  /* This function ensures that the AEC line count does not change the frame duration if the
  the max and min frame rate values have been forced to be the same. This helps resolve the FPS
  CTS test failure under low-light conditions */
  aec_port_adjust_for_fixed_fps(&private->state_update.u.output[output_index], private);

  /* exif, metadata, stats config handling moved to here */
  aec_port_pack_exif_info(port,
    &private->state_update.u.output[output_index]);
  if (private->state_update.u.output[output_index].eztune.running) {
    aec_port_send_aec_info_to_metadata(port,
    &private->state_update.u.output[output_index]);
  }

  MCT_OBJECT_LOCK(port);
  aec_port_update_aec_flash_state(port, &private->state_update.u.output[output_index]);
  MCT_OBJECT_UNLOCK(port);

  /* Use fixed AEC settings. This will be needed for testing */
  if (private->fixed_settings.fixed_settings_enable) {
    aec_port_update_fixed_settings_in_output(private,
      &private->state_update.u.output[output_index]);
  }

  aec_port_print_log(&private->state_update.u.output[output_index], trigger_name, private, output_index);

  AEC_LOW("AEC_UPDATE_DBG: %s: cam-role=%d,cam-mode:%d, camera_id:%d Sending AEC Update for Frame_ID:%d"
    " Curr_SOF_ID:%d ouput_index=%d", trigger_name,
    private->dual_cam_info.cam_role, private->dual_cam_info.mode,
    private->camera_id,
    aec_update_frame_id,
    private->cur_sof_id, output_index);

  private->state_update.type = AEC_PORT_STATE_ALGO_UPDATE;
  private->state_update.u.output[output_index].stats_update.flag = STATS_UPDATE_AEC;
  private->aec_update_flag = TRUE;
  aec_port_send_event(port, MCT_EVENT_MODULE_EVENT,
    MCT_EVENT_MODULE_STATS_AEC_UPDATE,
   (void *)(&(private->state_update.u.output[output_index].stats_update)), aec_update_frame_id);

  /* Forward AEC update data to slave session if there is a slave */
  (void)aec_port_forward_update_event_if_linked(port, &private->state_update.u.output[output_index]);

}

static void aec_port_stats_done_callback(void* p, void* stats)
{
  mct_port_t         *port = (mct_port_t *)p;
  aec_port_private_t *private = NULL;
  stats_t            *aec_stats = (stats_t *)stats;
  if (!port) {
    AEC_ERR("input error");
    return;
  }

  private = (aec_port_private_t *)(port->port_private);
  if (!private) {
    return;
  }

  AEC_LOW("DONE AEC STATS ACK back");
  if (aec_stats) {
    circular_stats_data_done(aec_stats->ack_data, port,
                             private->reserved_id, private->cur_sof_id);
  }
}

/** aec_port_apply_antibanding:
  *
  *    @antibanding: Antibanding mode to use to apply antibanding on the exposure time
  *    @exp_time:    Exposure time to apply antibanding on
  *
  * Applies antibanding on the given exposure time and returns the adjusted exposure time.
  * If not banding was applied, the unmodified exposure time is returned.
  *
  * Return: adjusted exposure time which takes antibanding into account
  */
static float aec_port_apply_antibanding(
  aec_antibanding_type_t antibanding,
  float exp_time)
{
  if (antibanding == STATS_PROC_ANTIBANDING_60HZ ||
      antibanding == STATS_PROC_ANTIBANDING_50HZ)
  {
    int freq = (antibanding == STATS_PROC_ANTIBANDING_60HZ ? (60*2) : (50*2));
    float band_period = 1.0f / freq;
    if (exp_time > band_period)
      exp_time = band_period * (int)(exp_time / band_period);
  }

  return exp_time;
}

/** aec_port_callback:
 *    @output: TODO
 *    @p:      TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static void aec_port_callback(aec_output_data_t *output, void *p)
{
  mct_port_t         *port = (mct_port_t *)p;
  aec_port_private_t *private = NULL;
  mct_event_t        event;
  boolean            low_light = FALSE;
  boolean            send_update_now = FALSE;


  if (!output || !port) {
    AEC_ERR("input error");
    return;
  }

  private = (aec_port_private_t *)(port->port_private);
  if (!private) {
    return;
  }

  /* First handle callback in extension if available */
  if (private->func_tbl.ext_callback) {
    stats_ext_return_type ret;
    ret = private->func_tbl.ext_callback(
      port, output, &output->stats_update.aec_update);
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      AEC_LOW("Callback handled. Skipping rest!");
      return;
    }
  }

  /* populate the stats_upate object to be sent out*/
  if (AEC_UPDATE == output->type) {

    /* skip stats for Unified Flash capture */
    if (private->stats_frame_capture.frame_capture_mode) {
      AEC_LOW("Skip CB processing in Unified Flash mode");
      return;
    } else {
      /* Do not send Auto update event in following cases:
        * 1. Manual mode: If callback comes after setting the manual mode,
        * then two aec updates(auto and manual) are sent on same frame.
        * 2. If stats skip count is non zero */
      if (private->aec_skip.skip_count || AEC_MANUAL == private->aec_auto_mode) {
        AEC_LOW("Send AEC Update: Skip sending update, skip count=%d, ae mode=%d",
          private->aec_skip.skip_count, private->aec_auto_mode);
        return;
      }
    }

    MCT_OBJECT_LOCK(port);

    uint32_t cur_sof_id = private->cur_sof_id;
    uint32_t cur_aec_out_frame_id = output->stats_update.aec_update.frame_id;
    uint8_t cb_output_index = 0;

    /* Initialization, after hitting first CB from algo */
    if (private->state_update.sof_output_index == 0xFF)
      private->state_update.sof_output_index = 0;
    if (private->state_update.cb_output_index == 0xFF)
      private->state_update.cb_output_index = 0;

    private->aec_update_flag = TRUE;
    /* Produce  AEC Update @ CB out index position, and update SOF index with CB index
       Consume AEC update @ SOF with SOF index */
    cb_output_index = private->state_update.cb_output_index;
    private->state_update.sof_output_index = cb_output_index;

    /* AEC Update goes out for "CurrSOF-1" always, in case of CB */
    if ((cur_sof_id > 1) && (cur_sof_id-1 == cur_aec_out_frame_id) && (private->still.capture_sof-1 != cur_aec_out_frame_id))
      send_update_now = TRUE;
    /* AEC Update goes out for CurrSOF during Aggressive and Fast AEC convergence */
    if(((AEC_CONVERGENCE_AGGRESSIVE == private->instant_aec_type) ||
      (AEC_CONVERGENCE_FAST == private->instant_aec_type)) &&
       output->stats_update.aec_update.settled) {
      send_update_now = TRUE;
    }
    /* Update the output index only if we dont send out AEC update in CB */
    if (send_update_now == FALSE) {
        private->state_update.cb_output_index++;
        private->state_update.cb_output_index %= AEC_OUTPUT_ARRAY_MAX_SIZE;
    }
    MCT_OBJECT_UNLOCK(port);

    aec_port_pack_update(port, output, cb_output_index);
    if(send_update_now == TRUE) {
      aec_port_send_aec_update(port, private, "CB: ", cb_output_index, cur_aec_out_frame_id);
    }
    else {
      if (cur_sof_id == cur_aec_out_frame_id) {
        AEC_LOW("AEC_UPDATE_DBG: PORT_CB: Normal: cam-role=%d,cam-mode:%d Send this update in next SOF_ID:%d"
          " OutputFrameId:%d output_index=%d -> Do Nothing",
         private->dual_cam_info.cam_role, private->dual_cam_info.mode,
         cur_sof_id+1, cur_aec_out_frame_id, cb_output_index);
      }
      else {
        AEC_INFO("AEC_UPDATE_DBG: PORT_CB: WARNING: cam-role=%d,cam-mode:%d threading issue: SOF_ID:%d"
          " OutputFrameId:%d output_index=%d -> Debug",
          private->dual_cam_info.cam_role, private->dual_cam_info.mode,
          cur_sof_id, cur_aec_out_frame_id, cb_output_index);
        aec_port_send_aec_update(port, private, "CB-WAR: ",
          cb_output_index,
          cur_aec_out_frame_id);
      }
    }
  }
  else if (AEC_SEND_EVENT == output->type) {
    /* Update start-up values in both outputs structs */
    int32 idx;

    /* Initialization, after hitting first CB from algo */
    if (private->state_update.sof_output_index == 0xFF)
      private->state_update.sof_output_index = 0;
    if (private->state_update.cb_output_index == 0xFF)
      private->state_update.cb_output_index = 0;
    private->state_update.u.output[0].stats_update.aec_update.frame_id = 0;

    aec_port_pack_update(port, output, 0);
    for(idx = 1; idx < AEC_OUTPUT_ARRAY_MAX_SIZE; idx++) {
      STATS_MEMCPY(&private->state_update.u.output[idx],sizeof(aec_output_data_t),
        &private->state_update.u.output[0], sizeof(aec_output_data_t));
      private->state_update.u.output[idx].stats_update.aec_update.frame_id = idx;
    }

#if 0
    /* exif, metadata, stats config handling moved to here */
    aec_port_pack_exif_info(port, output);
    if (output->eztune.running) {
      aec_port_send_aec_info_to_metadata(port,output);
    }
#endif
    aec_port_send_aec_update(port, private, "ALGO_STORE_UP: ", 0, 0);
  }

  /* Set configuration if required */
  if (output->need_config &&
      ((AEC_UPDATE == output->type) || (AEC_SEND_EVENT == output->type)) &&
      aec_port_can_stats_be_configured(private)) {
    aec_port_configure_stats(output, port);
    output->need_config = 0;
  }

  return;
}

/** aec_port_parse_RDI_stats_AE:
 *    @destLumaBuff: TODO
 *    @rawBuff:      TODO
 *
 * TODO description
 *
 * TODO Return
 **/

static int32_t aec_port_parse_RDI_stats_AE(aec_port_private_t *private,
  uint32_t *destLumaBuff, void *rawBuff)
{
  int32_t rc = -1;

  if (private->parse_RDI_stats.parse_VHDR_stats_callback != NULL) {
    rc = private->parse_RDI_stats.parse_VHDR_stats_callback(destLumaBuff,
        rawBuff);
  } else {
    AEC_LOW(" parse_VHDR_stats_callback not supported");
    rc = -1;
  }

  return rc;
}

/** aec_port_check_session
 *  @data1: session identity;
 *  @data2: new identity to compare.
 **/
static boolean aec_port_check_session(void *data1, void *data2)
 {
  return (((*(unsigned int *)data1) & 0xFFFF0000) ==
          ((*(unsigned int *)data2) & 0xFFFF0000) ?
          TRUE : FALSE);
}

static boolean aec_port_conv_fromhal_flashmode(cam_flash_mode_t flashmode ,
  flash_mode_t *internel_mode){

  switch (flashmode) {
    case CAM_FLASH_MODE_OFF: {
     *internel_mode = FLASH_MODE_OFF;
     break;
    }

    case CAM_FLASH_MODE_AUTO: {
     *internel_mode = FLASH_MODE_AUTO;
     break;
    }

    case CAM_FLASH_MODE_ON: {
      *internel_mode = FLASH_MODE_ON;
      break;
    }

    case CAM_FLASH_MODE_TORCH: {
      *internel_mode = FLASH_MODE_TORCH;
      break;
    }

    case CAM_FLASH_MODE_SINGLE: {
      *internel_mode = FLASH_MODE_SINGLE;
      break;
    }

    default: {
      *internel_mode = FLASH_MODE_MAX;
      break;
    }

  }
  return TRUE;
}


static boolean aec_port_conv_tohal_flashmode(cam_flash_mode_t *flashmode ,
  flash_mode_t internel_mode){

  switch (internel_mode) {
    case FLASH_MODE_OFF: {
     *flashmode = CAM_FLASH_MODE_OFF;
     break;
    }

    case FLASH_MODE_AUTO: {
     *flashmode = CAM_FLASH_MODE_AUTO;
     break;
    }

    case FLASH_MODE_ON: {
      *flashmode = CAM_FLASH_MODE_ON;
      break;
    }

    case FLASH_MODE_TORCH: {
      *flashmode = CAM_FLASH_MODE_TORCH;
      break;
    }

    case FLASH_MODE_SINGLE: {
      *flashmode = CAM_FLASH_MODE_SINGLE;
      break;
    }

    default: {
      *flashmode = CAM_FLASH_MODE_MAX;
      break;
    }
  }
  return TRUE;
}

/** aec_port_check_identity:
 *    @data1: session identity;
 *    @data2: new identity to compare.
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_port_check_identity(unsigned int data1, unsigned int data2)
{
  return ((data1 & 0xFFFF0000) == (data2 & 0xFFFF0000)) ? TRUE : FALSE;
}
/** aec_port_update_manual_setting:
 *    @private:   Private data of the port
 *    @aec_update: manula aec settings.
 *
 * This function converts manual settings from HAL3 to gain /linecount.
 *
 * Return: bool
 **/
static boolean aec_port_update_manual_setting(aec_port_private_t  *private ,
  aec_manual_update_t * aec_update)
{
  float new_sensitivity;

  if(!private || !aec_update) {
    AEC_ERR("Invalid parameters for manual mode");
    return FALSE;
  }

  if (!private->manual.is_exp_time_valid || !private->manual.is_gain_valid) {
    AEC_ERR(" both exp_time & gain need to be set");
    return FALSE;
  }

  if (private->manual.is_exp_time_valid) {
    aec_update->linecount = (1.0 * private->manual.exp_time *
      private->sensor_info.pixel_clock / private->sensor_info.pixel_clock_per_line);
    if (aec_update->linecount < 1) {
      aec_update->linecount = 1;
    }
  }
  if (private->manual.is_gain_valid) {
    aec_update->sensor_gain = private->manual.gain;
  }
  /* Update exp idx */
  new_sensitivity = aec_update->sensor_gain *
    aec_update->linecount;
  aec_update->lux_idx =
      log10(new_sensitivity/ private->init_sensitivity)/ log10(1.03);
  aec_update->exif_iso = (uint32_t)(aec_update->sensor_gain * 100 / private->ISO100_gain);
  AEC_LOW("Manual mode called gain %f  linecnt %d lux idx %f",
     aec_update->sensor_gain, aec_update->linecount, aec_update->lux_idx);
  aec_update->aec_settled = 1;

/* Update the metadata for Manual setting */
  STATS_MEMSET(&private->aec_info, 0, sizeof(cam_3a_params_t));
  private->aec_info.exp_time = private->manual.exp_time;
  private->aec_info.iso_value = aec_update->exif_iso;
  private->aec_info.real_gain = aec_update->sensor_gain;
  private->aec_info.settled = 1;

  return TRUE;
}

/** aec_port_unified_fill_manual:
 *    @frame_batch:  Frame data to be populated (output)
 *    @private: Private port data (input)
 *    @manual_3A_mode: Manual parameters to be configure (input)
 *    @capture_type: Unified capture type
 *
 * This function fill the batch structure using manual parameters.
 *
 * Return: TRUE on success
 **/
static boolean aec_port_unified_fill_manual(aec_capture_frame_info_t *frame_batch,
  aec_port_private_t  *private,
  cam_capture_manual_3A_t manual_3A_mode,
  cam_capture_type capture_type)
{
  float new_sensitivity = 0.0;


  if (CAM_CAPTURE_MANUAL_3A == capture_type) {
    if (!(CAM_SETTINGS_TYPE_ON == manual_3A_mode.exp_mode) ||
      !(CAM_SETTINGS_TYPE_ON == manual_3A_mode.iso_mode)) {
      AEC_ERR("Invalid parameter: exp_mode: %d, iso_mode: %d",
        manual_3A_mode.exp_mode, manual_3A_mode.iso_mode);
      return FALSE;
    }
    frame_batch->exp_time = (float)manual_3A_mode.exp_time / 1000000000;
    frame_batch->sensor_gain = manual_3A_mode.iso_value;
  } else if (CAM_CAPTURE_RESET == capture_type) {
    if (!private->manual.is_exp_time_valid || !private->manual.is_gain_valid) {
      AEC_ERR("Error: both exp_time & gain must be set");
      return FALSE;
    }
    frame_batch->exp_time = private->manual.exp_time;
    frame_batch->sensor_gain = private->manual.gain;
  } else {
    AEC_ERR("Error: Invalid capture type");
    return FALSE;
  }

  frame_batch->line_count = (1.0 * frame_batch->exp_time *
    private->sensor_info.pixel_clock / private->sensor_info.pixel_clock_per_line);
  if (frame_batch->line_count < 1) {
    frame_batch->line_count = 1;
  }

  new_sensitivity = frame_batch->sensor_gain * frame_batch->line_count;
  frame_batch->lux_idx =
    log10(new_sensitivity / private->init_sensitivity) / log10(1.03);

  frame_batch->iso = (uint32_t)(frame_batch->sensor_gain * 100 / private->ISO100_gain);

  return TRUE;
}

/** aec_port_unifed_request_batch_data:
 *    @private:   Private data of the port
 *
 * This function request to the algoritm the data to fill the batch information.
 *
 * Return: TRUE on success
 **/
static boolean aec_port_unifed_request_batch_data_to_algo(
  aec_port_private_t *private)
{
  boolean rc = FALSE;
  aec_frame_batch_t *priv_frame_info = NULL, *algo_frame_info = NULL;
  stats_update_t *stats_update = NULL;
  int i = 0, j = 0;

  q3a_thread_aecawb_msg_t *aec_msg =
    aec_port_create_msg(MSG_AEC_GET, AEC_GET_PARAM_UNIFIED_FLASH, private);

  if (!aec_msg) {
    AEC_ERR("Not enough memory");
    rc = FALSE;
    return rc;
  }
  aec_msg->sync_flag = TRUE;
  STATS_MEMCPY(&aec_msg->u.aec_get_parm.u.frame_info, sizeof(aec_frame_batch_t),
    &private->stats_frame_capture.frame_info, sizeof(aec_frame_batch_t));
  AEC_LOW("No. of Batch = %d",
    aec_msg->u.aec_get_parm.u.frame_info.num_batch);
  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  if (FALSE == rc) {
    AEC_ERR("Fail to get UNIFIED_FLASH");
    free(aec_msg);
    return rc;
  }

  priv_frame_info = &private->stats_frame_capture.frame_info;
  algo_frame_info = &aec_msg->u.aec_get_parm.u.frame_info;

  /* Verify that read-only data was not change by algo */
  if (priv_frame_info->num_batch != algo_frame_info->num_batch){
    AEC_ERR("Error: AEC_GET_PARAM_UNIFIED_FLASH call modified read-only param");
    free(aec_msg);
    return FALSE;
  }
  for (i = 0; i < priv_frame_info->num_batch; i++) {
    if ((priv_frame_info->frame_batch[i].capture_type !=
          algo_frame_info->frame_batch[i].capture_type) ||
        (priv_frame_info->frame_batch[i].flash_hal !=
          algo_frame_info->frame_batch[i].flash_hal) ||
        (priv_frame_info->frame_batch[i].flash_mode !=
          algo_frame_info->frame_batch[i].flash_mode) ||
        (priv_frame_info->frame_batch[i].hdr_exp !=
          algo_frame_info->frame_batch[i].hdr_exp)) {
      AEC_ERR("Error: AEC_GET_PARAM_UNIFIED_FLASH call modified capture_type param");
      free(aec_msg);
      return FALSE;
    }
  }

  /* Copy algo data to internal private sturcture */
  private->stats_frame_capture.frame_info = aec_msg->u.aec_get_parm.u.frame_info;

  for (i = 0; i < priv_frame_info->num_batch; i++) {

    if (priv_frame_info->frame_batch[i].drc_gains.ltm_ratio != -1) { /* ADRC enabled */
      AEC_HIGH("AEDBG,UnifiedFlash: Frame_capture_mode idx[%i] type=%d"
        " SensorGain:RealGain:LineCount = %f:%f:%d drc_gains=(%f, %f), "
        "drc_ratios=(%f,%f,%f,%f) lux_idx=%f", i,
        priv_frame_info->frame_batch[i].capture_type,
        priv_frame_info->frame_batch[i].sensor_gain,
        priv_frame_info->frame_batch[i].real_gain,
        priv_frame_info->frame_batch[i].hdr_gains.s_linecount,
        priv_frame_info->frame_batch[i].drc_gains.total_drc_gain,
        priv_frame_info->frame_batch[i].drc_gains.color_drc_gain,
        priv_frame_info->frame_batch[i].drc_gains.gtm_ratio,
        priv_frame_info->frame_batch[i].drc_gains.ltm_ratio,
        priv_frame_info->frame_batch[i].drc_gains.la_ratio,
        priv_frame_info->frame_batch[i].drc_gains.gamma_ratio,
        priv_frame_info->frame_batch[i].lux_idx);
    }
    else { /* ADRC disabled */
      AEC_HIGH("AEDBG,UnifiedFlash: Frame_capture_mode idx[%i] type=%d s_line_count=%d real_gain=%f "
        "sensor_gain=%f", i,
        priv_frame_info->frame_batch[i].capture_type,
        priv_frame_info->frame_batch[i].hdr_gains.s_linecount,
        priv_frame_info->frame_batch[i].real_gain,
        priv_frame_info->frame_batch[i].sensor_gain,
        priv_frame_info->frame_batch[i].lux_idx);
    }
  }

  private->frame_capture_update = private->state_update;

  stats_update =
    &private->frame_capture_update.u.output[0].stats_update;
  stats_update->flag = STATS_UPDATE_AEC;
  stats_update->aec_update.sof_id = private->cur_sof_id;
  stats_update->aec_update.use_led_estimation =
    priv_frame_info->use_led_estimation;

  stats_update->aec_update.led_off_params.linecnt = priv_frame_info->led_off_linecount;
  stats_update->aec_update.led_off_params.s_gain = priv_frame_info->led_off_hdr_gains.s_gain;
  stats_update->aec_update.led_off_params.s_linecnt = priv_frame_info->led_off_hdr_gains.s_linecount;
  stats_update->aec_update.led_off_params.l_gain = priv_frame_info->led_off_hdr_gains.l_gain;
  stats_update->aec_update.led_off_params.l_linecnt = priv_frame_info->led_off_hdr_gains.l_linecount;

  /* Update adrc specific fields here start*/
  stats_update->aec_update.led_off_params.real_gain = priv_frame_info->led_off_real_gain;
  stats_update->aec_update.led_off_params.sensor_gain = priv_frame_info->led_off_sensor_gain;
  stats_update->aec_update.led_off_params.total_drc_gain =
    priv_frame_info->led_off_drc_gains.total_drc_gain;
  stats_update->aec_update.led_off_params.color_drc_gain =
    priv_frame_info->led_off_drc_gains.color_drc_gain;
  stats_update->aec_update.led_off_params.gtm_ratio =
    priv_frame_info->led_off_drc_gains.gtm_ratio;
  stats_update->aec_update.led_off_params.ltm_ratio =
    priv_frame_info->led_off_drc_gains.ltm_ratio;
  stats_update->aec_update.led_off_params.la_ratio =
    priv_frame_info->led_off_drc_gains.la_ratio;
  stats_update->aec_update.led_off_params.gamma_ratio =
    priv_frame_info->led_off_drc_gains.gamma_ratio;

  if (stats_update->aec_update.led_off_params.ltm_ratio != -1) { /* ADRC enabled */
    AEC_LOW("UnifiedFlash: Off Gains: real_gain=%f"
      " sensor_gain=%f linecount=%d DRC gains OFF=(%f, %f)",
      stats_update->aec_update.led_off_params.real_gain,
      stats_update->aec_update.led_off_params.sensor_gain,
      stats_update->aec_update.led_off_params.linecnt,
      stats_update->aec_update.led_off_params.total_drc_gain,
      stats_update->aec_update.led_off_params.color_drc_gain);
  }
  /* Update adrc specific fields here end*/

  AEC_LOW("Current Batch no. =%d, frame_capture_mode set to true",
    private->stats_frame_capture.current_batch_count);
  free(aec_msg);

  return rc;
}

q3a_capture_type_t get_q3a_capture_type(cam_capture_type cap)
{
  switch(cap) {
  case CAM_CAPTURE_NORMAL:
    return Q3A_CAPTURE_NORMAL;
  case CAM_CAPTURE_FLASH:
    return Q3A_CAPTURE_FLASH;
  case CAM_CAPTURE_BRACKETING:
    return Q3A_CAPTURE_BRACKETING;
  case CAM_CAPTURE_LOW_LIGHT:
    return Q3A_CAPTURE_LOW_LIGHT;
  case CAM_CAPTURE_RESET:
    return Q3A_CAPTURE_RESET;
  case CAM_CAPTURE_MANUAL_3A:
    return Q3A_CAPTURE_MANUAL_3A;
  case CAM_CAPTURE_MAX:
  default:
    AEC_ERR("Invalid Capture type, fallback to normal capture");
    return Q3A_CAPTURE_NORMAL;
  }
}
/** aec_port_common_set_unified_flash:
 *    @private:   Private data of the port
 *    @cam_capture_frame_config_t: frame_info.
 *
 * This function converts gets the Frame batch info from HAL.
 *
 * Return: void
 **/
void aec_port_common_set_unified_flash(aec_port_private_t  *private,
  cam_capture_frame_config_t *frame_info)
{
  boolean rc = FALSE;
  int i = 0;
  boolean request_data_to_algo = FALSE;

  private->stats_frame_capture.frame_info.num_batch = frame_info->num_batch;
  AEC_LOW("No. of Batch from HAL =%d",
    private->stats_frame_capture.frame_info.num_batch);
  for (i = 0; i < private->stats_frame_capture.frame_info.num_batch; i++) {
    private->stats_frame_capture.frame_info.frame_batch[i].capture_type =
      get_q3a_capture_type(frame_info->configs[i].type);
    AEC_LOW("frame_batch[%d]: type=%d, aec_auto_mode=%d",
      i, frame_info->configs[i].type, private->aec_auto_mode);

    if (AEC_MANUAL == private->aec_auto_mode) { /* Full manual mode: preview and capture */
      rc = aec_port_unified_fill_manual(
        &private->stats_frame_capture.frame_info.frame_batch[i],
        private,
        frame_info->configs[i].manual_3A_mode,
        frame_info->configs[i].type);
      if (!rc) {
        AEC_ERR("Error, fail to fill manual data");
      }

      private->stats_frame_capture.frame_info.frame_batch[i].flash_mode = FALSE;
      private->stats_frame_capture.frame_info.frame_batch[i].hdr_exp = 0;
      continue; /* End of loop for Full manual mode */
    }

    if (frame_info->configs[i].type == CAM_CAPTURE_BRACKETING) {
      private->stats_frame_capture.frame_info.frame_batch[i].hdr_exp =
       frame_info->configs[i].hdr_mode.values;
    } else if (frame_info->configs[i].type == CAM_CAPTURE_FLASH) {
      AEC_HIGH("CAM_CAPTURE_FLASH: flash_mode=%d",
        frame_info->configs[i].flash_mode);
      aec_port_conv_fromhal_flashmode(frame_info->configs[i].flash_mode,
        &private->stats_frame_capture.frame_info.frame_batch[i].flash_hal);
      if ((frame_info->configs[i].flash_mode == CAM_FLASH_MODE_ON) ||
        (frame_info->configs[i].flash_mode == CAM_FLASH_MODE_TORCH) ||
        (frame_info->configs[i].flash_mode == CAM_FLASH_MODE_AUTO)) {
        private->stats_frame_capture.frame_info.frame_batch[i].flash_mode = TRUE;
      } else{
        private->stats_frame_capture.frame_info.frame_batch[i].flash_mode = FALSE;
      }
      private->stats_frame_capture.frame_info.frame_batch[i].hdr_exp = 0;
    } else if (frame_info->configs[i].type == CAM_CAPTURE_LOW_LIGHT) {
      private->stats_frame_capture.frame_info.frame_batch[i].flash_mode = FALSE;
      private->stats_frame_capture.frame_info.frame_batch[i].hdr_exp = 0;
    } else if (frame_info->configs[i].type == CAM_CAPTURE_MANUAL_3A ||
        (frame_info->configs[i].type == CAM_CAPTURE_RESET &&
        private->aec_auto_mode == AEC_MANUAL)) {
      /* Use manual value only for specific batch */
      rc = aec_port_unified_fill_manual(
        &private->stats_frame_capture.frame_info.frame_batch[i],
        private,
        frame_info->configs[i].manual_3A_mode,
        frame_info->configs[i].type);
      if (!rc) {
        AEC_ERR("Error: fail to fill manual data");
      }
      private->stats_frame_capture.frame_info.frame_batch[i].flash_mode = FALSE;
      private->stats_frame_capture.frame_info.frame_batch[i].hdr_exp = 0;
    }

    /* Verify if we need to call algo for data or not */
    if (frame_info->configs[i].type == CAM_CAPTURE_NORMAL ||
        frame_info->configs[i].type == CAM_CAPTURE_FLASH ||
        frame_info->configs[i].type == CAM_CAPTURE_BRACKETING ||
        frame_info->configs[i].type == CAM_CAPTURE_LOW_LIGHT) {
      /* Query algo only if necesary */
      request_data_to_algo = TRUE;
    }
  }

  if (request_data_to_algo) {
    /* Request batch data */
    rc = aec_port_unifed_request_batch_data_to_algo(private);
    if (FALSE == rc) {
      AEC_ERR("Fail to get batch data from AEC algo");
      STATS_MEMSET(&private->stats_frame_capture, 0, sizeof(aec_frame_capture_t));
      return;
    } else {
      aec_frame_batch_t *priv_frame_info = &private->stats_frame_capture.frame_info;
      for (i = 0; i < priv_frame_info->num_batch; i++) {
        if (priv_frame_info->frame_batch[i].capture_type == Q3A_CAPTURE_NORMAL ||
            priv_frame_info->frame_batch[i].capture_type == Q3A_CAPTURE_FLASH ||
            priv_frame_info->frame_batch[i].capture_type == Q3A_CAPTURE_BRACKETING ||
            priv_frame_info->frame_batch[i].capture_type == Q3A_CAPTURE_LOW_LIGHT) {

          private->aec_info.iso_value = priv_frame_info->frame_batch[i].iso;
          private->aec_info.exp_time = priv_frame_info->frame_batch[i].exp_time;
          private->aec_info.real_gain = priv_frame_info->frame_batch[i].real_gain;
        }
      }
    }
  }

  private->stats_frame_capture.frame_capture_mode = FALSE;
  private->stats_frame_capture.streamon_update_done = FALSE;
}

/** aec_port_unified_update_auto_aec_frame_batch:
 *    @aec_update: Destination, AEC update for the batch
 *    @frame_info: Source, AEC frame info from algo
 *    @curr_batch_cnt: batch to update
 *
 * Fill AEC update with current frame batch data
 *
 * Return: void
 **/
void aec_port_unified_update_auto_aec_frame_batch(
  aec_update_t *aec_update, aec_frame_batch_t *frame_info, uint8_t curr_batch_cnt)
{
  aec_update->linecount = frame_info->frame_batch[curr_batch_cnt].line_count;
  aec_update->real_gain = frame_info->frame_batch[curr_batch_cnt].real_gain;
  aec_update->sensor_gain = frame_info->frame_batch[curr_batch_cnt].sensor_gain;
  aec_update->s_real_gain = frame_info->frame_batch[curr_batch_cnt].hdr_gains.s_gain;
  aec_update->s_linecount = frame_info->frame_batch[curr_batch_cnt].hdr_gains.s_linecount;
  aec_update->l_real_gain = frame_info->frame_batch[curr_batch_cnt].hdr_gains.l_gain;
  aec_update->l_linecount = frame_info->frame_batch[curr_batch_cnt].hdr_gains.l_linecount;
  aec_update->hdr_sensitivity_ratio = frame_info->frame_batch[curr_batch_cnt].hdr_gains.hdr_sensitivity_ratio;
  aec_update->hdr_exp_time_ratio = frame_info->frame_batch[curr_batch_cnt].hdr_gains.hdr_exp_time_ratio;
  aec_update->lux_idx = frame_info->frame_batch[curr_batch_cnt].lux_idx;
  aec_update->gamma_flag = frame_info->frame_batch[curr_batch_cnt].gamma_flag;
  aec_update->nr_flag = frame_info->frame_batch[curr_batch_cnt].nr_flag;
  aec_update->exp_time = frame_info->frame_batch[curr_batch_cnt].exp_time;
  aec_update->exif_iso = frame_info->frame_batch[curr_batch_cnt].iso;
  aec_update->flash_sensitivity = frame_info->flash_si;

  /* Update adrc specific fields here start*/
  aec_update->total_drc_gain =
    frame_info->frame_batch[curr_batch_cnt].drc_gains.total_drc_gain;
  aec_update->color_drc_gain =
    frame_info->frame_batch[curr_batch_cnt].drc_gains.color_drc_gain;
  aec_update->gtm_ratio =
    frame_info->frame_batch[curr_batch_cnt].drc_gains.gtm_ratio;
  aec_update->ltm_ratio =
    frame_info->frame_batch[curr_batch_cnt].drc_gains.ltm_ratio;
  aec_update->la_ratio =
    frame_info->frame_batch[curr_batch_cnt].drc_gains.la_ratio;
  aec_update->gamma_ratio =
    frame_info->frame_batch[curr_batch_cnt].drc_gains.gamma_ratio;
  /* Update adrc specific fields here end*/

  AEC_LOW("UnifiedFlash: AEC_Update: capture_mode curr_idx[%d]"
    " SensorGain:RealGain:LineCount = %f:%f:%d drc_gains=(%f, %f), "
    "drc_ratios=(%f,%f,%f,%f) lux_idx=%f", curr_batch_cnt,
    aec_update->sensor_gain, aec_update->real_gain, aec_update->s_linecount,
    aec_update->total_drc_gain, aec_update->color_drc_gain, aec_update->gtm_ratio,
    aec_update->ltm_ratio ,aec_update->la_ratio ,aec_update->gamma_ratio,
    aec_update->lux_idx);

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x0310)
  aec_update->dual_led_setting.is_valid = true;
  aec_update->dual_led_setting.led1_low_setting =
    frame_info->frame_batch[curr_batch_cnt].dual_led_setting.low_setting.LED1_setting;
  aec_update->dual_led_setting.led2_low_setting =
    frame_info->frame_batch[curr_batch_cnt].dual_led_setting.low_setting.LED2_setting;
  aec_update->dual_led_setting.led1_high_setting =
    frame_info->frame_batch[curr_batch_cnt].dual_led_setting.high_setting.LED1_setting;
  aec_update->dual_led_setting.led2_high_setting =
    frame_info->frame_batch[curr_batch_cnt].dual_led_setting.high_setting.LED2_setting;

  aec_core_dual_led_settings_t* dual_led = &frame_info->frame_batch[curr_batch_cnt].dual_led_setting;

  AEC_LOW("low (led1,led2) : (%d, %d), high (led1,led2) : (%d, %d)",
    dual_led->low_setting.LED1_setting, dual_led->low_setting.LED2_setting,
    dual_led->high_setting.LED1_setting, dual_led->high_setting.LED2_setting);
#endif

  aec_port_conv_tohal_flashmode(&aec_update->flash_hal,
    frame_info->frame_batch[curr_batch_cnt].flash_hal);
}

/** aec_port_unified_update_manual_aec_frame_batch:
 *    @manual_update: Destination, AEC manual update for the batch
 *    @frame_info: Source, AEC manual frame data
 *    @curr_batch_cnt: batch to update
 *
 * Fill AEC update with current frame batch data
 *
 * Return: void
 **/
void aec_port_unified_update_manual_aec_frame_batch (
  aec_manual_update_t *manual_update, aec_frame_batch_t *frame_info, uint8_t curr_batch_cnt)
{
  manual_update->linecount = frame_info->frame_batch[curr_batch_cnt].line_count;
  manual_update->sensor_gain = frame_info->frame_batch[curr_batch_cnt].sensor_gain;
  manual_update->lux_idx = frame_info->frame_batch[curr_batch_cnt].lux_idx;
  manual_update->exif_iso = frame_info->frame_batch[curr_batch_cnt].iso;
}

/** aec_port_update_frame_capture_mode_data:
 *    @port: MCT port and private data of the AEC port
 *    @cur_sof_id: current SOF ID
 *
 * Populate AEC update with frame batch data and send AEC update.
 *
 * Return: TRUE on success
 **/
static boolean aec_port_update_frame_capture_mode_data(
  mct_port_t *port, uint32_t cur_sof_id)
{
  boolean rc = FALSE;
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
  mct_event_module_type_t update_type = MCT_EVENT_MODULE_STATS_AEC_UPDATE;
  aec_manual_update_t manual_update;

  /* Fill current batch data into AEC update */
  uint8_t curr_batch_cnt =
    private->stats_frame_capture.current_batch_count;
  aec_update_t *aec_update =
    &private->frame_capture_update.u.output[0].stats_update.aec_update;
  aec_frame_batch_t *frame_info =
    &private->stats_frame_capture.frame_info;

  /* Always reset estimation state to OFF in frame capture update */
  aec_update->est_state = AEC_EST_OFF;

  /* Handle AEC updates */
  switch (frame_info->frame_batch[curr_batch_cnt].capture_type) {
  case Q3A_CAPTURE_MANUAL_3A: {
    update_type = MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE;
    aec_port_unified_update_manual_aec_frame_batch(&manual_update, frame_info,
      curr_batch_cnt);
  }
    break;
  case Q3A_CAPTURE_RESET: {
    if (private->aec_auto_mode == AEC_MANUAL) {
      /* Manual reset case */
      update_type = MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE;
      aec_port_unified_update_manual_aec_frame_batch(&manual_update, frame_info,
        curr_batch_cnt);
    } else {
      /* Default reset case*/
      update_type = MCT_EVENT_MODULE_STATS_AEC_UPDATE;
      private->frame_capture_update.u.output[0].stats_update.flag =
        STATS_UPDATE_AEC;
      aec_update->sof_id = private->cur_sof_id;
      aec_port_unified_update_auto_aec_frame_batch(aec_update, frame_info,
        curr_batch_cnt);
    }
  }
    break;
  default: { /* All other cases where algorithm was query */
    update_type = MCT_EVENT_MODULE_STATS_AEC_UPDATE;
    private->frame_capture_update.u.output[0].stats_update.flag =
      STATS_UPDATE_AEC;
    aec_update->sof_id = private->cur_sof_id;
    aec_port_unified_update_auto_aec_frame_batch(aec_update, frame_info,
      curr_batch_cnt);
  }
    break;
  }


  /* Handle status/mode flags */
  switch (frame_info->frame_batch[curr_batch_cnt].capture_type) {
  case Q3A_CAPTURE_LOW_LIGHT: {
    /* Handle low_light_capture update */
    aec_update->low_light_capture_update_flag = TRUE;
  }
    break;
  case Q3A_CAPTURE_RESET: {
    private->stats_frame_capture.frame_capture_mode = FALSE;
    private->stats_frame_capture.streamon_update_done = FALSE;
    aec_update->low_light_capture_update_flag = FALSE;
    private->stats_frame_capture.frame_info.num_batch = 0;
  }
    break;
  default: {
    aec_update->low_light_capture_update_flag = FALSE;
  }
    break;
  }

  /* Update modules */
  if (MCT_EVENT_MODULE_STATS_AEC_UPDATE == update_type) {
    aec_port_print_log(&private->frame_capture_update.u.output[0],
      "FC-AEC_UP", private, -1);
    /* Send AEC update event */
    mct_event_t        event;
    event.direction = MCT_EVENT_UPSTREAM;
    event.identity = private->reserved_id;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.current_frame_id = cur_sof_id;
    event.u.module_event.type = update_type;
    event.u.module_event.module_event_data =
      (void *)(&private->frame_capture_update.u.output[0].stats_update);
    MCT_PORT_EVENT_FUNC(port)(port, &event);
    /* Forward AEC update data to slave session if there is a slave */
    (void)aec_port_forward_update_event_if_linked(port, &private->frame_capture_update.u.output[0]);
    rc = TRUE;
  } else if (MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE == update_type) {
    /* Send manual update */
    private->aec_update_flag = TRUE;
    aec_port_print_manual(private, "FC", &manual_update);
    aec_port_send_event(port, MCT_EVENT_MODULE_EVENT,
      update_type,
      (void *)(&manual_update), cur_sof_id);
    rc = TRUE;
  } else {
    AEC_ERR("Fail: invalid update type");
    rc = FALSE;
  }

  if (TRUE == rc) {
    /* UPDATE: exif and metadata */
    aec_port_pack_exif_info(port, &private->frame_capture_update.u.output[0]);
    if (private->frame_capture_update.u.output[0].eztune.running) {
      aec_port_send_aec_info_to_metadata(port,
        &private->frame_capture_update.u.output[0]);
    }
    /* Print all bus messages info */
    aec_port_print_bus("FC:INFO", private);
    aec_send_bus_message(port, MCT_BUS_MSG_AE_INFO,
      &private->aec_info, sizeof(cam_3a_params_t), private->cur_sof_id);
  }

  return rc;
}

/** aec_port_set_video_hdr_mode:
 *    @aec_mode: algo mode to be set
 *    @mode: mode coming from HAL
 *
 * Set the video hdr mode for algo.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean aec_port_set_video_hdr_mode(
  aec_video_hdr_mode_t *aec_mode, cam_sensor_hdr_type_t mode)
{
  boolean rc = TRUE;
  *aec_mode = AEC_VIDEO_HDR_MODE_OFF;
  AEC_LOW("Set video hdr mode: %d",mode);

  /* Translate HAL mode to the one algorithm understands*/
  switch (mode) {
  case CAM_SENSOR_HDR_OFF: {
    *aec_mode = AEC_VIDEO_HDR_MODE_OFF;
  }
    break;

  case CAM_SENSOR_HDR_IN_SENSOR: {
    *aec_mode = AEC_VIDEO_HDR_MODE_SENSOR;
  }
    break;

  case CAM_SENSOR_HDR_STAGGERED: {
    *aec_mode = AEC_VIDEO_HDR_MODE_STAGGERED;
  }
    break;
  default: {
    rc = FALSE;
  }
    break;
  }

  return rc;
} /* aec_port_set_video_hdr_mode */

/** aec_port_send_crop_info
 *    @private:   Private data of the port
 *
 * Return: boolean indicating success/failure in sending the event
 **/
static boolean aec_port_send_crop_info(aec_port_private_t* private)
{
  boolean rc = FALSE;

  if (private->vfe_out_width  == 0 ||
      private->vfe_out_height == 0) {
    /* Passive error; don't send the crop info until both crop info and output dimension are available */
    AEC_LOW("Skip sending crop info until ISP output dimension is available");
    return TRUE;
  } else if (private->crop_info.vfe_map_width  == 0 ||
             private->crop_info.vfe_map_height == 0) {
    /* Passive error; don't send the crop info until both crop info and output dimension are available */
    AEC_LOW("Skip sending crop info until all information is available)");
    return TRUE;
  }

  /* Update the crop info with the output dimensions */
  private->crop_info.vfe_out_width  = private->vfe_out_width;
  private->crop_info.vfe_out_height = private->vfe_out_height;

  /* Send the crop information */
  q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
    AEC_SET_PARAM_CROP_INFO, private);
  if (aec_msg != NULL) {
    aec_msg->u.aec_set_parm.u.stream_crop = private->crop_info;
    rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  }
  if (FALSE == rc) {
    AEC_ERR("Failed to send the stream crop information");
    return rc;
  }

  /* Trigger pack-output to store initial parameters and configure stats */
  if ((private->isp_infosync == (AEC_PORT_ISP_INFO_SYNC_CROP_INIT |
      AEC_PORT_ISP_INFO_SYNC_VFE_DIM_INIT)) &&
      private->aec_auto_mode != AEC_MANUAL) {
    AEC_HIGH("First CROP INFO - Trigger CB");

    aec_msg = aec_port_create_msg(MSG_AEC_SEND_EVENT,
      AEC_SET_PARAM_PACK_OUTPUT, private);
    if (aec_msg != NULL) {
      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
    }
    private->isp_infosync |= AEC_PORT_ISP_INFO_SYNC_INIT_DONE;
  }

  return rc;
}


/** aec_port_is_adrc_supported
 *    @private:   Private data of the port
 *
 * Return: ADRC Feature Enable/Disable
 **/
static boolean aec_port_is_adrc_supported(aec_port_private_t  *private)
{
  if(private->adrc_settings.adrc_force_disable != TRUE
    && private->adrc_settings.effect_mode == CAM_EFFECT_MODE_OFF
    && (private->adrc_settings.bestshot_mode == CAM_SCENE_MODE_OFF
      || private->adrc_settings.bestshot_mode == CAM_SCENE_MODE_FACE_PRIORITY
      || (private->adrc_settings.bestshot_mode == CAM_SCENE_MODE_HDR
        && private->snapshot_hdr == AEC_SENSOR_HDR_DRC))) {
    return TRUE;
  }

  return FALSE;
}

/** aec_port_set_adrc_enable
 *    @private:   Private data of the port
 *
 * Return: TRUE if no error
 **/
static boolean aec_port_set_adrc_enable(aec_port_private_t  *private)
{
  boolean rc = TRUE;
  boolean adrc_supported = aec_port_is_adrc_supported(private);

  if(adrc_supported != private->adrc_settings.is_adrc_feature_supported) {
    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_ADRC_ENABLE, private);
    if (aec_msg != NULL) {
      private->adrc_settings.is_adrc_feature_supported = adrc_supported;
      aec_msg->u.aec_set_parm.u.adrc_enable = adrc_supported;
      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
    }
  }

  return rc;
}

/** aec_port_led_cal_update:
 *    @port: private AEC port data
 *    @aec_update_state: pointer to the saved aec update state data
 *
 *
 * Returns void
 **/
static void aec_port_led_cal_update(mct_port_t *port,
  aec_state_update_data_t *aec_update_state)
{
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x0310)
  aec_port_private_t *private = NULL;
  aec_output_data_t *output = NULL;
  uint8_t output_index = 0;
  cam_capture_frame_config_t frame_config;
  aec_led_cal_state_t state = AEC_LED_CAL_INACTIVE;
  mct_event_t event;
  int32_t led_cal_result = 0;

  private = (aec_port_private_t *)port->port_private;
  output_index = aec_update_state->cb_output_index % AEC_OUTPUT_ARRAY_MAX_SIZE;
  output = &aec_update_state->u.output[output_index];
  state = (aec_led_cal_state_t)output->stats_update.aec_update.led_cal_state;

  if (state == AEC_LED_CAL_SUCCESS || state == AEC_LED_CAL_FAILED) {
    /* Turn off the Flash */
    frame_config.configs[0].num_frames = 1;
    frame_config.configs[0].type = CAM_CAPTURE_LED_CAL;
    frame_config.configs[0].flash_mode = CAM_FLASH_MODE_OFF;
    frame_config.num_batch = 1;
    aec_port_send_module_event(port,
      MCT_EVENT_MODULE_STATS_LED_CAL_CONFIG_DATA, (void*)&frame_config);

    /* Send to bus msg */
    led_cal_result = (state == AEC_LED_CAL_SUCCESS) ? 1 : 0;
    AEC_HIGH("led calibration result: %d, send to bus.", led_cal_result);
    aec_send_bus_message(port, MCT_BUS_MSG_LED_CAL_RESULT, (void*)&led_cal_result,
      sizeof(int32_t), private->cur_sof_id);
    private->led_cal_state = state;
  }
#else
  (void *)port;
  (void *)aec_update_state;
#endif
}

/** aec_port_set_led_cal:
 *    @port: private AEC port data
 *    @led_cal_enable: led calibration enabled flag
 *
 *
 * Returns true on success or false on failure.
 **/
static boolean aec_port_set_led_cal(mct_port_t *port, boolean led_cal_enable)
{
  boolean rc = TRUE;
  q3a_thread_aecawb_msg_t *aec_msg = NULL;
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x0310)
  int i = 0, num_batch = 0, led_toggle_num = 0;
  cam_capture_frame_config_t frame_config;
  aec_led_cal_config_t *cal_config = NULL;
  mct_event_t event;

  aec_msg = aec_port_create_msg(MSG_AEC_SET, AEC_SET_PARAM_LED_CAL, private);
  if (!aec_msg) {
    AEC_ERR("Not enough memory");
    return FALSE;
  }
  aec_msg->u.aec_set_parm.u.led_cal_enable = led_cal_enable;
  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  if (FALSE == rc) {
    AEC_ERR("Fail to set led cal");
    return rc;
  }

  if (led_cal_enable) {
    aec_msg = aec_port_create_msg(MSG_AEC_GET, AEC_GET_PARAM_LED_CAL_CONFIG, private);
    if (!aec_msg) {
      AEC_ERR("Not enough memory");
      return FALSE;
    }
    cal_config = &aec_msg->u.aec_get_parm.u.led_cal_config;
    aec_msg->sync_flag = TRUE;
    rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
    if (FALSE == rc) {
      AEC_ERR("Fail to get led cal info");
      free(aec_msg);
      return rc;
    }

    led_toggle_num = cal_config->led_toggle_num;
    for (i = 0; i < led_toggle_num && i < (MAX_CAPTURE_BATCH_NUM / 2); i++) {
      frame_config.configs[num_batch].num_frames = cal_config->led_off_interval;
      frame_config.configs[num_batch].type = CAM_CAPTURE_LED_CAL;
      frame_config.configs[num_batch].flash_mode = CAM_FLASH_MODE_OFF;
      num_batch++;

      frame_config.configs[num_batch].num_frames = cal_config->led_on_interval;
      frame_config.configs[num_batch].type = CAM_CAPTURE_LED_CAL;
      frame_config.configs[num_batch].flash_mode = CAM_FLASH_MODE_ON;
      num_batch++;
    }
    frame_config.num_batch = num_batch;
    aec_port_send_module_event(port,
      MCT_EVENT_MODULE_STATS_LED_CAL_CONFIG_DATA, (void*)&frame_config);
    private->led_cal_state = AEC_LED_CAL_RUNNING;
    AEC_LOW("Start Calibration: led_toggle_num: %d, on/off internval [%d, %d]",
      led_toggle_num, cal_config->led_on_interval, cal_config->led_off_interval);
    free(aec_msg);
  } else {
    AEC_LOW("Stop Calibration");
    private->led_cal_state = AEC_LED_CAL_INACTIVE;
  }
#else
  aec_msg = aec_port_create_msg(MSG_AEC_SET, AEC_SET_PARM_DUAL_LED_CALIB_MODE, private);
  if (!aec_msg) {
    AEC_ERR("Not enough memory");
    return FALSE;
  }
  aec_msg->u.aec_set_parm.u.led_cal_enable = led_cal_enable;
  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  if (FALSE == rc) {
    AEC_ERR("Fail to set led cal");
    free(aec_msg);
    return rc;
  }
#endif
  return rc;
}

/** aec_port_proc_downstream_ctrl:
 *    @port:   TODO
 *    @eventl: TODO
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_port_proc_downstream_ctrl(mct_port_t *port,
  mct_event_t *event)
{
  boolean             rc = TRUE;
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
  mct_event_control_t *mod_ctrl = &(event->u.ctrl_event);

  AEC_LOW("type =%d", event->u.ctrl_event.type);

  /* check if there's need for extended handling. */
  if (private->func_tbl.ext_handle_control_event) {
    stats_ext_return_type ret;
    AEC_LOW("Handle extended control event!");
    ret = private->func_tbl.ext_handle_control_event(port, mod_ctrl);
    /* Check if this event has been completely handled. If not we'll process it further here. */
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      AEC_LOW("Control event %d handled by extended functionality!",
        mod_ctrl->type);
      return rc;
    }
  }

  switch (mod_ctrl->type) {

  case MCT_EVENT_CONTROL_SOF: {
    mct_bus_msg_isp_sof_t *sof_event;
       sof_event =(mct_bus_msg_isp_sof_t *)(event->u.ctrl_event.control_event_data);

    uint32_t cur_stats_id = 0;
    uint32_t cur_sof_id = 0;
    uint32_t cur_aec_out_frame_id = 0;
    uint8_t sof_output_index = 0;
    boolean is_valid_index = FALSE;

    MCT_OBJECT_LOCK(port);
    cur_sof_id = private->cur_sof_id = sof_event->frame_id;
    cur_stats_id = private->cur_stats_id;
    sof_output_index = private->state_update.sof_output_index;
    is_valid_index = (sof_output_index != 0xFF);
    sof_output_index %= AEC_OUTPUT_ARRAY_MAX_SIZE;

    /* Start doing AEC update processing, only after first CB hit */
    if (is_valid_index) {
      cur_aec_out_frame_id =
        private->state_update.u.output[sof_output_index].stats_update.aec_update.frame_id;

      AEC_LOW("AEC_UPDATE_DBG: IN SOF: cam-role=%d,cam-mode:%d Curr: SOF_ID:%d Stats_ID:%d"
        " OutputFrameId:%d sof_output_idex:%d",
        private->dual_cam_info.cam_role, private->dual_cam_info.mode,
        cur_sof_id, cur_stats_id, cur_aec_out_frame_id, sof_output_index);
    }

    MCT_OBJECT_UNLOCK(port);

    if (private->stats_frame_capture.frame_capture_mode == TRUE ||
        private->stats_frame_capture.streamon_update_done == TRUE) {
      rc = aec_port_update_frame_capture_mode_data(port, cur_sof_id);
      if (FALSE == rc) {
        AEC_ERR("Fail to update frame_capture_mode data");
        break;
      }
    } else if ((private->aec_auto_mode == AEC_MANUAL) || (private->aec_on_off_mode == FALSE)) {
      /* If HAL3 manual mode is set, then send immediately and done enqueue sof msg
       * On HAL1 preview may also be required to be updated with manual AEC values */
      AEC_LOW("SOF: %d: AEC_MANUAL", cur_sof_id);
      aec_manual_update_t manual_update;
      rc = aec_port_update_manual_setting(private, &manual_update);
      if(rc) {
        private->aec_update_flag = TRUE;
        aec_port_print_manual(private, "SOF", &manual_update);
        aec_port_send_event(port, MCT_EVENT_MODULE_EVENT,
          MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE,
          (void *)(&manual_update), sof_event->frame_id);
      }
      q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
        AEC_SET_MANUAL_AUTO_SKIP, private);
      if (aec_msg != NULL)
        rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
     } else {
       /* Reset manual valid flag if mode is auto*/
       if (private->aec_auto_mode == AEC_AUTO) {
         private->manual.is_exp_time_valid = FALSE;
         private->manual.is_gain_valid = FALSE;
       }

       if (private->still.is_capture_intent) {
         private->still.capture_sof = private->cur_sof_id;
       }

       /* If there is capture intent for current SOF, just send it
        * out using saved snapshot gain */
       if(private->still.is_capture_intent &&
         private->state_update.type == AEC_PORT_STATE_ALGO_UPDATE) {
         if (is_valid_index) {
           private->aec_update_flag = TRUE;
           private->state_update.u.output[sof_output_index].stats_update.aec_update.frame_id = cur_sof_id-1;
           aec_port_pack_update(port, &private->state_update.u.output[sof_output_index],
             sof_output_index);
           aec_port_send_aec_update(port, private, "SOF_CI: ", sof_output_index, cur_sof_id-1);
         } else {
           AEC_LOW("AEC_UPDATE_DBG: SOF_UPDATE_C: cam-role=%d,cam-mode:%d AEC Update not yet available for SOF_ID:%d"
            " Curr_SOF_ID=%d Curr_Stats_ID:%d Output index=%d",
            private->dual_cam_info.cam_role, private->dual_cam_info.mode,
            cur_sof_id-1, cur_sof_id, cur_stats_id, sof_output_index);
         }
       } else {
         /* Two use cases to send AEC_UPDATE event:
                1. In MCT thread context (in SOF context itself):
                    If AEC update is available for last frame, send right away in SOF context itself
                2. In AECAWB thread context:
                    If AEC update is NOT available, do nothing here in SOF, wait for CB to happen and send
                    update in AEC thread context
              */

         /* Start doing AEC update processing, only after first CB hit */
         if ((is_valid_index) && cur_sof_id > 0) {
           if((cur_sof_id-1 == cur_aec_out_frame_id) ||
               aec_port_using_HDR_divert_stats(private)) {
             aec_port_send_aec_update(port, private, "SOF: ", sof_output_index, cur_sof_id-1);
           } else {
             AEC_LOW("AEC_UPDATE_DBG: SOF: cam-role=%d,cam-mode:%d AEC Update not yet available for SOF_ID:%d"
               " Curr_SOF_ID=%d Curr_Stats_ID:%d Output index=%d",
               private->dual_cam_info.cam_role, private->dual_cam_info.mode,
               cur_sof_id-1, cur_sof_id, cur_stats_id, sof_output_index);
           }
         }
       }
    }/*else*/

    /* Send exif info update from SoF */
    aec_port_send_exif_debug_data(port);
    aec_port_update_aec_state(private, &private->state_update);
    aec_send_batch_bus_message(port, STATS_REPORT_IMMEDIATE, sof_event->frame_id);
    if (private->led_cal_state == AEC_LED_CAL_RUNNING) {
      aec_port_led_cal_update(port, &private->state_update);
    }
  }
    break;

  case MCT_EVENT_CONTROL_PREPARE_SNAPSHOT: {
    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT, private);
    if (aec_msg != NULL) {
      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
      /* Starting precapture */
      private->aec_precap_start = TRUE;
      aec_port_print_bus("PRE_SNAP:STATE", private);
      aec_send_bus_message(port, MCT_BUS_MSG_SET_AEC_STATE, &private->aec_state,
        sizeof(cam_ae_state_t), private->cur_sof_id );
    }
  } /* MCT_EVENT_CONTROL_PREPARE_SNAPSHOT */
    break;

  case MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT: {
    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_LED_RESET, private);
    if (aec_msg != NULL) {
      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
    }
    STATS_MEMSET(&private->aec_get_data, 0, sizeof(private->aec_get_data));

    /* Unlock AEC update after the ZSL snapshot */
    private->in_zsl_capture = FALSE;
    private->stats_frame_capture.frame_capture_mode = FALSE;
    private->stats_frame_capture.streamon_update_done = FALSE;
    private->frame_capture_update.u.output[0].stats_update.aec_update.flash_hal =
     CAM_FLASH_MODE_OFF;
    private->frame_capture_update.u.output[1].stats_update.aec_update.flash_hal =
     CAM_FLASH_MODE_OFF;
  } /* MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT */
    break;

  case MCT_EVENT_CONTROL_STREAMON: {
    /* Custom optimization, provide manual exposure update */
    if (Q3A_CAPTURE_MANUAL_3A ==
      private->stats_frame_capture.frame_info.frame_batch[0].capture_type) {
      uint8_t curr_batch_cnt = 0;
      aec_update_t *aec_update =
        &private->frame_capture_update.u.output[0].stats_update.aec_update;
      aec_frame_batch_t *frame_info =
        &private->stats_frame_capture.frame_info;
      aec_manual_update_t manual_update;
      private->aec_update_flag = TRUE;
      manual_update.linecount = frame_info->frame_batch[curr_batch_cnt].line_count;
      manual_update.sensor_gain = frame_info->frame_batch[curr_batch_cnt].sensor_gain;
      manual_update.lux_idx = frame_info->frame_batch[curr_batch_cnt].lux_idx;
      manual_update.exif_iso = frame_info->frame_batch[curr_batch_cnt].iso;
      AEC_HIGH("AEUPD: STREAMON-MAN_AEC_UP: SOF ID=%d G=%f, lc=%u, lux_idx %f, exif_iso: %u",
        private->cur_sof_id, manual_update.sensor_gain, manual_update.linecount, manual_update.lux_idx,
        manual_update.exif_iso);
      private->stats_frame_capture.streamon_update_done = TRUE;
      aec_port_send_event(port, MCT_EVENT_MODULE_EVENT,
        MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE,
        (void *)(&manual_update), 0);
    }
    private->hdr_stats_sync.ae_3hdr_stats_frame_id = 0;
    private->hdr_stats_sync.bg_stats_frame_id = 0;

    rc = aec_port_get_ledfd_fd_capability(port);
  }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF: {
    mct_stream_info_t *stream_info =
      (mct_stream_info_t*)event->u.ctrl_event.control_event_data;

    aec_port_print_bus("STMOFF:STATE", private);
    aec_send_bus_message(port, MCT_BUS_MSG_SET_AEC_STATE,
      &private->aec_state, sizeof(cam_ae_state_t), private->cur_sof_id );
    STATS_MEMSET(&private->aec_get_data, 0, sizeof(private->aec_get_data));
    if (private->aec_on_off_mode == FALSE) {
      /* Reset only for HAL3: on stream-off reset manual params */
      private->aec_on_off_mode = TRUE; /* This variable is not used by HAL1 */
      private->aec_auto_mode = AEC_AUTO;
    }
    private->stats_frame_capture.frame_capture_mode = FALSE;
    private->stats_frame_capture.streamon_update_done = FALSE;
    private->frame_capture_update.u.output[0].stats_update.aec_update.flash_hal =
     CAM_FLASH_MODE_OFF;
    private->frame_capture_update.u.output[1].stats_update.aec_update.flash_hal =
     CAM_FLASH_MODE_OFF;

    /* Reset flash exposure settings for RAW and normal non-zsl snapshot.
     * For ZSL, its handled in MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT */
    if (stream_info && (stream_info->stream_type == CAM_STREAM_TYPE_RAW ||
        stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT)) {
      q3a_thread_aecawb_msg_t *aec_msg_get_hdr_type = aec_port_create_msg(MSG_AEC_SET,
        AEC_SET_PARAM_LED_RESET, private);
      if(private->snapshot_hdr && stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT) break;
      aec_port_reset_output_index(private);
      q3a_thread_aecawb_msg_t *aec_msg_led_reset = aec_port_create_msg(MSG_AEC_SET,
        AEC_SET_PARAM_LED_RESET, private);
      if (aec_msg_led_reset != NULL) {
        rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg_led_reset);
      }
      AEC_LOW("StreamOff for RAW StreamType Reset LED");
    }

  } /* MCT_EVENT_CONTROL_STREAMOFF */
    break;

  case MCT_EVENT_CONTROL_SET_PARM: {
    /* TODO: some logic shall be handled by stats and q3a port to achieve that,
     * we need to add the function to find the desired sub port;
     * however since it is not in place, for now, handle it here
     **/
    stats_set_params_type *stat_parm =
      (stats_set_params_type *)mod_ctrl->control_event_data;
    if (stat_parm->param_type == STATS_SET_Q3A_PARAM) {
      q3a_set_params_type *q3a_param = &(stat_parm->u.q3a_param);
      if (q3a_param->type == Q3A_SET_AEC_PARAM) {
        q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
          q3a_param->u.aec_param.type, private);
        if (aec_msg != NULL ) {
          aec_msg->u.aec_set_parm = q3a_param->u.aec_param;
          aec_msg->u.aec_set_parm.camera_id = private->camera_id;
          /* for some events we need to peak here */
          switch(q3a_param->u.aec_param.type) {
          case AEC_SET_PARAM_LOCK: {
            if (private->locked_from_hal == q3a_param->u.aec_param.u.aec_lock) {
              AEC_LOW("AEC_SET_PARAM_LOCK: %d, same hal value, do not update",
                 private->locked_from_hal);
              rc = FALSE;
            } else {
              private->locked_from_hal = q3a_param->u.aec_param.u.aec_lock;
              AEC_LOW("AEC_SET_PARAM_LOCK: %d new value, update algo",
                 private->locked_from_hal);
            }
          }
            break;

          case AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT: {
            private->aec_trigger.trigger =
              q3a_param->u.aec_param.u.aec_trigger.trigger;
            private->aec_trigger.trigger_id =
              q3a_param->u.aec_param.u.aec_trigger.trigger_id;
            if (q3a_param->u.aec_param.u.aec_trigger.trigger ==
              AEC_PRECAPTURE_TRIGGER_START) {
              if (private->locked_from_hal) {
                AEC_HIGH("Ignoring precapture AEC is locked!!!");
                rc = FALSE;
                break;
              }

              AEC_LOW(" SET Prepare SNAPSHOT");
              private->aec_trigger.trigger_id =
                q3a_param->u.aec_param.u.aec_trigger.trigger_id;
              private->aec_precap_start = TRUE;
              if (private->aec_precap_for_af) {
                /* Ignore precapture sequence in the AEC algo since it is
                 * already running for the AF estimation */
                AEC_LOW("Ignoring precapture trigger in the algo!!!");
                q3a_param->u.aec_param.u.aec_trigger.trigger =
                  AEC_PRECAPTURE_TRIGGER_IDLE;
              } else {
                /* start the preflash case */

                /* send the max current info to algo */
                aec_port_send_max_flash_current_to_algo(port);
              }
            } else if (q3a_param->u.aec_param.u.aec_trigger.trigger ==
              AEC_PRECAPTURE_TRIGGER_CANCEL) {
              AEC_LOW("CAM_INTF_META_AEC_PRECAPTURE_TRIGGER CANCEL");
              q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
                AEC_SET_PARAM_LED_RESET, private);
              if (aec_msg != NULL) {
                rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
              }
            }
          }
            break;

          case AEC_SET_PARAM_PREP_FOR_SNAPSHOT_NOTIFY: {
            private->aec_trigger.trigger =
              q3a_param->u.aec_param.u.aec_trigger.trigger;
            if(q3a_param->u.aec_param.u.aec_trigger.trigger ==
              AEC_PRECAPTURE_TRIGGER_START) {
              private->aec_trigger.trigger_id =
                q3a_param->u.aec_param.u.aec_trigger.trigger_id;

              AEC_LOW(" Ignoring precapture trigger (notify) in the algo!!!");
            }
          }
            break;

          case AEC_SET_PARAM_PRECAPTURE_START: {
            private->aec_precap_start = TRUE;

            /* we got a precapture trigger right after LED AF, the AEC preflash sequence won't run
             * again in this case per HAL3 spec, we need to send out the CAM_AE_STATE_PRECAPTURE
             * state to HAL on receiving AEC_TRIGGER, here we set the aec_precap_start flag to true
             * and also aec_reset_precap_start_flag to true, in this way, we could reset the
             * aec_precap_start after we send out the CAM_AE_STATE_PRECAPTURE, so the state machine
             * could move the state to converged otherwise, the state will be stuck at
             * CAM_AE_STATE_PRECAPTURE due to aec_precap_start flag is true and preflash is not
             * running
             *
             * if est_state ==  AEC_EST_START means we are currently undergoing pre-flash, the
             * aec_precap_start will be reset upon pre-flash done.
             *
             */
            if (private->est_state != AEC_EST_START) {
              private->aec_reset_precap_start_flag = true;
            }
            AEC_LOW("EC_SET_PARAM_PRECAPTURE_START");
          }
            break;

          case AEC_SET_PARAM_MANUAL_EXP_TIME: { /* HAL3 */
            /*convert nano sec to sec*/
            private->manual.exp_time = (float)q3a_param->u.aec_param.u.manual_expTime/1000000000;
            private->manual.is_exp_time_valid = TRUE;
          }
          break;
          /* HAL1 manual AEC: set exposure time */
          case AEC_SET_PARAM_EXP_TIME: { /* HAL1 */
            private->manual.exp_time =
              (float)q3a_param->u.aec_param.u.manual_exposure_time.value/1000000000;
            if (q3a_param->u.aec_param.u.manual_exposure_time.value !=
                AEC_MANUAL_EXPOSURE_TIME_AUTO) {
              private->manual.is_exp_time_valid = TRUE;
              private->aec_auto_mode = AEC_PARTIAL_AUTO;
            } else {
              if (!private->manual.is_gain_valid) {
                private->aec_auto_mode = AEC_AUTO;
              }
              private->manual.is_exp_time_valid = FALSE;
            }
          }
          break;

          /*HAL1 manual AEC: set iso mode / continuous iso */
          case AEC_SET_PARAM_ISO_MODE: {
            if (q3a_param->u.aec_param.u.iso.value != AEC_ISO_AUTO &&
                q3a_param->u.aec_param.u.iso.value != AEC_ISO_DEBLUR) {
              private->aec_auto_mode = AEC_PARTIAL_AUTO;
              private->manual.gain = private->aec_object.iso_to_real_gain
                (private->aec_object.aec,
                (uint32_t)q3a_param->u.aec_param.u.iso.value,
                private->camera_id);
              if (private->manual.gain) {
                private->manual.is_gain_valid = TRUE;
              } else {
                private->manual.is_gain_valid = FALSE;
                AEC_HIGH("Error getting ISO to real gain");
              }
            } else {
              /* Setting back to auto iso */
              private->manual.is_gain_valid = FALSE;
              if (!private->manual.is_exp_time_valid) {
                private->aec_auto_mode = AEC_AUTO;
              }
            }
          }
          break;
          case AEC_SET_PARAM_MANUAL_GAIN: {
            /* Convert from ISO to real gain*/
            private->manual.gain = (float)q3a_param->u.aec_param.u.manual_gain * private->ISO100_gain / 100.0;
            if (private->manual.gain < 1) {
              AEC_LOW("AEC_SET_PARAM_MANUAL_GAIN: ISO lower that expected: %d, using min gain",
                 q3a_param->u.aec_param.u.manual_gain);
              private->manual.gain = 1.0;
            }
            private->manual.is_gain_valid = TRUE;
          }
          break;
          case AEC_SET_PARAM_ON_OFF: {
            private->aec_on_off_mode = q3a_param->u.aec_param.u.enable_aec;
            aec_port_set_aec_mode(private);
            AEC_HIGH("AEDBG,ON_OFF_MODE=%d, Meta mode set=%d",
              private->aec_on_off_mode, private->aec_meta_mode);
          }
           break;
         case AEC_SET_PARAM_EXP_COMPENSATION:
           /* Set exposure compensation param in the same frame, sync_flag TRUE */
           aec_msg->sync_flag = TRUE;
           private->exp_comp = q3a_param->u.aec_param.u.exp_comp;
           break;
         case AEC_SET_PARAM_LED_MODE:
           private->led_mode = q3a_param->u.aec_param.u.led_mode;

           /* In HAL3, flash snapshot settings are stored in port side and
            * once capture intent recieved, settings are sent on next SOF.
            * But if LED mode is OFF after Touch LED AF, then reset the flag
            * on port side to ensure that led OFF settings are sent.*/
           if (private->led_mode == LED_MODE_OFF) {
              private->still.is_flash_snap_data = FALSE;
           }
           break;
         case AEC_SET_PARAM_FPS:
           private->fps = q3a_param->u.aec_param.u.fps;
           AEC_LOW("SET FPS: max = %d min = %d", private->fps.max_fps, private->fps.min_fps);
           break;
         case AEC_SET_PARAM_SENSOR_ROI:
           aec_port_update_roi(private, q3a_param->u.aec_param.u.aec_roi);
           break;
         case AEC_SET_PARM_FAST_AEC_DATA:
           private->fast_aec_data = q3a_param->u.aec_param.u.fast_aec_data;
           break;
         case AEC_SET_PARAM_EFFECT:
           private->adrc_settings.effect_mode=
             q3a_param->u.aec_param.u.effect_mode;
           rc = aec_port_set_adrc_enable(private);
           break;
         case AEC_SET_PARAM_ADRC_FEATURE_DISABLE_FROM_APP:
           private->adrc_settings.adrc_force_disable =
             q3a_param->u.aec_param.u.adrc_enable?FALSE:TRUE;
           rc = aec_port_set_adrc_enable(private);
           break;
         case AEC_SET_PARAM_CONV_SPEED: {
           if (private->aec_auto_mode == AEC_MANUAL) {
             private->conv_speed = q3a_param->u.aec_param.u.conv_speed;
           }
         }
           break;
         default: {
          }
           break;
          }
          if (rc) {
            rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
            if (aec_msg) {
              if (TRUE == aec_msg->sync_flag) {
              free(aec_msg);
              }
            }
          } else {
            AEC_LOW("MCT_EVENT_CONTROL_SET_PARM: Skiped %d",
               q3a_param->u.aec_param.type);
            free(aec_msg);
            rc = TRUE;
          }
        }
      } else if (q3a_param->type == Q3A_ALL_SET_PARAM) {
        switch (q3a_param->u.q3a_all_param.type) {
        case Q3A_ALL_SET_EZTUNE_RUNNIG: {
          q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
            AEC_SET_PARAM_EZ_TUNE_RUNNING, private);
          if (aec_msg != NULL ) {
            aec_msg->u.aec_set_parm.type = AEC_SET_PARAM_EZ_TUNE_RUNNING;
            aec_msg->u.aec_set_parm.u.ez_running =
              q3a_param->u.q3a_all_param.u.ez_runnig;
            rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
          }
        }
          break;

        case Q3A_ALL_SET_DO_LED_EST_FOR_AF: {
          int i = 0;
          const boolean previous_state = private->aec_precap_for_af;
          const boolean new_state = q3a_param->u.q3a_all_param.u.est_for_af;
          const boolean use_sync = (TRUE == previous_state &&
            FALSE == new_state) ? TRUE : FALSE;
          if (private->est_state == AEC_EST_START) {
            private->aec_precap_for_af = new_state;
          } else {

            q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
              AEC_SET_PARAM_DO_LED_EST_FOR_AF, private);
            if (aec_msg == NULL ) {
              break;
            }

            if (new_state) {
                /* will do preflash for AF, send the max current info to algo */
                aec_port_send_max_flash_current_to_algo(port);
            }

            /* Update estimation for AF start/end in core */
            aec_msg->is_priority = TRUE;
            aec_msg->sync_flag = use_sync;
            aec_msg->u.aec_set_parm.u.est_for_af = new_state;
            rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
            if (rc) {
              private->aec_precap_for_af = new_state;
            }

            /* Indicates AF is cancelled or completed, reset the internal LED state.
             * This is needed to ensure estimation state is updated correctly for
             * back to back Touch LED AF sequence */
            if (use_sync) {
              AEC_LOW("AEC EST state change: Old=%d New=%d", private->est_state, AEC_EST_OFF);
              if (private->est_state != AEC_EST_NO_LED_DONE) {
                MCT_OBJECT_LOCK(port);
                private->est_state = AEC_EST_OFF;

                for (i = 0; i < AEC_OUTPUT_ARRAY_MAX_SIZE; i++) {
                  private->state_update.u.output[i].stats_update.aec_update.est_state = AEC_EST_OFF;
                  private->state_update.u.output[i].stats_update.aec_update.led_state= 0;
                }
                MCT_OBJECT_UNLOCK(port);
              }
              free(aec_msg);
            }
          }
        }
          break;

        case Q3A_ALL_SET_LED_CAL: {
          rc = aec_port_set_led_cal(port,
            (boolean)q3a_param->u.q3a_all_param.u.led_cal_enable);
        }
          break;

        case Q3A_ALL_SET_CONFIG_AE_SCAN_TEST: {
          q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
            AEC_SET_PARAM_CONFIG_AE_SCAN_TEST, private);
          if (aec_msg != NULL ) {
            aec_msg->u.aec_set_parm.type = AEC_SET_PARAM_CONFIG_AE_SCAN_TEST;
            aec_msg->u.aec_set_parm.u.ae_scan_test_config.test_config=
              q3a_param->u.q3a_all_param.u.ae_scan_test_config.test_config;
            aec_msg->u.aec_set_parm.u.ae_scan_test_config.test_config_freq=
              q3a_param->u.q3a_all_param.u.ae_scan_test_config.test_config_freq;
            rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
          }
        }
          break;

        case Q3A_ALL_SET_EZ_DUAL_LED_FORCE_IDX: {
          q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
            AEC_SET_PARAM_EZ_FORCE_DUAL_LED_IDX, private);
          if (aec_msg != NULL ) {
            aec_msg->u.aec_set_parm.u.ez_force_dual_led_idx =
              q3a_param->u.q3a_all_param.u.ez_force_dual_led_idx;
            rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
          }
        }
          break;

        default: {
        }
          break;
        }
      }
    }
    /* If it's common params shared by many modules */
    else if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
      stats_common_set_parameter_t *common_param = &(stat_parm->u.common_param);
      if (common_param->type == COMMON_SET_PARAM_BESTSHOT ||
        common_param->type == COMMON_SET_PARAM_VIDEO_HDR ||
        common_param->type == COMMON_SET_PARAM_SNAPSHOT_HDR ||
        common_param->type == COMMON_SET_PARAM_STATS_DEBUG_MASK ||
        common_param->type == COMMON_SET_PARAM_STREAM_ON_OFF ||
        common_param->type == COMMON_SET_PARAM_INSTANT_AEC_DATA ||
        common_param->type == COMMON_SET_PARAM_FD) {
        q3a_thread_aecawb_msg_t *aec_msg = NULL;
        AEC_HIGH("AEDBG,Commn,type=%d",common_param->type);
        switch(common_param->type) {
        case COMMON_SET_PARAM_BESTSHOT: {
          aec_msg = aec_port_create_msg(MSG_AEC_SET,AEC_SET_PARAM_BESTSHOT,
            private);
          if (aec_msg != NULL ) {
            aec_port_set_bestshot_mode(&aec_msg->u.aec_set_parm.u.bestshot_mode,
              common_param->u.bestshot_mode);
            private->adrc_settings.bestshot_mode =
              common_param->u.bestshot_mode;
            rc = aec_port_set_adrc_enable(private);
          }
        }
          break;

        case COMMON_SET_PARAM_VIDEO_HDR: {
          aec_msg = aec_port_create_msg(MSG_AEC_SET,AEC_SET_PARAM_VIDEO_HDR,
            private);
          if (aec_msg != NULL ) {
            aec_port_set_video_hdr_mode(&private->video_hdr,
                common_param->u.video_hdr);
            aec_msg->u.aec_set_parm.u.video_hdr = private->video_hdr;
          }
        }
          break;

        case COMMON_SET_PARAM_SNAPSHOT_HDR: {
          aec_msg = aec_port_create_msg(MSG_AEC_SET,AEC_SET_PARAM_SNAPSHOT_HDR,
            private);
          if (aec_msg != NULL ) {
            aec_snapshot_hdr_type snapshot_hdr;
            if (common_param->u.snapshot_hdr == CAM_SENSOR_HDR_IN_SENSOR)
              snapshot_hdr = AEC_SENSOR_HDR_IN_SENSOR;
            else if (common_param->u.snapshot_hdr == CAM_SENSOR_HDR_ZIGZAG)
              snapshot_hdr = AEC_SENSOR_HDR_DRC;
            else if (common_param->u.snapshot_hdr == CAM_SENSOR_HDR_STAGGERED)
              snapshot_hdr = AEC_SENSOR_HDR_STAGGERED;
            else if (common_param->u.snapshot_hdr == CAM_SENSOR_3EXP_HDR_IN_SENSOR)
              snapshot_hdr = AEC_SENSOR_HDR_IN_SENSOR;
            else
              snapshot_hdr = AEC_SENSOR_HDR_OFF;

            if (common_param->u.snapshot_hdr == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
              private->is_3hdr_enabled = TRUE;
            }else {
              private->is_3hdr_enabled = FALSE;
            }

            aec_msg->u.aec_set_parm.u.snapshot_hdr = snapshot_hdr;
            private->snapshot_hdr = snapshot_hdr;
            rc = aec_port_set_adrc_enable(private);
            }
          }
            break;

        case COMMON_SET_PARAM_STATS_DEBUG_MASK: {
          //do nothing.
        }
          break;
        case COMMON_SET_PARAM_STREAM_ON_OFF: {
          AEC_LOW("Stream_On_Off: stream_on=%d fast_aec_data.enable=%d",
            common_param->u.stream_on, private->fast_aec_data.enable);
          if (!common_param->u.stream_on) {
            aec_port_unlink_from_peer(port, event);
          }
          if (!common_param->u.stream_on && !private->fast_aec_data.enable) {
            aec_port_reset_output_index(private);
          }
        }
          break;
        case COMMON_SET_PARAM_INSTANT_AEC_DATA: {
          aec_msg = aec_port_create_msg(MSG_AEC_SET, AEC_SET_PARM_INSTANT_AEC_DATA,
            private);
          if (aec_msg != NULL ) {
            aec_msg->u.aec_set_parm.u.instant_aec_type = common_param->u.instant_aec_type;
            private->instant_aec_type = common_param->u.instant_aec_type;
          }
        }
          break;
        case COMMON_SET_PARAM_FD: {
          private->led_fd_settings.is_fd_enable_from_ui = common_param->u.fd_enabled;
        }
          break;
        default: {
        }
          break;
        }

        if (aec_msg != NULL ) {
          rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
        }
      } else if (common_param->type == COMMON_SET_PARAM_META_MODE) {
        private->aec_meta_mode = common_param->u.meta_mode;
        aec_port_set_aec_mode(private);
        AEC_HIGH("AEDBG,META_MODE,In=%d,Out=%d",
          common_param->u.meta_mode, private->aec_meta_mode);

        if ((common_param->u.meta_mode != CAM_CONTROL_OFF &&
              private->aec_meta_mode == CAM_CONTROL_OFF) ||
            (common_param->u.meta_mode == CAM_CONTROL_OFF &&
              private->aec_meta_mode != CAM_CONTROL_OFF)) {
          /* pass the meta mode change only when switch into/out of manual mode */
          q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
            AEC_SET_PARAM_CTRL_MODE, private);
          if (aec_msg == NULL) {
             break;
          }

          aec_msg->u.aec_set_parm.u.aec_ctrl_mode = common_param->u.meta_mode;
          rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
        }
      } else if(common_param->type == COMMON_SET_CAPTURE_INTENT) {
        if (common_param->u.capture_type == CAM_INTENT_STILL_CAPTURE) {
          AEC_HIGH("AEDBG, CAPTURE_INTENT: sof_id: %d, stats: %d, aec_state: %d",
            private->cur_sof_id, private->cur_stats_id, private->aec_state);

          private->still.is_capture_intent = TRUE;
          if (private->still.is_flash_snap_data) {
            /* Skip stats under flash exposure */
            aec_set_skip_stats(private, private->cur_sof_id, STATS_FLASH_DELAY +
              STATS_FLASH_ON);
            /* Reset to algo to LED OFF */
            q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
              AEC_SET_PARAM_LED_RESET, private);
            if (aec_msg != NULL) {
              rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
            }
            /* Reset precapture trigger ID*/
            private->aec_trigger.trigger = 0;
            private->aec_trigger.trigger_id = 0;
          }
        }
      } else if (common_param->type == COMMON_SET_PARAM_UNIFIED_FLASH) {
        cam_capture_frame_config_t *frame_info =
         (cam_capture_frame_config_t *)&common_param->u.frame_info;
        if (frame_info->num_batch != 0 &&
            private->stats_frame_capture.frame_capture_mode) {
          AEC_LOW("frame_capture in progress, don't process %d, num_batch %d",
            private->stats_frame_capture.frame_capture_mode, frame_info->num_batch);
          break;
        }
        AEC_HIGH("AEDBG: UNIFIED_FLASH");
        STATS_MEMSET(&private->stats_frame_capture.frame_info, 0, sizeof(aec_frame_batch_t));
        aec_port_common_set_unified_flash(private, frame_info);
      } else if (common_param->type == COMMON_SET_PARAM_LONGSHOT_MODE) {
        q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
          AEC_SET_PARAM_LONGSHOT_MODE, private);
        if (aec_msg == NULL) {
          break;
        }
        private->in_longshot_mode = common_param->u.longshot_mode;
        aec_msg->u.aec_set_parm.u.longshot_mode = private->in_longshot_mode;
        AEC_HIGH("AEDBG: longshot_mode: %d", private->in_longshot_mode);
        if (aec_msg != NULL) {
          rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
        }
      }
    }
  }
  break;

  case MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT: {
    /* Lock AEC update only during the ZSL snapshot */
    private->in_zsl_capture = TRUE;

    STATS_MEMSET(&private->aec_get_data, 0, sizeof(private->aec_get_data));
    /*TO do: moving the get exposure handling to here */
  }
    break;
 case MCT_EVENT_CONTROL_SET_SUPER_PARM: {
   private->super_param_id = event->u.ctrl_event.current_frame_id;
  }
    break;

  case MCT_EVENT_CONTROL_MASTER_INFO: {
    AEC_LOW("AECDualCam - received MCT_EVENT_CONTROL_MASTER_INFO");
    aec_port_handle_role_switch(port, event);
   }
     break;

  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
    AEC_LOW("AECDualCam - received MCT_EVENT_CONTROL_LINK_INTRA_SESSION");
    aec_port_link_to_peer(port, event);
  }
    break;

  case MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION: {
    AEC_LOW("AECDualCam - received MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION");
    aec_port_unlink_from_peer(port, event);
  }
    break;

  case MCT_EVENT_CONTROL_HW_WAKEUP:
  case MCT_EVENT_CONTROL_HW_SLEEP: {
    aec_port_update_LPM(port,
      (cam_dual_camera_perf_control_t*)event->u.ctrl_event.control_event_data);
  }
    break;
  default: {
  }
    break;
  }
  AEC_LOW("X rc = %d", rc);

  return rc;
}

/** aec_port_handle_aec_update
 *    @port:   TODO

 * Return nothing
 **/

static void aec_port_handle_aec_update(mct_port_t * port)
{
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);

  stats_get_data_t stats_get_data ;
  STATS_MEMSET(&stats_get_data, 0, sizeof(stats_get_data_t));
  STATS_MEMSET(&private->aec_get_data, 0, sizeof(private->aec_get_data));

 /* query exposure parameters from core */
  aec_port_proc_get_aec_data(port, &stats_get_data);

  private->aec_get_data = stats_get_data.aec_get;
}

/** aec_port_handle_asd_update:
 *    @thread_data: TODO
 *    @mod_evt:     TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static void aec_port_handle_asd_update(aec_port_private_t *private,
  q3a_thread_data_t *thread_data,
  mct_event_module_t *mod_evt)
{
  aec_set_asd_param_t *asd_parm;
  stats_update_t      *stats_event;

  stats_event = (stats_update_t *)(mod_evt->module_event_data);

  AEC_LOW("Handle ASD update!");
  q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
    AEC_SET_PARAM_ASD_PARM, private);
  if (aec_msg == NULL) {
    return;
  }
  asd_parm = &(aec_msg->u.aec_set_parm.u.asd_param);
  asd_parm->backlight_detected = stats_event->asd_update.backlight_detected;
  asd_parm->backlight_luma_target_offset =
    stats_event->asd_update.backlight_luma_target_offset;
  asd_parm->snow_or_cloudy_scene_detected =
    stats_event->asd_update.snow_or_cloudy_scene_detected;
  asd_parm->snow_or_cloudy_luma_target_offset =
    stats_event->asd_update.snow_or_cloudy_luma_target_offset;
  asd_parm->landscape_severity = stats_event->asd_update.landscape_severity;
  asd_parm->soft_focus_dgr = stats_event->asd_update.asd_soft_focus_dgr;
  asd_parm->enable = stats_event->asd_update.asd_enable;
  AEC_LOW("backling_detected: %d offset: %d snow_detected: %d offset: %d"
    "landscape_severity: %d soft_focus_dgr: %f",
    asd_parm->backlight_detected, asd_parm->backlight_luma_target_offset,
    asd_parm->snow_or_cloudy_scene_detected,
    asd_parm->snow_or_cloudy_luma_target_offset,
    asd_parm->landscape_severity, asd_parm->soft_focus_dgr);
  q3a_aecawb_thread_en_q_msg(thread_data, aec_msg);
}

/** aec_port_handle_afd_update:
 *    @thread_data: TODO
 *    @mod_evt:     TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static void aec_port_handle_afd_update(aec_port_private_t *private,
  q3a_thread_data_t *thread_data,
  mct_event_module_t *mod_evt)
{
  aec_set_afd_parm_t *aec_afd_parm;
  stats_update_t     *stats_event;

  stats_event = (stats_update_t *)(mod_evt->module_event_data);
  q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
    AEC_SET_PARAM_AFD_PARM, private);
  if (aec_msg == NULL) {
    return;
  }

  aec_afd_parm = &(aec_msg->u.aec_set_parm.u.afd_param);
  aec_msg->is_priority = TRUE;
  aec_afd_parm->afd_enable = stats_event->afd_update.afd_enable;


  switch (stats_event->afd_update.afd_atb) {
  case AFD_TBL_OFF:
    aec_afd_parm->afd_atb =  STATS_PROC_ANTIBANDING_OFF;
    break;

  case AFD_TBL_60HZ:
    aec_afd_parm->afd_atb = STATS_PROC_ANTIBANDING_60HZ;
    break;

  case AFD_TBL_50HZ:
    aec_afd_parm->afd_atb = STATS_PROC_ANTIBANDING_50HZ;
    break;

  default:
    aec_afd_parm->afd_atb =  STATS_PROC_ANTIBANDING_OFF;
    break;
  }
  aec_afd_parm->afd_fps_mode = AEC_ANTIBANDING_FPS_DEFAULT;
  q3a_aecawb_thread_en_q_msg(thread_data, aec_msg);
}

/** aec_port_proc_get_aec_data:
 *    @port:           TODO
 *    @stats_get_data: TODO
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_port_proc_get_aec_data(mct_port_t *port,
  stats_get_data_t *stats_get_data)
{
  boolean rc = FALSE;
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  stats_custom_update_t *custom_data = (stats_custom_update_t *)
    &stats_get_data->aec_get.aec_get_custom_data;

  /* Update exposure settings in manual mode */
  if (private->aec_auto_mode == AEC_MANUAL) {
    aec_get_t *out = &stats_get_data->aec_get;
    float new_sensitivity;
    STATS_MEMSET(out, 0, sizeof(aec_get_t));

    /* General exposure information */
    out->sensor_gain[0] = private->manual.gain;
    out->real_gain[0] = private->manual.gain;
    out->linecount[0] = (1.0 * private->manual.exp_time *
                      private->sensor_info.pixel_clock /
                      private->sensor_info.pixel_clock_per_line);
    if (out->linecount[0] < 1) {
       out->linecount[0] = 1;
    }
    out->exp_time = private->manual.exp_time;
    new_sensitivity = out->sensor_gain[0] * out->linecount[0];
    out->lux_idx = log10(new_sensitivity/ private->init_sensitivity)/ log10(1.03);

    /* Disable ADRC in MANUAL mode */
    out->total_drc_gain = -1.0f;
    out->color_drc_gain = -1.0f;
    out->gtm_ratio = -1.0f;
    out->ltm_ratio = -1.0f;
    out->la_ratio = -1.0f;
    out->gamma_ratio = -1.0f;

    /* Update meta info */
    STATS_MEMSET(&private->aec_info, 0, sizeof(cam_3a_params_t));
    private->aec_info.exp_time  = out->exp_time;
    private->aec_info.real_gain = out->real_gain[0];
    private->aec_info.iso_value = (int32_t)(out->real_gain[0] * 100 / private->ISO100_gain);
    private->aec_info.settled   = 1;

    /* Print all bus messages info */
    AEC_HIGH("AEUPD: GETDATA-AEC_MANUAL: G=%02.3f,LC=%04d,LI=%03.3f,ET=%f",
      out->sensor_gain[0], out->linecount[0], out->lux_idx,out->exp_time);
    aec_port_print_bus("GETDATA:INFO_MANUAL", private);
    aec_send_bus_message(port, MCT_BUS_MSG_AE_INFO,
      &private->aec_info, sizeof(cam_3a_params_t), private->cur_sof_id);
  }

  /* Update previous exposure settings in auto mode */
  else if (private->aec_get_data.valid_entries) {
    stats_get_data->aec_get = private->aec_get_data;
    stats_get_data->flag = STATS_UPDATE_AEC;
  }

  /* Update new exposure settings with values from core */
  else {
    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_GET,
      AEC_GET_PARAM_EXPOSURE_PARAMS, private);

    if (aec_msg) {
      aec_exp_parms_t *from_core = &aec_msg->u.aec_get_parm.u.exp_params;
      aec_get_t *out = &stats_get_data->aec_get;
      aec_msg->sync_flag = TRUE;
      AEC_LOW("in payload %p size %d", custom_data->data, custom_data->size);
      if (custom_data->data && custom_data->size > 0) {
        from_core->custom_param.data = custom_data->data;
        from_core->custom_param.size = custom_data->size;
      }
      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
      if (TRUE == rc) {
        uint32_t i;
        uint32_t tmp_cur_sof_id = private->cur_sof_id;
        char str [1024] = "";
        stats_get_data->flag = STATS_UPDATE_AEC;

        /* Copy general exposure information */
        out->valid_entries = from_core->valid_exp_entries;
        for (i=0; i < out->valid_entries; i++) {
          out->real_gain[i] = from_core->real_gain[i];
          out->sensor_gain[i] = from_core->sensor_gain[i];
          out->linecount[i] = from_core->linecount[i];
        }
        out->lux_idx = from_core->lux_idx;
        out->exp_time = from_core->exp_time[0];

        /* Copy ADRC information */
        out->total_drc_gain = from_core->drc_gains.total_drc_gain;
        out->color_drc_gain = from_core->drc_gains.color_drc_gain;
        out->gtm_ratio = from_core->drc_gains.gtm_ratio;
        out->ltm_ratio = from_core->drc_gains.ltm_ratio;
        out->la_ratio = from_core->drc_gains.la_ratio;
        out->gamma_ratio = from_core->drc_gains.gamma_ratio;

        /* Copy HDR information */
        out->l_linecount = from_core->hdr_gains.l_linecount;
        out->l_real_gain = from_core->hdr_gains.l_gain;
        out->m_linecount = from_core->hdr_gains.m_linecount;
        out->m_real_gain = from_core->hdr_gains.m_gain;
        out->s_linecount = from_core->hdr_gains.s_linecount;
        out->s_real_gain = from_core->hdr_gains.s_gain;
        out->hdr_sensitivity_ratio = from_core->hdr_gains.hdr_sensitivity_ratio;
        out->hdr_exp_time_ratio = from_core->hdr_gains.hdr_exp_time_ratio;

        /* Copy Flash information */
        out->led_off_real_gain = from_core->led_off_real_gain;
        out->led_off_sensor_gain = from_core->led_off_sensor_gain;
        out->led_off_total_drc_gain = from_core->led_off_drc_gains.total_drc_gain;
        out->led_off_color_drc_gain = from_core->led_off_drc_gains.color_drc_gain;
        out->led_off_gtm_ratio = from_core->led_off_drc_gains.gtm_ratio;
        out->led_off_ltm_ratio = from_core->led_off_drc_gains.ltm_ratio;
        out->led_off_la_ratio = from_core->led_off_drc_gains.la_ratio;
        out->led_off_gamma_ratio = from_core->led_off_drc_gains.gamma_ratio;
        out->led_off_linecount = from_core->led_off_linecount;
        out->trigger_led = from_core->use_led_estimation;
        out->flash_sensitivity.off = from_core->flash_sensitivity.off;
        out->flash_sensitivity.low = from_core->flash_sensitivity.low;
        out->flash_sensitivity.high = from_core->flash_sensitivity.high;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x0310)
        aec_core_dual_led_settings_t* dual_led = &from_core->dual_led_setting;
        out->dual_led_setting.is_valid = true;
        out->dual_led_setting.led1_low_setting = dual_led->low_setting.LED1_setting;
        out->dual_led_setting.led2_low_setting = dual_led->low_setting.LED2_setting;
        out->dual_led_setting.led1_high_setting = dual_led->high_setting.LED1_setting;
        out->dual_led_setting.led2_high_setting = dual_led->high_setting.LED2_setting;
        AEC_LOW("low (led1,led2) : (%d, %d), high (led1,led2) : (%d, %d)",
          dual_led->low_setting.LED1_setting,dual_led->low_setting.LED2_setting,
          dual_led->high_setting.LED1_setting, dual_led->high_setting.LED2_setting);
#endif

        MCT_OBJECT_LOCK(port);

        private->min_gain = ((float)from_core->min_gain / Q8);
        private->min_line_count = from_core->min_line_count;
        /* Update the exposure settings for manual 3A */
        if (private->stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
          private->aec_update_flag = FALSE;
          tmp_cur_sof_id = STATS_REPORT_IMMEDIATE;

          if (private->manual.is_exp_time_valid || private->manual.is_gain_valid) {
            float real_gain = out->real_gain[0];
            float sensor_gain = out->sensor_gain[0];
            uint32_t linecount = out->linecount[0];
            float exp_time = out->exp_time;

            AEC_LOW("Before Manual Adjust: RG=%f, SG=%f, LC=%d, ET=%f",
              real_gain, sensor_gain, linecount, exp_time);

            /* HAL 1 manual 3A applies only to non-ZSL snapshot */
            /* In Manual AEC, ADRC is disabled*/
            aec_port_cal_manual_mode_exp_param(private, real_gain, linecount, exp_time,
              &out->real_gain[0], &out->sensor_gain[0], &out->linecount[0], &out->exp_time);
          }

          private->aec_get_data = stats_get_data->aec_get;
        } else if(private->in_zsl_capture == TRUE) {
          private->aec_get_data = stats_get_data->aec_get;
        }
        MCT_OBJECT_UNLOCK(port);

        for (i = 0, str[0] = 0; i < out->valid_entries; i++) {
          snprintf(str + strnlen(str, 1023), 1023, "[%d](RG %f, SG %f, LC %d) ", i,
          out->real_gain[i], out->sensor_gain[i],
          out->linecount[i]);
        }

        AEC_HIGH("AEUPD: GETDATA-AEC: %s DRC Gain(%f, %f),"
          " DRC Ratios(%f, %f, %f, %f), lux=%f, ET=%f valid_entry=%d",
          str, out->total_drc_gain,
          out->color_drc_gain, out->gtm_ratio, out->ltm_ratio, out->la_ratio,
          out->gamma_ratio,out->lux_idx, out->exp_time, out->valid_entries);

        /* Update exif information */
        STATS_MEMSET(&private->aec_info, 0, sizeof(cam_3a_params_t));
        private->aec_info.exp_time = out->exp_time;
        private->aec_info.real_gain = out->real_gain[0];
        private->aec_info.iso_value = (out->real_gain[0] * 100) / private->ISO100_gain;
        private->aec_info.flash_needed = from_core->flash_needed;
        private->aec_info.metering_mode =
          aec_port_get_hal_metering_mode(from_core->metering_type);
        private->aec_info.scenetype = 0x1;

        /* Print all bus messages info */
        aec_port_print_bus("GETDATA:INFO", private);
        aec_send_bus_message(port, MCT_BUS_MSG_AE_INFO,
          &private->aec_info, sizeof(cam_3a_params_t), tmp_cur_sof_id );
      }
      free(aec_msg);
    } else {
      AEC_ERR("Not enough memory");
    }
  }

  return rc;
}

static void aec_hdr_stats_send_event(aec_port_private_t *private)
{
  /* TODO: add lock for the send event arbitration*/
  boolean send_immediately = FALSE;
  /* stats sync*/
  if((private->hdr_stats_sync.ae_3hdr_stats_frame_id == private->hdr_stats_sync.bg_stats_frame_id) &&
      (private->hdr_stats_sync.stats_type_mask == (STATS_3HDR | STATS_BG_AEC))) {//debug
    private->hdr_stats_sync.stats_type_mask &= ~(STATS_3HDR | STATS_BG_AEC);
    send_immediately = TRUE;
  }else if(private->hdr_stats_sync.ae_3hdr_stats_frame_id > private->hdr_stats_sync.bg_stats_frame_id) {
    /* clear bg stats */
    private->hdr_stats_sync.stats_type_mask &= ~STATS_BG_AEC;
    send_immediately = FALSE;
  }else if(private->hdr_stats_sync.ae_3hdr_stats_frame_id < private->hdr_stats_sync.bg_stats_frame_id) {
    /* clear bg stats */
    private->hdr_stats_sync.stats_type_mask &= ~STATS_3HDR;
    send_immediately = FALSE;
  }else {
    send_immediately = FALSE;
  }
  AEC_HIGH("ae_3hdr_stats_frame_id=%d, bg_stats_frame_id=%d, ae_hist_stats_type_mask=0x%x, send_immediately=%d",
      private->hdr_stats_sync.ae_3hdr_stats_frame_id,
      private->hdr_stats_sync.bg_stats_frame_id,
      private->hdr_stats_sync.stats_type_mask,
      send_immediately);

  /* send stats to core*/
  if(send_immediately == TRUE) {
    q3a_thread_aecawb_msg_t *aec_msg =
        aec_port_create_msg(MSG_AEC_STATS_HDR, AEC_UNDEF, private);
    stats_t *aec_stats = (stats_t *)calloc(1, sizeof(stats_t));
    aec_stats->camera_id = private->camera_id;
    aec_stats->stats_type_mask = STATS_3HDR | STATS_BG_AEC;
    aec_stats->frame_id = private->hdr_stats_sync.bg_stats_frame_id;
    aec_stats->bayer_stats.p_q3a_bg_stats = private->hdr_stats_sync.p_q3a_bg_stats;
    aec_stats->hdr3_stats.stats_hdr3_hist_data = private->hdr_stats_sync.p_q3a_3hdr_stats->hdr3_hist_data;
    aec_stats->hdr3_stats.hdr3_sensor_type = private->hdr_stats_sync.p_q3a_3hdr_stats->hdr3_sensor_type;
    aec_msg->u.stats = aec_stats;
    q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  }
}
/** aec_port_handle_vhdr_buf:
 *    @port:  TODO
 *    @event: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static void aec_port_handle_vhdr_buf( aec_port_private_t *private, isp_buf_divert_t *hdr_stats_buff )
{
    q3a_thread_aecawb_msg_t *aec_msg =
      aec_port_create_msg(MSG_AEC_STATS_HDR, AEC_UNDEF, private);
    if (aec_msg == NULL ) {
      AEC_ERR("Not enough memory");
      return;
    }

    stats_t *aec_stats = (stats_t *)calloc(1, sizeof(stats_t));
    if (aec_stats == NULL) {
      AEC_ERR("Not enough memory");
      free(aec_msg);
      return;
    }

    if(private->is_3hdr_enabled == TRUE) {
      aec_port_parse_RDI_stats_AE(private, (uint32_t *)private->hdr_stats_sync.p_q3a_3hdr_stats, hdr_stats_buff->vaddr);
      private->hdr_stats_sync.ae_3hdr_stats_frame_id = hdr_stats_buff->buffer.sequence;
      private->hdr_stats_sync.stats_type_mask |= STATS_3HDR;
      aec_hdr_stats_send_event(private);
    }else {
      aec_stats->yuv_stats.p_q3a_aec_stats =
        (q3a_aec_stats_t*)calloc(1, sizeof(q3a_aec_stats_t));
      if (aec_stats->yuv_stats.p_q3a_aec_stats == NULL) {
        AEC_ERR("Not enough memory");
        free(aec_stats);
        free(aec_msg);
        return;
      }

      AEC_LOW("aec_msg=%p, aec_stats=%p, yuv_stats:%p HDR stats", aec_msg,
        aec_stats, aec_stats->yuv_stats.p_q3a_aec_stats);
      aec_msg->u.stats = aec_stats;
      aec_stats->camera_id = private->camera_id;
      aec_stats->stats_type_mask |= STATS_HDR_VID;
      aec_stats->yuv_stats.p_q3a_aec_stats->ae_region_h_num = 16;
      aec_stats->yuv_stats.p_q3a_aec_stats->ae_region_v_num = 16;
      aec_stats->frame_id = hdr_stats_buff->buffer.sequence;
      aec_port_parse_RDI_stats_AE(private, (uint32_t *)aec_stats->yuv_stats.p_q3a_aec_stats->SY,
        hdr_stats_buff->vaddr);

      q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);

    }

}

/** aec_port_is_handle_stats_required
 *    @private: private aec port data
 *    @stats_mask: type of stats provided
 *
 * Return: TRUE if stats required
 **/
static boolean aec_port_is_handle_stats_required(aec_port_private_t *private,
  uint32_t stats_mask)
{
  if (private->stream_type == CAM_STREAM_TYPE_SNAPSHOT  ||
    private->stream_type == CAM_STREAM_TYPE_RAW) {
    return FALSE;
  }
  /* skip stats in ZSL Flash capture */
  if( (private->in_zsl_capture == TRUE) && (private->in_longshot_mode == FALSE) ){
    AEC_LOW("Skipping STATS in ZSL mode");
    return FALSE;
  }

  if(!aec_port_is_algo_active(private)) {
    AEC_LOW("AECStats-[mode-%d role-%d] Not being handled",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    return FALSE;
  }

  /* skip stats for Unified Flash capture */
  if (private->stats_frame_capture.frame_capture_mode) {
    AEC_LOW("Skipping STATS in Unified Flash mode");
    return FALSE;
  }

  if (!((stats_mask & (1 << MSM_ISP_STATS_BHIST)) ||
    (stats_mask & (1 << MSM_ISP_STATS_HDR_BHIST)) ||
    (stats_mask & (1 << MSM_ISP_STATS_BG)) ||
    (stats_mask & (1 << MSM_ISP_STATS_AEC_BG)) ||
    (stats_mask & (1 << MSM_ISP_STATS_AEC)) ||
    (stats_mask & (1 << MSM_ISP_STATS_BE)) ||
    (stats_mask & (1 << MSM_ISP_STATS_HDR_BE)) ||
    (stats_mask & (1 << MSM_ISP_STATS_IHIST)))) {
    return FALSE;
  }
  /* We don't want to process ISP stats if video HDR mode is ON */
  if (((stats_mask & (1 << MSM_ISP_STATS_BG)) ||
    (stats_mask & (1 << MSM_ISP_STATS_AEC_BG)) ||
    (stats_mask & (1 << MSM_ISP_STATS_AEC))) &&
    aec_port_using_HDR_divert_stats(private)) {
    return FALSE;
  }

  return TRUE;
}

/** aec_port_handle_stats_data
 *    @port: MCT port data
 *    @event: MCT event data
 *
 * Return: TRUE if no error
 **/
static boolean aec_port_handle_stats_data(mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  mct_event_module_t *mod_evt = &(event->u.module_event);
  mct_event_stats_ext_t *stats_ext_event;
  mct_event_stats_isp_t *stats_event;
  uint32_t aec_stats_mask = 0;

  stats_ext_event = (mct_event_stats_ext_t *)(mod_evt->module_event_data);
  if (!stats_ext_event || !stats_ext_event->stats_data) {
    return rc;
  }
  stats_event = stats_ext_event->stats_data;

  /* Filter by the stats that algo has requested */
  aec_stats_mask = stats_event->stats_mask & private->required_stats_mask;

  /* copy bg stats only*/
  if(aec_stats_mask & (1 << MSM_ISP_STATS_AEC_BG) &&
      aec_port_using_HDR_divert_stats(private)) {
    private->hdr_stats_sync.bg_stats_frame_id = stats_event->frame_id;
    private->hdr_stats_sync.stats_type_mask |= STATS_BG_AEC;
    STATS_MEMCPY(private->hdr_stats_sync.p_q3a_bg_stats, sizeof(q3a_bg_stats_t),
      stats_event->stats_data[MSM_ISP_STATS_AEC_BG].stats_buf, sizeof(q3a_bg_stats_t));
    aec_hdr_stats_send_event(private);
  }

  AEC_LOW("event stats_mask = 0x%x, AEC requested stats = 0x%x",
    stats_event->stats_mask, aec_stats_mask);
  if (!aec_port_is_handle_stats_required(private, aec_stats_mask)) {
    return TRUE; /* Non error case */
  }

  if (is_aec_stats_skip_required(private, stats_event->frame_id)) {
    AEC_LOW("skip stats: %d", stats_event->frame_id);
    return TRUE; /* Non error case */
  }

  q3a_thread_aecawb_msg_t *aec_msg =
    aec_port_create_msg(AEC_UNDEF, AEC_UNDEF, private);
  if (aec_msg == NULL) {
    return rc;
  }
  stats_t *aec_stats = (stats_t *)calloc(1, sizeof(stats_t));
  if (aec_stats == NULL) {
    free(aec_msg);
    return rc;
  }

  aec_msg->u.stats = aec_stats;
  aec_stats->camera_id = private->camera_id;
  aec_stats->stats_type_mask = 0;
  aec_stats->frame_id = stats_event->frame_id;
  aec_stats->time_stamp.time_stamp_sec = stats_event->timestamp.tv_sec;
  aec_stats->time_stamp.time_stamp_us  = stats_event->timestamp.tv_usec;
  aec_stats->sof_timestamp.time_stamp_sec = stats_event->sof_timestamp.tv_sec;
  aec_stats->sof_timestamp.time_stamp_us  = stats_event->sof_timestamp.tv_usec;

  if (aec_stats_mask & (1 << MSM_ISP_STATS_AEC)) {
    aec_msg->type = MSG_AEC_STATS;
    aec_stats->stats_type_mask |= STATS_AEC;
    aec_stats->yuv_stats.p_q3a_aec_stats =
      stats_event->stats_data[MSM_ISP_STATS_AEC].stats_buf;
    rc = TRUE;
  } else if (aec_stats_mask & (1 << MSM_ISP_STATS_BG)) {
    aec_stats->stats_type_mask |= STATS_BG;
    aec_msg->type = MSG_BG_AEC_STATS;
    aec_stats->bayer_stats.p_q3a_bg_stats =
      stats_event->stats_data[MSM_ISP_STATS_BG].stats_buf;
    rc = TRUE;
  } else if (aec_stats_mask & (1 << MSM_ISP_STATS_AEC_BG)) {
    aec_stats->stats_type_mask |= STATS_BG_AEC;
    aec_msg->type = MSG_BG_AEC_STATS;
    aec_stats->bayer_stats.p_q3a_bg_stats =
      stats_event->stats_data[MSM_ISP_STATS_AEC_BG].stats_buf;
    rc = TRUE;
   } else if (aec_stats_mask & (1 << MSM_ISP_STATS_HDR_BE)) {
    aec_stats->stats_type_mask |= STATS_HDR_BE;
    aec_msg->type = MSG_HDR_BE_AEC_STATS;
    aec_stats->bayer_stats.p_q3a_hdr_be_stats =
      stats_event->stats_data[MSM_ISP_STATS_HDR_BE].stats_buf;
    rc = TRUE;
  } else if (aec_stats_mask & (1 << MSM_ISP_STATS_BE)) {
    aec_stats->stats_type_mask |= STATS_BE;
    aec_msg->type = MSG_BE_AEC_STATS;
    aec_stats->bayer_stats.p_q3a_be_stats =
      stats_event->stats_data[MSM_ISP_STATS_BE].stats_buf;
    rc = TRUE;
  }

  /* Ensure BG or AEC stats are preset to propagate to AEC algorithm.
  If it is missing then ignore the composite stats */
  if (rc && aec_stats_mask & (1 << MSM_ISP_STATS_HDR_BHIST)) {
    aec_stats->stats_type_mask |= STATS_HBHISTO;
    aec_stats->bayer_stats.p_q3a_bhist_stats =
      stats_event->stats_data[MSM_ISP_STATS_HDR_BHIST].stats_buf;
  } else if (rc && aec_stats_mask & (1 << MSM_ISP_STATS_BHIST)) {
    aec_stats->stats_type_mask |= STATS_BHISTO;
    aec_stats->bayer_stats.p_q3a_bhist_stats =
      stats_event->stats_data[MSM_ISP_STATS_BHIST].stats_buf;
  }
  if (rc && aec_stats_mask & (1 << MSM_ISP_STATS_IHIST)) {
    aec_stats->yuv_stats.p_q3a_ihist_stats =
      stats_event->stats_data[MSM_ISP_STATS_IHIST].stats_buf;
  }
  if (!rc) {
    free(aec_stats);
    free(aec_msg);
    return rc;
  }
  uint32_t cur_stats_id = 0;
  uint32_t cur_sof_id = 0;

  MCT_OBJECT_LOCK(port);
  cur_stats_id = private->cur_stats_id = stats_event->frame_id;
  cur_sof_id = private->cur_sof_id;
  AEC_LOW("AEC_UPDATE_DBG: IN STATS_DATA: cam-role=%d,cam-mode:%d Curr: SOF_ID:%d Stats_ID:%d",
    private->dual_cam_info.cam_role, private->dual_cam_info.mode,
    private->cur_sof_id, private->cur_stats_id);
  MCT_OBJECT_UNLOCK(port);

  if (aec_msg->type == MSG_BG_AEC_STATS ||
    aec_msg->type == MSG_BE_AEC_STATS ||
    aec_msg->type == MSG_HDR_BE_AEC_STATS) {
    aec_stats->ack_data = stats_ext_event;
    circular_stats_data_use(stats_ext_event);
  }

  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  /* If the aecawb thread is inactive, it will not enqueue our
  * message and instead will free it. Then we need to manually
  * free the payload */
  if (rc == FALSE) {
    if (aec_msg->type == MSG_BG_AEC_STATS ||
    aec_msg->type == MSG_BE_AEC_STATS ||
    aec_msg->type == MSG_HDR_BE_AEC_STATS) {
      circular_stats_data_done(stats_ext_event, 0, 0, 0);
    }
    /* In enqueue fail, memory is free inside q3a_aecawb_thread_en_q_msg() *
     * Return back from here */
    aec_stats = NULL;
    return rc;
  }

  return rc;
}

/** aec_port_unified_flash_trigger:
 *    @port:  mct port type containing aec port private data
 *
 * The first call to this function, will set-up unified capture sequence.
 *
 * Return: TRUE on success
 **/
static boolean aec_port_unified_flash_trigger(mct_port_t *port)
{
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  aec_frame_batch_t *frame_info = &private->stats_frame_capture.frame_info;
  int i = 0;

  if (0 == frame_info->num_batch) {
    AEC_ERR("No. of num_batch is zero");
    return FALSE;
  }

  if (FALSE == private->stats_frame_capture.frame_capture_mode) {
    /* Start unified sequence */
    private->stats_frame_capture.current_batch_count = 0;
    private->stats_frame_capture.frame_capture_mode = TRUE;
  } else {
    private->stats_frame_capture.current_batch_count++;
    AEC_LOW("Incremented Current Batch no. =%d",
      private->stats_frame_capture.current_batch_count);
    if (private->stats_frame_capture.current_batch_count >
        private->stats_frame_capture.frame_info.num_batch - 1) {
      private->stats_frame_capture.current_batch_count =
        private->stats_frame_capture.frame_info.num_batch - 1;
      AEC_HIGH("Limit the batch count to HAL value: %d, cur batch cnt: %d",
        private->stats_frame_capture.frame_info.num_batch,
        private->stats_frame_capture.current_batch_count);
    }
  }
  return TRUE;
}

/** aec_port_parse_sensor_info
 *
 *    @private: private AEC data
 *    @aec_msg: q3a thread msg to be fill
 *    @sensor_info: Use this data to fill msg
 *
 * Return TRUE on success.
 **/
static boolean aec_port_parse_sensor_info(aec_port_private_t *private,
  q3a_thread_aecawb_msg_t *aec_msg,
  sensor_out_info_t *sensor_info)
{
  /* TBG to change to sensor */
  float fps, max_fps;

  private->parse_RDI_stats =
      (sensor_RDI_parser_func_t)sensor_info->parse_RDI_statistics;

  private->max_sensor_delay = sensor_info->sensor_immediate_pipeline_delay +
    sensor_info->sensor_additive_pipeline_delay;

  fps = sensor_info->max_fps * 0x00000100;

  /*max fps supported by sensor*/
  max_fps = (float)sensor_info->vt_pixel_clk * 0x00000100 /
    (float)(sensor_info->ll_pck * sensor_info->fl_lines);

  /* Sanity check*/
  if (fps <= 0) {
    AEC_ERR(" Sensor fps is 0!!");
    /* default to 30*/
    fps = 30 * 256;
  }
  switch (private->stream_type) {
  case CAM_STREAM_TYPE_VIDEO: {
    aec_msg->u.aec_set_parm.u.init_param.op_mode = AEC_OPERATION_MODE_CAMCORDER;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.video_fps = fps;
    aec_msg->u.aec_set_parm.u.init_param.op_mode = AEC_OPERATION_MODE_CAMCORDER;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.video_fps = fps;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.preview_linesPerFrame =
      sensor_info->fl_lines;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.max_preview_fps = max_fps;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.preview_fps = fps;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.prev_max_line_cnt =
      sensor_info->max_linecount;
  }
    break;

  case CAM_STREAM_TYPE_PREVIEW:
  case CAM_STREAM_TYPE_CALLBACK: {
    aec_msg->u.aec_set_parm.u.init_param.op_mode = AEC_OPERATION_MODE_PREVIEW;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.preview_linesPerFrame =
      sensor_info->fl_lines;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.max_preview_fps = max_fps;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.preview_fps = fps;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.prev_max_line_cnt =
      sensor_info->max_linecount;
  }
    break;

  case CAM_STREAM_TYPE_RAW:
  case CAM_STREAM_TYPE_SNAPSHOT: {
    aec_msg->u.aec_set_parm.u.init_param.op_mode = AEC_OPERATION_MODE_SNAPSHOT;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.snap_linesPerFrame =
      sensor_info->fl_lines;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.snap_max_line_cnt =
      sensor_info->max_linecount;
    aec_msg->u.aec_set_parm.u.init_param.sensor_info.snapshot_fps = fps;
  }
    break;

  default: {
  }
    break;
  } /* switch (private->stream_type) */

  aec_msg->u.aec_set_parm.u.init_param.sensor_info.max_gain =
    sensor_info->max_gain;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.current_fps = fps;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.pixel_clock =
    sensor_info->vt_pixel_clk;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.pixel_clock_per_line =
    sensor_info->ll_pck;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.sensor_res_height =
    sensor_info->request_crop.last_line -
    sensor_info->request_crop.first_line + 1;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.sensor_res_width =
    sensor_info->request_crop.last_pixel -
    sensor_info->request_crop.first_pixel + 1;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.pixel_sum_factor =
    sensor_info->pixel_sum_factor;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.f_number =
    sensor_info->af_lens_info.f_number;
  /*Update sensor long and short capabilities*/
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.svhdr_use_separate_gain =
    sensor_info->svhdr_use_separate_gain;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.svhdr_use_separate_limits =
    sensor_info->svhdr_use_separate_limits;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.min_line_cnt[0] =
    sensor_info->min_line_cnt[0];
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.min_line_cnt[1] =
    sensor_info->min_line_cnt[1];
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.max_line_cnt[0] =
    sensor_info->max_line_cnt[0];
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.max_line_cnt[1] =
    sensor_info->max_line_cnt[1];

  /* Set pre-fill sensor_info */
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.sensor_type =
    private->sensor_info.sensor_type;
  aec_msg->u.aec_set_parm.u.init_param.sensor_info.lens_type =
    private->sensor_info.lens_type;

  /* Save all sensor info */
  STATS_MEMCPY(&private->sensor_info, sizeof(aec_sensor_info_t),
        &aec_msg->u.aec_set_parm.u.init_param.sensor_info, sizeof(aec_sensor_info_t));

  /* Initialize ROI as disable*/
  aec_interested_region_t roi;
  STATS_MEMSET(&roi, 0, sizeof(roi));
  aec_port_update_roi(private, roi);

  return TRUE;
}

/** aec_port_proc_downstream_event:
 *    port:  TODO
 *    event: TODO
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_port_proc_downstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean            rc = TRUE;
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  mct_event_module_t *mod_evt = &(event->u.module_event);
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;

  /* Check if extended handling to be performed */
  if (private->func_tbl.ext_handle_module_event) {
    ret = private->func_tbl.ext_handle_module_event(port, mod_evt);
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      AEC_LOW("Module event handled in extension function!");
      return TRUE;
    }
  }

  switch (mod_evt->type) {
  case MCT_EVENT_MODULE_STATS_AECAWB_GET_THREAD_OBJECT: {
    q3a_thread_aecawb_get_port_data_t *aecawb_get_data =
      (q3a_thread_aecawb_get_port_data_t *)(mod_evt->module_event_data);

    /* Getting data for thread */
    aecawb_get_data->camera.aec_port     = port;
    aecawb_get_data->camera.aec_cb       = aec_port_callback;
    aecawb_get_data->camera.aec_stats_cb = aec_port_stats_done_callback;
    aecawb_get_data->camera.aec_obj      = &(private->aec_object);

    rc = TRUE;
  }
    break;

  case MCT_EVENT_MODULE_STATS_AECAWB_SET_THREAD_OBJECT: {
    q3a_thread_aecawb_set_port_data_t *aecawb_set_data =
      (q3a_thread_aecawb_set_port_data_t *)(mod_evt->module_event_data);

    /* Set thread to be use to en_q msg*/
    private->camera_id    = aecawb_set_data->camera_id;
    private->thread_data  = aecawb_set_data->awbaec_thread_data;

    AEC_HIGH("Set thread object: camera_id: %u, algo: %p",
      private->camera_id, private->aec_object.aec);
    rc = TRUE;
  }
    break;

  case MCT_EVENT_MODULE_STATS_EXT_DATA: {
    rc = aec_port_handle_stats_data(port, event);
  }
    break;

  case MCT_EVENT_MODULE_ISP_DIVERT_TO_3A:
  case MCT_EVENT_MODULE_BUF_DIVERT: {
    isp_buf_divert_t        *stats_buff =
      (isp_buf_divert_t *)mod_evt->module_event_data;
    if(stats_buff->stats_type != HDR_STATS){
      /* Only AEC module only handles VHDR buffer */
      break;
    }
    /*TODO: property for 3exp ratio */
    char value[PROPERTY_VALUE_MAX];
    float ratio;
    /* ratio can be overwritten via setprops */
    property_get("persist.vendor.camera.HDR.ratio", value, "0.0");
    ratio = atof(value);
    if(ratio != 0.0f) {
      q3a_thread_aecawb_msg_t *aec_msg_set_ratio = NULL;
      aec_msg_set_ratio = aec_port_create_msg(MSG_AEC_SET, AEC_SET_PARAM_HDR_RATIO,
        private);
      if (!aec_msg_set_ratio) {
        AEC_ERR("Fail to allocate AEC_SET_PARAM_HDR_RATIO");
        break;
      }
      aec_msg_set_ratio->u.aec_set_parm.u.hdr_ratio = ratio;
      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg_set_ratio);
      if (!rc) {
        AEC_ERR("Fail to en_q AEC_SET_PARAM_HDR_RATIO");
        break;
      }
    }

    aec_port_handle_vhdr_buf(private, stats_buff);
    /*Since this buffer is consumed in same thread context,
        * piggy back  buffer by setting ack_flag to TRUE*/
    stats_buff->ack_flag = TRUE;
    stats_buff->is_buf_dirty = TRUE;
  }
    break;

  case MCT_EVENT_MODULE_SET_CHROMATIX_WRP: {
    chromatix_3a_parms_wrapper *chromatix_wrapper;
    q3a_thread_aecawb_msg_t *aec_msg;

    private->isp_infosync = AEC_PORT_ISP_INFO_SYNC_RESET;

    chromatix_wrapper = (chromatix_3a_parms_wrapper *)mod_evt->module_event_data;
    if(!chromatix_wrapper) {
      rc = FALSE;
      break;
    }

    aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_INIT_CHROMATIX_SENSOR, private);
    if (!aec_msg) {
      rc = FALSE;
      break;
    }

    AEC_LOW("aec_msg=%p,SET_CHROMATIX", aec_msg);
      switch (private->stream_type) {
      case CAM_STREAM_TYPE_VIDEO: {
        aec_msg->u.aec_set_parm.u.init_param.op_mode =
          AEC_OPERATION_MODE_CAMCORDER;
      }
        break;

      case CAM_STREAM_TYPE_PREVIEW:
      case CAM_STREAM_TYPE_CALLBACK: {
        aec_msg->u.aec_set_parm.u.init_param.op_mode =
          AEC_OPERATION_MODE_PREVIEW;
      }
        break;

      case CAM_STREAM_TYPE_RAW:
      case CAM_STREAM_TYPE_SNAPSHOT: {
        aec_msg->u.aec_set_parm.u.init_param.op_mode =
          AEC_OPERATION_MODE_SNAPSHOT;
      }
        break;

      default: {
      }
        break;
      } /* switch (private->stream_type) */

      AEC_LOW("stream_type=%d op_mode=%d",
        private->stream_type, aec_msg->u.aec_set_parm.u.init_param.op_mode);

      aec_msg->u.aec_set_parm.u.init_param.chromatix = (void*)chromatix_wrapper;
      aec_msg->u.aec_set_parm.u.init_param.warm_start.stored_params = private->stored_params;
      aec_msg->u.aec_set_parm.u.init_param.iot_config.reserved = NULL;/*Reserved for future*/

      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
      pthread_rwlock_rdlock(&chromatix_wrapper->lock);
      private->init_sensitivity =
         (chromatix_wrapper->AEC.aec_exposure_table.exposure_entries[0].gain /256.0) *
         chromatix_wrapper->AEC.aec_exposure_table.exposure_entries[0].line_count;
      private->ISO100_gain =
        chromatix_wrapper->AEC.aec_generic.ISO100_gain;
      private->fast_aec_forced_cnt = chromatix_wrapper->AEC.aec_generic.fast_aec_forced_cnt;

      if (private->fast_aec_data.num_frames == 0) {
        private->fast_aec_forced_cnt = 0;
      } else if (private->fast_aec_forced_cnt == 0 ||
        private->fast_aec_forced_cnt >= private->fast_aec_data.num_frames) {
        private->fast_aec_forced_cnt = private->fast_aec_data.num_frames - 2;
      }

      /* Read the Dual Camera exposure multiplier from reserved tuning parameters */
      float multiplier = chromatix_wrapper->AEC.aec_generic.dual_cam_exposure_mult;
      if (multiplier != 0.0f) {
        /* change the ratio from master/slave to salve/master */
        private->dual_cam_info.exp_multiplier_tuning =  1.0f / multiplier;
      } else {
        private->dual_cam_info.exp_multiplier_tuning = 1.0f;
      }
      AEC_HIGH("AECDualCam[role-%d]: Tuned Exposure Multiplier: %f",
        private->dual_cam_info.cam_role,
        private->dual_cam_info.exp_multiplier_tuning);

      char value[PROPERTY_VALUE_MAX];
      /* Multiplier can be overwritten via setprops */
      property_get("persist.vendor.camera.dual.expmult", value, "0.0");
      multiplier = atof(value);
      if (multiplier != 0.0f){
        if (CAM_ROLE_TELE == private->dual_cam_info.cam_role && multiplier > 0.0f) {
          multiplier = 1.0f / multiplier;
        }
        private->dual_cam_info.exp_multiplier_tuning = multiplier;
        AEC_HIGH("AECDualCam[role-%d]: Tuned Exposure Multiplier: %f - overwritten via setprops & disabling OTP data",
                 private->dual_cam_info.cam_role,
                 private->dual_cam_info.exp_multiplier_tuning);
        /* set OTP data to false to force the setprop value to be read */
        private->dual_cam_info.otp_data_valid = FALSE;
      }

      /* For now use a static table - this may move to tuning headers in the future */
      private->dual_cam_info.tuning_table[0].color_temperature  = 6500;    //D65
      private->dual_cam_info.tuning_table[0].compensation_ratio = chromatix_wrapper->AEC.aec_generic.dual_cam_comp_ratio[0];
      private->dual_cam_info.tuning_table[1].color_temperature  = 5000;    //D50
      private->dual_cam_info.tuning_table[1].compensation_ratio = chromatix_wrapper->AEC.aec_generic.dual_cam_comp_ratio[1];
      private->dual_cam_info.tuning_table[2].color_temperature  = 4500;    //CW
      private->dual_cam_info.tuning_table[2].compensation_ratio = chromatix_wrapper->AEC.aec_generic.dual_cam_comp_ratio[2];
      private->dual_cam_info.tuning_table[3].color_temperature  = 4200;    //TL84
      private->dual_cam_info.tuning_table[3].compensation_ratio = chromatix_wrapper->AEC.aec_generic.dual_cam_comp_ratio[3];
      private->dual_cam_info.tuning_table[4].color_temperature  = 3200;    //A
      private->dual_cam_info.tuning_table[4].compensation_ratio = chromatix_wrapper->AEC.aec_generic.dual_cam_comp_ratio[4];
      private->dual_cam_info.tuning_table[5].color_temperature  = 2600;    //Horizon
      private->dual_cam_info.tuning_table[5].compensation_ratio = chromatix_wrapper->AEC.aec_generic.dual_cam_comp_ratio[5];
      int start_index = chromatix_wrapper->AEC.aec_generic.aec_start_index;
      private->dual_cam_info.init_linecount = chromatix_wrapper->AEC.aec_exposure_table.exposure_entries[start_index].line_count;
      pthread_rwlock_unlock(&chromatix_wrapper->lock);
      AEC_HIGH("DualCamera Exposure Multiplier: %f", private->dual_cam_info.exp_multiplier_tuning);

      AEC_HIGH("Fast_AEC: forced cnt %d num_frames %d",
        private->fast_aec_forced_cnt, private->fast_aec_data.num_frames);

      /* Loading led FD tuning parameters */
      private->led_fd_settings.is_led_fd_enable_from_tuning =
        chromatix_wrapper->AEC.aec_led_fd.enable_led_fd_aec;
      private->led_fd_settings.led_fd_tuned_frame_count =
        chromatix_wrapper->AEC.aec_led_fd.led_fd_tuned_frame_count;

      /* Get max gain from aec exp table*/
      uint16_t exposure_entries = (uint16_t)chromatix_wrapper->AEC.aec_exposure_table.valid_entries;
      private->max_gain_aec_exp_table = chromatix_wrapper->AEC.aec_exposure_table.exposure_entries[exposure_entries - 1].gain / 256.0f;

      private->cur_stats_id = 0;
      private->cur_sof_id = 0;
  } /* case MCT_EVENT_MODULE_SET_CHROMATIX_PTR */
    break;

  case MCT_EVENT_MODULE_PREVIEW_STREAM_ID: {
    mct_stream_info_t  *stream_info =
      (mct_stream_info_t *)(mod_evt->module_event_data);

    AEC_LOW("Preview stream-id event: stream_type: %d width: %d height: %d",
      stream_info->stream_type, stream_info->dim.width, stream_info->dim.height);

    private->preview_width = stream_info->dim.width;
    private->preview_height = stream_info->dim.height;
    private->stream_identity = stream_info->identity;
  }
    break;

  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    sensor_out_info_t       *sensor_info =
      (sensor_out_info_t *)(mod_evt->module_event_data);
    q3a_thread_aecawb_msg_t *aec_msg = NULL;
    q3a_thread_aecawb_msg_t  *dim_msg = NULL;

    aec_msg = aec_port_create_msg(MSG_AEC_SET, AEC_SET_PARAM_INIT_SENSOR_INFO,
      private);
    if (!aec_msg) {
      AEC_ERR("Fail to allocate INIT_SENSOR_INFO");
      break;
    }

    rc = aec_port_parse_sensor_info(private, aec_msg, sensor_info);
    if (!rc) {
      AEC_ERR("Fail to parse sensor info");
      free(aec_msg);
      break;
    }

    rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
    if (!rc) {
      AEC_ERR("Fail to en_q AEC_SET_PARAM_INIT_SENSOR_INFO");
      break;
    }

    /* Also send the stream dimensions for preview */
    dim_msg = aec_port_create_msg(MSG_AEC_SET, AEC_SET_PARAM_UI_FRAME_DIM, private);
    if (!dim_msg) {
      AEC_ERR(" malloc failed for dim_msg");
      break;
    }

    if ((private->stream_type == CAM_STREAM_TYPE_PREVIEW) ||
      (private->stream_type == CAM_STREAM_TYPE_CALLBACK)||
      (private->stream_type == CAM_STREAM_TYPE_VIDEO)) {
      aec_set_parameter_init_t *init_param;
      init_param = &(dim_msg->u.aec_set_parm.u.init_param);
      init_param->frame_dim.width = private->preview_width;
      init_param->frame_dim.height = private->preview_height;
      AEC_LOW("enqueue msg update ui width %d and height %d",
        init_param->frame_dim.width, init_param->frame_dim.height);

      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, dim_msg);
    } else {
      free(dim_msg);
    }
  } /* MCT_EVENT_MODULE_SET_STREAM_CONFIG*/
    break;

  case MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE: {
    stats_get_data_t *stats_get_data =
      (stats_get_data_t *)mod_evt->module_event_data;
    if (!stats_get_data) {
      AEC_ERR("failed\n");
      break;
    }

    if (TRUE == private->stats_frame_capture.streamon_update_done) {
      AEC_LOW("MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE: skip update: streamon_update_done");
      break;
    }

    if(private->stream_type == CAM_STREAM_TYPE_VIDEO)
      STATS_MEMSET(&private->aec_get_data, 0, sizeof(private->aec_get_data));
    aec_port_proc_get_aec_data(port, stats_get_data);
  } /* MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE X*/
    break;

  case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
    mct_stream_info_t *stream_info =
      (mct_stream_info_t *)(event->u.module_event.module_event_data);
    if(!stream_info) {
      AEC_ERR("failed\n");
      break;
    }

    if (stream_info->identity == private->stream_identity) {
      private->vfe_out_width  = stream_info->dim.width;
      private->vfe_out_height = stream_info->dim.height;

      AEC_LOW("Output dimension from ISP received. (%d %d)",
        private->vfe_out_width,
        private->vfe_out_height);
      private->isp_infosync |= AEC_PORT_ISP_INFO_SYNC_VFE_DIM_INIT;
      rc = aec_port_send_crop_info(private);
    }
  }
  break;

  case MCT_EVENT_MODULE_STREAM_CROP: {
    mct_bus_msg_stream_crop_t *stream_crop =
      (mct_bus_msg_stream_crop_t *)mod_evt->module_event_data;

    if (!stream_crop) {
      AEC_ERR("failed\n");
      break;
    }
    private->crop_info.pp_x           = stream_crop->x;
    private->crop_info.pp_y           = stream_crop->y;
    private->crop_info.pp_crop_out_x  = stream_crop->crop_out_x;
    private->crop_info.pp_crop_out_y  = stream_crop->crop_out_y;
    private->crop_info.vfe_map_x      = stream_crop->x_map;
    private->crop_info.vfe_map_y      = stream_crop->y_map;
    private->crop_info.vfe_map_width  = stream_crop->width_map;
    private->crop_info.vfe_map_height = stream_crop->height_map;
    private->crop_info.vfe_out_width  = private->vfe_out_width;
    private->crop_info.vfe_out_height = private->vfe_out_height;

    AEC_LOW("Crop Event from ISP received. PP (%d %d %d %d)", stream_crop->x,
      stream_crop->y, stream_crop->crop_out_x, stream_crop->crop_out_y);
    AEC_LOW("vfe map: (%d %d %d %d) vfe_out: (%d %d)", stream_crop->x_map,
      stream_crop->y_map, stream_crop->width_map, stream_crop->height_map,
      private->vfe_out_width, private->vfe_out_height);

    private->isp_infosync |= AEC_PORT_ISP_INFO_SYNC_CROP_INIT;
    rc = aec_port_send_crop_info(private);
  }
  break;

  case MCT_EVENT_MODULE_FACE_INFO: {
    mct_face_info_t *face_info = (mct_face_info_t *)mod_evt->module_event_data;
    if (!face_info) {
      AEC_ERR("error: Empty event");
      break;
    }

    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_FD_ROI, private);
    if (aec_msg != NULL) {
      uint8_t idx = 0;
      uint32_t face_count = face_info->face_count;
      if (face_count > MAX_STATS_ROI_NUM) {
        AEC_HIGH("face_count %d exceed stats roi limitation, cap to max", face_count);
        face_count = MAX_STATS_ROI_NUM;
      }
      if (face_count > MAX_ROI) {
        AEC_HIGH("face_count %d exceed max roi limitation, cap to max", face_count);
        face_count = MAX_ROI;
      }

      /* Copy original face coordinates, do the transform in aec_biz.c */
      aec_msg->u.aec_set_parm.u.fd_roi.type = ROI_TYPE_GENERAL;
      aec_msg->u.aec_set_parm.u.fd_roi.num_roi = face_count;
      for (idx = 0; idx < aec_msg->u.aec_set_parm.u.fd_roi.num_roi; idx++) {
        aec_msg->u.aec_set_parm.u.fd_roi.roi[idx].x =
          face_info->faces[idx].roi.left;
        aec_msg->u.aec_set_parm.u.fd_roi.roi[idx].y =
          face_info->faces[idx].roi.top;
        aec_msg->u.aec_set_parm.u.fd_roi.roi[idx].dx =
          face_info->faces[idx].roi.width;
        aec_msg->u.aec_set_parm.u.fd_roi.roi[idx].dy =
          face_info->faces[idx].roi.height;
      }
      if (aec_port_get_led_fd_enable(private)) {
        memcpy(&private->fd_roi, &aec_msg->u.aec_set_parm.u.fd_roi, sizeof(aec_proc_roi_info_t));
      }
      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_GET_DATA: {
    stats_get_data_t *stats_get_data =
      (stats_get_data_t *)mod_evt->module_event_data;
    if (!stats_get_data) {
      AEC_ERR("failed\n");
      break;
    }

    if (TRUE == private->stats_frame_capture.streamon_update_done) {
      AEC_LOW("MCT_EVENT_MODULE_STATS_GET_DATA: skip update: streamon_update_done");
      break;
    }
    aec_port_proc_get_aec_data(port, stats_get_data);
  }
    break;

  case MCT_EVENT_MODULE_STATS_GET_LED_DATA: {
  }
    break;

  case MCT_EVENT_MODULE_STATS_AFD_UPDATE: {
    /* Do not send the AFD update to core if AEC locked by HAL */
    if (FALSE == private->locked_from_hal) {
      aec_port_handle_afd_update(private, private->thread_data, mod_evt);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
    stats_update_t *stats_update =
      (stats_update_t *)(mod_evt->module_event_data);
    if (stats_update->flag != STATS_UPDATE_AWB) {
      break;
    }
    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_AWB_PARM, private);
    if (aec_msg != NULL ) {
      aec_set_awb_parm_t *awb_param;
      awb_param = &(aec_msg->u.aec_set_parm.u.awb_param);
      awb_param->r_gain = stats_update->awb_update.gain.r_gain;
      awb_param->g_gain = stats_update->awb_update.gain.g_gain;
      awb_param->b_gain = stats_update->awb_update.gain.b_gain;
      awb_param->unadjusted_r_gain = stats_update->awb_update.unadjusted_gain.r_gain;
      awb_param->unadjusted_g_gain = stats_update->awb_update.unadjusted_gain.g_gain;
      awb_param->unadjusted_b_gain = stats_update->awb_update.unadjusted_gain.b_gain;
      awb_param->dual_led_flux_gain = stats_update->awb_update.dual_led_flux_gain;
      if (CAM_WB_MODE_INCANDESCENT == (cam_wb_mode_type)stats_update->awb_update.wb_mode)
        awb_param->is_wb_mode_incandescent = TRUE;
      else
        awb_param->is_wb_mode_incandescent = FALSE;

      /* Handle custom parameters update (3a ext) */
      if (stats_update->awb_update.awb_custom_param_update.data &&
        stats_update->awb_update.awb_custom_param_update.size) {
        awb_param->awb_custom_param_update.data =
          malloc(stats_update->awb_update.awb_custom_param_update.size);
        if (awb_param->awb_custom_param_update.data) {
          awb_param->awb_custom_param_update.size =
            stats_update->awb_update.awb_custom_param_update.size;
          STATS_MEMCPY(awb_param->awb_custom_param_update.data,
            awb_param->awb_custom_param_update.size,
            stats_update->awb_update.awb_custom_param_update.data,
            awb_param->awb_custom_param_update.size);
        } else {
          AEC_ERR("Error: Fail to allocate memory for custom parameters");
          free(aec_msg);
          rc = FALSE;
          break;
        }
      }

      rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
      if (!rc) {
        AEC_ERR("Fail to queue msg");
      }

      private->awb_color_temp = (uint16_t)stats_update->awb_update.color_temp;
    } /* if (aec_msg != NULL ) */
  } /* MCT_EVENT_MODULE_STATS_AWB_UPDATE*/
    break;

  case MCT_EVENT_MODULE_STATS_ASD_UPDATE: {
    aec_port_handle_asd_update(private, private->thread_data, mod_evt);
  }
    break;
  case MCT_EVENT_MODULE_STATS_GYRO_STATS: {
    aec_algo_gyro_info_t *gyro_info = NULL;
    mct_event_gyro_stats_t *gyro_update =
      (mct_event_gyro_stats_t *)mod_evt->module_event_data;
    int i = 0;

    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_GYRO_INFO, private);;
    if (!aec_msg) {
      AEC_ERR("Fail to malloc gyro msg");
      break;
    }
    /* Copy gyro data now */
    gyro_info = &(aec_msg->u.aec_set_parm.u.gyro_info);
    gyro_info->q16_ready = TRUE;
    gyro_info->float_ready = TRUE;

    for (i = 0; i < 3; i++) {
      gyro_info->q16[i] = (long) gyro_update->q16_angle[i];
      gyro_info->flt[i] = (float) gyro_update->q16_angle[i] / (1 << 16);
      AEC_LOW("i: %d q16: %d flt: %f", i,
        gyro_info->q16[i], gyro_info->flt[i]);
    }
    q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  }
    break;
  case MCT_EVENT_MODULE_MODE_CHANGE: {
    /* Stream mode has changed */
    private->stream_type =
      ((stats_mode_change_event_data*)
      (event->u.module_event.module_event_data))->stream_type;
    private->reserved_id =
      ((stats_mode_change_event_data*)
      (event->u.module_event.module_event_data))->reserved_id;
  }
    break;
  case MCT_EVENT_MODULE_LED_STATE_TIMEOUT: {
    AEC_HIGH("Received LED state timeout. Reset LED state!");
    q3a_thread_aecawb_msg_t *reset_led_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_RESET_LED_EST, private);

    if (!reset_led_msg) {
      AEC_ERR("malloc failed for dim_msg");
      break;
    }
    rc = q3a_aecawb_thread_en_q_msg(private->thread_data, reset_led_msg);
  }
    break;
  case MCT_EVENT_MODULE_REQUEST_STATS_TYPE: {
    uint32_t required_stats_mask = 0;
    isp_rgn_skip_pattern rgn_skip_pattern = RGN_SKIP_PATTERN_MAX;
    mct_event_request_stats_type *stats_info =
      (mct_event_request_stats_type *)mod_evt->module_event_data;

    if (ISP_STREAMING_OFFLINE == stats_info->isp_streaming_type) {
      AEC_HIGH("AEC doesn't support offline processing yet. Returning.");
      break;
    }

    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_GET,
      AEC_GET_PARAM_REQUIRED_STATS, private);
    if (!aec_msg) {
      AEC_ERR("malloc failed for AEC_GET_PARAM_REQUIRED_STATS");
      rc = FALSE;
      break;
    }

    /* Fill msg with the supported stats data */
    aec_msg->u.aec_get_parm.u.request_stats.supported_stats_mask =
      stats_info->supported_stats_mask;
    aec_msg->u.aec_get_parm.u.request_stats.supported_rgn_skip_mask =
      stats_info->supported_rgn_skip_mask;
    /* Get the list of require stats from algo library */
    aec_msg->sync_flag = TRUE;
    rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
    required_stats_mask = aec_msg->u.aec_get_parm.u.request_stats.enable_stats_mask;
    rgn_skip_pattern =
      (isp_rgn_skip_pattern)(aec_msg->u.aec_get_parm.u.request_stats.enable_rgn_skip_pattern);
    free(aec_msg);
    aec_msg = NULL;
    if (!rc) {
      AEC_ERR("Error: fail to get required stats");
      return rc;
    }

    /* Verify if require stats are supported */
    if (required_stats_mask !=
      (stats_info->supported_stats_mask & required_stats_mask)) {
      AEC_ERR("Error: Stats not supported: 0x%x, supported stats = 0x%x",
        required_stats_mask, stats_info->supported_stats_mask);
      rc = FALSE;
      break;
    }

    /* Update query and save internally */
    stats_info->enable_stats_mask |= required_stats_mask;
    stats_info->enable_stats_parse_mask |= required_stats_mask;
    stats_info->stats_comp_grp_mask[STATS_GROUP_0] |= required_stats_mask;

    private->required_stats_mask = required_stats_mask;

    AEC_LOW("MCT_EVENT_MODULE_REQUEST_STATS_TYPE:Required AEC stats mask = 0x%x parsemask=0x%x",
      stats_info->enable_stats_mask,stats_info->enable_stats_parse_mask);

    /* Set requested tap locations. Only interested in AEC_BG */

    /* Note for 8998: AEC's HDR_BE stats will still be called as AEC_BG in stats sw,
     * since the name "HDR_BE" has been taken by ISP for other purpose, hence
     * not available for stats sw.
     */

    isp_stats_tap_loc supported_tap_mask =
      stats_info->supported_tap_location[MSM_ISP_STATS_AEC_BG];
    isp_stats_tap_loc requested_tap_mask = ISP_STATS_TAP_DEFAULT;

    if (supported_tap_mask & ISP_STATS_TAP_AFTER_LENS_ROLLOFF) {
      requested_tap_mask = ISP_STATS_TAP_AFTER_LENS_ROLLOFF;
    }
    else if (supported_tap_mask & ISP_STATS_TAP_BEFORE_LENS_ROLLOFF) {
      requested_tap_mask = ISP_STATS_TAP_BEFORE_LENS_ROLLOFF;
    }

    /* Update query and save internally */
    stats_info->requested_tap_location[MSM_ISP_STATS_AEC_BG] |= requested_tap_mask;
    STATS_MEMCPY(private->requested_tap_location,
           sizeof(private->requested_tap_location),
           stats_info->requested_tap_location,
           sizeof(uint32_t) * MSM_ISP_STATS_MAX);
    stats_info->enable_rgn_skip_pattern[MSM_ISP_STATS_AEC_BG] = rgn_skip_pattern;
  }
    break;
  case MCT_EVENT_MODULE_ISP_STATS_INFO: {
    mct_stats_info_t *stats_info =
      (mct_stats_info_t *)mod_evt->module_event_data;

    q3a_thread_aecawb_msg_t *stats_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_STATS_DEPTH, private);

    if (!stats_msg) {
      AEC_ERR("malloc failed for stats_msg");
      break;
    }
    stats_msg->u.aec_set_parm.u.stats_depth = stats_info->stats_depth;
    rc = q3a_aecawb_thread_en_q_msg(private->thread_data, stats_msg);
  }
    break;

  case MCT_EVENT_MODULE_TRIGGER_CAPTURE_FRAME: {
    AEC_LOW("MCT_EVENT_MODULE_TRIGGER_CAPTURE_FRAME!");
    rc = aec_port_unified_flash_trigger(port);
  }
    break;

  case MCT_EVENT_MODULE_STATS_LIGHT_UPDATE: {
    mct_event_light_sensor_update_t *light_update =
      (mct_event_light_sensor_update_t *)mod_evt->module_event_data;

    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_LIGHT_SENSOR_INFO, private);
    if (!aec_msg) {
      AEC_ERR("malloc failed for aec_msg");
      break;
    }

    aec_msg->u.aec_set_parm.u.light_sensor_info = light_update->lux;
    rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  }
    break;

  case MCT_EVENT_MODULE_SET_FLASH_MODE: {
    cam_flash_mode_t flash_mode =
      *(cam_flash_mode_t *)mod_evt->module_event_data;
    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_SENSOR_FLASH_MODE, private);
    if (!aec_msg) {
      AEC_ERR("malloc failed for aec_msg");
      break;
    }
    aec_port_conv_fromhal_flashmode(flash_mode,
      &aec_msg->u.aec_set_parm.u.sensor_flash_mode);
    rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  }
    break;

  default: {
  }
    break;
  } /* switch (mod_evt->type) */
  return rc;
}

/** aec_port_event:
 *    @port:  TODO
 *    @event: TODO
 *
 * aec sink module's event processing function. Received events could be:
 * AEC/AWB/AF Bayer stats;
 * Gyro sensor stat;
 * Information request event from other module(s);
 * Informatin update event from other module(s);
 * It ONLY takes MCT_EVENT_DOWNSTREAM event.
 *
 * Return TRUE if the event is processed successfully.
 **/
static boolean aec_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  aec_port_private_t *private;

  /* sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (aec_port_private_t *)(port->port_private);
  if (!private) {
    return FALSE;
  }

  /* sanity check: ensure event is meant for port with same identity*/
  if (!aec_port_check_identity(private->reserved_id, event->identity)) {
    return FALSE;
  }

  AEC_LOW("AEC_EVENT: %s Dir %d",
    event->type == MCT_EVENT_CONTROL_CMD ?
    stats_port_get_mct_event_ctrl_string(event->u.ctrl_event.type):
    (event->type == MCT_EVENT_MODULE_EVENT ?
    stats_port_get_mct_event_module_string(event->u.module_event.type):
    "INVALID EVENT"), MCT_EVENT_DIRECTION(event));

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {
    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      rc = aec_port_proc_downstream_event(port, event);
    } /* case MCT_EVENT_MODULE_EVENT */
      break;

    case MCT_EVENT_CONTROL_CMD: {
      rc = aec_port_proc_downstream_ctrl(port,event);
    }
      break;

    default: {
    }
      break;
    }
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  case MCT_EVENT_UPSTREAM: {
    mct_port_t *peer = MCT_PORT_PEER(port);
    rc = MCT_PORT_EVENT_FUNC(peer)(peer, event);
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

  return rc;
}

/** aec_port_ext_link:
 *    @identity: session id + stream id
 *    @port:  aec module's sink port
 *    @peer:  q3a module's sink port
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean            rc = FALSE;
  aec_port_private_t *private;

  /* aec sink port's external peer is always q3a module's sink port */
  if (!port || !peer ||
    strcmp(MCT_OBJECT_NAME(port), "aec_sink") ||
    strcmp(MCT_OBJECT_NAME(peer), "q3a_sink")) {
    AEC_ERR("Invalid parameters!");
    return FALSE;
  }

  private = (aec_port_private_t *)port->port_private;
  if (!private) {
    AEC_ERR("Null Private port!");
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case AEC_PORT_STATE_RESERVED:
    /* Fall through, no break */
  case AEC_PORT_STATE_UNLINKED:
    /* Fall through, no break */
  case AEC_PORT_STATE_LINKED: {
    if (!aec_port_check_identity(private->reserved_id, identity)) {
      break;
    }
  }
  /* Fall through, no break */
  case AEC_PORT_STATE_CREATED: {
    rc = TRUE;
  }
    break;

  default: {
  }
    break;
  }

  if (rc == TRUE) {
    private->state = AEC_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }
  MCT_OBJECT_UNLOCK(port);
  mct_port_add_child(identity, port);

  AEC_LOW("X rc=%d", rc);
  return rc;
}

/** aec_port_ext_unlink
 *    @identity: TODO
 *    @port:     TODO
 *    @peer:     TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static void aec_port_ext_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  aec_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer) {
    return;
  }

  private = (aec_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (private->state == AEC_PORT_STATE_LINKED &&
    aec_port_check_identity(private->reserved_id, identity)) {
    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = AEC_PORT_STATE_UNLINKED;
      private->aec_update_flag = FALSE;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  mct_port_remove_child(identity, port);

  return;
}

/** aec_port_set_caps:
 *    @port: TODO
 *    @caps: TODO
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_port_set_caps(mct_port_t *port, mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "aec_sink")) {
    return FALSE;
  }

  port->caps = *caps;
  return TRUE;
}

/** aec_port_check_caps_reserve:
 *    @port:        TODO
 *    @caps:        TODO
 *    @stream_info: TODO
 *
 *  AEC sink port can ONLY be re-used by ONE session. If this port
 *  has been in use, AEC module has to add an extra port to support
 *  any new session(via module_aec_request_new_port).
 *
 * TODO Return
 **/
boolean aec_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *info)
{
  boolean            rc = FALSE;
  mct_port_caps_t    *port_caps;
  aec_port_private_t *private;
  mct_stream_info_t *stream_info = (mct_stream_info_t *) info;

  AEC_LOW("E");

  if (!port || !caps || !stream_info ||
    strcmp(MCT_OBJECT_NAME(port), "aec_sink")) {
    return rc;
  }

  MCT_OBJECT_LOCK(port);

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (aec_port_private_t *)port->port_private;
  switch (private->state) {
  case AEC_PORT_STATE_LINKED: {
    if (aec_port_check_identity(private->reserved_id, stream_info->identity)) {
      rc = TRUE;
    }
  }
    break;

  case AEC_PORT_STATE_CREATED:
  case AEC_PORT_STATE_UNRESERVED: {
    private->reserved_id = stream_info->identity;
    private->stream_type = stream_info->stream_type;
    private->stream_info = *stream_info;
    private->state       = AEC_PORT_STATE_RESERVED;
    rc = TRUE;
  }
    break;

  case AEC_PORT_STATE_RESERVED: {
    if (aec_port_check_identity(private->reserved_id, stream_info->identity)) {
      rc = TRUE;
    }
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** aec_port_check_caps_unreserve:
 *    port:     TODO
 *    identity: TODO
 *
 * TODO description
 *
 * TODO Return
 **/
static boolean aec_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  aec_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "aec_sink")) {
    return FALSE;
  }

  private = (aec_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  if ((private->state == AEC_PORT_STATE_UNLINKED ||
    private->state == AEC_PORT_STATE_LINKED ||
    private->state == AEC_PORT_STATE_RESERVED) &&
    aec_port_check_identity(private->reserved_id, identity)) {
    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = AEC_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return TRUE;
}

/** aec_port_find_identity:
 *    @port:     TODO
 *    @identity: TODO
 *
 * TODO description
 *
 * TODO Return
 **/
boolean aec_port_find_identity(mct_port_t *port, unsigned int identity)
{
  aec_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "aec_sink")) {
    return FALSE;
  }

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000) ?
      TRUE : FALSE);
  }
  return FALSE;
}

/** aec_port_deinit
 *    @port: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
void aec_port_deinit(mct_port_t *port)
{
  aec_port_private_t *private;
  int ret;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "aec_sink")) {
    return;
  }

  private = port->port_private;
  if (!private) {
    return;
  }

  AEC_HIGH("De-init: camera: %u, aec_port: %p AEC algo: %p",
    private->camera_id,
    private, private->aec_object.aec);

  AEC_DESTROY_LOCK(&private->aec_object);
  ret = pthread_mutex_destroy(&private->update_state_lock);
  if (ret) {
    if (ret == EBUSY) {
      AEC_ERR("Unable to destroy update_state_lock, lock is acquired");
    } else if (ret == EINVAL) {
      AEC_ERR("Unable to destroy update_state_lock, lock is invalid");
    }
  }
  private->aec_object.deinit(private->aec_object.aec);
  if (private->func_tbl.ext_deinit) {
    private->func_tbl.ext_deinit(port);
  }
  if (FALSE == private->aec_extension_use) {
    aec_port_unload_function(private);
  } else {
    aec_port_ext_unload_function(private);
  }

  aec_port_free_3hdr_stats_buffer(private);

  free(private);
  private = NULL;
}

/** aec_port_update_func_table:
 *    @ptr: pointer to internal aec pointer object
 *
 * Update extendable function pointers.
 *
 * Return None
 **/
boolean aec_port_update_func_table(aec_port_private_t *private)
{
  private->func_tbl.ext_init = NULL;
  private->func_tbl.ext_deinit = NULL;
  private->func_tbl.ext_callback = NULL;
  private->func_tbl.ext_handle_module_event = NULL;
  private->func_tbl.ext_handle_control_event = NULL;
  return TRUE;
}
boolean aec_port_initialize_3hdr_stats_buffer(aec_port_private_t *private)
{
  boolean rc = TRUE;

  q3a_bg_stats_t* bg_stats = malloc(sizeof(q3a_bg_stats_t));
  if (bg_stats != NULL) {
    private->hdr_stats_sync.p_q3a_bg_stats = bg_stats;
  }else {
    AEC_ERR("Not enough memory");
    rc = FALSE;
    return rc;
  }

  q3a_3hdr_stats_t* hdr3_stats = malloc(sizeof(q3a_3hdr_stats_t));
  if (hdr3_stats != NULL) {
    private->hdr_stats_sync.p_q3a_3hdr_stats = hdr3_stats;
  }else {
    AEC_ERR("Not enough memory");
    rc = FALSE;
    return rc;
  }

  stats_hdr3_hist_data_type* hdr3_stats_buffer =
    &private->hdr_stats_sync.p_q3a_3hdr_stats->hdr3_hist_data;

  for (uint32_t channel_count = 0; channel_count < MAX_NUM_OF_3HDR_BHIST_CHANNELS; channel_count++) {
    hdr3_stats_buffer->split_bayer_hist.hist_data[channel_count] =
      (uint32* )malloc(sizeof(uint32) * MAX_NUM_OF_3HDR_BHIST_BINS);
    hdr3_stats_buffer->merged_bayer_hist.hist_data[channel_count] =
      (uint32* )malloc(sizeof(uint32) * MAX_NUM_OF_3HDR_BHIST_BINS);
    hdr3_stats_buffer->grid_bayer_stats.grid_data[channel_count] =
      (uint32* )malloc(sizeof(float) * MAX_NUM_OF_3HDR_GRID_REGIONS);

    if (hdr3_stats_buffer->split_bayer_hist.hist_data[channel_count] == NULL ||
        hdr3_stats_buffer->merged_bayer_hist.hist_data[channel_count] == NULL ||
        hdr3_stats_buffer->grid_bayer_stats.grid_data[channel_count] == NULL) {
      AEC_ERR("Not enough memory");
      rc = FALSE;
      return rc;
    }
  }

  private->is_3hdr_enabled = FALSE;

  return rc;
}

void aec_port_free_3hdr_stats_buffer(aec_port_private_t *private)
{
  stats_hdr3_hist_data_type* hdr3_stats_buffer =
    &private->hdr_stats_sync.p_q3a_3hdr_stats->hdr3_hist_data;

  for (uint32_t channel_count = 0; channel_count < MAX_NUM_OF_3HDR_BHIST_CHANNELS; channel_count++) {
    free(hdr3_stats_buffer->split_bayer_hist.hist_data[channel_count]);
    free(hdr3_stats_buffer->merged_bayer_hist.hist_data[channel_count]);
    free(hdr3_stats_buffer->grid_bayer_stats.grid_data[channel_count]);;
  }
  free(private->hdr_stats_sync.p_q3a_3hdr_stats);
  free(private->hdr_stats_sync.p_q3a_bg_stats);
}

/** aec_port_init:
 *    @port:      aec's sink port to be initialized
 *    @sessionid: TODO
 *
 *  aec port initialization entry point. Becase AEC module/port is
 *  pure software object, defer aec_port_init when session starts.
 *
 * TODO Return
 **/
boolean aec_port_init(mct_port_t *port, unsigned int *sessionid)
{
  boolean            rc = TRUE;
  mct_port_caps_t    caps;
  unsigned int       *session;
  mct_list_t         *list;
  aec_port_private_t *private;
  unsigned int       session_id = GET_SESSION_ID(*sessionid);

  if (!port || strcmp(MCT_OBJECT_NAME(port), "aec_sink")) {
    return FALSE;
  }

  private = (void *)malloc(sizeof(aec_port_private_t));
  if (!private) {
    return FALSE;
  }

  AEC_HIGH("init: aec_port: %p, session_id: 0x%x", private, *sessionid);
  STATS_MEMSET(private, 0, sizeof(aec_port_private_t));
  pthread_mutex_init(&private->update_state_lock, NULL);
  /* initialize AEC object */
  AEC_INITIALIZE_LOCK(&private->aec_object);

  private->aec_auto_mode = AEC_AUTO;
  private->aec_on_off_mode = TRUE;

  private->reserved_id = *sessionid;
  private->state       = AEC_PORT_STATE_CREATED;
  aec_port_reset_dual_cam_info(private);

  aec_port_reset_output_index(private);

  aec_port_initialize_3hdr_stats_buffer(private);

  port->port_private   = private;
  port->direction      = MCT_PORT_SINK;
  caps.port_caps_type  = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag    = (MCT_PORT_CAP_STATS_Q3A | MCT_PORT_CAP_STATS_CS_RS);
  private->adrc_settings.is_adrc_feature_supported = TRUE;
  private->ae_test_config = 0;

  /* Set default functions to keep clean & bug free code*/
  rc &= aec_port_load_dummy_default_func(&private->aec_object);
  rc &= aec_port_update_func_table(private);

  /* this is sink port of aec module */
  mct_port_set_event_func(port, aec_port_event);
  mct_port_set_intra_event_func(port, aec_port_intra_event);
  mct_port_set_ext_link_func(port, aec_port_ext_link);
  mct_port_set_unlink_func(port, aec_port_ext_unlink);
  mct_port_set_set_caps_func(port, aec_port_set_caps);
  mct_port_set_check_caps_reserve_func(port, aec_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, aec_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }

  /* If 3A fixed setting enabled, load the setting from XML */
  if (STATS_TEST(STATS_DBG_TEST_FIXED_3A)) {
    aec_port_load_fixed_settings_from_xml(private);
  }

  return rc;
}

/** aec_port_set_stored_parm:
 *    @port: AEC port pointer
 *    @stored_params: Previous session stored parameters.
 *
 * This function stores the previous session parameters.
 *
 **/
void aec_port_set_stored_parm(mct_port_t *port, aec_stored_params_t* stored_params)
{
  aec_port_private_t *private =(aec_port_private_t *)port->port_private;

  if (!stored_params || !private) {
    AEC_ERR("aec port or init param pointer NULL");
    return;
  }

  private->stored_params = stored_params;
}

/*Start of AEC Dual Camera Port handling*/

/* Summary of code changes:
 */

/** aec_port_link_to_peer:
 *    @port:  MCT port data
 *    @event: module event received
 *
 * Links to peer AEC Port (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_link_to_peer(mct_port_t* port,
                              mct_event_t* event)
{
  boolean                                rc            = FALSE;
  mct_port_t*                            this_port     = port;
  mct_port_t*                            peer_port     = NULL;
  cam_sync_related_sensors_event_info_t* link_param    = NULL;
  uint32_t                               peer_identity = 0;
  int                                    syncOverride  = 0;
  aec_port_private_t*                    private       = (aec_port_private_t*)(port->port_private);

  link_param = (cam_sync_related_sensors_event_info_t *)
    (event->u.ctrl_event.control_event_data);
  peer_identity = link_param->related_sensor_session_id;

  rc = stats_util_get_peer_port(event, peer_identity, this_port, &peer_port);
  if (rc == FALSE) {
    AEC_ERR("FAIL to Get Peer Port");
    return rc;
  }

  private->dual_cam_info.cam_role      = link_param->cam_role;
  private->dual_cam_info.mode          = link_param->mode;
  private->dual_cam_info.sync_3a_mode  = link_param->sync_3a_config.sync_mode_stats;
  syncOverride = q3a_prop_get("persist.vendor.camera.aec.sync","0");
  if(syncOverride == 1) {
    private->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_FOLLOW;
  } else if(syncOverride == 2) {
    private->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_NONE;
  }
  private->dual_cam_info.is_LPM_on     = FALSE;
  private->dual_cam_info.intra_peer_id = peer_identity;

  MCT_PORT_INTRALINKFUNC(this_port)(peer_identity, this_port, peer_port);

  AEC_HIGH("AECDualCam-[mode-%d role-%d 3aSync-%d AuxSync-%d AuxUpdate-%d] Linking to session %x Success",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    private->dual_cam_info.sync_3a_mode,
    private->dual_cam_info.is_aux_sync_enabled,
    private->dual_cam_info.is_aux_update_enabled,
    peer_identity);

  aec_port_handle_role_switch(port, NULL);

  /* Send any configuration data that is needed to the peer*/
  if (CAM_ROLE_WIDE == private->dual_cam_info.cam_role &&
      private->dual_cam_info.otp_data_valid)
  {
    aec_port_peer_config config;
    /* Wide camera has OTP data but Tele doesn't; pass this info to the Tele peer */
    config.exp_multiplier_otp = private->dual_cam_info.exp_multiplier_otp;

    rc &= stats_util_post_intramode_event(port,
                                          private->dual_cam_info.intra_peer_id,
                                          MCT_EVENT_MODULE_STATS_PEER_CONFIG_EVENT,
                                          (void*)&config);
  }

  return rc;
}

/** aec_port_unlink_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer AEC Port (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_unlink_from_peer(mct_port_t *port,
                                  mct_event_t *event)
{
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
  if(private->dual_cam_info.intra_peer_id){
    AEC_HIGH("AECDualCam-[mode -%d role-%d] Unlink Started",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    aec_port_local_unlink(port,event);
    aec_port_remote_unlink(port);
    AEC_HIGH("AECDualCam-[mode-%d role-%d] Unlink complete",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    aec_port_reset_dual_cam_info(private);
  }
  return TRUE;
}

/** aec_port_local_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer AEC Port locally (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_local_unlink(mct_port_t *port,
                              mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  aec_port_private_t                    *private = (aec_port_private_t *)(port->port_private);
  uint32_t                               peer_identity = private->dual_cam_info.intra_peer_id;

  if (peer_identity) {
    rc = stats_util_get_peer_port(event, peer_identity, this_port, &peer_port);
    if (rc == TRUE) {
      MCT_PORT_INTRAUNLINKFUNC(peer_port);
    }
    AEC_HIGH("AECDualCam-[mode-%d role-%d] Unlinking from peer session",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
  } else {
    rc = TRUE;
  }
  return rc;
}

/** aec_port_remote_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Asks Peer to do unlink (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_remote_unlink(mct_port_t *port)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
  /* Forward the AEC update info to the slave session */
  AEC_HIGH("AECDualCam-[mode-%d role-%d] Remote Unlink issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_UNLINK,
                                       NULL);
  return rc;
}

/** aec_port_remote_LPM:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Informs peer that we have gone to LPM mode
 *
 * Return boolean
 **/
boolean aec_port_remote_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
  /* Forward the LPM to the Master session */
  AEC_HIGH("AECDualCam-[mode -%d role-%d] Remote LPM issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT,
                                       perf_ctrl);
  return rc;
}

/** aec_port_LPM_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * LPM from peer AEC Port (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_LPM_from_peer(mct_port_t *port,
                                     cam_dual_camera_perf_control_t* perf_ctrl)
{
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    AEC_HIGH("AECDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    aec_port_update_aux_sync_and_update(private);
  }
  return TRUE;
}

/** aec_port_handle_role_switch:
 *    @port: MCT port data
 *
 * Handles Role switch from Master to Slave and Vice versa (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_handle_role_switch(mct_port_t* port, mct_event_t* event)
{
  aec_port_private_t* private = (aec_port_private_t*)(port->port_private);

  if (event) {
    cam_dual_camera_master_info_t* p_dual_master_info = NULL;
    p_dual_master_info = (cam_dual_camera_master_info_t *)(event->u.ctrl_event.control_event_data);

    if (p_dual_master_info) {
      private->dual_cam_info.mode = p_dual_master_info->mode;
    }
  }

  aec_port_update_aux_sync_and_update(private);
  aec_port_set_role_switch(private);

  AEC_HIGH("AECDualCam-[mode-%d role-%d 3aSync-%d AuxSync-%d AuxUpdate-%d] Role Switch Success",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    private->dual_cam_info.sync_3a_mode,
    private->dual_cam_info.is_aux_sync_enabled,
    private->dual_cam_info.is_aux_update_enabled);

  return TRUE;
}

/** aec_port_set_role_switch
 *    @private: Private data of the port
 *
 * Return: TRUE if no error
 **/
static boolean aec_port_set_role_switch(aec_port_private_t* private)
{
  boolean rc = TRUE;
  q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(
                                       MSG_AEC_SET,
                                       AEC_SET_PARAM_ROLE_SWITCH,
                                       private);
  if(!aec_msg) {
    return FALSE;
  }

  aec_role_switch_params_t* p_role_switch_msg = &aec_msg->u.aec_set_parm.u.role_switch_params;
  STATS_MEMSET(p_role_switch_msg, 0, sizeof(*p_role_switch_msg));

  if(private->dual_cam_info.mode == CAM_MODE_PRIMARY) {
    aec_update_t* p_interpolated_update = &private->dual_cam_info.interpolated_update.aec_update;

    p_role_switch_msg->master     = TRUE;
    p_role_switch_msg->exp_index  = p_interpolated_update->exp_index;
    p_role_switch_msg->lux_idx    = p_interpolated_update->lux_idx;
    p_role_switch_msg->exp_time   = p_interpolated_update->exp_time;
    p_role_switch_msg->real_gain  = p_interpolated_update->real_gain;
    p_role_switch_msg->linecount  = p_interpolated_update->linecount;
    p_role_switch_msg->total_drc_gain = p_interpolated_update->total_drc_gain;
  } else {
    p_role_switch_msg->master     = FALSE;
  }

  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);
  AEC_HIGH("AECDualCam-[mode-%d role-%d] Role Switch Master=%d Informed EI=%d LI=%f ET=%f G=%f LC=%d",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    p_role_switch_msg->master,
    p_role_switch_msg->exp_index,
    p_role_switch_msg->lux_idx,
    p_role_switch_msg->exp_time,
    p_role_switch_msg->real_gain,
    p_role_switch_msg->linecount);

  return rc;
}

/** aec_port_reset_dual_cam_info:
 *    @port: AEC Private  port data
 *
 * Resets Dual Cam Info (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_reset_dual_cam_info(aec_port_private_t  *port)
{
  AEC_HIGH("AECDualCam-[mode-%d role-%d] Reset",
    port->dual_cam_info.mode,
    port->dual_cam_info.cam_role);
  port->dual_cam_info.mode                  = CAM_MODE_PRIMARY;
  port->dual_cam_info.sync_3a_mode          = CAM_3A_SYNC_NONE;
  port->dual_cam_info.is_LPM_on             = FALSE;
  port->dual_cam_info.is_aux_sync_enabled   = FALSE;
  port->dual_cam_info.is_aux_update_enabled = FALSE;
  port->dual_cam_info.intra_peer_id         = 0;
  port->dual_cam_info.is_algo_active        = TRUE;
  return TRUE;
}

/** aec_port_update_aux_sync_and_update:
 *    @port: AEC Private  port data
 *
 * Updates Aux sync and update conditions (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_update_aux_sync_and_update(aec_port_private_t* port)
{
  boolean is_aux_sync_enabled   = FALSE;
  boolean is_aux_update_enabled = FALSE;
  boolean is_algo_active        = TRUE;

  if(port->dual_cam_info.is_LPM_on) {
    port->dual_cam_info.is_aux_sync_enabled   = FALSE;
    port->dual_cam_info.is_aux_update_enabled = FALSE;
    AEC_HIGH("AECDualCam-[mode -%d role-%d] Aux Sync Enabled=%d Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled,
      is_aux_update_enabled);
    return TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)                 &&
      (port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW) &&
      (port->dual_cam_info.mode == CAM_MODE_PRIMARY)           &&
      (port->dual_cam_info.intra_peer_id != 0)) {
    is_aux_sync_enabled = TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)                 &&
      (port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW) &&
      (port->dual_cam_info.mode == CAM_MODE_SECONDARY)         &&
      (port->dual_cam_info.intra_peer_id != 0)) {
    is_aux_update_enabled = TRUE;
  }

  if((port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW) &&
     (port->dual_cam_info.mode == CAM_MODE_SECONDARY)) {
    is_algo_active = FALSE;
  }

  if(is_algo_active != port->dual_cam_info.is_algo_active) {
    AEC_HIGH("AECDualCam-[mode-%d role-%d] is_algo_active=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_algo_active);
    port->dual_cam_info.is_algo_active = is_algo_active;
  }

  if(is_aux_sync_enabled != port->dual_cam_info.is_aux_sync_enabled) {
    AEC_HIGH("AECDualCam-[mode-%d role-%d] Aux Sync Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled);
    port->dual_cam_info.is_aux_sync_enabled = is_aux_sync_enabled;
  }

  if(is_aux_update_enabled != port->dual_cam_info.is_aux_update_enabled) {
    AEC_HIGH("AECDualCam-[mode-%d role-%d] Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_update_enabled);
    port->dual_cam_info.is_aux_update_enabled = is_aux_update_enabled;
  }

  return TRUE;
}

/** aec_is_aux_sync_enabled:
 *
 *    @port: port data
 *
 * Returns if Syncing to Aux Camera is allowed
 *
 * Return boolean
 **/
static boolean aec_is_aux_sync_enabled(aec_port_private_t  *port)
{
  return port->dual_cam_info.is_aux_sync_enabled;
}

/** aec_is_aux_update_enabled:
 *
 *    @port: port data
 *
 * Returns if Update from aux Camera is allowed
 *
 * Return boolean
 **/
static boolean aec_is_aux_update_enabled(aec_port_private_t  *port)
{
  return port->dual_cam_info.is_aux_update_enabled;
}

/** aec_port_update_LPM:
 *
 *    @port: port data
 *    @LPM: LPM enable
 *
 * Update LPM mode  (Master/Slave)
 *
 * Return boolean
 **/
boolean aec_port_update_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl)
{
  boolean                                rc = FALSE;
  aec_port_private_t  *private = (aec_port_private_t *)(port->port_private);
  q3a_thread_aecawb_msg_t *aec_msg = NULL;

  aec_msg = aec_port_create_msg(MSG_AEC_SET, AEC_SET_PARAM_LOW_POWER_MODE,
    private);
  if (NULL == aec_msg) {
    rc = FALSE;
    AEC_ERR("LPM malloc fail");
    return rc;
  }

  aec_msg->u.aec_set_parm.u.low_power_mode.enable = perf_ctrl->enable;
  aec_msg->u.aec_set_parm.u.low_power_mode.priority = perf_ctrl->priority;
  q3a_port_map_perf_type(&aec_msg->u.aec_set_parm.u.low_power_mode.perf_mode,
    perf_ctrl->perf_mode);

  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);

  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    AEC_HIGH("AECDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    aec_port_update_aux_sync_and_update(private);
  }
  return rc;
}

/** aec_port_save_peer_aec_state:
 *
 *    @port: port data
 *    @state: Master's AEC State
 *
 * Save Master's AEC state  (Slave)
 *
 * Return boolean
 **/
static boolean aec_port_save_peer_aec_state(aec_port_private_t *aec_port, uint8_t state)
{
  if(aec_port->dual_cam_info.aec_state_saved != state) {
    AEC_HIGH("AECDualCam-[mode-%d role-%d] MasterStateUpdate=%d",
      aec_port->dual_cam_info.mode,
      aec_port->dual_cam_info.cam_role,
      state);
    aec_port->dual_cam_info.aec_state_saved = state;
  }
  return TRUE;
}

/** aec_port_get_aec_state:
  *
  *    @aec_port:         AEC port handle
  * get current AEC state if Master elese saved Master state if auxillary
 *
*/
static uint8_t aec_port_get_aec_state(aec_port_private_t *aec_port)
{
  if((aec_port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)&&
     (aec_port->dual_cam_info.mode == CAM_MODE_SECONDARY)) {
    AEC_LOW("AECDualCam-[mode-%d role-%d] Sending Stored Master's State=%d",
      aec_port->dual_cam_info.mode,
      aec_port->dual_cam_info.cam_role,
      aec_port->dual_cam_info.aec_state_saved);
    return aec_port->dual_cam_info.aec_state_saved;
  }
  return aec_port->aec_state;
}

/** aec_port_send_bus_message_enabled:
  *
  *    @aec_port:         AEC port handle
  *    @bus_msg_type: Bus message Type
  *  Checks if port can send Bus messages
 *
*/
static boolean aec_port_send_bus_message_enabled(aec_port_private_t *aec_port,mct_bus_msg_type_t bus_msg_type)
{
  /* Slave will not send its own bus messages except for IMMEDIATE  */
  if ((aec_port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)&&
      (aec_port->dual_cam_info.mode == CAM_MODE_SECONDARY) &&
      (bus_msg_type != MCT_BUS_MSG_AEC_IMMEDIATE)) {
    AEC_LOW("AECDualCam-[mode-%d role-%d] Sending Bus Message Disabled bus_msg_type=%d",
      aec_port->dual_cam_info.mode,
      aec_port->dual_cam_info.cam_role,
      bus_msg_type);
    return FALSE;
  }
  return TRUE;
}

/** aec_port_send_update_message_enabled:
  *
  *    @aec_port: AEC port handle
  *  Checks if AUX port can send update messages
  *
*/
static boolean aec_port_send_update_message_enabled(aec_port_private_t *aec_port)
{
  /* Slave camera does not send its own update, but instead interpolates the main camera's update */
  if ((aec_port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)&&
      (aec_port->dual_cam_info.mode == CAM_MODE_SECONDARY)) {
    AEC_LOW("AECDualCam-[mode-%d role-%d] Sending Update Message Disabled",
      aec_port->dual_cam_info.mode,
      aec_port->dual_cam_info.cam_role);
    return FALSE;
  }
  return TRUE;
}

/** aec_port_is_algo_active:
  *
  *    @aec_port: AEC port handle
  *  Checks if AEC algo is active
 *
*/
static boolean aec_port_is_algo_active(aec_port_private_t *aec_port)
{
  return aec_port->dual_cam_info.is_algo_active;
}

/** aec_port_can_stats_be_configured:
  *
  *    @aec_port:   AEC port handle
  *  Checks if Stats can be configured
  *
*/
static boolean aec_port_can_stats_be_configured(aec_port_private_t *aec_port)
{
  boolean allowed = TRUE;
  if ((aec_port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)&&
      (aec_port->dual_cam_info.mode == CAM_MODE_SECONDARY) &&
      (aec_port->dual_cam_info.cam_role == CAM_ROLE_MONO) &&
      !(stats_exif_debug_mask & EXIF_DEBUG_MASK_BE)){
    AEC_LOW("AECDualCam-[mode-%d role-%d] Config Stats Disabled",
      aec_port->dual_cam_info.mode,
      aec_port->dual_cam_info.cam_role);
    allowed = FALSE;
  }
  return allowed;
}

/** aec_port_forward_bus_message_if_linked:
  *
  *    @mct_port: MCT port object
  *    @event:    Event to be forwarded
*/
static boolean aec_port_forward_bus_message_if_linked(mct_port_t* mct_port, mct_event_t event)
{
  boolean result = TRUE;
  aec_port_private_t* aec_port = (aec_port_private_t*)(mct_port->port_private);

  if(aec_is_aux_sync_enabled(aec_port)) {
    /* Forward the event to the slave session */
    result = stats_util_post_intramode_event(mct_port,
                                             aec_port->dual_cam_info.intra_peer_id,
                                             MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT,
                                             (void*)&event);
  }
  return result;
}

/** aec_port_get_dual_cam_multiplier_unadjusted:
  *
  *    @dual_cam_info:      AEC Dual Camera information
  *
  * Gets the correction multiplier to be applied to the master AEC update in order
  * to interpolate the slave's AEC update. This is the multiplier without adjusting for
  * AWB color temperature
  *
  * Return: Multipler to be used for the slave to interpolate the master AEC update.
  *         This is the brightness ratio of (slave / master)
  */
static float aec_port_get_dual_cam_multiplier_unadjusted(aec_dual_cam_info* dual_cam_info)
{
  float multiplier = 1.0f;

  if (dual_cam_info->otp_data_valid &&
      dual_cam_info->exp_multiplier_otp > 0.0f) {

    multiplier = dual_cam_info->exp_multiplier_otp;
  } else if (dual_cam_info->exp_multiplier_tuning != 0.0f) {

    multiplier = dual_cam_info->exp_multiplier_tuning;
  }

  if (multiplier <= 0.0f) {
    multiplier = 1.0f;
  }

  return multiplier;
}

/** aec_port_get_dual_cam_multiplier:
  *
  *    @dual_cam_info:      AEC Dual Camera information
  *    @master_update_info: Input parameter; information on the AEC update from the master
  *
  * Calculates a correction multiplier to be applied to the master AEC update in order
  * to interpolate the slave's AEC update. The multiplier is from OTP sensor data, but
  * if that's not available, it's obtained from the Chromatix header instead. The multiplier
  * itself is then interpolated from its base value depending on the current AWB color
  * temperature.
  *
  * Return: Multipler to be used for the slave to interpolate the master AEC update.
  *         This is the brightness ratio of (slave / master)
  */
static float aec_port_get_dual_cam_multiplier(
  aec_dual_cam_info* dual_cam_info,
  aec_port_peer_aec_update* master_update_info)
{
  float multiplier = aec_port_get_dual_cam_multiplier_unadjusted(dual_cam_info);
  AEC_LOW("AECDualCam Using multiplier value of %f for slave interpolation. Tuning value: %f, OTP value(valid:%d): %f",
    multiplier,
    dual_cam_info->exp_multiplier_tuning,
    dual_cam_info->otp_data_valid,
    dual_cam_info->exp_multiplier_otp
    );

  float tuned_ratio = -1.0f;
  float awb_ratio = -1.0f;
  uint16_t awb_colortemp = master_update_info->awb_color_temp;
  int i = 0;

  /* Find the compensation ratio matching the tuned color temperature */
  for (i = 0; i < AEC_DUAL_CAM_COMP_TABLE_SIZE; i++) {
    if (dual_cam_info->tuned_color_temp ==
        dual_cam_info->tuning_table[i].color_temperature) {
      tuned_ratio = dual_cam_info->tuning_table[i].compensation_ratio;
    }
  }
  if (tuned_ratio == -1.0f || tuned_ratio == 0) tuned_ratio = 1.0f;


  /* Find a matching color temperature for AWB in the tuning table. Assumes that
     the tuning table has temperature values sorted in decreasing order*/
  for (i = 0; i < AEC_DUAL_CAM_COMP_TABLE_SIZE; i++) {
    if (awb_colortemp >= dual_cam_info->tuning_table[i].color_temperature) {
      break;
    }
  }

  if (i == 0) {
    awb_ratio = dual_cam_info->tuning_table[0].compensation_ratio;
  } else if(i >= AEC_DUAL_CAM_COMP_TABLE_SIZE) {
    awb_ratio = dual_cam_info->tuning_table[AEC_DUAL_CAM_COMP_TABLE_SIZE-1].compensation_ratio;
  } else {
    /* Interpolate the compensation ratio for the current AWB temperature */
    uint16_t x0 = dual_cam_info->tuning_table[i-1].color_temperature;
    float    y0 = dual_cam_info->tuning_table[i-1].compensation_ratio;
    uint16_t x1 = dual_cam_info->tuning_table[i].color_temperature;
    float    y1 = dual_cam_info->tuning_table[i].compensation_ratio;
    uint16_t x  = awb_colortemp;

    if(x0 == x1) {
      /* bad tuning table, will result in division by zero */
      awb_ratio = 1.0f;
    } else {
      awb_ratio = y0 + (y1-y0)*(x-x0)/(x1-x0);
    }
  }

  if (awb_ratio == -1.0f || awb_ratio == 0) awb_ratio = 1.0f;
  float comp_ratio = tuned_ratio / awb_ratio;

  /* Sanity & error handling */
  if (comp_ratio > 1.5f || comp_ratio < 0.5f) {
    comp_ratio = 1.0f;
  }

  /* Adjust the exposure multiplier by compensation ratio to compensate for
     AWB color temperature change */
  multiplier *= comp_ratio;
  if (multiplier <= 0.0f) {
    multiplier = 1.0f;
  }
  AEC_LOW("AECDualCam Using adjusted multiplier value of %f at %dK temperature for slave interpolation", multiplier, awb_colortemp);

  return multiplier;
}

/** aec_port_interpolate_aec_update_from_peer:
  *
  *    @aec_port:                 AEC port handle
  *    @master_update_info: Input parameter; information on the AEC update from the master
  *    @slave_stats_update:  Output parameter; slave update event to interpolate the results into
  *
  * Interpolate the given master update event to match the slave's characteristics
  *
  * Return: void
  */
static void aec_port_interpolate_aec_update_from_peer(
  aec_port_private_t*       aec_port,
  aec_port_peer_aec_update* master_update_info,
  stats_update_t*           slave_stats_update)
{
  aec_antibanding_type_t master_antibanding = master_update_info->output.cur_atb;
  aec_update_t* master_update = &(master_update_info->output.stats_update.aec_update);
  aec_update_t* slave_update  = &(slave_stats_update->aec_update);
  int      AEC_Q8             = 0x00000100;
  float    max_exp_time       = (float)AEC_Q8 / (float)aec_port->fps.min_fps;
  float    slave_exp_time     = 1.0f;
  float    slave_gain         = 1.0f;
  uint16_t slave_linecount    = 1;
  float    multiplier         = 1.0f;
  float    res_multiplier     = 1.0f;
  int i;

  multiplier = aec_port_get_dual_cam_multiplier(&aec_port->dual_cam_info, master_update_info);

  /* Make a shallow copy to initialize the slave data */
  (*slave_stats_update) = master_update_info->output.stats_update;

  if (master_update->exp_time == 0) {
    AEC_LOW("AECDualCam master exposure time is 0");
    return;
  }

  /* Interpolate exposure time and gain based on the given multiplier */
  /* Preference is to adjust gain first so as to try to keep exposure time the same between
   * master and slave. If gain reaches its cap however, there'll be some residual that would
   * require exposure time to be recalculated. */

  /* First, try to adjust gain */
  slave_gain = master_update->sensor_gain * multiplier;
  /* Clip the gain value between 1.0 and maximum sensor gain */
  slave_gain = MAX(1.0f, slave_gain);
  slave_gain = MIN(slave_gain, aec_port->max_gain_aec_exp_table);

  /* Adjust exposure time with the gain residual multiplier and clip it*/
  res_multiplier = (master_update->sensor_gain * multiplier) / slave_gain;
  slave_exp_time = master_update->exp_time * res_multiplier;
  slave_exp_time = MIN(slave_exp_time, max_exp_time);

  /* Apply anti-banding */
  slave_exp_time = aec_port_apply_antibanding(master_antibanding, slave_exp_time);

  /* Calculate the exposure residual to adjust gain again */
  res_multiplier = (master_update->exp_time * res_multiplier) / slave_exp_time;
  slave_gain *= res_multiplier;
  /* Clip the gain value between 1.0 and maximum sensor gain */
  slave_gain = MAX(1.0f, slave_gain);
  slave_gain = MIN(slave_gain, aec_port->max_gain_aec_exp_table);


  /* Calculate line count based on exposure time */
  if (0 == aec_port->sensor_info.pixel_clock_per_line ||
      0 == aec_port->sensor_info.pixel_clock)
  {
    slave_linecount = aec_port->dual_cam_info.init_linecount;
    AEC_INFO("No pixel clock info available yet! Using intit linecount of %d from tuning header", slave_linecount);
  }
  else
  {
    slave_linecount = (slave_exp_time *
                       aec_port->sensor_info.pixel_clock /
                       aec_port->sensor_info.pixel_clock_per_line);
  }

  slave_linecount = MAX(1, slave_linecount);

  slave_update->exp_time    = slave_exp_time;
  slave_update->real_gain   = slave_gain;
  slave_update->sensor_gain = slave_gain;
  slave_update->linecount   = slave_linecount;

  /* Adjust for ADRC gains - if enabled */
  if (slave_update->total_drc_gain > 0.0f) {
    slave_update->real_gain *= slave_update->total_drc_gain;
  }

  /* Calculate the exposure index for slave */
  float exp_index_temp    = (slave_gain * slave_linecount) / (aec_port->min_gain * aec_port->min_line_count);
  slave_update->exp_index = (int32_t)(((float)log10(exp_index_temp)/ LOG10_1P03));
  /* Estimate the lux index, using masters delta of exposure & lux index as an estimate of delta here */
  slave_update->lux_idx   = (float)slave_update->exp_index
                          + (master_update->lux_idx - master_update->exp_index);
  slave_update->dual_exp_multiplier = 1.0f / multiplier;
  aec_port->lux_idx       = slave_update->lux_idx;
  /* Send the LED estimation state on master module only so as to not confuse other entities down the pipeline */
  slave_update->est_state = AEC_EST_OFF;

  for (i = 0; i < AEC_OUTPUT_ARRAY_MAX_SIZE; i++) {
    aec_port->state_update.u.output[i] = master_update_info->output;
  }

  AEC_LOW("\nAECDualInterpolation: frame_id: %d, slave [role-%d], multiplier: %02.4f\n"
          "AECDualInterpolation: Master exp_time: %02.4f, SG:RG:LC=(%02.4f : %02.4f : %04d), exp_index: %03d, lux_idx: %03.4f\n"
          "AECDualInterpolation: Slave  exp_time: %02.4f, SG:RG:LC=(%02.4f : %02.4f : %04d), exp_index: %03d, lux_idx: %03.4f",
          master_update->frame_id,
          aec_port->dual_cam_info.cam_role,
          multiplier,
          master_update->exp_time,
          master_update->sensor_gain,
          master_update->real_gain,
          master_update->linecount,
          master_update->exp_index,
          master_update->lux_idx,
          slave_update->exp_time,
          slave_update->sensor_gain,
          slave_update->real_gain,
          slave_update->linecount,
          slave_update->exp_index,
          slave_update->lux_idx);
}

/** aec_port_handle_peer_aec_update:
  *    @mct_port:   MCT port object
  *    @aec_update: AEC update info
  *
  * Handles a forwarded peer event for AEC update, where the master session has
  * forwraded the AEC update info to the slave session
  *
  * Return: void
  */
static void aec_port_handle_peer_aec_update(
  mct_port_t* mct_port,
  aec_port_peer_aec_update* aec_update)
{
  stats_update_t interpolated_update;
  aec_port_private_t* aec_port = (aec_port_private_t *)(mct_port->port_private);

  AEC_LOW("AECDualCam-[mode-%d role-%d] Received peer AEC update event",
    aec_port->dual_cam_info.mode,
    aec_port->dual_cam_info.cam_role);

  if(!aec_is_aux_update_enabled(aec_port)) {
    return;
  }

  STATS_MEMSET(&interpolated_update, 0, sizeof(interpolated_update));
  /* Interpolate the peer event to match the slave's AEC characteristics */
  aec_port_interpolate_aec_update_from_peer(aec_port, aec_update, &interpolated_update);
  aec_port->dual_cam_info.interpolated_update_is_valid = TRUE;
  aec_port->dual_cam_info.interpolated_update = interpolated_update;
  aec_port->aec_update_flag = TRUE;

  /* Send the interpolated update event */
  aec_port_send_event(mct_port,
      MCT_EVENT_MODULE_EVENT,
      MCT_EVENT_MODULE_STATS_AEC_UPDATE,
      (void*)&interpolated_update,
      aec_update->output.stats_update.aec_update.sof_id);
}

/** aec_port_handle_peer_event:
  *    @mct_port: MCT port object
  *    @event:    AEC event from master
  *
  * Handles MCT event forwarded from Master session
  *
  * Return: void
  */
static void aec_port_handle_peer_event(
  mct_port_t* mct_port,
  mct_event_t* event)
{
  aec_port_private_t* aec_port = (aec_port_private_t *)(mct_port->port_private);
  /* update the identity to slave's */
  event->identity = aec_port->reserved_id;

  AEC_LOW("AECDualCam-[mode-%d role-%d] Received peer AEC status event",
    aec_port->dual_cam_info.mode,
    aec_port->dual_cam_info.cam_role);

  if (MCT_EVENT_MODULE_STATS_POST_TO_BUS == event->u.module_event.type) {
    mct_bus_msg_t* busmsg = (mct_bus_msg_t*)event->u.module_event.module_event_data;
    /* update the bus message identity to slave's */
    busmsg->sessionid = (event->identity >> 16);

    /* Store master's AE state but do not forward this particular message. Slave will send its own. */
    if (busmsg->type == MCT_BUS_MSG_AEC_IMMEDIATE) {
      mct_bus_msg_aec_immediate_t* aec_urgent = (mct_bus_msg_aec_immediate_t*)busmsg->msg;
      aec_port_save_peer_aec_state(aec_port, aec_urgent->aec_state);
      return;
    }
  }

  if(!aec_is_aux_update_enabled(aec_port)) {
    return;
  }
  MCT_PORT_EVENT_FUNC(mct_port)(mct_port, event);
}

/** aec_port_handle_peer_config:
  *    @mct_port: MCT port object
  *    @config:   AEC peer configuration data
  *
  * Handles peer configuration data
  *
  * Return: void
  */
static void aec_port_handle_peer_config(
  mct_port_t* mct_port,
  aec_port_peer_config* config)
{
  aec_port_private_t* aec_port = (aec_port_private_t *)(mct_port->port_private);

  aec_port->dual_cam_info.otp_data_valid = TRUE;
  aec_port->dual_cam_info.exp_multiplier_otp = 1.0f / config->exp_multiplier_otp;

  AEC_LOW("AECDualCam: Received AEC OTP calibration from peer. Exposure multiplier: %f", aec_port->dual_cam_info.exp_multiplier_otp);
}

/** aec_port_intra_event:
 *    @port:  MCT port
 *    @event: MCT module
 *
 * Handles the intra-module events sent between AEC master and slave sessions
 *
 * Return TRUE if the event is processed successfully.
 **/
static boolean aec_port_intra_event(mct_port_t *port, mct_event_t *event)
{
  aec_port_private_t *private;

  /* sanity check */
  if (!port || !event || !port->port_private) {
    return FALSE;
  }

  private = (aec_port_private_t *)(port->port_private);

  AEC_LOW("AECDualCam-[mode-%d role-%d] Received AEC intra-module peer event",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);

  /* sanity check: ensure event is meant for port with same identity */
  if (!aec_port_check_identity(private->reserved_id, event->identity)) {
    return FALSE;
  }

  /* check if there's need for extended handling. */
  if (private->func_tbl.ext_handle_intra_event) {
    stats_ext_return_type ret;
    AEC_LOW("Handle extended intra port event!");
    ret = private->func_tbl.ext_handle_intra_event(port, event);
    /* Check if this event has been completely handled. If not we'll process it further here. */
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      AEC_LOW("Intra port event %d handled by extended functionality!",
        event->u.module_event.type);
      return ret;
    }
  }


  switch(event->u.module_event.type)
  {
    case MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT:
      AEC_LOW("AECDualCam-[mode-%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      aec_port_handle_peer_aec_update(port,
          (aec_port_peer_aec_update*)event->u.module_event.module_event_data);
      break;

    case MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT:
      AEC_LOW("AECDualCam-[mode-%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      aec_port_handle_peer_event(port,
        (mct_event_t*)event->u.module_event.module_event_data);
      break;

    case MCT_EVENT_MODULE_STATS_PEER_UNLINK:
      AEC_LOW("AECDualCam-[mode-%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UNLINK",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      aec_port_local_unlink(port, event);
      aec_port_reset_dual_cam_info(private);
      break;

    case MCT_EVENT_MODULE_STATS_PEER_CONFIG_EVENT:
      AEC_LOW("AECDualCam-[mode-%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_CONFIG_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      aec_port_handle_peer_config(port,
        (aec_port_peer_config*)event->u.module_event.module_event_data);
      break;

    case MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT:
      AEC_LOW("AECDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      aec_port_LPM_from_peer(port,
        (cam_dual_camera_perf_control_t*)event->u.ctrl_event.control_event_data);
      break;

    default:
      AEC_ERR("AECDualCam-[mode-%d role-%d] Error! Received unknown intra-module event type: %d",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role,
        event->u.module_event.type);
      break;
  }

  return TRUE;
}

/*End of AEC Dual Camera Port handling*/
