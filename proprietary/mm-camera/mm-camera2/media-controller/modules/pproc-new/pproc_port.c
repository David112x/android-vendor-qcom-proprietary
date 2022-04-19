/****************************************************************************
* Copyright (c) 2013-2019 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                      *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                *
****************************************************************************/

#include "camera_dbg.h"
#include "cam_intf.h"
#include "cam_types.h"
#include "modules.h"
#include "mct_list.h"
#include "mct_module.h"
#include "mct_port.h"
#include "pproc_module.h"
#include "pproc_port.h"
#include "mct_controller.h"
#include "chromatix_metadata.h"
#include "module_imglib.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if 0
#define DEBUG_PPROC_PORT
#ifdef DEBUG_PPROC_PORT
#undef PP_LOW
#define PP_LOW PP_ERR
#endif
#endif

//#define PPROC_ENABLE_UNPROC_DIV_PREV_ONLY
#define PPROC_METADATA_QUEUE_SIZE 8

static int num_stream_con = 0;

typedef enum {
  PPROC_PORT_RESERVE_OK,
  PPROC_PORT_RESERVE_FAILED,
  PPROC_PORT_RESERVE_NEW_SESSION,
  PPROC_PORT_RESERVE_INVALID
} pproc_port_reserve_result_t;

typedef enum {
  PPROC_PORT_STATE_UNRESERVED,
  PPROC_PORT_STATE_RESERVED,
  PPROC_PORT_STATE_LINKED,
} pproc_port_state_t;

typedef enum _pproc_port_type {
  PPROC_PORT_TYPE_STREAM,
  PPROC_PORT_TYPE_CAPTURE,
  PPROC_PORT_TYPE_PARAM,
  PPROC_PORT_TYPE_INVALID
} pproc_port_type_t;

typedef struct _pproc_port_metadata_queue_entry {
  meta_data_container *entry;
  boolean used;
} pproc_port_metadata_queue_entry;

/** _pproc_port_stream_info:
 *    @state:              state of port for this stream.
 *    @pproc_stream:       mct_stream object representing topology for
 *                         this stream;
 *    @divert_featue_mask: stores feature mask needed to decide
 *                         divert config.
 *
 *  stream info attached in pproc port
 **/
typedef struct _pproc_port_stream_info {
  pproc_port_state_t state;
  mct_stream_t      *pproc_stream;
  mct_stream_info_t *stream_info;
  mct_port_t        *int_link;
  uint32_t           meta_frame_count;
} pproc_port_stream_info_t;

/** _pproc_port_private:
 *    @streams:            pproc port's attached stream information.
 *    @port_type:          pproc port type attached based on stream;
 *    @num_streams:        total streams attached on this port.
 *    @sessionid:          session id attached on this port.
 *
 *  pproc port's private data structure
 **/
typedef struct _pproc_port_private {
  pproc_port_stream_info_t streams[PPROC_MAX_STREAM_PER_PORT];
  pproc_port_metadata_queue_entry metadata_list[PPROC_METADATA_QUEUE_SIZE];
  pthread_mutex_t          metadata_list_lock;
  pproc_port_type_t        port_type;
  uint32_t                 num_streams;
  uint32_t                 sessionid;
} pproc_port_private_t;

/** _pproc_port_match_data:
 *    @stream_info:     pproc port's attached stream information.
 *    @port:   pproc port type attached based on stream;
 *
 *  pproc port
 **/
typedef struct _pproc_port_match_data {
  mct_stream_info_t *stream_info;
  mct_port_t *port;
} pproc_port_match_data_t;


boolean pproc_port_destroy_stream_topology(mct_module_t *pproc,
  mct_port_t *port, pproc_port_stream_info_t *port_stream_info,
  mct_stream_info_t *stream_info);

/** pproc_port_check_identity
 *    @d1: session+stream identity
 *    @d2: session+stream identity
 *
 *  To find out if both identities are matching;
 *  Return TRUE if matches.
 **/
static boolean pproc_port_check_identity(void *d1, void *d2)
{
  unsigned int v1, v2;

  v1 = *((unsigned int *)d1);
  v2 = *((unsigned int *)d2);

  return ((v1 == v2) ? TRUE : FALSE);
}

/** pproc_port_check_identity_in_port
 *    @data1: this port on which identity needs to be
 *            checked
 *    @data2: session+stream identity
 *
 *  To find out if identities is attached in port;
 *
 *  Return TRUE if matches.
 **/
