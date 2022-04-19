/**********************************************************************
* Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __MODULE_CAC_H__
#define __MODULE_CAC_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "cac.h"
#include "module_imglib_common.h"
#include "camera_dbg.h"
#include "modules.h"
#include "mct_pipeline.h"
#include "chromatix_swpostproc.h"
#include "chromatix_metadata.h"
#include "chromatix.h"
#include <stdbool.h>

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_CAC_SHIFT

#define IMGLIB_MOD_NAME "cac"
#define MAX_NUM_FRAMES 1
#define MAX_CAC_STATIC_PORTS 8
#define MAX_SESSIONS 10

#define MODULE_CAC_DEBUG 1

#define MODULE_CAC_MIN_NUM_PP_BUFS 1
#define MAX_CAC_STREAMS 6
#define MAX_PER_FRAME_LIST_COUNT 20
#define USE_RNR_HYSTERISIS

/** cac1_config_t
 *   @r_gamma: R gamma table
 *   @g_gamma: G gamma table
 *   @b_gamma: B gamma table
 *   @chromatix: Chromatix Ptr
 *   @cac_3a_info_t: awb gain data
 *   @cds_enable_flag: Chroma Downsampling enabled/disabled
 *   @img_3A_info: awb gain data
 *   @fd_data: face detection data
 *   @lds_enabled: LDS enabled flag
 *   @chroma_order: chroma order cbcr/crcb
 *   @yuyv422_to_420_enabled: YUYV422 to 420 conversion flag
 *   @metadata_buff: pointer to meta buffer
 *   @cds_enable_flag: flag which indicates that cds is already
 *                   applied
 *
 *
 */
typedef struct {
  img_gamma_t r_gamma;
  img_gamma_t g_gamma;
  img_gamma_t b_gamma;
  void *chromatix_ptr;
  cac_3a_info_t cac_3a_data;
  metadata_buffer_t *metadata_buff;
  uint8_t cds_enable_flag;
  img_3A_data_t img_3A_info;
  face_detection_data_t fd_data;
  uint8_t lds_enable_flag;
  cac_chroma_order chroma_order;
  uint8_t yuyv422_to_420_enabled;
} cac_config_t;


/** stream_port_mapping_t
 *   @stream_info: pointer to hold stream info
 *   @identity: MCT session/stream identity
 *   @p_sinkport: sink port associated with the stream
 *   @p_srcport: source port associated with the stream
 *   @stream_off: Flag to indicate whether streamoff is called

 *   CAC stream-based params
 **/
typedef struct {
  mct_stream_info_t *stream_info;
  uint32_t identity;
  mct_port_t *p_sinkport;
  mct_port_t *p_srcport;
  int8_t stream_off;
  img_dim_t in_dim;
} stream_port_mapping_t;


/** cac_client_t
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: component ops structure
 *   @frame: frame info from the module
 *   @state: state of face detection client
 *   @cac_cfg_info: CAC cfg info
 *   @frame: array of image frames
 *   @parent: pointer to the parent module
 *   @p_mod: pointer to the module
 *   @dump_input_frame: Flag to indicate whether input frame needs to be dumped
 *   @dump_output_frame: Flag to indicate whether out frame needs to be dumped
 *   @stream: maping for all streams and ports connecting them
 *   @stream_cnt: number of streams mapped to client
 *   @stream_mask: mask of all streams associated with the client
 *   @event_identity: MCT event identity
 *   @debug_meta_info: Meta info used for debug
 *   @cb_sent: Flag indicate if the CAC early callback is sent
 *   @use_sw_cds: Flag to indicate whether SW CDS is enabled
 *   @ion_fd: ion file descriptor
 *   @exec_mode: execution mode
 *   @access_mode: access mode
 *
 *   CAC client structure
 **/
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_component_ops_t comp;
  int state;
  cac_config_t cac_cfg_info;
  isp_buf_divert_t *p_buf_divert_data;
  img_frame_t frame[MAX_NUM_FRAMES];
  mct_module_t *parent_mod;
  void *p_mod;
  img_comp_mode_t mode;
  boolean dump_input_frame;
  boolean dump_output_frame;
  stream_port_mapping_t stream[MAX_CAC_STREAMS];
  int stream_cnt;
  uint32_t stream_mask;
  uint32_t event_identity;
  img_debug_info_t debug_meta_info;
  int cb_sent;
  uint8_t use_sw_cds;
  int32_t ion_fd;
  img_comp_execution_mode_t exec_mode;
  img_comp_access_mode_t access_mode;
} cac_client_t;

