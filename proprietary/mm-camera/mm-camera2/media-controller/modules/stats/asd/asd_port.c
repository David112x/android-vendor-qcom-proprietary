/* asd_port.c
 *
 * Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <dlfcn.h>
#include "asd_port.h"
#include "asd_thread.h"
#include "asd.h"
#include "modules.h"
#include "stats_event.h"
#include "stats_util.h"
#include "asd_ext.h"

#undef LOG_TAG
#define LOG_TAG "ASD"

/*Forward Declarations*/
static boolean asd_port_link_to_peer(mct_port_t *port,
                                         mct_event_t *event);
static boolean asd_port_unlink_from_peer(mct_port_t *port,
                                               mct_event_t *event);
static boolean asd_port_local_unlink(mct_port_t *port,
                                        mct_event_t *event);
static boolean asd_port_remote_unlink(mct_port_t *port);
static boolean asd_port_handle_role_switch(mct_port_t *port,
                                                mct_event_t *event);
static boolean asd_port_set_role_switch(asd_port_private_t  *private);
static boolean asd_port_reset_dual_cam_info(asd_port_private_t  *port);
static boolean asd_port_if_stats_can_be_configured(asd_port_private_t  *port);
static boolean asd_port_update_aux_sync_and_update(asd_port_private_t  *port);
static boolean asd_port_update_LPM(mct_port_t *port, cam_dual_camera_perf_control_t* perf_ctrl);
static boolean asd_port_is_algo_active(mct_port_t *port);
static boolean asd_port_forward_bus_message_if_linked(mct_port_t *mct_port ,
                                                              mct_bus_msg_asd_decision_t* asd_scene_msg);
static boolean asd_port_forward_update_event_if_linked(mct_port_t* mct_port,
                                                              stats_update_t* stats_update);
static boolean asd_port_store_peer_bus_message(mct_port_t *mct_port ,
                                                       mct_bus_msg_asd_decision_t* asd_scene_msg);
static boolean asd_port_store_peer_update_event(mct_port_t* mct_port,
                                                      stats_update_t* stats_update);
static boolean asd_port_intra_event(mct_port_t *port, mct_event_t *event);

/** asd_port_reset_ext_func_tbl
 *    @private: port private data
 *
 *  Clean ASD extension function pointers
 *
 *  Return void
 **/
void asd_port_reset_ext_func_tbl(asd_port_private_t *private)
{
  private->func_tbl.ext_init = NULL;
  private->func_tbl.ext_deinit = NULL;
  private->func_tbl.ext_callback = NULL;
  private->func_tbl.ext_handle_module_event = NULL;
  private->func_tbl.ext_handle_control_event = NULL;
  return;
}

/** asd_port_is_asd_algo_iface_valid
 *    @asd_ops: ASD algo interface
 *
 *  Verify if the ASD interface have been set
 *
 *  Return TRUE on success.
 **/
boolean asd_port_is_asd_algo_iface_valid(asd_ops_t *asd_ops)
{
  boolean rc = FALSE;
  if (asd_ops->init &&
      asd_ops->deinit &&
      asd_ops->set_parameters &&
      asd_ops->get_parameters &&
      asd_ops->process) {
    rc = TRUE;
  }

  return rc;
}

/** asd_port_fill_module_update
 *
 *    @private: ASD port private data
 *    @stats_update: Structure to fill base on @output
 *    @output: Resulting output from ASD algo
 *
 *  Fill ASD update structure with the data coming from algo.
 *  Use asd_update_type to decide what data to fill in ASD update.
 *
 *  Return TRUE on success.
 **/
static boolean asd_port_fill_module_update(asd_port_private_t *private,
  stats_update_t *stats_update, asd_output_data_t *output)
{
  uint32_t           *scene_severity = NULL;
  cam_auto_scene_t   scene_idx = 0;
  boolean do_update = TRUE;

  /* Set ASD update type */
  stats_update->flag = STATS_UPDATE_ASD;

  /* Main Enable/Disable switch */
  if (output->asd_enable || private->asd_enable != output->asd_enable) {
    /* update if disable, but only once */
    do_update = TRUE;
  } else {
    /* it's disable already, no need to update again */
    do_update = FALSE;
  }

  private->asd_enable = output->asd_enable;
  stats_update->asd_update.asd_enable = output->asd_enable;

  if (FALSE == output->asd_enable) {
    return do_update;
  }

  /* Update reported scene, but value may be overwritten based on update_type */
  stats_update->asd_update.scene = output->scene;

  /* Backlight */
  stats_update->asd_update.histo_backlight_detected =
    output->histo_backlight_detected;
  stats_update->asd_update.backlight_detected = output->backlight_detected;
  stats_update->asd_update.backlight_luma_target_offset =
    output->backlight_luma_target_offset;
  stats_update->asd_update.backlight_scene_severity =
    output->backlight_scene_severity;
  stats_update->asd_update.mixed_light = output->mixed_light;

  /* Snow */
  stats_update->asd_update.snow_or_cloudy_scene_detected =
    output->snow_or_cloudy_scene_detected;
  stats_update->asd_update.snow_or_cloudy_luma_target_offset =
    output->snow_or_cloudy_luma_target_offset;

  /* Landscape */
  stats_update->asd_update.landscape_severity = output->landscape_severity;

  /* Portrait */
  stats_update->asd_update.portrait_severity = output->portrait_severity;
  stats_update->asd_update.asd_soft_focus_dgr = output->soft_focus_dgr;

  /* HDR */
  stats_update->asd_update.is_hdr_scene = output->is_hdr_scene;
  stats_update->asd_update.hdr_confidence = output->hdr_confidence;

  if (stats_update->asd_update.asd_enable) {
    scene_severity = &stats_update->asd_update.severity[0];
    for (scene_idx = 0; scene_idx < S_MAX; scene_idx++) {
      scene_severity[scene_idx] = output->severity[scene_idx];
    }
  }

  ASD_LOW("ASD enable: %d / %d, scene: %d, reported_scene: %d, do_update: %d",
    output->asd_enable, stats_update->asd_update.asd_enable,
    output->scene,
    stats_update->asd_update.scene, do_update);
  return do_update;
}

/** asd_send_bus_message
 *    @port: asd port
 *    @bus_msg_type:   bus_msg_type
 *    @payload: payload
 *    @size: size
 *    @sof_id: sof
 **/
void asd_send_bus_message(mct_port_t *port,
  mct_bus_msg_type_t bus_msg_type,
  void* payload,
  int size,
  int sof_id)
{
  asd_port_private_t *asd_port = (asd_port_private_t *)(port->port_private);
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  STATS_MEMSET(&bus_msg, 0, sizeof(mct_bus_msg_t));

  bus_msg.sessionid = (asd_port->reserved_id >> 16);
  bus_msg.type = bus_msg_type;
  bus_msg.msg = payload;
  bus_msg.size = size;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = asd_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.current_frame_id = sof_id;
  event.u.module_event.module_event_data = (void *)(&bus_msg);

  MCT_PORT_EVENT_FUNC(port)(port, &event);
  return;
}

void asd_port_send_asd_info_to_eztune(mct_port_t *port,
                                               asd_output_data_t *output)
{
  mct_event_t        event;
  mct_bus_msg_t      bus_msg;
  asd_ez_tune_t      asd_info;
  asd_port_private_t *private;
  int32_t            size;

  if (!output || !port) {
    ASD_ERR("input error");
    return;
  }

  private = (asd_port_private_t *)(port->port_private);
  STATS_MEMSET(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_ASD_EZTUNING_INFO;
  bus_msg.msg = (void *)&asd_info;
  size = (int32_t)sizeof(asd_ez_tune_t);
  bus_msg.size = size;

  STATS_MEMCPY(&asd_info, sizeof(asd_ez_tune_t), &output->asd_eztune, size);

  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);

  MCT_PORT_EVENT_FUNC(port)(port, &event);

  return;
}

/** asd_port_send_exif_debug_data:
 * Return nothing
 **/