boolean pproc_port_check_identity_in_port(void *data1, void *data2)
{
  boolean       rc = FALSE;
  mct_port_t   *port = (mct_port_t *)data1;
  uint32_t *identity = (uint32_t *)data2;

  PP_LOW("E\n");
  /* check for sanity */
  if (!data1 || !data2) {
    PP_ERR("error data1: %p, data2: %p\n", data1, data2);
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  if (mct_list_find_custom(MCT_OBJECT_CHILDREN(port), identity,
    pproc_port_check_identity) != NULL) {
    rc = TRUE;
  }

  MCT_OBJECT_UNLOCK(port);
  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_reserve_compatible_port
 *    @data1: submods port
 *    @data2: stream attributes used to reserve this port;
 *
 *  To reserve port on module in stream.
 *
 *  Reserve status from submod
 **/
static boolean pproc_port_reserve_compatible_port(void *data1, void *data2)
{
  mct_port_t        *port = (mct_port_t *)data1;
  pproc_port_match_data_t *match_data = (pproc_port_match_data_t *)data2;
  mct_stream_info_t *stream_info;
  mct_port_t *pproc_port;

  if (!port || !match_data) {
    PP_ERR("error port: %p match_data: %p\n", port, match_data);
    return FALSE;
  }

  stream_info = match_data->stream_info;
  pproc_port = match_data->port;
  /* Adopt the same logic as mct_port_check_link to avoid different cont. port mapped to
      same submod port */
  if (port->peer != NULL && port->peer != pproc_port) {
    return FALSE;
  }

  /* For SINK port need to make sure streams from same SOURCE port of
     upstream module should be connected to same submodule port */
  if (pproc_port->direction == MCT_PORT_SINK) {
    int32_t i = 0;
    pproc_port_private_t *port_private =
      (pproc_port_private_t *)MCT_OBJECT_PRIVATE(pproc_port);
    if (port_private->num_streams > 1) {
      for (i = 0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
        if ((port_private->streams[i].state != PPROC_PORT_STATE_UNRESERVED) &&
          (port_private->streams[i].stream_info != stream_info)) {
          if (port_private->streams[i].int_link != port)
            return FALSE;
          else
            /* Found a reserved internal link which match the current port */
            break;
        }
      }
    }
  }

  return port->check_caps_reserve(port, &pproc_port->caps, stream_info);
}

/** pproc_port_resrv_port_on_module
 *    @submod:      pproc's submodule in stream
 *    @stream_info: stream attributes used to reserve the port;
 *
 *  To request and reserve port on sub module in stream.
 *
 *  Return port from sub module if success
 **/
mct_port_t *pproc_port_resrv_port_on_module(mct_module_t *submod,
  mct_stream_info_t *stream_info, mct_port_direction_t direction,
  mct_port_t *pproc_port)
{
  mct_port_t *sub_port = NULL;
  boolean     rc = FALSE;
  mct_list_t *port_holder;
  mct_list_t *submod_port_holder;
  pproc_port_match_data_t port_match_data;

  PP_LOW("E\n");
  if (!submod || !stream_info) {
    PP_ERR("error submod: %p, stream_info: %p\n", submod, stream_info);
    return NULL;
  }

  submod_port_holder = MCT_MODULE_SRCPORTS(submod);
  if (direction == MCT_PORT_SINK) {
    submod_port_holder = MCT_MODULE_SINKPORTS(submod);
  }

  /* traverse through the allowed ports in the module, trying to find a
     compatible port */
  port_match_data.stream_info = stream_info;
  port_match_data.port = pproc_port;
  port_holder = mct_list_find_custom(submod_port_holder, &port_match_data,
    pproc_port_reserve_compatible_port);

  if (!port_holder) {
    if (submod->request_new_port) {
      sub_port = submod->request_new_port(stream_info, direction, submod, NULL);

      if (!sub_port) {
        PP_ERR("error creating submod sink port\n");
      }
    }
  } else {
    sub_port = (mct_port_t *)port_holder->data;
  }

  PP_LOW("X sub_port: %p\n", sub_port);
  return sub_port;
}

/** pproc_port_get_reserved_port
 *    @module: pproc module
 *    @identity: identity for which attached stream info will be
 *             returned
 *
 *  Return pproc sink port for this module
 *
 *  Return SUCCESS: attached port
 *         FAILURE: NULL
 **/
mct_port_t *pproc_port_get_reserved_port(mct_module_t *module,
  uint32_t identity)
{
  mct_list_t *p_list = NULL;

  /* Validate input parameters */
  if (!module) {
    PP_ERR("failed: module NULL\n");
    goto ERROR;
  }
  /* Lock module mutex */
  MCT_OBJECT_LOCK(module);

  /* Pick pproc sink port for this identity */
  p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &identity, pproc_port_check_identity_in_port);

  /* Unlock module mutex */
  MCT_OBJECT_UNLOCK(module);

  if (!p_list) {
    PP_ERR("error no matching sink port found\n");
    goto ERROR;
  }

  /* Return attached port */
  return p_list->data;
ERROR:
  /* Error - return NULL */
  return NULL;
}

/** pproc_port_get_attached_stream_info
 *    @module: pproc module
 *    @identity: identity for which attached stream info will be
 *             returned
 *
 *  1) Find pproc sink port for this module
 *  2) Find pproc port private for the sink port
 *  3) Match stream id and extract stream info from port private
 *
 *  Return SUCCESS: attached stream info
 *         FAILURE: NULL
 **/
mct_stream_info_t *pproc_port_get_attached_stream_info(mct_port_t *port,
  uint32_t identity)
{
  boolean               rc = TRUE;
  uint32_t              i = 0;
  mct_stream_info_t    *stream_info = NULL;
  pproc_port_private_t *port_private = NULL;

  /* Validate input parameters */
  if (!port) {
    PP_ERR("failed: port NULL\n");
    goto ERROR;
  }

  /* Lock port mutex */
  MCT_OBJECT_LOCK(port);

  /* Pick port private */
  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  if (!port_private) {
    PP_ERR("failed: port private NULL\n");
    /* Unlock port mutex */
    MCT_OBJECT_UNLOCK(port);
    goto ERROR;
  }

  /* Find stream info from port private */
  for (i = 0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
    if (port_private->streams[i].stream_info &&
       (port_private->streams[i].stream_info->identity == identity)) {
      stream_info = port_private->streams[i].stream_info;
    }
  }

  /* Unlock module mutex */
  MCT_OBJECT_UNLOCK(port);

  /* Return stream info */
  return stream_info;
ERROR:
  /* Error - return NULL */
  return NULL;
}

/** pproc_port_match_module_type
 *    @stream: pproc first submodule in stream
 *
 *  To match module based on module type.
 *
 *  Return success if module type matches
 **/
static boolean pproc_port_match_module_type(void *data1, void *data2)
{
  mct_module_t      *module = (mct_module_t *)data1;
  mct_module_type_identity_t *mod_type =
    (mct_module_type_identity_t *)data2;

  PP_LOW("E\n");
  if (!module || !mod_type) {
    PP_ERR("error module: %p, mod_type: %p\n", module, mod_type);
    return FALSE;
  }

  if ((mct_module_find_type(module, mod_type->identity)) == mod_type->type) {
    PP_LOW("X\n");
    return TRUE;
  }

  PP_LOW("X\n");
  return FALSE;
}

static boolean pproc_port_add_modules_to_stream(
  pproc_port_stream_info_t *port_stream_info, mct_module_t **submodarr,
  uint32_t num_submods, mct_stream_info_t *stream_info, mct_port_t *port)
{
  boolean       rc = FALSE;
  uint32_t      i = 0, j = 0;
  mct_module_type_t module_type = MCT_MODULE_FLAG_SINK;

  PP_LOW("E\n");

  if ((NULL == submodarr) || (num_submods > PPROC_MAX_SUBMODS) ||
    (num_submods <= 0) || (NULL == submodarr[num_submods-1])) {
    PP_ERR("error, invalid submodarr:%p, num_submods:%d\n",
      submodarr, num_submods);
    return rc;
  }

  /* To add modules to a stream
     1. Need to set the module type
     2. Reserve SINK port on the first module and link it to pproc int_link
     3. Loop through the modules to link them together */

  /* Set the last module as SINK */
  submodarr[num_submods-1]->set_mod(submodarr[num_submods-1],
    MCT_MODULE_FLAG_SINK, stream_info->identity);
  /* If there are more than one submodule then set the appropriate type */
  if (num_submods > 1) {
    for (i = num_submods - 2; i > 0; i--) {
      if (submodarr[i]) {
        submodarr[i]->set_mod(submodarr[i], MCT_MODULE_FLAG_INDEXABLE,
          stream_info->identity);
      } else {
        PP_ERR("submodarr[%d] is NULL, X rc: %d\n", i, rc);
        /* Remove the module types */
        for (j = i + 1; j < num_submods; j++) {
          submodarr[j]->set_mod(submodarr[j], MCT_MODULE_FLAG_INVALID,
            stream_info->identity);
        }
        return rc;
      }
    }
    if (submodarr[i]) {
      submodarr[i]->set_mod(submodarr[i], MCT_MODULE_FLAG_SOURCE,
        stream_info->identity);
    } else {
      PP_ERR("submodarr[%d] is NULL, X rc: %d\n", i, rc);
      for (j = i + 1; j < num_submods; j++) {
        submodarr[j]->set_mod(submodarr[j], MCT_MODULE_FLAG_INVALID,
          stream_info->identity);
      }
      return rc;
    }
  }

  /* Reserve sink port on first module */
  port_stream_info->int_link = pproc_port_resrv_port_on_module(submodarr[0],
    stream_info, MCT_PORT_SINK, port);
  PP_INFO("in identity %x stream %d int_link = %p", stream_info->identity,
    stream_info->stream_type,
    port_stream_info->int_link);
  if (port_stream_info->int_link) {
    /* Invoke ext link for the submod port */
    port_stream_info->int_link->ext_link(stream_info->identity,
      port_stream_info->int_link, port);
    rc = mct_port_add_child(stream_info->identity,
      port_stream_info->int_link);
    if (rc == TRUE) {
      if (num_submods > 1) {
        for (i = 0; i < num_submods-1; i++) {
          PP_INFO(":LINK linking mods %s and %s for identity %x",
            MCT_OBJECT_NAME(submodarr[i]), MCT_OBJECT_NAME(submodarr[i+1]),
            stream_info->identity);
          /* Loop through rest of the modules to link them together */
          rc = mct_stream_link_modules(port_stream_info->pproc_stream,
            submodarr[i], submodarr[i+1], NULL);
          if (rc == FALSE) {
            PP_ERR("error, link module failed\n");
            break;
          }
        }
      } else {
        /* Just one submodule */
        rc = mct_object_set_parent(MCT_OBJECT_CAST(submodarr[0]),
          MCT_OBJECT_CAST(port_stream_info->pproc_stream));
      }
    } else {
      PP_ERR("error adding child\n");
    }
  }

  /* TODO: Under error need to clean up the stream and reserve/links */
  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_create_stream_topology
 *    @pproc:            pproc module
 *    @port:             this port where topology is created.
 *    @port_stream_info: pproc port stream info structure to
 *                       store stream & related attributes.
 *    @stream_info:      stream attributes used to reserve this
 *                       port;
 *
 *  To create a topoloty based on stream's information.
 *
 *  cpp or vpe is a must module for pproc. Need to check stream
 *  attributes to identify if other sub-modules are needed.
 *
 *  Return TRUE if topology stream is created.
 **/
static boolean pproc_port_create_stream_topology(mct_module_t *pproc,
  mct_port_t *port, pproc_port_stream_info_t *port_stream_info,
  mct_stream_info_t *stream_info)
{
  mct_module_t *submod1 = NULL, *submod2 = NULL, *ops_submod = NULL;
  mct_module_t *cac = NULL, *wnr = NULL, *cpp = NULL, *vpe = NULL, *c2d = NULL;
  mct_module_t *vpu = NULL;
  mct_module_t *tmod = NULL;
  mct_module_t *tmod2 = NULL;
  mct_module_t *eztune = NULL;
  mct_module_t *afs = NULL;
  mct_module_t *llvd = NULL;
  mct_module_t *sw_tnr = NULL;
  mct_module_t *svhdr = NULL;
  mct_module_t *bincorr = NULL;
  mct_module_t *ppeiscore = NULL;
  mct_module_t *thirdpartyeis_p = NULL;
  mct_module_t *thirdpartyeis_v = NULL;
  mct_module_t *quadracfa = NULL;
  mct_module_t *sat = NULL;
  mct_module_t *sac = NULL;
  mct_module_t *rtbdm = NULL;
  mct_module_t *rtb = NULL;
  mct_module_t *depth_map = NULL;
  mct_module_t *mfproc_precpp = NULL;
  int enabled = FALSE;
  boolean       rc = TRUE;
  boolean  is_preview_video = FALSE;
  uint32_t      num_submods = 0;
  mct_module_t *submodarr[PPROC_MAX_SUBMODS];
  char value[PROPERTY_VALUE_MAX];
  const mct_pipeline_session_data_t *session_data;
  const cam_stream_size_info_t *meta_stream_info = NULL;
  bool llvd_before_tmod = false;
  uint32_t eis_post_cpp = 0;
  uint32_t rtb_post_cpp = 0;
  uint32_t thirdpartyeis_enabled = 0;

  PP_LOW("E\n");
  port_stream_info->pproc_stream =
    mct_stream_new(stream_info->identity & 0x0000FFFF);

  property_get("persist.vendor.camera.pproc.eispostcpp", value, "0");
  eis_post_cpp  = atoi(value);

  rtb = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "rtb");
  if (rtb) {
    bool is_qc_sol = false;
    module_qdc_common_check_lib_flavour(rtb, &is_qc_sol);
    if (is_qc_sol == true) {
      property_get("persist.vendor.camera.rtb.after_cpp", value, "1");
      rtb_post_cpp  = atoi(value);
    } else  {
      property_get("persist.vendor.camera.rtb.after_cpp", value, "0");
      rtb_post_cpp  = atoi(value);
    }
  }

  property_get("persist.vendor.camera.thirdpartyeis_enabled", value, "0");
  thirdpartyeis_enabled = atoi(value);

  if (port_stream_info->pproc_stream == NULL) {
    PP_ERR("error in stream creation\n");
    return FALSE;
  }

  port_stream_info->pproc_stream->streaminfo = *stream_info;
  if (!stream_info->stream) {
    PP_ERR("Invalid stream_info->stream for iden:0x%x\n", stream_info->identity);
    return FALSE;
  }
  port_stream_info->pproc_stream->buffers = stream_info->stream->buffers;

  port_stream_info->pproc_stream->streaminfo.stream =
    port_stream_info->pproc_stream;
  memset(submodarr, 0, sizeof(submodarr));

  session_data = mct_pipeline_lookup_session_data(pproc,
    ((stream_info->identity & 0xFFFF0000) >> 16));
  if (session_data) {
    meta_stream_info = &(session_data->streamConfigInfo);
  }
  eztune = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "ezt");

  tmod = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "tmod");
  if (tmod) {
    submodarr[num_submods++] = tmod;
  }

  if (eis_post_cpp) {
     tmod2 =pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "tmod2");
     if (!tmod2) {
       PP_ERR("tmod2 failed");
     }
  }

  if (!(port->caps.u.frame.format_flag & MCT_PORT_CAP_INTERLEAVED)) {
    if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QCOM_FEATURE_PAAF) || (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
      afs = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "paaf");
      if (afs) {
        submodarr[num_submods++] = afs;
      }
    }
  }

  if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QTI_FEATURE_SAT) || (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
    sat = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "sat");
    if (sat) {
      PP_DBG("SAT added to sub mod array");
      submodarr[num_submods++] = sat;
    }
  }

  if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QTI_FEATURE_SAC) || (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
    sac = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "sac");
    if (sac) {
      PP_DBG("SAC added to sub mod array");
      submodarr[num_submods++] = sac;
    }
  }
  if (rtb_post_cpp == 0) {
    if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
      CAM_QTI_FEATURE_RTB) || (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
      rtb = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "rtb");
      if (rtb) {
        PP_DBG("RTB added to sub mod array");
        submodarr[num_submods++] = rtb;
      }
    }
  }

  if (((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QTI_FEATURE_MFPROC_PRECPP) ||
    (port_stream_info->pproc_stream->streaminfo.reprocess_config.
    pp_feature_config.feature_mask & CAM_QTI_FEATURE_MFPROC_PRECPP)) ||
    (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
      mfproc_precpp = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,
        "mfproc_precpp");
      PP_DBG("pproc creating mfproc_precpp module %0x \n", mfproc_precpp);
  }

  if((CAM_STREAM_TYPE_PREVIEW == stream_info->stream_type) ||
    (CAM_STREAM_TYPE_VIDEO == stream_info->stream_type)) {
    num_stream_con ++;
    is_preview_video = TRUE;
  }

  property_get("persist.vendor.camera.feature.bincorr", value, "0");
  enabled = atoi(value);
  /*If binning correction is enabled, disable svhdr*/
  if (!enabled) {
      if(((num_stream_con == 1) && (is_preview_video))||
        (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
        svhdr = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,
          "bayer_proc_svhdr");
        if(svhdr) {
          PP_HIGH("svhdr lib loading successful");
          submodarr[num_submods++] = svhdr;
        } else {
          PP_HIGH("svhdr NULL, failed to load lib");
        }
      }
  } else {
    if ((CAM_STREAM_TYPE_PREVIEW == stream_info->stream_type) ||
      (CAM_STREAM_TYPE_VIDEO == stream_info->stream_type) ||
      (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
        bincorr = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,
          "bayer_proc_bincorr");
      if (bincorr) {
        PP_HIGH("binning correction lib loading successful");
        submodarr[num_submods++] = bincorr;
      } else {
        PP_HIGH("binning correction NULL, failed to load lib");
      }
    }
  }

  cpp = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "cpp");
  submod1 = cpp;
  if (!submod1) {
    vpe = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,"vpe");
    submod1 = vpe;
  }

  if (thirdpartyeis_enabled) {
    thirdpartyeis_p = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,"thirdpartyeis_p");
    thirdpartyeis_v = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,"thirdpartyeis_v");
  }
  else {
    ppeiscore = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,"ppeiscore");
  }

  c2d = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,"c2d");
  if (!submod1) {
    submod1 = c2d;
  }

  if (!submod1) {
    PP_ERR("error because both cpp/vpe cannot be NULL\n");
    rc = FALSE;
    goto create_topology_done;
  }
  vpu = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "vpu");

  if (CAM_STREAM_TYPE_PARM == stream_info->stream_type) {
    if (vpu)
      submodarr[num_submods++] = vpu;
    if (thirdpartyeis_enabled) {
      if (thirdpartyeis_p)
        submodarr[num_submods++] = thirdpartyeis_p;
      if (thirdpartyeis_v)
        submodarr[num_submods++] = thirdpartyeis_v;
    }
    else if (ppeiscore)
      submodarr[num_submods++] = ppeiscore;
    if (c2d)
      submodarr[num_submods++] = c2d;
    if (cpp)
      submodarr[num_submods++] = cpp;
  }

  PP_HIGH("feature mask %llx %llx",
    port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask,
    port_stream_info->pproc_stream->streaminfo.reprocess_config.
    pp_feature_config.feature_mask);
  if (((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QCOM_FEATURE_CAC) || (CAM_STREAM_TYPE_PARM == stream_info->stream_type) ||
    (CAM_STREAM_TYPE_PREVIEW == stream_info->stream_type) ||
    (CAM_STREAM_TYPE_CALLBACK == stream_info->stream_type) ||
    (CAM_STREAM_TYPE_VIDEO == stream_info->stream_type) ||
    ((port_stream_info->pproc_stream->streaminfo.reprocess_config.
    pp_feature_config.feature_mask & CAM_QCOM_FEATURE_CAC))) &&
    (CAM_STREAM_TYPE_ANALYSIS != stream_info->stream_type)) {
    cac = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "cac");
    if (cac) {
      submodarr[num_submods++] = cac;
    }
  }
  if (((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QCOM_FEATURE_DENOISE2D) || (CAM_STREAM_TYPE_PARM == stream_info->stream_type) ||
    ((port_stream_info->pproc_stream->streaminfo.reprocess_config.
    pp_feature_config.feature_mask & CAM_QCOM_FEATURE_DENOISE2D))) &&
    (CAM_STREAM_TYPE_ANALYSIS != stream_info->stream_type)) {
    /* This is based on the assumption that sw-wnr is available only when
       enabled based on target HW */
    wnr = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "wnr");
    if (wnr) {
      submodarr[num_submods++] = wnr;
    }
  }

  if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QCOM_FEATURE_LLVD) ||
    (CAM_STREAM_TYPE_PARM == stream_info->stream_type) ||
    (port_stream_info->pproc_stream->streaminfo.reprocess_config.
    pp_feature_config.feature_mask & CAM_QCOM_FEATURE_LLVD)) {
    llvd = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "llvd");
  }

  if (((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QCOM_FEATURE_QUADRA_CFA) ||
    (port_stream_info->pproc_stream->streaminfo.reprocess_config.
    pp_feature_config.feature_mask & CAM_QCOM_FEATURE_QUADRA_CFA))||
    (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
    quadracfa = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data,
      "quadracfa");
    PP_DBG("pproc creating quadracfa module created %0x \n", quadracfa);
  }

  if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
      CAM_QTI_FEATURE_SW_TNR) || (CAM_STREAM_TYPE_PARM == stream_info->stream_type) ||
      ((port_stream_info->pproc_stream->streaminfo.reprocess_config.
      pp_feature_config.feature_mask & CAM_QTI_FEATURE_SW_TNR)) ||
      (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
    sw_tnr = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "sw_tnr");
    if (sw_tnr) {
      submodarr[num_submods++] = sw_tnr;
    }
  }

  if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QTI_FEATURE_RTBDM) || (CAM_STREAM_TYPE_PARM == stream_info->stream_type)) {
    rtbdm = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "rtbdm");
    if (rtbdm) {
      PP_DBG("RTBDM added to sub mod array");
      submodarr[num_submods++] = rtbdm;
    }
  }

   if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
    CAM_QTI_FEATURE_DEPTH_MAP) ||
    ((CAM_STREAM_TYPE_DEPTH == stream_info->stream_type) ||
     (CAM_STREAM_TYPE_PARM == stream_info->stream_type))) {
    depth_map = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "depth_map");
    if (depth_map) {
      PP_DBG("DEPTH_MAP added to sub mod array");
      submodarr[num_submods++] = depth_map;
    } else {
      PP_ERR("Error: DEPTH_MAP Not added to sub mod array");
    }
  }

  if (CAM_STREAM_TYPE_OFFLINE_PROC != stream_info->stream_type) {
    ops_submod = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "sample");
    if (ops_submod) {
      submodarr[num_submods++] = ops_submod;
    }
  }

  if (rtb_post_cpp != 0) {
    if (CAM_STREAM_TYPE_PARM == stream_info->stream_type) {
      rtb = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "rtb");
      if (rtb) {
        PP_DBG("RTB added to sub mod array");
        submodarr[num_submods++] = rtb;
      }
    }
  }

  port_stream_info->int_link = NULL;

  /* If this is a single module stream, just need to get its sink port;
   * otherwise, need to determin which module needs to be added and use
   * mct_stream_link_modules to link them
   * 1. Set the correct module type per identity
   * 2. Request and reserve the sink port on first module
   * 3. Set the internal link
   * 4. Add identity as child to sink port
   * 5. Add the modules to stream and link them if needed.
   * 6. If applicable set the divert information */
  switch (stream_info->stream_type) {
  case CAM_STREAM_TYPE_DEPTH: {
    PP_DBG("stream type CAM_STREAM_TYPE_DEPTH");
    rc = pproc_port_add_modules_to_stream(port_stream_info, &submodarr[0],
      num_submods, stream_info, port);
  }
  break;
  case CAM_STREAM_TYPE_ANALYSIS: {
    rc = pproc_port_add_modules_to_stream(port_stream_info, &submodarr[0],
      num_submods, stream_info, port);
  }
    break;
  case CAM_STREAM_TYPE_POSTVIEW: {
    if((port->caps.u.frame.format_flag & MCT_PORT_CAP_INTERLEAVED)) {
      if (c2d)
        submodarr[num_submods++] = c2d;
      submodarr[num_submods++] = submod1;
    } else {
      submodarr[num_submods++] = submod1;
    }
    rc = pproc_port_add_modules_to_stream(port_stream_info, &submodarr[0],
      num_submods, stream_info, port);
  }
    break;
  case CAM_STREAM_TYPE_PARM:
    if (llvd) {
      submodarr[num_submods++] = llvd;
    }
    if (eztune) {
      submodarr[num_submods++] = eztune;
    }
    if (quadracfa) {
      submodarr[num_submods++] = quadracfa;
    }
    if (mfproc_precpp) {
      submodarr[num_submods++] = mfproc_precpp;
    }
    rc = pproc_port_add_modules_to_stream(port_stream_info, &submodarr[0],
      num_submods, stream_info, port);
    break;
  case CAM_STREAM_TYPE_CALLBACK:
  case CAM_STREAM_TYPE_PREVIEW: {
    if (vpu) {
      submodarr[num_submods++] = vpu;
    }

    PP_DBG("eis_post_cpp %d ",eis_post_cpp);
    if ((stream_info->is_type == IS_TYPE_EIS_2_0) ||
       (port->caps.u.frame.format_flag & MCT_PORT_CAP_INTERLEAVED)) {
      IS_LLVD_BEFORE_TMOD(meta_stream_info, stream_info, llvd_before_tmod);
      if (!llvd_before_tmod) {
        if (llvd) {
          submodarr[num_submods++] = llvd;
        }
      } else {
        INSERT_LLVD_AT_TOP(submodarr, num_submods, llvd);
      }
      /*If EIS is connected after cpp in video then don't connect c2d now.
      Connect c2d to tmod2 later in the pipline.
      This use case is for native buffer in cpp */
      if (eis_post_cpp) {
        submodarr[num_submods++] = submod1;
        submodarr[num_submods++] = tmod2;
        if (c2d) {
          submodarr[num_submods++] = c2d;
        }
      } else {
        if (c2d)
          submodarr[num_submods++] = c2d;
          submodarr[num_submods++] = submod1;
        }
    } else if ((stream_info->is_type == IS_TYPE_VENDOR_EIS) ||
       (port->caps.u.frame.format_flag & MCT_PORT_CAP_INTERLEAVED)) {
      IS_LLVD_BEFORE_TMOD(meta_stream_info, stream_info, llvd_before_tmod);
      if (!llvd_before_tmod) {
        if (llvd) {
          submodarr[num_submods++] = llvd;
        }
      } else {
        INSERT_LLVD_AT_TOP(submodarr, num_submods, llvd);
      }
      /*If EIS is connected after cpp in video then don't connect c2d now.
      Connect c2d to tmod2 later in the pipline.
      This use case is for native buffer in cpp */
      if (eis_post_cpp) {
        submodarr[num_submods++] = submod1;
        submodarr[num_submods++] = tmod2;
        if (thirdpartyeis_enabled && thirdpartyeis_p)
          submodarr[num_submods++] = thirdpartyeis_p;
        else if (c2d) {
          submodarr[num_submods++] = c2d;
        }
      } else {
        if (thirdpartyeis_enabled && thirdpartyeis_p)
          submodarr[num_submods++] = thirdpartyeis_p;
        else if (c2d)
          submodarr[num_submods++] = c2d;
        submodarr[num_submods++] = submod1;
      }
    } else {
        if (llvd) {
          submodarr[num_submods++] = llvd;
        }
        submodarr[num_submods++] = submod1;
    }

    if (rtb_post_cpp != 0) {
      if ((port_stream_info->pproc_stream->streaminfo.pp_config.feature_mask &
        CAM_QTI_FEATURE_RTB)) {
        rtb = pproc_module_get_sub_mod(MCT_OBJECT_PARENT(port)->data, "rtb");
        if (rtb) {
          PP_DBG("RTB added to sub mod array");
          submodarr[num_submods++] = rtb;
        }
      }
    }

    if (eztune &&
      (stream_info->pp_config.feature_mask & CAM_QCOM_FEATURE_EZTUNE)) {
      submodarr[num_submods++] = eztune;
    }
    rc = pproc_port_add_modules_to_stream(port_stream_info, &submodarr[0],
      num_submods, stream_info, port);
  }
    break;

  case CAM_STREAM_TYPE_SNAPSHOT: {

    if((port->caps.u.frame.format_flag & MCT_PORT_CAP_INTERLEAVED)) {
    /* In ZSL snapshot, run C2D to convert from YUV422i to NV21 */
      if (stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
        if (c2d)
          submodarr[num_submods++] = c2d;
      } else {
        if (c2d)
          submodarr[num_submods++] = c2d;
        submodarr[num_submods++] = submod1;
        if (eztune &&
          (stream_info->pp_config.feature_mask & CAM_QCOM_FEATURE_EZTUNE)) {
          submodarr[num_submods++] = eztune;
        }
      }
    } else {
      if (submod1 && submod1->query_mod) {
        mct_pipeline_cap_t query_buf;
        memset(&query_buf, 0, sizeof(mct_pipeline_cap_t));
        submod1->query_mod(submod1, &query_buf,
          PPROC_GET_SESSION_ID(stream_info->identity));
        if (query_buf.pp_cap.feature_mask &
          stream_info->pp_config.feature_mask) {
          submodarr[num_submods++] = submod1;
        }
      }
      if (eztune &&
        (stream_info->pp_config.feature_mask & CAM_QCOM_FEATURE_EZTUNE)) {
        submodarr[num_submods++] = eztune;
      }
    }

    rc = pproc_port_add_modules_to_stream(port_stream_info, &submodarr[0],
      num_submods, stream_info, port);
  }
    break;

  case CAM_STREAM_TYPE_VIDEO: {
    if (vpu) {
      submodarr[num_submods++] = vpu;
    }
    if ((stream_info->is_type == IS_TYPE_EIS_2_0) ||
      (port->caps.u.frame.format_flag & MCT_PORT_CAP_INTERLEAVED)) {
      if (llvd) {
        submodarr[num_submods++] = llvd;
      }
      if (eis_post_cpp) {
        submodarr[num_submods++] = submod1;
        submodarr[num_submods++] = tmod2;
        if (c2d)
          submodarr[num_submods++] = c2d;
        } else {
          if (c2d) {
            submodarr[num_submods++] = c2d;
          }
          submodarr[num_submods++] = submod1;
        }
    } else if (stream_info->is_type == IS_TYPE_EIS_3_0) {
        IS_LLVD_BEFORE_TMOD(meta_stream_info, stream_info, llvd_before_tmod);
      if (!llvd_before_tmod) {
        if (llvd) {
          submodarr[num_submods++] = llvd;
        }
      } else {
        INSERT_LLVD_AT_TOP (submodarr, num_submods, llvd);
      }
      if (eis_post_cpp){
        submodarr[num_submods++] = submod1;
        submodarr[num_submods++] = tmod2;
        if (ppeiscore) {
          submodarr[num_submods++] = ppeiscore;
        }
      } else {
        if (ppeiscore) {
          submodarr[num_submods++] = ppeiscore;
        }
        submodarr[num_submods++] = submod1;
      }
    } else if (stream_info->is_type == IS_TYPE_VENDOR_EIS) {
      IS_LLVD_BEFORE_TMOD(meta_stream_info, stream_info, llvd_before_tmod);
      if (!llvd_before_tmod) {
        if (llvd) {
          submodarr[num_submods++] = llvd;
        }
      } else {
        INSERT_LLVD_AT_TOP (submodarr, num_submods, llvd);
      }
      if (eis_post_cpp) {
        submodarr[num_submods++] = submod1;
        submodarr[num_submods++] = tmod2;
        if (thirdpartyeis_enabled && thirdpartyeis_v)
          submodarr[num_submods++] = thirdpartyeis_v;
      } else {
        if (thirdpartyeis_enabled && thirdpartyeis_v)
          submodarr[num_submods++] = thirdpartyeis_v;
      }
      submodarr[num_submods++] = submod1;
    } else {
      if (llvd) {
       submodarr[num_submods++] = llvd;
      }
      submodarr[num_submods++] = submod1;
    }
    rc = pproc_port_add_modules_to_stream(port_stream_info, &submodarr[0],
      num_submods, stream_info, port);
  }
    break;

  case CAM_STREAM_TYPE_OFFLINE_PROC: {
    PP_LOW("feature mask %llx %llx", stream_info->pp_config.feature_mask,
      CAM_QCOM_FEATURE_QUADRA_CFA );
    if (quadracfa) {
      submodarr[num_submods++] = quadracfa;
      PP_LOW("pproc adding quadracfa module to submodarr at idx=%d , submod1"
        " %0x quadracfa %0x\n", num_submods -1, submod1, quadracfa );
    } else {
      if (mfproc_precpp) {
        submodarr[num_submods++] = mfproc_precpp;
        PP_LOW("pproc adding mfproc_precpp module to submodarr at idx=%d , submod1"
          " %0x mfproc_precpp %0x\n", num_submods -1, submod1, mfproc_precpp );
      }
      submodarr[num_submods++] = submod1;
    }

    rc = pproc_port_add_modules_to_stream(port_stream_info, &submodarr[0],
      num_submods, stream_info, port);
  }
    break;

  case CAM_STREAM_TYPE_RAW:
  default:
    /* pproc should not be linked !*/
    PP_ERR("error in pproc link\n");
    rc = FALSE;
    break;
  } /* switch (stream_info->stream_type) */

