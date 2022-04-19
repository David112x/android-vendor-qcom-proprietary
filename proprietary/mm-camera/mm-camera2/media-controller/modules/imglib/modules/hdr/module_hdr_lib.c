/***************************************************************************
* Copyright (c) 2013-2014, 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/

#include "module_hdr_dbg.h"
#include "module_imglib_common.h"
#include "module_hdr_lib.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "hdr.h"
#include "hdr_chromatix.h"

/** hdr_lib_handle_t:
 *   @core_ops: core ops
 *   @mutex: used protecting single instance library
 *
 *  This structure defines hdr library handle
 **/

static hdr_chromatix_t g_hdr_chromatix = {
  #include "hdr_chromatix_data.h"
};

typedef struct
{
  img_core_ops_t core_ops;
  pthread_mutex_t mutex;
  hdr_chromatix_t *hdr_chromatix;
} hdr_lib_handle_t;

/** hdr_lib_t:
 *    @lib_handle: hdr library handle
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @user_data: user data
 *    @cb: notification cb
 *    @core_ops: function table for the operation
 *    @comp: pointer to the component ops
 *
 *  This structure defines hdr library instance
 **/
typedef struct
{
  hdr_lib_handle_t* lib_handle;
  module_hdr_buf_t* out_buff[HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS];
  module_hdr_buf_t* in_buff[HDR_LIB_IN_BUFFS];
  void* user_data;
  module_hdr_lib_notify_cb cb;
  img_core_ops_t core_ops;
  img_component_ops_t comp;
} hdr_lib_t;

#ifdef HDR_LIB_GHOSTBUSTER

/** MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX:
 *
 * Defines hdr library inplace output buffer index
 *
 * Returns hdr library inplace output buffer index
 **/
#define MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX (2)
static const int hdr_mod_frame_exposure_vals[HDR_LIB_IN_BUFFS] = { 0, -6, 6 };
static const int hdr_mod_frame_exposure_sequence[HDR_LIB_IN_BUFFS] = { 2, 1, 0 };

#else

/** MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX:
 *
 * Defines hdr library inplace output buffer index
 *
 * Returns hdr library inplace output buffer index
 **/
#define MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX (1)
static const int hdr_mod_frame_exposure_vals[HDR_LIB_IN_BUFFS] = { -6, 6 };
static const int hdr_mod_frame_exposure_sequence[HDR_LIB_IN_BUFFS] = { 1, 0 };

#endif

/** module_hdr_lib_event_handler
 *    @appdata: hdr library instance
 *    @event: pointer to the event
 *
 * Event handler for hdr library
 *
 * Returns IMG_SUCCESS in case of success or IMG_ERR_GENERAL
 **/