/** cac_per_frame_params_t
 *
 *   @valid_str_list: Valid list of streams for frameid
 *   @frameid: Frameid
 *
 *   CAC Per frame settings
 */
typedef struct {
  cam_stream_ID_t valid_str_list;
  uint32_t frameid;
} cac_per_frame_params_t;

/** cac_session_params_t
 *
 *   @mem_alloc_ref_count: memory allocation ref
 *                       count
 *   @prealloc_mem_rel: prealloc buffers release flag
 *   @msg_thread: message thread per session
 *   @sessionid : Sesionid
 *   @session_data: session data for per frame control
 *   @fpc_list: Array of frameid and List of streams that have a
 *            valid request for the frame
 *   @hal_version: HAL Version
 *   @cac_mode: CAC mode - Off, fast or quality
 *   @camif_output_size: Camif output size
 *   @hyst_info: Hysterisis Info for LDS and RNR
 *   @lds_enable_flag: LDS enable/disable flag
 *   @chromatix: Chromatix ptr received fromt the set chromatix
 *             ptr event
 *
 *   CAC Session Params
 */
typedef struct {
  int32_t mem_alloc_ref_count;
  int32_t prealloc_mem_rel;
  mod_imglib_msg_th_t msg_thread;
  uint32_t sessionid;
  mct_pipeline_session_data_t session_data;
  cac_per_frame_params_t fpc_list[MAX_PER_FRAME_LIST_COUNT];
  int32_t hal_version;
  cam_aberration_mode_t cac_mode;
  img_size_t camif_output;
  hysterisis_info_t hyst_info;
  uint8_t lds_enable_flag;
  chromatix_sw_postproc_type *chromatix;
} cac_session_params_t;

/** module_cac_t
 *   @cac_client_cnt: Variable to hold the number of CAC
 *              clients
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: core operation structure
 *   @lib_ref_count: reference count for cac library access
 *   @cac_client: List of CAC clients
 *   @parent_mod: pointer to the parent module
 *   @session_params: List of session params
 *   @module_type: Hold last updated module type
 *
 *   CAC module structure
 **/
typedef struct {
  int cac_client_cnt;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_core_ops_t core_ops;
  int lib_ref_count;
  mct_list_t *cac_client;
  mct_module_t *parent_mod;
  mct_list_t *session_params;
  mct_module_type_t module_type;
} module_cac_t;


/*Common APIs*/
void *module_cac_get_session_params(module_cac_t *p_mod, uint32_t session_id);

int module_cac_get_stream_by_id(cac_client_t *p_client, uint32_t identity);

/*CAC client APIs*/
int module_cac_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  mct_stream_info_t *stream_info);

void module_cac_client_destroy(cac_client_t *p_client);

int module_cac_client_stop(cac_client_t *p_client);

int module_cac_client_exec(cac_client_t *p_client,
  uint32_t *p_buffer_access, boolean *p_invalidate_cache_required);

void module_cac_client_divert_exec(void *userdata, void *data);

int module_cac_config_client(cac_client_t *p_client);

void module_cac_client_allocate_cac_buffer(void *userdata, void *data);

void module_cac_client_release_cac_buffer(void *userdata, void *data);

#endif