create_topology_done:
  if (rc == FALSE) {
    if (port_stream_info->int_link) {
      port_stream_info->int_link->check_caps_unreserve(
        port_stream_info->int_link, stream_info->identity);
      port_stream_info->int_link = NULL;
    }
    pproc_port_destroy_stream_topology(pproc, port,
      port_stream_info, stream_info);
  }

  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_destroy_stream_topology
 *    @pproc:            pproc module
 *    @port:             this port where topology is to deleted.
 *    @port_stream_info: pproc port stream info structure to
 *                       store stream & related attributes.
 *    @stream_info:      stream attributes used to reserve this
 *                       port;
 *
 *  To delete a topology based on stream's information.
 *
 *  Return TRUE if topology stream is deleted.
 **/
boolean pproc_port_destroy_stream_topology(mct_module_t *pproc,
  mct_port_t *port, pproc_port_stream_info_t *port_stream_info,
  mct_stream_info_t *stream_info)
{
  mct_port_t   *int_link;
  boolean       rc = TRUE;
  mct_stream_t *stream;
  num_stream_con = 0;

  PP_LOW("E\n");
  if (!pproc || !port || !port_stream_info || !stream_info) {
    PP_ERR("error pproc:%p, port:%p, port_stream_info:%p\n",
      pproc, port, port_stream_info);
    return FALSE;
  }

  int_link = port_stream_info->int_link;
  stream = port_stream_info->pproc_stream;
  /* 1. For the sink port of first module (int_link)in stream do the following,
         a. Do the ext unlink
         b. Do caps unreserve
         c. Remove identity (port's children)
     2. If stream children is more than 2 then operate_unlink on submods
     3. Detach module/stream's child/parent relationship */
  if (port_stream_info->int_link) {
    port_stream_info->int_link->un_link(stream_info->identity, int_link, port);
    rc = port_stream_info->int_link->check_caps_unreserve(int_link,
      stream_info->identity);
    if (rc == FALSE) {
      PP_ERR("error in caps unreserve\n");
      return rc;
    }
    mct_port_remove_child(stream_info->identity, int_link);
    port_stream_info->int_link = NULL;
  }

  if (!stream) {
    PP_ERR("error in deleting pproc stream\n");
    return rc;
  }
  if (MCT_OBJECT_NUM_CHILDREN(stream)) {
    if (MCT_OBJECT_NUM_CHILDREN(stream) > 1) {
      mct_list_operate_nodes(MCT_OBJECT_CHILDREN(stream),
        mct_stream_operate_unlink, stream);
    } else {
      /* Type is removed in unlink modules,since we have only one
       * module it is not linked, remove type here */
      mct_module_t *single_module = MCT_OBJECT_CHILDREN(stream)->data;
      mct_module_remove_type(single_module, stream->streaminfo.identity);
    }
  }


  /* 1. free stream from module's parent list;
     2. free module object from stream's children list */
  mct_list_free_all_on_data(MCT_OBJECT_CHILDREN(stream),
    mct_stream_remove_stream_from_module, stream);
  MCT_OBJECT_CHILDREN(stream) = NULL;
  MCT_STREAM_NUM_CHILDREN(stream) = 0;
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(stream));
  free(port_stream_info->pproc_stream);
  port_stream_info->pproc_stream = NULL;

  PP_LOW("X\n");
  return rc;
}