static int module_hdr_lib_event_handler(void* appdata, img_event_t *event)
{
  int ret_val = IMG_ERR_GENERAL;
  hdr_lib_t *hdr_lib = (hdr_lib_t *)appdata;
  img_frame_t *p_frame = NULL;
  hdr_crop_t out_crop;
  module_hdr_crop_t module_hdr_out_crop;
  int rc;

  IDBG_MED("%s +", __func__);

  if (event && hdr_lib && hdr_lib->cb) {
    IDBG_HIGH("%s:%d] type %d", __func__, __LINE__, event->type);

    switch (event->type) {
    case QIMG_EVT_BUF_DONE: {
      IMG_COMP_DQ_BUF(&hdr_lib->comp, &p_frame);
      break;
    }

    case QIMG_EVT_ERROR:
    case QIMG_EVT_DONE: {
      rc = IMG_COMP_GET_PARAM(&hdr_lib->comp, QHDR_OUT_CROP, (void *)&out_crop);
      if (IMG_SUCCESS != rc) {
        IDBG_ERROR("Cannot get QHDR_OUT_CROP in %s:%d \n", __func__, __LINE__);
        break;
      }

      module_hdr_out_crop.start_x = out_crop.start_x;
      module_hdr_out_crop.start_y = out_crop.start_y;
      module_hdr_out_crop.width = out_crop.width;
      module_hdr_out_crop.height = out_crop.height;

      hdr_lib->cb(hdr_lib->user_data, hdr_lib->out_buff, hdr_lib->in_buff,
        &module_hdr_out_crop);
    }
    default:
      break;
    }

    ret_val = IMG_SUCCESS;
  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_query_mod
 *    @caps: query capabilities data
 *
 * Fills capability and requirements for enabling this module
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_query_mod(mct_pipeline_cap_t* caps)
{
  boolean ret_val = FALSE;
  int i;
  int32_t ind;

  IDBG_MED("%s:%d] +", __func__, __LINE__);

  if (NULL != caps) {
    mct_pipeline_imaging_cap_t *buf =
        (mct_pipeline_imaging_cap_t *)&caps->imaging_cap;

    //specify number of frames needed for HDR to work
    buf->hdr_bracketing_setting.num_frames = HDR_LIB_IN_BUFFS;

    //specify exposure bracketing to turn on
    buf->hdr_bracketing_setting.exp_val.mode =
      CAM_EXP_BRACKETING_ON;

    //specify exposure values for each of the frames
    for (i = 0; i < HDR_LIB_IN_BUFFS; i++) {
      buf->hdr_bracketing_setting.exp_val.values[i] =
        (uint8_t)hdr_mod_frame_exposure_vals[i];
    }

    /* set HDR capability bit in imaging feature mask */
    buf->feature_mask |= (cam_feature_mask_t)CAM_QCOM_FEATURE_HDR;

    if ((buf->supported_scene_modes_cnt + 1) < CAM_SCENE_MODE_MAX) {
      ind = buf->supported_scene_modes_cnt;
      buf->supported_scene_modes[ind] = CAM_SCENE_MODE_HDR;
      buf->scene_mode_overrides[ind].ae_mode  = CAM_AE_MODE_ON;
      buf->scene_mode_overrides[ind].awb_mode  = CAM_WB_MODE_AUTO;
      buf->scene_mode_overrides[ind].af_mode  = CAM_FOCUS_MODE_AUTO;
      buf->supported_scene_modes_cnt++;
      IDBG_HIGH("%s:%d] Supported scenemode cnt %d", __func__, __LINE__,
        buf->supported_scene_modes_cnt);

      ret_val = TRUE;
    } else {
      IDBG_ERROR("%s:%d] Error cannot add scenemode", __func__, __LINE__);
    }

  } else {
    IDBG_ERROR("%s: Null pointer detected", __func__);
  }

  IDBG_MED("%s:%d] -", __func__, __LINE__);

  return ret_val;
}

/** module_hdr_lib_set_hdr_lib_params:
 *    @lib_instance: library handle instance
 *    @p_meta_list: metadata list
 *
 * Function to set hdr library input parameters
 *
 * Returns TRUE in case of success
 **/

static boolean module_hdr_lib_set_hdr_lib_params(void* lib_instance,
  mct_list_t* p_meta_list)
{
  boolean ret_val = TRUE;
  int32_t rc;
  unsigned int i;
  hdr_lib_t* hdr_lib = lib_instance;
  hdr_mode_t mode = MULTI_FRAME;
  img_meta_t* p_meta;
  void *p_data;

  IDBG_MED("%s +", __func__);

  if (NULL == hdr_lib) {
    IDBG_ERROR("%s:%d] Null pointer detected", __func__, __LINE__);
    ret_val = FALSE;
    goto end;
  }

  //set library mode
  rc = IMG_COMP_SET_PARAM(&hdr_lib->comp, QHDR_MODE,
      (void *)&mode);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    ret_val = FALSE;
    goto end;
  }

  rc = IMG_COMP_SET_PARAM(&hdr_lib->comp, QHDR_HDR_CHROMATIX,
    (void *)hdr_lib->lib_handle->hdr_chromatix);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    ret_val = FALSE;
    goto end;
  }

  for (i = 0; i < HDR_LIB_IN_BUFFS; i++) {
    rc = module_imglib_common_meta_find_by_frameid(
      hdr_lib->in_buff[i]->img_frame->frame_id, p_meta_list, &p_meta);

    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      ret_val = FALSE;
      goto end;
    }

    p_data = img_get_meta(p_meta, IMG_META_G_GAMMA);
    if (p_data) {
      rc = IMG_COMP_SET_PARAM(&hdr_lib->comp, QHDR_GAMMA_TABLE, p_data);
      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
        ret_val = FALSE;
        goto end;
      }
    }

    p_data = img_get_meta(p_meta, IMG_META_AEC_INFO);
    if (p_data) {
      rc = IMG_COMP_SET_PARAM(&hdr_lib->comp, QHDR_HDR_AEC_INFO, p_data);
      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
        ret_val = FALSE;
        goto end;
      }
    }
  }