static void asd_port_send_exif_debug_data(mct_port_t *port)
{
  mct_event_t          event;
  mct_bus_msg_t        bus_msg;
  cam_asd_exif_debug_t *asd_info;
  asd_port_private_t   *private;
  int                  size;

  if (!port) {
    ASD_ERR("input error");
    return;
  }
  private = (asd_port_private_t *)(port->port_private);
  if (private == NULL) {
    return;
  }

  /* Send exif data if data size is valid */
  if (!private->asd_debug_data_size) {
    ASD_LOW("asd_port: Debug data not available");
    return;
  }
  asd_info = (cam_asd_exif_debug_t *) malloc(sizeof(cam_asd_exif_debug_t));
  if (!asd_info) {
    ASD_ERR("Failure allocating memory for debug data");
    return;
  }
  STATS_MEMSET(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_ASD_EXIF_DEBUG_INFO;
  bus_msg.msg = (void *)asd_info;
  size = (int)sizeof(cam_asd_exif_debug_t);
  bus_msg.size = size;
  STATS_MEMSET(asd_info, 0, size);
  asd_info->asd_debug_data_size = private->asd_debug_data_size;

  ASD_LOW("asd_debug_data_size: %d", private->asd_debug_data_size);
  STATS_MEMCPY(asd_info->asd_private_debug_data, sizeof(asd_info->asd_private_debug_data),
    private->asd_debug_data_array, private->asd_debug_data_size);
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
  if (asd_info) {
    free(asd_info);
  }
}

static void asd_port_stats_done_callback(void *p, void* stats)
{
  mct_port_t         *port    = (mct_port_t *)p;
  asd_port_private_t *private = NULL;
  stats_t            *asd_stats = (stats_t *)stats;

  if (!port || !stats) {
      return;
  }

  private = (asd_port_private_t *)(port->port_private);
  if (!private) {
    return;
  }

  ASD_HIGH("Done ASD STATS ACK back");

  circular_stats_data_done(asd_stats->ack_data, port,
                           private->reserved_id, 0);
}

/** asd_port_callback
 *
 **/
static void asd_port_callback(asd_output_data_t *output, void *p)
{
  mct_port_t         *port    = (mct_port_t *)p;
  asd_port_private_t *private = NULL;
  mct_event_t        event;
  stats_update_t     stats_update;
  cam_auto_scene_t   idx = S_NORMAL;
  float              scene_confidence = 0.0;
  boolean            do_asd_update = FALSE;
  mct_bus_msg_asd_decision_t  asd_scene_msg;

  if (!output || !port) {
      return;
  }
  private = (asd_port_private_t *)(port->port_private);
  STATS_MEMSET(&stats_update, 0, sizeof(stats_update_t));

  /* First handle callback in extension if available */
  if (private->func_tbl.ext_callback) {
    stats_ext_return_type ret;
    ret = private->func_tbl.ext_callback(
      port, output, &stats_update);
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      ASD_LOW("Callback handled. Skipping rest!");
      return;
    }
  }

  /* Verify if update to other modules is required */
  if (ASD_UPDATE == output->type || ASD_SKIP_UPDATE == output->type) {
    if (output->asd_eztune.ez_running) {
      asd_port_send_asd_info_to_eztune(port, output);
    }

    do_asd_update = asd_port_fill_module_update(private, &stats_update, output);
    asd_port_forward_update_event_if_linked(port,&stats_update);

    if (do_asd_update) {
      /* Send ASD modules update*/
      event.direction = MCT_EVENT_UPSTREAM;
      event.identity  = private->reserved_id;
      event.type      = MCT_EVENT_MODULE_EVENT;
      event.u.module_event.type            = MCT_EVENT_MODULE_STATS_ASD_UPDATE;
      event.u.module_event.module_event_data = (void *)(&stats_update);
      MCT_PORT_EVENT_FUNC(port)(port, &event);
    }
  }

  /* Verify if update to HAL is required (BUS_MSG) */
  if (ASD_UPDATE == output->type) {
    asd_scene_msg.detected_scene = output->scene;
    asd_scene_msg.max_n_scenes = output->max_n_scenes;
    asd_scene_msg.scene_info[asd_scene_msg.detected_scene].auto_compensation = TRUE;
    asd_scene_msg.scene_info[asd_scene_msg.detected_scene].detected = TRUE;
    for (idx = 0; idx < asd_scene_msg.max_n_scenes; idx++) {
      if (idx != S_HDR) {
        scene_confidence = (float)output->severity[idx];
        scene_confidence = scene_confidence / 255.0f; /* Convert to % */
        asd_scene_msg.scene_info[idx].confidence = scene_confidence;
      }
    }

    /* Algo handles HDR scene independently, fill data here */
    asd_scene_msg.scene_info[S_HDR].detected = output->is_hdr_scene;
    asd_scene_msg.scene_info[S_HDR].confidence = output->hdr_confidence;
    asd_scene_msg.scene_info[S_HDR].auto_compensation = FALSE;

    /* Save last update, to be use if no new data is available */
    private->last_asd_decision = asd_scene_msg;
    asd_port_forward_bus_message_if_linked(port,&asd_scene_msg);
    asd_send_bus_message(port, MCT_BUS_MSG_AUTO_SCENE_INFO,
      (void*)&asd_scene_msg, sizeof(mct_bus_msg_asd_decision_t),
      STATS_REPORT_IMMEDIATE);

    ASD_LOW("BSUPD Report scene: %d, is_hdr: %d, hdr_confidence: %f",
      asd_scene_msg.detected_scene,
      asd_scene_msg.scene_info[S_HDR].detected,
      asd_scene_msg.scene_info[S_HDR].confidence);

    /* Save the debug data in private data struct to be sent out later */
    private->asd_debug_data_size = output->asd_debug_data_size;
    if (output->asd_debug_data_size) {
      STATS_MEMCPY(private->asd_debug_data_array, sizeof(private->asd_debug_data_array),
        output->asd_debug_data_array, output->asd_debug_data_size);
    }
  } else if (ASD_SEND_EVENT == output->type) {
    /* This will not be invoked from asd thread.
     * Keeping the output->type to keep the interface same as other modules */
  } else if (ASD_SKIP_UPDATE == output->type &&
    (do_asd_update || output->asd_enable)) {
    ASD_LOW("Report scene: %d, is_hdr: %d, hdr_confidence: %f (saved prev scene)",
      private->last_asd_decision.detected_scene,
      private->last_asd_decision.scene_info[S_HDR].detected,
      private->last_asd_decision.scene_info[S_HDR].confidence);
    asd_send_bus_message(port, MCT_BUS_MSG_AUTO_SCENE_INFO,
      (void*)&(private->last_asd_decision), sizeof(mct_bus_msg_asd_decision_t),
      STATS_REPORT_IMMEDIATE);
    ASD_LOW("BSUPD Report scene: %d, is_hdr: %d, hdr_confidence: %f",
      private->last_asd_decision.detected_scene,
      private->last_asd_decision.scene_info[S_HDR].detected,
      private->last_asd_decision.scene_info[S_HDR].confidence);
  }

  return;
}

/** asd_port_start_threads
 *    @port:
 **/
static boolean asd_port_init_threads(mct_port_t *port)
{
  boolean     rc = FALSE;
  asd_port_private_t *private = port->port_private;

  private->thread_data = asd_thread_init();
  ASD_LOW("private->thread_data: %p", private->thread_data);
  if (private->thread_data != NULL) {
    rc = TRUE;
    private->thread_data->asd_port = port;
    private->thread_data->asd_obj = &private->asd_object;
    private->thread_data->asd_cb = asd_port_callback;
    private->thread_data->asd_stats_cb = asd_port_stats_done_callback;
  } else {
    ASD_ERR("Error: private->thread_data is NULL");
  }
  return rc;
}

/** asd_port_start_threads
 *    @port:
 **/
static boolean asd_port_start_threads(mct_port_t *port)
{
  boolean     rc = FALSE;
  asd_port_private_t *private = port->port_private;

  if (private->thread_data != NULL) {
    ASD_LOW("Start asd thread");
    rc = asd_thread_start(private->thread_data);
    if (rc == FALSE) {
      asd_thread_deinit(private->thread_data);
    }
  }
  return rc;
}

/** asd_port_check_session_id
 *    @d1: session+stream identity
 *    @d2: session+stream identity
 *
 *  To find out if both identities are matching;
 *  Return TRUE if matches.
 **/
static boolean asd_port_check_session_id(void *d1, void *d2)
{
  unsigned int v1, v2;
  v1 = *((unsigned int *)d1);
  v2 = *((unsigned int *)d2);

  return (((v1 & 0xFFFF0000) ==
          (v2 & 0xFFFF0000)) ? TRUE : FALSE);
}

