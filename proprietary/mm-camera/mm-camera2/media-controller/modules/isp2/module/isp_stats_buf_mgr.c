/* isp_stats_buf_mgr.c
 *
 * Copyright (c) 2014, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>

/* kernel headers */
#include <media/msmb_isp.h>

/* isp headers */
#include "isp_module.h"
#include "isp_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

static const uint32_t s_parsed_stats_size[] = {
  [MSM_ISP_STATS_RS]          = sizeof(q3a_rs_stats_t),
  [MSM_ISP_STATS_CS]          = sizeof(q3a_cs_stats_t),
  [MSM_ISP_STATS_IHIST]       = sizeof(q3a_ihist_stats_t),
  [MSM_ISP_STATS_BG]          = sizeof(q3a_bg_stats_t),
  [MSM_ISP_STATS_AEC_BG]      = sizeof(q3a_bg_stats_t),
  [MSM_ISP_STATS_BF]          = sizeof(q3a_bf_stats_t),
  [MSM_ISP_STATS_BE]          = sizeof(q3a_be_stats_t),
  [MSM_ISP_STATS_BHIST]       = sizeof(q3a_bhist_stats_t),
  [MSM_ISP_STATS_BF_SCALE]    = sizeof(q3a_bf_stats_t),
  [MSM_ISP_STATS_HDR_BE]      = sizeof(q3a_hdr_be_stats_t),
  [MSM_ISP_STATS_HDR_BHIST]   = sizeof(q3a_bhist_stats_t),
};

