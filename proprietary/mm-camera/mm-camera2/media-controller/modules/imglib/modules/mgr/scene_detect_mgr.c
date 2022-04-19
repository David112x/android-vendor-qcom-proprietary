/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "scene_detect_mgr.h"
#include "img_dbg.h"
#include "img_common.h"
#include "modules.h"

/**
 * magic number to generate the unique client index
 */
#define CLIENT_IDX_MAGIC_NO 0xffee0000

/**
 * singleton scene detect manager
 * ToDo: unify all the managers
 */
static img_scene_detect_mgr_t *g_p_scene_detect_mgr;

/** img_scene_mgr_init:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *
 * Description: This function initializes scene detect manager
 *
 * Return values:
 *     imaging errors
 *
 * Notes: none
 **/
int32_t img_scene_mgr_init(img_scene_detect_mgr_t *p_scene_mgr)
{
  if (!p_scene_mgr) {
    IDBG_ERROR("%s:%d] Error: invalid instance", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (!p_scene_mgr->is_init) {
    memset(p_scene_mgr, 0x0, sizeof(*p_scene_mgr));
    pthread_mutex_init(&p_scene_mgr->lock, NULL);
    p_scene_mgr->is_init = true;
    g_p_scene_detect_mgr = p_scene_mgr;
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/** img_scene_mgr_deinit:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *
 * Description: This function de-initializes scene detect
 * manager
 *
 * Return values:
 *     imaging errors
 *
 * Notes: none
 **/
int32_t img_scene_mgr_deinit(img_scene_detect_mgr_t *p_scene_mgr)
{
  if (!p_scene_mgr) {
    IDBG_ERROR("%s:%d] Error: invalid instance", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (p_scene_mgr->is_init) {
    pthread_mutex_destroy(&p_scene_mgr->lock);
    p_scene_mgr->is_init = false;
    g_p_scene_detect_mgr = NULL;
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/** img_scene_mgr_register:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @p_client_data: pointer to the client data
 *
 * Description: This function registers scene detect data to the
 *      manager
 *
 * Return values:
 *     client id, else return MAX_IMG_SCENE_CLIENT
 *
 * Notes: none
 **/
int32_t get_free_client_idx(img_scene_detect_mgr_t *p_scene_mgr)
{
  int i;
  for (i = 0; i < MAX_IMG_SCENE_CLIENT; i++) {
    if (!p_scene_mgr->client_info[i].is_used)
      break;
  }
  return i;
}

/** img_scene_mgr_register:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @p_client_data: pointer to the client data
 *
 * Description: This function registers scene detect data to the
 *      manager
 *
 * Return values:
 *     client id, else return 0
 *
 * Notes: none
 **/
int32_t img_scene_mgr_register(img_scene_detect_mgr_t *p_scene_mgr,
  img_sd_client_data_t *p_client_data)
{
  uint32_t idx;
  img_sd_client_internal_t *p_client_info;
  if (!p_scene_mgr || !p_scene_mgr->is_init || !p_client_data) {
    IDBG_ERROR("%s:%d] Error: invalid instance %p %p", __func__, __LINE__,
      p_scene_mgr, p_client_data);
    return 0;
  }

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if ((p_scene_mgr->num_clients >= MAX_IMG_SCENE_CLIENT) ||
    !p_client_data->p_appdata || !p_client_data->p_detect) {
    IDBG_ERROR("%s:%d] Error: cannot register %d %p %p", __func__, __LINE__,
      p_scene_mgr->num_clients,
      p_client_data->p_appdata,
      p_client_data->p_detect);
    return 0;
  }

  QIMG_LOCK(&p_scene_mgr->lock);
  idx = get_free_client_idx(p_scene_mgr);
  if (idx >= MAX_IMG_SCENE_CLIENT) {
    IDBG_ERROR("%s:%d] Error: invalid client_idx %d", __func__, __LINE__,
      p_scene_mgr->num_clients);
    QIMG_UNLOCK(&p_scene_mgr->lock);
    return 0;
  }

  p_client_info = &p_scene_mgr->client_info[idx];
  p_client_info->is_used = true;
  p_client_info->client_id = (CLIENT_IDX_MAGIC_NO | idx);
  p_client_info->data = *p_client_data;
  p_scene_mgr->num_clients++;
  QIMG_UNLOCK(&p_scene_mgr->lock);

  IDBG_MED("%s:%d] client %x X", __func__, __LINE__,
    p_scene_mgr->client_info[idx].client_id);
  return p_scene_mgr->client_info[idx].client_id;
}

/** img_scene_mgr_unregister:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @p_client_data: pointer to the client data
 *
 * Description: This function unregisters scene detect data to
 *      the manager
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_unregister(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t client_idx)
{
  int32_t idx = client_idx & 0xff;
  img_sd_client_internal_t *p_client_info;

  if (!p_scene_mgr || !p_scene_mgr->is_init || (idx >= MAX_IMG_SCENE_CLIENT)) {
    IDBG_ERROR("%s:%d] Error: invalid instance %p %d", __func__, __LINE__,
      p_scene_mgr, idx);
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_MED("%s:%d] client %x E", __func__, __LINE__, client_idx);
  QIMG_LOCK(&p_scene_mgr->lock);
  p_client_info = &p_scene_mgr->client_info[idx];

  if (!p_client_info->is_used) {
    IDBG_ERROR("%s:%d] Error: invalid client idx %d", __func__, __LINE__,
      idx);
    QIMG_UNLOCK(&p_scene_mgr->lock);
    return IMG_ERR_INVALID_INPUT;
  }

  p_client_info->is_used = false;
  p_client_info->client_id = 0;
  p_scene_mgr->num_clients--;
  QIMG_UNLOCK(&p_scene_mgr->lock);

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/** img_scene_mgr_handle_chromatix_update:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @p_chromatix: chromatix pointer
 *
 * Description: This function is called during chromatix update
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_handle_chromatix_update(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  void *p_chromatix)
{
  img_scene_detect_list_t *p_scene_list;
  modulesChromatix_t *p_chromatix_param = (modulesChromatix_t *)p_chromatix;
  if (!p_scene_mgr || !p_scene_mgr->is_init ||
    (session_id > MAX_IMG_SCENE_SESSIONS)) {
    IDBG_ERROR("%s:%d] Error: invalid instance", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  QIMG_LOCK(&p_scene_mgr->lock);
  IDBG_LOW("%s:%d] p_chromatix %p", __func__, __LINE__, p_chromatix);
  p_scene_list = &p_scene_mgr->scene_list[session_id - 1];
  p_scene_list->chromatix_param = *p_chromatix_param;
  QIMG_UNLOCK(&p_scene_mgr->lock);
  return IMG_SUCCESS;
}

/** img_scene_mgr_handle_sof_update:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @frame_id: frame idx
 *
 * Description: This function is called during SOF update
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_handle_sof_update(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  int32_t frame_id)
{
  img_scene_detect_list_t *p_scene_list;
  if (!p_scene_mgr || !p_scene_mgr->is_init ||
    (session_id > MAX_IMG_SCENE_SESSIONS)) {
    IDBG_ERROR("%s:%d] Error: invalid instance", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  QIMG_LOCK(&p_scene_mgr->lock);
  IDBG_LOW("%s:%d] frame_idx %d", __func__, __LINE__, frame_id);
  p_scene_list = &p_scene_mgr->scene_list[session_id - 1];
  p_scene_list->frame_id = frame_id;
  QIMG_UNLOCK(&p_scene_mgr->lock);
  return IMG_SUCCESS;
}

/** img_scene_mgr_detect:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *
 * Description: This function is for sending the stats/scene
 *              updates for the registered clients
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_detect(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id)
{
  uint32_t count = 0;
  int32_t i, rc = IMG_SUCCESS;
  img_sd_client_internal_t *p_client_info;
  img_scene_detect_list_t *p_scene_list;
  img_dyn_feat_metadata_t *p_md_dynfeat;

  p_scene_list = &p_scene_mgr->scene_list[session_id - 1];
  p_md_dynfeat = &p_scene_mgr->md_dynfeat[p_scene_list->frame_id
    % MAX_IMG_PIPELINE_DELAY];
  p_md_dynfeat->dyn_data_ready = false;
  /* memset since this needs to be updated by all the clients.
     default values are 0 */
  memset(&p_md_dynfeat->dyn_data, 0x0, sizeof(p_md_dynfeat->dyn_data));
  for (i = 0; (i < MAX_IMG_SCENE_CLIENT) &&
    (count < p_scene_mgr->num_clients); i++) {
    p_client_info = &p_scene_mgr->client_info[i];
    if (p_client_info->is_used && p_client_info->data.p_detect) {
      rc = p_client_info->data.p_detect(p_client_info->data.p_appdata,
        session_id, p_scene_list, &p_md_dynfeat->dyn_data);
      if (IMG_ERROR(rc)) {
        IDBG_HIGH("%s:%d] failed for client %d", __func__, __LINE__, i);
      }
    }
    count++;
  }
  /* set magic number */
  p_md_dynfeat->dyn_data.dyn_feature_mask |= (1ULL << 63);
  IDBG_LOW("%s:%d] Enable feature 0x%llx", __func__, __LINE__,
    p_md_dynfeat->dyn_data.dyn_feature_mask);

  /* ToDo: optimize for toggles */
  p_md_dynfeat->dyn_data_ready = true;

  return IMG_SUCCESS;
}

/** img_scene_mgr_handle_aec_update:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @p_aec_update: pointer to AEC update
 *
 * Description: This function is called during AEC update
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_handle_aec_update(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  aec_update_t *p_aec_update)
{
  img_scene_detect_list_t *p_scene_list;
  img_scene_detect_data_t *p_sd_data;
  int32_t rc;

  if (!p_scene_mgr || !p_scene_mgr->is_init || !p_aec_update ||
    (session_id > MAX_IMG_SCENE_SESSIONS)) {
    IDBG_ERROR("%s:%d] Error: invalid instance %p %p %u",
      __func__, __LINE__,
      p_scene_mgr, p_aec_update, session_id);
    return IMG_ERR_INVALID_INPUT;
  }

  p_scene_list = &p_scene_mgr->scene_list[session_id - 1];
  if (p_scene_list->frame_id >= p_aec_update->frame_id) {
    /* skip update */
    return IMG_ERR_BUSY;
  }

  QIMG_LOCK(&p_scene_mgr->lock);

  IDBG_LOW("%s:%d] g %f lc %f frame_id %d %d E", __func__, __LINE__,
    p_aec_update->real_gain,
    p_aec_update->lux_idx,
    p_scene_list->frame_id,
    p_aec_update->frame_id);

  p_scene_list->frame_id = p_aec_update->frame_id;
  /* update current index */
  p_scene_list->cur_idx = (p_scene_list->cur_idx + 1) % MAX_IMG_SCENE_DATA;
  p_sd_data = &p_scene_list->sd_data[p_scene_list->cur_idx];
  p_sd_data->gain = p_aec_update->real_gain;
  p_sd_data->lux_idx = p_aec_update->lux_idx;
  if (p_scene_list->count < MAX_IMG_SCENE_DATA) {
    p_scene_list->count++;
  }

  rc = img_scene_mgr_detect(p_scene_mgr, session_id);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error: detect failed %d", __func__, __LINE__, rc);
    goto end;
  }

end:
  QIMG_UNLOCK(&p_scene_mgr->lock);
  IDBG_LOW("%s:%d] X", __func__, __LINE__);
  return rc;
}

/** img_scene_mgr_handle_awb_update:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @p_aec_update: pointer to AWB update
 *
 * Description: This function is called during AWB update
 *
 * Return values:
 *     imaging error
 *
 * Notes: Not supported now
 **/
int32_t img_scene_mgr_handle_awb_update(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  awb_update_t *p_awb_update)
{
  IMG_UNUSED(p_scene_mgr);
  IMG_UNUSED(p_awb_update);
  IMG_UNUSED(session_id);
  return IMG_ERR_NOT_SUPPORTED;
}

/** img_scene_mgr_reset:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @max_apply_delay: maximum applying delay for the session
 *  @max_report_delay: maximum report delay for the session
 *
 * Description: This function resets the scene manager
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void img_scene_mgr_reset(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  uint32_t max_apply_delay,
  uint32_t max_report_delay)
{
  uint32_t s_idx = session_id - 1;
  if (!p_scene_mgr || !p_scene_mgr->is_init ||
    (session_id > MAX_IMG_SCENE_SESSIONS)) {
    IDBG_ERROR("%s:%d] Error: invalid instance %p %d",
      __func__, __LINE__, p_scene_mgr, session_id);
    return;
  }
  p_scene_mgr->scene_list[s_idx].frame_id = 0;
  p_scene_mgr->scene_list[s_idx].count = 0;
  p_scene_mgr->max_apply_delay = max_apply_delay;
  p_scene_mgr->max_report_delay = max_report_delay;
}

/** img_scene_mgr_get_featmeta:
 *
 *  Arguments:
 *  @frame_id: frame_id
 *
 * Description: This function gets the feature metadata
 *
 * Return values:
 *     feat metadata pointer
 *
 * Notes: none
 **/
img_dyn_feat_metadata_t *img_scene_mgr_get_featmeta(
  img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t frame_id)
{
  uint32_t cur_frame_id = frame_id;
  int32_t t_idx, index = -1;

  if (!p_scene_mgr) {
    IDBG_ERROR("Error invalid scene manager");
    return NULL;
  }

  while ((cur_frame_id > 0) &&
    ((frame_id - cur_frame_id) <= MAX_IMG_PIPELINE_DELAY)) {
    t_idx = (cur_frame_id - p_scene_mgr->max_apply_delay
      - p_scene_mgr->max_report_delay + MAX_IMG_PIPELINE_DELAY)
      % MAX_IMG_PIPELINE_DELAY;
    if (p_scene_mgr->md_dynfeat[t_idx].dyn_data_ready) {
      index = t_idx;
      break;
    }
    cur_frame_id--;
  }
  return (index >= 0) ? &p_scene_mgr->md_dynfeat[index] : NULL;
}

/** get_scene_mgr:
 *
 *  Arguments:
 *  none
 *
 * Description: This function gets the scene manager
 *
 * Return values:
 *     scene detect manager instance, NULL if scene manager is
 *     not available
 *
 * Notes: can be called by the clients
 **/
inline img_scene_detect_mgr_t *get_scene_mgr()
{
  return (g_p_scene_detect_mgr && g_p_scene_detect_mgr->is_init) ?
    g_p_scene_detect_mgr : NULL;
}