static boolean asd_port_event_stats_data( asd_port_private_t *port_private,
                               mct_event_t *event)
{
  boolean rc = TRUE;
  mct_event_module_t *mod_evt = &(event->u.module_event);
  mct_event_stats_ext_t *stats_ext_event;
  mct_event_stats_isp_t *stats_event ;
  stats_ext_event = (mct_event_stats_ext_t *)(mod_evt->module_event_data);
  stats_event = stats_ext_event->stats_data;

  if (stats_event) {
    if (stats_event->stats_mask) {
      if(!((stats_event->stats_mask & (1 << MSM_ISP_STATS_IHIST)) ||
           stats_event->stats_mask & (1 << MSM_ISP_STATS_HDR_BHIST) ||
           stats_event->stats_mask & (1 << MSM_ISP_STATS_BHIST)) ) {
        return TRUE;
      }
      stats_t * asd_stats = (stats_t *)calloc(1, sizeof(stats_t));
      if(asd_stats == NULL)
        return TRUE;
      asd_stats->frame_id = stats_event->frame_id;
      ASD_LOW("hist received stats of mask: %d",
           stats_event->stats_mask);
      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_IHIST)) {
        ASD_LOW("IHISTO Stats!");
        asd_stats->stats_type_mask |= STATS_IHISTO;
        asd_stats->yuv_stats.p_histogram = stats_event->stats_data[MSM_ISP_STATS_IHIST].stats_buf;
      }

      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_HDR_BHIST)) {
        ASD_LOW("HDR BHISTO Stats!");
        asd_stats->stats_type_mask |= STATS_HBHISTO;
        asd_stats->bayer_stats.p_q3a_bhist_stats = stats_event->stats_data[MSM_ISP_STATS_HDR_BHIST].stats_buf;
      } else if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BHIST)) {
        ASD_LOW("BHISTO Stats!");
        asd_stats->stats_type_mask |= STATS_BHISTO;
        asd_stats->bayer_stats.p_q3a_bhist_stats = stats_event->stats_data[MSM_ISP_STATS_BHIST].stats_buf;
      }

      asd_thread_msg_t * asd_msg = (asd_thread_msg_t *)
        malloc(sizeof(asd_thread_msg_t));

      if (asd_msg) {
        STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
        asd_msg->type = MSG_ASD_STATS;
        asd_msg->u.stats = asd_stats;

        asd_stats->ack_data = stats_ext_event;
        circular_stats_data_use(stats_ext_event);

        rc = asd_thread_en_q_msg(port_private->thread_data, asd_msg);

        if (!rc) {
          circular_stats_data_done(stats_ext_event, 0, 0, 0);
          /* stats msg and payload will be free'd from inside enq_msg call */
        }
      } else {
        free(asd_stats);
      }
    }
  }
  return rc;
}


/** asd_port_proc_downstream_event:
 *
 **/
static boolean asd_port_proc_downstream_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;
  asd_port_private_t *private = (asd_port_private_t *)(port->port_private);
  mct_event_module_t *mod_evt = &(event->u.module_event);

  ASD_LOW("Received module event of type: %d", mod_evt->type);
  /* Check if extended handling to be performed */
  if (private->func_tbl.ext_handle_module_event) {
    ret = private->func_tbl.ext_handle_module_event(port, mod_evt);
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      ASD_LOW("Module event %d fully handled in extension function!",
        mod_evt->type);
      return TRUE;
    }
  }

  switch (mod_evt->type) {
  case MCT_EVENT_MODULE_SET_CHROMATIX_WRP: {
    chromatix_3a_parms_wrapper *chromatix_wrapper = (chromatix_3a_parms_wrapper *)mod_evt->module_event_data;
    asd_thread_msg_t *asd_msg   =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      ASD_LOW("MSG_ASD_SET: Set chromatix for ASD!");
      STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_ASD_SET;

      asd_msg->u.asd_set_parm.type = ASD_SET_PARAM_INIT_CHROMATIX;
      asd_msg->u.asd_set_parm.u.init_param.chromatix3A = chromatix_wrapper;
      asd_msg->u.asd_set_parm.u.init_param.enable_mixedlight = ASD_ENABLE_MIXED_LIGHT;
      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  } /* case MCT_EVENT_MODULE_SET_CHROMATIX_PTR */
    break;

  case MCT_EVENT_MODULE_STATS_EXT_DATA: {
    if(asd_port_is_algo_active(port)) {
      rc = asd_port_event_stats_data(private, event);
    } else {
       ASD_LOW("ASD_Stats-[mode -%d role-%d] Not being handled",
         private->dual_cam_info.mode,
         private->dual_cam_info.cam_role);
    }
  } /* case MCT_EVENT_MODULE_STATS_DATA */
    break;
  case MCT_EVENT_MODULE_START_STOP_STATS_THREADS: {
    uint8_t *start_flag = (uint8_t*)(mod_evt->module_event_data);
    ASD_LOW("MCT_EVENT_MODULE_START_STOP_STATS_THREADS start_flag: %d",
      *start_flag);

    if (*start_flag) {
      if (asd_port_start_threads(port) == FALSE) {
        ASD_LOW("asd thread start failed");
        rc = FALSE;
      }
    } else {
      if (private->thread_data) {
        asd_thread_stop(private->thread_data);
      }
    }
  }
    break;
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    stats_update_t *stats_update =
      (stats_update_t *)mod_evt->module_event_data;

    if (!stats_update || stats_update->flag != STATS_UPDATE_AEC
      || (!asd_port_is_algo_active(port))) {
      rc = FALSE;
      break;
    }

    asd_thread_msg_t *asd_msg =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_AEC_DATA;

      asd_msg->u.aec_data.num_of_regions
         = stats_update->aec_update.numRegions;
      asd_msg->u.aec_data.pixels_per_region =
        stats_update->aec_update.pixelsPerRegion;
      asd_msg->u.aec_data.target_luma = stats_update->aec_update.target_luma;
      asd_msg->u.aec_data.comp_luma = stats_update->aec_update.comp_luma;
      asd_msg->u.aec_data.exp_index = stats_update->aec_update.exp_index;
      asd_msg->u.aec_data.exp_tbl_val = stats_update->aec_update.exp_tbl_val;
      asd_msg->u.aec_data.extreme_green_cnt =
        stats_update->aec_update.asd_extreme_green_cnt;
      asd_msg->u.aec_data.extreme_blue_cnt =
        stats_update->aec_update.asd_extreme_blue_cnt;
      asd_msg->u.aec_data.extreme_tot_regions =
        stats_update->aec_update.asd_extreme_tot_regions;
      asd_msg->u.aec_data.lux_idx = stats_update->aec_update.lux_idx;
      if (stats_update->aec_update.SY_data.is_valid &&
          stats_update->aec_update.SY_data.SY) {
        STATS_MEMCPY(asd_msg->u.aec_data.SY_data.SY, sizeof(asd_msg->u.aec_data.SY_data.SY),
          stats_update->aec_update.SY_data.SY,
          sizeof(stats_update->aec_update.SY_data.SY[0]) * MAX_YUV_STATS_NUM);
        asd_msg->u.aec_data.SY_data.is_valid =
          stats_update->aec_update.SY_data.is_valid;
      }
      /* Currently not updated by AEC */
      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  }
  break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
    stats_update_t *stats_update =
      (stats_update_t *)mod_evt->module_event_data;

    if (!stats_update || stats_update->flag != STATS_UPDATE_AWB
      || (!asd_port_is_algo_active(port))) {
      rc = FALSE;
      break;
    }
    asd_thread_msg_t *asd_msg =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_AWB_DATA;

      STATS_MEMCPY(asd_msg->u.awb_data.stat_sample_decision,
        sizeof(asd_msg->u.awb_data.stat_sample_decision),
        stats_update->awb_update.sample_decision,
        sizeof(stats_update->awb_update.sample_decision));

      asd_msg->u.awb_data.decision = stats_update->awb_update.decision;

      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  }
    break;
  case MCT_EVENT_MODULE_FACE_INFO: {
    asd_thread_msg_t *asd_msg =
      (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg) {
      STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_FACE_INFO;
      mct_face_info_t * info = (mct_face_info_t *)mod_evt->module_event_data;
      asd_data_face_info_t * face_info = &(asd_msg->u.face_data);
      size_t i;

      face_info->face_count = info->face_count;
      if (face_info->face_count > MAX_STATS_ROI_NUM) {
        ASD_HIGH("face_count %d exceed stats roi limitation, cap to max",
          face_info->face_count);
        face_info->face_count = MAX_STATS_ROI_NUM;
      }
      if (face_info->face_count > MAX_ROI) {
        ASD_HIGH("face_count %d exceed max roi limitation, cap to max",
          face_info->face_count);
        face_info->face_count = MAX_ROI;
      }

      for (i = 0; i < face_info->face_count; i++) {
        face_info->faces[i].roi = info->faces[i].roi;
        face_info->faces[i].score = info->faces[i].score;
      }

      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  }
    break;
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    asd_thread_msg_t *asd_msg = NULL;
    sensor_out_info_t       *sensor_info =
      (sensor_out_info_t *)(mod_evt->module_event_data);

    if (private->stream_type == CAM_STREAM_TYPE_PREVIEW) {
      asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));

      if (asd_msg) {
        STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
        asd_msg->type = MSG_ASD_SET;
        asd_set_parameter_t *param = &asd_msg->u.asd_set_parm;
        param->type = ASD_SET_UI_FRAME_DIM;
        param->u.init_param.camif_w = sensor_info->request_crop.last_pixel -
          sensor_info->request_crop.first_pixel + 1;
        param->u.init_param.camif_h = sensor_info->request_crop.last_line -
          sensor_info->request_crop.first_line + 1;
        rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
      }
    }

    /* send op mode to ASD algorithm */
    asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));

    if (asd_msg) {
      STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
      asd_msg->type = MSG_ASD_SET;
      asd_set_parameter_t *param = &asd_msg->u.asd_set_parm;
      param->type = ASD_SET_PARAM_OP_MODE;

      switch (private->stream_type) {
      case CAM_STREAM_TYPE_VIDEO: {
         param->u.init_param.op_mode = Q3A_OPERATION_MODE_CAMCORDER;
      }
        break;

      case CAM_STREAM_TYPE_PREVIEW: {
        param->u.init_param.op_mode = Q3A_OPERATION_MODE_PREVIEW;
      }
        break;

      case CAM_STREAM_TYPE_RAW:
      case CAM_STREAM_TYPE_SNAPSHOT: {
        param->u.init_param.op_mode = Q3A_OPERATION_MODE_SNAPSHOT;
      }
        break;
      default:
        param->u.init_param.op_mode = Q3A_OPERATION_MODE_PREVIEW;
        break;
      } /* switch (private->stream_type) */

      rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
    }
  }
    break;

  case MCT_EVENT_MODULE_MODE_CHANGE: {
    //Stream mode has changed
    private->stream_type = ((stats_mode_change_event_data*)
      (event->u.module_event.module_event_data))->stream_type;
    private->reserved_id = ((stats_mode_change_event_data*)
      (event->u.module_event.module_event_data))->reserved_id;
    ASD_LOW("MCT_EVENT_MODULE_MODE_CHANGE event. mode: %d",
      private->stream_type);
  }
    break;

  case MCT_EVENT_MODULE_REQUEST_STATS_TYPE: {
     mct_event_request_stats_type *stats_info =
      (mct_event_request_stats_type *)mod_evt->module_event_data;

     if (ISP_STREAMING_OFFLINE == stats_info->isp_streaming_type) {
       ASD_HIGH("ASD doesn't support offline processing yet. Returning.");
       break;
     } else if(!asd_port_if_stats_can_be_configured(private)) {
       ASD_HIGH("ASDDualCam-[mode -%d role-%d] is not supported in Auxillary mode",
         private->dual_cam_info.mode,
         private->dual_cam_info.cam_role);
       break;
     }

     /* Check if HDR BHIST or BHIST is supported.*/
     if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_HDR_BHIST)) {
       stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_HDR_BHIST);
       stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_HDR_BHIST);
       stats_info->stats_comp_grp_mask[STATS_GROUP_0] |= (1 << MSM_ISP_STATS_HDR_BHIST);
     } else if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_BHIST)) {
       stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_BHIST);
       stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_BHIST);
       stats_info->stats_comp_grp_mask[STATS_GROUP_0] |= (1 << MSM_ISP_STATS_BHIST);
     } else if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_IHIST)) {
       stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_IHIST);
       stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_IHIST);
       stats_info->stats_comp_grp_mask[STATS_GROUP_0] |= (1 << MSM_ISP_STATS_IHIST);
     }
   }
    break;
  default:
    break;
  } /* switch (mod_evt->type) */

  return rc;
}

