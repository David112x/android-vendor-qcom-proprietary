/* stats_utils.h
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __STATS_UTIL_H__
#define __STATS_UTIL_H__
#include "mct_stream.h"
#include "stats_debug.h"
#include "stats_event.h"

/**
 * Memory macros.
 */
#define STATS_MEMSET(ptr, value, num_bytes)       memset(ptr, value, num_bytes)
/*TODO: Replace memcpy with memscpy */
#define STATS_MEMCPY(dest, destLength, source, sourceLength) memcpy(dest, source, sourceLength)
#define STATS_MEMCMP(pSource1, pSource2, nLength) memcmp(pSource1, pSource2, nLength)

typedef enum {
  STATS_EXT_HANDLING_FAILURE = 0,  /**< Failure handling the event by extension.
                                      Business logic can process it.*/
  STATS_EXT_HANDLING_PARTIAL,      /**< Partially handled the event by extension.
                                      Business logic should still process it.*/
  STATS_EXT_HANDLING_COMPLETE,     /**< Completely handled the event by extension.
                                      Business logic should ignore it.*/
} stats_ext_return_type;

/* Common function pointers definitions to extend/override some
   module capabilities for external OEMs */
typedef stats_ext_return_type (*ext_init)(mct_port_t *port, uint32_t session_id);
typedef void (*ext_deinit)(mct_port_t *port);
typedef stats_ext_return_type (*ext_handle_module_event)(
  mct_port_t *port,
  mct_event_module_t *mod_evt);
typedef stats_ext_return_type (*ext_handle_control_event)(
  mct_port_t *port, mct_event_control_t *ctrl_evt);
typedef stats_ext_return_type (*ext_callback)(
  mct_port_t *port,
  void *core_out,
  void *update);
typedef stats_ext_return_type (*ext_handle_intra_event)(
  mct_port_t *port,
  mct_event_t *event);

/** stats_util_override_func_t: List of functions that can be
*    customized as per different requirement.
**/
typedef struct _stats_util_override_func {
  ext_init ext_init;            /**< Init extension */
  ext_deinit ext_deinit;         /**< De-init extension */
  ext_callback ext_callback;    /**< extension callback */
  ext_handle_module_event ext_handle_module_event; /**< handle extension module event */
  ext_handle_control_event ext_handle_control_event; /**< handle extension control event */
  ext_handle_intra_event ext_handle_intra_event; /**< handle extension port intra event */
} stats_util_override_func_t;

/* Call this function before use the stats_ext_event, it shall add 1 to the ref_count
 * @return  the count of stats_ext_event
 */
int32_t circular_stats_data_use(mct_event_stats_ext_t* stats_ext_event);

/* Call this function after finish using the stats data, it shall decrease ref_count by 1
 * @return  the count of stats_ext_event after decreasing
 */
int32_t circular_stats_data_done(mct_event_stats_ext_t* stats_ext_event,
                                 mct_port_t *port, uint32_t identity, uint32_t frame_id);

boolean stats_util_post_intramode_event(mct_port_t *src_port,
  uint32_t peer_identity,mct_event_module_type_t type, void *data);

boolean stats_util_get_peer_port(mct_event_t *event,
  uint32_t peer_identity, mct_port_t *this_port, mct_port_t **peer_port);

/* Callback function done using the stats buffer */
typedef void    (* stats_buffer_done_callback_func)(void* port, void* stats_buffer);

#endif //__STATS_UTIL_H__
