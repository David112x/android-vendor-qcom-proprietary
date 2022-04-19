/* awb_port_ext.c
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "awb_port.h"
#include "awb_ext.h"
#include "awb_ext_test_wrapper.h"
#include "camera_config.h"

#include <dlfcn.h>
#include "ext_awb_wrapper.h"

/** awb_port_ext_update_opaque_input_params
 *
 *  @awb_port: port information
 *  @type: type of the parameter
 *  @data: payload
 *
 *  Package custom parameter inside opaque payload.
 *
 **/
static boolean awb_port_ext_update_opaque_input_params(
  awb_port_private_t *awb_port,
  int16_t type,
  q3a_custom_data_t *payload)
{
  boolean rc = FALSE;

  if (!payload->size) {
    AWB_ERR("Error: payload size zero");
    return rc;
  }

  /* Pass directly as set param call */
  awb_set_parameter_t *set_parm;
  q3a_custom_data_t input_param;
  STATS_MEMSET(&input_param, 0, sizeof(input_param));
  q3a_thread_aecawb_msg_t *awb_msg =  awb_port_create_msg(MSG_AWB_SET,
    type, awb_port);
  if (NULL == awb_msg) {
    AWB_ERR("Memory allocation failure!");
    return rc;
  }

  /* populate af message to post to thread */
  set_parm = &awb_msg->u.awb_set_parm;
  input_param.size = payload->size;
    input_param.data = malloc(payload->size);
  if (NULL == input_param.data) {
    AWB_ERR("Failure allocating memory to store data!");
    free(awb_msg);
    return rc;
  }
  STATS_MEMCPY(input_param.data, input_param.size, payload->data, payload->size);
  set_parm->u.awb_custom_data = input_param;
  rc = q3a_aecawb_thread_en_q_msg(awb_port->thread_data, awb_msg);

  return rc;
}

/** awb_port_ext_unifed_request_batch:
 *    @priv:   priv data of the port
 *
 * This function request to the algoritm the data to fill the batch information.
 *
 * Return: TRUE on success
 **/
static void awb_port_ext_unified_saved_batch(awb_port_private_t *priv)
{
  boolean rc = FALSE;
  int i = 0, j = 0;
  awb_ext_param_t *ext_param = (awb_ext_param_t *)priv->ext_param;
  awb_frame_batch_t *fi = &ext_param->fi;

  /* workaround */
  fi->num_batch = 2;
  fi->frame_batch[0].flash_mode = TRUE;
  fi->frame_batch[1].flash_mode = FALSE;

  q3a_thread_aecawb_msg_t *awb_msg =
    (q3a_thread_aecawb_msg_t *)calloc(1, sizeof(q3a_thread_aecawb_msg_t));
  if (NULL == awb_msg) {
    AWB_ERR("Not enough memory");
    return;
  }
  awb_msg->sync_flag = TRUE;
  awb_msg->type = MSG_AWB_GET;
  awb_msg->u.awb_get_parm.type = AWB_UNIFIED_FLASH;
  memcpy(&awb_msg->u.awb_get_parm.u.frame_info, fi, sizeof(awb_frame_batch_t));
  rc = q3a_aecawb_thread_en_q_msg(priv->thread_data, awb_msg);
  if (TRUE == rc) {
    for (i = 0; i < fi->num_batch; i++) {
      if ((ext_param->awb_best_shot != AWB_BESTSHOT_OFF) ||
        (ext_param->awb_current_wb != CAMERA_WB_AUTO)) {
        fi->frame_batch[i].capture_type =
          awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].capture_type;
        fi->frame_batch[i].r_gain = ext_param->gain.r_gain;
        fi->frame_batch[i].g_gain = ext_param->gain.g_gain;
        fi->frame_batch[i].b_gain = ext_param->gain.b_gain;
        fi->frame_batch[i].color_temp = ext_param->color_temp;
        fi->frame_batch[i].ccm = ext_param->ccm;
      } else {
        fi->frame_batch[i].capture_type =
          awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].capture_type;
        fi->frame_batch[i].r_gain =
          awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].r_gain;
        fi->frame_batch[i].b_gain =
          awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].b_gain;
        fi->frame_batch[i].g_gain =
          awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].g_gain;
        fi->frame_batch[i].color_temp =
          awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].color_temp;
        fi->frame_batch[i].ccm =
          awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].ccm;
      }
      AWB_HIGH("batch[%d] type[%d] [%f:%f:%f] [%f:%f:%f:%f:%f:%f:%f:%f:%f]",
        i,
        fi->frame_batch[i].capture_type,
        fi->frame_batch[i].r_gain,
        fi->frame_batch[i].b_gain,
        fi->frame_batch[i].g_gain,
        fi->frame_batch[i].ccm.ccm[0][0],
        fi->frame_batch[i].ccm.ccm[0][1],
        fi->frame_batch[i].ccm.ccm[0][2],
        fi->frame_batch[i].ccm.ccm[1][0],
        fi->frame_batch[i].ccm.ccm[1][1],
        fi->frame_batch[i].ccm.ccm[1][2],
        fi->frame_batch[i].ccm.ccm[2][0],
        fi->frame_batch[i].ccm.ccm[2][1],
        fi->frame_batch[i].ccm.ccm[2][2]);
    }
  }

  /* save unified gain */
  SET_UNIFIED(ext_param);

  free(awb_msg);
}

/** awb_port_ext_msg_enq_lock
 *  @awb_port: port information
 *  @lock: lock flag for algo
 *
 **/
static boolean awb_port_ext_msg_enq_lock(awb_port_private_t *private, int lock)
{
  boolean rc = FALSE;

  q3a_thread_aecawb_msg_t *awb_msg =
    (q3a_thread_aecawb_msg_t *)malloc(sizeof(q3a_thread_aecawb_msg_t));
  if (awb_msg == NULL) {
    AWB_ERR("Error: AWB_SET_PARAM_LOCK msg malloc failed");
    return rc;
  }
  memset(awb_msg, 0, sizeof(q3a_thread_aecawb_msg_t));
  awb_msg->type = MSG_AWB_SET;
  awb_msg->u.awb_set_parm.type = AWB_SET_PARAM_LOCK;
  awb_msg->u.awb_set_parm.u.awb_lock = lock;
  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, awb_msg);
  if (!rc) {
    AWB_ERR("Error: AWB_SET_PARAM_LOCK lock");
  }

  return rc;
}

/** awb_port_ext_lock
 *  @awb_port: port information
 *  @lock: lock flag for algo
 *
 **/
static boolean awb_port_ext_lock(awb_port_private_t *private, int lock)
{
  boolean rc = FALSE;
  awb_ext_param_t *ext_param = (awb_ext_param_t *)private->ext_param;

  if (ext_param->lock.algo != lock) {
    rc = awb_port_ext_msg_enq_lock(private, lock);
    if (rc) {
      ext_param->lock.algo = lock;
      AWB_HIGH("AWB msg lock[%d]", lock);
    }
  } else {
    AWB_HIGH("AWB msg lock[%d] already %s.", lock, lock ? "set" : "clear");
  }

  return rc;
}

/** awb_port_ext_clear_off_gain
 *  @awb_port: port information
 *  @lock: lock flag for algo
 *
 **/
static void awb_port_ext_clear_off_gain(awb_port_private_t *private)
{
  awb_ext_param_t *ext_param = (awb_ext_param_t *)private->ext_param;

  memset(&ext_param->off_gain, 0, sizeof(awb_gain_t));
  memset(&ext_param->off_ccm, 0, sizeof(awb_ccm_type));
  ext_param->off_color_temp = 0;

  ext_param->is_off_saved = FALSE;
}