/** asd_port_proc_downstream_ctrl
 *
 **/
static boolean asd_port_proc_downstream_ctrl(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;
  asd_port_private_t  *private  = (asd_port_private_t *)(port->port_private);
  mct_event_control_t *mod_ctrl = &(event->u.ctrl_event);

  /* check if there's need for extended handling. */
  if (private->func_tbl.ext_handle_control_event) {
    AEC_LOW("Handle extended control event: %d", mod_ctrl->type);
    ret = private->func_tbl.ext_handle_control_event(port, mod_ctrl);
    /* Check if this event has been completely handled. If not we'll process it further here. */
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      ASD_LOW("Control event %d fully handle by extended functionality!",
        mod_ctrl->type);
      return rc;
    }
  }

  switch (mod_ctrl->type) {

  case MCT_EVENT_CONTROL_SET_PARM: {
    stats_set_params_type *stat_parm =
      (stats_set_params_type *)mod_ctrl->control_event_data;

    if (stat_parm->param_type == STATS_SET_ASD_PARAM) {
      asd_thread_msg_t *asd_msg =
        (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
      if (asd_msg != NULL ) {
        STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
        asd_msg->type = MSG_ASD_SET;
        asd_msg->u.asd_set_parm = stat_parm->u.asd_param;

        rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
      }
    }
    /* If it's common params shared by many modules */
    else if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
      stats_common_set_parameter_t *common_param =
        &(stat_parm->u.common_param);
      asd_thread_msg_t *asd_msg = NULL;
      boolean asd_enable = FALSE;
      asd_set_parameter_t asd_param;
      if (common_param->type == COMMON_SET_PARAM_BESTSHOT) {
        private->scene_mode = common_param->u.bestshot_mode;
        asd_enable = (common_param->u.bestshot_mode == CAM_SCENE_MODE_AUTO ||
                      common_param->u.bestshot_mode == CAM_SCENE_MODE_FACE_PRIORITY) ? TRUE : FALSE;
        ASD_LOW("MSG_ASD_SET: Enable/disable ASD? %d, HAL(best_shot) %d",
          asd_enable, common_param->u.bestshot_mode);
        asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
        if (asd_msg != NULL) {
          STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
          asd_msg->type = MSG_ASD_SET;
          asd_param.type = ASD_SET_ENABLE;
          asd_param.u.asd_enable = asd_enable;
          asd_msg->u.asd_set_parm = asd_param;
          rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
        }
      } else if (common_param->type == COMMON_SET_PARAM_STATS_DEBUG_MASK) {
        asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
        if (asd_msg != NULL) {
          STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
          asd_msg->type = MSG_ASD_SET;
          asd_msg->u.asd_set_parm.type = ASD_SET_STATS_DEBUG_MASK;
          rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
        }
      } else if (common_param->type == COMMON_SET_PARAM_STREAM_ON_OFF) {
        ASD_LOW("COMMON_SET_PARAM_STREAM_ON_OFF %d", common_param->u.stream_on);
        private->thread_data->no_stats_mode = !common_param->u.stream_on;

        // stream off, need to flush existing stats
        // send a sync msg here to flush the stats & other msg
        if (!common_param->u.stream_on) {
          asd_port_unlink_from_peer(port, event);
          asd_thread_msg_t asd_msg;
          STATS_MEMSET(&asd_msg, 0, sizeof(asd_thread_msg_t));
          asd_msg.type = MSG_ASD_STATS_MODE;
          asd_msg.sync_flag = TRUE;
          asd_thread_en_q_msg(private->thread_data, &asd_msg);
          ASD_LOW("COMMON_SET_PARAM_STREAM_ON_OFF end");
        }
      }
    } else if (stat_parm->param_type == STATS_SET_Q3A_PARAM) {
      if (stat_parm->u.q3a_param.type == Q3A_ALL_SET_PARAM &&
          stat_parm->u.q3a_param.u.q3a_all_param.type ==
          Q3A_ALL_SET_EZTUNE_RUNNIG) {

        asd_thread_msg_t *asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
        if (asd_msg != NULL) {
          STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
          asd_msg->type = MSG_ASD_SET;
          asd_msg->u.asd_set_parm.type = ASD_SET_EZTUNE_RUNNING;
          asd_msg->u.asd_set_parm.u.ez_running =
            stat_parm->u.q3a_param.u.q3a_all_param.u.ez_runnig;
          rc = asd_thread_en_q_msg(private->thread_data, asd_msg);
        }
      }
    }
  } /* MCT_EVENT_CONTROL_SET_PARM */
    break;
  case MCT_EVENT_CONTROL_SOF: {
    mct_bus_msg_isp_sof_t *sof_event;
      sof_event =(mct_bus_msg_isp_sof_t *)(mod_ctrl->control_event_data);
    asd_thread_msg_t *asd_msg = (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
    if (asd_msg == NULL)
      return FALSE;
    STATS_MEMSET(asd_msg, 0, sizeof(asd_thread_msg_t));
    asd_msg->type                =  MSG_ASD_SET;
    asd_msg->u.asd_set_parm.type =  ASD_SET_SOF;
    asd_msg->u.asd_set_parm.u.current_sof_id = sof_event->frame_id;
    rc = asd_thread_en_q_msg(private->thread_data, asd_msg);

    /* Update scene mode to HAL3 */
    asd_send_bus_message(port, MCT_BUS_MSG_SCENE_MODE,
      (void*)&private->scene_mode, sizeof(int32_t), sof_event->frame_id);
    ASD_LOW("BSUPD SOF scene_mode:%d", &private->scene_mode);

    /* Send the ASD debug data from here in SoF context */
    ASD_LOW("Send exif info update with debug data");
    asd_port_send_exif_debug_data(port);
  }
  break;

  case MCT_EVENT_CONTROL_MASTER_INFO: {
     asd_port_handle_role_switch(port,event);
   }
    break;

  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
    asd_port_link_to_peer(port,event);
  }
    break;

  case MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION: {
    asd_port_unlink_from_peer(port,event);
  }
    break;

  case MCT_EVENT_CONTROL_HW_WAKEUP:
  case MCT_EVENT_CONTROL_HW_SLEEP:{
    asd_port_update_LPM(port,
      (cam_dual_camera_perf_control_t*)event->u.ctrl_event.control_event_data);
  }
  break;

  case MCT_EVENT_CONTROL_STREAMOFF: {
  }
  break;

  default:
    break;
  }

  return rc;
}