end:
  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_start_hdr_filter:
 *    @lib_instance: library handle instance
 *    @buff: buffer handler for input/output image
 *
 * Function to start inplace hdr filter on the image data
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_lib_start_hdr_filter(void* lib_instance)
{
  boolean ret_val = FALSE;
  hdr_lib_t* hdr_lib = lib_instance;
  int rc = IMG_ERR_INVALID_INPUT;
  int i;

  IDBG_MED("%s +", __func__);

  if (hdr_lib) {

    rc = IMG_SUCCESS;

    for (i = 0; i < HDR_LIB_OUT_BUFFS; i++) {
      if (hdr_lib->out_buff[i]) {
        rc = IMG_COMP_Q_BUF(&hdr_lib->comp, hdr_lib->out_buff[i]->img_frame,
            IMG_IN);
        if (IMG_SUCCESS != rc) {
          break;
        }
      }
    }

    if (IMG_SUCCESS == rc) {
      for (i = 0; i < HDR_LIB_IN_BUFFS; i++) {
        if (hdr_lib->in_buff[i]) {
          rc = IMG_COMP_Q_BUF(&hdr_lib->comp, hdr_lib->in_buff[i]->img_frame,
              IMG_IN);
          if (IMG_SUCCESS != rc) {
            break;
          }
        }
      }
    }

    //if buffers enqueued successfully start processing
    if (IMG_SUCCESS == rc) {
      IDBG_HIGH("Start hdr processing");

      rc = IMG_COMP_START(&hdr_lib->comp, NULL);

      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("Cannot start hdr in %s\n", __func__);
      }
    } else {
      IDBG_ERROR("Cannot queue buffer in %s\n", __func__);
    }
  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  ret_val = GET_STATUS(rc);
  if (!ret_val) {
    IDBG_ERROR("Cannot apply hdr filter on the image data in %s\n", __func__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_get_output_inplace_index:
 *    @number: sequential number for inplace buffers
 *    @index: output inplace index
 *
 * Function to process image data
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_get_output_inplace_index(uint32_t number,
  uint32_t* index)
{
  boolean ret_val = FALSE;

  if (index) {
    if (number < IMGLIB_ARRAY_SIZE(hdr_mod_frame_exposure_sequence)) {

      *index = MODULE_HDR_LIB_INPLACE_OUTPUT_BUFFER_INDEX;
      if (*index > 0) {
        ret_val = TRUE;
      }

    } else {
      IDBG_ERROR("Input sequence number %d is more than array size %zu %s\n",
        number, IMGLIB_ARRAY_SIZE(hdr_mod_frame_exposure_sequence)-1, __func__);
    }
  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  return ret_val;
}

/** module_hdr_lib_process:
 *    @lib_instance: library handle instance
 *    @out_buff: output buffer handler array
 *    @in_buff: input buffer handler array
 *    @p_meta_list: list of metadata
 *    @user_data: user data
 *    @cb: notification cb
 *
 * Function to process image data
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_process(void* lib_instance, module_hdr_buf_t **out_buff,
  module_hdr_buf_t **in_buff, void* p_meta_list, void* user_data,
  module_hdr_lib_notify_cb cb)
{
  hdr_lib_t* hdr_lib = lib_instance;
  boolean ret_val = FALSE;
  int i;

  IDBG_MED("%s +", __func__);

  if (hdr_lib && hdr_lib->lib_handle && out_buff && in_buff && user_data
    && cb) {

    for (i = 0; i < HDR_LIB_IN_BUFFS; i++) {
      hdr_lib->in_buff[i] = in_buff[hdr_mod_frame_exposure_sequence[i]];
    }

    if (module_hdr_lib_set_hdr_lib_params(hdr_lib, p_meta_list)) {
      hdr_lib->user_data = user_data;
      hdr_lib->cb = cb;

      for (i = 0; i < HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS; i++) {
        hdr_lib->out_buff[i] = out_buff[i];
      }

      if (module_hdr_lib_start_hdr_filter(hdr_lib)) {
        ret_val = TRUE;
      }
    } else {
      IDBG_ERROR(" %s: HDR library setparams failed\n", __func__);
    }

  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_abort
 *    @lib_instance: library handle instance
 *
 * Aborts hdr library processing
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_abort(void* lib_instance)
{
  hdr_lib_t* hdr_lib = lib_instance;
  boolean ret_val = FALSE;
  int rc;

  IDBG_MED("%s +", __func__);

  if (hdr_lib && hdr_lib->lib_handle) {

    rc = IMG_COMP_ABORT(&hdr_lib->comp, NULL);

    ret_val = GET_STATUS(rc);
    if (!ret_val) {
      IDBG_ERROR("Cannot abort hdr library in %s\n", __func__);
    }
  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_deinit
 *    @lib_instance: library handle instance
 *
 * Deinitializes hdr library
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_deinit(void* lib_instance)
{
  boolean ret_val = FALSE;
  int rc;
  hdr_lib_t* hdr_lib = lib_instance;
  hdr_lib_handle_t* lib_handle;

  IDBG_MED("%s +", __func__);

  if (hdr_lib && hdr_lib->lib_handle) {

    rc = IMG_COMP_DEINIT(&hdr_lib->comp);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Cannot deinit hdr lib in %s\n", __func__);
    }

    lib_handle = hdr_lib->lib_handle;
    free(hdr_lib);

    ret_val = GET_STATUS(rc);
    if (!ret_val) {
      IDBG_ERROR("Cannot deinitialize hdr library in %s\n", __func__);
    }
  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_init
 *    @lib_handle: library handle
 *
 * Initializes hdr library
 *
 * Returns Library handle instance in case of success or NULL
 **/
void* module_hdr_lib_init(void* lib_handle)
{
  hdr_lib_t* hdr_lib = NULL;
  int rc;
  hdr_lib_handle_t* hdr_lib_handle = lib_handle;
  hdr_mode_t mode = MULTI_FRAME;

  IDBG_MED("%s +", __func__);

  if (lib_handle) {

    hdr_lib = malloc(sizeof(hdr_lib_t));

    if (hdr_lib) {

      hdr_lib->lib_handle = lib_handle;
      hdr_lib->core_ops = hdr_lib_handle->core_ops;

      rc = IMG_COMP_CREATE(&hdr_lib->core_ops, &hdr_lib->comp);
      if (rc == IMG_SUCCESS) {
        rc = IMG_COMP_INIT(&hdr_lib->comp, hdr_lib, &mode);
        if (rc == IMG_SUCCESS) {
          rc = IMG_COMP_SET_CB(&hdr_lib->comp,
              module_hdr_lib_event_handler);

          if (rc != IMG_SUCCESS) {
            IDBG_ERROR("Cannot set cb for hdr lib in %s\n", __func__);
          }
        } else {
          IDBG_ERROR("Cannot init hdr lib in %s\n", __func__);
        }
      } else {
        IDBG_ERROR("Cannot create hdr lib in %s\n", __func__);
      }

      if (rc != IMG_SUCCESS) {
        free(hdr_lib);
        hdr_lib = NULL;
      }
    } else {
      IDBG_ERROR("Cannot allocate memory for hdr library interface in %s\n",
        __func__);
    }

  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  IDBG_MED("%s -", __func__);

  return hdr_lib;
}

/** module_hdr_lib_unload
 *    @lib_handle: library handle
 *
 * Unloads hdr library
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_unload(void* lib_handle)
{
  boolean ret_val = FALSE;
  int rc;
  hdr_lib_handle_t* hdr_lib_handle = lib_handle;

  IDBG_MED("%s +", __func__);

  if (lib_handle) {
    rc = IMG_COMP_UNLOAD(&hdr_lib_handle->core_ops);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Cannot unload hdr lib in %s\n", __func__);
    }

    ret_val = GET_STATUS(rc);
  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_lib_load
 *
 * Loads hdr library
 *
 * Returns library handle in case of success or NULL
 **/
void* module_hdr_lib_load()
{
  hdr_lib_handle_t* hdr_lib_handle;
  int rc = IMG_ERR_GENERAL;

  IDBG_MED("%s +", __func__);

  hdr_lib_handle = malloc(sizeof(hdr_lib_handle_t));

  if (hdr_lib_handle) {
    if (!pthread_mutex_init(&hdr_lib_handle->mutex, NULL)) {
      rc = img_core_get_comp(IMG_COMP_HDR, "qcom.hdr",
        &hdr_lib_handle->core_ops);
      if (rc == IMG_SUCCESS) {
        rc = IMG_COMP_LOAD(&hdr_lib_handle->core_ops, NULL);
        if (rc != IMG_SUCCESS)
          IDBG_ERROR("Cannot load hdr lib in %s\n", __func__);
      } else {
        IDBG_ERROR("Cannot get hdr lib component in %s\n", __func__);
      }

      hdr_lib_handle->hdr_chromatix = &g_hdr_chromatix;

    } else {
      IDBG_ERROR("Cannot create mutex\n");
    }

    if (rc != IMG_SUCCESS) {
      free(hdr_lib_handle);
      hdr_lib_handle = NULL;
    }
  }

  IDBG_MED("%s -", __func__);

  return hdr_lib_handle;
}