/** isp_stats_buf_mgr_init:
 *
 *  @buf_mgr: buffer manager handle
 *  @resource_request: resource request handle
 *
 *  Initialize buffer manager
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_stats_buf_mgr_init(isp_stats_buf_mgr_t *buf_mgr,
  iface_resource_request_t *resource_request,
  boolean aec_bg_hdr_be_swapped)
{
  boolean                  ret = TRUE;
  uint32_t                 i = 0,
                           j = 0,
                           num_stats_irq[STATS_COMPOSITE_GRP_MAX],
                           comp_flag = 0,
                           stats_mask = 0;
  isp_stats_buf_info_t    *stats_buf_info = NULL;
  isp_stats_info_t        *isp_stats_info = NULL;
  enum msm_isp_stats_type  stats_type = MSM_ISP_STATS_MAX;
  enum msm_isp_stats_type  ext_stats_type = MSM_ISP_STATS_MAX;

  if (!buf_mgr || !resource_request) {
    ISP_ERR("failed: %p %p", buf_mgr, resource_request);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&buf_mgr->mutex);
  if (buf_mgr->init_done == TRUE) {
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return TRUE;
  }

  if (!resource_request->num_stats_stream) {
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    /* No stats */
    return TRUE;
  }

  if (resource_request->num_stats_stream >= MSM_ISP_STATS_MAX) {
    ISP_ERR("failed num_stats_stream %d max %d",
      resource_request->num_stats_stream, MSM_ISP_STATS_MAX);
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return FALSE;
  }

  /* Sanity checks */
  /* Check whether stats_event_data pointer is non NULL */
  if (buf_mgr->stats_event_data) {
    /* Error */
    ISP_ERR("error: stats_event_data %p", buf_mgr->stats_event_data);
    free(buf_mgr->stats_event_data);
  }
  /* Check whether stats_event_data_state pointer is non NULL */
  if (buf_mgr->stats_event_data_state) {
    /* Error */
    ISP_ERR("error: stats_event_data_state %p",
      buf_mgr->stats_event_data_state);
    free(buf_mgr->stats_event_data_state);
  }

  memset(buf_mgr, 0, sizeof(isp_stats_buf_mgr_t));

  /* Count number of composite and individual stats irq's required
   * 1) stats that has STATS_COMPOSITE_GRP_NONE as comp flag needs
   *    individual irq
   * 2) stats that has other comp_flag are composite together
   */
  memset(num_stats_irq, 0, sizeof(uint32_t) * STATS_COMPOSITE_GRP_MAX);
  for (i = 0; i < resource_request->num_stats_stream; i++) {
    comp_flag = resource_request->isp_stats_info[i].comp_flag;
    if (comp_flag < STATS_COMPOSITE_GRP_MAX) {
      num_stats_irq[comp_flag]++;
    }
  }

  buf_mgr->num_stats_event_data = num_stats_irq[STATS_COMPOSITE_GRP_NONE];

  for (i = STATS_COMPOSITE_GRP_1; i < STATS_COMPOSITE_GRP_MAX; i++) {
    if (num_stats_irq[i]) {
      buf_mgr->num_stats_event_data++;
    }
  }

  /* We get buf_mgr->num_stats_event_data (N) irq's per frame. So, per frame,
   * we send N 3a parsed structs to 3a. Have 3 more copies for rotation
   */
  buf_mgr->num_stats_event_data *= MAX_STATS_BUFFERS;
  ISP_DBG("num_stats_event_data %d", buf_mgr->num_stats_event_data);

  if (ret == TRUE) {
    buf_mgr->stats_event_data =
      (mct_event_stats_isp_t *)CAM_CALLOC(buf_mgr->num_stats_event_data,
         sizeof(mct_event_stats_isp_t));
    if (!buf_mgr->stats_event_data) {
      ISP_ERR("failed: no memory");
      ret = FALSE;
    }
  }

  if (ret == TRUE) {
    buf_mgr->stats_event_data_state =
      (isp_buf_state_t *)CAM_CALLOC(buf_mgr->num_stats_event_data,
         sizeof(isp_buf_state_t));
    if (!buf_mgr->stats_event_data_state) {
      ISP_ERR("failed: no memory");
      ret = FALSE;
    }
  }

  if (ret == TRUE) {
    /* Initialize mutex and  stats_event_data_state */
    for (i = 0; i < buf_mgr->num_stats_event_data; i++) {
      buf_mgr->stats_event_data_state[i] = ISP_BUF_STATE_FREE;
      ISP_DBG("alloc stats_event_data[%d] %p state %d", i,
        &buf_mgr->stats_event_data[i], buf_mgr->stats_event_data_state[i]);
    }
  }

  if (ret == TRUE) {
    buf_mgr->num_stats_buf_info = MAX_STATS_BUFFERS;
    for (i = 0; i < resource_request->num_stats_stream; i++) {
      isp_stats_info = &resource_request->isp_stats_info[i];
      ext_stats_type = stats_type = isp_stats_info->stats_type;

      if (aec_bg_hdr_be_swapped == TRUE) {
        if (stats_type == MSM_ISP_STATS_AEC_BG)
          ext_stats_type = MSM_ISP_STATS_HDR_BE;
        else if (stats_type == MSM_ISP_STATS_HDR_BE)
          ext_stats_type = MSM_ISP_STATS_AEC_BG;
      }

      if (stats_type >= ARRAY_SIZE(s_parsed_stats_size)) {
        ISP_ERR("failed: invalid stats type %d max %d", stats_type,
          MSM_ISP_STATS_MAX);
        continue;
      }

      if (!s_parsed_stats_size[stats_type]) {
        ISP_ERR("failed: stats size %d for stats type %d",
          s_parsed_stats_size[stats_type], stats_type);
        break;
      }
      if (buf_mgr->stats_buf_info[ext_stats_type]) {
        ISP_ERR("error: stats_type %d already initialized", ext_stats_type);
        break;
      }
      buf_mgr->stats_buf_info[ext_stats_type] =
        (isp_stats_buf_info_t *)CAM_MALLOC(sizeof(isp_stats_buf_info_t) *
        buf_mgr->num_stats_buf_info);
      if (!buf_mgr->stats_buf_info[ext_stats_type]) {
        ISP_ERR("failed: no memory");
        ret = FALSE;
      } else {
        stats_buf_info = buf_mgr->stats_buf_info[ext_stats_type];
        for (j = 0; j < MAX_STATS_BUFFERS; j++) {
          stats_buf_info[j].addr =
            CAM_CALLOC(1, s_parsed_stats_size[ext_stats_type]);
          if (!stats_buf_info[j].addr) {
            ISP_ERR("failed: no memory");
            ret = FALSE;
            break;
          }
          memset(stats_buf_info[j].addr, 0, s_parsed_stats_size[ext_stats_type]);
          stats_buf_info[j].buf_size = s_parsed_stats_size[ext_stats_type];
          stats_buf_info[j].buf_state = ISP_BUF_STATE_FREE;
          ISP_DBG("alloc stats type %d idx %d addr %p len %d state %d",
            stats_type, j, stats_buf_info[j].addr, stats_buf_info[j].buf_size,
            stats_buf_info[j].buf_state);
        }
        if (ret == FALSE) {
          break;
        }
      }
    }
  }

  if (ret == FALSE) {
    for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
      if (!buf_mgr->stats_buf_info[i]) {
        continue;
      }
      stats_buf_info = buf_mgr->stats_buf_info[i];
      for (j = 0; j < MAX_STATS_BUFFERS; j++) {
        if (stats_buf_info[j].addr) {
          free(stats_buf_info[j].addr);
          stats_buf_info[j].addr = NULL;
        }
      }
    }
    free(buf_mgr->stats_event_data_state);
    free(buf_mgr->stats_event_data);
    memset(buf_mgr, 0, sizeof(isp_stats_buf_mgr_t));
  }
  if (ret == TRUE) {
    buf_mgr->init_done = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
  return ret;
} /* isp_stats_buf_mgr_init */