/** asd_port_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because ASD module works no more than a sink module,
 *  hence its upstream event will need a little bit processing.
 *
 *  Return TRUE for successful event processing.
 **/
static boolean asd_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  asd_port_private_t *private;

  /* sanity check */
  if (!port || !event)
    return FALSE;

  private = (asd_port_private_t *)(port->port_private);
  if (!private)
    return FALSE;

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000))
    return FALSE;

  ASD_LOW("ASD_EVENT: %s Dir %d", event->type == MCT_EVENT_CONTROL_CMD ?
            stats_port_get_mct_event_ctrl_string(event->u.ctrl_event.type):
           (event->type == MCT_EVENT_MODULE_EVENT ?
            stats_port_get_mct_event_module_string(event->u.module_event.type):
            "INVALID EVENT"), MCT_EVENT_DIRECTION(event))
  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {

    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      rc = asd_port_proc_downstream_event(port, event);
    } /* case MCT_EVENT_MODULE_EVENT */
      break;

    case MCT_EVENT_CONTROL_CMD: {
      rc = asd_port_proc_downstream_ctrl(port,event);
    }
      break;

    default:
      break;
    }
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  /* ASD port's peer should be Stats module's port */
  case MCT_EVENT_UPSTREAM: {
    mct_port_t *peer = MCT_PORT_PEER(port);
    MCT_PORT_EVENT_FUNC(peer)(peer, event);
  }
    break;

  default:
    rc = FALSE;
    break;
  }

  return rc;
}

/** asd_port_set_caps
 *    @port: port object which the caps to be set;
 *    @caps: this port's capability.
 *
 *  Return TRUE if it is valid soruce port.
 *
 *  Function overwrites a ports capability.
 **/
static boolean asd_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "asd_sink"))
    return FALSE;

  port->caps = *caps;
  return TRUE;
}

/** asd_port_check_caps_reserve
 *    @port: this interface module's port;
 *    @peer_caps: the capability of peer port which wants to match
 *                interterface port;
 *    @stream_info:
 *
 *  Stats modules are pure s/w software modules, and every port can
 *  support one identity. If the identity is different, support
 *  can be provided via create a new port. Regardless source or
 *  sink port, once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return
 *    failure
 *  - If the requested stream belongs to a different session, the port
 *  can not be used.
 **/
static boolean asd_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *info)
{
  boolean            rc = FALSE;
  mct_port_caps_t    *port_caps;
  asd_port_private_t *private;
  mct_stream_info_t *stream_info = (mct_stream_info_t *) info;

  if (!port || !caps || !stream_info ||
      strcmp(MCT_OBJECT_NAME(port), "asd_sink")) {
    return rc;
  }

  MCT_OBJECT_LOCK(port);

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (asd_port_private_t *)port->port_private;
  switch (private->state) {
  case ASD_PORT_STATE_LINKED: {
    if ((private->reserved_id & 0xFFFF0000) ==
        (stream_info->identity & 0xFFFF0000))
      rc = TRUE;
  }
    break;

  case ASD_PORT_STATE_CREATED:
  case ASD_PORT_STATE_UNRESERVED: {

    private->reserved_id = stream_info->identity;
    private->stream_type = stream_info->stream_type;
    private->state       = ASD_PORT_STATE_RESERVED;
    private->stream_info = *stream_info;
    rc = TRUE;
  }
    break;

  case ASD_PORT_STATE_RESERVED:
    if ((private->reserved_id & 0xFFFF0000) ==
        (stream_info->identity & 0xFFFF0000))
      rc = TRUE;
    break;

  default:
    rc = FALSE;
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** asd_port_check_caps_unreserve
 *    @port: this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *    Return FALSE if the identity is not existing.
 *
 *  This function frees the identity from port's children list.
 **/
static boolean asd_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  asd_port_private_t *private;
  boolean rc = FALSE;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "asd_sink"))
    return FALSE;

  private = (asd_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  MCT_OBJECT_LOCK(port);
  if ((private->state == ASD_PORT_STATE_UNLINKED   ||
       private->state == ASD_PORT_STATE_LINKED   ||
       private->state == ASD_PORT_STATE_RESERVED) &&
      ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = ASD_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
    rc                   = TRUE;
  } else {
    rc = FALSE;
  }
  MCT_OBJECT_UNLOCK(port);

  return rc;
}

/** asd_port_ext_link
 *    @identity:  Identity of session/stream
 *    @port: SINK of ASD ports
 *    @peer: For ASD sink- peer is STATS sink port
 *
 *  Set ASD port's external peer port, which is STATS module's
 *  sink port.
 *
 *  Return TRUE on success.
 **/
static boolean asd_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean rc = FALSE, thread_init = FALSE;
  asd_port_private_t  *private;
  mct_event_t         event;

  if (strcmp(MCT_OBJECT_NAME(port), "asd_sink")){
    return FALSE;
  }

  private = (asd_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case ASD_PORT_STATE_RESERVED:
  case ASD_PORT_STATE_UNLINKED:
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }
  /* Fall through */
  case ASD_PORT_STATE_CREATED:
    thread_init = TRUE;
    rc = TRUE;
    break;

  case ASD_PORT_STATE_LINKED:
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      rc = TRUE;
      thread_init = FALSE;
    }
    break;

  default:
    break;
  }

  if (rc == TRUE) {
    if (thread_init == TRUE) {
      if (NULL == private->thread_data) {
        rc = FALSE;
        goto asd_ext_link_done;
      }
    }

    private->state = ASD_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

asd_ext_link_done:
  MCT_OBJECT_UNLOCK(port);
  mct_port_add_child(identity, port);
  return rc;
}

/** asd_port_ext_unlink
 *
 *  @identity: Identity of session/stream
 *  @port: asd module's sink port
 *  @peer: peer of stats sink port
 *
 * This funtion unlink the peer ports of stats sink, src ports
 * and its peer submodule's port
 *
 **/
