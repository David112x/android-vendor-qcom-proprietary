 /* iface_hvx.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IFACE_HVX_H__
#define __IFACE_HVX_H__

/* std headers */
#include <stdio.h>
#include <pthread.h>

/* iface headers */
#include "iface_def.h"
#include "hvx_lib.h"
#include "hvx_stub.h"

#define PING 0
#define PONG 1
#define NUM_PING_PONG 2
#define NUM_FRAME_BUF_ITER 1

typedef enum _iface_hvx_state_t {
  IFACE_STATE_INVALID,
  IFACE_STATE_OPENED,
  IFACE_STATE_CONFIGURED,
  IFACE_STATE_STREAMING,
  IFACE_STATE_MAX
} iface_hvx_state_t;

typedef enum _iface_hvx_dump_state_t
{
  IFACE_DUMP_STATE_INVALID,
  IFACE_DUMP_STATE_ALLOCATED,
  IFACE_DUMP_STATE_PROCEEDING,
  IFACE_DUMP_STATE_FINISHED,
  IFACE_DUMP_STATE_RESET,
  IFACE_DUMP_STATE_MAX
} iface_hvx_dump_state_t;

/** iface_hvx_singleton_t:
 *
 *  @mutex: lock
 *  @num_session: count of number of session
 *  @max_hvx_units: total num of hvx units supported
 *  @active_hvx_units: num of active hvx units
 *  @max_vector_mode: max vector mode supported
 *  @avail_vector_mode: available vector mode
 **/
typedef struct {
  pthread_mutex_t        mutex;
  uint32_t               num_session;
  uint32_t               max_hvx_units;
  uint32_t               available_hvx_units;
  hvx_stub_vector_mode_t max_vector_mode;
  hvx_stub_vector_mode_t available_vector_mode[VFE_MAX];
} iface_hvx_singleton_t;

typedef struct {
  uint64_t  buffer;
  int       buf_idx;
  char      buf_label;
} iface_hvx_adsp_buffers_t;

/** iface_hvx_t:
 *
 *  @mutex: mutex
 *  @enabled: Is HVX enabled for this session
 *  @state: current HVX state
 *  @num_dumped_frame: No of frames dumped
 *  @num_dump_frame: No of frames need to dump
 *  @hvx_info: sensor / hvx info
 *  @adsp_handle: ADSP handle
 *  @isp_id_mask: mask of isp id's used for this session
 *  @num_isp: number of isp
 *  @frame_id:
 *  @stub_vfe_type: vfe stub type
 *  @stub_lib_name: stub library name
 *  @set_dim: Sensor dimension
 *  @request_buffers: stats buffer information
 *  @hvx_stub_lib_handle: hvx stub lib handle
 *  @hvx_stub_event: Function pointer to store hvx stub lib dl
 *                 symbol
 *  @hvx_oem_lib_handle: OEM lib handle
 *  @adsp_handle: ADSP handle
 *  @hvx_oem_lib_fill_function_table: Function pointer to store
 *                                  HVX OEM lib dl symbol
 *  @hvx_oem_lib_func_table: function table to store OEM lib
 *             function pointers
 *  @hvx_oem_lib_data: OEM lib private data
 *
 **/
typedef struct _iface_hvx_t {
  /*************************************/
  /* Common data */
  pthread_mutex_t           mutex;
  boolean                   enabled;
  iface_hvx_state_t         state;
  iface_hvx_dump_state_t    dump_state;
  int                       num_dumped_frame;
  int                       num_dump_frame;
  hvx_lib_get_hvx_info_t    hvx_info;
  int                       adsp_handle;
  uint32_t                  isp_id_mask;
  uint32_t                  num_isp;
  uint32_t                  frame_id;
  enum hvx_stub_vfe_type_t  stub_vfe_type;
  char                      *stub_lib_name;
  sensor_set_dim_t          set_dim;
  /*************************************/

  /* Stats buffer information */
  hvx_stub_request_buffers_t request_buffers;
  iface_hvx_adsp_buffers_t   adsp_buffers[MAX_HVX_VFE][NUM_PING_PONG];
  /*************************************/

  /* frame buffer information */
  hvx_stub_request_buffers_t request_dump_buffers;
  iface_hvx_adsp_buffers_t   adsp_dump_buffers[MAX_HVX_VFE][NUM_PING_PONG];
  int32_t                  left_dump_id;
  int32_t                  right_dump_id;
  uint32_t                 dump_frame_size;
  /*************************************/

  /* HVX stub lib data */
  void                     *hvx_stub_lib_handle;
  int                     (*hvx_stub_event)(int handle,
    hvx_stub_event_type_t event_type, void *data, unsigned int data_size);
  /*************************************/

  /* HVX OEM lib data */
  void                     *hvx_oem_lib_handle;
  hvx_ret_type_t          (*hvx_oem_lib_fill_function_table)(
    hvx_lib_function_table_t *func_table);
  hvx_lib_function_table_t  hvx_oem_lib_func_table;
  void                     *hvx_oem_lib_data;
  pthread_mutex_t           buf_mutex;
  pthread_mutex_t           dump_mutex;
  /*************************************/

  /* SOF params*/
  hvx_lib_sof_params_t     current_sof_params;
  hvx_lib_sof_params_t     new_sof_params;
  /*************************************/


  /* Callback Func*/
  hvx_stub_set_callback_func_t set_callback_func;
} iface_hvx_t;


typedef struct iface_hvx_handle_session_map_t {
  int adsp_handle;
  iface_hvx_t *iface_hvx;
} iface_hvx_handle_session_map_t;

int32_t iface_hvx_callback_error(int handle, hvx_stub_vfe_type_t vfe_type,
   const char* error_msg, int error_msgLen, int frame_id);
#endif