/** isp_stats_buf_mgr_deinit:
 *
 *  @buf_mgr: buffer manager handle
 *
 *  Deinit buffer manager
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_stats_buf_mgr_deinit(isp_stats_buf_mgr_t *buf_mgr)
{
  boolean                ret = TRUE;
  uint32_t               i = 0,
                         j = 0;
  isp_stats_buf_info_t  *stats_buf_info = NULL;

  if (!buf_mgr) {
    ISP_ERR("failed: %p", buf_mgr);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&buf_mgr->mutex);
  if (buf_mgr->init_done == FALSE) {
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return TRUE;
  }

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (!buf_mgr->stats_buf_info[i]) {
      continue;
    }
    stats_buf_info = buf_mgr->stats_buf_info[i];
    for (j = 0; j < MAX_STATS_BUFFERS; j++) {
      if (stats_buf_info[j].addr) {
        ISP_DBG("free stats type %d idx %d addr %p len %d state %d",
          i, j, stats_buf_info[j].addr, stats_buf_info[j].buf_size,
          stats_buf_info[j].buf_state);
        free(stats_buf_info[j].addr);
        stats_buf_info[j].addr = NULL;
      }
    }
    free(stats_buf_info);
  }
  for (i = 0; i < buf_mgr->num_stats_event_data; i++) {
    ISP_DBG("free stats_event_data[%d] %p state %d", i,
      &buf_mgr->stats_event_data[i], buf_mgr->stats_event_data_state[i]);
  }
  free(buf_mgr->stats_event_data_state);
  free(buf_mgr->stats_event_data);
  memset(buf_mgr, 0, sizeof(isp_stats_buf_mgr_t));
  buf_mgr->init_done = FALSE;
  PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
  return ret;
} /* isp_stats_buf_mgr_deinit */

/** isp_stats_buf_mgr_get_buf:
 *
 *  @buf_mgr: buffer manager handle
 *  @stats_mask: stats type mask
 *
 *  Get free buffer from queue
 *
 *  Return buffers on success and NULL on failure
 **/