static void asd_port_ext_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  asd_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer)
    return;

  private = (asd_port_private_t *)port->port_private;
  if (!private)
    return;

  MCT_OBJECT_LOCK(port);
  if (private->state == ASD_PORT_STATE_LINKED &&
    (private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {

    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      ASD_LOW("asd_data=%p", private->thread_data);
      private->state = ASD_PORT_STATE_UNLINKED;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  mct_port_remove_child(identity, port);

  return;
}

/** asd_dlsym
 *
 *    @lib_handler: Handler to library
 *    @fn_ptr: Function to initialize
 *    @fn_name: Name of symbol to find in library
 *
 * Return: TRUE on success
 **/
static boolean asd_dlsym(void *lib_handler, void **fn_ptr,
  const char *fn_name)
{
  char *error = NULL;

  if (NULL == lib_handler || NULL == fn_ptr) {
    ASD_ERR("Error Loading %s", fn_name);
    return FALSE;
  }

  *(void **)(fn_ptr) = dlsym(lib_handler, fn_name);
  if (!fn_ptr) {
    error = (char *)dlerror();
    ASD_ERR("Error: %s", error);
    return FALSE;
  }

  ASD_HIGH("Loaded %s @ %p", fn_name, fn_ptr);
  return TRUE;
}

/** asd_port_clear_algo_ops
 *
 *    @asd_ops: Function pointers to clear
 *
 *  Reset algo interface
 *
 * Return: void
 **/
static void asd_port_clear_algo_ops(asd_ops_t *asd_ops)
{
  asd_ops->set_parameters = NULL;
  asd_ops->get_parameters = NULL;
  asd_ops->process = NULL;
  asd_ops->init = NULL;
  asd_ops->deinit = NULL;
  return;
}

/** asd_port_unload_interface
 *
 *    @asd_ops: Function pointers to clear
 *    @asd_handle: Library handle previously obtain with dlopen()
 *
 *  Close lib handle and reset algo interface
 *
 * Return: void
 **/
void asd_port_unload_interface(asd_ops_t *asd_ops, void* asd_handle)
{
  if (asd_handle) {
    dlclose(asd_handle);
  }
  asd_port_clear_algo_ops(asd_ops);
  return;
}

/** aasd_port_load_interface
 *
 *    @asd_ops: Function pointers to clear
 *
 *  Load default ASD algo library.
 *  Init algo interface with the ASD library.
 *
 * Return: Handle to library on success, NULL if failure.
 **/
void * asd_port_load_interface(asd_ops_t *asd_ops)
{
  void *lib_handle = NULL;

  if (!asd_ops) {
    return NULL;
  }

  do {
    dlerror(); /* Clear previous errors */
    lib_handle = dlopen("libmmcamera2_stats_algorithm.so", RTLD_NOW);
    if (!lib_handle) {
      ASD_ERR("dlerror: %s", dlerror());
      return NULL;
    }

    if (!asd_dlsym(lib_handle, (void**)&asd_ops->set_parameters,
      "asd_set_parameters")) {
      break;
    }
    if (!asd_dlsym(lib_handle, (void**)&asd_ops->get_parameters,
      "asd_get_parameters")) {
      break;
    }
    if (!asd_dlsym(lib_handle, (void**)&asd_ops->process,
      "asd_process")) {
      break;
    }
    if (!asd_dlsym(lib_handle, (void**)&asd_ops->init,
      "asd_init")) {
      break;
    }
    if (!asd_dlsym(lib_handle, (void**)&asd_ops->deinit,
      "asd_destroy")) {
      break;
    }

    return lib_handle;
  } while (0);

  /* Handling error */
  if (lib_handle) {
    dlclose(lib_handle);
  }
  asd_port_clear_algo_ops(asd_ops);

  return NULL;
}

/** asd_port_deinit_interface
 *
 *    @private: ASD port private structure
 *
 *  Free resources allocated using asd_port_init_interface
 *
 * Return: void
 **/
void asd_port_deinit_interface(asd_port_private_t *private)
{
  if (FALSE == private->use_extension) {
    /* Use default algo only */
    asd_port_unload_interface(&(private->asd_object.asd_ops),
      private->asd_iface_handle);
  } else {
    asd_port_ext_unload_interface(&(private->asd_object.asd_ops),
      private->asd_iface_handle);
  }

  return;
}

/** asd_port_init_interface
 *    @private: port private data
 *
 *  Load ASD algo interface and init port extension functions if required.
 *
 *  Return TRUE on success.
 **/
boolean asd_port_init_interface(asd_port_private_t *private)
{

  asd_port_reset_ext_func_tbl(private);

  do {
    private->use_extension = asd_port_ext_is_extension_required(private);
    if (FALSE == private->use_extension) {
      /* Use default algo only */
      private->asd_iface_handle = asd_port_load_interface(&(private->asd_object.asd_ops));
      if (!private->asd_iface_handle) {
        ASD_ERR("Fail to load ASD interface");
        break;
      }
    } else {
      /* Use algo base on OEM preference via extension */
      private->asd_iface_handle =
        asd_port_ext_load_interface(&(private->asd_object));
      if (!private->asd_iface_handle) {
       ASD_ERR("Fail to load OEM ASD interface");
       break;
      }
      asd_port_ext_update_port_func_table(private);
    }

    if (!(asd_port_is_asd_algo_iface_valid(&(private->asd_object.asd_ops)))) {
      ASD_ERR("Invalid ASD iface fail to init");
      break;
    }
    ASD_HIGH("ASD iface handle: %p", private->asd_iface_handle);
    return TRUE;
  }while (0);

  /* Handle errors */
  if (FALSE == private->use_extension) {
    asd_port_unload_interface(&(private->asd_object.asd_ops),
      private->asd_iface_handle);
  } else {
    asd_port_ext_unload_interface(&(private->asd_object.asd_ops),
      private->asd_iface_handle);
  }

  return FALSE;
}

/** asd_port_deinit
 *    @port: asd sink port
 *
 *  de-initialize one ASD sink port
 *
 *  Return nothing
 **/
void asd_port_deinit(mct_port_t *port)
{
  asd_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "asd_sink"))
    return;

  private = port->port_private;
  if (!private)
    return;

  asd_thread_deinit(private->thread_data);

  if (private->func_tbl.ext_deinit) {
    private->func_tbl.ext_deinit(port);
  }
  asd_port_reset_ext_func_tbl(private);

  if (private->asd_object.asd_ops.deinit) {
    private->asd_object.asd_ops.deinit(private->asd_object.asd);
    private->asd_object.asd = NULL;
  }

  asd_port_deinit_interface(private);

  ASD_HIGH("free port_private: %p", port->port_private);
  free(port->port_private);
  port->port_private = NULL;

  return;
}

/** asd_port_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities. Stats
 *  module should have ONLY sink port.
 *
 *  Return TRUE on success.
 **/
boolean asd_port_init(mct_port_t *port, unsigned int identity)
{
  boolean rc = FALSE;
  mct_port_caps_t    caps;
  asd_port_private_t *private;
  mct_list_t         *list;

  private = calloc(1, sizeof(asd_port_private_t));
  if (private == NULL) {
    return rc;
  }


  /* initialize ASD object */
  rc = asd_port_init_interface(private);
  if (FALSE == rc) {
    ASD_ERR("Fail to load ASD algo interface");
    free(private);
    return rc;
  }

  ASD_INITIALIZE_LOCK(&private->asd_object);
  private->asd_object.asd =
    private->asd_object.asd_ops.init(private->asd_iface_handle);
  if (NULL == private->asd_object.asd) {
    ASD_DESTROY_LOCK(&private->asd_object);
    ASD_ERR("error free private: %p", private);
    free(private);
    return FALSE;
  }

  private->reserved_id = identity;
  private->state       = ASD_PORT_STATE_CREATED;
  asd_port_reset_dual_cam_info(private);

  port->port_private  = private;
  port->direction     = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag   = (MCT_PORT_CAP_STATS_ASD | MCT_PORT_CAP_STATS_HIST |
                         MCT_PORT_CAP_STATS_BHIST);

  private->asd_enable = FALSE;

  if (private->func_tbl.ext_init) {
    private->func_tbl.ext_init(port, identity);
  }

  rc = asd_port_init_threads(port);
  if (NULL == private->thread_data) {
    ASD_ERR("aecawb init failed");
  }

  mct_port_set_event_func(port, asd_port_event);
  mct_port_set_intra_event_func(port, asd_port_intra_event);
  mct_port_set_set_caps_func(port, asd_port_set_caps);
  mct_port_set_ext_link_func(port, asd_port_ext_link);
  mct_port_set_unlink_func(port, asd_port_ext_unlink);
  mct_port_set_check_caps_reserve_func(port, asd_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, asd_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }

  return rc;
}

/** asd_port_find_identity
 *
 **/
boolean asd_port_find_identity(mct_port_t *port, unsigned int identity)
{
  asd_port_private_t *private;

  if (!port)
    return FALSE;

  if(strcmp(MCT_OBJECT_NAME(port), "asd_sink"))
      return FALSE;

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000) ?
      TRUE : FALSE);
  }

  return FALSE;
}

/*Start of ASD Dual Camera Port handling*/

/* Summary of code changes:
  ASD can be in stand alone mode or in follow mode
  While in Stand alone, it may have obligation to send information
  to peer if its linked  and running Qualcomm code and sync is enabled
  While in follow mode, Algorithm wont be running, however
  ASD update and  status from peer needs to be stored and later
  used during Role Switch
  Low Power Modes may turn off sync
  Roles can be changed
*/

