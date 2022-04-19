/**********************************************************************
* Copyright (c) 2016, 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __MULTIFRAMEPROC_COMP_H__
#define __MULTIFRAMEPROC_COMP_H__

#include "img_comp_priv.h"

// Maximum camera sessions per manager
#define MFC_MAX_CAMERA_SESSIONS 4

// Maximum camera session managers supported
#define MFC_MAX_CAMERA_MGR 2

// Active bundle list count
#define MFC_ACTIVE_BUNDLE_CNT 2

// Invalid frame id - magic number
#define MFC_INVALID_FRAMEID 0xffffffff

// Fwd decl
struct __multi_frameproc_mgr_t;
struct __multi_frameproc_lib_info_t;

/** mfp_mode_t:
 *  MFP_MODE_SINGLE: single camera mode
 *  MFP_MODE_DUAL: multi camera mode
 *
 *  Mode of operation the component
 */
typedef enum {
  MFP_MODE_SINGLE,
  MFP_MODE_MULTI,
} mfp_op_mode_t;

/** mfp_sync_t:
 *  MFP_SYNC_TIMESTAMP: Timestamp based synchronization
 *  MFP_SYNC_FRAMEID: Frame ID based synchronization
 *
 * Synchronization type
 */
typedef enum {
  MFP_SYNC_TIMESTAMP,
  MFP_SYNC_FRAMEID,
} mfp_sync_t;

/** img_active_bundle_t
 *   @bundle: frame bundle
 *   @valid: indicates if the element is valid
 *   @frame_id: frame ID of the bundle
 *
 *   Structure to hold the active bundle entry information
 **/
typedef struct {
  img_frame_bundle_t bundle;
  bool valid;
  uint32_t frame_id;
} img_active_bundle_t;

/** img_active_bundle_list_t:
 *  @active_bundle: array of active bundles
 *  @min_fidx: minimum frame index
 *  @count: count of active bundles
 *
 *  Represents the list of active bundles
 */
typedef struct {
  img_active_bundle_t active_bundle[MFC_ACTIVE_BUNDLE_CNT];
  uint32_t min_fidx;
  uint32_t count;
} img_active_bundle_list_t;

/** multiframeproc_comp_t
 *  @frame_id: frame id
 *  @is_master: master info
 *
 *  frame drop info
 */
typedef struct {
  uint32_t frame_id;
  uint32_t is_master;
} multiframeproc_drop_t;

/** multiframeproc_comp_t
 *   @b: base component
 *   @abort: Flag to indicate whether the algo is aborted
 *   @p_lib: place holder for frameproc library info
 *   @p_mgr: pointer to the manager
 *   @p_init_params: initialization parameters
 *   @is_master: indicate whether the component is master
 *   @last_frame_id: last frame ID processed
 *   @abl: active bundle list for the component
 *
 *   Frameproc component
 **/
typedef struct {
  img_component_t b;
  int8_t abort;
  void *p_lib;
  struct __multi_frameproc_mgr_t *p_mgr;
  img_init_params_t init_params;
  bool is_master;
  uint32_t last_frame_id;
  img_active_bundle_list_t abl;
} multi_frameproc_comp_t;

/** multi_frameproc_mgr_t
 *   @p_comp: pointer to the list of components
 *   @comp_cnt: number of active components
 *   @p_lib: multi frameproc library info
 *   @p_algocontext: algorithm context
 *   @msgQ: Message queue
 *   @is_used: check if the manager is in use.
 *   @th_client_id: thread handle
 *   @job_id: thread job id
 *   @dropQ: Frame drop Queue
 *   @abort: Flag to indicate whether the thread needs to be
 *         aborted
 *   @caps: capabilities of the manager
 *   @active_comp_cnt: active component count
 *
 *   Multi-Frameproc component
 **/
typedef struct __multi_frameproc_mgr_t {
  multi_frameproc_comp_t *p_comp[MFC_MAX_CAMERA_SESSIONS];
  uint32_t comp_cnt;
  struct __multi_frameproc_lib_info_t *p_lib;
  void *p_algocontext;
  img_queue_t msgQ;
  bool is_used;
  uint32_t th_client_id;
  uint32_t job_id;
  img_queue_t dropQ;
  bool abort;
  img_caps_t caps;
  uint32_t active_comp_cnt;
  int8_t num_active_meta;
} multi_frameproc_mgr_t;


/** frameproc_comp_t
 *   @ptr: library handle
 *   @img_algo_process: Main algo process function pointer
 *   @img_algo_init: Function pointer to create and initialize
 *                 the algorithm wrapper
 *   @img_algo_deinit: Function pointer to deinitialize and
 *                   destroy the algorithm wrapper
 *   @img_algo_abort: Function pointer to abort algorithm
 *                   processing
 *   @img_algo_frame_ind: Function pointer to indicate when an
 *             input frame is available
 *   @img_algo_meta_ind: Function pointer to indicate when an
 *             meta frame is available
 *   @img_algo_set_frame_ops: Function pointer to set frame ops
 *   @img_algo_preload: Function pointer for preload/
 *                    initialization of the algorithm parameters
 *                    before the algorithm is invoked.
 *   @img_algo_shutdown: Function pointer for deinitializing the
 *                    parameters which are done during preload
 *   @img_algo_get_caps: Function pointer to get algo
 *                    capabilities.
 *   @img_algo_get_lib_config: Function pointer to get algo
 *                    config.
 *   @img_algo_set_lib_config: Function pointer to set algo
 *                    config.
 *   @base_ops: Operation table and preload parameters for
 *            memory/thread operations
 *   @mgr: list of managers
 *   @mutex: lock
 *
 *   Frameproc library function pointers
 **/
typedef struct __multi_frameproc_lib_info_t {
  void *ptr;
  int (*img_algo_process)(void *p_context,
    img_frame_t *p_in_frame[],
    int in_frame_cnt,
    img_frame_t *p_out_frame[],
    int out_frame_cnt,
    img_meta_t *p_meta[],
    int meta_cnt);
  int (*img_algo_init)(void **pp_context,
    img_multicam_init_params_t *p_params);
  int (*img_algo_deinit)(void *p_context);
  int (*img_algo_abort)(void *p_context);
  int (*img_algo_frame_ind)(void *p_context, img_frame_t *p_frame);
  int (*img_algo_meta_ind)(void *p_context, img_meta_t *p_meta);
  int (*img_algo_set_frame_ops)(void *p_context, img_frame_ops_t *p_ops);
  int (*img_algo_preload)(img_base_ops_t *p_ops);
  int (*img_algo_shutdown)(img_base_ops_t *p_ops);
  int (*img_algo_get_caps)(void *p_context, void *p_data);
  int (*img_algo_get_lib_config)(void *p_context, void* p_data);
  int (*img_algo_set_lib_config)(void *p_context, void* p_data);
  img_base_ops_t base_ops;
  multi_frameproc_mgr_t mgr[MFC_MAX_CAMERA_MGR];
  pthread_mutex_t mutex;
} multi_frameproc_lib_info_t;

#endif //__MULTIFRAMEPROC_COMP_H__