/** awb_port_ext_save_off_gain
 *  @awb_port: port information
 *  @lock: lock flag for algo
 *
 **/
static void awb_port_ext_save_off_gain(awb_port_private_t *private)
{
  awb_ext_param_t *ext_param = (awb_ext_param_t *)private->ext_param;

  memcpy(&ext_param->off_gain, &ext_param->gain, sizeof(awb_gain_t));
  memcpy(&ext_param->off_ccm, &ext_param->ccm, sizeof(awb_ccm_type));
  ext_param->off_color_temp = ext_param->color_temp;

  ext_param->is_off_saved = TRUE;
}

/** awb_port_ext_restore_off_gain
 *  @awb_port: port information
 *  @lock: lock flag for algo
 *
 **/
static void awb_port_ext_restore_off_gain(awb_output_data_t *output,
    awb_port_private_t *private)
{
  awb_ext_param_t *ext_param = (awb_ext_param_t *)private->ext_param;

  output->stats_update.awb_update.gain = ext_param->off_gain;
  memcpy(&output->stats_update.awb_update.ccm_update.ccm,
    &ext_param->off_ccm.ccm, sizeof(output->ccm.ccm));
  memcpy(&output->stats_update.awb_update.ccm_update.ccm_offset,
    &ext_param->off_ccm.ccm_offset, sizeof(output->ccm.ccm_offset));
  output->stats_update.awb_update.ccm_update.ccm_update_flag =
    ext_param->off_ccm.override_ccm;
  output->stats_update.awb_update.color_temp = ext_param->off_color_temp;
}

/** awb_port_ext_update_led_state:
 *    @priv:   priv data of the port
 *
 * Return: TRUE on success
 **/
static void awb_port_ext_update_led_state(awb_port_private_t *priv,
    awb_ext_led_state_machine led_sm)
{
  awb_ext_param_t *ext_param = (awb_ext_param_t *)priv->ext_param;

  switch (led_sm) {
  case AWB_EXT_LED_STATUS_OFF: {
    if (IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_OFF))
      break;

    CLEAR_UNIFIED(ext_param);
    CLEAR_AF_DONE(ext_param);
    CLEAR_EST_DONE(ext_param);
    awb_port_ext_clear_off_gain(priv);
    SET_LED_STATE(ext_param, AWB_EXT_LED_STATUS_OFF);
  }
    break;

  case AWB_EXT_LED_STATUS_PREFLASH_AEC: {
    if (!IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_OFF))
      break;

    awb_port_ext_save_off_gain(priv);
    SET_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AEC);
  }
    break;

  case AWB_EXT_LED_STATUS_PREFLASH_AF: {
    if (!IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AEC))
      break;

    SET_EST_DONE(ext_param);
    SET_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AF);
  }
    break;

  case AWB_EXT_LED_STATUS_PREFLASH_AWB: {
    if (!IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AF))
      break;

    awb_port_ext_unified_saved_batch(priv);
    SET_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AWB);
  }
    break;

  case AWB_EXT_LED_STATUS_PREFLASH_LOCKED: {
    if (!IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AWB))
      break;

    awb_port_ext_lock(priv, 1);
    CLEAR_AF_DONE(ext_param);
    CLEAR_EST_DONE(ext_param);
    SET_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_LOCKED);
  }
    break;

  case AWB_EXT_LED_STATUS_RESTORE_OFFGAIN: {
    if (IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN))
      break;

    awb_port_ext_lock(priv, 0);
    ext_param->cnt_restore = MAX_CNT_RESTORE;
    SET_LED_STATE(ext_param, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN);
  }
    break;

  default:
    break;
  }
}

/** awb_port_ext_handle_set_parm_awb_event
 *
 *  @port: port information
 *  @evt_param: set parameter received.
 *
 *  Extend handling of awb specific set parameters as filtered
 *  by stats port.
*/
static stats_ext_return_type awb_port_ext_handle_set_parm_awb_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  awb_port_private_t  *awb_port = (awb_port_private_t *)(port->port_private);
  awb_set_parameter_t *param =
      (awb_set_parameter_t *)evt_param;
  awb_ext_param_t *ext_param = NULL;
  ext_param = (awb_ext_param_t *)awb_port->ext_param;

  /* Handle other set parameters here if required to extend. */
  switch (param->type) {
  case AWB_SET_PARAM_LOCK: {
    ext_param->lock.hal = param->u.awb_lock;
    AWB_HIGH("AWB_SET_PARAM_LOCK[%d] save %d", param->u.awb_lock, ext_param->lock.hal);

    if (ext_param->lock.hal) {
      if (IS_NO_LED(ext_param)) {
        awb_port_ext_lock(awb_port, 1);
      }
    }
    if (!ext_param->lock.hal) {
      awb_port_ext_lock(awb_port, 0);
    }

    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;

  case AWB_SET_PARAM_BESTSHOT: {
    ext_param->awb_best_shot = param->u.awb_best_shot;
    AWB_HIGH("AWB_SET_PARAM_BESTSHOT[%d]", ext_param->awb_best_shot);
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;

  case AWB_SET_PARAM_WHITE_BALANCE: {
    ext_param->awb_current_wb = param->u.awb_current_wb;
    AWB_HIGH("AWB_SET_PARAM_WHITE_BALANCE[%d]", ext_param->awb_current_wb);
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;

  default:
    break;
  }

  return rc;
}

/** awb_port_ext_handle_set_parm_af_event
 *
 *  @port: port information
 *  @evt_param: set parameter received.
 *
 *  Extend handling of awb specific set parameters as filtered
 *  by stats port.
*/
static stats_ext_return_type awb_port_ext_handle_set_parm_af_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  awb_port_private_t  *awb_port = (awb_port_private_t *)(port->port_private);
  af_set_parameter_t *param = (af_set_parameter_t *)evt_param;
  awb_ext_param_t *ext_param = NULL;
  ext_param = (awb_ext_param_t *)awb_port->ext_param;

  /* Handle other set parameters here if required to extend. */
  switch (param->type) {
  case AF_SET_PARAM_START:
    AWB_HIGH("AWB_SET_PARAM_START");
    rc = STATS_EXT_HANDLING_PARTIAL;
    break;

  case AF_SET_PARAM_CANCEL_FOCUS:
    AWB_HIGH("AWB_SET_PARAM_CANCEL_FOCUS");
    if (!ext_param->lock.algo) {
      if (IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AEC) ||
        IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AF) ||
        IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AWB) ||
        IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_LOCKED)) {
        awb_port_ext_update_led_state(awb_port, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN);
      }
    }

    /* corner case for preventing remained use_led_est by Touch AEC.
     * use_led_est is changed urgently, so need more frame(20) for restore. */
    if (IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_OFF) &&
      IS_FLASH_ON(ext_param)) {
      awb_port_ext_save_off_gain(awb_port);
      awb_port_ext_update_led_state(awb_port, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN);
    }

    rc = STATS_EXT_HANDLING_PARTIAL;
    break;

  default:
    break;
  }

  return rc;
}

/** awb_port_ext_unifed_request_batch:
 *    @priv:   priv data of the port
 *
 * This function request to the algoritm the data to fill the batch information.
 *
 * Return: TRUE on success
 **/