/** asd_port_link_to_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Links to peer ASD Port (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_link_to_peer(mct_port_t *port,
                                         mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  asd_port_private_t  *private = (asd_port_private_t *)(port->port_private);

  link_param = (cam_sync_related_sensors_event_info_t *)
    (event->u.ctrl_event.control_event_data);
  peer_identity = link_param->related_sensor_session_id;

  rc = stats_util_get_peer_port(event, peer_identity,this_port,
    &peer_port);

  if (rc == FALSE) {
    ASD_ERR("ASDDualCam FAIL to Get Peer Port");
    return rc;
  }
  private->dual_cam_info.cam_role = link_param->cam_role;
  private->dual_cam_info.mode = link_param->mode;
  private->dual_cam_info.sync_3a_mode = link_param->sync_3a_config.sync_mode_stats;
  private->dual_cam_info.is_LPM_on = FALSE;
  private->dual_cam_info.intra_peer_id = peer_identity;
  asd_port_handle_role_switch(port,NULL);
  MCT_PORT_INTRALINKFUNC(this_port)(peer_identity, this_port, peer_port);
  ASD_HIGH("ASDDualCam [ role-%d mode-%d 3aSync-%d AuxSync-%d AuxUpdate-%d] Linking to session %x Success",
    private->dual_cam_info.cam_role,
    private->dual_cam_info.mode,
    private->dual_cam_info.sync_3a_mode,
    private->dual_cam_info.is_aux_sync_enabled,
    private->dual_cam_info.is_aux_update_enabled,
    peer_identity);
  return rc;
}

/** asd_port_unlink_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer ASD Port (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_unlink_from_peer(mct_port_t *port,
                                               mct_event_t *event)
{
  asd_port_private_t  *private = (asd_port_private_t *)(port->port_private);
  if(private->dual_cam_info.intra_peer_id){
    ASD_HIGH("ASDDualCam-[mode -%d role-%d] Unlink Started",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    asd_port_local_unlink(port,event);
    asd_port_remote_unlink(port);
    ASD_HIGH("ASDDualCam-[mode -%d role-%d] Unlink complete",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    asd_port_reset_dual_cam_info(private);
  }
  return TRUE;
}

/** asd_port_local_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer ASD Port locally (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_local_unlink(mct_port_t *port,
                                        mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  asd_port_private_t                    *private = (asd_port_private_t *)(port->port_private);
  uint32_t                               peer_identity = private->dual_cam_info.intra_peer_id;

  if (peer_identity) {
    rc = stats_util_get_peer_port(event, peer_identity, this_port, &peer_port);
    if (rc == TRUE) {
      MCT_PORT_INTRAUNLINKFUNC(peer_port);
    }
    ASD_LOW("ASDDualCam-[mode -%d role-%d] Unlinking from peer session",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
  } else {
    rc = TRUE;
  }

  return rc;
}


/** asd_port_remote_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Asks Peer to do unlink (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_remote_unlink(mct_port_t *port)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  asd_port_private_t  *private = (asd_port_private_t *)(port->port_private);
  /* Forward the ASD update info to the slave session */
  ASD_LOW("ASDDualCam-[mode -%d role-%d] Remote Unlink issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_UNLINK,
                                       NULL);
  return rc;
}

/** asd_port_remote_LPM:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Informs peer that we have gone to LPM mode
 *
 * Return boolean
 **/
boolean asd_port_remote_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  asd_port_private_t  *private = (asd_port_private_t *)(port->port_private);
  /* Forward the LPM to the Master session */
  ASD_HIGH("ASDDualCam-[mode -%d role-%d] Remote LPM issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT,
                                       perf_ctrl);
  return rc;
}

/** asd_port_LPM_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * LPM from peer ASD Port (Master/Slave)
 *
 * Return boolean
 **/
boolean asd_port_LPM_from_peer(mct_port_t *port,
                                     cam_dual_camera_perf_control_t* perf_ctrl)
{
  asd_port_private_t  *private = (asd_port_private_t *)(port->port_private);
  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    ASD_HIGH("ASDDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    asd_port_update_aux_sync_and_update(private);
  }
  return TRUE;
}


/** asd_port_handle_role_switch:
 *    @port: MCT port data
 *
 * Handles Role switch from Master to Slave and Vice versa (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_handle_role_switch(mct_port_t *port,
                                                  mct_event_t *event)
{
  mct_port_t                            *this_port = port;
  asd_port_private_t  *private = (asd_port_private_t *)(port->port_private);
  if (event) {
    cam_dual_camera_master_info_t *dual_master_info = (cam_dual_camera_master_info_t *)
      (event->u.ctrl_event.control_event_data);
    if (dual_master_info){
      private->dual_cam_info.mode = dual_master_info->mode;
    }
  }
  asd_port_update_aux_sync_and_update(private);
  asd_port_set_role_switch(private);
  ASD_HIGH("ASDDualCam-[mode -%d role-%d] Role Switched",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);

  return TRUE;
}

/** asd_port_set_role_switch:
 *    @port: ASD Private  port data
 *
 * Set Role Switch information to Algorithm
 *
 * Return boolean
 **/
static boolean asd_port_set_role_switch(asd_port_private_t  *private)
{
  asd_thread_msg_t *asd_msg   =
    (asd_thread_msg_t *)malloc(sizeof(asd_thread_msg_t));
  if (asd_msg == NULL) {
    return FALSE;
  }
  memset(asd_msg, 0, sizeof(asd_thread_msg_t));
  asd_msg->type = MSG_ASD_SET;
  asd_msg->u.asd_set_parm.type = ASD_SET_PARAM_ROLE_SWITCH;
  memset(&asd_msg->u.asd_set_parm.u.role_switch, 0, sizeof(asd_role_switch_params_t));
  if(private->dual_cam_info.mode == CAM_MODE_PRIMARY &&
    private->dual_cam_info.peer_stats_update.validUpdate) {
    asd_msg->u.asd_set_parm.u.role_switch.master = TRUE;
    memcpy(&asd_msg->u.asd_set_parm.u.role_switch.peer_data,
      &private->dual_cam_info.peer_stats_update.stats_update.asd_update,
      sizeof(asd_update_t));
  } else {
    asd_msg->u.asd_set_parm.u.role_switch.peer_data.scene = S_MAX;
  }
  ASD_HIGH("ASDDualCam-[mode -%d role-%d] Role Switch Master=%d Informed",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    asd_msg->u.asd_set_parm.u.role_switch.master);
 return asd_thread_en_q_msg(private->thread_data, asd_msg);
}

/** asd_port_reset_dual_cam_info:
 *    @port: ASD Private  port data
 *
 * Resets Dual Cam Info (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_reset_dual_cam_info(asd_port_private_t  *port)
{
  ASD_HIGH("ASDDualCam-[mode -%d role-%d] Reset",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role);
  port->dual_cam_info.mode = CAM_MODE_PRIMARY;
  port->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_NONE;
  port->dual_cam_info.is_LPM_on = FALSE;
  port->dual_cam_info.is_aux_sync_enabled = FALSE;
  port->dual_cam_info.is_aux_update_enabled = FALSE;
  port->dual_cam_info.is_algo_active = TRUE;
  port->dual_cam_info.intra_peer_id = 0;
  port->dual_cam_info.peer_stats_update.validUpdate = FALSE;
  return TRUE;
}

/** asd_port_if_stats_can_be_configured:
 *    @port: ASD Private  port data
 *
 * Checks if Stats can be configured (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_if_stats_can_be_configured(asd_port_private_t  *port)
{
  boolean config = TRUE;

  /*We dont want to configure stats if we are in Follow and Mono mode*/
  if ((port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW) &&
      (port->dual_cam_info.mode == CAM_MODE_SECONDARY) &&
      (port->dual_cam_info.cam_role == CAM_ROLE_MONO)) {
      ASD_HIGH("ASDDualCam-[mode -%d role-%d] Stats cannot be configured",
        port->dual_cam_info.mode,
        port->dual_cam_info.cam_role);
      config = FALSE;
  }
  return config;
}