/** pproc_port_check_port_state
 *    @port:        this pproc module's port;
 *    @stream_info: stream attributes used to reserve this port;
 *
 *  To check the state of port based on stream's information.
 *
 *  If the session ID is different, support can be provided via
 *  create a new port.
 *
 *  Once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return
 *    failure;
 *  - If the requested stream belongs to a different session,
 *    the port can not be used;
 *
 *  Return TRUE if port can be reserved.
 **/
static boolean pproc_port_check_port_state(mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  boolean               rc = TRUE;
  pproc_port_private_t *port_private;

  PP_LOW("E\n");
  if ((port->caps.port_caps_type != MCT_PORT_CAPS_FRAME) ||
    (!(port->caps.u.frame.format_flag & MCT_PORT_CAP_FORMAT_YCBCR) &&
     stream_info->stream_type != CAM_STREAM_TYPE_DEPTH) ||
    (port->caps.u.frame.size_flag != MCT_PORT_CAP_SIZE_20MB)) {
    PP_ERR("error because port_pproc caps is wrong\n");
    rc = FALSE;
    goto check_port_state_done;
  }

  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);

  if(port_private->num_streams >= PPROC_MAX_STREAM_PER_PORT) {
    PP_LOW("info: max streams reached\n");
    rc = FALSE;
    goto check_port_state_done;
  }

  if ((port->direction == MCT_PORT_SRC) && (port_private->num_streams)) {
    rc = FALSE;
    goto check_port_state_done;
  }

  if ((port_private->sessionid) &&
    (port_private->sessionid != (stream_info->identity & 0xFFFF0000))) {
    PP_LOW("info: reserved by session:%d\n", port_private->sessionid);
    rc = FALSE;
    goto check_port_state_done;
  }

  switch(port_private->port_type) {
  case PPROC_PORT_TYPE_INVALID: {
    port_private->num_streams = 0;
    if ((stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) ||
      (stream_info->streaming_mode == CAM_STREAMING_MODE_BATCH)){
        port_private->port_type = PPROC_PORT_TYPE_STREAM;
        if (stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
            PP_LOW("CAM_STREAM_TYPE_PRAM\n");
            port_private->port_type = PPROC_PORT_TYPE_PARAM;
        }
    } else if (stream_info->streaming_mode == CAM_STREAMING_MODE_BURST) {
      port_private->port_type = PPROC_PORT_TYPE_CAPTURE;
    }
  }
    break;

  case PPROC_PORT_TYPE_CAPTURE: {
    mct_stream_info_t *pstreaminfo;
    int32_t i;
    //check if port is already reserved for offline
    for (i = 0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
      pstreaminfo = port_private->streams[i].stream_info;
      if (pstreaminfo &&
        (pstreaminfo->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC)) {
        PP_LOW("info: already reserved for offline:%d\n",
        pstreaminfo->stream_type);
        rc = FALSE;
        break;
      }
    }
  }
    break;
  case PPROC_PORT_TYPE_STREAM: {
    if (stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      PP_LOW("info: streaming mode doesn't match\n");
      rc = FALSE;
    }
  }
    break;
  case PPROC_PORT_TYPE_PARAM:
    PP_LOW("PPROC_PORT_TYPE_PARM\n");
    rc = FALSE;
    break;
  default:
    PP_ERR("error bad port_type=%d\n", port_private->port_type);
    rc = FALSE;
    break;
  }