static boolean awb_port_ext_unified_request_batch(awb_port_private_t *priv)
{
  boolean rc = FALSE;
  int i = 0, j = 0;
  awb_frame_batch_t *fi = &priv->stats_frame_capture.frame_info;
  awb_ext_param_t *ext_param = (awb_ext_param_t *)priv->ext_param;

  q3a_thread_aecawb_msg_t *awb_msg =
    (q3a_thread_aecawb_msg_t *)calloc(1, sizeof(q3a_thread_aecawb_msg_t));
  if (NULL == awb_msg) {
    AWB_ERR("Not enough memory");
    return rc;
  }
  awb_msg->sync_flag = TRUE;
  awb_msg->type = MSG_AWB_GET;
  awb_msg->u.awb_get_parm.type = AWB_UNIFIED_FLASH;
  memcpy(&awb_msg->u.awb_get_parm.u.frame_info, fi, sizeof(awb_frame_batch_t));
  rc = q3a_aecawb_thread_en_q_msg(priv->thread_data, awb_msg);
  if (TRUE == rc) {
    for (i = 0; i < fi->num_batch; i++) {
      if (IS_UNIFIED_SAVED(ext_param)) {
        fi->frame_batch[i].capture_type =
          awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].capture_type;
        fi->frame_batch[i].r_gain = ext_param->fi.frame_batch[i].r_gain;
        fi->frame_batch[i].g_gain = ext_param->fi.frame_batch[i].g_gain;
        fi->frame_batch[i].b_gain = ext_param->fi.frame_batch[i].b_gain;
        fi->frame_batch[i].color_temp = ext_param->fi.frame_batch[i].color_temp;
        fi->frame_batch[i].ccm = ext_param->fi.frame_batch[i].ccm;

        /* consumed unified gain */
        CLEAR_UNIFIED(ext_param);
      } else {
        if ((ext_param->awb_best_shot != AWB_BESTSHOT_OFF) ||
          (ext_param->awb_current_wb != CAMERA_WB_AUTO)) {
          fi->frame_batch[i].capture_type =
            awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].capture_type;
          fi->frame_batch[i].r_gain = ext_param->gain.r_gain;
          fi->frame_batch[i].g_gain = ext_param->gain.g_gain;
          fi->frame_batch[i].b_gain = ext_param->gain.b_gain;
          fi->frame_batch[i].color_temp = ext_param->color_temp;
          fi->frame_batch[i].ccm = ext_param->ccm;
        } else {
          fi->frame_batch[i].capture_type =
            awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].capture_type;
          fi->frame_batch[i].r_gain =
            awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].r_gain;
          fi->frame_batch[i].b_gain =
            awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].b_gain;
          fi->frame_batch[i].g_gain =
            awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].g_gain;
          fi->frame_batch[i].color_temp =
            awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].color_temp;
          fi->frame_batch[i].ccm =
            awb_msg->u.awb_get_parm.u.frame_info.frame_batch[i].ccm;
        }
      }
      AWB_HIGH("batch[%d] type[%d] [%f:%f:%f] [%f:%f:%f:%f:%f:%f:%f:%f:%f]",
        i,
        fi->frame_batch[i].capture_type,
        fi->frame_batch[i].r_gain,
        fi->frame_batch[i].b_gain,
        fi->frame_batch[i].g_gain,
        fi->frame_batch[i].ccm.ccm[0][0],
        fi->frame_batch[i].ccm.ccm[0][1],
        fi->frame_batch[i].ccm.ccm[0][2],
        fi->frame_batch[i].ccm.ccm[1][0],
        fi->frame_batch[i].ccm.ccm[1][1],
        fi->frame_batch[i].ccm.ccm[1][2],
        fi->frame_batch[i].ccm.ccm[2][0],
        fi->frame_batch[i].ccm.ccm[2][1],
        fi->frame_batch[i].ccm.ccm[2][2]);
    }
  }
  free(awb_msg);

  return rc;
}

/** awb_port_ext_handle_set_parm_common_event
 *
 *  @port: port information
 *  @param: set parameter received.
 *
 *  Extend handling of set parameter call of type common.
 *
 **/
static stats_ext_return_type awb_port_ext_handle_set_parm_common_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  awb_port_private_t  *awb_port = (awb_port_private_t *)(port->port_private);
  stats_common_set_parameter_t *param =
      (stats_common_set_parameter_t *)evt_param;
  q3a_custom_data_t payload;


  switch (param->type) {
  case COMMON_SET_PARAM_BESTSHOT: {
    /* Sample on how to handle existing HAL events */
    q3a_thread_aecawb_msg_t *awb_msg = awb_port_create_msg(MSG_AWB_SET,
      AWB_SET_PARAM_BESTSHOT, awb_port);
    if (NULL == awb_msg) {
      AEC_ERR("Memory allocation failure!");
      ret = FALSE;
    } else {
      /* populate af message to post to thread */
      /* This value is usually map at port to convert from HAL to algo enum types,
       * in this case passing the value directly since port doesn't know the
       * required mapping for custom algo */
      /* 3rd party could do the mapping here if prefered*/
      awb_msg->u.awb_set_parm.u.awb_best_shot = (awb_bestshot_mode_type_t)param->u.bestshot_mode;

      ret = q3a_aecawb_thread_en_q_msg(awb_port->thread_data, awb_msg);
      if (!ret) {
        AWB_ERR("Fail to queue msg");
      }
    }
    /* Marking as complete since no further processing required */
    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;

  case COMMON_SET_PARAM_CUSTOM: {
    /* For custom parameters from HAL, we'll save in a list and then
       send to core algorithm every frame at once during stats
       trigger. */
    payload.data = param->u.custom_param.data;
    payload.size = param->u.custom_param.size;
    ret = awb_port_ext_update_opaque_input_params(awb_port,
      AWB_SET_PARM_CUSTOM_EVT_HAL, &payload);
    if (FALSE == ret) {
      AWB_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    }
  }
    break;

  case COMMON_SET_PARAM_UNIFIED_FLASH: {
    awb_ext_param_t *ext_param = (awb_ext_param_t *)awb_port->ext_param;

    /* pre-operation for unified flash */
    awb_port_ext_update_led_state(awb_port, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN);

    if (param->u.frame_info.num_batch != 0 &&
      awb_port->stats_frame_capture.frame_capture_mode == TRUE) {
      AWB_HIGH("frame_capture mode in progress, don't process");
      break;
    }
    STATS_MEMSET(&awb_port->stats_frame_capture.frame_info, 0, sizeof(awb_frame_batch_t));
    awb_port->stats_frame_capture.frame_capture_mode = FALSE;
    awb_port->stats_frame_capture.frame_info.num_batch = param->u.frame_info.num_batch;
    if (0 == awb_port->stats_frame_capture.frame_info.num_batch) {
      AWB_ERR("No. of num_batch is zero");
      break;
    }
    int i = 0;
    AWB_HIGH("No. of Batch from HAL = %d", awb_port->stats_frame_capture.frame_info.num_batch);
    for (i = 0; i < awb_port->stats_frame_capture.frame_info.num_batch; i++) {
      AWB_HIGH("frame_batch[%d] type: %d, flash_mode: %d", i,
        param->u.frame_info.configs[i].type,
        param->u.frame_info.configs[i].flash_mode);

      awb_port->stats_frame_capture.frame_info.frame_batch[i].capture_type =
        (uint32_t)param->u.frame_info.configs[i].type;
      if (CAM_CAPTURE_FLASH == param->u.frame_info.configs[i].type &&
          ((param->u.frame_info.configs[i].flash_mode == CAM_FLASH_MODE_ON) ||
           (param->u.frame_info.configs[i].flash_mode == CAM_FLASH_MODE_TORCH) ||
           (param->u.frame_info.configs[i].flash_mode == CAM_FLASH_MODE_AUTO))) {
        awb_port->stats_frame_capture.frame_info.frame_batch[i].flash_mode = TRUE;
      } else {
        awb_port->stats_frame_capture.frame_info.frame_batch[i].flash_mode = FALSE;
      }
    }

    ret = awb_port_ext_unified_request_batch(awb_port);
    if (FALSE == ret) {
      AWB_ERR("Fail to get batch data from AWB algo");
      STATS_MEMSET(&awb_port->stats_frame_capture, 0, sizeof(awb_frame_capture_t));
      break;
    }

    awb_frame_batch_t *fi = &awb_port->stats_frame_capture.frame_info;
    AWB_HIGH("COMMON_SET_PARAM_UNIFIED_FLASH [%f:%f:%f]",
      fi->frame_batch[0].r_gain,
      fi->frame_batch[0].g_gain,
      fi->frame_batch[0].b_gain);

    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;

  default: {
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;
  }

  return rc;
}


/** awb_port_ext_handle_set_parm_event
 *
 *  @port: port information
 *  @param: set parameter received.
 *
 *  Extend handling of set parameter call from HAL.
 *
 **/
static stats_ext_return_type awb_port_ext_handle_set_parm_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  awb_port_private_t  *awb_port = (awb_port_private_t *)(port->port_private);
  stats_set_params_type *stat_parm = (stats_set_params_type *)evt_param;

  if (!stat_parm || !awb_port) {
    AWB_ERR("Invalid parameters!");
    return FALSE;
  }

  AWB_LOW("Extended handling set param event of type: %d", stat_parm->param_type);
  /* These cases can be extended to handle other parameters here if required */
  if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
    rc = awb_port_ext_handle_set_parm_common_event(
      port, (void *)&(stat_parm->u.common_param));
  } else {
    /* Handle af specific set parameters here if different from af port handling */
    if (stat_parm->param_type == STATS_SET_Q3A_PARAM) {
      q3a_set_params_type  *q3a_param = &(stat_parm->u.q3a_param);
      if (q3a_param->type == Q3A_SET_AWB_PARAM) {
        rc = awb_port_ext_handle_set_parm_awb_event(
          port, (void *)&q3a_param->u.awb_param);
      } else if (q3a_param->type == Q3A_SET_AF_PARAM) {
        rc = awb_port_ext_handle_set_parm_af_event(
          port, (void *)&q3a_param->u.af_param);
      } else {
        rc = STATS_EXT_HANDLING_PARTIAL;
      }
    }
  }

  return rc;
} /* awb_port_ext_handle_set_parm_event */