mct_event_stats_isp_t *isp_stats_buf_mgr_get_buf(isp_stats_buf_mgr_t *buf_mgr,
  uint32_t stats_mask)
{
  boolean                     ret = TRUE;
  uint32_t                    i = 0,
                              j = 0,
                              k = 0;
  isp_stats_buf_info_t       *stats_buf_info = NULL;
  mct_event_stats_isp_t      *stats_event_data = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;

  if (!buf_mgr || !stats_mask) {
    ISP_ERR("failed: buf_mgr %p stats_mask %x", buf_mgr, stats_mask);
    return NULL;
  }
  PTHREAD_MUTEX_LOCK(&buf_mgr->mutex);
  if (buf_mgr->init_done == FALSE) {
    ISP_ERR("failed: get buf called without buf mgr initialization");
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return NULL;
  }
  ISP_DBG("get_buf stats_mask %x", stats_mask);

  /* Find stats_event_data buffer */
  for (i = 0; i < buf_mgr->num_stats_event_data; i++) {
    if (buf_mgr->stats_event_data_state[i] == ISP_BUF_STATE_FREE) {
      buf_mgr->stats_event_data_state[i] = ISP_BUF_STATE_USED;
      stats_event_data = &buf_mgr->stats_event_data[i];
      ISP_DBG("get_buf stats_event_data %p state %d", stats_event_data,
        buf_mgr->stats_event_data_state[i]);
      break;
    }
  }

  if (!stats_event_data) {
    ISP_ERR("failed: could not find stats instance");
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return NULL;
  }
  memset(stats_event_data, 0, sizeof(mct_event_stats_isp_t));
  stats_event_data->stats_mask = stats_mask;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if ((1 << i) & stats_mask) {
      ISP_DBG("get_buf stats_type %d mask %x", i, stats_mask);
      stats_buf_info = buf_mgr->stats_buf_info[i];
      if (!buf_mgr->stats_buf_info[i]) {
        ISP_ERR("failed: stats_buf_info %p for stats type %d",
          buf_mgr->stats_buf_info[i], i);
        ret = FALSE;
        break;
      }
      stats_data = &stats_event_data->stats_data[i];
      for (j = 0; j < MAX_STATS_BUFFERS; j++) {
        if ((stats_buf_info[j].buf_state == ISP_BUF_STATE_FREE) &&
          stats_buf_info[j].addr) {
          memset(stats_buf_info[j].addr, 0, stats_buf_info[j].buf_size);
          stats_data->stats_type = i;
          stats_data->stats_buf = stats_buf_info[j].addr;
          stats_data->buf_size = stats_buf_info[j].buf_size;
          stats_data->buf_idx = j;
          stats_buf_info[j].buf_state = ISP_BUF_STATE_USED;
          ISP_DBG("get_buf stats_type %d idx %d addr %p state %d",
            stats_data->stats_type, stats_data->buf_idx, stats_data->stats_buf,
            stats_buf_info[j].buf_state);
          break;
        }
      }
      if (j >= MAX_STATS_BUFFERS) {
        ISP_ERR("failed: to get buf for stats type %d", i);
        ret = FALSE;
        break;
      }
    }
  }

  if (ret == FALSE) {
    for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
      stats_data = &stats_event_data->stats_data[i];
      if (stats_data->stats_buf) {
        /* Return buffer*/
        stats_buf_info = buf_mgr->stats_buf_info[stats_data->stats_type];
        for (j = 0; j < MAX_STATS_BUFFERS; j++) {
          if (stats_data->stats_buf == stats_buf_info[j].addr) {
            stats_buf_info[j].buf_state = ISP_BUF_STATE_FREE;
            memset(stats_buf_info[j].addr, 0, stats_buf_info[j].buf_size);
            break;
          }
        }
        if (j >= MAX_STATS_BUFFERS) {
          ISP_ERR("failed: to put buf %p of stats type %d",
            stats_data->stats_buf, i);
        }
      }
    }
    /* Return stats_event_data */
    for (i = 0; i < buf_mgr->num_stats_event_data; i++) {
      if (stats_event_data == &buf_mgr->stats_event_data[i]) {
        buf_mgr->stats_event_data_state[i] = ISP_BUF_STATE_FREE;
        break;
      }
    }
    if (i >= buf_mgr->num_stats_event_data) {
      ISP_ERR("failed: to put stats_event_data %p", stats_event_data);
    }
    stats_event_data = NULL;
  }

  PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
  return stats_event_data;
} /* isp_stats_buf_mgr_get_buf */