/** asd_port_update_aux_sync_and_update:
 *    @port: ASD Private  port data
 *
 * Updates Aux sync and update conditions (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_update_aux_sync_and_update(asd_port_private_t  *port)
{
  boolean is_aux_sync_enabled = FALSE;
  boolean is_aux_update_enabled = FALSE;
  boolean is_algo_active = TRUE;

  if(port->dual_cam_info.is_LPM_on) {
    port->dual_cam_info.is_aux_sync_enabled = FALSE;
    port->dual_cam_info.is_aux_update_enabled = FALSE;
    ASD_HIGH("ASDDualCam-[mode -%d role-%d] Aux Sync Enabled=%d Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled,
      is_aux_update_enabled);
    return TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)
    && (port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
    && (port->dual_cam_info.mode == CAM_MODE_PRIMARY)
    && (port->dual_cam_info.intra_peer_id != 0)) {
      is_aux_sync_enabled = TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)
      &&(port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
      && (port->dual_cam_info.mode == CAM_MODE_SECONDARY)
      && (port->dual_cam_info.intra_peer_id != 0)) {
     is_aux_update_enabled = TRUE;
  }

  if((port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
      && (port->dual_cam_info.mode == CAM_MODE_SECONDARY)) {
      is_algo_active = FALSE;
    }

  if(is_algo_active != port->dual_cam_info.is_algo_active) {
    ASD_HIGH("ASDDualCam-[mode -%d role-%d] is_algo_active=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_algo_active);
    port->dual_cam_info.is_algo_active = is_algo_active;
  }


  if(is_aux_sync_enabled != port->dual_cam_info.is_aux_sync_enabled) {
    ASD_HIGH("ASDDualCam-[mode -%d role-%d] Aux Sync Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled);
    port->dual_cam_info.is_aux_sync_enabled = is_aux_sync_enabled;
  }

  if(is_aux_update_enabled != port->dual_cam_info.is_aux_update_enabled) {
    ASD_HIGH("ASDDualCam-[mode -%d role-%d] Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_update_enabled);
    port->dual_cam_info.is_aux_update_enabled = is_aux_update_enabled;
  }
  return TRUE;
}

/** asd_port_update_LPM:
 *    @port: ASD Private  port data
 *    @perf_ctrl: LPM mode enabled or not
 *
 * Update LPM mode  (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_update_LPM(mct_port_t *port, cam_dual_camera_perf_control_t* perf_ctrl)
{
  asd_port_private_t  *private = (asd_port_private_t *)(port->port_private);
  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    ASD_HIGH("ASDDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    asd_port_update_aux_sync_and_update(private);
  }
  asd_port_remote_LPM(port,perf_ctrl);
  return TRUE;
}

/** asd_port_is_algo_active:
 *    @port:  Private  port data
 *
 * Is Algorithm Active  (Master/Slave)
 *
 * Return boolean
 **/
static boolean asd_port_is_algo_active(mct_port_t *port)
{
  asd_port_private_t  *private = (asd_port_private_t *)(port->port_private);
  return private->dual_cam_info.is_algo_active;
}

/** asd_port_forward_bus_message_if_linked:
 *    @port: MCT  port data
 &    @asd_scene_msg: Scene info
 *
 * Forward the Bus Message to the slave session if: dual camera is in use, the
 * cameras are linked, and this method is called from the master session (Master)
 *
 * Return boolean
 **/
static boolean asd_port_forward_bus_message_if_linked(mct_port_t *mct_port , mct_bus_msg_asd_decision_t* asd_scene_msg)
{
  asd_port_private_t* asd_port = (asd_port_private_t *)(mct_port->port_private);
  boolean result = true;

  /* Only forward the ASD BUS Message if Aux Sync is Enabled*/
  if (asd_port->dual_cam_info.is_aux_sync_enabled)
  {
    ASD_LOW("ASDDualCam-[mode -%d role-%d] Bus message fwded",
      asd_port->dual_cam_info.mode,
      asd_port->dual_cam_info.cam_role);
    /* Forward the ASD BUS Message update info to the slave session */
    result = stats_util_post_intramode_event(mct_port,
                                             asd_port->dual_cam_info.intra_peer_id,
                                             MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT,
                                             (void*)asd_scene_msg);
    if (!result)
    {
      ASD_MSG_ERROR("ASDDualCam-[mode -%d role-%d] Error! failed to forward the ASD BUS Message event to the slave (id=%d)",
        asd_port->dual_cam_info.mode,
        asd_port->dual_cam_info.cam_role,
        asd_port->dual_cam_info.intra_peer_id );
    }
  }
  return result;
}

/** asd_port_forward_update_event_if_linked:
  *    @mct_port:   MCT port object
  *    @stats_update: ASD Status update
  *
  * Forward the ASD update event to the slave session if: dual camera is in use, the
  * cameras are linked, and this method is called from the master session (Master)
  *
  * Return: boolean value indicating success or failure
  */
static boolean asd_port_forward_update_event_if_linked( mct_port_t* mct_port,stats_update_t* stats_update)
{
  asd_port_private_t* asd_port = (asd_port_private_t *)(mct_port->port_private);
  boolean result = true;

  /* Only forward the ASD update if Aux Sync is Enabled*/
  if (asd_port->dual_cam_info.is_aux_sync_enabled)
  {
    ASD_LOW("ASDDualCam-[mode -%d role-%d] Update message fwded",
    asd_port->dual_cam_info.mode,
    asd_port->dual_cam_info.cam_role);
    /* Forward the ASD update info to the slave session */
    result = stats_util_post_intramode_event(mct_port,
                                             asd_port->dual_cam_info.intra_peer_id,
                                             MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT,
                                             (void*)stats_update);
    if (!result)
    {
      ASD_MSG_ERROR("ASDDualCam-[mode -%d role-%d] Error! failed to forward the ASD UPdate Message event to the slave (id=%d)",
        asd_port->dual_cam_info.mode,
        asd_port->dual_cam_info.cam_role,
        asd_port->dual_cam_info.intra_peer_id);
    }
  }
  return result;

}

/** asd_port_intra_event:
 *    @port:  MCT port
 *    @event: MCT module
 *
 * Handles the intra-module events sent between ASD master and slave sessions
 *
 * Return TRUE if the event is processed successfully.
 **/
static boolean asd_port_intra_event(mct_port_t *port, mct_event_t *event)
{
  asd_port_private_t *private;

  /* sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (asd_port_private_t *)(port->port_private);
  if (!private) {
    return FALSE;
  }

  ASD_LOW("ASDDualCam-[mode -%d role-%d] Received ASD intra-module peer event",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000)) {
    return FALSE;
  }

  /* check if there's need for extended handling. */
  if (private->func_tbl.ext_handle_intra_event) {
    stats_ext_return_type ret;
    ASD_LOW("Handle extended intra port event!");
    ret = private->func_tbl.ext_handle_intra_event(port, event);
    /* Check if this event has been completely handled. If not we'll process it further here. */
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      ASD_LOW("Intra port event %d handled by extended functionality!",
        event->u.module_event.type);
      return TRUE;
    }
  }

  switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT:
      ASD_LOW("ASDDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      asd_port_store_peer_update_event(port,(stats_update_t*)event->u.module_event.module_event_data);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT:
      ASD_LOW("ASDDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      asd_port_store_peer_bus_message(port,(mct_bus_msg_asd_decision_t*)event->u.module_event.module_event_data);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_UNLINK:
      ASD_LOW("ASDDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UNLINK",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      asd_port_local_unlink(port,event);
      asd_port_reset_dual_cam_info(private);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT:
      ASD_LOW("ASDDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      asd_port_LPM_from_peer(port,
        (cam_dual_camera_perf_control_t*)event->u.ctrl_event.control_event_data);
      break;
    default:
      ASD_ERR("ASDDualCam-[mode -%d role-%d] Error! Received unknown intra-module event type: %d",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role,
        event->u.module_event.type);
      break;
  }

  return TRUE;
}

/** asd_port_store_peer_bus_message:
 *    @port:  MCT port
 *    @asd_scene_msg: Master Status message to be saved
 *
 * Return TRUE if the event is processed successfully.
 **/
static boolean asd_port_store_peer_bus_message(mct_port_t *mct_port , mct_bus_msg_asd_decision_t* asd_scene_msg)
{
  asd_port_private_t *private = (asd_port_private_t *)(mct_port->port_private);
  ASD_LOW("ASDDualCam-[mode -%d role-%d] Stored Peer Bus message",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  private->dual_cam_info.peer_stats_update.validUpdate = TRUE;
  Q3A_MEMCPY(&private->dual_cam_info.peer_stats_update.asd_scene_msg,asd_scene_msg,sizeof(mct_bus_msg_asd_decision_t));
  return TRUE;
}

/** asd_port_store_peer_update_event:
 *    @port:              MCT port
 *    @stats_update: Stats result from Master
 *
 * Return TRUE if the event is processed successfully.
 **/
static boolean asd_port_store_peer_update_event( mct_port_t* mct_port, stats_update_t* stats_update)
{
  asd_port_private_t *private = (asd_port_private_t *)(mct_port->port_private);
  ASD_LOW("ASDDualCam-[mode -%d role-%d] Stored PeerUpdate message",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  private->dual_cam_info.peer_stats_update.validUpdate = TRUE;
  Q3A_MEMCPY(&private->dual_cam_info.peer_stats_update.stats_update,stats_update,sizeof(stats_update_t));
  return TRUE;
}
/*End of ASD Dual Camera Port handling*/