/** awb_port_ext_handle_control_event:
 *    @port: port info
 *    @ctrl_evt: control event
 *
 * Extension of control event handling. Here OEM can further
 * handle/process control events. The control events can be OEM
 * specific or general. If it's OEM specific, OEM can either
 * process it here and send to core algorithm if required; or
 * just send the payload to core algorithm to process.
 *
 * Return stats_ext_return_type value.
 */
static stats_ext_return_type awb_port_ext_handle_control_event(
  mct_port_t *port,
  mct_event_control_t *ctrl_evt)
{
  awb_port_private_t *awb_port =
    (awb_port_private_t *)(port->port_private);
  q3a_custom_data_t payload;
  mct_custom_data_payload_t *cam_custom_ctr =
    (mct_custom_data_payload_t *)ctrl_evt->control_event_data;
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;

  switch (ctrl_evt->type) {
  case MCT_EVENT_CONTROL_CUSTOM: {
    boolean ret = FALSE;
    payload.data = cam_custom_ctr->data;
    payload.size = cam_custom_ctr->size;
    ret = awb_port_ext_update_opaque_input_params(awb_port,
      AWB_SET_PARM_CUSTOM_EVT_CTRL, &payload);
    if (FALSE == ret) {
      AF_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    } else {
      rc = STATS_EXT_HANDLING_COMPLETE;
    }
  }
    break;
  case MCT_EVENT_CONTROL_SET_PARM: {
    rc = awb_port_ext_handle_set_parm_event(port,
      ctrl_evt->control_event_data);
  }
    break;
  case MCT_EVENT_CONTROL_STREAMON: {
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
  break;
  case MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT: {
    awb_ext_param_t *ext_param = (awb_ext_param_t *)awb_port->ext_param;
    if (IS_USE_LED_EST(ext_param) &&
      IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_OFF)) {
      awb_port_ext_save_off_gain(awb_port);
    }
    AWB_LOW("MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT");
  }
    break;
  case MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT: {
    awb_ext_param_t *ext_param = (awb_ext_param_t *)awb_port->ext_param;
    if (IS_USE_LED_EST(ext_param) &&
      IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_OFF)) {
      awb_port_ext_update_led_state(awb_port, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN);
    }
    AWB_LOW("MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT");
  }
    break;
  default: {
    rc = STATS_EXT_HANDLING_FAILURE;
  }
    break;
  }

  return rc;
} /* awb_port_ext_handle_control_event */