/** isp_stats_buf_mgr_put_buf:
 *
 *  @buf_mgr: buffer manager handle
 *  @stats_event_data: 3a event payload
 *
 *  Put buffer back to buffer manager
 *
 *  Return buffer address on success and NULL on failure
 **/
boolean isp_stats_buf_mgr_put_buf(isp_stats_buf_mgr_t *buf_mgr,
  mct_event_stats_isp_t *stats_event_data)
{
  uint32_t                    i = 0,
                              j = 0;
  isp_stats_buf_info_t       *stats_buf_info = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;

  if (!buf_mgr || !stats_event_data) {
    ISP_ERR("failed: buf_mgr %p stats_event_data %p", buf_mgr,
      stats_event_data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&buf_mgr->mutex);
  if (buf_mgr->init_done == FALSE) {
    ISP_ERR("failed: put buf called without buf mgr initialization");
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return FALSE;
  }

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    stats_data = &stats_event_data->stats_data[i];
    if (stats_data->stats_buf) {
      /* Return buffer*/
      stats_buf_info = buf_mgr->stats_buf_info[stats_data->stats_type];
      for (j = 0; j < MAX_STATS_BUFFERS; j++) {
        if (stats_data->stats_buf == stats_buf_info[j].addr) {
          if (stats_buf_info[j].buf_state != ISP_BUF_STATE_USED) {
            ISP_ERR("error: invalid state %d for buf %p of stats type %d",
              stats_buf_info[j].buf_state, stats_data->stats_buf, i);
          }
          stats_buf_info[j].buf_state = ISP_BUF_STATE_FREE;
          ISP_DBG("put_buf stats_type %d idx %d addr %p state %d",
            stats_data->stats_type, stats_data->buf_idx, stats_data->stats_buf,
            stats_buf_info[j].buf_state);
          break;
        }
      }
      if (j >= MAX_STATS_BUFFERS) {
        ISP_ERR("failed: to put buf %p of stats type %d",
          stats_data->stats_buf, i);
      }
    }
  }
  /* Return stats_event_data */
  for (i = 0; i < buf_mgr->num_stats_event_data; i++) {
    if (stats_event_data == &buf_mgr->stats_event_data[i]) {
      if (buf_mgr->stats_event_data_state[i] != ISP_BUF_STATE_USED) {
        ISP_ERR("error: invalid state %d for stats_event_data %p",
          buf_mgr->stats_event_data_state[i], stats_event_data);
      }
      buf_mgr->stats_event_data_state[i] = ISP_BUF_STATE_FREE;
      ISP_DBG("put_buf stats_event_data %p state %d", stats_event_data,
        buf_mgr->stats_event_data_state[i]);
      break;
    }
  }
  if (i >= buf_mgr->num_stats_event_data) {
    ISP_ERR("failed: to put stats_event_data %p", stats_event_data);
  }

  PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
  return TRUE;
} /* isp_stats_buf_mgr_put_buf */