check_port_state_done:
  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_sink_check_caps_reserve
 *    @port:        this pproc module's port;
 *    @peer_caps:   the capability of peer port which wants to
 *                  match interface port;
 *    @stream_info: stream attributes used to reserve this port;
 *
 *  To reserve a sink port based on stream's information.
 *
 *  If the session ID is different, support can be provided via
 *  create a new port in case sub-module can do so during
 *  request new port.
 *
 *  Once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return
 *    failure;
 *  - If the requested stream belongs to a different session,
 *    the port can not be used;
 *  - For both sink port and source port, need to handle
 *    internal  cap reserve.
 *
 *  Return TRUE if port can be reserved.
 **/
static boolean pproc_port_sink_check_caps_reserve(mct_port_t *port, void *caps,
  void *info)
{
  uint32_t                         i;
  boolean                     rc = TRUE;
  mct_port_caps_t            *peer_caps;
  pproc_port_private_t       *port_private;
  mct_module_t               *module;
  mct_list_t                 *list;
  pproc_port_reserve_result_t result;
  mct_stream_info_t          *stream_info = (mct_stream_info_t *)info;

  PP_LOW("E\n");


  if (!port || !stream_info ||
      strncmp(MCT_OBJECT_NAME(port), "pproc_sink", strlen("pproc_sink"))) {
    PP_ERR("error because invalid parameters!\n");
    rc = FALSE;
    return rc;
  }
  MCT_OBJECT_LOCK(port);
  peer_caps = (mct_port_caps_t *)caps;

  if (peer_caps && peer_caps->port_caps_type != MCT_PORT_CAPS_FRAME) {
    PP_LOW("error because caps Type:%d not supported.\n",
      peer_caps->port_caps_type);
    rc = FALSE;
    goto reserve_done;
  }

  if ((stream_info->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) && peer_caps)
    port->caps.u.frame.format_flag = peer_caps->u.frame.format_flag;

  if (pproc_port_check_port_state(port, stream_info) == FALSE) {
    rc = FALSE;
    goto reserve_done;
  }


  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  /* reserve the port for this stream */
  for(i=0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
    if(port_private->streams[i].state == PPROC_PORT_STATE_UNRESERVED) {
      port_private->streams[i].state = PPROC_PORT_STATE_RESERVED;
      port_private->streams[i].stream_info = stream_info;
      port_private->streams[i].meta_frame_count = 0;
      port_private->num_streams++;
      rc = pproc_port_create_stream_topology(MCT_PORT_PARENT(port)->data, port,
        &port_private->streams[i], stream_info);

      if (rc == TRUE) {
        port_private->sessionid = stream_info->identity & 0xFFFF0000;
      }

      break;
    }
  }

  if(i == PPROC_MAX_STREAM_PER_PORT) {
    PP_ERR("error, unexpected error!!!");
    /* TODO: need to reset the port_type and streaming_mode and handle
       num_streams */
    rc = FALSE;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_sink_check_caps_unreserve
 *    @port: this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *    Return FALSE if the identity is not existing.
 *
 *  This function unreserves the identity on this port.
 **/
static boolean pproc_port_sink_check_caps_unreserve(mct_port_t *port,
  uint32_t identity)
{
  uint32_t                    i;
  boolean                     rc = TRUE;
  pproc_port_private_t       *port_private;
  mct_module_t               *module;
  mct_list_t                 *list;
  pproc_port_reserve_result_t result;

  PP_LOW("E\n");


  if (!port ||
      strncmp(MCT_OBJECT_NAME(port), "pproc_sink", strlen("pproc_sink"))) {
    PP_ERR("error because invalid parameters!\n");
    rc = FALSE;
    return rc;
  }
  MCT_OBJECT_LOCK(port);

  module = MCT_MODULE_CAST(MCT_PORT_PARENT(port)->data);
  /* 1. Check the identity is mapped
     2. Destroy stream topology
     3. Change stream state to UNRESERVED and decrement num_streams
     4. If num_streams is zero change port_type to invalid & remove session */
  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  for (i=0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
    if ((port_private->streams[i].state == PPROC_PORT_STATE_RESERVED) &&
      (port_private->streams[i].stream_info->identity == identity)) {
      port_private->streams[i].state = PPROC_PORT_STATE_UNRESERVED;
      rc = pproc_port_destroy_stream_topology(MCT_PORT_PARENT(port)->data,
        port, &port_private->streams[i],
        port_private->streams[i].stream_info);

      if (rc == TRUE) {
        port_private->num_streams--;
        if (!port_private->num_streams) {
          port_private->sessionid = 0;
          port_private->port_type = PPROC_PORT_TYPE_INVALID;
        }
        port_private->streams[i].stream_info = NULL;
        port_private->streams[i].meta_frame_count = 0;
      }
      break;
    }
  }

  if(i == PPROC_MAX_STREAM_PER_PORT) {
    PP_ERR("error, unexpected error!!!");
    /* TODO: need to reset the port_type and streaming_mode and handle
       num_streams */
    rc = FALSE;
  }

  MCT_OBJECT_UNLOCK(port);
  /* Because MCT is not calling the set_mod for every stream that is reserved
     we clear it here. */
  module->set_mod(module, MCT_MODULE_FLAG_INVALID, identity);

  CDBG("%s:%d] X rc: %d\n", __func__, __LINE__, rc);
  return rc;

pproc_unreserve_done:
  MCT_OBJECT_UNLOCK(port);
  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_source_check_caps_reserve
 *    @port:        this pproc module's source port;
 *    @peer_caps:   the capability of peer port which wants to
 *                  match interface port;
 *    @stream_info: stream attributes used to reserve this port;
 *
 *  To reserve a source port based on stream's information.
 *
 *  If the session ID is different, support can be provided via
 *  create a new port.
 *
 *  Once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return
 *    failure;
 *  - If the requested stream belongs to a different session,
 *    the port can not be used;
 *  - Might need to create new topology; but for now only
 *    possible case is unprocessed divert path.
 *
 *  Return TRUE if port can be reserved.
 **/
static boolean pproc_port_source_check_caps_reserve(mct_port_t *port,
  void *caps __unused, void *info)
{
  mct_module_t         *pproc;
  mct_list_t           *p_list;
  mct_port_t           *sink_port;
  boolean               rc = TRUE;
  uint32_t                   i, j;
  pproc_port_private_t *port_private;
  pproc_port_private_t *sink_port_private;
  mct_module_type_identity_t     mod_type;
  mct_module_t         *sink_submodule;
  mct_stream_info_t    *stream_info = (mct_stream_info_t *)info;
  mct_event_t           event;
  pproc_divert_config_t divert_config;

  PP_LOW("E\n");


  /* sanity check */
  if (!port || !stream_info ||
      strncmp(MCT_OBJECT_NAME(port), "pproc_source", strlen("pproc_source"))) {
    PP_ERR("error port %p, stream_info: %p\n", port, stream_info);
    rc = FALSE;
    return rc;
  }
  MCT_OBJECT_LOCK(port);

  if (pproc_port_check_port_state(port, stream_info) == FALSE) {
    rc = FALSE;
    goto source_reserve_done;
  }

  port_private = MCT_OBJECT_PRIVATE(port);
  /* reserve the port for this stream */
  for(i=0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
    if(port_private->streams[i].state == PPROC_PORT_STATE_UNRESERVED) {
      port_private->streams[i].state = PPROC_PORT_STATE_RESERVED;
      port_private->streams[i].stream_info = stream_info;
      port_private->num_streams++;

      /* Get the module's sink ports and retrieve the existing stream */
      pproc = (mct_module_t *)MCT_PORT_PARENT(port)->data;
      MCT_OBJECT_LOCK(pproc);
      p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(pproc),
        &stream_info->identity, pproc_port_check_identity_in_port);
      MCT_OBJECT_UNLOCK(pproc);

      if (!p_list) {
        PP_ERR("error no matching sink port found\n");
        rc = FALSE;
        goto source_reserve_done;
      }

      sink_port = (mct_port_t *)p_list->data;
      /* Pick up the last module in the stream attached to sink port */
      sink_port_private = MCT_OBJECT_PRIVATE(sink_port);
      for (j = 0; j < PPROC_MAX_STREAM_PER_PORT; j++) {
        if (sink_port_private->streams[j].stream_info &&
          sink_port_private->streams[j].stream_info->identity ==
          stream_info->identity) {
          /* TODO: Handle sink modules also */
          mod_type.type = MCT_MODULE_FLAG_SINK;
          mod_type.identity = stream_info->identity;
          p_list = mct_list_find_custom(
            MCT_STREAM_CHILDREN(sink_port_private->streams[j].pproc_stream),
            &mod_type, pproc_port_match_module_type);

          if (!p_list) {
            PP_ERR("error invalid stream\n");
            rc = FALSE;
            goto source_reserve_done;
          }

          sink_submodule = (mct_module_t *)p_list->data;
          port_private->streams[i].int_link =
            pproc_port_resrv_port_on_module(sink_submodule, stream_info,
            MCT_PORT_SRC, port);
          if (port_private->streams[i].int_link) {
            /* Invoke ext link for the submod port */
            port_private->streams[i].int_link->ext_link(stream_info->identity,
              port_private->streams[i].int_link, port);
            rc = mct_port_add_child(stream_info->identity,
              port_private->streams[i].int_link);
            /* Change the module type */
            /* TODO: Handle sink modules also */
            sink_submodule->set_mod(sink_submodule, MCT_MODULE_FLAG_INDEXABLE,
              stream_info->identity);
            break;
          }
        }
      }

      if(j == PPROC_MAX_STREAM_PER_PORT) {
        PP_ERR("error, unexpected error!!!");
        rc = FALSE;
      }

      if (rc == TRUE) {
        port_private->sessionid = stream_info->identity & 0xFFFF0000;
        port_private->streams[i].pproc_stream =
          sink_port_private->streams[j].pproc_stream;
      }

      break;
    }
  }

  if(i == PPROC_MAX_STREAM_PER_PORT) {
    PP_ERR("error, unexpected error!!!");
    rc = FALSE;
  }

source_reserve_done:
  MCT_OBJECT_UNLOCK(port);
  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_source_check_caps_unreserve
 *    @port: this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *    Return FALSE if the identity is not existing.
 *
 *  This function unreserves the identity on this port.
 **/
static boolean pproc_port_source_check_caps_unreserve(mct_port_t *port,
  uint32_t identity)
{
  uint32_t                    i;
  boolean                     rc = TRUE;
  pproc_port_private_t       *port_private;
  mct_module_t               *module;
  mct_list_t                 *list;
  pproc_port_reserve_result_t result;
  pproc_port_stream_info_t   *port_stream_info;

  PP_LOW("E\n");


  if (!port ||
      strncmp(MCT_OBJECT_NAME(port), "pproc_source", strlen("pproc_source"))) {
    PP_ERR("error because invalid parameters!\n");
    rc = FALSE;
    return rc;
  }
  MCT_OBJECT_LOCK(port);

  module = MCT_MODULE_CAST(MCT_PORT_PARENT(port)->data);
  /* 1. Check the identity is mapped
     2. Destroy stream topology
     3. Change stream state to UNRESERVED and decrement num_streams
     4. If num_streams is zero change port_type to invalid & remove session */
  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  for (i=0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
    if ((port_private->streams[i].state == PPROC_PORT_STATE_RESERVED) &&
      (port_private->streams[i].stream_info->identity == identity)) {
      port_private->streams[i].state = PPROC_PORT_STATE_UNRESERVED;
      port_stream_info = &port_private->streams[i];
      port_stream_info->int_link->un_link(identity, port_stream_info->int_link,
        port);
      rc = port_stream_info->int_link->check_caps_unreserve(
        port_stream_info->int_link, identity);
      mct_port_remove_child(identity, port_stream_info->int_link);
      port_stream_info->int_link = NULL;
      port_stream_info->stream_info = NULL;
      port_private->num_streams--;
      if (!port_private->num_streams) {
        port_private->sessionid = 0;
        port_private->port_type = PPROC_PORT_TYPE_INVALID;
      }
      break;
    }
  }

  if(i == PPROC_MAX_STREAM_PER_PORT) {
    PP_ERR("error, unexpected error!!!");
    /* TODO: need to reset the port_type and streaming_mode and handle
       num_streams */
    rc = FALSE;
  }

pproc_src_unreserve_done:
  MCT_OBJECT_UNLOCK(port);
  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_set_caps
 *    @port: port object which the caps to be set;
 *    @caps: this port's capability.
 *
 *  Return TRUE if it is valid port.
 *
 *  Function overwrites a ports capability.
 **/
static boolean pproc_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  PP_LOW("E\n");
  if (!port || !caps) {
    PP_ERR("error port: %p caps: %p\n", port, caps);
    return FALSE;
  }

  if (strncmp(MCT_OBJECT_NAME(port), "pproc_sink", strlen("pproc_sink")) &&
      strncmp(MCT_OBJECT_NAME(port), "pproc_source", strlen("pproc_source"))) {
    PP_ERR("error invalid port\n");
    return FALSE;
  }

  port->caps = *caps;
  PP_LOW("X\n");
  return TRUE;
}

/** pproc_port_check_divert_type
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Check existence of downstream module. If yes, send
 *  downstream event to check for processed/unprocessed divert
 *  to be set on pproc submodules.
 *
 *  Return TRUE for sucess.
 **/
static boolean pproc_port_check_divert_type(mct_port_t *int_link,
  uint32_t identity, uint32_t *divert_mask)
{
  mct_event_t event;
  boolean     rc = TRUE;

  *divert_mask = 0;
  /* Send event to downstream module to check for requirement of processed
     or unprocessed frame */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_QUERY_DIVERT_TYPE;
  event.u.module_event.module_event_data = (void *)divert_mask;

  rc = int_link->event_func(int_link, &event);
  if (rc == FALSE) {
    PP_ERR("error: Query divert type failed\n");
    return FALSE;
  }

  return TRUE;
}

static int32_t pproc_port_dump_metaentry_prefix(int32_t file_fd,
  pproc_meta_entry_t *entry)
{
  write(file_fd, &entry->dump_type, sizeof(entry->dump_type));
  write(file_fd, &entry->len, sizeof(entry->len));
  write(file_fd, &entry->start_addr, sizeof(entry->start_addr));
  write(file_fd, &entry->lux_idx, sizeof(entry->lux_idx));
  write(file_fd, &entry->gain, sizeof(entry->gain));
  write(file_fd, &entry->component_revision_no,
    sizeof(entry->component_revision_no));
  return 0;
}


static int32_t pproc_port_dump_entry(int32_t file_fd,
  pproc_meta_entry_t *entry, uint32_t size)
{
  uint32_t written_len = 0;
  if (entry->dump_type == PPROC_META_DATA_INVALID) {
    return 0;
  }

  pproc_port_dump_metaentry_prefix(file_fd, entry);
  written_len =
    write(file_fd, entry->pproc_meta_dump, size);
  if (written_len != size) {
    PP_ERR("invalid written len %d write size %d \n", written_len, size);
  }

  return 0;
}

boolean pproc_port_check_meta_data_dump(cam_stream_type_t stream_type)
{
  int32_t enabled = 0;
  char value[PROPERTY_VALUE_MAX];

  if ((stream_type != CAM_STREAM_TYPE_SNAPSHOT) &&
    (stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)) {
    return FALSE;
  }

  property_get("persist.vendor.camera.dumpmetadata", value, "0");
  enabled = atoi(value);
  if (enabled == 0) {
    return FALSE;
  }

  return TRUE;
}

static int32_t pproc_port_dump_metadata(mct_port_t *port,
  pproc_port_stream_info_t *port_stream,
  meta_data_container *meta_data_cont, uint32_t frame_id)
{
  uint32_t i, index = 0;
  int32_t enabled = 0;
  char value[PROPERTY_VALUE_MAX];
  int32_t meta_frame_count = 0;
  int32_t frm_num = 0;
  char buf[128];
  int32_t file_fd = 0;
  int32_t written_len = 0;
  char stream_type_str[32];
  cam_stream_type_t stream_type;
  cpp_info_t *cpp_meta_info;
  rnr_info_t *rnr_info;
  uint32_t prefix_size = 0;
  char timeBuf[256];
  time_t current_time;
  struct tm * timeinfo;
  pproc_port_private_t *port_private;
  pproc_meta_data_t *meta_data = meta_data_cont->pproc_meta_data;

  time(&current_time);
  timeinfo = localtime(&current_time);

  if (!port_stream || !meta_data || !timeinfo) {
    PP_DBG("fail port_private %p meta_data %p timeinfo:%p\n",
      port_stream, meta_data, timeinfo);
    return -EINVAL;
  }
  /* sanity check */
  if (!port ) {
    PP_ERR("error port: %p \n", port);
    return FALSE;
  }

  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  if (!port_private) {
    PP_ERR("error port_private cannot be null\n");
    return FALSE;
  }


  memset(buf, 0, sizeof(buf));
  memset(stream_type_str, 0, sizeof(stream_type_str));

  strftime(timeBuf, sizeof(timeBuf),QCAMERA_DUMP_FRM_LOCATION"PPROC_%Y%m%d_%H%M%S_", timeinfo);
  stream_type = port_stream->stream_info->stream_type;

  meta_frame_count = port_stream->meta_frame_count;
  property_get("persist.vendor.camera.dumpmetadata", value, "0");
  enabled = atoi(value);
  if (enabled == 0) {
    goto POST_DUMP_EXIT;
  }

  frm_num = ((enabled & 0xffff0000) >> 16);
  if(frm_num == 0) {
    frm_num = 10; //default 10 frames
  }
  if(frm_num > 256) {
    frm_num = 256; //256 buffers cycle around
  }
  if((frm_num == 256) && (meta_frame_count >= frm_num)) {
    // reset frame count if cycling
    meta_frame_count = 0;
  }
  if (((meta_frame_count >= frm_num) &&
    ((stream_type != CAM_STREAM_TYPE_SNAPSHOT) ||
    (stream_type != CAM_STREAM_TYPE_OFFLINE_PROC))) ||
    (stream_type == CAM_STREAM_TYPE_PREVIEW) ||
    (stream_type == CAM_STREAM_TYPE_ANALYSIS)) {
    goto POST_DUMP_EXIT;
  }

  if (stream_type == CAM_STREAM_TYPE_PREVIEW) {
    snprintf(stream_type_str, sizeof(stream_type_str),
      "%s", "preview");
  } else if (stream_type == CAM_STREAM_TYPE_VIDEO) {
    snprintf(stream_type_str, sizeof(stream_type_str),
      "%s", "video");
    port_stream->meta_frame_count = 0;
  } else if (stream_type == CAM_STREAM_TYPE_SNAPSHOT ||
    stream_type == CAM_STREAM_TYPE_POSTVIEW ||
    stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    snprintf(stream_type_str, sizeof(stream_type_str),
      "%s", "snapshot");
  } else {
    PP_ERR("failed: invalid stream type %d\n", stream_type);
    stream_type_str[0] = '\n';
  }

  CREATE_DUMP_FILENAME(buf, timeBuf,
    port_stream->stream_info->reprocess_config.pp_feature_config,
    meta_frame_count, stream_type_str, frame_id);
  PP_LOW("Filename %s", buf);
  file_fd = open(buf, O_RDWR | O_CREAT, 0777);
  if (file_fd < 0) {
    PP_ERR("failed: invalid file_fd %d", file_fd);
    goto POST_DUMP_EXIT;
  }

  prefix_size = sizeof(pproc_meta_entry_t) - sizeof(void *);
  meta_data->header.version = METADATA_HEADER_VERSION;
  for(i = 0; i < PPROC_META_DATA_MAX_IDX; i++) {
    if(meta_data->entry[i].dump_type != PPROC_META_DATA_INVALID) {
      meta_data->header.tuning_size[i] += prefix_size;
    }
  }

  /* Dump pproc_meta_header_t */
  written_len =
    write(file_fd, &meta_data->header, sizeof(pproc_meta_header_t));
  if (written_len != sizeof(pproc_meta_header_t)) {
    PP_ERR("invalid written len %d write size %d", written_len,
      sizeof(pproc_meta_header_t));
  }

  for(i = 0; i < PPROC_META_DATA_MAX_IDX ; i++) {
    pproc_port_dump_entry(file_fd, &meta_data->entry[i],
      meta_data->entry[i].len);
  }

  close(file_fd);

  port_stream->meta_frame_count++;

POST_DUMP_EXIT:
  if (meta_data) {
    if (meta_data->entry[PPROC_META_DATA_CPP_IDX].dump_type ==
      PPROC_META_DATA_CPP) {
      cpp_meta_info =
        meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump;
      if (cpp_meta_info) {
        free(cpp_meta_info);
      }
    }
    if (meta_data->entry[PPROC_META_DATA_FE_IDX].dump_type ==
      PPROC_META_DATA_FE) {
      free(meta_data->entry[PPROC_META_DATA_FE_IDX].pproc_meta_dump);
    }
    if (meta_data->entry[PPROC_META_DATA_FD_IDX].dump_type ==
      PPROC_META_DATA_FD) {
      free(meta_data->entry[PPROC_META_DATA_FD_IDX].pproc_meta_dump);
    }
    if (meta_data->entry[PPROC_META_DATA_LDS_IDX].dump_type ==
      PPROC_META_DATA_LDS) {
      free(meta_data->entry[PPROC_META_DATA_LDS_IDX].pproc_meta_dump);
    }
    if (meta_data->entry[PPROC_META_DATA_CAC3_IDX].dump_type ==
      PPROC_META_DATA_CAC3) {
      free(meta_data->entry[PPROC_META_DATA_CAC3_IDX].pproc_meta_dump);
    }
    if (meta_data->entry[PPROC_META_DATA_RNR_IDX].dump_type ==
      PPROC_META_DATA_RNR) {
      free(meta_data->entry[PPROC_META_DATA_RNR_IDX].pproc_meta_dump);
    }
    free(meta_data);
    pthread_mutex_lock(&port_private->metadata_list_lock);
    for( index = 0; index < PPROC_METADATA_QUEUE_SIZE; index++) {
      if(port_private->metadata_list[index].used &&
          port_private->metadata_list[index].entry == meta_data_cont) {
        PP_LOW("Found entry for %p !\n", (void *)meta_data_cont);
        port_private->metadata_list[index].entry = NULL;
        port_private->metadata_list[index].used = FALSE;
        break;
      }
    }
    pthread_mutex_unlock(&port_private->metadata_list_lock);
    if(index == PPROC_METADATA_QUEUE_SIZE) {
      PP_ERR("Entry not found!!\n");
    }
  }
  return 0;
}

/** pproc_port_sink_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because pproc module works no more than a event pass throgh module,
 *  hence its event handling should be fairely straight-forward.
 *
 *  Return TRUE for successful event processing.
 **/
static boolean pproc_port_sink_event(mct_port_t *port, mct_event_t *event)
{
  uint32_t              i, index = 0;
  boolean               rc = TRUE;
  pproc_port_private_t *port_private;
  mct_list_t           *p_list = NULL;
  uint32_t              divert_mask = 0;

  PP_LOW("E\n");
  /* sanity check */
  if (!port || !event) {
    PP_ERR("error port: %p event: %p\n", port, event);
    return FALSE;
  }

  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  if (!port_private) {
    PP_ERR("error port_private cannot be null\n");
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_UPSTREAM: {
    /* Capture the get_parent_module event, this event
       does not need to be forwarded. Forward all other events */
    if (event->type == MCT_EVENT_MODULE_EVENT) {
      switch (event->u.module_event.type) {
#if ASF_OSD
        case MCT_EVENT_MODULE_GET_PARENT_MODULE:
          if(event->u.module_event.module_event_data == NULL) {
            PP_ERR("failed, module_event_data = NULL\n");
          } else {
            mct_module_t **p_module = (void *) (event->u.module_event.module_event_data);
            if(MCT_OBJECT_PARENT(port) == NULL) {
              PP_ERR("failed, MCT_OBJECT_PARENT(port) = NULL\n");
            } else {
              *p_module = (mct_module_t *)(MCT_OBJECT_PARENT(port)->data);
            }
          }
          //do not need to be forwarded
          goto end;
#endif
        case MCT_EVENT_MODULE_PP_SUBMOD_POST_TO_BUS:
          rc = mct_module_post_bus_msg((mct_module_t *)MCT_PORT_PARENT(port)->data,
            event->u.module_event.module_event_data);
          goto end;

        case MCT_EVENT_MODULE_BUF_DIVERT_ACK:
          PP_DBG("Got MCT_EVENT_MODULE_BUF_DIVERT_ACK id %x frid %d \n",
            event->identity ,event->u.module_event.current_frame_id);

        break;
        case MCT_EVENT_MODULE_PPROC_DUMP_METADATA:
          PP_LOW("Got MCT_EVENT_MODULE_PPROC_DUMP_METADATA id %x ptr %p \n",
            event->identity ,event->u.module_event.module_event_data);
          for (i = 0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
            if (port_private->streams[i].stream_info &&
              (port_private->streams[i].stream_info->identity ==
              event->identity)) {
              pproc_meta_data_dump_t *metadata_dump =
                event->u.module_event.module_event_data;
              if (metadata_dump) {
                /* Extract the meta data and dump it to file */
                meta_data_container *md_container = metadata_dump->meta_data;
                if(md_container){
                  pproc_port_dump_metadata(port, &port_private->streams[i],
                    md_container, metadata_dump->frame_id);
                  free(md_container);
                } else {
                   PP_LOW("md_container NULL\n");
                }
              }
              break;
            }
          }
          break;
        default:
        break;
      }
    }
    if (MCT_PORT_PEER(port)) {
      /* always forward the event to upstream */
      rc = mct_port_send_event_to_peer(port, event);
    }
  } /* case MCT_EVENT_TYPE_UPSTREAM */
    break;

  case MCT_EVENT_DOWNSTREAM: {
    for (i = 0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
      if (port_private->streams[i].stream_info
        && (port_private->streams[i].stream_info->identity == event->identity)
        && (port_private->streams[i].int_link)) {
        /* Because mct_stream_t does not have mct_stream_info_t as a pointer,
           We need to take a copy of the stream_info->img_buffer_list in the
           duplicate stream created in pproc. For this reason we need to peek
           into the STREAMON/STREAMOFF events to update the img_buffer_list */
        switch(event->type) {
        case MCT_EVENT_CONTROL_CMD:
          switch(event->u.ctrl_event.type) {
          case MCT_EVENT_CONTROL_STREAMON: {
            mct_stream_info_t *streaminfo =
              (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
            /* Update the stream_info in duplicate stream */
            port_private->streams[i].pproc_stream->streaminfo.img_buffer_list =
              streaminfo->img_buffer_list;

            rc = port_private->streams[i].int_link->event_func(
              port_private->streams[i].int_link, event);
          }
            break;
          case MCT_EVENT_CONTROL_STREAMOFF: {
            rc = port_private->streams[i].int_link->event_func(
              port_private->streams[i].int_link, event);
            /* Update the stream_info in duplicate stream */
            port_private->streams[i].pproc_stream->streaminfo.img_buffer_list =
              NULL;
          }
            break;
          case MCT_EVENT_CONTROL_SET_PARM: {
            mct_module_t *pproc_module =
              (mct_module_t *)MCT_PORT_PARENT(port)->data;
            if (pproc_module && pproc_module->process_event) {
              pproc_module->process_event(pproc_module, event);
            }
            rc = port_private->streams[i].int_link->event_func(
              port_private->streams[i].int_link, event);
          }
            break;
          case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
            mct_module_t *pproc_module =
              (mct_module_t *)MCT_PORT_PARENT(port)->data;
            if (pproc_module) {
              mct_module_type_t mod_type = mct_module_find_type(pproc_module,
                event->identity);
              rc = port_private->streams[i].int_link->event_func(
                port_private->streams[i].int_link, event);
              if ((mod_type == MCT_MODULE_FLAG_INDEXABLE ||
                mod_type == MCT_MODULE_FLAG_SINK) &&
                pproc_module->process_event) {
                pproc_module->process_event(pproc_module, event);
              }
            }
          }
            break;
          default:
            rc = port_private->streams[i].int_link->event_func(
              port_private->streams[i].int_link, event);
            break;
          }
          break;
        case MCT_EVENT_MODULE_EVENT:
          switch(event->u.module_event.type) {
          case MCT_EVENT_MODULE_SET_SENSOR_OUTPUT_INFO: {
            /* Needed only for upcoming bayer processing modules */
            rc = port_private->streams[i].int_link->event_func(
              port_private->streams[i].int_link, event);
            break;
          }
          case MCT_EVENT_MODULE_BUF_DIVERT: {
            uint8_t need_meta;
            PP_DBG("Got MCT_EVENT_MODULE_BUF_DIVERT id %x frid %d \n",
              event->identity ,event->u.module_event.current_frame_id);
            isp_buf_divert_t *buf_divert =
              (isp_buf_divert_t *)event->u.module_event.module_event_data;
            need_meta = pproc_port_check_meta_data_dump(
              port_private->streams[i].stream_info->stream_type);

            /* Attach meta data buffer to make pproc submodules to fill */
            if ((!buf_divert->meta_data) && need_meta) {
              pthread_mutex_lock(&port_private->metadata_list_lock);
              for(index = 0; index < PPROC_METADATA_QUEUE_SIZE; index++) {
                if(port_private->metadata_list[index].used == FALSE) {
                  port_private->metadata_list[index].used = TRUE;
                  PP_ERR("Found spot for meta idx %d \n", index);
                  break;
                  }
              }
              if(index == PPROC_METADATA_QUEUE_SIZE) {
                PP_ERR("Error!! No space for meta \n");
                pthread_mutex_unlock(&port_private->metadata_list_lock);
              } else {
                port_private->metadata_list[index].entry =
                  (meta_data_container *)CAM_MALLOC(sizeof(meta_data_container));
                port_private->metadata_list[index].used = TRUE;
                meta_data_container *md_container =
                  port_private->metadata_list[index].entry;
                PP_LOW("Found spot for meta idx %d ptr %p\n",
                  index, (void *)md_container);

                pthread_mutex_unlock(&port_private->metadata_list_lock);
                if (!md_container) {
                  PP_ERR("alloc failed\n");
                } else {
                  memset(md_container, 0, sizeof(meta_data_container));
                  md_container->pproc_meta_data =
                    (pproc_meta_data_t *)CAM_MALLOC(sizeof(pproc_meta_data_t));
                  if (!md_container->pproc_meta_data) {
                    PP_ERR("alloc failed\n");
                    free(md_container);
                  } else {
                    memset(md_container->pproc_meta_data, 0,
                      sizeof(pproc_meta_data_t));
                    buf_divert->meta_data = md_container;
                  }
                }
              }
            } else if((buf_divert->meta_data) && need_meta){
              PP_LOW("Metadata is not null!!\n");
              pthread_mutex_lock(&port_private->metadata_list_lock);
              for(index = 0; index < PPROC_METADATA_QUEUE_SIZE; index++) {
                if(port_private->metadata_list[index].used == FALSE) {
                  port_private->metadata_list[index].used = TRUE;
                  PP_LOW(" Found spot for meta idx %d\n", index);
                  break;
                  }
              }
              if(index == PPROC_METADATA_QUEUE_SIZE) {
                PP_ERR("Error!! No space for meta\n");
              } else {
                port_private->metadata_list[index].entry = buf_divert->meta_data;
                port_private->metadata_list[index].used = TRUE;
              }
              pthread_mutex_unlock(&port_private->metadata_list_lock);
            }
            rc = port_private->streams[i].int_link->event_func(
              port_private->streams[i].int_link, event);

            if ((buf_divert->meta_data) && (buf_divert->ack_flag == TRUE)) {
              meta_data_container *md_container = buf_divert->meta_data;
              /* Extract the meta data and dump it to file */
              pproc_port_dump_metadata(port, &port_private->streams[i],
                md_container, buf_divert->buffer.sequence);
              free(md_container);
            }
            break;
          }
          default:
            rc = port_private->streams[i].int_link->event_func(
              port_private->streams[i].int_link, event);
            break;
          }
          break;
        default:
          rc = port_private->streams[i].int_link->event_func(
            port_private->streams[i].int_link, event);
          break;
        }
        break;
      }
    }

  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  default:
    rc = FALSE;
    break;
  }

end:
  PP_LOW("X rc: %d\n", rc);
  return rc;
}

/** pproc_port_source_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because pproc module works no more than a event pass throgh module,
 *  hence its event handling should be fairely straight-forward.
 *
 *  Return TRUE for successful event processing.
 **/
static boolean pproc_port_source_event(mct_port_t *port, mct_event_t *event)
{
  uint32_t              i;
  boolean               rc = TRUE;
  pproc_port_private_t *port_private;
  mct_list_t           *p_list = NULL;

  PP_LOW("E\n");
  /* sanity check */
  if (!port || !event) {
    PP_ERR("error port: %p event: %p\n", port, event);
    return FALSE;
  }

  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  if (!port_private) {
    PP_ERR("error port_private cannot be null\n");
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {

    if (MCT_PORT_PEER(port)) {
      /* always forward the event to upstream */
      rc = mct_port_send_event_to_peer(port, event);
    }

  } /* case MCT_EVENT_TYPE_UPSTREAM */
    break;

  case MCT_EVENT_UPSTREAM: {
    /* In case of sink port, no need to peek into the event,
     * instead just simply forward the event to internal link */

    for (i = 0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
      if (port_private->streams[i].stream_info
        && port_private->streams[i].stream_info->identity == event->identity
        && (port_private->streams[i].int_link)) {
        rc = port_private->streams[i].int_link->event_func(
          port_private->streams[i].int_link, event);
      }
    }

  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  default:
    rc = FALSE;
    break;
  }

  PP_LOW("X\n");
  return rc;
}

/** pproc_port_ext_link
 *    @identity:  Identity of session/stream
 *    @port: SINK of pproc ports
 *    @peer: For pproc sink- peer is most likely isp port
 *           For src module -  peer is submodules sink.
 *
 *  Set pproc port's external peer port.
 **/
static boolean pproc_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  pproc_port_private_t *port_data;
  uint32_t              i;

  PP_LOW("E\n");
  if(!port || !peer) {
    PP_ERR("error port=%p, peer=%p", port, peer);
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  if (MCT_PORT_PEER(port) && (MCT_PORT_PEER(port) != peer)) {
    PP_ERR("error old_peer:%s, new_peer:%s\n",
      MCT_OBJECT_NAME(MCT_PORT_PEER(port)), MCT_OBJECT_NAME(peer));
    MCT_OBJECT_UNLOCK(port);
    return FALSE;
  }

  /* TODO: Need to ref count for multiple connections */
  port_data = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  for(i=0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
    if((port_data->streams[i].state == PPROC_PORT_STATE_RESERVED) &&
      (port_data->streams[i].stream_info) &&
      (port_data->streams[i].stream_info->identity == identity)) {
      port_data->streams[i].state = PPROC_PORT_STATE_LINKED;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = peer;
      }
      MCT_OBJECT_REFCOUNT(port) += 1;
      MCT_OBJECT_UNLOCK(port);
      PP_LOW("X\n");
      return TRUE;
    }
  }

  MCT_OBJECT_UNLOCK(port);
  PP_LOW("X\n");
  return FALSE;
}

/** pproc_port_unlink
 *    @identity: identity to be unlinked
 *    @port:     this port to unlink
 *    @peer:     peer to be unlinked
 *
 * This funtion unlink the peer ports of pproc sink, src ports
 * and its peer submodule's port
 *
 *  Return TRUE for successful unlink.
 **/
static void pproc_port_unlink(uint32_t identity,
  mct_port_t *port, mct_port_t *peer)
{
  pproc_port_private_t *port_private;
  uint32_t              i;

  PP_LOW("E\n");
  if (!port || !peer) {
    PP_ERR("error port: %p, peer: %p\n", port,
      peer);
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (MCT_PORT_PEER(port) != peer) {
    PP_ERR("error peer:%p, unlink_peer:%p\n", MCT_PORT_PEER(port), peer);
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  if (MCT_OBJECT_REFCOUNT(port) == 0) {
    PP_ERR("error zero refcount on port\n");
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  /* TODO: Need to ref count for multiple connections */
  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  for(i = 0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
    if(port_private->streams[i].state == PPROC_PORT_STATE_LINKED &&
      port_private->streams[i].stream_info &&
      port_private->streams[i].stream_info->identity == identity) {
      port_private->streams[i].state = PPROC_PORT_STATE_RESERVED;
      MCT_OBJECT_REFCOUNT(port) -= 1;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = NULL;
      }
      MCT_OBJECT_UNLOCK(port);
      PP_LOW("X\n");
      return;
    }
  }

  MCT_OBJECT_UNLOCK(port);
  PP_LOW("X\n");
  return;
}

/** pproc_port_sink_int_link
 *    @identity:
 *    @port:
 **/
static mct_list_t *pproc_port_int_link(unsigned int identity __unused, mct_port_t *port __unused)
{
  /* TODO */
  PP_LOW("E\n");
  PP_LOW("X\n");
  return NULL;
}

/** pproc_port_deinit
 *    @port: port object to be deinit
 **/
void pproc_port_deinit(mct_port_t *port)
{
  pproc_port_private_t *port_private;

  PP_LOW("E\n");
  if (!port) {
    PP_ERR("error port: %p\n", port);
    return;
  }

  port_private = (pproc_port_private_t *)MCT_OBJECT_PRIVATE(port);
  if (!port_private) {
    PP_ERR("error because missing port_private data\n");
    return;
  }

  if (strncmp(MCT_OBJECT_NAME(port), "pproc_sink", strlen("pproc_sink")) &&
      strncmp(MCT_OBJECT_NAME(port), "pproc_source", strlen("pproc_source"))) {
    PP_ERR("error because port is invalid\n");
    return;
  }

  /* TODO: need to ensure all attached streams on this port is removed */

  free(port_private);

  mct_port_destroy(port);
  PP_LOW("X\n");
  return;
}

/** pproc_port_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to create pproc module's
 *  sink/source ports and overwrite default port methods and install
 *  capabilities.
 *
 *  Return mct_port_t object on success, otherwise return NULL.
 **/
mct_port_t *pproc_port_init(const char *name)
{
  uint32_t              i;
  mct_port_t           *port;
  pproc_port_private_t *port_private;
  mct_port_caps_t       caps;

  PP_LOW("E %s\n", name);
  port = mct_port_create(name);
  if (port == NULL) {
    PP_ERR("error creating port\n");
    return NULL;
  }

  port_private = CAM_MALLOC(sizeof(pproc_port_private_t));
  if (port_private == NULL) {
    PP_ERR("error allocating port private\n");
    goto private_error;
  }

  memset(port_private, 0, sizeof(pproc_port_private_t));

  for (i = 0; i < PPROC_MAX_STREAM_PER_PORT; i++) {
    port_private->streams[i].state = PPROC_PORT_STATE_UNRESERVED;
    port_private->port_type = PPROC_PORT_TYPE_INVALID;
  }
  pthread_mutex_init(&port_private->metadata_list_lock , NULL);
  MCT_OBJECT_PRIVATE(port) = port_private;
  caps.port_caps_type  = MCT_PORT_CAPS_FRAME;

  caps.u.frame.format_flag  = MCT_PORT_CAP_FORMAT_YCBCR;
  caps.u.frame.size_flag    = MCT_PORT_CAP_SIZE_20MB;

  if (!strncmp(name, "pproc_sink", strlen("pproc_sink"))) {
    port->direction = MCT_PORT_SINK;

    mct_port_set_event_func(port, pproc_port_sink_event);
    mct_port_set_check_caps_reserve_func(port,
      pproc_port_sink_check_caps_reserve);
    mct_port_set_check_caps_unreserve_func(port,
      pproc_port_sink_check_caps_unreserve);
  } else if (!strncmp(name, "pproc_source", strlen("pproc_source"))) {
    port->direction = MCT_PORT_SRC;

    mct_port_set_event_func(port, pproc_port_source_event);
    mct_port_set_check_caps_reserve_func(port,
      pproc_port_source_check_caps_reserve);
    mct_port_set_check_caps_unreserve_func(port,
      pproc_port_source_check_caps_unreserve);
  } else {
    PP_ERR("error invalid pproc port\n");
    goto port_type_error;
  }

  mct_port_set_int_link_func(port, pproc_port_int_link);
  mct_port_set_set_caps_func(port, pproc_port_set_caps);
  mct_port_set_ext_link_func(port, pproc_port_ext_link);
  mct_port_set_unlink_func(port, pproc_port_unlink);

  if (port->set_caps)
    port->set_caps(port, &caps);

  PP_LOW("X\n");
  return port;

port_type_error:
  free(port_private);
private_error:
  mct_port_destroy(port);
  PP_LOW("X\n");
  return NULL;
}