/** awb_port_ext_handle_module_event:
 *    @port: port info
 *    @ctrl_evt: control event
 *
 * Extension of module event handling. Here OEM can further
 * handle/process module events. The module events can be OEM
 * specific or general. If it's OEM specific, OEM can either
 * process it here and send to core algorithm if required; or
 * just send the payload to core algorithm to process.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type awb_port_ext_handle_module_event(
  mct_port_t *port,
  mct_event_module_t *mod_evt)
{
  q3a_custom_data_t payload;
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  awb_port_private_t *awb_port =
    (awb_port_private_t *)(port->port_private);
  mct_custom_data_payload_t *cam_custom_evt =
    (mct_custom_data_payload_t *)mod_evt->module_event_data;
  awb_ext_param_t *ext_param = NULL;
  ext_param = (awb_ext_param_t *)awb_port->ext_param;

  AWB_LOW("Handle AWB module event of type: %d", mod_evt->type);

  switch (mod_evt->type) {
  case MCT_EVENT_MODULE_CUSTOM_STATS_DATA_AWB: {
    boolean ret = FALSE;
    payload.data = cam_custom_evt->data;
    payload.size = cam_custom_evt->size;
    rc = awb_port_ext_update_opaque_input_params(awb_port,
      AWB_SET_PARM_CUSTOM_EVT_MOD, &payload);
    if (FALSE == ret) {
      AWB_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    } else {
      rc = STATS_EXT_HANDLING_COMPLETE;
    }
  }
    break;
  case MCT_EVENT_MODULE_CUSTOM: {
      boolean ret = FALSE;
      payload.data = cam_custom_evt->data;
      payload.size = cam_custom_evt->size;
      ret = awb_port_ext_update_opaque_input_params(awb_port,
        AWB_SET_PARM_CUSTOM_EVT_MOD, &payload);
      if (FALSE == ret) {
        AF_ERR("Failure handling the custom parameter!");
        rc = STATS_EXT_HANDLING_FAILURE;
      } else {
        rc = STATS_EXT_HANDLING_COMPLETE;
      }
    }
      break;

    case MCT_EVENT_MODULE_STATS_AECAWB_GET_THREAD_OBJECT: {
      q3a_thread_aecawb_get_port_data_t *aecawb_get_data =
        (q3a_thread_aecawb_get_port_data_t *)(mod_evt->module_event_data);

      /* need it for dual cameras sharing thread, to backup original data */
      aecawb_get_data->camera_id   = awb_port->camera_id;
      aecawb_get_data->thread_data = awb_port->thread_data;

      rc = STATS_EXT_HANDLING_PARTIAL;
    }
      break;

    case MCT_EVENT_MODULE_STATS_AECAWB_SET_THREAD_OBJECT: {
      q3a_thread_aecawb_set_port_data_t *aecawb_set_data =
        (q3a_thread_aecawb_set_port_data_t *)(mod_evt->module_event_data);


      if (aecawb_set_data->awb_algo_instance) {
        /* need it for dual cameras sharing thread, set the algo instance to be use*/
        awb_port->awb_object.awb  = aecawb_set_data->awb_algo_instance;
        AWB_HIGH("AWB: session_id: %d: set camera_id: %d, set algo: %p",
          GET_SESSION_ID(awb_port->reserved_id), awb_port->camera_id,
          awb_port->awb_object.awb);
      }

      rc = STATS_EXT_HANDLING_PARTIAL;
    }
      break;

  case MCT_EVENT_MODULE_STATS_AECAWB_RESTORE_EXT_THREAD_OBJECT: {
    q3a_thread_restore_thread_data_t *restore_obj =
      (q3a_thread_restore_thread_data_t *)(mod_evt->module_event_data);

    AWB_HIGH("AWB port (%p): camera_id: %u algo: %p, restore for camera_id: %u thread data %p algo %p",
      awb_port, awb_port->camera_id, awb_port->awb_object.awb,
      restore_obj->camera_id, restore_obj->q3a_thread_data,
      restore_obj->awb_algo_instance);

    if (NULL == restore_obj) {
      AWB_ERR("Restore obj NULL");
      return FALSE;
    }

    awb_port->camera_id = restore_obj->camera_id;
    awb_port->thread_data = restore_obj->q3a_thread_data;
    awb_port->awb_object.awb = restore_obj->awb_algo_instance;

    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;

  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    boolean ret = FALSE;
    stats_update_t *stats_update = (stats_update_t *)mod_evt->module_event_data;
    q3a_thread_aecawb_msg_t *awb_msg =
      (q3a_thread_aecawb_msg_t *)malloc(sizeof(q3a_thread_aecawb_msg_t));
    if (awb_msg != NULL) {
      memset(awb_msg, 0, sizeof(q3a_thread_aecawb_msg_t));
      awb_msg->type = MSG_AWB_SET;
      awb_msg->is_priority = TRUE;
      awb_msg->u.awb_set_parm.type = AWB_SET_PARAM_AEC_PARM;
      awb_msg->u.awb_set_parm.u.aec_parms.average_luma =
        stats_update->aec_update.avg_luma;
      awb_msg->u.awb_set_parm.u.aec_parms.exp_index =
        stats_update->aec_update.exp_index;
      awb_msg->u.awb_set_parm.u.aec_parms.lux_idx =
        stats_update->aec_update.lux_idx;
      awb_msg->u.awb_set_parm.u.aec_parms.aec_settled =
        stats_update->aec_update.settled;
      awb_msg->u.awb_set_parm.u.aec_parms.cur_luma =
        stats_update->aec_update.cur_luma;
      awb_msg->u.awb_set_parm.u.aec_parms.target_luma =
        stats_update->aec_update.target_luma;
      awb_msg->u.awb_set_parm.u.aec_parms.cur_line_cnt =
        stats_update->aec_update.linecount;
      awb_msg->u.awb_set_parm.u.aec_parms.cur_real_gain =
        stats_update->aec_update.real_gain;
      awb_msg->u.awb_set_parm.u.aec_parms.stored_digital_gain =
        stats_update->aec_update.stored_digital_gain;
      awb_msg->u.awb_set_parm.u.aec_parms.total_drc_gain =
        stats_update->aec_update.total_drc_gain;
      awb_msg->u.awb_set_parm.u.aec_parms.flash_sensitivity =
        stats_update->aec_update.flash_sensitivity;
      awb_msg->u.awb_set_parm.u.aec_parms.led_state =
        stats_update->aec_update.led_state;
      awb_msg->u.awb_set_parm.u.aec_parms.use_led_estimation  =
        stats_update->aec_update.use_led_estimation;
      awb_msg->u.awb_set_parm.u.aec_parms.est_state =
        stats_update->aec_update.est_state;
      awb_msg->u.awb_set_parm.u.aec_parms.exp_tbl_val =
        stats_update->aec_update.exp_tbl_val;

      awb_msg->u.awb_set_parm.u.aec_parms.Bv =
        stats_update->aec_update.Bv;
      awb_msg->u.awb_set_parm.u.aec_parms.Tv =
        stats_update->aec_update.Tv;
      awb_msg->u.awb_set_parm.u.aec_parms.Sv =
        stats_update->aec_update.Sv;
      awb_msg->u.awb_set_parm.u.aec_parms.Av =
        stats_update->aec_update.Av;

      awb_msg->u.awb_set_parm.u.aec_parms.led_mode =
        stats_update->aec_update.led_mode;

      /* Store use_led_estimation */
      ext_param->use_led_estimation.prev =
        ext_param->use_led_estimation.crnt;
      ext_param->use_led_estimation.crnt =
        stats_update->aec_update.use_led_estimation;
      ext_param->led_state.prev = ext_param->led_state.crnt;
      ext_param->led_state.crnt = stats_update->aec_update.led_state;
      ext_param->led_mode = stats_update->aec_update.led_mode;
      ext_param->led_needed = stats_update->aec_update.led_needed;

      /* Handle custom parameters update (3a ext) */
      if (stats_update->aec_update.aec_custom_param_update.data &&
        stats_update->aec_update.aec_custom_param_update.size) {
        awb_msg->u.awb_set_parm.u.aec_parms.custom_param_awb.data =
          malloc(stats_update->aec_update.aec_custom_param_update.size);
        if (awb_msg->u.awb_set_parm.u.aec_parms.custom_param_awb.data) {
          awb_msg->u.awb_set_parm.u.aec_parms.custom_param_awb.size =
            stats_update->aec_update.aec_custom_param_update.size;
          memcpy(awb_msg->u.awb_set_parm.u.aec_parms.custom_param_awb.data,
            stats_update->aec_update.aec_custom_param_update.data,
            awb_msg->u.awb_set_parm.u.aec_parms.custom_param_awb.size);
        } else {
          AWB_ERR("Error: Fail to allocate memory for custom parameters");
          free(awb_msg);
          awb_msg = NULL;
          ret = FALSE;
          break;
        }
      }
      ret = q3a_aecawb_thread_en_q_msg(awb_port->thread_data, awb_msg);
    }

    if (FALSE == ret) {
      rc = STATS_EXT_HANDLING_FAILURE;
    } else {
      rc = STATS_EXT_HANDLING_COMPLETE;
    }
  }
    break;
  case MCT_EVENT_MODULE_STATS_AF_UPDATE: {
    boolean ret = FALSE;
    stats_update_t *stats_update = (stats_update_t *)mod_evt->module_event_data;

    if (IS_AF_UPDATE_VALID(stats_update) &&
      IS_AF_STATUS_DONE(stats_update->af_update.af_status)) {
      if (IS_USE_LED_EST(ext_param) &&
        !IS_UNIFIED_SAVED(ext_param) &&
        !IS_AF_DONE(ext_param)) {
        AWB_HIGH("AF_UPDATE module status:%d", stats_update->af_update.af_status);
        SET_AF_DONE(ext_param);

        awb_port_ext_update_led_state(awb_port, AWB_EXT_LED_STATUS_PREFLASH_AWB);
      }
    }
  }
    break;
  default: {
    AWB_LOW("Default. no action!");
  }
    break;
  }

  return rc;
} /* awb_port_ext_handle_module_event */