/** isp_bundled_stats_buf_init:
 *
 *  @buf_mgr: buffer manager handle
 *
 *  Initialize buffer manager
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_bundled_stats_buf_init(isp_bundled_stats_buf_mgr_t *buf_mgr)
{
  uint32_t i;

  if (!buf_mgr) {
    ISP_ERR("failed: %p", buf_mgr);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&buf_mgr->mutex);
  if (buf_mgr->init_done == TRUE) {
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return TRUE;
  }

  memset(buf_mgr, 0, sizeof(isp_bundled_stats_buf_mgr_t));
  buf_mgr->bundled_stats_buf_info =
    calloc(MAX_STATS_CONFIG_BUF, sizeof(isp_bundled_stats_buf_info_params_t));
  if (!buf_mgr->bundled_stats_buf_info) {
    ISP_ERR("failed: no memory");
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return FALSE;
  }
  for (i = 0; i < MAX_STATS_CONFIG_BUF; i++) {
    buf_mgr->bundled_stats_buf_info[i].buf_state = ISP_BUNDLED_BUF_STATE_FREE;
  }
  buf_mgr->init_done = TRUE;
  ISP_DBG("Init done\n");

  PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
  return TRUE;
}

/** isp_bundled_stats_buf_deinit:
 *
 * @buf_mgr: buffer manager handle
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean isp_bundled_stats_buf_deinit(isp_bundled_stats_buf_mgr_t *buf_mgr)
{
  boolean ret = TRUE;
  uint32_t i;

  if (!buf_mgr) {
    ISP_ERR("failed: %p", buf_mgr);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&buf_mgr->mutex);
  if (buf_mgr->init_done == FALSE) {
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return TRUE;
  }

  free(buf_mgr->bundled_stats_buf_info);
  buf_mgr->init_done = FALSE;
  PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
  return ret;
}

/** isp_bundled_stats_buf_mgr_get_buf:
 *
 *  @buf_mgr: buffer manager handle
 *
 *  Get free buffer from queue
 *
 * Return buffers on success and NULL on failure
 **/
isp_bundled_stats_buf_info_params_t *isp_bundled_stats_buf_mgr_get_buf(
  isp_bundled_stats_buf_mgr_t *buf_mgr)
{
  uint32_t i;
  isp_bundled_stats_buf_info_params_t *bundled_stats_buf_info = NULL;

  if (!buf_mgr) {
    ISP_ERR("failed: buf_mgr %p", buf_mgr);
    return NULL;
  }
  PTHREAD_MUTEX_LOCK(&buf_mgr->mutex);
  if (buf_mgr->init_done == FALSE) {
    ISP_ERR("failed: get buf called without buf mgr initialization");
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return NULL;
  }
  for (i = 0; i < MAX_STATS_CONFIG_BUF; i++) {
    bundled_stats_buf_info = &buf_mgr->bundled_stats_buf_info[i];
    if (!bundled_stats_buf_info) {
      ISP_ERR("failed: bundled stats buffer initialization failure\n");
      break;
    }
    if (bundled_stats_buf_info->buf_state == ISP_BUNDLED_BUF_STATE_FREE) {
      bundled_stats_buf_info->buf_state = ISP_BUNDLED_BUF_STATE_USED;
      ISP_DBG("get_bundled stats buf %p state %d", bundled_stats_buf_info,
        bundled_stats_buf_info->buf_state);
      break;
    }
  }

  if (i == MAX_STATS_CONFIG_BUF) {
    ISP_ERR("failed: cannot find free bundled stats buffer\n");
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
    return NULL;
  }

  PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
  return bundled_stats_buf_info;
}

/** isp_bundled_stats_buf_mgr_put_buf:
 *
 *  @buf_mgr: buffer manager handle
 *
 *  Put buffer back to buffer manager
 **/
boolean isp_bundled_stats_buf_mgr_put_buf(isp_bundled_stats_buf_mgr_t *buf_mgr,
  isp_bundled_stats_buf_info_params_t *bundled_stats_info)
{
  if (!buf_mgr) {
    ISP_ERR("failed: buf_mgr %p", buf_mgr);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&buf_mgr->mutex);
  if (buf_mgr->init_done == FALSE) {
    ISP_ERR("failed: get buf called without buf mgr initialization");
    PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
     return FALSE;
  }
  if (bundled_stats_info->buf_state != ISP_BUNDLED_BUF_STATE_USED) {
    ISP_ERR("error: invalid state %d for bundled_stats_buf_info %p",
      bundled_stats_info->buf_state, bundled_stats_info);
  } else {
    bundled_stats_info->buf_state = ISP_BUNDLED_BUF_STATE_FREE;
    ISP_DBG("put bundled_stats_buf_info %p state %d", bundled_stats_info,
      bundled_stats_info->buf_state);
  }

  PTHREAD_MUTEX_UNLOCK(&buf_mgr->mutex);
  return TRUE;
}
