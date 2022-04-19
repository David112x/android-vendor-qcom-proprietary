/* isp_algo.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* kernel headers */
#include <media/msmb_isp.h>

/* mctl headers */
#include "mct_event_stats.h"
#include "media_controller.h"
#include "mct_list.h"

/* isp headers */
#include "isp_log.h"
#include "isp_algo.h"
#include "isp_trigger_thread.h"
#include "la40_algo.h"
#include "ltm44_algo.h"
#include "gtm46_algo.h"
#include "tintless40_algo.h"

/* registered module algorithms */
static isp_algo_t* isp44_algo_list[] = {
  &algo_la40,
  &algo_ltm44,
  &algo_gtm46,
  &algo_tintless40,
};

/** isp_algo_execute_internal_algo:
 *
 *  @module: mct module handle
 *  @session_param: session parameters
 *  @stats_mask: stats mask
 *  @parsed_stats: parsed stats
 *  @algo_param: algorithm parameters
 *
 *  Execute algo by parsed stats and algo parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_algo_execute_internal_algo(mct_module_t *module,
  isp_session_param_t *session_param, mct_event_stats_isp_t *stats_data,
  isp_algo_params_t *algo_parm)
{
  uint32_t                  i;
  isp_saved_algo_params_t  *saved_algo_parm = NULL;

  if (!module || !algo_parm || !session_param) {
    ISP_ERR("failed, module %p algo_parm %p sess_parm %p",
      module, algo_parm, session_param);
    return FALSE;
  }

  /* get parser_parm*/
  saved_algo_parm = &session_param->parser_params.algo_saved_parm;

  for (i = 0; i < sizeof(isp44_algo_list) / sizeof(isp_algo_t*); i++) {
    void *algo_output = NULL;
    boolean ret = isp44_algo_list[i]->algo_func(module, stats_data,
      algo_parm, saved_algo_parm, &algo_output,
      stats_data->frame_id, session_param->svhdr_enb);
    if (ret == TRUE) {
      /* if operation is successful AND there is valid output to send */
      if (algo_output != NULL) {
        mct_event_t event;
        memset(&event, 0, sizeof(event));

        event.type = MCT_EVENT_MODULE_EVENT;
        event.identity = session_param->session_based_ide;
        event.direction = MCT_EVENT_DOWNSTREAM;
        event.u.module_event.type = isp44_algo_list[i]->event_type;
        event.u.module_event.module_event_data = (void *)algo_output;
        PTHREAD_MUTEX_LOCK(&session_param->mutex);
        ret = isp_trigger_thread_enqueue_event(&event,
          &session_param->saved_events, session_param->session_based_ide);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_trigger_thread_enqueue_event type %d",
            event.u.ctrl_event.type);
        }
        PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
        if (session_param->state == ISP_STATE_STREAMING) {
          /* Copy event to trigger update queue */
          PTHREAD_MUTEX_LOCK(&session_param->trigger_update_params.mutex);
          ret = isp_trigger_thread_enqueue_event(&event,
            &session_param->trigger_update_params.new_events,
            session_param->session_based_ide);
          if (ret == FALSE) {
            ISP_ERR("failed: isp_trigger_thread_enqueue_event type %d",
              event.u.ctrl_event.type);
          }
          PTHREAD_MUTEX_UNLOCK(&session_param->trigger_update_params.mutex);
        }
      }
    } else {
      ISP_ERR("failed to run algo %s", isp44_algo_list[i]->name);
      return ret;
    }
  }

  return TRUE;
}

/** isp_algo_stop_session:
 *
 *  @module: mct module handle
 *  @session_param: session param
 *
 *  Call stop session on all algo's
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_algo_stop_session(mct_module_t *module,
  isp_session_param_t *session_param)
{
  uint32_t                  i = 0;
  boolean                   ret = TRUE;

  if (!module || !session_param) {
    ISP_ERR("failed: module %p session_param %p", module, session_param);
    return FALSE;
  }

  /* get parser_parm*/

  for (i = 0; i < sizeof(isp44_algo_list) / sizeof(isp_algo_t *); i++) {
    if (isp44_algo_list[i] && isp44_algo_list[i]->stop_session_func) {
      ret = isp44_algo_list[i]->stop_session_func(module,
        &session_param->parser_params.algo_saved_parm);
      if (ret == FALSE) {
        ISP_ERR("failed: stop_session_func %s", isp44_algo_list[i]->name);
      }
    }
  }

  return ret;
}