static stats_ext_return_type awb_port_ext_intra_event(
  mct_port_t *port, mct_event_t *event)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  awb_port_private_t *private;
  AWB_LOW("Received AWB intra-module peer event");

  /* sanity check */
  if (!port || !port->port_private || !event) {
    return FALSE;
  }

  private = (awb_port_private_t *)(port->port_private);

  switch(event->u.module_event.type) {
    default:
      rc = STATS_EXT_HANDLING_PARTIAL;
      break;
  }

  return rc;
}

static void awb_port_ext_adjust_led_gains(awb_output_data_t *output,
    awb_port_private_t *private)
{
  awb_ext_param_t *ext_param = (awb_ext_param_t *)private->ext_param;

  /* adjust gains */
  switch (ext_param->led_sm) {
  case AWB_EXT_LED_STATUS_PREFLASH_LOCKED:
  case AWB_EXT_LED_STATUS_RESTORE_OFFGAIN:
    awb_port_ext_restore_off_gain(output, private);
    break;

  case AWB_EXT_LED_STATUS_OFF:
  case AWB_EXT_LED_STATUS_PREFLASH_AEC:
  case AWB_EXT_LED_STATUS_PREFLASH_AF:
  case AWB_EXT_LED_STATUS_PREFLASH_AWB:
  default:
    break;
  }

  AWB_HIGH("[%s] gain[%1.3f:%1.3f:%1.3f-%04d] off[%1.3f:%1.3f:%1.3f-%04d] " \
    "led[%d]-[%d:%d] seq[%d:%d:%d] lock[%d:%d] off:uni[%d:%d] ledmode[%d] restorecnt[%d]",
    ext_param->led_sm == AWB_EXT_LED_STATUS_OFF ? "LED_OFF" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_PREFLASH_AEC ? "LED_PREFLASH_AEC" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_PREFLASH_AF ? "LED_PREFLASH_AF" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_PREFLASH_AWB ? "LED_PREFLASH_AWB" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_PREFLASH_LOCKED ? "LED_PREFLASH_LOCKED" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_RESTORE_OFFGAIN ? "LED_RESTORE_OFFGAIN" :
    "LED_STATUS_NONE!!!!",
    output->stats_update.awb_update.gain.r_gain,
    output->stats_update.awb_update.gain.g_gain,
    output->stats_update.awb_update.gain.b_gain,
    output->stats_update.awb_update.color_temp,
    ext_param->off_gain.r_gain,
    ext_param->off_gain.g_gain,
    ext_param->off_gain.b_gain,
    ext_param->off_color_temp,
    ext_param->led_state.crnt,
    ext_param->use_led_estimation.prev,
    ext_param->use_led_estimation.crnt,
    IS_EST_ALGO_DONE(ext_param),
    IS_EST_PORT_DONE(ext_param),
    IS_AF_DONE(ext_param),
    ext_param->lock.hal,
    ext_param->lock.algo,
    IS_OFFGAIN_SAVED(ext_param),
    IS_UNIFIED_SAVED(ext_param),
    ext_param->led_mode,
    ext_param->cnt_restore);
}

static void awb_port_ext_adjust_led_flags(awb_output_data_t *output,
    awb_port_private_t *private)
{
  awb_ext_param_t *ext_param = (awb_ext_param_t *)private->ext_param;

  /* adjust led flags */

  /* clear for starting preflash */
  /* Before normal operation for reseting starting pre-flash */
  if (IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN)) {
    if (ext_param->cnt_restore >= 0) {
      if (ext_param->led_state.crnt) {
        awb_port_ext_update_led_state(private, AWB_EXT_LED_STATUS_OFF);
      }
    }
  }

  /* normal preflash operation */
  if (IS_HISTORY_L_TO_H(ext_param->led_state)) {
    awb_port_ext_update_led_state(private, AWB_EXT_LED_STATUS_PREFLASH_AEC);
  }
  if (IS_EST_ALGO_DONE(ext_param) &&
    IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AEC) &&
    !IS_EST_PORT_DONE(ext_param) &&
    !IS_UNIFIED_SAVED(ext_param)) {
    awb_port_ext_update_led_state(private, AWB_EXT_LED_STATUS_PREFLASH_AF);
  }
  if (IS_AF_DONE(ext_param) &&
    IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_AWB) &&
    IS_EST_PORT_DONE(ext_param)) {
    awb_port_ext_update_led_state(private, AWB_EXT_LED_STATUS_PREFLASH_LOCKED);
  }
  if ((IS_HISTORY_H_TO_L(ext_param->use_led_estimation) &&
    !IS_EST_ALGO_DONE(ext_param) &&
    !IS_EST_PORT_DONE(ext_param)) &&
    !IS_OFFGAIN_SAVED(ext_param) &&
    IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN)) {
    awb_port_ext_update_led_state(private, AWB_EXT_LED_STATUS_OFF);
  }
  if (IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN)) {
    if (ext_param->cnt_restore-- == 0) {
      awb_port_ext_update_led_state(private, AWB_EXT_LED_STATUS_OFF);
    }
  }

  /* After normal operation for preventing remained use_led_est by Touch AEC */
  if (IS_LED_STATE(ext_param, AWB_EXT_LED_STATUS_PREFLASH_LOCKED)) {
    if (!IS_LOCKED_ALGO(ext_param)) {
      awb_port_ext_update_led_state(private, AWB_EXT_LED_STATUS_RESTORE_OFFGAIN);
    }
  }

  AWB_HIGH("[%s] gain[%1.3f:%1.3f:%1.3f-%04d] off[%1.3f:%1.3f:%1.3f-%04d] " \
    "led[%d]-[%d:%d] seq[%d:%d:%d] lock[%d:%d] off:uni[%d:%d] ledmode[%d] restorecnt[%d]",
    ext_param->led_sm == AWB_EXT_LED_STATUS_OFF ? "LED_OFF" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_PREFLASH_AEC ? "LED_PREFLASH_AEC" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_PREFLASH_AF ? "LED_PREFLASH_AF" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_PREFLASH_AWB ? "LED_PREFLASH_AWB" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_PREFLASH_LOCKED ? "LED_PREFLASH_LOCKED" :
    ext_param->led_sm == AWB_EXT_LED_STATUS_RESTORE_OFFGAIN ? "LED_RESTORE_OFFGAIN" :
    "LED_STATUS_NONE!!!!",
    output->stats_update.awb_update.gain.r_gain,
    output->stats_update.awb_update.gain.g_gain,
    output->stats_update.awb_update.gain.b_gain,
    output->stats_update.awb_update.color_temp,
    ext_param->off_gain.r_gain,
    ext_param->off_gain.g_gain,
    ext_param->off_gain.b_gain,
    ext_param->off_color_temp,
    ext_param->led_state.crnt,
    ext_param->use_led_estimation.prev,
    ext_param->use_led_estimation.crnt,
    IS_EST_ALGO_DONE(ext_param),
    IS_EST_PORT_DONE(ext_param),
    IS_AF_DONE(ext_param),
    ext_param->lock.hal,
    ext_param->lock.algo,
    IS_OFFGAIN_SAVED(ext_param),
    IS_UNIFIED_SAVED(ext_param),
    ext_param->led_mode,
    ext_param->cnt_restore);
}

static void awb_port_ext_pack_output(awb_output_data_t *output,
  awb_gain_t *rgb_gain,
  awb_port_private_t *private)
{
  awb_ext_param_t *ext_param = NULL;
  ext_param = (awb_ext_param_t *)private->ext_param;
  output->stats_update.flag = STATS_UPDATE_AWB;

  /* STATS_MEMSET the output struct */
  STATS_MEMSET(&output->stats_update.awb_update, 0,
    sizeof(awb_update_t));

  output->stats_update.awb_update.stats_frm_id = output->frame_id;

  /*RGB gain*/
  rgb_gain->r_gain = output->r_gain;
  rgb_gain->g_gain = output->g_gain;
  rgb_gain->b_gain = output->b_gain;

  /* color_temp */
  output->stats_update.awb_update.gain = *rgb_gain;
  output->stats_update.awb_update.unadjusted_gain.r_gain = output->unadjusted_r_gain;
  output->stats_update.awb_update.unadjusted_gain.g_gain = output->unadjusted_g_gain;
  output->stats_update.awb_update.unadjusted_gain.b_gain = output->unadjusted_b_gain;
  output->stats_update.awb_update.color_temp = output->color_temp;
  output->stats_update.awb_update.wb_mode = output->wb_mode;
  output->stats_update.awb_update.best_mode = output->best_mode;
  output->stats_update.awb_update.decision = output->decision;

  /* Copy ccm output to awb_update */
  output->stats_update.awb_update.ccm_update.awb_ccm_enable =
    output->awb_ccm_enable;
  private->awb_ccm_enable = output->awb_ccm_enable;
  output->stats_update.awb_update.ccm_update.ccm_update_flag =
    output->ccm.override_ccm;
  STATS_MEMCPY(&output->stats_update.awb_update.ccm_update.ccm,
    sizeof(output->stats_update.awb_update.ccm_update.ccm),
    &output->ccm.ccm, sizeof(output->ccm.ccm));
  STATS_MEMCPY(&output->stats_update.awb_update.ccm_update.ccm_offset,
    sizeof(output->stats_update.awb_update.ccm_update.ccm_offset),
    &output->ccm.ccm_offset, sizeof(output->ccm.ccm_offset));

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  /* Dual led driving currents and corresponding AEC flux */
  output->stats_update.awb_update.dual_led_setting = output->dual_led_settings;
  output->stats_update.awb_update.dual_led_setting.is_valid = true;
#endif
  output->stats_update.awb_update.dual_led_flux_gain = output->dual_led_flux_gain;
  STATS_MEMCPY(output->stats_update.awb_update.sample_decision,
    sizeof(output->stats_update.awb_update.sample_decision),
    output->samp_decision, sizeof(output->samp_decision));
  /* Handle Post bus msgs*/
  private->awb_roi.rect.left   = output->awb_roi_info.roi[0].x;
  private->awb_roi.rect.top    = output->awb_roi_info.roi[0].y;
  private->awb_roi.rect.width  = output->awb_roi_info.roi[0].dx;
  private->awb_roi.rect.height = output->awb_roi_info.roi[0].dy;
  private->awb_roi.weight      = output->awb_roi_info.weight[0];
  private->op_mode             = output->op_mode;
  private->awb_locked          = output->awb_lock;
  private->is_awb_converge     = output->is_awb_converge;
  private->conv_speed          = output->conv_speed;

  /* Pack to out awb_state */
  output->stats_update.awb_update.awb_state = private->awb_state;

  /* Adjust gains before update state/wb */
  awb_port_ext_adjust_led_gains(output, private);

  STATS_MEMCPY(&private->awb_output, sizeof(private->awb_output),
    output, sizeof(awb_output_data_t));
  awb_port_update_state(private);
  awb_port_update_wb(private, output);

  /* Save the awb update to stored param for next use */
  if (private->stored_params && private->stored_params->enable && !private->flash_on) {
    awb_stored_params_type *stored_params = private->stored_params;
    stored_params->gains.r_gain = output->stats_update.awb_update.gain.r_gain;
    stored_params->gains.g_gain = output->stats_update.awb_update.gain.g_gain;
    stored_params->gains.b_gain = output->stats_update.awb_update.gain.b_gain;
    stored_params->color_temp = output->stats_update.awb_update.color_temp;
  }

  /* Adjust internal flags after update state/wb */
  awb_port_ext_adjust_led_flags(output, private);
}

/** awb_port_ext_callback:
 *    @port: port info
 *    @awb_out: Output parameters from core algorithm
 *    @update: awb updates to be sent to other modules.
 *
 * Extension of awb core callback. Here OEM can process output
 * parameters received from AF core. There might be OEM specific
 * parameters as well as standard output which OEM might want to
 * handle on its own.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type awb_port_ext_callback(
  mct_port_t *port,
  void *core_out,
  void *update)
{
  uint8_t cam_cnt = 0;
  awb_port_private_t *private = (awb_port_private_t *)(port->port_private);
  awb_output_data_t *awb_out = (awb_output_data_t *)core_out;
  awb_update_t *awb_update = (awb_update_t *)update;
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;
  awb_gain_t rgb_gain;

  /* Handle custom params */
  /* Send the custom parameters as it is. Receiver will handle as required */
  awb_update->awb_custom_param_update.data = awb_out->awb_custom_param.data;
  awb_update->awb_custom_param_update.size = awb_out->awb_custom_param.size;

  /* handle other parameters if required. */
  /* populate the stats_upate object to be sent out*/
  AWB_LOW("%s: STATS_UPDATE_AWB", __func__);
  if (AWB_UPDATE_OFFLINE == awb_out->type) {
    AWB_LOW("Offline AWB status update!");
    awb_port_callback_offline_proc(port, awb_out);
  } else if (AWB_UPDATE == awb_out->type) {
    MCT_OBJECT_LOCK(port);
    private->awb_update_flag = TRUE;
    MCT_OBJECT_UNLOCK(port);
  } else if ((AWB_SEND_OUTPUT_EVENT == awb_out->type) &&
    (FALSE == private->stats_frame_capture.frame_capture_mode)) {
    awb_port_ext_pack_output(awb_out, &rgb_gain, private);
    awb_port_send_event(port, MCT_EVENT_MODULE_EVENT,
      MCT_EVENT_MODULE_STATS_AWB_UPDATE,
      (void *)(&awb_out->stats_update),awb_out->sof_id);
    if (awb_out->need_config) {
      awb_port_configure_stats(awb_out, port);
      awb_out->need_config = 0;
    }

    if (awb_out->eztune_data.ez_running) {
        awb_port_send_awb_info_to_metadata(port, awb_out);
    }
    /* Save the awb debug data in private data struct to be sent out later */
    private->awb_debug_data_size = awb_out->awb_debug_data_size;
    if (awb_out->awb_debug_data_size) {
      memcpy(private->awb_debug_data_array, awb_out->awb_debug_data_array,
        awb_out->awb_debug_data_size);
    }
  }

  return ret;
}


/** awb_port_ext_init:
 *    @port: port info
 *    @session_id: current session id
 *
 * Extension of awb port init.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type awb_port_ext_init(
  mct_port_t *port,
  unsigned int session_id)
{
  (void)session_id;
  awb_port_private_t *private = (awb_port_private_t *)(port->port_private);
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  awb_ext_param_t *ext_param = NULL;

  ext_param = (awb_ext_param_t *) calloc(1, sizeof(awb_ext_param_t));
  if (NULL == ext_param) {
    AF_ERR("Error allocating memory for extension init!");
    rc = STATS_EXT_HANDLING_FAILURE;
  } else {
    /* Output of core algorithm will have void pointer to point to custom output
       parameters. Assign space to save those parameters. */
    private->awb_object.awb_custom_param.data =
      (void *)&ext_param->custom_output;
    private->awb_object.awb_custom_param.size =
      sizeof(ext_param->custom_output);
    private->ext_param = ext_param;
  }

  return rc;
}

/** awb_port_ext_deinit:
 *    @port: port info
 *
 * Extension of awb port init.
 *
 * Return: none
 **/
static void awb_port_ext_deinit(mct_port_t *port)
{
  awb_port_private_t *private = (awb_port_private_t *)(port->port_private);
  if (private->ext_param) {
    free(private->ext_param);
  }
}

/** awb_port_ext_update_func_table:
 *    @ptr: pointer to internal awb pointer object
 *
 * Update extendable function pointers.
 *
 * Return: TRUE in sucess
 **/
boolean awb_port_ext_update_func_table(void *ptr)
{
  awb_port_private_t *private = (awb_port_private_t *)ptr;
  private->func_tbl.ext_init = awb_port_ext_init;
  private->func_tbl.ext_deinit = awb_port_ext_deinit;
  private->func_tbl.ext_callback = awb_port_ext_callback;
  private->func_tbl.ext_handle_module_event = awb_port_ext_handle_module_event;
  private->func_tbl.ext_handle_control_event =
    awb_port_ext_handle_control_event;
  private->func_tbl.ext_handle_intra_event = awb_port_ext_intra_event;

  return TRUE;
}

/**
 * awb_port_ext_is_extension_required
 *
 * @awb_libptr: Pointer to the vendor library
 * @cam_position: Camera position
 * @use_default_algo: The decision to use or not default (QC) algo is returned by this flag
 *
 * Return: TRUE is extension is required
 **/
boolean awb_port_ext_is_extension_required(void **awb_libptr,
  cam_position_t cam_position, boolean *use_default_algo)
{
  boolean rc = FALSE;
  int test_dual_out = 0;
  (void)cam_position;
  *use_default_algo = FALSE;

  if (*awb_libptr) {
    rc = TRUE;
  } else {
#ifdef _AWB_EXTENSION_
    *awb_libptr = dlopen(_AWB_EXT_ALGO_FNAME, RTLD_NOW);
    if (*awb_libptr) {
      *use_default_algo = FALSE;
    } else {
      AWB_HIGH("dl_ret_code %s, set use_default_algo.", dlerror());
      *use_default_algo = TRUE;
    }
    AWB_HIGH("handle:%p, use_default_alg:%d", *awb_libptr, *use_default_algo);
    rc = TRUE;
#endif
  }

  STATS_TEST_DUAL_OUTPUT(test_dual_out);
  if (test_dual_out) {
    /* use default algo */
    *use_default_algo = TRUE;
    /* Enable extension */
    rc =  TRUE;
  }

  return rc;
}

void *awb_port_load_external(awb_ops_t *awb_ops, void *libptr,
  cam_position_t pos)
{
  if (!awb_ops) {
    return NULL;
  }
  if (pos == CAM_POSITION_BACK) {
    awb_ops->init            = ext_awb_algo_rear_init;
  } else {
    awb_ops->init            = ext_awb_algo_front_init;
  }
  awb_ops->set_parameters  = ext_awb_algo_set_param;
  awb_ops->get_parameters  = ext_awb_algo_get_param;
  awb_ops->process         = ext_awb_algo_process;
  awb_ops->deinit          = ext_awb_algo_deinit;
  awb_ops->estimate_cct    = ext_awb_algo_estimate_cct;
  awb_ops->estimate_ccm    = ext_awb_algo_estimate_ccm;

  AWB_HIGH("Load External algo functions %p, for %s",
    libptr, pos == CAM_POSITION_BACK ? "BACK" : "FRONT");

  return libptr;
}

/**
 * awb_port_ext_load_function
 *
 * @awb_ops: structure with function pointers to be assign
 * @awb_libptr: Parameter to provide the pointer to the aec library (optional)
 * @cam_position: Camera position
 * @use_default_algo: Using or not default algo flag
 *
 *  This function is intended to be used by OEM.
 *  The OEM must use this fuction to populate the algo interface
 *  function pointers.
 *
 * Return: Handle to AWB interface library
 **/
void * awb_port_ext_load_function(awb_ops_t *awb_ops, void *awb_libptr,
  cam_position_t cam_position, boolean use_default_algo)
{
  void *awb_handle = NULL;
  int test_dual_out = 0;

  if (use_default_algo) {
    AWB_HIGH("Load default algo functions");
    STATS_TEST_DUAL_OUTPUT(test_dual_out);
    if (test_dual_out) {
      awb_handle = awb_ext_test_wrapper_load_function(awb_ops);
    } else {
      awb_handle = awb_port_load_function(awb_ops);
    }
  } else {
    if (awb_libptr) {
#ifdef _EXTERNAL_USE_FRONT_
      if (cam_position == CAM_POSITION_BACK) {
        awb_handle = awb_port_load_function(awb_ops);
      } else {
        awb_handle = awb_port_load_external(awb_ops, awb_libptr, cam_position);
      }
#elif _EXTERNAL_USE_REAR_
      if (cam_position == CAM_POSITION_BACK) {
        awb_handle = awb_port_load_external(awb_ops, awb_libptr, cam_position);
      } else {
        awb_handle = awb_port_load_function(awb_ops);
      }
#elif _EXTERNAL_USE_BOTH_
      awb_handle = awb_port_load_external(awb_ops, awb_libptr, cam_position);
#elif _EXTERNAL_USE_NONE_
      awb_handle = awb_port_load_function(awb_ops);
#endif
      AWB_HIGH("Load algo for position %d", cam_position);
    } else {
      AWB_ERR("Error: This is a DUMMY function, used only for reference");
      awb_handle = NULL;
    }
  }

  return awb_handle;
}

/** awb_port_ext_unload_function
 *
 *    @private: Port private structure
 *
 *  This function is intended to be used by OEM.
 *  The OEM must use this fuction to free resources allocated at
 *  awb_port_ext_load_function()
 *
 * Return: void
 **/
void awb_port_ext_unload_function(awb_port_private_t *private)
{
  int test_dual_out = 0;
  if (!private) {
    return;
  }

  if (private->use_default_algo) {
    STATS_TEST_DUAL_OUTPUT(test_dual_out);
    if (test_dual_out) {
      awb_ext_test_wrapper_unload_function(&private->awb_object.awb_ops,
        private->awb_iface_lib);
      awb_port_load_dummy_default_func(&private->awb_object.awb_ops);
      private->awb_iface_lib = NULL;
    } else {
      awb_port_unload_function(private);
    }
  } else {
    if (private->awb_iface_lib) {
      awb_ops_t *awb_ops = &private->awb_object.awb_ops;
      awb_ops->set_parameters = NULL;
      awb_ops->get_parameters = NULL;
      awb_ops->process = NULL;
      awb_ops->init = NULL;
      awb_ops->deinit = NULL;
      awb_ops->estimate_cct = NULL;
      awb_ops->estimate_gains = NULL;

      dlclose(private->awb_iface_lib);
      private->awb_iface_lib = NULL;
      awb_port_load_dummy_default_func(&private->awb_object.awb_ops);
      AWB_HIGH("Unload External algo");
    } else {
      AWB_ERR("Error: This is a DUMMY function, used only for reference");
      /* Reset original value of interface */
      awb_port_load_dummy_default_func(&private->awb_object.awb_ops);
    }
  }

  return;
}
