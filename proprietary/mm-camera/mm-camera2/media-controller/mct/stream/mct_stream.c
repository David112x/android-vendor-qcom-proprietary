/* Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "cam_ker_headers.h"
#include "mct_controller.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "mct_module.h"
#include "camera_dbg.h"
#include "cam_intf.h"
#include "supermct.h"

#include <media/msmb_generic_buf_mgr.h>

char *preview_secure_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp","pproc"},
  { "isp", "stats"},
  {},
};
char *raw_secure_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp"},
  {},
};
char *preview_bayer_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp","pproc","imglib"},
  { "isp", "stats"},
  {},
};
char *preview_yuv_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","pproc","imglib"},
  {},
};
char *depth_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","pproc"},
  {},
};
char *param_bayer_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp","pproc","imglib"},
  { "isp", "stats"},
  {},
};
char *param_yuv_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","pproc","imglib"},
  {},
};
char *callback_bayer_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp","pproc","imglib"},
  { "isp", "stats"},
  {},
};
char *callback_yuv_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","pproc"},
  {},
};
char *zsl_snapshot_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp"},
  { "isp", "stats"},
  {},
};
char *video_bayer_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp","pproc"},
  { "isp", "stats"},
  {},
};
char *video_yuv_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","pproc"},
  {},
};
char *analysis_bayer_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp","pproc","imglib"},
  {},
};
char *analysis_yuv_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","pproc","imglib"},
  {},
};

char *raw_bayer_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface","isp"},
  {"isp","stats"},
  {},
};

char *raw_yuv_mod[][MAX_STREAM_MODULES] = {
  {"sensor","iface"},
  {},
};
char *meta_mod[][MAX_STREAM_MODULES] = {
  {},
};
/** g_imglib_feature_mask:
 *
 *  Composite feature mask of the features supported by imglib
 **/
static cam_feature_mask_t g_imglib_feature_mask =
  CAM_QCOM_FEATURE_REGISTER_FACE |
  CAM_QCOM_FEATURE_FACE_DETECTION |
  CAM_QCOM_FEATURE_HDR |
  CAM_QCOM_FEATURE_CHROMA_FLASH |
  CAM_QCOM_FEATURE_UBIFOCUS |
  CAM_QCOM_FEATURE_REFOCUS |
  CAM_QCOM_FEATURE_OPTIZOOM |
  CAM_QCOM_FEATURE_TRUEPORTRAIT |
  CAM_OEM_FEATURE_1 |
  CAM_OEM_FEATURE_2 |
  CAM_QCOM_FEATURE_STILLMORE |
  CAM_QTI_FEATURE_MFPROC_POSTCPP;

/** mct_stream_check_module:
 *    @
 *    @
 *
 **/
static boolean mct_stream_check_module(void *d1, void *d2)
{
  return (!strcmp(MCT_OBJECT_NAME(d1), MCT_OBJECT_NAME(d2)) ? TRUE : FALSE);
}

/** mct_stream_check_name:
 *    @
 *    @
 *
 **/
static boolean mct_stream_check_name(void *mod, void *name)
{
  return ((!strcmp(MCT_OBJECT_NAME(mod), (char *)name)) ? TRUE : FALSE);
}

/** mct_stream_get_module:
 *    @mods: modules list
 *    @name: module name to retrive from modules list
 *
 *  Return corresponding module or NULL if not found in
 *  the list.
 **/
static mct_module_t *mct_stream_get_module(mct_list_t *mods, char *name)
{
  mct_list_t *module;
  module = mct_list_find_custom(mods, name, mct_stream_check_name);
  if (module)
    return (mct_module_t *)(module->data);

  return NULL;
}

/** mct_stream_operate_unlink:
 *    @d1: module1
 *    @d2: module2
 *    @user_data: MctStream_t stream
 *
 *  To unlink module1 and module2 on one stream
 **/
void mct_stream_operate_unlink(void *d1, void *d2,
  const void *user_data)
{
  mct_module_t *mod1   = (mct_module_t *)d1;
  mct_module_t *mod2   = (mct_module_t *)d2;
  mct_stream_t *stream = (mct_stream_t *)user_data;

  if (stream->unlink)
    stream->unlink(stream, mod1, mod2);
}

/** mct_stream_add_module:
 *    @
 *    @
 *
 **/
static boolean mct_stream_add_module(mct_stream_t *stream,
  mct_module_t *module)
{
  /* can't add ourself to ourself */
  if (&((MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)))->module) == module) {
    CLOGE(CAM_MCT_MODULE, "Failed Module = %s ", MCT_OBJECT_NAME(module));
    return FALSE;
  }

  /* check to see if the module's name is already existing in the stream */
  if (mct_object_check_uniqueness(MCT_OBJECT_CHILDREN(stream),
       (const char *)MCT_OBJECT_NAME(module)) != NULL)
    return TRUE;

  /* set the module's parent and add the module to the
     pipeline's list of children */
  MCT_STREAM_LOCK(stream);
  if (mct_object_set_parent(MCT_OBJECT_CAST(module),
       MCT_OBJECT_CAST(stream)) == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Failed Module = %s ", MCT_OBJECT_NAME(module));
    MCT_STREAM_UNLOCK(stream);
    return FALSE;
  }
  MCT_STREAM_UNLOCK(stream);
  return TRUE;
}

/** mct_stream_add_branch_module:
 *    @stream:mct_stream_t* - stream pointer
 *    @module:mct_module_t* - module to be added as branch
 *    @branchto:mct_module_t* - module to be branched
 *
 *    Adds modules as branch to the given module - branchto
 *
 **/
static boolean mct_stream_add_branch_module(mct_stream_t *stream,
  mct_module_t *module, mct_module_t *branchto)
{
  /* can't add ourself to ourself */
  if (&((MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)))->module) == module)
    return FALSE;

  /* check to see if the module's name is already existing in the stream */
  if (mct_object_check_uniqueness(MCT_OBJECT_CHILDREN(stream),
       (const char *)MCT_OBJECT_NAME(module)) != NULL) {
    return TRUE;
  }

  /* set the module's parent and add the module to the pipeline's list of
   * children */
  if (mct_object_set_branch_parent(MCT_OBJECT_CAST(module),
       MCT_OBJECT_CAST(stream), MCT_OBJECT_CAST(branchto)) == FALSE)
    return FALSE;

  return TRUE;
}

/** mct_stream_link_branch_modules:
 *    @stream:mct_stream_t* - stream pointer
 *    @mod1:mct_module_t* - module to be branched
 *    @mod2, ...:mct_module_t* - modules to be added as branch
 *
 *    Links one or more modules as branch to the first module - mod1
 *
 **/
boolean mct_stream_link_branch_modules(mct_stream_t *stream,
  mct_module_t *mod1, mct_module_t *mod2, ...)
{
  va_list args;
  if (!mod1 || !mod2 || !stream)
    return FALSE;

  va_start(args, mod2);
  while (mod2) {
    if (mct_module_link((void *)(&(stream->streaminfo)), mod1, mod2) == TRUE) {
      if ((mct_stream_add_branch_module(stream, mod1, mod1) == FALSE) ||
          (mct_stream_add_branch_module(stream, mod2, mod1) == FALSE))
        goto error;
    } else {
      goto error;
    }

    mod1 = mod2;
    mod2 = va_arg(args, mct_module_t *);
  }
  va_end(args);

  return TRUE;

error:
  CLOGE(CAM_MCT_MODULE, "Error");
  /*unlinking is done during stream destroy*/
  return FALSE;
}

/** mct_stream_link_modules:
 *    @
 *    @
 *
 **/
boolean mct_stream_link_modules(mct_stream_t *stream,
  mct_module_t *mod1, mct_module_t *mod2, ...)
{
  va_list args;
  if (!mod1 || !mod2 || !stream) {
    CLOGE(CAM_MCT_MODULE, "Failed!");
    return FALSE;
  }

  ATRACE_BEGIN_SNPRINTF(30, "Camera:StreamLink-type%d",
                        stream->streaminfo.stream_type);

  va_start(args, mod2);
  while(mod2) {
    CLOGD(CAM_MCT_MODULE, "Linking modules %s and %s",
      MCT_MODULE_NAME(mod1), MCT_MODULE_NAME(mod2));

    ATRACE_BEGIN_SNPRINTF(30, "Link:%s %s",
                          MCT_MODULE_NAME(mod1), MCT_MODULE_NAME(mod2));

    if (mct_module_link((void *)(&(stream->streaminfo)), mod1, mod2) == TRUE) {
      CLOGD(CAM_MCT_MODULE, "Module = %s ", MCT_OBJECT_NAME(mod1));
      CLOGD(CAM_MCT_MODULE, "Module = %s ", MCT_OBJECT_NAME(mod2));
      if ((mct_stream_add_module(stream, mod1) == FALSE) ||
          (mct_stream_add_module(stream, mod2) == FALSE))
        goto error;
    } else {
      CLOGE(CAM_MCT_MODULE, "mct_module_link failed for mod1 (%s) mod2(%s) ",
         MCT_OBJECT_NAME(mod1), MCT_OBJECT_NAME(mod2));
      goto error;
    }

    ATRACE_END();
    mod1 = mod2;
    mod2 = va_arg(args, mct_module_t *);
  }
  va_end(args);
  ATRACE_END();
  return TRUE;

error:
  /*unlinking is done during stream destroy*/
  ATRACE_END();
  ATRACE_END();
  return FALSE;
}

static boolean mct_iface_port_caps_reserve(void *data1, void *data2)
{
  mct_port_t        *port = (mct_port_t *)data1;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data2;

  if (!port || !stream_info) {
    CLOGE(CAM_MCT_MODULE, "error port: %p stream_info: %p",
      port, stream_info);
    return FALSE;
  }

  return port->check_caps_reserve(port, NULL, stream_info);
}

static boolean mct_pproc_sink_port_caps_reserve(void *data1, void *data2)
{
  mct_port_t        *port = (mct_port_t *)data1;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data2;
  mct_port_caps_t    peer_caps;

  if (!port || !stream_info) {
    CLOGE(CAM_MCT_MODULE, "error port: %p stream_info: %p",
      port, stream_info);
    return FALSE;
  }

  if (port->peer != NULL) {
    CLOGD(CAM_MCT_MODULE, "The port %p is not free",port);
    return FALSE;
  }
  peer_caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  return port->check_caps_reserve(port, &peer_caps, stream_info);
}

/** mct_stream_link_module_array:
 *    @stream: stream object of mct_stream_t
 *    @stream_type_mod: list of module to link
 *
 * based on the stream type caller will pass module of array
 * to link the module for a particlular stream.
 *
 * Return FALSE if the stream link fails.
 **/
boolean mct_stream_link_module_array(mct_stream_t *stream,
  char *(*stream_type_mod)[MAX_STREAM_MODULES])
{
  boolean              ret = TRUE;
  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  mct_list_t           *modules =
    MCT_PIPELINE_MODULES(MCT_OBJECT_PARENT(stream)->data);
  mct_module_t *module[MAX_STREAM_MODULES];
  int mod_type;
  int mod_row=0;
  int mod_col=0;
  int num_mod=0;

  if (stream_type_mod[mod_row][mod_col] == NULL) {
    return ret;
  }

  while(1) {
    if (mod_col > MAX_STREAM_MODULES) {
      CLOGE(CAM_MCT_MODULE, "array overflow");
      return FALSE;
    }
    if (stream_type_mod[mod_row][mod_col]) {
      module[num_mod] = mct_stream_get_module(modules,stream_type_mod[mod_row][mod_col]);
      if (module[num_mod] == NULL) {
        CLOGE(CAM_MCT_MODULE, "Null: %p",module[num_mod]);
        return FALSE;
      }
      mod_col++;
      num_mod++;
    } else {
      for (num_mod=0; num_mod< mod_col;num_mod++) {
        if (num_mod == 0 && mod_row ==0) {
           mod_type = MCT_MODULE_FLAG_SOURCE;
        } else if (num_mod == (mod_col-1)) {
          mod_type = MCT_MODULE_FLAG_SINK;
        } else {
          mod_type = MCT_MODULE_FLAG_INDEXABLE;
        }
        module[num_mod]->set_mod(module[num_mod],mod_type,stream->streaminfo.identity);
      }
      for (num_mod=0; num_mod< (mod_col-1);num_mod++) {
        if (mod_row!=0) {
          ret = mct_stream_link_branch_modules(stream,module[num_mod],module[num_mod+1], NULL);
          if (ret  == FALSE) {
            CLOGE(CAM_MCT_MODULE, "Linking failed in branch modules");
            return FALSE;
          }
        } else {
          ret = mct_stream_link_modules(stream,module[num_mod],module[num_mod+1], NULL);
          if (ret == FALSE) {
            CLOGE(CAM_MCT_MODULE, "Linking failed");
            return FALSE;
          }
        }
      }
      mod_row++;
      mod_col=0;
      num_mod=0;
      if (stream_type_mod[mod_row][mod_col] == NULL) {
        break;
      }
    }
  };
  if (ret == FALSE) {
    CLOGE(CAM_MCT_MODULE, "link failed");
    return FALSE;
  }
  return ret;
}

/** mct_stream_start_link:
 *    @stream: stream object of mct_stream_t
 *
 * Analyses stream information and links corresponding
 * modules for this steam.
 *
 * Return FALSE if the stream link fails.
 **/
static boolean mct_stream_start_link(mct_stream_t *stream)
{
  uint32_t sessionid;
  cam_stream_info_t    *stream_info;
  boolean              ret = FALSE;
  mct_stream_map_buf_t *info    = MCT_STREAM_STREAMINFO(stream);
  mct_list_t           *modules =
    MCT_PIPELINE_MODULES(MCT_OBJECT_PARENT(stream)->data);
  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  mct_module_t *sensor = NULL;
  mct_module_t *iface  = NULL;
  mct_module_t *isp    = NULL;
  mct_module_t *stats  = NULL;
  mct_module_t *pproc  = NULL;
  mct_module_t *imglib = NULL;
  char *(*link_mod)[MAX_STREAM_MODULES] = NULL;
  uint32_t size = 0;

  if (info == NULL)
    return FALSE;

  stream_info = (cam_stream_info_t *)info;

  sessionid = MCT_PIPELINE_SESSION(
    MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));

  stream->streaminfo.identity = pack_identity(sessionid, stream->streamid);
  stream->streaminfo.stream_type = stream_info->stream_type;
  stream->streaminfo.fmt = stream_info->fmt;
  stream->streaminfo.dim = stream_info->dim;
  stream->streaminfo.streaming_mode = stream_info->streaming_mode;
  stream->streaminfo.num_burst = stream_info->num_of_burst;
  stream->streaminfo.buf_planes = stream_info->buf_planes;
  stream->streaminfo.pp_config = stream_info->pp_config;
  stream->streaminfo.reprocess_config = stream_info->reprocess_config;
  stream->streaminfo.num_bufs = stream_info->num_bufs;
  stream->streaminfo.is_type = stream_info->is_type;
  stream->streaminfo.dis_enable = stream_info->dis_enable;
  stream->streaminfo.is_secure = stream_info->is_secure;
  stream->streaminfo.secure_mode = stream_info->secure_mode;
  stream->streaminfo.perf_mode = stream_info->perf_mode;
  stream->streaminfo.user_buf_info = stream_info->user_buf_info;
  stream->streaminfo.dt = stream_info->dt;
  stream->streaminfo.vc = stream_info->vc;
  stream->streaminfo.sub_format = stream_info->sub_format_type;
  /* TODO: temporary solution for now */
  stream->streaminfo.stream = stream;

  switch (stream->streaminfo.stream_type) {
  case CAM_STREAM_TYPE_DEPTH:
    link_mod = depth_mod;
    size = sizeof(depth_mod);
    break;
  case CAM_STREAM_TYPE_POSTVIEW:
  case CAM_STREAM_TYPE_PREVIEW: {

    if(CAM_STREAM_TYPE_PREVIEW == stream->streaminfo.stream_type) {
      CLOGI(CAM_MCT_MODULE, "Start linking preview stream 0x%x",
        stream->streaminfo.identity);
    } else {
      CLOGI(CAM_MCT_MODULE, "Start linking postview stream 0x%x",
        stream->streaminfo.identity);
    }

    if(SECURE == stream_info->is_secure) {
      CLOGI(CAM_MCT_MODULE, "secure stream linking");
      link_mod = preview_secure_mod;
      size = sizeof(preview_secure_mod);
    } else {
      CLOGI(CAM_MCT_MODULE, "non secure stream linking");
      if (pipeline->session_data.sensor_format != FORMAT_YCBCR) {
        link_mod = preview_bayer_mod;
        size = sizeof(preview_bayer_mod);
      } else {
        link_mod = preview_yuv_mod;
        size = sizeof(preview_yuv_mod);
      }
   }
 }
    break;

  case CAM_STREAM_TYPE_PARM: {
    CLOGI(CAM_MCT_MODULE, "Start linking Session-stream 0x%x",
      stream->streaminfo.identity);
    if (pipeline->session_data.sensor_format != FORMAT_YCBCR) {
      link_mod = param_bayer_mod;
      size = sizeof(param_bayer_mod);
    } else {
      link_mod = param_yuv_mod;
      size = sizeof(param_yuv_mod);
    }
  }
    break;

  case CAM_STREAM_TYPE_CALLBACK:
  case CAM_STREAM_TYPE_SNAPSHOT: {
    /* regular snapshot stream and ZSL which needs to link pproc module,
     * reason being VFE has 16x downscaling limit. If snapshot dimension
     * is more than 16x smaller than sensor output size, pproc needs to
     * be linked to do further downscaling. */

    if (CAM_STREAM_TYPE_CALLBACK == stream->streaminfo.stream_type) {
      CLOGI(CAM_MCT_MODULE, "Start linking callback stream 0x%x",
        stream->streaminfo.identity);
    } else {
      CLOGI(CAM_MCT_MODULE, "Start linking snapshot stream 0x%x",
        stream->streaminfo.identity);
    }
/*  Regular snapshot where PPROC is part of module link */

    if (stream->streaminfo.pp_config.feature_mask &
      CAM_QCOM_FEATURE_PP_SUPERSET) {
      if (pipeline->session_data.sensor_format != FORMAT_YCBCR) {
        link_mod = callback_bayer_mod;
        size = sizeof(callback_bayer_mod);
      }
      else {
        link_mod = callback_yuv_mod;
        size = sizeof(callback_yuv_mod);
      }
    }
/*  ZSL Snapshot case, where PPROC is part of Offline stream*/
    else {
      if (pipeline->session_data.sensor_format != FORMAT_YCBCR) {
        link_mod = zsl_snapshot_mod;
        size = sizeof(zsl_snapshot_mod);
      }
      else {
        link_mod = callback_yuv_mod;
        size = sizeof(callback_yuv_mod);
      }
    }

  }
    break;

  case CAM_STREAM_TYPE_VIDEO: {
    CLOGI(CAM_MCT_MODULE, "Start linking video stream 0x%x",
      stream->streaminfo.identity);
    if (pipeline->session_data.sensor_format != FORMAT_YCBCR) {
      link_mod = video_bayer_mod;
      size = sizeof(video_bayer_mod);
    } else {
      link_mod = video_yuv_mod;
      size = sizeof(video_yuv_mod);
    }
  }
    break;

  case CAM_STREAM_TYPE_RAW: {
    CLOGI(CAM_MCT_MODULE, "Start linking raw stream 0x%x, is_secure %d",
      stream->streaminfo.identity, stream_info->is_secure);

    if(SECURE == stream_info->is_secure) {
       link_mod = raw_secure_mod;
       size = sizeof(raw_secure_mod);
    } else {
        if (pipeline->session_data.sensor_format != FORMAT_YCBCR) {
          link_mod = raw_bayer_mod;
          size = sizeof(raw_bayer_mod);
        } else {
          link_mod = raw_yuv_mod;
          size = sizeof(raw_yuv_mod);
        }
    }
  }
    break;

  case CAM_STREAM_TYPE_ANALYSIS: {
    CLOGI(CAM_MCT_MODULE, "Starting Analysis stream linking");

    if (pipeline->session_data.sensor_format != FORMAT_YCBCR) {
      link_mod = analysis_bayer_mod;
      size = sizeof(analysis_bayer_mod);
    } else {
      link_mod = analysis_yuv_mod;
      size = sizeof(analysis_yuv_mod);
    }
  }
    break;

  case CAM_STREAM_TYPE_METADATA:
    CLOGI(CAM_MCT_MODULE, "Link Metadata stream 0x%x: do nothing",
      stream->streaminfo.identity);
      link_mod = meta_mod;
      size = sizeof(meta_mod);
    break;

  case CAM_STREAM_TYPE_OFFLINE_PROC: {
    mct_module_t *single_module = NULL;
    CLOGI(CAM_MCT_MODULE, "Start linking offline stream 0x%x",
      stream->streaminfo.identity);

    if ((stream->streaminfo.reprocess_config.pp_feature_config.feature_mask &
       (CAM_QCOM_FEATURE_METADATA_PROCESSING | CAM_QCOM_FEATURE_METADATA_BYPASS))) {
      /* No linking for offline metadata processing */
      CLOGI(CAM_MCT_MODULE, "Metadata collection for offline stream");
      return TRUE;
    }

    /*Check if iface module is needed  based on feature mask */
    if (stream->streaminfo.reprocess_config.pp_feature_config.feature_mask &
      CAM_QCOM_FEATURE_RAW_PROCESSING) {
      iface = mct_stream_get_module(modules, "iface");
      if (!iface){
        CLOGE(CAM_MCT_MODULE, "NULL iface module");
        return FALSE;
      }
      isp = mct_stream_get_module(modules, "isp");
      if (!isp){
        CLOGE(CAM_MCT_MODULE, "NULL isp module");
        return FALSE;
      }
    }

   /*Check if Imglib module is needed based on feature mask*/
    if (stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & g_imglib_feature_mask){
      imglib = mct_stream_get_module(modules, "imglib");
      if (!imglib) {
        CLOGE(CAM_MCT_MODULE, "Null: imglib Module");
        return FALSE;
       }
    }

   /*Check if PPROC module is needed based on feature mask*/
    if (stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QCOM_FEATURE_DENOISE2D
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QCOM_FEATURE_SHARPNESS
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QCOM_FEATURE_EFFECT
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QCOM_FEATURE_ROTATION
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QCOM_FEATURE_CROP
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QCOM_FEATURE_SCALE
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QCOM_FEATURE_QUADRA_CFA
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QTI_FEATURE_MFPROC_PRECPP){

     if (stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
       & CAM_QCOM_FEATURE_QUADRA_CFA) {
       CLOGD(CAM_MCT_MODULE, "Quadra CFA feature mask set");
     }

     if (stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
       & CAM_QTI_FEATURE_MFPROC_PRECPP) {
       CLOGD(CAM_MCT_MODULE, "MultiFrame Process Precpp feature mask set");
     }

      pproc = mct_stream_get_module(modules, "pproc");
      if (!pproc) {
        CLOGE(CAM_MCT_MODULE, "Null: postproc Module");
        return FALSE;
      }
    }

    if (!iface || !isp || !pproc || !imglib){
      CLOGI(CAM_MCT_MODULE, "iface=%p, isp=%p, pproc=%p, imglib=%p",
        iface, isp, pproc, imglib);
    }

    if (iface){
      mct_list_t *sinkport = NULL;
      mct_port_t *port_iface = NULL;
      uint32_t ret_val;

      sinkport = mct_list_find_custom(MCT_MODULE_SINKPORTS(iface),
        &stream->streaminfo, mct_iface_port_caps_reserve);

      if (!sinkport) {
        CLOGE(CAM_MCT_MODULE, "failed: to caps reserve");
        return FALSE;
      }

      port_iface = (mct_port_t *)(sinkport->data);
      ret_val = mct_port_add_child(stream->streaminfo.identity, port_iface);

      if (!ret_val){
        CLOGE(CAM_MCT_MODULE, "failed: reserved port NULL");
      }

      /*Setting iface as source*/

      iface->set_mod(iface, MCT_MODULE_FLAG_SOURCE,
        stream->streaminfo.identity);
      if (pproc){
        if (imglib){
          isp->set_mod(isp, MCT_MODULE_FLAG_INDEXABLE,
            stream->streaminfo.identity);
          pproc->set_mod(pproc,MCT_MODULE_FLAG_INDEXABLE,
            stream->streaminfo.identity);
          imglib->set_mod(imglib,MCT_MODULE_FLAG_SINK,
            stream->streaminfo.identity);
          CLOGI(CAM_MCT_MODULE, "Linking iface->isp->pproc->imglib");
          ret = mct_stream_link_modules(stream, iface, isp, pproc,
                                        imglib, NULL);
        }
        isp->set_mod(isp, MCT_MODULE_FLAG_INDEXABLE,
          stream->streaminfo.identity);
        pproc->set_mod(pproc,MCT_MODULE_FLAG_SINK,
          stream->streaminfo.identity);
        CLOGI(CAM_MCT_MODULE, "Linking iface->isp->pproc");
        ret = mct_stream_link_modules(stream, iface, isp, pproc, NULL);
      } else {
        isp->set_mod(isp, MCT_MODULE_FLAG_SINK,
          stream->streaminfo.identity);
        CLOGI(CAM_MCT_MODULE, "Linking iface->isp");
        ret = mct_stream_link_modules(stream, iface, isp, NULL);
      }
    } else {

      if (pproc) {
        mct_list_t  *lport = NULL;
        mct_port_t  *port = NULL;
        uint32_t rc;

        lport = mct_list_find_custom(MCT_MODULE_SINKPORTS(pproc),
          &stream->streaminfo, mct_pproc_sink_port_caps_reserve);
        if (!lport) {
          CLOGE(CAM_MCT_MODULE, "failed: to caps reserve");
          return FALSE;
        }
        /* Extract pproc port from mct list */
        port = (mct_port_t *)(lport->data);
        if (!port) {
          CLOGE(CAM_MCT_MODULE, "failed: reserved port NULL");
          return  FALSE;
        }
        /* Add identity in port's children */
        rc = mct_port_add_child(stream->streaminfo.identity, port);
        if (rc == FALSE) {
          CLOGE(CAM_MCT_MODULE, "failed: to add child");
          return FALSE;
        }
      }

      if (pproc == NULL && imglib == NULL)
        return FALSE;

        /* Start linking modules
       * pproc (if present) will always be the source
       * If all three present connect 'pproc to imglib'  and 'pproc to hdr'
       * only imglib and hdr is not a valid combination
       */
      if (pproc && imglib) {

        CLOGD(CAM_MCT_MODULE, "Linking pproc and MultiFrame Process");
        pproc->set_mod(pproc, MCT_MODULE_FLAG_SOURCE,
          stream->streaminfo.identity);
        imglib->set_mod(imglib, MCT_MODULE_FLAG_SINK,
          stream->streaminfo.identity);
        ret = mct_stream_link_modules(stream, pproc, imglib, NULL);
      }
      else {
      /* We have only one module to the stream
                 and we can not establish the link */
        CLOGI(CAM_MCT_MODULE, "Linking single module");
        if (imglib)
          single_module = imglib;
        else if (pproc)
          single_module = pproc;

        single_module->set_mod(single_module, MCT_MODULE_FLAG_SOURCE,
          stream->streaminfo.identity);
        CLOGI(CAM_MCT_MODULE, "single_module=%p", single_module);
        /*add module to the stream*/
        MCT_STREAM_LOCK(stream);
        MCT_STREAM_CHILDREN(stream) = mct_list_append(
          MCT_STREAM_CHILDREN(stream), single_module, NULL, NULL);
        if (!MCT_OBJECT_CHILDREN(stream)) {
          CLOGE(CAM_MCT_MODULE, "Error configuring single module");
          MCT_STREAM_UNLOCK(stream);
          return FALSE;
        }

        /* If module is added to the stream set his parent */

        MCT_MODULE_PARENT(single_module) =
          mct_list_append(MCT_MODULE_PARENT(single_module),
                          stream, NULL, NULL);
        if (!MCT_MODULE_PARENT(single_module)) {
          CLOGE(CAM_MCT_MODULE, "Couldn't append");
          MCT_STREAM_UNLOCK(stream);
          return FALSE;
        }
        MCT_STREAM_UNLOCK(stream);
      }
    }
  return TRUE;

  }
  default:
    CLOGE(CAM_MCT_MODULE, "Trying to link unknown stream type: %d",
      stream_info->stream_type);
    return TRUE;
  } /* switch (stream->streaminfo.stream_type) */

  if (pipeline->deferred_state != CAM_DEFER_START) {
    ret = mct_stream_link_module_array(stream,link_mod);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "link failed");
      return FALSE;
    }
  } else {
    mct_event_t cmd_event;
    mct_event_control_t event_data;

    event_data.type = MCT_EVENT_CONTROL_START_LINK;
    event_data.control_event_data = link_mod;
    event_data.size = size;
    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event");
      return FALSE;
    }

    if (pipeline->send_event) {
      CLOGE(CAM_MCT_MODULE, "DEFER LINKING Event = stream %d session %d",
          stream->streaminfo.stream_type, pipeline->session);
      ret = pipeline->send_event(pipeline, stream->streamid, &cmd_event);
    }
  }
  return TRUE;
}

/** mct_stream_start_unlink:
 *    @stream:
 *    @module1:
 *    @module2:
 *
 **/
static void mct_stream_start_unlink(mct_stream_t *stream,
  mct_module_t *module1, mct_module_t *module2)
{
  mct_list_t *sinkports = NULL;
  mct_port_t *port = NULL;
  CLOGD(CAM_MCT_MODULE, "Un-linking %s and %s",
    MCT_MODULE_NAME(module1), MCT_MODULE_NAME(module2));

  /*Adding a check when the src module is connected via
       port and not through process_event, unreserving
       the sinkport of the module*/

  sinkports = mct_list_find_custom(MCT_MODULE_SINKPORTS(module1),
    &stream->streaminfo.identity, mct_port_find_port_with_matching_identity);

  if (sinkports)
    port = (mct_port_t *)(sinkports->data);

  if ((mct_module_find_type(module1,stream->streaminfo.identity)
    == MCT_MODULE_FLAG_SOURCE) && port)
    port->check_caps_unreserve(port, stream->streaminfo.identity);

  mct_module_unlink(stream->streaminfo.identity, module1, module2);
  mct_module_remove_type(module1, stream->streaminfo.identity);
  mct_module_remove_type(module2, stream->streaminfo.identity);

  return;
}

/** mct_stream_find_metadata_buf:
 *    @stream: mct_stream_t object which to receive the event
 *    @event:  mct_event_t object to send to this stream
 *
 *  Used for matching metadata buffer in to stream list.
 *
 *  Return TRUE on success
 **/
static boolean mct_stream_find_metadata_buf(void *data, void *user_data)
{
  boolean check_index;

  check_index = (((mct_stream_map_buf_t *)data)->buf_index ==
      *((uint32_t *)user_data));

  return ((check_index) ? TRUE : FALSE);
}

/** mct_stream_find_stream_buf:
 *    @stream: mct_stream_t object which to receive the event
 *    @event:  mct_event_t object to send to this stream
 *
 *  Used for matching image buffer in to stream list.
 *
 *  Return TRUE on success
 **/
static boolean mct_stream_find_stream_buf(void *data, void *user_data)
{
  boolean check_index;

  check_index = (((mct_stream_map_buf_t *)data)->buf_index ==
    ((mct_serv_ds_msg_t *)user_data)->index) &&
    (((mct_stream_map_buf_t *)data)->buf_type ==
    ((mct_serv_ds_msg_t *)user_data)->buf_type);

  return ((check_index) ? TRUE : FALSE);
}

/** Name: mct_stream_find_cont_buf
 *
 *  Arguments/Fields:
 *    @stream: mct_stream_t object which to receive the event
 *    @event:  mct_event_t object to send to this stream
 *
 *  Used for matching container buffer present in buffer container list.
 *
 *..Return TRUE on success
 **/

static boolean mct_stream_find_cont_buf(void *data, void *user_data)
{
  boolean check_index;

  check_index = (((mct_stream_map_buf_t *)data)->buf_index ==
    ((mct_serv_ds_msg_t *)user_data)->index) &&
    (((mct_stream_map_buf_t *)data)->buf_type ==
    ((mct_serv_ds_msg_t *)user_data)->buf_type);

  return ((check_index) ? TRUE : FALSE);
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  This implementation is incomplete. Need to redo module linking
 **/
static boolean mct_stream_remove_module(mct_stream_t *stream,
  mct_module_t *module)
{
  if (MCT_STREAM_CAST((MCT_MODULE_PARENT(module))->data) != stream)
    return FALSE;

  MCT_STREAM_CHILDREN(stream) =
    mct_list_remove(MCT_STREAM_CHILDREN(stream), module);
  (MCT_STREAM_NUM_CHILDREN(stream))--;

  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_find_bfr_mngr_subdev(int *buf_mgr_fd)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t dev_fd = 0, ioctl_ret;
  boolean ret = FALSE;

  while (1) {
    int32_t num_entities = 0;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      CLOGD(CAM_MCT_MODULE, "Done enumerating media devices");
      break;
    }
    num_media_devices++;
    ioctl_ret = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (ioctl_ret < 0) {
      CLOGE(CAM_MCT_MODULE, "%d Done enumerating media devices");
      close(dev_fd);
      break;
    }
    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      CLOGD(CAM_MCT_MODULE, "entity id %d", entity.id);
      ioctl_ret = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (ioctl_ret < 0) {
        CLOGE(CAM_MCT_MODULE, "Done enumerating media entities");
        ret = FALSE;
        close(dev_fd);
        break;
      }
      num_entities = entity.id;
      CLOGD(CAM_MCT_MODULE, "entity name %s type %x group id %d",
        entity.name, entity.type, entity.group_id);

      CLOGD(CAM_MCT_MODULE, "group_id=%d", entity.group_id);

      if (mct_util_match_subdev_type(entity, MSM_CAMERA_SUBDEV_BUF_MNGR,
        MEDIA_ENT_T_V4L2_SUBDEV)) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);
        CLOGD(CAM_MCT_MODULE, "subdev_name=%s", subdev_name);
        *buf_mgr_fd = open(subdev_name, O_RDWR);
        CLOGD(CAM_MCT_MODULE, "*buf_mgr_fd=%d", *buf_mgr_fd);
        if (*buf_mgr_fd < 0) {
          CLOGE(CAM_MCT_MODULE, "Open subdev failed");
          continue;
        }
        ret = TRUE;
        CLOGD(CAM_MCT_MODULE, "ret=%d", ret);
        close(dev_fd);
        return ret;
      }
    }
    close(dev_fd);
  }
  CLOGD(CAM_MCT_MODULE, "ret=%d", ret);
  return ret;
}

/* HAL3: Helper functions for new meta-data style */
static boolean mct_stream_find_frame_num_id_combo(void *data, void *user_data)
{
  boolean check_index;
  check_index = (((mct_event_frame_request_t *)data)->frame_index ==
      *((unsigned int *)user_data));
  return ((check_index) ? TRUE : FALSE);
}

/** Name: mct_stream_get_frame_number
 *
 *  Arguments/Fields:
 *    @pipeline: Container for frame_id<->frame_num match list
 *    @frame_id: Current frame ID
 *    @frame_number: Frame number to retrieve from list
 *    @modify: If set to TRUE, contents of matched entry are removed
 *
 *  Return: If valid frame_number found return 1 else return 0
 *
 *  Description:
 *  This function returns the matching frame_id in a list storing
 *  corresponding frame_nums issued by HAL if present.
 *
 **/

unsigned int mct_stream_get_frame_number(mct_pipeline_t*pipeline,
  unsigned int frame_id, unsigned int *frame_number, boolean modify)
{
  mct_list_t *holder = NULL;
  mct_event_frame_request_t *data;
  uint32_t frame_number_found = 0;

  MCT_OBJECT_LOCK(pipeline);
  holder = mct_list_find_custom(pipeline->frame_num_idx_list,
        &frame_id, mct_stream_find_frame_num_id_combo);
  MCT_OBJECT_UNLOCK(pipeline);
  if (holder && holder->data) {
    data = (mct_event_frame_request_t *)holder->data;
    *frame_number = data->frame_number;
    frame_number_found = 1;
    CLOGD(CAM_MCT_MODULE, "Found matching frame_number: %d for frame_id %d",
      *frame_number, frame_id);

    if (modify == TRUE) {
      MCT_OBJECT_LOCK(pipeline);
      mct_event_frame_request_t *node =
      (mct_event_frame_request_t *)mct_list_return_node_by_idx(pipeline->frame_num_idx_list,0);
      do {
        if(node->is_valid && (node->frame_index != 0) && (node->frame_index < frame_id))
        {
          pipeline->frame_num_idx_list = mct_list_remove(pipeline->frame_num_idx_list, node);
          free(node);
          node = NULL;
        }
        node = (mct_event_frame_request_t *)
          mct_list_return_node_by_idx(pipeline->frame_num_idx_list,0);
      }while((node != NULL) && (node->frame_index < frame_id));
      pipeline->frame_num_idx_list =
        mct_list_remove(pipeline->frame_num_idx_list, data);
      MCT_OBJECT_UNLOCK(pipeline);
      free(data);
      data = NULL;
    }
  }
  else {
    CLOGD(CAM_MCT_MODULE, "No match found for frame_id %d modify %d", frame_id,modify);
    if (modify == TRUE) {
      MCT_OBJECT_LOCK(pipeline);
      mct_event_frame_request_t *node =
        (mct_event_frame_request_t *)mct_list_return_node_by_idx(pipeline->frame_num_idx_list,0);
      while((node != NULL) && (node->frame_index < frame_id)) {
        if(node->is_valid && (node->frame_index != 0))
        {
          pipeline->frame_num_idx_list = mct_list_remove(pipeline->frame_num_idx_list, node);
          CLOGD(CAM_MCT_MODULE, "node frame_index %d", node->frame_number);
          *frame_number = node->frame_number;
          free(node);
          node = NULL;
        }
        node = (mct_event_frame_request_t *)
          mct_list_return_node_by_idx(pipeline->frame_num_idx_list,0);
      }
      MCT_OBJECT_UNLOCK(pipeline);
      if(*frame_number)
        frame_number_found = FRAME_META_INCORRECT;
      CLOGD(CAM_MCT_MODULE, "No match found for frame_id %d %s smaller framenum %d",
        frame_id,(frame_number_found > 0) ? "found":"not found",*frame_number);
    }
  }
  return frame_number_found;
}

/** Name: mct_stream_map_frame_number:
 *  Arguments/Fields:
 *    @pipeline: Container for frame_id<->frame_num match list
 *    @frame_num: frame_num of the current super-param
 *    @frame_id: Current frame ID to be mapped with incoming request
 *  Description:
 *     This routine stores frame_number associated with the request
 *     with current SOF frame ID for future reference.
 **/
boolean mct_stream_map_frame_number(mct_pipeline_t *pipeline,
  uint32_t frame_num, uint32_t frame_id)
{
  unsigned int overall_delay = 0;
  uint32_t ack_frame_id;
  mct_event_frame_request_t* frame_num_id_entry = NULL;

  overall_delay = pipeline->session_data.max_pipeline_frame_applying_delay +
    pipeline->session_data.max_pipeline_meta_reporting_delay;
  ack_frame_id = frame_id + overall_delay;

  /* Store frame number in list for later reference */
  CLOGH(CAM_MCT_MODULE,
    "SOF %d:session %d: Book-keeping (frame_num, ack_frame_id) = [%d, %d] in list",
     frame_id, pipeline->session, frame_num, ack_frame_id);

  ATRACE_BEGIN_SNPRINTF(64, "book-keeping SOF %d frame %d, ack_frame %d",
    frame_id, frame_num, ack_frame_id);

  frame_num_id_entry = (mct_event_frame_request_t*)malloc(
    sizeof(mct_event_frame_request_t));
  if(frame_num_id_entry) {
    frame_num_id_entry->frame_index = ack_frame_id;
    frame_num_id_entry->frame_number = frame_num;
    frame_num_id_entry->is_valid = 1;
    MCT_OBJECT_LOCK(pipeline);
    pipeline->frame_num_idx_list = mct_list_append(
      pipeline->frame_num_idx_list, frame_num_id_entry, NULL, NULL);
    pipeline->max_ack_id = ack_frame_id;
    MCT_OBJECT_UNLOCK(pipeline);
    if(is_supermct_exist(pipeline))
    {
        supermct_update_ack_id(frame_id);
    }
  } else {
    ATRACE_END();
    return FALSE;
  }
  ATRACE_END();
  return TRUE;
}

/** Name: mct_stream_update_frame_id
 *
 *  Arguments/Fields:
 *    @pipeline: Container for frame_id<->frame_num match list
 *    @cur_frame_id: Current frame ID
 *    @new_frame_id: Frame ID to set in list
 *  Return: If valid frame_number found return 1 else return 0
 *
 *  Description:
 *  This function checks for matching frame_id in a list storing
 *  corresponding frame_nums issued by HAL. If found, the frame_id
 *  entry is updated with passed value from caller.
 **/

boolean mct_stream_update_frame_id(mct_pipeline_t*pipeline,
  unsigned int cur_frame_id, unsigned int new_frame_id)
{
  mct_list_t *holder = NULL;
  mct_event_frame_request_t *data;
  unsigned int frame_number = 0;
  boolean frame_number_set = FALSE;

  CLOGD(CAM_MCT_MODULE,
        "Looking for match for cur_frame_id = %d", cur_frame_id);
  MCT_OBJECT_LOCK(pipeline);
  holder = mct_list_find_custom(pipeline->frame_num_idx_list,
      &cur_frame_id, mct_stream_find_frame_num_id_combo);

  if (holder && holder->data) {
    data = (mct_event_frame_request_t *)holder->data;
      data->frame_index = new_frame_id;
      frame_number = data->frame_number;
      frame_number_set = TRUE;
      CLOGD(CAM_MCT_MODULE,
            "Updated entry: (frame_number, frame_id) =  [%d, %d]",
         frame_number, new_frame_id);
  }
  else
    CLOGD(CAM_MCT_MODULE, "No match found for frame_id %d", cur_frame_id);
  MCT_OBJECT_UNLOCK(pipeline);
  return frame_number_set;

}

static boolean mct_stream_update_frame_id_by(void *data, void *user_data)
{
  mct_event_frame_request_t *req = data;
  unsigned int *delay = user_data;
  ATRACE_BEGIN_SNPRINTF(64, "book-keeping RemapFrame frame %d,frameIndex %d",
    req->frame_number, req->frame_index);
  req->frame_index += *delay;
  CLOGI(CAM_MCT_MODULE, "(fr_num, fr_id) = (%u, %u)", req->frame_number, req->frame_index);
  ATRACE_END();
  return TRUE;
}


boolean mct_stream_update_all_frame_id_by(mct_pipeline_t *pipeline,
  unsigned int delay, unsigned int cur_sof_id)
{
  CLOGI(CAM_MCT_MODULE, "Delay all frame by %u", delay);
  MCT_OBJECT_LOCK(pipeline);
  mct_list_t *frame_idx_list = pipeline->frame_num_idx_list;
  while (frame_idx_list) {
    mct_event_frame_request_t* req = frame_idx_list->data;
    if (req->frame_index <= cur_sof_id) {
      if (frame_idx_list->next) {
        frame_idx_list = *(frame_idx_list->next);
      } else {
        frame_idx_list = NULL;
      }
    } else {
      break;
    }
  }
  if (!frame_idx_list) {
    CLOGI(CAM_MCT_MODULE, "Update not required");
    MCT_OBJECT_UNLOCK(pipeline);
    return TRUE;
  }
  mct_list_traverse(frame_idx_list,
    mct_stream_update_frame_id_by, &delay);
  pipeline->max_ack_id += delay;
  MCT_OBJECT_UNLOCK(pipeline);
  return TRUE;
}

/** Name: mct_stream_free_frame_num_list
 *
 *  Arguments/Fields:
 *    @data: Current entry in list
 *  Return: TRUE if entry successfully removed and deleted from list
 *
 *  Description:
 *  Frees contents of each node of frame_num_idx_list.
 **/

boolean mct_stream_free_frame_num_list(void *data, void *user_data __unused)
{
  mct_event_frame_request_t *request =
    (mct_event_frame_request_t *)data;
  if (request) {
    free(request);
    request = NULL;
  }
  return TRUE;
}

/** remove_metadata_entry:
 *    @
 *    @
 *
 **/
void remove_metadata_entry(int meta_type, metadata_buffer_t *m_table)
{
  void* dst;
  if ((meta_type >= CAM_INTF_PARM_MAX) || (NULL == m_table)) {
    CLOGE(CAM_MCT_MODULE, "Invalid input meta_type: %d, m_table: %p",
      meta_type, m_table);
    return;
  }

  dst = get_pointer_of(meta_type, m_table);
  if(dst != NULL) {
    m_table->is_valid[meta_type] = 0;
  }
  return;
}

/** add_metadata_entry:
 *    @
 *    @
 *
 **/
void add_metadata_entry(int meta_type, uint32_t meta_length,
  void *meta_value, metadata_buffer_t *m_table)
{
  void* dst;
  if ((meta_type >= CAM_INTF_PARM_MAX) || (NULL == meta_value) ||
      (NULL == m_table)) {
      CLOGE(CAM_MCT_MODULE,
            "Invalid input meta_type: %d, meta_value: %p, m_table: %p",
            meta_type, meta_value, m_table);
    return;
  }

  /* Validate the length */
  if (meta_length > get_size_of(meta_type)) {
    CLOGE(CAM_MCT_MODULE,
          "source size: %d greater than destination size: %d for %d",
          meta_length, get_size_of(meta_type), meta_type);
    return;
  }

  dst = get_pointer_of(meta_type, m_table);
  if(dst != NULL){
    memcpy(dst, meta_value, meta_length);
    m_table->is_valid[meta_type] = 1;
  }

  return;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_send_event_stream_on(void *data,
  void *user_data __unused)
{
  boolean ret = TRUE;
  mct_stream_t *stream = (mct_stream_t *)data;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

  event_data.type = MCT_EVENT_CONTROL_STREAMON;
  event_data.size = 0;
  event_data.control_event_data = (void *)&stream->streaminfo;
  CLOGI(CAM_MCT_MODULE, "stream_type = %d stream state = %d",
    stream->streaminfo.stream_type, stream->state);

  if (MCT_ST_STATE_PENDING_RESTART == stream->state) {
    event_data.type = MCT_EVENT_CONTROL_STREAMON;
    event_data.control_event_data = (void *)&stream->streaminfo;
    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
                                 (pack_identity(
                                    MCT_PIPELINE_SESSION(
                                    MCT_PIPELINE_CAST(
                                    MCT_OBJECT_PARENT(stream)->data)),
                                    stream->streamid)),
                                  MCT_EVENT_DOWNSTREAM,
                                  &event_data, &cmd_event);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event");
    }

    ret = stream->send_event(stream, &cmd_event);
    if (ret) {
      stream->state = MCT_ST_STATE_RUNNING;
    } else {
      CLOGE(CAM_MCT_MODULE, "Stream ON failed for stream 0x%x",
        stream->streaminfo.identity);
    }
  } else {
    CLOGI(CAM_MCT_MODULE, "Stream state %d. Skipping", stream->state);
  }

  return ret;
}

/**Name: mct_stream_send_event_stream_off
 *
 * Arguments/Fields:
 * @data: structure of mct_stream_t
 * @user_data: unused
 *
 *  Return: boolean
 *    TRUE on success, FALSE on failure
 *
 * Description:
 *      This function sent event for stream off
 **/
static boolean mct_stream_send_event_stream_off(void *data,
void *user_data __unused)
{
  boolean ret = TRUE;
  mct_stream_t *stream = (mct_stream_t *)data;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

  event_data.type = MCT_EVENT_CONTROL_STREAMOFF;
  event_data.size = 0;
  event_data.control_event_data = (void *)&stream->streaminfo;
  CLOGI(CAM_MCT_MODULE, "stream_type = %d stream state = %d",
    stream->streaminfo.stream_type, stream->state);

  if (MCT_ST_STATE_RUNNING == stream->state) {
    event_data.type = MCT_EVENT_CONTROL_STREAMOFF;
    event_data.size = 0;
    event_data.control_event_data = (void *)&stream->streaminfo;

    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(
        MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data)),
        stream->streamid)),
        MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event");
    }
    ret = stream->send_event(stream, &cmd_event);
    if (ret) {
      stream->state = MCT_ST_STATE_PENDING_RESTART;
    } else {
      CLOGE(CAM_MCT_MODULE, "Stream OFF failed for stream 0x%x",
        stream->streaminfo.identity);
    }
  } else {
    CLOGI(CAM_MCT_MODULE, "Stream state %d. Skipping", stream->state);
  }

  return ret;
}

static boolean mct_stream_send_event_metadata_update (
  mct_pipeline_t *pipeline, void *metadata)
{
  boolean ret = TRUE;
  mct_event_control_t event_data;
  mct_event_t cmd_event;
  mct_stream_t *parm_stream = NULL;
  mct_pipeline_get_stream_info_t info;

  if (!pipeline || !metadata) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: pipeline [%p], metadata [%p]",
      pipeline, metadata);
    return FALSE;
  }
  /* Find session-stream to send ctrl event*/
  info.check_type   = CHECK_INDEX;
  info.stream_index = MCT_SESSION_STREAM_ID;
  parm_stream = mct_pipeline_get_stream(pipeline, &info);
  if (!parm_stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find session stream");
    return FALSE;
  }

  event_data.type = MCT_EVENT_CONTROL_METADATA_UPDATE;
  event_data.control_event_data = metadata;
  event_data.size = 0;
  ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
    parm_stream->streaminfo.identity,
      MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
  if (ret == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Error in packing event ");
  }

  ret = parm_stream->send_event(parm_stream, &cmd_event);
  if (!ret) {
    CLOGE(CAM_MCT_MODULE, "update metadata failed for stream 0x%x",
      parm_stream->streaminfo.identity);
  }

  return ret;
}


/** Name: mct_stream_send_hw_error:
 *  Arguments/Fields:
 *    @parm_stream: Session stream on which super events are sent
 *
 *  Description:
 *    This function sends HW error to all modules
 **/
boolean mct_stream_send_hw_error(mct_stream_t *stream)
{
  boolean ret = TRUE;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Invalid stream ptr");
    return FALSE;
  }
  event_data.type = MCT_EVENT_CONTROL_HW_ERROR;
  event_data.size = 0;
  ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
                                stream->streaminfo.identity,
                                MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);

  if (ret == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Error in packing hw error control event");
  }
  ret = stream->send_event(stream, &cmd_event);
  if (FALSE == ret) {
    CLOGE(CAM_MCT_MODULE, "Failed to send MCT_EVENT_CONTROL_HW_ERROR");
  } else {
    CLOGD(CAM_MCT_MODULE,
          "Successfully sent MCT_EVENT_CONTROL_HW_ERROR");
  }
  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_handle_no_vfe_resources_msg(mct_stream_t *metadata_stream)
{
  boolean ret = TRUE;
  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(metadata_stream))->data);

  /* send stream Off event */
  ret = mct_list_traverse(MCT_PIPELINE_CHILDREN(pipeline),
    mct_stream_send_event_stream_off, NULL);

  if (!ret) {
    CLOGE(CAM_MCT_MODULE, "stream off event failed.");
    return ret;
  }

  /* send stream On event */
  ret = mct_list_traverse(MCT_PIPELINE_CHILDREN(pipeline),
    mct_stream_send_event_stream_on, NULL);

  if (!ret) {
    CLOGE(CAM_MCT_MODULE, "stream off event failed.");
    return ret;
  }

  return ret;
}

static boolean mct_handle_no_vfe_resources_msg_v3(mct_stream_t *metadata_stream)
{
  boolean ret = TRUE;
  mct_bus_msg_t *p_bus_msg = NULL;
  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(metadata_stream))->data);

  /* send stream Off event */
  ret = mct_list_traverse(MCT_PIPELINE_CHILDREN(pipeline),
    mct_stream_send_event_stream_off, NULL);

  if (!ret) {
    CLOGE(CAM_MCT_MODULE, "stream off event failed.");
    return ret;
  }

  pthread_mutex_lock(&pipeline->bus->priority_q_lock);
  mct_bus_priority_queue_flush(pipeline->bus);
  pthread_mutex_unlock(&pipeline->bus->priority_q_lock);

  /* send stream On event */
  ret = mct_list_traverse(MCT_PIPELINE_CHILDREN(pipeline),
    mct_stream_send_event_stream_on, NULL);

  if (!ret) {
    CLOGE(CAM_MCT_MODULE, "stream on event failed.");
    return ret;
  }

  pthread_mutex_lock(&pipeline->bus->priority_q_lock);
  mct_bus_priority_queue_flush(pipeline->bus);
  pthread_mutex_unlock(&pipeline->bus->priority_q_lock);

  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_handle_error_msg(mct_stream_t *stream, void *msg)
{
    boolean ret = TRUE;
    mct_pipeline_t *pipeline = NULL;
    mct_bus_msg_error_message_t *err_msg =
      (mct_bus_msg_error_message_t *)(msg);
    switch (*err_msg) {
      case MCT_ERROR_MSG_RSTART_VFE_STREAMING:
      case MCT_ERROR_MSG_RESUME_VFE_STREAMING: {
        pipeline = (MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)->data));
        if (!pipeline) {
          CDBG_ERROR("%s: Not valid pipeline for this stream 0x%x",
            __func__, stream->streaminfo.identity);
          return FALSE;
        }
        if (pipeline->hal_version == CAM_HAL_V1)
          ret = mct_handle_no_vfe_resources_msg(stream);
        else
          pipeline->int_streamoff = 1;
      }
      break;
      case MCT_ERROR_MSG_INT_RESTART_STREAMING:
        ret = mct_handle_no_vfe_resources_msg_v3(stream);
      break;
      default:
        CLOGE(CAM_MCT_MODULE, "Unsupported message type %d", *err_msg);
        ret = FALSE;
        break;
    }
    return ret;
}

/** Name: mct_stream_fill_metadata_buffer
 *
 *  Arguments/Fields:
 *    @pdst: Metadata buffer allocated by HAL
 *    @psrc: Bus messages sent by modules
 *    @type: Bus message type
 *    @size: Bus message size
 *    @local: Session metadata info
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    This method fills the pre-allocated HAL buffer
 *    with contents of bus messages sent by modules.
 **/
boolean mct_stream_fill_metadata_buffer(metadata_buffer_t *pdst,
  void *psrc, mct_bus_msg_type_t type, unsigned int size,
  mct_stream_session_metadata_info *local)
{

  if (!pdst || !psrc) {
    CLOGE(CAM_MCT_MODULE, "buf is null");
    return FALSE;
  }
  uint8_t valid_flag = 0;
  switch (type) {
  case MCT_BUS_MSG_SENSOR_AF_STATUS:
  case MCT_BUS_MSG_Q3A_AF_STATUS: {
    mct_bus_msg_af_status_t *af_msg = (mct_bus_msg_af_status_t *)psrc;
    cam_auto_focus_data_t focus_data;
    focus_data.focus_dist = af_msg->f_distance;
    focus_data.focus_state = (cam_af_state_t)af_msg->focus_state;
    focus_data.focus_mode= af_msg->focus_mode;
    valid_flag = 1;
    add_metadata_entry(CAM_INTF_META_AUTOFOCUS_DATA,
      sizeof(cam_auto_focus_data_t), &focus_data, pdst);
    }
    break;

  case MCT_BUS_MSG_AUTO_SCENE_INFO: {
    size_t idx = 0;
    mct_bus_msg_asd_decision_t *asd_msg =
                                 (mct_bus_msg_asd_decision_t *)psrc;
    cam_asd_decision_t asd_scene_info;
    asd_scene_info.detected_scene = asd_msg->detected_scene;
    asd_scene_info.max_n_scenes = asd_msg->max_n_scenes;
    for (idx = 0; idx < S_MAX; idx++) {
      asd_scene_info.scene_info[idx].detected =
        asd_msg->scene_info[idx].detected;
      asd_scene_info.scene_info[idx].confidence =
        asd_msg->scene_info[idx].confidence;
      asd_scene_info.scene_info[idx].auto_compensation =
        asd_msg->scene_info[idx].auto_compensation;
    }
    add_metadata_entry(CAM_INTF_META_ASD_SCENE_INFO,
      sizeof(cam_asd_decision_t), &asd_scene_info, pdst);

    /* Report HDR */
    cam_asd_hdr_scene_data_t asd_hdr_scene_data;
    asd_hdr_scene_data.is_hdr_scene = asd_msg->scene_info[S_HDR].detected;
    asd_hdr_scene_data.hdr_confidence = asd_msg->scene_info[S_HDR].confidence;
    add_metadata_entry(CAM_INTF_META_ASD_HDR_SCENE_DATA,
      sizeof(cam_asd_hdr_scene_data_t), &asd_hdr_scene_data, pdst);
    }
    break;

  case MCT_BUS_MSG_FACE_INFO:
    add_metadata_entry(CAM_INTF_META_FACE_DETECTION,
      sizeof(cam_face_detection_data_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_FACE_INFO_FACE_LANDMARKS:
    add_metadata_entry(CAM_INTF_META_FACE_LANDMARK,
      sizeof(cam_face_landmarks_data_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_FACE_INFO_BLINK:
    add_metadata_entry(CAM_INTF_META_FACE_BLINK,
      sizeof(cam_face_blink_data_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_FACE_INFO_SMILE:
    add_metadata_entry(CAM_INTF_META_FACE_SMILE,
      sizeof(cam_face_smile_data_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_FACE_INFO_GAZE:
    add_metadata_entry(CAM_INTF_META_FACE_GAZE,
      sizeof(cam_face_gaze_data_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_FACE_INFO_CONTOUR:
    add_metadata_entry(CAM_INTF_META_FACE_CONTOUR,
      sizeof(cam_face_contour_data_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_FACE_INFO_RECOG:
    add_metadata_entry(CAM_INTF_META_FACE_RECOG,
      sizeof(cam_face_recog_data_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_AF: {
    mct_bus_msg_af_t *af_msg = (mct_bus_msg_af_t *)psrc;

    if (af_msg->af_roi_valid) {
      add_metadata_entry(CAM_INTF_META_AF_ROI, sizeof(cam_area_t),
         &af_msg->af_roi, pdst);
      add_metadata_entry(CAM_INTF_META_AF_DEFAULT_ROI, sizeof(cam_rect_t),
         &af_msg->default_roi, pdst);
    }
  }
    break;
  case MCT_BUS_MSG_AF_UPDATE: {
    mct_bus_msg_af_update_t *af_msg = (mct_bus_msg_af_update_t *)psrc;
    cam_focus_distances_info_t* focus_info = &af_msg->f_distance;
    float focusRange[2];
    focusRange[0] = focus_info->focus_distance[CAM_FOCUS_DISTANCE_NEAR_INDEX];
    focusRange[1] = focus_info->focus_distance[CAM_FOCUS_DISTANCE_FAR_INDEX];

    float focusDistance = focus_info->focus_distance
      [CAM_FOCUS_DISTANCE_OPTIMAL_INDEX];

    add_metadata_entry(CAM_INTF_META_LENS_FOCUS_DISTANCE, sizeof(float),
      &focusDistance, pdst);
    add_metadata_entry(CAM_INTF_META_LENS_FOCUS_RANGE, sizeof(focusRange),
      focusRange, pdst);
  }
  break;

  case MCT_BUS_MSG_AF_IMMEDIATE: {
    mct_bus_msg_af_immediate_t *af_msg = (mct_bus_msg_af_immediate_t *)psrc;
    add_metadata_entry(CAM_INTF_META_FOCUS_VALUE, sizeof(float),
      &af_msg->focus_value, pdst);
    add_metadata_entry(CAM_INTF_META_SPOT_LIGHT_DETECT, sizeof(uint8_t),
      &af_msg->spot_light_detected, pdst);
    add_metadata_entry(CAM_INTF_META_LENS_STATE, sizeof(cam_af_lens_state_t),
      &af_msg->lens_state, pdst);
    add_metadata_entry(CAM_INTF_META_AF_STATE, sizeof(uint8_t),
       &af_msg->af_state, pdst);
    add_metadata_entry(CAM_INTF_META_AF_TRIGGER, sizeof(cam_trigger_t),
      &af_msg->af_trigger, pdst);
    add_metadata_entry(CAM_INTF_PARM_FOCUS_MODE, sizeof(uint8_t),
      &af_msg->focus_mode, pdst);
    add_metadata_entry(CAM_INTF_AF_STATE_TRANSITION, sizeof(uint8_t),
      &af_msg->force_update, pdst);
    add_metadata_entry(CAM_INTF_META_FOCUS_DEPTH_INFO, sizeof(uint8_t),
      &af_msg->is_depth_based_focus, pdst);
    add_metadata_entry(CAM_INTF_META_AF_OBJ_DIST_CM, sizeof(int32_t),
      &af_msg->object_distance_cm, pdst);

    cam_focus_pos_info_t cur_pos;
    cur_pos.diopter = af_msg->diopter;
    cur_pos.scale = af_msg->scale_ratio;

     add_metadata_entry(CAM_INTF_META_FOCUS_POSITION,
      sizeof (cam_focus_pos_info_t), &cur_pos, pdst);

    cam_focal_length_ratio_t focal_length_ratio;
    focal_length_ratio.focalLengthRatio = af_msg->focal_length_ratio;
    add_metadata_entry(CAM_INTF_META_AF_FOCAL_LENGTH_RATIO,
                       sizeof (cam_focal_length_ratio_t),
                       &focal_length_ratio, pdst);

    cam_af_focus_pos_t focus_pos;
    memset(&focus_pos, 0, sizeof(focus_pos));
    focus_pos.lens_shift_um = af_msg->lens_shift_um;
    focus_pos.object_distance_cm = af_msg->object_distance_cm;
    focus_pos.near_field_cm = af_msg->near_field_cm;
    focus_pos.far_field_cm = af_msg->far_field_cm;

    add_metadata_entry(CAM_INTF_META_AF_FOCUS_POS,
      sizeof (cam_af_focus_pos_t), &focus_pos, pdst);

  }
    break;

  case MCT_BUS_MSG_HIST_STATS_INFO: {
    mct_bus_msg_isp_bhist_stats_t *bhist_msg =
      (mct_bus_msg_isp_bhist_stats_t *)psrc;
    add_metadata_entry(CAM_INTF_META_STATS_HISTOGRAM_MODE,
      sizeof(int32_t), &bhist_msg->bhist_meta_enable, pdst);
    add_metadata_entry(CAM_INTF_META_HISTOGRAM,
      sizeof(cam_hist_stats_t), &bhist_msg->bhist_info, pdst);
  }
    break;

  case MCT_BUS_MSG_PREPARE_HW_DONE:
    add_metadata_entry(CAM_INTF_META_PREP_SNAPSHOT_DONE,
      sizeof(uint32_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_PREPARE_HDR_ZSL_DONE:
    add_metadata_entry(CAM_INTF_META_PREP_SNAPSHOT_DONE,
      sizeof(cam_prep_snapshot_state_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_ZSL_TAKE_PICT_DONE:
    add_metadata_entry(CAM_INTF_META_GOOD_FRAME_IDX_RANGE,
      sizeof(cam_frame_idx_range_t), psrc, pdst);
    remove_metadata_entry(CAM_INTF_META_PREP_SNAPSHOT_DONE, pdst);
    break;

  case MCT_BUS_MSG_ISP_SESSION_CROP: {
    uint8_t index = 0;
    cam_crop_data_t crop_data;
    memset(&crop_data, 0, sizeof(cam_crop_data_t));
    mct_bus_msg_session_crop_info_t *session_crop_data =
      (mct_bus_msg_session_crop_info_t *)psrc;
    for (index = 0; index < session_crop_data->num_of_streams &&
      index < MAX_NUM_STREAMS; index++) {
      crop_data.crop_info[index].stream_id =
        session_crop_data->crop_info[index].stream_id;
      crop_data.crop_info[index].crop.left =
        (int32_t)session_crop_data->crop_info[index].x;
      crop_data.crop_info[index].crop.top =
        (int32_t)session_crop_data->crop_info[index].y;
      crop_data.crop_info[index].crop.width =
        (int32_t)session_crop_data->crop_info[index].crop_out_x;
      crop_data.crop_info[index].crop.height =
        (int32_t)session_crop_data->crop_info[index].crop_out_y;
      crop_data.crop_info[index].roi_map.left =
        (int32_t)session_crop_data->crop_info[index].x_map;
      crop_data.crop_info[index].roi_map.top =
        (int32_t)session_crop_data->crop_info[index].y_map;
      crop_data.crop_info[index].roi_map.width =
        (int32_t)session_crop_data->crop_info[index].width_map;
      crop_data.crop_info[index].roi_map.height =
        (int32_t)session_crop_data->crop_info[index].height_map;
      crop_data.crop_info[index].user_zoom =
        (uint32_t)session_crop_data->crop_info[index].user_zoom;
      crop_data.crop_info[index].stream_zoom =
        (uint32_t)session_crop_data->crop_info[index].stream_zoom;
      crop_data.crop_info[index].scale_ratio =
        session_crop_data->crop_info[index].scale_ratio;
    }
    crop_data.num_of_streams = session_crop_data->num_of_streams;
    add_metadata_entry(CAM_INTF_META_CROP_DATA,
      sizeof(cam_crop_data_t), &crop_data, pdst);
  }
  break;

  case MCT_BUS_MSG_ISP_STREAM_CROP: {
    mct_bus_msg_stream_crop_t *crop_msg = (mct_bus_msg_stream_crop_t *)psrc;
    cam_crop_data_t crop_data;
    uint8_t num_of_streams = 0;
    uint8_t i;
    memset(&crop_data, 0, sizeof(cam_crop_data_t));

    IF_META_AVAILABLE(cam_crop_data_t, temp_crop_data,
      CAM_INTF_META_CROP_DATA, pdst) {
      num_of_streams = temp_crop_data->num_of_streams;
      for (i = 0; i < num_of_streams && i < MAX_NUM_STREAMS; i++) {
        crop_data.crop_info[i] = temp_crop_data->crop_info[i];
      }
    }
    crop_data.crop_info[num_of_streams].stream_id = crop_msg->stream_id;
    crop_data.crop_info[num_of_streams].crop.left = (int32_t)crop_msg->x;
    crop_data.crop_info[num_of_streams].crop.top = (int32_t)crop_msg->y;
    crop_data.crop_info[num_of_streams].crop.width
                                         = (int32_t)crop_msg->crop_out_x;
    crop_data.crop_info[num_of_streams].crop.height
                                         = (int32_t)crop_msg->crop_out_y;
    crop_data.crop_info[num_of_streams].roi_map.left
                                         = (int32_t)crop_msg->x_map;
    crop_data.crop_info[num_of_streams].roi_map.top
                                         = (int32_t)crop_msg->y_map;
    crop_data.crop_info[num_of_streams].roi_map.width
                                         = (int32_t)crop_msg->width_map;
    crop_data.crop_info[num_of_streams].roi_map.height
                                         = (int32_t)crop_msg->height_map;
    crop_data.crop_info[num_of_streams].user_zoom
                                         = (uint32_t)crop_msg->user_zoom;
    crop_data.crop_info[num_of_streams].stream_zoom
                                         = (uint32_t)crop_msg->stream_zoom;
    crop_data.crop_info[num_of_streams].scale_ratio
                                         = crop_msg->scale_ratio;
    if ((num_of_streams + 1) < (uint8_t)(
        sizeof(crop_data.crop_info) / sizeof(crop_data.crop_info[0]))) {
      num_of_streams++;
    }
    crop_data.num_of_streams = num_of_streams;
    add_metadata_entry(CAM_INTF_META_CROP_DATA,
      sizeof(cam_crop_data_t), &crop_data, pdst);
  }
  break;

  case MCT_BUS_MSG_ISP_CROP_REGION:
    add_metadata_entry(CAM_INTF_META_SCALER_CROP_REGION,
      size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_SENSOR_INFO:
    memcpy(&(local->sensor_data), psrc, sizeof(mct_bus_msg_sensor_metadata_t));
    break;

  case MCT_BUS_MSG_SET_STATS_AEC_INFO:
    if (sizeof(local->stats_aec_data) >= size) {
      memcpy(&(local->stats_aec_data), psrc, size);
    }
    break;

  case MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO:
    if (sizeof(local->isp_stats_awb_data) >= size) {
      memcpy(&(local->isp_stats_awb_data), psrc, size);
    }
    break;
  case MCT_BUS_MSG_AEC: {
    mct_bus_msg_aec_t * aec_msg = (mct_bus_msg_aec_t *)psrc;
    if (aec_msg->aec_roi_valid) {
      add_metadata_entry(CAM_INTF_META_AEC_ROI,
        sizeof(cam_area_t), &aec_msg->aec_roi, pdst);
    }
    add_metadata_entry(CAM_INTF_PARM_EXPOSURE_COMPENSATION,
      sizeof(int32_t), &aec_msg->exp_comp, pdst);
    add_metadata_entry(CAM_INTF_PARM_AEC_LOCK, sizeof(uint8_t),
       &aec_msg->ae_lock, pdst);
    add_metadata_entry(CAM_INTF_PARM_FPS_RANGE, sizeof(cam_fps_range_t),
      &aec_msg->fps, pdst);
    add_metadata_entry(CAM_INTF_META_LOW_LIGHT, sizeof(cam_low_light_mode_t),
      &aec_msg->lls_flag, pdst);
  }
  break;
  case MCT_BUS_MSG_AEC_IMMEDIATE: {
    mct_bus_msg_aec_immediate_t * aec_msg =
                               (mct_bus_msg_aec_immediate_t *)psrc;
    add_metadata_entry(CAM_INTF_META_AEC_STATE,
      sizeof(uint8_t),  &aec_msg->aec_state, pdst);
    add_metadata_entry(CAM_INTF_META_AEC_PRECAPTURE_TRIGGER,
      sizeof(cam_trigger_t), &aec_msg->aec_trigger, pdst);
    add_metadata_entry(CAM_INTF_META_AEC_MODE,
      sizeof(uint8_t),  &aec_msg->aec_mode, pdst);
    add_metadata_entry(CAM_INTF_PARM_LED_MODE, sizeof(int32_t),
      &aec_msg->led_mode, pdst);
    add_metadata_entry(CAM_INTF_META_TOUCH_AE_RESULT, sizeof(int32_t),
      &aec_msg->touch_ev_status, pdst);
    add_metadata_entry(CAM_INTF_META_AEC_LUX_INDEX, sizeof(float),
      &aec_msg->lux_index, pdst);
    }
    break;
    case MCT_BUS_MSG_LED_MODE_OVERRIDE:
      add_metadata_entry(CAM_INTF_META_LED_MODE_OVERRIDE,
        sizeof(cam_flash_mode_t), psrc, pdst);
      break;
  case MCT_BUS_MSG_SET_AEC_STATE:
    add_metadata_entry(CAM_INTF_META_AEC_STATE,
      sizeof(cam_ae_state_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_AE_INFO:
    add_metadata_entry(CAM_INTF_META_AEC_INFO,
      size, psrc, pdst);
    break;
  case MCT_BUS_MSG_AWB_IMMEDIATE: {
    mct_bus_msg_awb_immediate_t * awb_msg =
                      (mct_bus_msg_awb_immediate_t *)psrc;
    add_metadata_entry(CAM_INTF_META_AWB_STATE,
      sizeof(int32_t), &awb_msg->awb_state, pdst);
    add_metadata_entry(CAM_INTF_PARM_WHITE_BALANCE,
      sizeof(int32_t), &awb_msg->awb_mode, pdst);
    add_metadata_entry(CAM_INTF_META_AWB_INFO,
      sizeof(cam_awb_params_t), &awb_msg->awb_info, pdst);
    if (sizeof(local->stats_awb_data) >= size) {
      memcpy(&(local->stats_awb_data), psrc, size);
    }
  }
  break;
  case MCT_BUS_MSG_AWB: {
    mct_bus_msg_awb_t * awb_msg = (mct_bus_msg_awb_t *)psrc;
    add_metadata_entry(CAM_INTF_PARM_AWB_LOCK,
      sizeof(uint32_t), &awb_msg->awb_lock, pdst);
  }
  break;
  case MCT_BUS_MSG_AE_EXIF_DEBUG_INFO:
    add_metadata_entry(CAM_INTF_META_EXIF_DEBUG_AE,
      sizeof(cam_ae_exif_debug_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_AWB_EXIF_DEBUG_INFO:
    add_metadata_entry(CAM_INTF_META_EXIF_DEBUG_AWB,
      sizeof(cam_awb_exif_debug_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_AF_EXIF_DEBUG_INFO:
    add_metadata_entry(CAM_INTF_META_EXIF_DEBUG_AF,
      sizeof(cam_af_exif_debug_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_ASD_EXIF_DEBUG_INFO:
    add_metadata_entry(CAM_INTF_META_EXIF_DEBUG_ASD,
      sizeof(cam_asd_exif_debug_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_STATS_EXIF_DEBUG_INFO:
    add_metadata_entry(CAM_INTF_META_EXIF_DEBUG_STATS,
      sizeof(cam_stats_buffer_exif_debug_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_BESTATS_EXIF_DEBUG_INFO:
    add_metadata_entry(CAM_INTF_META_EXIF_DEBUG_BESTATS,
        sizeof(cam_bestats_buffer_exif_debug_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_3A_TUNING_EXIF_DEBUG_INFO:
    add_metadata_entry(CAM_INTF_META_EXIF_DEBUG_3A_TUNING,
      sizeof(cam_q3a_tuning_info_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_BHIST_EXIF_DEBUG_INFO:
    add_metadata_entry(CAM_INTF_META_EXIF_DEBUG_BHIST,
      sizeof(cam_bhist_buffer_exif_debug_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SENSOR_INFO:
    add_metadata_entry(CAM_INTF_META_SENSOR_INFO,
      size, psrc, pdst);
    break;
  case MCT_BUS_MSG_SCENE_MODE: {
    add_metadata_entry(CAM_INTF_PARM_BESTSHOT_MODE,
      sizeof(int32_t), psrc, pdst);
  }
  break;

  case MCT_BUS_MSG_ISP_SOF:
  case MCT_BUS_MSG_ERROR_MESSAGE:
    break;

  case MCT_BUS_MSG_SET_SENSOR_SENSITIVITY:
    add_metadata_entry(CAM_INTF_META_SENSOR_SENSITIVITY,
      sizeof(int32_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_TONE_MAP:
    add_metadata_entry(CAM_INTF_META_TONEMAP_CURVES, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_TONE_MAP_MODE:
    add_metadata_entry(CAM_INTF_META_TONEMAP_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_LENS_SHADING_INFO:
    add_metadata_entry(CAM_INTF_META_LENS_SHADING_MAP, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_LENS_SHADING_MODE:
    add_metadata_entry(CAM_INTF_META_SHADING_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_LENS_SHADING_MAP_MODE:
    add_metadata_entry(CAM_INTF_META_LENS_SHADING_MAP_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_ISP_GAIN:
    add_metadata_entry(CAM_INTF_META_ISP_SENSITIVITY, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_CC_MODE:
    add_metadata_entry(CAM_INTF_META_COLOR_CORRECT_MODE,size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_CC_TRANSFORM:
    add_metadata_entry(CAM_INTF_META_COLOR_CORRECT_TRANSFORM,
      size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_PRED_CC_TRANSFORM:
    add_metadata_entry(CAM_INTF_META_PRED_COLOR_CORRECT_TRANSFORM,
      size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_CC_GAIN:
    add_metadata_entry(CAM_INTF_META_COLOR_CORRECT_GAINS, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_PRED_CC_GAIN:
    add_metadata_entry(CAM_INTF_META_PRED_COLOR_CORRECT_GAINS,
      size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_BLACK_LEVEL_LOCK:
    add_metadata_entry(CAM_INTF_META_BLACK_LEVEL_LOCK, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_CONTROL_MODE:
    add_metadata_entry(CAM_INTF_META_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_ABF_MODE:
    add_metadata_entry(CAM_INTF_META_NOISE_REDUCTION_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_HOT_PIX_MODE:
    add_metadata_entry(CAM_INTF_META_HOTPIXEL_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_CAPTURE_INTENT:
    add_metadata_entry(CAM_INTF_META_CAPTURE_INTENT, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_ZOOM_VALUE:
    add_metadata_entry(CAM_INTF_PARM_USERZOOM, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_CONTRAST:
    add_metadata_entry(CAM_INTF_PARM_CONTRAST, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ISP_SATURATION:
    add_metadata_entry(CAM_INTF_PARM_SATURATION, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_FACEDETECT_MODE:
    add_metadata_entry(CAM_INTF_META_STATS_FACEDETECT_MODE,
      size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_SENSOR_EXPOSURE_TIME:
    add_metadata_entry(CAM_INTF_META_SENSOR_EXPOSURE_TIME,
      sizeof(int64_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SET_SENSOR_FRAME_DURATION:
    add_metadata_entry(CAM_INTF_META_SENSOR_FRAME_DURATION,
      sizeof(int64_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SET_ISP_GAMMA_INFO:
    if (sizeof(local->isp_gamma_data) >= size) {
      memcpy(&(local->isp_gamma_data), psrc, size);
    }
    break;
  case MCT_BUS_MSG_SET_HDR_CURVES_INFO:
    if (sizeof(local->hdr_data) >= size) {
      memcpy(&(local->hdr_data), psrc, size);
    }
    break;
  case MCT_BUS_MSG_SET_ISP_LPM_INFO:
    if (sizeof(local->isp_lpm_data) >= size) {
      memcpy(&(local->isp_lpm_data), psrc, size);
    }
    break;
  case MCT_BUS_MSG_IFACE_METADATA:
    if (sizeof(local->iface_metadata) >= size) {
      memcpy(&(local->iface_metadata), psrc, size);
    }
    break;

  case MCT_BUS_MSG_AFD: {
    mct_bus_msg_afd_t * afd_msg = (mct_bus_msg_afd_t *)psrc;
    add_metadata_entry(CAM_INTF_META_SCENE_FLICKER,
      sizeof(uint8_t), &afd_msg->scene_flicker, pdst);
    add_metadata_entry(CAM_INTF_PARM_ANTIBANDING,
      sizeof(cam_antibanding_mode_type), &afd_msg->antibanding_mode, pdst);
  }
    break;

  case MCT_BUS_MSG_SENSOR_APERTURE:
    add_metadata_entry(CAM_INTF_META_LENS_APERTURE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SENSOR_FOCAL_LENGTH:
    add_metadata_entry(CAM_INTF_META_LENS_FOCAL_LENGTH, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SENSOR_FILTERDENSITY:
    add_metadata_entry(CAM_INTF_META_LENS_FILTERDENSITY, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SENSOR_OPT_STAB_MODE:
    add_metadata_entry(CAM_INTF_META_LENS_OPT_STAB_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SENSOR_FLASH_MODE:
    add_metadata_entry(CAM_INTF_META_FLASH_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SENSOR_FLASH_STATE:
    add_metadata_entry(CAM_INTF_META_FLASH_STATE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_FRAME_DROP:
    add_metadata_entry(CAM_INTF_META_FRAME_DROPPED, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_SHARPNESS:
    add_metadata_entry(CAM_INTF_PARM_SHARPNESS, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_EFFECT:
    add_metadata_entry(CAM_INTF_PARM_EFFECT, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_EDGE_MODE:
    add_metadata_entry(CAM_INTF_META_EDGE_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_NOISE_REDUCTION_MODE:
    add_metadata_entry(CAM_INTF_META_NOISE_REDUCTION_MODE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_WAVELET_DENOISE:
    add_metadata_entry(CAM_INTF_PARM_WAVELET_DENOISE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_TEMPORAL_DENOISE:
    add_metadata_entry(CAM_INTF_PARM_TEMPORAL_DENOISE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_ROTATION:
    add_metadata_entry(CAM_INTF_PARM_ROTATION, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_AE_EZTUNING_INFO:
    add_metadata_entry(CAM_INTF_META_CHROMATIX_LITE_AE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_AWB_EZTUNING_INFO:
    add_metadata_entry(CAM_INTF_META_CHROMATIX_LITE_AWB, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_AF_EZTUNING_INFO:
    add_metadata_entry(CAM_INTF_META_CHROMATIX_LITE_AF, size, psrc, pdst);
    break;
  case MCT_BUS_MSG_ASD_EZTUNING_INFO:
    add_metadata_entry(CAM_INTF_META_CHROMATIX_LITE_ASD, size, psrc, pdst);
    break;
  case MCT_BUS_MSG_ISP_CHROMATIX_LITE:
    add_metadata_entry(CAM_INTF_META_CHROMATIX_LITE_ISP, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_PP_CHROMATIX_LITE:
    add_metadata_entry(CAM_INTF_META_CHROMATIX_LITE_PP, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_ISP_META: {
    isp_meta_t *isp_meta = (isp_meta_t *)psrc;
    uint32_t i;
    tuning_params_t *tuning_params = POINTER_OF_META(CAM_INTF_META_TUNING_PARAMS, pdst);
    tuning_params->tuning_vfe_data_size = 0;
    /* Copy everything other than meta_entry */
    memcpy(&tuning_params->data[TUNING_VFE_DATA_OFFSET +
      tuning_params->tuning_vfe_data_size],
      isp_meta,
      (sizeof(isp_meta_t) - (sizeof(isp_meta_entry_t) * ISP_META_MAX)));
    tuning_params->tuning_vfe_data_size +=
      (sizeof(isp_meta_t) - (sizeof(isp_meta_entry_t) * ISP_META_MAX));

    for (i = 0; i < ISP_META_MAX; i++) {
      if (isp_meta->meta_entry[i].len > 0) {
        /* Copy header of meta_entry[i] */
        memcpy(&tuning_params->data[TUNING_VFE_DATA_OFFSET
          + tuning_params->tuning_vfe_data_size],
          &isp_meta->meta_entry[i],
          (sizeof(isp_meta->meta_entry[i].dump_type) +
           sizeof(isp_meta->meta_entry[i].len) +
           sizeof(isp_meta->meta_entry[i].start_addr)));
        tuning_params->tuning_vfe_data_size +=
          (sizeof(isp_meta->meta_entry[i].dump_type)
          + sizeof(isp_meta->meta_entry[i].len)
          + sizeof(isp_meta->meta_entry[i].start_addr));
        /* Now copy contents of meta_entry[i] */
        memcpy(&tuning_params->data[TUNING_VFE_DATA_OFFSET
        + tuning_params->tuning_vfe_data_size],
        isp_meta->meta_entry[i].isp_meta_dump, isp_meta->meta_entry[i].len);
        tuning_params->tuning_vfe_data_size +=
          isp_meta->meta_entry[i].len;
      }
    }
    break;
  }

  case MCT_BUS_MSG_SENSOR_BET_META: {
    tuning_params_t *tuning_params = POINTER_OF_META(CAM_INTF_META_TUNING_PARAMS, pdst);
    tuning_params->tuning_sensor_data_size = size;
    memcpy(&tuning_params->data[TUNING_SENSOR_DATA_OFFSET], psrc, size);
    break;
  }
  case MCT_BUS_MSG_STATS_AEC_META: {
    tuning_params_t *tuning_params = POINTER_OF_META(CAM_INTF_META_TUNING_PARAMS, pdst);
    tuning_params->tuning_mod1_stats_data_size += size;
    memcpy(&tuning_params->data[TUNING_MOD1_AEC_DATA_OFFSET],
      psrc, size);
    break;
  }

  case MCT_BUS_MSG_STATS_AWB_META: {
    tuning_params_t *tuning_params = POINTER_OF_META(CAM_INTF_META_TUNING_PARAMS, pdst);
    tuning_params->tuning_mod1_stats_data_size += size;
    memcpy(&tuning_params->data[TUNING_MOD1_AWB_DATA_OFFSET],
      psrc, size);
    break;
  }

  case MCT_BUS_MSG_STATS_AF_META: {
    tuning_params_t *tuning_params = POINTER_OF_META(CAM_INTF_META_TUNING_PARAMS, pdst);
    tuning_params->tuning_mod1_stats_data_size += size;
    memcpy(&tuning_params->data[TUNING_MOD1_AF_DATA_OFFSET],
      psrc, size);
    break;
  }

  case MCT_BUS_MSG_META_CURRENT_SCENE:
    add_metadata_entry(CAM_INTF_META_CURRENT_SCENE, size, psrc, pdst);
    break;

  case MCT_BUS_MSG_SENSOR_EXPOSURE_TIMESTAMP:
    add_metadata_entry(CAM_INTF_META_SENSOR_TIMESTAMP,
      sizeof(int64_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_PROFILE_TONE_CURVE:
    add_metadata_entry(CAM_INTF_META_PROFILE_TONE_CURVE, size, psrc, pdst);
    break;
  case MCT_BUS_MSG_NEUTRAL_COL_POINT:
    add_metadata_entry(CAM_INTF_META_NEUTRAL_COL_POINT, size, psrc, pdst);
    break;
  case MCT_BUS_MSG_SET_SENSOR_HDR_MODE:
    add_metadata_entry(CAM_INTF_PARM_SENSOR_HDR, size, psrc, pdst);
    break;
  case MCT_BUS_MSG_LED_REDEYE_REDUCTION_MODE:
    add_metadata_entry(CAM_INTF_PARM_REDEYE_REDUCTION, size, psrc, pdst);
    break;
  case MCT_BUS_MSG_SET_IR_MODE:
    add_metadata_entry(CAM_INTF_META_IR_MODE, size, psrc, pdst);
    break;
  case MCT_BUG_MSG_OTP_WB_GRGB:
    add_metadata_entry(CAM_INTF_META_OTP_WB_GRGB, size, psrc, pdst);
  break;
  case MCT_BUS_MSG_SENSOR_ROLLING_SHUTTER_SKEW:
    add_metadata_entry(CAM_INTF_META_SENSOR_ROLLING_SHUTTER_SKEW,
      sizeof(int64_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_TEST_PATTERN_DATA:
    add_metadata_entry(CAM_INTF_META_TEST_PATTERN_DATA,
      sizeof(cam_test_pattern_data_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SET_CAC_MODE:
    add_metadata_entry(CAM_INTF_PARM_CAC, sizeof(cam_aberration_mode_t),
      psrc, pdst);
  break;
  case MCT_BUS_MSG_BUF_DIVERT_INFO:
    add_metadata_entry(CAM_INTF_BUF_DIVERT_INFO, sizeof(cam_buf_divert_info_t),
      psrc, pdst);
    break;
  case MCT_BUS_MSG_DCRF_RESULT:
    add_metadata_entry(CAM_INTF_META_DCRF, sizeof(cam_dcrf_result_t),
      psrc, pdst);
    break;
  case MCT_BUS_MSG_CAC_STAGE_DONE:
    add_metadata_entry(CAM_INTF_META_CAC_INFO,
      sizeof(cam_cac_info_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_IMG_HYSTERSIS_INFO:
    add_metadata_entry(CAM_INTF_META_IMG_HYST_INFO,
      sizeof(cam_img_hysterisis_info_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_SNAP_CROP_INFO_SENSOR:
    add_metadata_entry(CAM_INTF_META_SNAP_CROP_INFO_SENSOR,
      sizeof(cam_stream_crop_info_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SNAP_CROP_INFO_CAMIF:
    add_metadata_entry(CAM_INTF_META_SNAP_CROP_INFO_CAMIF,
      sizeof(cam_stream_crop_info_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SNAP_CROP_INFO_ISP:
    add_metadata_entry(CAM_INTF_META_SNAP_CROP_INFO_ISP,
      sizeof(cam_stream_crop_info_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SNAP_CROP_INFO_PP:
    add_metadata_entry(CAM_INTF_META_SNAP_CROP_INFO_CPP,
      sizeof(cam_stream_crop_info_t), psrc, pdst);
    break;
#ifdef ASF_OSD
  case MCT_BUS_MSG_TRIGGER_REGION:
   add_metadata_entry(CAM_INTF_META_ASF_TRIGGER_REGION,
     sizeof(cam_asf_trigger_regions_t), psrc, pdst);
   break;
#endif
  case MCT_BUS_MSG_SET_CDS:
    add_metadata_entry(CAM_INTF_PARM_CDS_MODE, sizeof(int32_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SET_IMG_DYN_FEAT: {
    IF_META_AVAILABLE(cam_dyn_img_data_t, dyn_mask,
      CAM_INTF_META_IMG_DYN_FEAT, pdst) {
      dyn_mask->dyn_feature_mask |=
        ((cam_dyn_img_data_t *)psrc)->dyn_feature_mask;
    } else {
      add_metadata_entry(CAM_INTF_META_IMG_DYN_FEAT,
                         sizeof(cam_dyn_img_data_t),
                         psrc, pdst);
    }
    break;
  }
  case MCT_BUS_MSG_SET_VIDEO_STAB_MODE:
    add_metadata_entry(CAM_INTF_META_VIDEO_STAB_MODE,
      sizeof(int32_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_DC_SAC_OUTPUT_INFO:
    add_metadata_entry(CAM_INTF_META_DC_SAC_OUTPUT_INFO,
      sizeof(cam_sac_output_info_t), psrc, pdst);
    memcpy(&local->sac_output_info, psrc, sizeof(cam_sac_output_info_t));
    break;
  case MCT_BUS_MSG_SET_TINTLESS:
    add_metadata_entry(CAM_INTF_PARM_TINTLESS,
      sizeof(int32_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_LED_CAL_RESULT:
    add_metadata_entry(CAM_INTF_META_LED_CALIB_RESULT,
      sizeof(int32_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_RTB_RESULT:
    add_metadata_entry(CAM_INTF_META_RTB_DATA,
      sizeof(cam_rtb_msg_type_t), psrc, pdst);
    break;
   case MCT_BUS_MSG_OIS_READ_DATA:
   {
     mct_custom_data_payload_t* payload = (mct_custom_data_payload_t *)psrc;
     uint8_t *data = (uint8_t*)payload->data;
     cam_ois_data_t oisData;
     memset(&oisData, 0x00, sizeof(oisData));
     if (payload->size < OIS_DATA_MAX_SIZE) {
       oisData.size = payload->size;
     } else {
       oisData.size = OIS_DATA_MAX_SIZE;
     }
     memcpy(&oisData.data[0], data, oisData.size);
     add_metadata_entry(CAM_INTF_META_OIS_READ_DATA,
       sizeof(cam_ois_data_t), &oisData, pdst);
     break;
   }
  default:
    CLOGW(CAM_MCT_MODULE, "Unsupported message type %d", type);
      /* fall through */
  }
  return TRUE;
}

/** Name: mct_stream_put_metadata_buffer
 *
 *  Arguments/Fields:
 *    @stream: Pointer to metadata stream
 *
 *  Return: boolean; TRUE on success; FALSE on failure
 *
 *  Description:
 *  This function put metadata buffer back to
 *  the V4L2 queue.
 **/

boolean mct_stream_put_metadata_buffer(mct_stream_t *stream)
{
  struct msm_buf_mngr_info buf_info;
  int rc;
  boolean ret = TRUE;

  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Input validation failed stream=%p", stream);
    return FALSE;
  }
  /* Return any buffer if still held by MCT */
  if (!stream->metadata_stream.get_buf_err) {
    pthread_mutex_lock(&stream->metadata_stream.metadata_buf_lock);
    buf_info.index = (uint32_t)stream->metadata_stream.current_buf_idx;
    buf_info.frame_id = 0;
    buf_info.stream_id = (uint32_t)stream->streamid;
    buf_info.session_id = MCT_PIPELINE_SESSION(
      MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));
    rc = ioctl(stream->metadata_stream.buf_mgr_dev_fd,
      VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buf_info);
    if (rc < 0) {
      CLOGE(CAM_MCT_MODULE, "put_buf failed - errno: %s! : %d",
        strerror(errno), rc);
      ret = FALSE;
    }
    stream->metadata_stream.get_buf_err = TRUE;
    pthread_mutex_unlock(&stream->metadata_stream.metadata_buf_lock);
  }
  return ret;
}

/** Name: mct_stream_get_metadata_buffer
 *
 *  Arguments/Fields:
 *    @stream: Pointer to metadata stream
 *    @hal_buffer: Holder for new buffer got from V4L2 queue
 *
 *  Return: boolean; TRUE on success; FALSE on failure
 *
 *  Description:
 *  This function fetches a new available metadata buffer from
 *  the V4L2 queue to be used to fill metadata for current SOF.
 *  Additionally, the buffer is also memset before its first use.
 **/
boolean mct_stream_get_metadata_buffer(mct_stream_t *stream,
  metadata_buffer_t **hal_buffer)
{
  boolean ret = TRUE;
  int status = 0;
  mct_list_t *new_buf_holder = NULL;
  mct_stream_map_buf_t *new_buf;
  struct msm_buf_mngr_info get_buf;

  if(!stream) {
    CLOGE(CAM_MCT_MODULE, "NULL stream");
    return FALSE;
  }
  if (stream->state != MCT_ST_STATE_RUNNING) {
    CLOGI(CAM_MCT_MODULE, "Metadata stream state: %d. Returning",
      stream->state);
    return FALSE;
  }
  /* Acquire a new metadata buffer from HAL */
  get_buf.stream_id = (uint32_t)stream->streamid;
  get_buf.session_id = MCT_PIPELINE_SESSION(
          MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));
  get_buf.type = MSM_CAMERA_BUF_MNGR_BUF_PLANAR;

  status = ioctl(stream->metadata_stream.buf_mgr_dev_fd,
    VIDIOC_MSM_BUF_MNGR_GET_BUF, &get_buf);
  if (status < 0) {
    CLOGE(CAM_MCT_MODULE, "Failed to get_buf - errno: %s!!!",
      strerror(errno));
    ret = FALSE;
    stream->metadata_stream.get_buf_err = TRUE;
    stream->metadata_stream.current_buf_idx = -1;
  } else {
    stream->metadata_stream.get_buf_err = FALSE;
    stream->metadata_stream.current_buf_idx = (int32_t)get_buf.index;
    CLOGD(CAM_MCT_MODULE, "Obtained new metadata buffer %d from HAL",
      stream->metadata_stream.current_buf_idx);


    pthread_mutex_lock(&stream->buffers.lock_img_buf);
    new_buf_holder = mct_list_find_custom(
                       stream->buffers.img_buf,
                       &(stream->metadata_stream.current_buf_idx),
                       mct_stream_find_metadata_buf);
    pthread_mutex_unlock(&stream->buffers.lock_img_buf);

    if ((new_buf_holder) && (new_buf_holder->data)) {
      new_buf = new_buf_holder->data;
      *hal_buffer = (metadata_buffer_t *)(new_buf->buf_planes[0].buf);

      clear_metadata_buffer(*hal_buffer);
    } else {
      CLOGE(CAM_MCT_MODULE, "NULL ptr");
      ret = FALSE;
    }
  }
  return ret;
}

/** Name: mct_stream_process_bus_msg
 *
 *  Arguments/Fields:
 *    @stream: Pointer to metadata stream
 *    @priority_bus_msg: Bus msgs that wake up MCT worker thread
 *            and invoke processing on other bus messages.
 *  Return: boolean; TRUE on success; FALSE on failure
 *
 *  Description:
 *  This function handles all bus msgs posted on bus queue.
 *  It also posts metadata corresponding to the current frame to HAL
 *  upon receiving SOF notification from mct_controller.
 **/

boolean mct_stream_process_bus_msg(mct_stream_t *stream,
  mct_bus_msg_t *priority_bus_msg)
{
  struct msm_buf_mngr_info buf_info;
  int status = 0;
  boolean ret = TRUE;
  metadata_buffer_t *current_meta_buf = NULL;
  mct_pipeline_t *pipeline = NULL;
  uint32_t meta_frame_number = 0;
  uint32_t meta_urgent_frame_number = 0;
  uint32_t urgent_frame_num_valid = 0;
  uint32_t frame_num_valid = 0;
  unsigned int overall_delay = 0, urgent_frame_id = 0;
  int meta_frame_id = 0;
  uint8_t enable_lpm = 0;

  if(!stream || !priority_bus_msg) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: stream [%p], bus_msg [%p]",
      stream, priority_bus_msg);
    return FALSE;
  }
  pipeline = (MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)->data));
  if(!pipeline)
  {
    CLOGE(CAM_MCT_MODULE, "FATAL! Pipeline doesn't exist!");
    return FALSE;
  }

  switch (priority_bus_msg->type) {
    case MCT_BUS_MSG_ERROR_MESSAGE:
      ret = mct_handle_error_msg(stream, priority_bus_msg->msg);
      break;
    case MCT_BUS_MSG_DELAY_SUPER_PARAM:
    case MCT_BUS_MSG_FRAME_SKIP:
    {
      mct_bus_msg_delay_dequeue_t st_delay_dequeue  =
        *(mct_bus_msg_delay_dequeue_t *)(priority_bus_msg->msg);
      /* Block dequeuing super-params on next few SOFs as specified. */
      pipeline->delay_dequeue_count = st_delay_dequeue.delay_numframes;

      if (MCT_BUS_MSG_FRAME_SKIP == priority_bus_msg->type) {
        /* Delay reporting metadata by specified number of frames. */
        uint32_t curr_frame_id = st_delay_dequeue.curr_frame_id;
        CLOGI(CAM_MCT_MODULE,
              "Request on frame id %d to delay metadata for %d frame(s)",
              curr_frame_id, st_delay_dequeue.delay_numframes);
        overall_delay =
          pipeline->session_data.max_pipeline_frame_applying_delay +
          pipeline->session_data.max_pipeline_meta_reporting_delay;
        uint32_t delay_frame_id = curr_frame_id + overall_delay;
        uint32_t new_frame_id = delay_frame_id +
          st_delay_dequeue.delay_numframes;
        boolean set = mct_stream_update_frame_id(pipeline,
          delay_frame_id,new_frame_id);
        if(!set) {
          CLOGE(CAM_MCT_MODULE,
                "Couldn't apply delay to metadata ack frame_id %d",
                curr_frame_id);
        }
      }
      break;
    }
    case MCT_BUS_MSG_ISP_SOF:
    case MCT_BUS_INTMSG_PROC_SOF_WO_PARAM:
    case MCT_BUS_INTMSG_PROC_SOF_W_PARAM:
    {
      pthread_mutex_lock(&stream->metadata_stream.metadata_buf_lock);
      if (stream->metadata_stream.get_buf_err == FALSE) {
        /*Update the local metadata buffer pointer */
        current_meta_buf = stream->metadata_stream.metadata_buffer;
        mct_bus_msg_isp_sof_t *isp_sof_bus_msg = priority_bus_msg->msg;

        /* First send metadata to tuning modules */
        if (FALSE == mct_stream_send_event_metadata_update(
                       pipeline, current_meta_buf)) {
          CLOGE(CAM_MCT_MODULE, "Failed to send tuning metadata.");
        }
        /* Populate miscellaneous metadata entries  Its ORed with Stats data as Stats data
             will be be generated by certain testcases.*/
        if (current_meta_buf) {
            tuning_params_t *tuning_params =
                POINTER_OF_META(CAM_INTF_META_TUNING_PARAMS, current_meta_buf);
            if (((tuning_params->tuning_sensor_data_size > 0) &&
            (tuning_params->tuning_vfe_data_size > 0)) ||
            (tuning_params->tuning_mod1_stats_data_size > 0)) {
            current_meta_buf->is_valid[CAM_INTF_META_TUNING_PARAMS] = 1;
          }
        }
        /* urgent_frame_number is looked up in advance
            considering pipeline delay and reporting delay. */
        unsigned int frame_id = isp_sof_bus_msg->frame_id -
        (((isp_sof_bus_msg->kstate_ptr != NULL ) &&
         (isp_sof_bus_msg->kstate_ptr->isp_kstate_ptr != NULL) &&
          (isp_sof_bus_msg->kstate_ptr->isp_kstate_ptr->kernel_sofid > 0))
           ?isp_sof_bus_msg->kstate_ptr->isp_kstate_ptr->dual_cam_drop:0);
        if((isp_sof_bus_msg->kstate_ptr != NULL ) &&
         (isp_sof_bus_msg->kstate_ptr->isp_kstate_ptr != NULL)){
          isp_sof_bus_msg->kstate_ptr->isp_kstate_ptr->dual_cam_drop = 0;
        }

        urgent_frame_id = frame_id +
          pipeline->session_data.max_pipeline_frame_applying_delay +
          pipeline->session_data.max_pipeline_meta_reporting_delay;

        urgent_frame_num_valid = mct_stream_get_frame_number (
          pipeline, urgent_frame_id, &meta_urgent_frame_number, FALSE);
        frame_num_valid = mct_stream_get_frame_number (
          pipeline, frame_id, &meta_frame_number, TRUE);

        add_metadata_entry(CAM_INTF_META_PRIVATE_DATA,
          sizeof(mct_stream_session_metadata_info),
          &(stream->metadata_stream.session_meta), current_meta_buf);
        add_metadata_entry(CAM_INTF_META_FRAME_NUMBER_VALID, sizeof(int32_t),
          &frame_num_valid, current_meta_buf);
        if (frame_num_valid)
          add_metadata_entry(CAM_INTF_META_FRAME_NUMBER, sizeof(uint32_t),
          &meta_frame_number, current_meta_buf);
        add_metadata_entry(CAM_INTF_META_URGENT_FRAME_NUMBER_VALID,
          sizeof(int32_t), &urgent_frame_num_valid,
          current_meta_buf);
        if (pipeline->module_hw_state == MCT_MODULE_STATE_ASLEEP) {
            enable_lpm = 1;
        }
        add_metadata_entry(CAM_INTF_META_DC_LOW_POWER_ENABLE, sizeof(uint8_t),
          &enable_lpm, current_meta_buf);
        if (urgent_frame_num_valid)
          add_metadata_entry(CAM_INTF_META_URGENT_FRAME_NUMBER,
                             sizeof(uint32_t),
                             &meta_urgent_frame_number,
                             current_meta_buf);

        /* Now send the metadata buffer back to HAL meta_frame_id
              tells the frame for which this metadata was generated*/
        meta_frame_id = (int) (frame_id -
          pipeline->session_data.max_pipeline_meta_reporting_delay) < 0 ?
          0 : (frame_id -
          pipeline->session_data.max_pipeline_meta_reporting_delay);
        stream->metadata_stream.current_frame_idx = frame_id;
        buf_info.index = (uint32_t)stream->metadata_stream.current_buf_idx;
        buf_info.frame_id = (unsigned int)meta_frame_id;
        buf_info.stream_id = (uint32_t)stream->streamid;
        buf_info.timestamp = isp_sof_bus_msg->timestamp;
        buf_info.session_id = MCT_PIPELINE_SESSION(
          MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));

        CLOGD(CAM_MCT_MODULE, "Sending metadata buffer %d", buf_info.index);
        status = ioctl(stream->metadata_stream.buf_mgr_dev_fd,
            VIDIOC_MSM_BUF_MNGR_BUF_DONE, &buf_info);
        if (status < 0) {
          CLOGE(CAM_MCT_MODULE,
                "Failed to do buf_done errno: %s!!! for buffer with "
                "index: %d, stream id: %x, session id: %d", strerror(errno),
                buf_info.index, buf_info.stream_id, buf_info.session_id);
          ret = FALSE;
        }
        /* Reset valid metadata buffer flag */
        stream->metadata_stream.get_buf_err = TRUE;
        stream->metadata_stream.current_buf_idx = -1;
        stream->valid_buffer = FALSE;
        pthread_mutex_unlock(&stream->metadata_stream.metadata_buf_lock);
      }
      else {
        pthread_mutex_unlock(&stream->metadata_stream.metadata_buf_lock);
        CLOGE(CAM_MCT_MODULE, "Inconsistent Behavior. We should not be here");
      }
      break;
    }
    default:
      CLOGE(CAM_MCT_MODULE, "case is not implemented for type=%d",
        priority_bus_msg->type);
      break;
  }

  return ret;
}

/** Name: mct_stream_update_metadata_buffer
 *
 *  Arguments/Fields:
 *    @stream: pointer to metadata stream
 *    @bus_msg: bus_msgs which needs to be copied to metadata buffer
 *  Return: boolean; TRUE on success; FALSE on failure
 *
 *  Description:
 *  This function updates the metadata buffer with the bus messages
 *  present in bus_msg queue.  These bus_messages are stored in
 *   queue when no metadata buffer is available to directly write into
 **/

boolean mct_stream_update_metadata_buffer(mct_stream_t *stream,
  metadata_buffer_t *metadata_buffer)
{
  boolean ret = TRUE;
  mct_pipeline_t *pipeline;
  mct_bus_msg_t *bus_msg;

  if (!stream || !metadata_buffer) {
    CLOGE(CAM_MCT_MODULE, "Input validation failed stream=%p \
      metadata_buffer=%p", stream, metadata_buffer);
    return FALSE;
  }
  pipeline = (MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)->data));
  if(!pipeline) {
    CLOGE(CAM_MCT_MODULE, "FATAL! Pipeline doesn't exist!");
    return FALSE;
  }
  if (stream->state != MCT_ST_STATE_RUNNING) {
    CLOGI(CAM_MCT_MODULE, "Metadata stream state= %d. Returning",
      stream->state);
    stream->metadata_stream.metadata_buffer = NULL;
    stream->valid_buffer = FALSE;
    return FALSE;
  }
  pthread_mutex_lock(&stream->metadata_stream.metadata_buf_lock);
  while (1) {
    pthread_mutex_lock(&pipeline->bus->bus_msg_q_lock);
    bus_msg = (mct_bus_msg_t *)
      mct_queue_pop_head(pipeline->bus->bus_queue);
    pthread_mutex_unlock(&pipeline->bus->bus_msg_q_lock);

    if (!bus_msg) {
      break;
    }
    ret = mct_stream_fill_metadata_buffer(metadata_buffer, bus_msg->msg,
      bus_msg->type, bus_msg->size, &stream->metadata_stream.session_meta);
    if (!ret) {
      CLOGE(CAM_MCT_MODULE,
            "Error while filling up metadata buffer type bus msg=%d",
            bus_msg->type);
    }

    if (bus_msg->msg) {
      free(bus_msg->msg);
      bus_msg->msg = NULL;
    }
    if (bus_msg) {
      free(bus_msg);
      bus_msg = NULL;
    }
  }
  add_metadata_entry(CAM_INTF_META_DC_SAC_OUTPUT_INFO,
    sizeof(cam_sac_output_info_t),
    &stream->metadata_stream.session_meta.sac_output_info, metadata_buffer);
  stream->metadata_stream.metadata_buffer = metadata_buffer;
  stream->valid_buffer= TRUE;
  pthread_mutex_unlock(&stream->metadata_stream.metadata_buf_lock);

  return TRUE;
}

/** mct_stream_metadata_ctrl_event:
 *    @stream: Metadata stream
 *    @event: Stream-On/Stream-Off
 *
 *  Return: boolean; TRUE on success; FALSE on failure
 *
 *  Description:
 *  This function does special processing for
 *  stream-on/off on metadata stream in handling HAL
 *  allocated metadata buffers from buf_manager subdev.
 **/
boolean mct_stream_metadata_ctrl_event(mct_stream_t *stream,
  mct_event_t *event)
{
  boolean ret = TRUE;
  int ioctl_ret = 0;
  struct msm_buf_mngr_info buf_info;
  mct_list_t *new_buf_holder = NULL;

  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Not valid pipeline for this stream 0x%x",
      stream->streaminfo.identity);
    return FALSE;
  }

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    switch (event->u.ctrl_event.type) {
    case MCT_EVENT_CONTROL_STREAMON: {
       ret = mct_stream_find_bfr_mngr_subdev(
         &stream->metadata_stream.buf_mgr_dev_fd);
       if (ret == FALSE) {
         CLOGE(CAM_MCT_MODULE, "failed to find buffer manager subdev");
         break;
       }
    }
      break;

    case MCT_EVENT_CONTROL_STREAMOFF: {
      /* Return any buffer if still held by MCT */
      if (!stream->metadata_stream.get_buf_err) {
        pthread_mutex_lock(&stream->metadata_stream.metadata_buf_lock);
        buf_info.index = (uint32_t)stream->metadata_stream.current_buf_idx;
        buf_info.frame_id = 0;
        buf_info.stream_id = (uint32_t)stream->streamid;
        buf_info.session_id = MCT_PIPELINE_SESSION(
          MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));

        ret = ioctl(stream->metadata_stream.buf_mgr_dev_fd,
          VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buf_info);
        if (ret < 0) {
          CLOGE(CAM_MCT_MODULE,
            "Failed to do put_buf during stream off - errno: %s!!! "
            "for buffer info - index: %d, stream id: %d, session id: %d",
            strerror(errno),
            buf_info.index, buf_info.stream_id, buf_info.session_id);
        }
        stream->metadata_stream.get_buf_err = TRUE;
        stream->metadata_stream.metadata_buffer = NULL;
        pthread_mutex_unlock(&stream->metadata_stream.metadata_buf_lock);
      }
      stream->state = MCT_ST_STATE_IDLE;
      close(stream->metadata_stream.buf_mgr_dev_fd);
      stream->metadata_stream.buf_mgr_dev_fd = -1;

      mct_bus_t *bus = pipeline->bus;
      if (!bus)
        return FALSE;

      pthread_mutex_lock(&bus->bus_msg_q_lock);
      mct_bus_queue_flush(bus);
      pthread_mutex_unlock(&bus->bus_msg_q_lock);
      ret = TRUE;
    }
      break;

    default: {
      ret = TRUE;
      CLOGE(CAM_MCT_MODULE, "ret=%d Unsupported cmd %d",
        ret, event->u.ctrl_event.type);
    }
      break;
  } /* ctrl event type switch*/
  }
  break;

  default: {
    CLOGE(CAM_MCT_MODULE, "ret=%d Unsupported event type %d",
      ret, event->type);
    break;
  }
  } /* event type switch*/
  return ret;
}

/**mct_module_find_offline_reprocessing_input_buffer
 *
 * @data1: structure of mct_stream_map_buf_t
 * @data2: buffer index
 *
 *  Return: boolean
 *    TRUE on success, FALSE on failure
 *
 * Description:
 *      This function finds offline reprocess buffer in stream
 **/
static boolean mct_module_find_offline_reprocessing_input_buffer
  (void *data1, void *data2)
{
  mct_stream_map_buf_t *buf_holder = (mct_stream_map_buf_t *)data1;
  uint32_t *buf_index = (uint32_t *)data2;
  if (!buf_holder || !buf_index) {
    CLOGE(CAM_MCT_MODULE, "Failed buf_holder %p buf_index %p",
      buf_holder, buf_index);
    return FALSE;
  }
  if ((buf_holder->buf_index == *buf_index) &&
      (buf_holder->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)) {
    return TRUE;
  }
  return FALSE;
}


/**mct_stream_process_offline_message
 *
 * @stream: structure of mct_stream_t
 * @mct_offline_meta_payload: Offline metadata payload
 *
 *  Return: boolean
 *    TRUE on success, FALSE on failure
 *
 * Description:
 *      This function processes offline metadata payload
 **/
static boolean mct_stream_process_offline_message(mct_stream_t *stream,
  mct_offline_meta_payload *msg)
{
  struct msm_buf_mngr_info get_buf;
  mct_list_t *input_buf_holder = NULL;
  mct_list_t *output_buf_holder = NULL;
  mct_stream_map_buf_t *new_buf;
  mct_stream_t *input_stream = NULL;
  mct_stream_t *output_stream = NULL;
  mct_stream_t *meta_stream = NULL;
  mct_pipeline_get_stream_info_t info;
  uint32_t input_stream_id = 0;
  int status = 0;

  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Not valid pipeline for this stream 0x%x",
      stream->streaminfo.identity);
    return FALSE;
  }

  switch (msg->type) {
    case MCT_OFFLINE_META_INPUT_BUF: {
      mct_offline_meta_input_buf_index *input_buf_index = msg->data;
      if (stream->streaminfo.reprocess_config.pp_type ==
        CAM_ONLINE_REPROCESS_TYPE) {
        /* Getting input stream id from HAL */
        input_stream_id =
          stream->streaminfo.reprocess_config.online.input_stream_id;
        input_stream = mct_pipeline_find_stream_from_stream_id(pipeline,
          input_stream_id);
        if (!input_stream) {
          CLOGE(CAM_MCT_MODULE, "Error in getting input stream id=%d",
            input_stream_id);
          return FALSE;
        }
        /* Get input reprocess buffer for online case */
        pthread_mutex_lock(&input_stream->buffers.lock_img_buf);
        input_buf_holder = mct_list_find_custom(
                           input_stream->buffers.img_buf,
                           &(input_buf_index->buf_index),
                           mct_stream_find_metadata_buf);
        pthread_mutex_unlock(&input_stream->buffers.lock_img_buf);

        if ((input_buf_holder) && (input_buf_holder->data)) {
          new_buf = input_buf_holder->data;
          pipeline->offline_meta.input_metadata_buffer =
            (metadata_buffer_t *)(new_buf->buf_planes[0].buf);
        }
     } else if (stream->streaminfo.reprocess_config.pp_type ==
       CAM_OFFLINE_REPROCESS_TYPE) {
       input_buf_holder =
         mct_list_find_custom(stream->streaminfo.img_buffer_list,
           &stream->streaminfo.parm_buf.reprocess.buf_index,
           mct_module_find_offline_reprocessing_input_buffer);

       if ((input_buf_holder) && (input_buf_holder->data)) {
         new_buf = input_buf_holder->data;
         pipeline->offline_meta.input_metadata_buffer =
           (metadata_buffer_t *)(new_buf->buf_planes[0].buf);
       }
     }
     /* Get output reprocess buffer */
     info.check_type   = CHECK_FEATURE_MASK;
     info.stream_type  = CAM_STREAM_TYPE_OFFLINE_PROC;
     info.feature_mask = CAM_QCOM_FEATURE_METADATA_PROCESSING;
     if (input_buf_index->is_offline_meta_bypass) {
       info.feature_mask = CAM_QCOM_FEATURE_METADATA_BYPASS;
     }
     output_stream = mct_pipeline_get_stream(pipeline, &info);
     if (!output_stream) {
       CLOGE(CAM_MCT_MODULE, "Couldn't find offline metadata stream");
       return FALSE;
     }

     get_buf.stream_id = (uint32_t)output_stream->streamid;
     get_buf.session_id = MCT_PIPELINE_SESSION(
             MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(output_stream)->data));
     get_buf.type = MSM_CAMERA_BUF_MNGR_BUF_PLANAR;

     status = ioctl(pipeline->offline_meta.buf_mgr_fd,
       VIDIOC_MSM_BUF_MNGR_GET_BUF, &get_buf);
     if (status < 0) {
       CLOGE(CAM_MCT_MODULE,
             "Failed to get offline metadata buf - errno: %s!!!",
             strerror(errno));
       return FALSE;
     }
     pipeline->offline_meta.output_buf_idx = get_buf.index;
     pipeline->offline_meta.stream_id = get_buf.stream_id;

     pthread_mutex_lock(&output_stream->buffers.lock_img_buf);
     output_buf_holder = mct_list_find_custom(
                        output_stream->buffers.img_buf,
                        &(get_buf.index),
                        mct_stream_find_metadata_buf);
     pthread_mutex_unlock(&output_stream->buffers.lock_img_buf);

     if ((output_buf_holder) && (output_buf_holder->data)) {
       new_buf = output_buf_holder->data;
       pipeline->offline_meta.output_metadata_buffer =
         (metadata_buffer_t *)(new_buf->buf_planes[0].buf);
    }

    /* Copy Input online metadata buffer to output offline metadata buffer */
    memcpy (pipeline->offline_meta.output_metadata_buffer,
      pipeline->offline_meta.input_metadata_buffer,
      sizeof (metadata_buffer_t));

    /*Do BUF Done for the offline metadata which needed bypass*/
      if (input_buf_index->is_offline_meta_bypass) {
        get_buf.frame_id = stream->streaminfo.parm_buf.reprocess.frame_idx;
        status = ioctl(pipeline->offline_meta.buf_mgr_fd,
           VIDIOC_MSM_BUF_MNGR_BUF_DONE, &get_buf);
        if (status < 0) {
          CLOGE(CAM_MCT_MODULE,
           "Failed to get offline metadata buf - errno: %s!!!",
           strerror(errno));
          return FALSE;
        }
      }
    }
      break;

    default:
      break;
  }
  return TRUE;
}

/**Name: mct_offline_metadata_thread
 *
 * Arguments/Fields:
 *     @data: structure of mct_pipeline_t
 *
 * Return:
 *    Void *
 *
 * Description:
 *    This thread process message sent for offline metadata
 **/
static void* mct_offline_metadata_thread(void *data)
{
  mct_offline_meta_payload *offline_meta_payload;
  mct_pipeline_get_stream_info_t info;
  mct_stream_t *stream = NULL;
  boolean ret = TRUE;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)data;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "FATAL! Pipeline doesn't exist!");
    return NULL;
  }

  do {
    pthread_mutex_lock(&pipeline->offline_meta.offline_meta_mutex);
    if (!pipeline->offline_meta.offline_meta_queue->length) {
      pthread_cond_wait(&pipeline->offline_meta.offline_meta_cond,
        &pipeline->offline_meta.offline_meta_mutex);
    }
    pthread_mutex_unlock(&pipeline->offline_meta.offline_meta_mutex);

    /* Received signal for offline metadata event */
    while (1) {
      pthread_mutex_lock(&pipeline->offline_meta.offline_meta_queue_lock);
        offline_meta_payload = (mct_offline_meta_payload *)mct_queue_pop_head
          (pipeline->offline_meta.offline_meta_queue);
      pthread_mutex_unlock(&pipeline->offline_meta.offline_meta_queue_lock);

      if (!offline_meta_payload) {
        break;
      }
      if (offline_meta_payload->type == MCT_OFFLINE_META_THREAD_EXIT) {
        goto offline_meta_thread_exit;
      }

      mct_offline_meta_input_buf_index *input_buf_index = offline_meta_payload->data;
      /* Find offline meta stream */
      info.check_type   = CHECK_FEATURE_MASK;
      info.stream_type  = CAM_STREAM_TYPE_OFFLINE_PROC;
      info.feature_mask = CAM_QCOM_FEATURE_METADATA_PROCESSING;
      if (input_buf_index->is_offline_meta_bypass) {
        info.feature_mask = CAM_QCOM_FEATURE_METADATA_BYPASS;
      }
      stream = mct_pipeline_get_stream(pipeline, &info);
      if (!stream) {
        CLOGE(CAM_MCT_MODULE, "Couldn't find offline metadata stream");
        return FALSE;
      }


      /* Process queue message */
      ret = mct_stream_process_offline_message(stream,
        offline_meta_payload);

      if (offline_meta_payload->data) {
        free(offline_meta_payload->data);
        offline_meta_payload->data = NULL;
      }

      if (offline_meta_payload) {
        free(offline_meta_payload);
        offline_meta_payload = NULL;
      }
    }
  } while(1);

offline_meta_thread_exit:
  CLOGI(CAM_MCT_MODULE, "Exiting offline metadata thread");
  mct_queue_free(pipeline->offline_meta.offline_meta_queue);
  pthread_mutex_destroy(&pipeline->offline_meta.offline_meta_queue_lock);
  pthread_mutex_destroy(&pipeline->offline_meta.offline_meta_mutex);
  pthread_cond_destroy(&pipeline->offline_meta.offline_meta_cond);
  return NULL;
}

/**Name: mct_offline_metadata_ctrl_event:
 *
 * Arguments/Fields:
 *    @stream: Offline stream
 *    @event: Control event
 *
 *  Return: boolean;
 *    TRUE on success; FALSE on failure
 *
 *  Description:
 *    This function does special processing for
 *    offline metadata processing to collect stats during offline
 *    reprocessing
 **/
static boolean mct_offline_metadata_ctrl_event(mct_stream_t *stream,
  mct_event_t *event)
{
  boolean ret = TRUE;
  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Not valid pipeline for this stream 0x%x",
      stream->streaminfo.identity);
    return FALSE;
  }

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    switch (event->u.ctrl_event.type) {
    case MCT_EVENT_CONTROL_STREAMON: {
    /* Spawn new thread for offline metadata collection */
    if (pipeline->offline_meta.stream_on_cnt == 0) {
        ret = mct_stream_find_bfr_mngr_subdev(
          &pipeline->offline_meta.buf_mgr_fd);
        if (ret == FALSE) {
          CLOGE(CAM_MCT_MODULE, "failed to find buffer manager subdev");
          break;
        }
        pipeline->offline_meta.offline_meta_queue = malloc(sizeof(mct_queue_t));
        if (!pipeline->offline_meta.offline_meta_queue) {
            CLOGE(CAM_MCT_MODULE, "No memory available");
            return FALSE;
        }
        mct_queue_init(pipeline->offline_meta.offline_meta_queue);
        pthread_mutex_init(&pipeline->offline_meta.offline_meta_queue_lock, NULL);

        pthread_mutex_init(&pipeline->offline_meta.offline_meta_mutex, NULL);
        pthread_cond_init(&pipeline->offline_meta.offline_meta_cond, NULL);

        ret = mct_spawn_thread(mct_offline_metadata_thread, pipeline,
          "CAM_MctOffMeta", PTHREAD_CREATE_DETACHED);
        }
        pipeline->offline_meta.stream_on_cnt++;
    }
      break;

    case MCT_EVENT_CONTROL_STREAMOFF: {
      if (pipeline->offline_meta.stream_on_cnt == 1) {
        /* Add message in queue */
         mct_offline_meta_payload *streamoff_meta_payload =
          malloc(sizeof(mct_offline_meta_payload));
        if (!streamoff_meta_payload) {
          CLOGE(CAM_MCT_MODULE, "No memory available");
          break;
        }

        if (event->u.ctrl_event.type == MCT_EVENT_CONTROL_STREAMOFF) {
          streamoff_meta_payload->type = MCT_OFFLINE_META_THREAD_EXIT;
          streamoff_meta_payload->data = NULL;
          streamoff_meta_payload->size = 0;
        }
        pthread_mutex_lock(&pipeline->offline_meta.offline_meta_queue_lock);
        if (NULL != pipeline->offline_meta.offline_meta_queue) {
          mct_queue_push_tail(pipeline->offline_meta.offline_meta_queue,
            streamoff_meta_payload);
        } else {
          CLOGE(CAM_MCT_MODULE, "offline_meta_queue is null");
	  free(streamoff_meta_payload);
        }
        pthread_mutex_unlock(&pipeline->offline_meta.offline_meta_queue_lock);
        if (pipeline->offline_meta.buf_mgr_fd >= 0) {
          close(pipeline->offline_meta.buf_mgr_fd);
          pipeline->offline_meta.buf_mgr_fd = -1;
        }

        /* Signal Offline meta thread */
        pthread_mutex_lock(&pipeline->offline_meta.offline_meta_mutex);
        pthread_cond_signal(&pipeline->offline_meta.offline_meta_cond);
        pthread_mutex_unlock(&pipeline->offline_meta.offline_meta_mutex);
      }
      pipeline->offline_meta.stream_on_cnt--;
    }
    break;

    case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
    /* Get Parm stream control event */
    cam_stream_parm_buffer_t *param =
      (cam_stream_parm_buffer_t *)event->u.ctrl_event.control_event_data;

      if (CAM_STREAM_PARAM_TYPE_DO_REPROCESS == param->type) {
        /* Add message in queue */
        mct_offline_meta_payload *offline_meta_payload =
          malloc(sizeof(mct_offline_meta_payload));
        if (!offline_meta_payload) {
          CLOGE(CAM_MCT_MODULE, "No memory available");
          break;
        }

        if (event->u.ctrl_event.type == MCT_EVENT_CONTROL_PARM_STREAM_BUF) {
          mct_offline_meta_input_buf_index *input_buf
            = malloc (sizeof(mct_offline_meta_input_buf_index));
          if (!input_buf) {
           CLOGE(CAM_MCT_MODULE, "No memory available");
	   free(offline_meta_payload);
           break;
          }
          input_buf->buf_index = param->reprocess.buf_index;
          input_buf->streamid  = stream->streamid;
          input_buf->is_offline_meta_bypass  = param->reprocess.is_offline_meta_bypass;

          offline_meta_payload->type = MCT_OFFLINE_META_INPUT_BUF;
          offline_meta_payload->data = input_buf;
          offline_meta_payload->size =
                              sizeof(mct_offline_meta_input_buf_index);
        }

        pthread_mutex_lock(&pipeline->offline_meta.offline_meta_queue_lock);
        if (NULL != pipeline->offline_meta.offline_meta_queue) {
          mct_queue_push_tail(pipeline->offline_meta.offline_meta_queue,
            offline_meta_payload);
        } else {
          CLOGE(CAM_MCT_MODULE, "Offline_meta_queue is null");
	  free(offline_meta_payload);
        }
        pthread_mutex_unlock(&pipeline->offline_meta.offline_meta_queue_lock);

        /* Signal Offline meta thread */
        pthread_mutex_lock(&pipeline->offline_meta.offline_meta_mutex);
        pthread_cond_signal(&pipeline->offline_meta.offline_meta_cond);
        pthread_mutex_unlock(&pipeline->offline_meta.offline_meta_mutex);
      }
    }
      break;

    case MCT_EVENT_CONTROL_REMOVE_BUF_INFO:
      break;

    default:
      CLOGE(CAM_MCT_MODULE, "Ret=%d Unsupported cmd %d",
        ret, event->u.ctrl_event.type);
      break;
    } /* event->u.ctrl_event.type */
  }
     break;
  default:
    CLOGE(CAM_MCT_MODULE, "Ret=%d Unsupported event type %d",
      ret, event->type);
    break;
  } /* event type switch*/
  return ret;
}


/** Name: mct_stream_send_event
 *
 *  Arguments/Fields:
 *    @stream: Stream on which to send event
 *    @event: MCT event to be sent downstream
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Sends MCT event to downstream source module.
 *    If src_module does not have a source port and instead
 *    exposes process_event() method it is used directly.
 *    If a src_port exists, its event_func() is invoked.
 *
 **/
static boolean mct_stream_send_event(mct_stream_t *stream, mct_event_t *event)
{
  mct_module_t *src_module = NULL;
  boolean ret = FALSE;
  mct_list_t *sinkports;
  mct_port_t *port = NULL;
  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    event->logNestingIndex = 0;
    event->logNestingIndex++;
    ATRACE_INT_SNPRINTF(event->logNestingIndex,20, "EventCmd%d",
                           event->u.ctrl_event.type);
    ATRACE_INT_SNPRINTF(event->logNestingIndex,20, "EventCmd%d type%d",
                        event->u.ctrl_event.type,
                        stream->streaminfo.stream_type);
  }

  /*Pushing the commands in to the deferred queue */
  if (pipeline->deferred_state == CAM_DEFER_START) {
    ret = mct_pipeline_push_defer_cmd(pipeline, event, stream);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Failed to queue defer cmd");
    }
    return ret;
  }

  if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_METADATA) {
    ret = mct_stream_metadata_ctrl_event(stream, event);

  } else if ((stream->streaminfo.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC)
             &&
             ((stream->streaminfo.reprocess_config.\
              pp_feature_config.feature_mask &
              (CAM_QCOM_FEATURE_METADATA_PROCESSING | CAM_QCOM_FEATURE_METADATA_BYPASS)))) {
      ret = mct_offline_metadata_ctrl_event(stream, event);
  } else {
    if (MCT_STREAM_CHILDREN(stream)) {
      src_module = (mct_module_t *)(MCT_STREAM_CHILDREN(stream)->data);
    }
    if (!src_module) {
      CLOGE(CAM_MCT_MODULE, "Source module not present");
      return FALSE;
    }
    if ((mct_module_find_type(src_module, event->identity)
         == MCT_MODULE_FLAG_SOURCE) && src_module->process_event) {
      ret = src_module->process_event(src_module, event);
    } else {
      sinkports = mct_list_find_custom(MCT_MODULE_SINKPORTS(src_module),
        &event->identity, mct_port_find_port_with_matching_identity);
      if (sinkports) {
        port = (mct_port_t *)(sinkports->data);
        if (port && port->event_func) {
          ret = port->event_func(port, event);
        } else {
          CLOGI(CAM_MCT_MODULE, "NULL ptr detected.");
        }
      } else {
        CLOGI(CAM_MCT_MODULE, "No sinkport found.");
      }
    }
  }

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    --event->logNestingIndex;
    ATRACE_INT_SNPRINTF(event->logNestingIndex,20, "EventCmd%d",
                           event->u.ctrl_event.type);
    ATRACE_INT_SNPRINTF(event->logNestingIndex,20, "EventCmd%d type%d",
                        event->u.ctrl_event.type,
                        stream->streaminfo.stream_type);
  }
  return ret;
}

/** Name: mct_steam_send_super_event:
 *  Arguments/Fields:
 *    @parm_stream: Session stream on which super events are sent
 *    @super_event: Currently processed single super-event
 *  Description:
 *    Routine to send super-param event to all downstream modules
 **/

boolean mct_stream_send_super_event(mct_stream_t *parm_stream,
  mct_event_super_control_parm_t *super_event)
{
  boolean ret;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

  if (!parm_stream || !super_event) {
    CLOGE(CAM_MCT_MODULE,
          "NULL ptr detected: parm_stream [%p], super_event [%p]",
          parm_stream, super_event);
    return FALSE;
  }

  event_data.type = MCT_EVENT_CONTROL_SET_SUPER_PARM;
  event_data.control_event_data = super_event;
  event_data.size = 0;
  event_data.current_frame_id = parm_stream->current_frame_id;
  ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
                                parm_stream->streaminfo.identity,
                                MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
  if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event ");
  }
  ret = parm_stream->send_event(parm_stream, &cmd_event);
  if (FALSE == ret) {
    CLOGE(CAM_MCT_MODULE, "Failed to send CONTROL_SET_SUPER_PARM for frame %d",
      parm_stream->current_frame_id);
  } else {
    CLOGD(CAM_MCT_MODULE,
          "Successfully sent CONTROL_SET_SUPER_PARM for frame %d",
          parm_stream->current_frame_id);
  }
  return ret;
}

boolean mct_stream_send_super_event_slave(mct_stream_t *parm_stream,
  mct_event_super_control_parm_t *super_event)
{
  boolean ret;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

  if (!parm_stream || !super_event) {
    CLOGE(CAM_MCT_MODULE,
          "NULL ptr detected: parm_stream [%p], super_event [%p]",
          parm_stream, super_event);
    return FALSE;
  }

  event_data.type = MCT_EVENT_CONTROL_SET_SUPER_PARM;
  event_data.control_event_data = super_event;
  event_data.size = 0;
  event_data.current_frame_id = 0;
  ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
                                parm_stream->streaminfo.identity,
                                MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
  if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event ");
  }
  ret = parm_stream->send_event(parm_stream, &cmd_event);
  if (FALSE == ret) {
    CLOGE(CAM_MCT_MODULE, "Failed to send CONTROL_SET_SUPER_PARM for slave");
  } else {
    CLOGD(CAM_MCT_MODULE,
          "Successfully sent CONTROL_SET_SUPER_PARM for slave");
  }
  return ret;
}

/** Name: mct_stream_send_ctrl_sof:
 *  Arguments/Fields:
 *    @parm_stream: Session stream on which super events are sent
 *    @isp_sof_bus_msg: Bus msg containing SOF information
 *  Description:
 *    Routine to send SOF notification event to all downstream modules
 **/
boolean mct_stream_send_ctrl_sof(mct_stream_t *parm_stream,
  mct_bus_msg_isp_sof_t *isp_sof_bus_msg)
{
  boolean ret;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

    if (!parm_stream || !isp_sof_bus_msg) {
      CLOGE(CAM_MCT_MODULE,
            "NULL ptr detected: parm_stream [%p], isp_sof_bus_msg [%p]",
            parm_stream, isp_sof_bus_msg);
      return FALSE;
    }

  event_data.type = MCT_EVENT_CONTROL_SOF;
  event_data.current_frame_id = isp_sof_bus_msg->frame_id;
  event_data.control_event_data = isp_sof_bus_msg;
  event_data.size = sizeof(mct_bus_msg_isp_sof_t);
  ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
                                parm_stream->streaminfo.identity,
                                MCT_EVENT_DOWNSTREAM,
                                &event_data, &cmd_event);
  if (ret == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Error in packing event ");
  }
  ret = parm_stream->send_event(parm_stream, &cmd_event);
  if (FALSE == ret) {
    CLOGE(CAM_MCT_MODULE, "Failed to send MCT_EVENT_CONTROL_SOF for frame %d",
      isp_sof_bus_msg->frame_id);
  } else {
    CLOGD(CAM_MCT_MODULE,
          "Successfully sent MCT_EVENT_CONTROL_SOF for frame %d",
          isp_sof_bus_msg->frame_id);
  }

  return ret;
}

/** Name: mct_stream_send_offline_meta:
 *  Arguments/Fields:
 *    @parm_stream: Session stream on which super events are sent
 *    @isp_sof_bus_msg: Bus msg containing SOF information
 *  Description:
 *    Routine to send offline metadata event to all downstream modules
 **/
boolean mct_stream_send_offline_meta(mct_stream_t *stream,
  mct_bus_msg_isp_rd_done_t *isp_rd_done_msg)
{
  boolean ret;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Invalid stream ptr");
    return FALSE;
  }

  if (!isp_rd_done_msg) {
    CLOGE(CAM_MCT_MODULE,"Data from bus msg NULL");
    return FALSE;
  }

  event_data.type = MCT_EVENT_CONTROL_OFFLINE_METADATA;
  event_data.current_frame_id = isp_rd_done_msg->frame_id;
  event_data.size = 0;
  ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
                                stream->streaminfo.identity,
                                MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
  if (ret == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Error in packing offline metadata event");
  }
  ret = stream->send_event(stream, &cmd_event);
  if (FALSE == ret) {
    CLOGE(CAM_MCT_MODULE, "Failed to send MCT_EVENT_CONTROL_OFFLINE_METADATA");
  } else {
    CLOGD(CAM_MCT_MODULE,
          "Successfully sent MCT_EVENT_CONTROL_OFFLINE_METADATA");
  }
  return ret;
}


/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static mct_stream_map_buf_t *mct_stream_create_buffers(
  cam_stream_info_t *stream_info, mct_list_t **p_img_list,
  mct_serv_ds_msg_t *msg)
{
  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_MCT_CREATE_BUF);
  uint32_t i;
  mct_stream_map_buf_t *buf_holder;
  mct_list_t *buf_holder_list;
  mct_list_t* img_list = *p_img_list;
  buf_holder_list = mct_list_find_custom(img_list, msg,
    mct_stream_find_stream_buf);
  cam_stream_buf_plane_info_t buf_planes;

  switch (msg->buf_type) {
  case CAM_MAPPING_BUF_TYPE_MISC_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
    buf_planes.plane_info.num_planes = 1;
    buf_planes.plane_info.frame_len = msg->size;
    buf_planes.plane_info.sp.len = msg->size;
    break;
  case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
    buf_planes = stream_info->reprocess_config.offline.input_buf_planes;
    break;
  default:
    buf_planes = stream_info->buf_planes;
    break;
  }

  if (!buf_holder_list) {
    buf_holder = malloc(sizeof(mct_stream_map_buf_t));
    if (!buf_holder)
      goto error;
    /* Add this buffer holder to the image list */
    *p_img_list = mct_list_append(img_list, buf_holder, NULL, NULL);

    buf_holder->buf_index = msg->index;
    buf_holder->buf_type = msg->buf_type;
    buf_holder->num_planes = buf_planes.plane_info.num_planes;
    buf_holder->common_fd = FALSE;
  } else {
    buf_holder = (mct_stream_map_buf_t *)(buf_holder_list->data);
    if ((buf_holder->num_planes == 1) ||
        (buf_holder->buf_planes[msg->plane_idx].buf != NULL))
      goto finish;
  }
  //Before mmap lets check the fd value
  if (buf_holder->num_planes == 1) {
    if ((SECURE == stream_info->is_secure) &&
      (CAM_STREAM_TYPE_METADATA != stream_info->stream_type)) {
      CLOGI(CAM_MCT_MODULE, "Not going to map secure buffer");
        buf_holder->buf_planes[0].buf = NULL;
    } else {
#ifndef DAEMON_PRESENT
      if (msg->buffer) {
        buf_holder->buf_planes[0].buf =  msg->buffer;
      } else {
        CLOGE(CAM_MCT_MODULE, "HAL buffer not present");
        goto error1;
      }
#else
      buf_holder->buf_planes[0].buf = mmap(NULL, msg->size,
        PROT_READ | PROT_WRITE, MAP_SHARED, msg->fd, 0);
      if (buf_holder->buf_planes[0].buf == MAP_FAILED) {
        CLOGE(CAM_MCT_MODULE, "Mapping failed with error %s", strerror(errno));
        goto error1;
      }
#endif
    }
    buf_holder->buf_size             = msg->size;
    buf_holder->buf_planes[0].size   = msg->size;
    buf_holder->buf_planes[0].offset = 0;
    buf_holder->buf_planes[0].fd     = msg->fd;
  } else if(buf_holder->num_planes > 1) { /*num_planes > 1*/
    if (msg->plane_idx == -1) { /*same fd is used for all buffers*/
      buf_holder->common_fd = TRUE;
      if (SECURE == stream_info->is_secure) {
        CLOGI(CAM_MCT_MODULE, "Not going to map secure buffer");
        buf_holder->buf_planes[0].buf = NULL;
      } else {
#ifndef DAEMON_PRESENT
        if (msg->buffer) {
          buf_holder->buf_planes[0].buf =  msg->buffer;
        } else {
          CLOGH(CAM_MCT_MODULE, "HAL buffer not present");
          goto error1;
        }
#else
        buf_holder->buf_planes[0].buf = mmap(NULL, msg->size,
          PROT_READ | PROT_WRITE, MAP_SHARED, msg->fd, 0);
        if (buf_holder->buf_planes[0].buf == MAP_FAILED) {
          CLOGE(CAM_MCT_MODULE, "Mapping failed with error %s", strerror(errno));
          goto error1;
        }
#endif
      }

      buf_holder->buf_size             = msg->size;
      buf_holder->buf_planes[0].size   =
        buf_planes.plane_info.mp[0].len;
      buf_holder->buf_planes[0].offset =
        0 + buf_planes.plane_info.mp[0].offset;
      buf_holder->buf_planes[0].fd     = msg->fd;
      buf_holder->buf_planes[0].stride =
        buf_planes.plane_info.mp[0].stride;
      buf_holder->buf_planes[0].scanline =
        buf_planes.plane_info.mp[0].scanline;

      for (i = 1; i < buf_holder->num_planes; i++) {
        buf_holder->buf_planes[i].size   =
          buf_planes.plane_info.mp[i].len;
        buf_holder->buf_planes[i].offset =
          buf_planes.plane_info.mp[i].offset;
        buf_holder->buf_planes[i].fd  = buf_holder->buf_planes[0].fd;
        buf_holder->buf_planes[i].buf = buf_holder->buf_planes[0].buf;
        buf_holder->buf_planes[i].stride   =
          buf_planes.plane_info.mp[i].stride;
        buf_holder->buf_planes[i].scanline =
          buf_planes.plane_info.mp[i].scanline;
      }
      for (i = 0; i < buf_holder->num_planes; i++) {
        CLOGD(CAM_MCT_MODULE,
              "plane idx = %d, offset %d, stride %d, scanline = %d, len %d",
              i, buf_planes.plane_info.mp[i].offset,
              buf_planes.plane_info.mp[i].stride,
              buf_planes.plane_info.mp[i].scanline,
              buf_planes.plane_info.mp[i].len);
      }
    } else { /*different fds for different planes; not supported atm*/
      buf_holder->common_fd = FALSE;
    }/*different fds for different planes*/
  } /*num_planes > 1*/
finish:
  ATRACE_CAMSCOPE_END(CAMSCOPE_MCT_CREATE_BUF);
  return buf_holder;
error1:
  *p_img_list = mct_list_remove(img_list, buf_holder);
  free(buf_holder);
error:
  ATRACE_CAMSCOPE_END(CAMSCOPE_MCT_CREATE_BUF);
  CLOGE(CAM_MCT_MODULE, "for stream type: %d", stream_info->stream_type);
  return NULL;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_destroy_buffers(void *data, void *user_data)
{
  mct_stream_map_buf_t *mbuf = (mct_stream_map_buf_t *)data;
  mct_stream_t *stream  = (mct_stream_t *)user_data;
  uint32_t i,j;
  int rc = 0;

  if (!mbuf)
    return FALSE;

  for (i = 0; i < mbuf->num_planes; i++) {
    if (mbuf->buf_planes[i].buf) {
#ifdef DAEMON_PRESENT
      if (mbuf->common_fd == FALSE) {
        rc = munmap(mbuf->buf_planes[i].buf, mbuf->buf_planes[i].size);
        if (rc < 0) {
          CLOGE(CAM_MCT_MODULE,
            "Failed for unmap buffer for buf type =%d err =%s",
            mbuf->buf_type, strerror(errno));
        }
      }
      else {
        rc = munmap(mbuf->buf_planes[i].buf, mbuf->buf_size);
        if (rc < 0) {
          CLOGE(CAM_MCT_MODULE,
            "Failed for unmap buffer for buf type =%d err =%s",
            mbuf->buf_type, strerror(errno));
        }
      }
#endif
      mbuf->buf_planes[i].buf = NULL;
      for (j = 0; j< mbuf->num_planes; j++) {
        if (mbuf->buf_planes[j].buf != NULL &&
              mbuf->buf_planes[j].fd == mbuf->buf_planes[i].fd)
          mbuf->buf_planes[j].buf = NULL;
      }
    }

#ifdef DAEMON_PRESENT
      if (mbuf->common_fd == TRUE) {
        if (i == 0)
         close(mbuf->buf_planes[0].fd);
      }
      else {
        close(mbuf->buf_planes[i].fd);
      }
#endif
  }

  return TRUE;
}

/** mct_stream_map_parm
 *    @stream: this stream object
 *
 *  Maps streaminfo addr of session based stream.
 *
 *  Return TRUE if mapped successfully.
 **/
static boolean mct_stream_map_parm(mct_stream_t *stream)
{
  mct_pipeline_t *pipeline;
  if (!stream ) {
    CLOGE(CAM_MCT_MODULE, "Invalid stream ptr");
    return FALSE;
  }
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline ) {
    CLOGE(CAM_MCT_MODULE, "No valid pipeline for this stream!");
    return FALSE;
  }
  stream->buffers.stream_info =
             (cam_stream_info_t *)pipeline->session_stream.session_stream_info;
  if (!stream->buffers.stream_info) {
     CLOGE(CAM_MCT_MODULE, "Mapping failed with error");
     return FALSE;
  } else
    return TRUE;
}

/** mct_stream_map_parm
 *    @stream: this stream object
 *
 *  Unmaps streaminfo addr of session based stream.
 *
 *  Return TRUE if unmapped successfully.
 **/
static boolean mct_stream_unmap_parm(mct_stream_t *stream)
{
  if (!stream ) {
    CLOGE(CAM_MCT_MODULE, "Invalid stream ptr");
    return FALSE;
  }
  stream->buffers.stream_info = NULL;
  return TRUE;
}

/** mct_stream_map_buf
 *    @message: mct_serv_msg_t message sent from pipeline
 *    @stream: this stream object
 *
 *  Stream processes HAL buffer mappings. In case of SERV_BUF_STREAMINFO,
 *  as stream information is ready, the Stream will start module linking.
 *
 *  Return TRUE if mapped successfully.
 **/
static boolean mct_stream_map_buf(void *message, mct_stream_t *stream)
{
  boolean ret = FALSE;
  mct_serv_ds_msg_t *msg = (mct_serv_ds_msg_t *)message;
  mct_stream_map_buf_t *buf_holder;
  mct_pipeline_t *pipeline;
  boolean ioctl_ret = TRUE;
  mct_list_t *buf_holder_list;
  struct msm_buf_mngr_main_cont_info cont_buf_info;

  if (!msg || !stream || (uint32_t)msg->stream != stream->streamid) {
    ret = FALSE;
    goto finish;
  }
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);

  switch (msg->buf_type) {
  /* Below message are per Stream */
  case CAM_MAPPING_BUF_TYPE_STREAM_INFO: {
#ifndef DAEMON_PRESENT
    if (msg->buffer) {
      stream->buffers.stream_info = msg->buffer;
    } else {
      CLOGE(CAM_MCT_MODULE, "Stream Info buffer NULL");
      ret = FALSE;
      break;
    }
#else
    stream->buffers.stream_info = mmap(NULL, msg->size, PROT_READ | PROT_WRITE,
      MAP_SHARED, msg->fd, 0);

    if (stream->buffers.stream_info == MAP_FAILED) {
      CLOGE(CAM_MCT_MODULE, "Mapping failed with error %s", strerror(errno));
      ret = FALSE;
      break;
    }
#endif
    stream->buffers.stream_size = msg->size;
    stream->buffers.stream_fd   = msg->fd;
    ret = TRUE;
  }
    break;

  case CAM_MAPPING_BUF_TYPE_MISC_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
  case CAM_MAPPING_BUF_TYPE_STREAM_BUF: {
    pthread_mutex_lock(&stream->buffers.lock_img_buf);
    buf_holder = mct_stream_create_buffers(stream->buffers.stream_info,
      &stream->buffers.img_buf, msg);
    pthread_mutex_unlock(&stream->buffers.lock_img_buf);

    if (!buf_holder)
      goto error;

    if (!stream->buffers.img_buf)
      goto error1;

    pthread_mutex_lock(&stream->buffers.lock_img_buf);
    stream->streaminfo.img_buffer_list = stream->buffers.img_buf;
    pthread_mutex_unlock(&stream->buffers.lock_img_buf);

    if ((stream->state == MCT_ST_STATE_RUNNING) &&
        (pipeline->deferred_state != CAM_DEFER_START)) {
      mct_event_t         cmd_event;
      mct_event_control_t event_data;
      event_data.type = MCT_EVENT_CONTROL_UPDATE_BUF_INFO;
      event_data.control_event_data = (void *)buf_holder;
      event_data.size = sizeof(mct_stream_map_buf_t);
      ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
        (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
         MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
      if (ret == FALSE) {
        CLOGE(CAM_MCT_MODULE, "Error in packing event ");
        break;
      }
      if (!stream->send_event(stream, &cmd_event)) {
        CLOGE(CAM_MCT_MODULE, "Failure in sending UPDATE_BUF_INFO event");
        ret = FALSE;
        break;
      }
    }
    ret = TRUE;
  }
    break;

  case CAM_MAPPING_BUF_TYPE_STREAM_USER_BUF: {

    if (stream->buf_mgr_fd < 0) {
      ret = mct_stream_find_bfr_mngr_subdev(
        &stream->buf_mgr_fd);
      if (ret == FALSE) {
        CLOGE(CAM_MCT_MODULE, "Failed to find buffer manager subdev, fd=%d",
          stream->buf_mgr_fd);
      break;
      }
    }
  /* Getting hold of any container buffers present */
    buf_holder_list = mct_list_find_custom(stream->buffers.container_buf,
                      msg, mct_stream_find_cont_buf);

    if (!buf_holder_list) {
      buf_holder = malloc(sizeof(mct_stream_map_buf_t));
      if (!buf_holder) {
        CLOGE(CAM_MCT_MODULE, "No memory available");
        ret = FALSE;
        break;
      }
      stream->buffers.container_buf = mct_list_append(
        stream->buffers.container_buf, buf_holder, NULL, NULL);

      buf_holder->buf_type = msg->buf_type;
      buf_holder->buf_index = msg->index;
      memset(buf_holder->buf_planes, 0, sizeof(buf_holder->buf_planes));
      buf_holder->buf_size = msg->size;
      buf_holder->num_planes = 0;
      buf_holder->container_fd = msg->fd;

#ifndef DAEMON_PRESENT
      if (msg->buffer) {
        buf_holder->buf_container_info = msg->buffer;
      } else {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "USER_BUF buffer null");
        mct_list_remove(stream->buffers.container_buf, buf_holder);
        free(buf_holder);
        break;
      }
#else
      buf_holder->buf_container_info = mmap(NULL, msg->size,
        PROT_READ | PROT_WRITE, MAP_SHARED, msg->fd, 0);
      if (buf_holder->buf_container_info == MAP_FAILED) {
        CLOGE(CAM_MCT_MODULE, "Mapping of container buffer failed err=%s",
          strerror(errno));
        mct_list_remove(stream->buffers.container_buf, buf_holder);
        free(buf_holder);
        ret = FALSE;
        break;
      }
#endif
    /*Sending IOCTL call for mapping */
    cont_buf_info.stream_id = msg->stream;
    cont_buf_info.cnt = msg->size;
    cont_buf_info.cont_fd = msg->fd;
    cont_buf_info.session_id = pipeline->session;
    cont_buf_info.cmd = MSM_CAMERA_BUF_MNGR_CONT_MAP;

    ioctl_ret = ioctl(stream->buf_mgr_fd,
          VIDIOC_MSM_BUF_MNGR_CONT_CMD, &cont_buf_info);
    if (ioctl_ret < 0) {
      CLOGE(CAM_MCT_MODULE,
            "Failed in sending buffer map details to kernel err=%s",
            strerror(errno));
#ifdef DAEMON_PRESENT
      munmap(buf_holder->buf_container_info, buf_holder->buf_size);
#else
      buf_holder->buf_container_info = NULL;
#endif
      mct_list_remove(stream->buffers.container_buf, buf_holder);
      free(buf_holder);
      ret = FALSE;
      break;
    }

    ret = TRUE;
    }
    else {
      CLOGE(CAM_MCT_MODULE, "Buffer with same the index and type \
        already present in the list");
      ret = TRUE;
      break;
    }
  }
  break;

  default:
    goto error;
    break;
  } /* switch */

  goto finish;

error1:
  mct_stream_destroy_buffers(buf_holder, stream);
error:
  ret = FALSE;
finish:
  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
boolean mct_stream_unmap_buf(void *message, mct_stream_t *stream)
{
  boolean ret = FALSE;
  int rc;
  mct_serv_ds_msg_t *msg = (mct_serv_ds_msg_t *)message;
  mct_stream_map_buf_t *buf_holder;
  mct_list_t *buf_holder_list;
  mct_pipeline_t *pipeline;
  boolean ioctl_ret = TRUE;
  struct msm_buf_mngr_main_cont_info cont_buf_info;

  if (!msg || !stream || (uint32_t)msg->stream != stream->streamid) {
    ret = FALSE;
    goto finish;
  }
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);

  if(!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Pipeline not found for stream 0x%x",
      stream->streaminfo.identity);
    return FALSE;
  }

  switch ((cam_mapping_buf_type)(msg->buf_type)) {
  /* Below message are per Stream */
  case CAM_MAPPING_BUF_TYPE_STREAM_INFO: {
#ifdef DAEMON_PRESENT
    rc = munmap(stream->buffers.stream_info, stream->buffers.stream_size);
    close(stream->buffers.stream_fd);
    if (rc < 0) {
      CLOGE(CAM_MCT_MODULE, "Un-mapping failed for stream info buffer err =%s",
            strerror(errno));
      ret = FALSE;
      break;
    }
#endif
    stream->buffers.stream_info = NULL;
    stream->buffers.stream_size = 0;
    ret = TRUE;
  }
    break;

  case CAM_MAPPING_BUF_TYPE_MISC_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
  case CAM_MAPPING_BUF_TYPE_STREAM_BUF: {
    pthread_mutex_lock(&stream->buffers.lock_img_buf);

    buf_holder_list = mct_list_find_custom(stream->buffers.img_buf,
      msg, mct_stream_find_stream_buf);
    if (!buf_holder_list) {
      CLOGE(CAM_MCT_MODULE,
            "Buf idx =%d of buf type =%d not present in mapped buf list",
            msg->index, msg->buf_type);
      pthread_mutex_unlock(&stream->buffers.lock_img_buf);
      ret = FALSE;
      goto finish;
    }
    pthread_mutex_unlock(&stream->buffers.lock_img_buf);

    buf_holder = (mct_stream_map_buf_t *)(buf_holder_list->data);
    if(!buf_holder) {
      CLOGE(CAM_MCT_MODULE, "Missing map_buf");
      ret = FALSE;
      goto finish;
    }
    /* Notify downstream modules of buffer to be unmapped */
    if (CAM_STREAM_TYPE_METADATA != stream->streaminfo.stream_type) {
      mct_event_t         cmd_event;
      mct_event_control_t event_data;
      event_data.type = MCT_EVENT_CONTROL_REMOVE_BUF_INFO;
      event_data.control_event_data = (void *)(buf_holder);
      event_data.size = sizeof(mct_stream_map_buf_t);
      ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
        (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
         MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
      if (ret == FALSE) {
        CLOGE(CAM_MCT_MODULE, "Error in packing event ");
        break;
      }
      if (!stream->send_event(stream, &cmd_event)) {
        CLOGE(CAM_MCT_MODULE, "Failure in sending REMOVE BUF_INFO event");
        ret = FALSE;
        break;
      }
    }

    pthread_mutex_lock(&stream->buffers.lock_img_buf);
    if (!mct_stream_destroy_buffers(buf_holder, stream)) {
      CLOGE(CAM_MCT_MODULE, "Failed to destroy buffers");
      ret = FALSE;
      break;
    }

    stream->buffers.img_buf = mct_list_remove(stream->buffers.img_buf,
                                buf_holder);
    free(buf_holder);
    buf_holder = NULL;
    stream->streaminfo.img_buffer_list = stream->buffers.img_buf;
    pthread_mutex_unlock(&stream->buffers.lock_img_buf);

    ret = TRUE;
  }
  break;

  case CAM_MAPPING_BUF_TYPE_STREAM_USER_BUF: {

    buf_holder_list = mct_list_find_custom(stream->buffers.container_buf,
                        msg, mct_stream_find_cont_buf);

    if (!buf_holder_list) {
      CLOGE(CAM_MCT_MODULE, "No container buffer found");
      ret = FALSE;
      break;
    }

    buf_holder = (mct_stream_map_buf_t *)(buf_holder_list->data);
    if (!buf_holder) {
      CLOGE(CAM_MCT_MODULE, "Missing container mapped buffer");
      ret = FALSE;
      break;
    }

    /*Sending IOCTL call for unmapping */
    cont_buf_info.stream_id = msg->stream;
    cont_buf_info.cnt = buf_holder->buf_size;
    cont_buf_info.cont_fd = buf_holder->container_fd;
    cont_buf_info.session_id = pipeline->session;
    cont_buf_info.cmd = MSM_CAMERA_BUF_MNGR_CONT_UNMAP;

    ioctl_ret = ioctl(stream->buf_mgr_fd,
      VIDIOC_MSM_BUF_MNGR_CONT_CMD, &cont_buf_info);
    if (ioctl_ret < 0) {
      CLOGE(CAM_MCT_MODULE,
            "Failed in sending buffer unmap details to kernel err=%s",
            strerror(errno));
      ret = FALSE;
      break;
    }
    /*Unmapping of container buffer */
#ifdef DAEMON_PRESENT
    rc = munmap(buf_holder->buf_container_info,buf_holder->buf_size);
    if (rc < 0) {
      CLOGE(CAM_MCT_MODULE, "Unmapping failed for container buffers err=%s",
            strerror(errno));
      ret = FALSE;
      break;
    }
#endif
    if(stream->buf_mgr_fd >= 0) {
      close(stream->buf_mgr_fd);
    }
    stream->buffers.container_buf = mct_list_remove(
      stream->buffers.container_buf, buf_holder);

    free(buf_holder);
    stream->buf_mgr_fd = -1;

  ret = TRUE;
  }
  break;

  default:
    ret = FALSE;
    goto finish;
  }

finish:
  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
mct_stream_t* mct_stream_new(uint32_t stream_id)
{
  mct_stream_t *stream;

  stream = malloc(sizeof(mct_stream_t));
  if (!stream)
    return NULL;

  memset(stream, 0, sizeof(mct_stream_t));
  stream->streamid  = stream_id;
  stream->state     = MCT_ST_STATE_IDLE;

  pthread_mutex_init(MCT_OBJECT_GET_LOCK(stream), NULL);
  pthread_mutex_init(&stream->buffers.lock_img_buf, NULL);
  pthread_mutex_init(&stream->metadata_stream.metadata_buf_lock, NULL);
  stream->buf_mgr_fd = -1;

  stream->add_module     = mct_stream_add_module;
  stream->remove_module  = mct_stream_remove_module;
  stream->send_event     = mct_stream_send_event;
  stream->map_buf        = mct_stream_map_buf;
  stream->unmap_buf      = mct_stream_unmap_buf;
  stream->map_parm       = mct_stream_map_parm;
  stream->unmap_parm     = mct_stream_unmap_parm;
  stream->link           = mct_stream_start_link;
  stream->unlink         = mct_stream_start_unlink;
  return stream;
}

/** mct_stream_remove_stream_from_module:
 *    @data:
 *    @user_data:
 *
 *  Remove the stream from a module's parent's list
 *
 *  Return TRUE always.
 **/
boolean mct_stream_remove_stream_from_module(void *data,
  void *user_data)
{
  mct_module_t *module = (mct_module_t *)data;
  mct_stream_t *stream = (mct_stream_t *)user_data;
  mct_list_t   *list = NULL;

  MCT_MODULE_LOCK(module);
  MCT_OBJECT_PARENT(module) = mct_list_remove(
    MCT_OBJECT_PARENT(module), MCT_OBJECT_CAST(stream));
  MCT_MODULE_UNLOCK(module);

  MCT_OBJECT_NUM_PARENTS(module) -= 1;
  return TRUE;
}

static boolean mct_stream_streamoff(void *data, void *user_data)
{
  boolean ret = TRUE;
  mct_stream_t *stream   = (mct_stream_t *)data;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)user_data;
  mct_event_t cmd_event;
  mct_event_control_t event_data;

  if (stream->state == MCT_ST_STATE_RUNNING) {
    /* Clear super-param queue if this is the last stream to be streamed-off */
    if (1 == pipeline->stream_on_cnt) {
      MCT_OBJECT_LOCK(pipeline);
      mct_list_free_all(pipeline->frame_num_idx_list,
        mct_stream_free_frame_num_list);
      pipeline->frame_num_idx_list = NULL;
      MCT_OBJECT_UNLOCK(pipeline);
      if (!MCT_QUEUE_IS_EMPTY(pipeline->super_param_queue)) {
        mct_pipeline_flush_super_param_queue(pipeline);
      }
    }
    event_data.type = MCT_EVENT_CONTROL_STREAMOFF;
    event_data.size = 0;
    event_data.control_event_data = (void *)&stream->streaminfo;
    CLOGI(CAM_MCT_MODULE, "STREAMING OFF stream 0x%x",
      stream->streaminfo.identity);
    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
    (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
    MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event ");
    }
    ret = stream->send_event(stream, &cmd_event);
    if (ret) {
      CLOGI(CAM_MCT_MODULE, "Stream 0x%x successfully streamed off.",
        stream->streaminfo.identity);
      stream->state = MCT_ST_STATE_IDLE;
      if ((stream->streaminfo.stream_type != CAM_STREAM_TYPE_METADATA) &&
        (stream->streaminfo.stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)) {
        pipeline->stream_on_cnt--;
        if (0 == pipeline->stream_on_cnt) {
          if (pipeline->bus) {
            pipeline->bus->prev_sof_id = 0;  /* Reset SOF tracker */
          }
        }
      }
    }
    else {
      CLOGE(CAM_MCT_MODULE, "Stream 0x%x failed to stream off.",
       stream->streaminfo.identity);
    }
  }
  return ret;
}

/**Name: mct_stream_del_offline_stream
 *
 * Arguments/Fields:
 * @data: structure of mct_stream_t
 * @user_data: structure of mct_pipeline_t
 *
 *  Return: boolean
 *    TRUE on success, FALSE on failure
 *
 * Description:
 *      This function deletes offline metadata
 **/
static boolean mct_stream_del_offline_stream(void *data, void *user_data)
{
  mct_stream_t *stream   = (mct_stream_t *)data;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)user_data;
  mct_event_t cmd_event;
  mct_event_control_t event_data;
  boolean ret = TRUE;

  event_data.type = MCT_EVENT_CONTROL_DEL_OFFLINE_STREAM;
  event_data.control_event_data = (void *)&stream->streaminfo;
  event_data.size = 0;
  ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
    (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
    MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
  if (ret == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Error in packing event ");
    return ret;
  }

  ret = stream->send_event(stream, &cmd_event);
  if (ret == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Error in sending event");
  }
  /*set state to IDLE even if downstream modules fails to handle DEL_STREAM
    notification */
  stream->state = MCT_ST_STATE_IDLE;
  return ret;
}

/** mct_stream_streamoff_no_offline:
 *    @
 *
 **/
static boolean mct_stream_streamoff_no_offline(void *data, void *user_data)
{
  mct_stream_t *stream   = (mct_stream_t *)data;
  if (stream->streaminfo.stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    return mct_stream_streamoff(data, user_data);
  }
  return TRUE;
}

/** mct_stream_destroy:
 *    @
 *
 **/
boolean mct_stream_destroy(mct_stream_t *stream)
{
  boolean ret = TRUE;
  mct_pipeline_t *pipeline;
  boolean rc = TRUE;
  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Stream is NULL");
    return FALSE;
  }
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Pipeline is NULL");
    return FALSE;
  }

  ATRACE_BEGIN_SNPRINTF(30, "Camera:StreamUnlink-type%d",
                        stream->streaminfo.stream_type);

  if (MCT_OBJECT_CHILDREN(stream))
  {
    if (stream->streaminfo.stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)
    {
      /*make sure all streams are streamed off. In ideal case this should
        already be true*/
      ret = mct_list_traverse(MCT_PIPELINE_CHILDREN(pipeline),
        mct_stream_streamoff_no_offline,
        pipeline);
    }
    else if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC)
    {
      ret = mct_stream_streamoff(stream, pipeline);
      rc = mct_stream_del_offline_stream(stream, pipeline);
      if (rc == FALSE) {
        CLOGE(CAM_MCT_MODULE, "Failed to delete offline stream")
      }
    }

    if (MCT_OBJECT_NUM_CHILDREN(stream) > 1)
    {
      mct_list_operate_nodes(MCT_OBJECT_CHILDREN(stream),
        mct_stream_operate_unlink, stream);
    }
    else
    {
      /* Type is removed in unlink modules,since we have only one
       * module it is not linked, remove type here */
      mct_module_t *single_module = MCT_OBJECT_CHILDREN(stream)->data;
      mct_module_remove_type(single_module, stream->streaminfo.identity);
    }
    /* 1. free stream from module's parent list;
     * 2. free module object from stream's children list */
    mct_list_free_all_on_data(MCT_OBJECT_CHILDREN(stream),
      mct_stream_remove_stream_from_module, stream);
    MCT_OBJECT_CHILDREN(stream) = NULL;
    MCT_STREAM_NUM_CHILDREN(stream) = 0;
  }
  else if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_METADATA)
  {
    ret = mct_stream_streamoff(stream, pipeline);
  }

  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(stream));

  if (stream->buffers.stream_info) {
#ifdef DAEMON_PRESENT
    munmap(stream->buffers.stream_info,
      stream->buffers.stream_size);
    close(stream->buffers.stream_fd);
#endif
    stream->buffers.stream_info = NULL;
  }

  pthread_mutex_lock(&stream->buffers.lock_img_buf);
  if (stream->buffers.img_buf) {
    mct_list_free_all(stream->buffers.img_buf, mct_stream_destroy_buffers);
    stream->buffers.img_buf = NULL;
  }
  pthread_mutex_unlock(&stream->buffers.lock_img_buf);

  pthread_mutex_destroy(&stream->buffers.lock_img_buf);
  pthread_mutex_destroy(&stream->metadata_stream.metadata_buf_lock);

  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  mct_object_unparent(MCT_OBJECT_CAST(stream), MCT_OBJECT_CAST(pipeline));

  free(stream);
  ATRACE_END();

  return ret;
}

/** get_pointer_of:
 *    @
 *
 **/
void *get_pointer_of(cam_intf_parm_type_t meta_id, metadata_buffer_t* metadata)
{
  switch(meta_id) {
    case CAM_INTF_META_HISTOGRAM:
      return POINTER_OF_META(CAM_INTF_META_HISTOGRAM, metadata);
    case CAM_INTF_META_FACE_DETECTION:
      return POINTER_OF_META(CAM_INTF_META_FACE_DETECTION, metadata);
    case CAM_INTF_META_FACE_RECOG:
      return POINTER_OF_META(CAM_INTF_META_FACE_RECOG, metadata);
    case CAM_INTF_META_FACE_BLINK:
      return POINTER_OF_META(CAM_INTF_META_FACE_BLINK, metadata);
    case CAM_INTF_META_FACE_GAZE:
      return POINTER_OF_META(CAM_INTF_META_FACE_GAZE, metadata);
    case CAM_INTF_META_FACE_SMILE:
      return POINTER_OF_META(CAM_INTF_META_FACE_SMILE, metadata);
    case CAM_INTF_META_FACE_LANDMARK:
      return POINTER_OF_META(CAM_INTF_META_FACE_LANDMARK, metadata);
    case CAM_INTF_META_FACE_CONTOUR:
      return POINTER_OF_META(CAM_INTF_META_FACE_CONTOUR, metadata);
    case CAM_INTF_META_AUTOFOCUS_DATA:
      return POINTER_OF_META(CAM_INTF_META_AUTOFOCUS_DATA, metadata);
    case CAM_INTF_META_CROP_DATA:
      return POINTER_OF_META(CAM_INTF_META_CROP_DATA, metadata);
    case CAM_INTF_META_PREP_SNAPSHOT_DONE:
      return POINTER_OF_META(CAM_INTF_META_PREP_SNAPSHOT_DONE, metadata);
    case CAM_INTF_META_GOOD_FRAME_IDX_RANGE:
      return POINTER_OF_META(CAM_INTF_META_GOOD_FRAME_IDX_RANGE, metadata);
    case CAM_INTF_META_ASD_HDR_SCENE_DATA:
      return POINTER_OF_META(CAM_INTF_META_ASD_HDR_SCENE_DATA, metadata);
    case CAM_INTF_META_ASD_SCENE_INFO:
      return POINTER_OF_META(CAM_INTF_META_ASD_SCENE_INFO, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_ISP:
      return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_ISP, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_PP:
      return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_PP, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_AE:
      return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_AE, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_AWB:
      return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_AWB, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_AF:
      return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_AF, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_ASD:
      return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_ASD, metadata);
    case CAM_INTF_META_FRAME_NUMBER_VALID:
      return POINTER_OF_META(CAM_INTF_META_FRAME_NUMBER_VALID, metadata);
    case CAM_INTF_META_URGENT_FRAME_NUMBER_VALID:
      return POINTER_OF_META(CAM_INTF_META_URGENT_FRAME_NUMBER_VALID,
                             metadata);
    case CAM_INTF_META_FRAME_DROPPED:
      return POINTER_OF_META(CAM_INTF_META_FRAME_DROPPED, metadata);
    case CAM_INTF_META_FRAME_NUMBER:
      return POINTER_OF_META(CAM_INTF_META_FRAME_NUMBER, metadata);
    case CAM_INTF_META_URGENT_FRAME_NUMBER:
      return POINTER_OF_META(CAM_INTF_META_URGENT_FRAME_NUMBER, metadata);
    case CAM_INTF_META_COLOR_CORRECT_MODE:
      return POINTER_OF_META(CAM_INTF_META_COLOR_CORRECT_MODE, metadata);
    case CAM_INTF_META_COLOR_CORRECT_TRANSFORM:
      return POINTER_OF_META(CAM_INTF_META_COLOR_CORRECT_TRANSFORM, metadata);
    case CAM_INTF_META_COLOR_CORRECT_GAINS:
      return POINTER_OF_META(CAM_INTF_META_COLOR_CORRECT_GAINS, metadata);
    case CAM_INTF_META_PRED_COLOR_CORRECT_TRANSFORM:
      return POINTER_OF_META(CAM_INTF_META_PRED_COLOR_CORRECT_TRANSFORM,
        metadata);
    case CAM_INTF_META_PRED_COLOR_CORRECT_GAINS:
      return POINTER_OF_META(CAM_INTF_META_PRED_COLOR_CORRECT_GAINS, metadata);
    case CAM_INTF_META_AEC_ROI:
      return POINTER_OF_META(CAM_INTF_META_AEC_ROI, metadata);
    case CAM_INTF_META_CURRENT_SCENE:
      return POINTER_OF_META(CAM_INTF_META_CURRENT_SCENE, metadata);
    case CAM_INTF_META_AEC_STATE:
      return POINTER_OF_META(CAM_INTF_META_AEC_STATE, metadata);
    case CAM_INTF_PARM_FOCUS_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_FOCUS_MODE, metadata);
    case CAM_INTF_PARM_MANUAL_FOCUS_POS:
      return POINTER_OF_META(CAM_INTF_PARM_MANUAL_FOCUS_POS, metadata);
    case CAM_INTF_META_AF_ROI:
      return POINTER_OF_META(CAM_INTF_META_AF_ROI, metadata);
    case CAM_INTF_META_AF_DEFAULT_ROI:
      return POINTER_OF_META(CAM_INTF_META_AF_DEFAULT_ROI, metadata);
    case CAM_INTF_META_AF_STATE:
      return POINTER_OF_META(CAM_INTF_META_AF_STATE, metadata);
    case CAM_INTF_PARM_WHITE_BALANCE:
      return POINTER_OF_META(CAM_INTF_PARM_WHITE_BALANCE, metadata);
    case CAM_INTF_META_AWB_REGIONS:
      return POINTER_OF_META(CAM_INTF_META_AWB_REGIONS, metadata);
    case CAM_INTF_META_AWB_STATE:
      return POINTER_OF_META(CAM_INTF_META_AWB_STATE, metadata);
    case CAM_INTF_META_BLACK_LEVEL_LOCK:
      return POINTER_OF_META(CAM_INTF_META_BLACK_LEVEL_LOCK, metadata);
    case CAM_INTF_META_MODE:
      return POINTER_OF_META(CAM_INTF_META_MODE, metadata);
    case CAM_INTF_META_EDGE_MODE:
      return POINTER_OF_META(CAM_INTF_META_EDGE_MODE, metadata);
    case CAM_INTF_META_FLASH_POWER:
      return POINTER_OF_META(CAM_INTF_META_FLASH_POWER, metadata);
    case CAM_INTF_META_FLASH_FIRING_TIME:
      return POINTER_OF_META(CAM_INTF_META_FLASH_FIRING_TIME, metadata);
    case CAM_INTF_META_FLASH_MODE:
      return POINTER_OF_META(CAM_INTF_META_FLASH_MODE, metadata);
    case CAM_INTF_META_FLASH_STATE:
      return POINTER_OF_META(CAM_INTF_META_FLASH_STATE, metadata);
    case CAM_INTF_META_HOTPIXEL_MODE:
      return POINTER_OF_META(CAM_INTF_META_HOTPIXEL_MODE, metadata);
    case CAM_INTF_META_LENS_APERTURE:
      return POINTER_OF_META(CAM_INTF_META_LENS_APERTURE, metadata);
    case CAM_INTF_META_LENS_FILTERDENSITY:
      return POINTER_OF_META(CAM_INTF_META_LENS_FILTERDENSITY, metadata);
    case CAM_INTF_META_LENS_FOCAL_LENGTH:
      return POINTER_OF_META(CAM_INTF_META_LENS_FOCAL_LENGTH, metadata);
    case CAM_INTF_META_LENS_FOCUS_DISTANCE:
      return POINTER_OF_META(CAM_INTF_META_LENS_FOCUS_DISTANCE, metadata);
    case CAM_INTF_META_FOCUS_VALUE:
      return POINTER_OF_META(CAM_INTF_META_FOCUS_VALUE, metadata);
    case CAM_INTF_META_SPOT_LIGHT_DETECT:
      return POINTER_OF_META(CAM_INTF_META_SPOT_LIGHT_DETECT, metadata);
    case CAM_INTF_META_LENS_FOCUS_RANGE:
      return POINTER_OF_META(CAM_INTF_META_LENS_FOCUS_RANGE, metadata);
    case CAM_INTF_META_LENS_STATE:
      return POINTER_OF_META(CAM_INTF_META_LENS_STATE, metadata);
    case CAM_INTF_META_LENS_OPT_STAB_MODE:
      return POINTER_OF_META(CAM_INTF_META_LENS_OPT_STAB_MODE, metadata);
    case CAM_INTF_META_NOISE_REDUCTION_MODE:
      return POINTER_OF_META(CAM_INTF_META_NOISE_REDUCTION_MODE, metadata);
    case CAM_INTF_META_NOISE_REDUCTION_STRENGTH:
      return POINTER_OF_META(CAM_INTF_META_NOISE_REDUCTION_STRENGTH, metadata);
    case CAM_INTF_META_SCALER_CROP_REGION:
      return POINTER_OF_META(CAM_INTF_META_SCALER_CROP_REGION, metadata);
    case CAM_INTF_META_SCENE_FLICKER:
      return POINTER_OF_META(CAM_INTF_META_SCENE_FLICKER, metadata);
    case CAM_INTF_META_SENSOR_EXPOSURE_TIME:
      return POINTER_OF_META(CAM_INTF_META_SENSOR_EXPOSURE_TIME, metadata);
    case CAM_INTF_META_SENSOR_FRAME_DURATION:
      return POINTER_OF_META(CAM_INTF_META_SENSOR_FRAME_DURATION, metadata);
    case CAM_INTF_META_SENSOR_SENSITIVITY:
      return POINTER_OF_META(CAM_INTF_META_SENSOR_SENSITIVITY, metadata);
    case CAM_INTF_META_SENSOR_TIMESTAMP:
      return POINTER_OF_META(CAM_INTF_META_SENSOR_TIMESTAMP, metadata);
    case CAM_INTF_META_SHADING_MODE:
      return POINTER_OF_META(CAM_INTF_META_SHADING_MODE, metadata);
    case CAM_INTF_META_STATS_FACEDETECT_MODE:
      return POINTER_OF_META(CAM_INTF_META_STATS_FACEDETECT_MODE, metadata);
    case CAM_INTF_META_STATS_HISTOGRAM_MODE:
      return POINTER_OF_META(CAM_INTF_META_STATS_HISTOGRAM_MODE, metadata);
    case CAM_INTF_META_STATS_SHARPNESS_MAP_MODE:
      return POINTER_OF_META(CAM_INTF_META_STATS_SHARPNESS_MAP_MODE, metadata);
    case CAM_INTF_META_STATS_SHARPNESS_MAP:
      return POINTER_OF_META(CAM_INTF_META_STATS_SHARPNESS_MAP, metadata);
    case CAM_INTF_META_TONEMAP_CURVES:
      return POINTER_OF_META(CAM_INTF_META_TONEMAP_CURVES, metadata);
    case CAM_INTF_META_LENS_SHADING_MAP:
      return POINTER_OF_META(CAM_INTF_META_LENS_SHADING_MAP, metadata);
    case CAM_INTF_META_AEC_INFO:
      return POINTER_OF_META(CAM_INTF_META_AEC_INFO, metadata);
    case CAM_INTF_META_SENSOR_INFO:
      return POINTER_OF_META(CAM_INTF_META_SENSOR_INFO, metadata);
    case CAM_INTF_META_EXIF_DEBUG_AE:
      return POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_AE, metadata);
    case CAM_INTF_META_EXIF_DEBUG_AWB:
      return POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_AWB, metadata);
    case CAM_INTF_META_EXIF_DEBUG_AF:
      return POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_AF, metadata);
    case CAM_INTF_META_EXIF_DEBUG_ASD:
      return POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_ASD, metadata);
    case CAM_INTF_META_EXIF_DEBUG_STATS:
      return POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_STATS, metadata);
    case CAM_INTF_META_EXIF_DEBUG_BESTATS:
      return POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_BESTATS, metadata);
    case CAM_INTF_META_EXIF_DEBUG_BHIST:
      return POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_BHIST, metadata);
    case CAM_INTF_META_EXIF_DEBUG_3A_TUNING:
      return POINTER_OF_META(CAM_INTF_META_EXIF_DEBUG_3A_TUNING, metadata);
    case CAM_INTF_PARM_EFFECT:
      return POINTER_OF_META(CAM_INTF_PARM_EFFECT, metadata);
    case CAM_INTF_META_PRIVATE_DATA:
      return POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, metadata);
    case CAM_INTF_PARM_HAL_VERSION:
      return POINTER_OF_META(CAM_INTF_PARM_HAL_VERSION, metadata);
    case CAM_INTF_PARM_ANTIBANDING:
      return POINTER_OF_META(CAM_INTF_PARM_ANTIBANDING, metadata);
    case CAM_INTF_PARM_EXPOSURE_COMPENSATION:
      return POINTER_OF_META(CAM_INTF_PARM_EXPOSURE_COMPENSATION, metadata);
    case CAM_INTF_PARM_EV_STEP:
      return POINTER_OF_META(CAM_INTF_PARM_EV_STEP, metadata);
    case CAM_INTF_PARM_AEC_LOCK:
      return POINTER_OF_META(CAM_INTF_PARM_AEC_LOCK, metadata);
    case CAM_INTF_PARM_FPS_RANGE:
      return POINTER_OF_META(CAM_INTF_PARM_FPS_RANGE, metadata);
    case CAM_INTF_PARM_AWB_LOCK:
      return POINTER_OF_META(CAM_INTF_PARM_AWB_LOCK, metadata);
    case CAM_INTF_PARM_BESTSHOT_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_BESTSHOT_MODE, metadata);
    case CAM_INTF_PARM_DIS_ENABLE:
      return POINTER_OF_META(CAM_INTF_PARM_DIS_ENABLE, metadata);
    case CAM_INTF_PARM_LED_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_LED_MODE, metadata);
    case CAM_INTF_META_LED_MODE_OVERRIDE:
      return POINTER_OF_META(CAM_INTF_META_LED_MODE_OVERRIDE, metadata);
    case CAM_INTF_PARM_QUERY_FLASH4SNAP:
      return POINTER_OF_META(CAM_INTF_PARM_QUERY_FLASH4SNAP, metadata);
    case CAM_INTF_PARM_EXPOSURE:
      return POINTER_OF_META(CAM_INTF_PARM_EXPOSURE, metadata);
    case CAM_INTF_PARM_SHARPNESS:
      return POINTER_OF_META(CAM_INTF_PARM_SHARPNESS, metadata);
    case CAM_INTF_PARM_CONTRAST:
      return POINTER_OF_META(CAM_INTF_PARM_CONTRAST, metadata);
    case CAM_INTF_PARM_SATURATION:
      return POINTER_OF_META(CAM_INTF_PARM_SATURATION, metadata);
    case CAM_INTF_PARM_BRIGHTNESS:
      return POINTER_OF_META(CAM_INTF_PARM_BRIGHTNESS, metadata);
    case CAM_INTF_PARM_ISO:
      return POINTER_OF_META(CAM_INTF_PARM_ISO, metadata);
    case CAM_INTF_PARM_EXPOSURE_TIME:
      return POINTER_OF_META(CAM_INTF_PARM_EXPOSURE_TIME, metadata);
    case CAM_INTF_PARM_USERZOOM:
      return POINTER_OF_META(CAM_INTF_PARM_USERZOOM, metadata);
    case CAM_INTF_PARM_ROLLOFF:
      return POINTER_OF_META(CAM_INTF_PARM_ROLLOFF, metadata);
    case CAM_INTF_PARM_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_MODE, metadata);
    case CAM_INTF_PARM_AEC_ALGO_TYPE:
      return POINTER_OF_META(CAM_INTF_PARM_AEC_ALGO_TYPE, metadata);
    case CAM_INTF_PARM_FOCUS_ALGO_TYPE:
      return POINTER_OF_META(CAM_INTF_PARM_FOCUS_ALGO_TYPE, metadata);
    case CAM_INTF_PARM_AEC_ROI:
      return POINTER_OF_META(CAM_INTF_PARM_AEC_ROI, metadata);
    case CAM_INTF_PARM_AF_ROI:
      return POINTER_OF_META(CAM_INTF_PARM_AF_ROI, metadata);
    case CAM_INTF_PARM_SCE_FACTOR:
      return POINTER_OF_META(CAM_INTF_PARM_SCE_FACTOR, metadata);
    case CAM_INTF_PARM_FD:
      return POINTER_OF_META(CAM_INTF_PARM_FD, metadata);
    case CAM_INTF_PARM_MCE:
      return POINTER_OF_META(CAM_INTF_PARM_MCE, metadata);
    case CAM_INTF_PARM_HFR:
      return POINTER_OF_META(CAM_INTF_PARM_HFR, metadata);
    case CAM_INTF_PARM_REDEYE_REDUCTION:
      return POINTER_OF_META(CAM_INTF_PARM_REDEYE_REDUCTION, metadata);
    case CAM_INTF_PARM_WAVELET_DENOISE:
      return POINTER_OF_META(CAM_INTF_PARM_WAVELET_DENOISE, metadata);
    case CAM_INTF_PARM_TEMPORAL_DENOISE:
      return POINTER_OF_META(CAM_INTF_PARM_TEMPORAL_DENOISE, metadata);
    case CAM_INTF_PARM_HISTOGRAM:
      return POINTER_OF_META(CAM_INTF_PARM_HISTOGRAM, metadata);
    case CAM_INTF_PARM_ASD_ENABLE:
      return POINTER_OF_META(CAM_INTF_PARM_ASD_ENABLE, metadata);
    case CAM_INTF_PARM_RECORDING_HINT:
      return POINTER_OF_META(CAM_INTF_PARM_RECORDING_HINT, metadata);
    case CAM_INTF_PARM_HDR:
      return POINTER_OF_META(CAM_INTF_PARM_HDR, metadata);
    case CAM_INTF_PARM_FRAMESKIP:
      return POINTER_OF_META(CAM_INTF_PARM_FRAMESKIP, metadata);
    case CAM_INTF_PARM_ZSL_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_ZSL_MODE, metadata);
    case CAM_INTF_PARM_HDR_NEED_1X:
      return POINTER_OF_META(CAM_INTF_PARM_HDR_NEED_1X, metadata);
    case CAM_INTF_PARM_LOCK_CAF:
      return POINTER_OF_META(CAM_INTF_PARM_LOCK_CAF, metadata);
    case CAM_INTF_PARM_VIDEO_HDR:
      return POINTER_OF_META(CAM_INTF_PARM_VIDEO_HDR, metadata);
    case CAM_INTF_PARM_VT:
      return POINTER_OF_META(CAM_INTF_PARM_VT, metadata);
    case CAM_INTF_META_USE_AV_TIMER:
      return POINTER_OF_META(CAM_INTF_META_USE_AV_TIMER, metadata);
    case CAM_INTF_PARM_SET_AUTOFOCUSTUNING:
      return POINTER_OF_META(CAM_INTF_PARM_SET_AUTOFOCUSTUNING, metadata);
    case CAM_INTF_PARM_SET_VFE_COMMAND:
      return POINTER_OF_META(CAM_INTF_PARM_SET_VFE_COMMAND, metadata);
    case CAM_INTF_PARM_SET_PP_COMMAND:
      return POINTER_OF_META(CAM_INTF_PARM_SET_PP_COMMAND, metadata);
    case CAM_INTF_PARM_MAX_DIMENSION:
      return POINTER_OF_META(CAM_INTF_PARM_MAX_DIMENSION, metadata);
    case CAM_INTF_PARM_RAW_DIMENSION:
      return POINTER_OF_META(CAM_INTF_PARM_RAW_DIMENSION, metadata);
    case CAM_INTF_PARM_TINTLESS:
      return POINTER_OF_META(CAM_INTF_PARM_TINTLESS, metadata);
    case CAM_INTF_PARM_WB_MANUAL:
      return POINTER_OF_META(CAM_INTF_PARM_WB_MANUAL, metadata);
    case CAM_INTF_PARM_EZTUNE_CMD:
      return POINTER_OF_META(CAM_INTF_PARM_EZTUNE_CMD, metadata);
    case CAM_INTF_PARM_INT_EVT:
      return POINTER_OF_META(CAM_INTF_PARM_INT_EVT, metadata);
    case CAM_INTF_PARM_RDI_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_RDI_MODE, metadata);
    case CAM_INTF_PARM_CDS_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_CDS_MODE, metadata);
    case CAM_INTF_PARM_BURST_NUM:
      return POINTER_OF_META(CAM_INTF_PARM_BURST_NUM, metadata);
    case CAM_INTF_PARM_RETRO_BURST_NUM:
      return POINTER_OF_META(CAM_INTF_PARM_RETRO_BURST_NUM, metadata);
    case CAM_INTF_PARM_BURST_LED_ON_PERIOD:
      return POINTER_OF_META(CAM_INTF_PARM_BURST_LED_ON_PERIOD, metadata);
    case CAM_INTF_PARM_LONGSHOT_ENABLE:
      return POINTER_OF_META(CAM_INTF_PARM_LONGSHOT_ENABLE, metadata);
    case CAM_INTF_META_STREAM_INFO:
      return POINTER_OF_META(CAM_INTF_META_STREAM_INFO, metadata);
    case CAM_INTF_META_AEC_MODE:
      return POINTER_OF_META(CAM_INTF_META_AEC_MODE, metadata);
    case CAM_INTF_META_TOUCH_AE_RESULT:
      return POINTER_OF_META(CAM_INTF_META_TOUCH_AE_RESULT, metadata);
    case CAM_INTF_META_AEC_PRECAPTURE_TRIGGER:
      return POINTER_OF_META(CAM_INTF_META_AEC_PRECAPTURE_TRIGGER, metadata);
    case CAM_INTF_META_AF_TRIGGER:
      return POINTER_OF_META(CAM_INTF_META_AF_TRIGGER, metadata);
    case CAM_INTF_META_CAPTURE_INTENT:
      return POINTER_OF_META(CAM_INTF_META_CAPTURE_INTENT, metadata);
    case CAM_INTF_META_DEMOSAIC:
      return POINTER_OF_META(CAM_INTF_META_DEMOSAIC, metadata);
    case CAM_INTF_META_SHARPNESS_STRENGTH:
      return POINTER_OF_META(CAM_INTF_META_SHARPNESS_STRENGTH, metadata);
    case CAM_INTF_META_GEOMETRIC_MODE:
      return POINTER_OF_META(CAM_INTF_META_GEOMETRIC_MODE, metadata);
    case CAM_INTF_META_GEOMETRIC_STRENGTH:
      return POINTER_OF_META(CAM_INTF_META_GEOMETRIC_STRENGTH, metadata);
    case CAM_INTF_META_LENS_SHADING_MAP_MODE:
      return POINTER_OF_META(CAM_INTF_META_LENS_SHADING_MAP_MODE, metadata);
    case CAM_INTF_META_ISP_SENSITIVITY:
      return POINTER_OF_META(CAM_INTF_META_ISP_SENSITIVITY, metadata);
    case CAM_INTF_META_SHADING_STRENGTH:
      return POINTER_OF_META(CAM_INTF_META_SHADING_STRENGTH, metadata);
    case CAM_INTF_META_TONEMAP_MODE:
      return POINTER_OF_META(CAM_INTF_META_TONEMAP_MODE, metadata);
    case CAM_INTF_META_AWB_INFO:
      return POINTER_OF_META(CAM_INTF_META_AWB_INFO, metadata);
    case CAM_INTF_META_FOCUS_POSITION:
      return POINTER_OF_META(CAM_INTF_META_FOCUS_POSITION, metadata);
    case CAM_INTF_META_USERZOOM:
      return POINTER_OF_META(CAM_INTF_META_USERZOOM, metadata);
    case CAM_INTF_META_STREAM_ID:
      return POINTER_OF_META(CAM_INTF_META_STREAM_ID, metadata);
    case CAM_INTF_PARM_STATS_DEBUG_MASK:
      return POINTER_OF_META(CAM_INTF_PARM_STATS_DEBUG_MASK, metadata);
    case CAM_INTF_PARM_STATS_AF_PAAF:
      return POINTER_OF_META(CAM_INTF_PARM_STATS_AF_PAAF, metadata);
    case CAM_INTF_PARM_FOCUS_BRACKETING:
      return POINTER_OF_META(CAM_INTF_PARM_FOCUS_BRACKETING, metadata);
    case CAM_INTF_PARM_FLASH_BRACKETING:
      return POINTER_OF_META(CAM_INTF_PARM_FLASH_BRACKETING, metadata);
    case CAM_INTF_META_JPEG_GPS_COORDINATES:
      return POINTER_OF_META(CAM_INTF_META_JPEG_GPS_COORDINATES, metadata);
    case CAM_INTF_META_JPEG_GPS_PROC_METHODS:
      return POINTER_OF_META(CAM_INTF_META_JPEG_GPS_PROC_METHODS, metadata);
    case CAM_INTF_META_JPEG_GPS_TIMESTAMP:
      return POINTER_OF_META(CAM_INTF_META_JPEG_GPS_TIMESTAMP, metadata);
    case CAM_INTF_META_JPEG_QUALITY:
      return POINTER_OF_META(CAM_INTF_META_JPEG_QUALITY, metadata);
    case CAM_INTF_META_OTP_WB_GRGB:
      return POINTER_OF_META(CAM_INTF_META_OTP_WB_GRGB, metadata);
    case CAM_INTF_META_JPEG_THUMB_QUALITY:
      return POINTER_OF_META(CAM_INTF_META_JPEG_THUMB_QUALITY, metadata);
    case CAM_INTF_META_JPEG_THUMB_SIZE:
      return POINTER_OF_META(CAM_INTF_META_JPEG_THUMB_SIZE, metadata);
    case CAM_INTF_META_JPEG_ORIENTATION:
      return POINTER_OF_META(CAM_INTF_META_JPEG_ORIENTATION, metadata);
    case CAM_INTF_META_PROFILE_TONE_CURVE:
      return POINTER_OF_META(CAM_INTF_META_PROFILE_TONE_CURVE, metadata);
    case CAM_INTF_META_NEUTRAL_COL_POINT:
      return POINTER_OF_META(CAM_INTF_META_NEUTRAL_COL_POINT, metadata);
    case CAM_INTF_META_SENSOR_ROLLING_SHUTTER_SKEW:
      return POINTER_OF_META(CAM_INTF_META_SENSOR_ROLLING_SHUTTER_SKEW,
        metadata);
    case CAM_INTF_PARM_CAC:
      return POINTER_OF_META(CAM_INTF_PARM_CAC, metadata);
    case CAM_INTF_META_IMG_HYST_INFO:
      return POINTER_OF_META(CAM_INTF_META_IMG_HYST_INFO, metadata);
    case CAM_INTF_META_CAC_INFO:
      return POINTER_OF_META(CAM_INTF_META_CAC_INFO, metadata);
    case CAM_INTF_META_TEST_PATTERN_DATA:
      return POINTER_OF_META(CAM_INTF_META_TEST_PATTERN_DATA, metadata);
    case CAM_INTF_PARM_UPDATE_DEBUG_LEVEL:
      return POINTER_OF_META(CAM_INTF_PARM_UPDATE_DEBUG_LEVEL, metadata);
    case CAM_INTF_PARM_ROTATION:
      return POINTER_OF_META(CAM_INTF_PARM_ROTATION, metadata);
    case CAM_INTF_PARM_FLIP:
      return POINTER_OF_META(CAM_INTF_PARM_FLIP, metadata);
    case CAM_INTF_PARM_TONE_MAP_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_TONE_MAP_MODE, metadata);
    case CAM_INTF_META_IMGLIB:
      return POINTER_OF_META(CAM_INTF_META_IMGLIB, metadata);
    case CAM_INTF_PARM_CAPTURE_FRAME_CONFIG:
      return POINTER_OF_META(CAM_INTF_PARM_CAPTURE_FRAME_CONFIG, metadata);
#ifdef ASF_OSD
    case CAM_INTF_META_ASF_TRIGGER_REGION:
      return POINTER_OF_META(CAM_INTF_META_ASF_TRIGGER_REGION, metadata);
#endif
    case CAM_INTF_META_SNAP_CROP_INFO_SENSOR:
      return POINTER_OF_META(CAM_INTF_META_SNAP_CROP_INFO_SENSOR, metadata);
    case CAM_INTF_META_SNAP_CROP_INFO_CAMIF:
      return POINTER_OF_META(CAM_INTF_META_SNAP_CROP_INFO_CAMIF, metadata);
    case CAM_INTF_META_SNAP_CROP_INFO_ISP:
      return POINTER_OF_META(CAM_INTF_META_SNAP_CROP_INFO_ISP, metadata);
    case CAM_INTF_META_SNAP_CROP_INFO_CPP:
      return POINTER_OF_META(CAM_INTF_META_SNAP_CROP_INFO_CPP, metadata);
    case CAM_INTF_PARM_CUSTOM:
      return POINTER_OF_META(CAM_INTF_PARM_CUSTOM, metadata);
    case CAM_INTF_PARM_RELATED_SENSORS_CALIBRATION:
      return POINTER_OF_META(CAM_INTF_PARM_RELATED_SENSORS_CALIBRATION,
        metadata);
    case CAM_INTF_META_AF_FOCAL_LENGTH_RATIO:
      return POINTER_OF_META(CAM_INTF_META_AF_FOCAL_LENGTH_RATIO, metadata);
    case CAM_INTF_META_DCRF:
      return POINTER_OF_META(CAM_INTF_META_DCRF, metadata);
    case CAM_INTF_BUF_DIVERT_INFO:
      return POINTER_OF_META(CAM_INTF_BUF_DIVERT_INFO, metadata);
    case CAM_INTF_META_LOW_LIGHT:
      return POINTER_OF_META(CAM_INTF_META_LOW_LIGHT, metadata);
    case CAM_INTF_META_IMG_DYN_FEAT:
      return POINTER_OF_META(CAM_INTF_META_IMG_DYN_FEAT, metadata);
    case CAM_INTF_AF_STATE_TRANSITION:
      return POINTER_OF_META(CAM_INTF_AF_STATE_TRANSITION, metadata);
    case CAM_INTF_PARM_DUAL_LED_CALIBRATION:
      return POINTER_OF_META(CAM_INTF_PARM_DUAL_LED_CALIBRATION, metadata);
    case CAM_INTF_PARM_INITIAL_EXPOSURE_INDEX:
      return POINTER_OF_META(CAM_INTF_PARM_INITIAL_EXPOSURE_INDEX, metadata);
    case CAM_INTF_PARM_SENSOR_HDR:
      return POINTER_OF_META(CAM_INTF_PARM_SENSOR_HDR, metadata);
    case CAM_INTF_PARM_INSTANT_AEC:
      return POINTER_OF_META(CAM_INTF_PARM_INSTANT_AEC, metadata);
    case CAM_INTF_PARM_ADV_CAPTURE_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_ADV_CAPTURE_MODE, metadata);
    case CAM_INTF_META_VIDEO_STAB_MODE:
      return POINTER_OF_META(CAM_INTF_META_VIDEO_STAB_MODE, metadata);
    case CAM_INTF_META_FOCUS_DEPTH_INFO:
      return POINTER_OF_META(CAM_INTF_META_FOCUS_DEPTH_INFO, metadata);
    case CAM_INTF_META_IR_MODE:
      return POINTER_OF_META(CAM_INTF_META_IR_MODE, metadata);
    case CAM_INTF_META_AEC_CONVERGENCE_SPEED:
      return POINTER_OF_META(CAM_INTF_META_AEC_CONVERGENCE_SPEED, metadata);
    case CAM_INTF_META_AWB_CONVERGENCE_SPEED:
      return POINTER_OF_META(CAM_INTF_META_AWB_CONVERGENCE_SPEED, metadata);
    case CAM_INTF_META_RAW:
      return POINTER_OF_META(CAM_INTF_META_RAW, metadata);
    case CAM_INTF_PARM_QUADRA_CFA:
      return POINTER_OF_META(CAM_INTF_PARM_QUADRA_CFA, metadata);
    case CAM_INTF_META_STREAM_INFO_FOR_PIC_RES:
      return POINTER_OF_META(CAM_INTF_META_STREAM_INFO_FOR_PIC_RES, metadata);
    case CAM_INTF_META_DC_SAC_OUTPUT_INFO:
      return POINTER_OF_META(CAM_INTF_META_DC_SAC_OUTPUT_INFO, metadata);
    case CAM_INTF_META_DC_LOW_POWER_ENABLE:
      return POINTER_OF_META(CAM_INTF_META_DC_LOW_POWER_ENABLE, metadata);
    case CAM_INTF_PARM_FOV_COMP_ENABLE:
      return POINTER_OF_META(CAM_INTF_PARM_FOV_COMP_ENABLE, metadata);
    case CAM_INTF_META_LED_CALIB_RESULT:
      return POINTER_OF_META(CAM_INTF_META_LED_CALIB_RESULT, metadata);
    case CAM_INTF_PARM_SYNC_DC_PARAMETERS:
      return POINTER_OF_META(CAM_INTF_PARM_SYNC_DC_PARAMETERS, metadata);
    case CAM_INTF_META_BINNING_CORRECTION_MODE:
      return POINTER_OF_META(CAM_INTF_META_BINNING_CORRECTION_MODE, metadata);
    case CAM_INTF_META_AEC_LUX_INDEX:
      return POINTER_OF_META(CAM_INTF_META_AEC_LUX_INDEX, metadata);
    case CAM_INTF_META_AF_OBJ_DIST_CM:
      return POINTER_OF_META(CAM_INTF_META_AF_OBJ_DIST_CM, metadata);
    case CAM_INTF_PARAM_BOKEH_BLUR_LEVEL:
      return POINTER_OF_META(CAM_INTF_PARAM_BOKEH_BLUR_LEVEL, metadata);
    case CAM_INTF_META_RTB_DATA:
      return POINTER_OF_META(CAM_INTF_META_RTB_DATA, metadata);
    case CAM_INTF_META_DC_CAPTURE:
      return POINTER_OF_META(CAM_INTF_META_DC_CAPTURE, metadata);
    case CAM_INTF_PARM_VFE1_RESERVED_RDI:
      return POINTER_OF_META(CAM_INTF_PARM_VFE1_RESERVED_RDI, metadata);
    case CAM_INTF_META_AF_FOCUS_POS:
      return POINTER_OF_META(CAM_INTF_META_AF_FOCUS_POS, metadata);
    case CAM_INTF_PARM_SKIP_FINE_SCAN:
      return POINTER_OF_META(CAM_INTF_PARM_SKIP_FINE_SCAN, metadata);
    case CAM_INTF_META_OIS_READ_DATA:
      return POINTER_OF_META(CAM_INTF_META_OIS_READ_DATA, metadata);
    case CAM_INTF_PARM_BOKEH_MODE:
      return POINTER_OF_META(CAM_INTF_PARM_BOKEH_MODE, metadata);
    case CAM_INTF_PARM_CLOSE_HINT:
      return POINTER_OF_META(CAM_INTF_PARM_CLOSE_HINT, metadata);
    case CAM_INTF_PARAM_COUNTER:
      return POINTER_OF_META(CAM_INTF_PARAM_COUNTER, metadata);
    default:
      CLOGD(CAM_MCT_MODULE, "case is not implemented for meta_id %d", meta_id);
      return NULL;
    }
}

uint32_t get_size_of(cam_intf_parm_type_t param_id)
{
  metadata_buffer_t* metadata = NULL;
  switch(param_id) {
    case CAM_INTF_META_HISTOGRAM:
      return SIZE_OF_PARAM(CAM_INTF_META_HISTOGRAM, metadata);
    case CAM_INTF_META_FACE_DETECTION:
      return SIZE_OF_PARAM(CAM_INTF_META_FACE_DETECTION, metadata);
    case CAM_INTF_META_FACE_RECOG:
      return SIZE_OF_PARAM(CAM_INTF_META_FACE_RECOG, metadata);
    case CAM_INTF_META_FACE_BLINK:
      return SIZE_OF_PARAM(CAM_INTF_META_FACE_BLINK, metadata);
    case CAM_INTF_META_FACE_GAZE:
      return SIZE_OF_PARAM(CAM_INTF_META_FACE_GAZE, metadata);
    case CAM_INTF_META_FACE_SMILE:
      return SIZE_OF_PARAM(CAM_INTF_META_FACE_SMILE, metadata);
    case CAM_INTF_META_FACE_LANDMARK:
      return SIZE_OF_PARAM(CAM_INTF_META_FACE_LANDMARK, metadata);
    case CAM_INTF_META_FACE_CONTOUR:
      return SIZE_OF_PARAM(CAM_INTF_META_FACE_CONTOUR, metadata);
    case CAM_INTF_META_AUTOFOCUS_DATA:
      return SIZE_OF_PARAM(CAM_INTF_META_AUTOFOCUS_DATA, metadata);
    case CAM_INTF_META_CROP_DATA:
      return SIZE_OF_PARAM(CAM_INTF_META_CROP_DATA, metadata);
    case CAM_INTF_META_PREP_SNAPSHOT_DONE:
      return SIZE_OF_PARAM(CAM_INTF_META_PREP_SNAPSHOT_DONE, metadata);
    case CAM_INTF_META_GOOD_FRAME_IDX_RANGE:
      return SIZE_OF_PARAM(CAM_INTF_META_GOOD_FRAME_IDX_RANGE, metadata);
    case CAM_INTF_META_ASD_HDR_SCENE_DATA:
      return SIZE_OF_PARAM(CAM_INTF_META_ASD_HDR_SCENE_DATA, metadata);
    case CAM_INTF_META_ASD_SCENE_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_ASD_SCENE_INFO, metadata);
    case CAM_INTF_META_CURRENT_SCENE:
      return SIZE_OF_PARAM(CAM_INTF_META_CURRENT_SCENE, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_ISP:
      return SIZE_OF_PARAM(CAM_INTF_META_CHROMATIX_LITE_ISP, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_PP:
      return SIZE_OF_PARAM(CAM_INTF_META_CHROMATIX_LITE_PP, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_AE:
      return SIZE_OF_PARAM(CAM_INTF_META_CHROMATIX_LITE_AE, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_AWB:
      return SIZE_OF_PARAM(CAM_INTF_META_CHROMATIX_LITE_AWB, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_AF:
      return SIZE_OF_PARAM(CAM_INTF_META_CHROMATIX_LITE_AF, metadata);
    case CAM_INTF_META_CHROMATIX_LITE_ASD:
      return SIZE_OF_PARAM(CAM_INTF_META_CHROMATIX_LITE_ASD, metadata);
    case CAM_INTF_META_FRAME_NUMBER_VALID:
      return SIZE_OF_PARAM(CAM_INTF_META_FRAME_NUMBER_VALID, metadata);
    case CAM_INTF_META_URGENT_FRAME_NUMBER_VALID:
      return SIZE_OF_PARAM(CAM_INTF_META_URGENT_FRAME_NUMBER_VALID, metadata);
    case CAM_INTF_META_FRAME_DROPPED:
      return SIZE_OF_PARAM(CAM_INTF_META_FRAME_DROPPED, metadata);
    case CAM_INTF_META_FRAME_NUMBER:
      return SIZE_OF_PARAM(CAM_INTF_META_FRAME_NUMBER, metadata);
    case CAM_INTF_META_URGENT_FRAME_NUMBER:
      return SIZE_OF_PARAM(CAM_INTF_META_URGENT_FRAME_NUMBER, metadata);
    case CAM_INTF_META_COLOR_CORRECT_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_COLOR_CORRECT_MODE, metadata);
    case CAM_INTF_META_COLOR_CORRECT_TRANSFORM:
      return SIZE_OF_PARAM(CAM_INTF_META_COLOR_CORRECT_TRANSFORM, metadata);
    case CAM_INTF_META_COLOR_CORRECT_GAINS:
      return SIZE_OF_PARAM(CAM_INTF_META_COLOR_CORRECT_GAINS, metadata);
    case CAM_INTF_META_PRED_COLOR_CORRECT_TRANSFORM:
      return SIZE_OF_PARAM(CAM_INTF_META_PRED_COLOR_CORRECT_TRANSFORM,
        metadata);
    case CAM_INTF_META_PRED_COLOR_CORRECT_GAINS:
      return SIZE_OF_PARAM(CAM_INTF_META_PRED_COLOR_CORRECT_GAINS, metadata);
    case CAM_INTF_META_AEC_ROI:
      return SIZE_OF_PARAM(CAM_INTF_META_AEC_ROI, metadata);
    case CAM_INTF_META_AEC_STATE:
      return SIZE_OF_PARAM(CAM_INTF_META_AEC_STATE, metadata);
    case CAM_INTF_PARM_FOCUS_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FOCUS_MODE, metadata);
    case CAM_INTF_PARM_MANUAL_FOCUS_POS:
      return SIZE_OF_PARAM(CAM_INTF_PARM_MANUAL_FOCUS_POS, metadata);
    case CAM_INTF_META_AF_ROI:
      return SIZE_OF_PARAM(CAM_INTF_META_AF_ROI, metadata);
    case CAM_INTF_META_AF_DEFAULT_ROI:
      return SIZE_OF_PARAM(CAM_INTF_META_AF_DEFAULT_ROI, metadata);
    case CAM_INTF_META_AF_STATE:
      return SIZE_OF_PARAM(CAM_INTF_META_AF_STATE, metadata);
    case CAM_INTF_PARM_WHITE_BALANCE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_WHITE_BALANCE, metadata);
    case CAM_INTF_META_AWB_REGIONS:
      return SIZE_OF_PARAM(CAM_INTF_META_AWB_REGIONS, metadata);
    case CAM_INTF_META_AWB_STATE:
      return SIZE_OF_PARAM(CAM_INTF_META_AWB_STATE, metadata);
    case CAM_INTF_META_BLACK_LEVEL_LOCK:
      return SIZE_OF_PARAM(CAM_INTF_META_BLACK_LEVEL_LOCK, metadata);
    case CAM_INTF_META_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_MODE, metadata);
    case CAM_INTF_META_EDGE_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_EDGE_MODE, metadata);
    case CAM_INTF_META_FLASH_POWER:
      return SIZE_OF_PARAM(CAM_INTF_META_FLASH_POWER, metadata);
    case CAM_INTF_META_FLASH_FIRING_TIME:
      return SIZE_OF_PARAM(CAM_INTF_META_FLASH_FIRING_TIME, metadata);
    case CAM_INTF_META_FLASH_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_FLASH_MODE, metadata);
    case CAM_INTF_META_FLASH_STATE:
      return SIZE_OF_PARAM(CAM_INTF_META_FLASH_STATE, metadata);
    case CAM_INTF_META_HOTPIXEL_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_HOTPIXEL_MODE, metadata);
    case CAM_INTF_META_LENS_APERTURE:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_APERTURE, metadata);
    case CAM_INTF_META_LENS_FILTERDENSITY:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_FILTERDENSITY, metadata);
    case CAM_INTF_META_LENS_FOCAL_LENGTH:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_FOCAL_LENGTH, metadata);
    case CAM_INTF_META_LENS_FOCUS_DISTANCE:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_FOCUS_DISTANCE, metadata);
    case CAM_INTF_META_FOCUS_VALUE:
      return SIZE_OF_PARAM(CAM_INTF_META_FOCUS_VALUE, metadata);
    case CAM_INTF_META_SPOT_LIGHT_DETECT:
      return SIZE_OF_PARAM(CAM_INTF_META_SPOT_LIGHT_DETECT, metadata);
    case CAM_INTF_META_LENS_FOCUS_RANGE:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_FOCUS_RANGE, metadata);
    case CAM_INTF_META_LENS_STATE:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_STATE, metadata);
    case CAM_INTF_META_LENS_OPT_STAB_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_OPT_STAB_MODE, metadata);
    case CAM_INTF_META_NOISE_REDUCTION_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_NOISE_REDUCTION_MODE, metadata);
    case CAM_INTF_META_NOISE_REDUCTION_STRENGTH:
      return SIZE_OF_PARAM(CAM_INTF_META_NOISE_REDUCTION_STRENGTH, metadata);
    case CAM_INTF_META_SCALER_CROP_REGION:
      return SIZE_OF_PARAM(CAM_INTF_META_SCALER_CROP_REGION, metadata);
    case CAM_INTF_META_SCENE_FLICKER:
      return SIZE_OF_PARAM(CAM_INTF_META_SCENE_FLICKER, metadata);
    case CAM_INTF_META_SENSOR_EXPOSURE_TIME:
      return SIZE_OF_PARAM(CAM_INTF_META_SENSOR_EXPOSURE_TIME, metadata);
    case CAM_INTF_META_SENSOR_FRAME_DURATION:
      return SIZE_OF_PARAM(CAM_INTF_META_SENSOR_FRAME_DURATION, metadata);
    case CAM_INTF_META_SENSOR_SENSITIVITY:
      return SIZE_OF_PARAM(CAM_INTF_META_SENSOR_SENSITIVITY, metadata);
    case CAM_INTF_META_SENSOR_TIMESTAMP:
      return SIZE_OF_PARAM(CAM_INTF_META_SENSOR_TIMESTAMP, metadata);
    case CAM_INTF_META_SHADING_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_SHADING_MODE, metadata);
    case CAM_INTF_META_STATS_FACEDETECT_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_STATS_FACEDETECT_MODE, metadata);
    case CAM_INTF_META_STATS_HISTOGRAM_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_STATS_HISTOGRAM_MODE, metadata);
    case CAM_INTF_META_STATS_SHARPNESS_MAP_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_STATS_SHARPNESS_MAP_MODE, metadata);
    case CAM_INTF_META_STATS_SHARPNESS_MAP:
      return SIZE_OF_PARAM(CAM_INTF_META_STATS_SHARPNESS_MAP, metadata);
    case CAM_INTF_META_TONEMAP_CURVES:
      return SIZE_OF_PARAM(CAM_INTF_META_TONEMAP_CURVES, metadata);
    case CAM_INTF_META_LENS_SHADING_MAP:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_SHADING_MAP, metadata);
    case CAM_INTF_META_AEC_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_AEC_INFO, metadata);
    case CAM_INTF_META_SENSOR_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_SENSOR_INFO, metadata);
    case CAM_INTF_META_EXIF_DEBUG_AE:
      return SIZE_OF_PARAM(CAM_INTF_META_EXIF_DEBUG_AE, metadata);
    case CAM_INTF_META_EXIF_DEBUG_AWB:
      return SIZE_OF_PARAM(CAM_INTF_META_EXIF_DEBUG_AWB, metadata);
    case CAM_INTF_META_EXIF_DEBUG_AF:
      return SIZE_OF_PARAM(CAM_INTF_META_EXIF_DEBUG_AF, metadata);
    case CAM_INTF_META_EXIF_DEBUG_ASD:
      return SIZE_OF_PARAM(CAM_INTF_META_EXIF_DEBUG_ASD, metadata);
    case CAM_INTF_META_EXIF_DEBUG_STATS:
      return SIZE_OF_PARAM(CAM_INTF_META_EXIF_DEBUG_STATS, metadata);
    case CAM_INTF_META_EXIF_DEBUG_BESTATS:
      return SIZE_OF_PARAM(CAM_INTF_META_EXIF_DEBUG_BESTATS, metadata);
    case CAM_INTF_META_EXIF_DEBUG_BHIST:
      return SIZE_OF_PARAM(CAM_INTF_META_EXIF_DEBUG_BHIST, metadata);
    case CAM_INTF_META_EXIF_DEBUG_3A_TUNING:
      return SIZE_OF_PARAM(CAM_INTF_META_EXIF_DEBUG_3A_TUNING, metadata);
    case CAM_INTF_PARM_EFFECT:
      return SIZE_OF_PARAM(CAM_INTF_PARM_EFFECT, metadata);
    case CAM_INTF_META_PRIVATE_DATA:
      return SIZE_OF_PARAM(CAM_INTF_META_PRIVATE_DATA, metadata);
    case CAM_INTF_PARM_HAL_VERSION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_HAL_VERSION, metadata);
    case CAM_INTF_PARM_ANTIBANDING:
      return SIZE_OF_PARAM(CAM_INTF_PARM_ANTIBANDING, metadata);
    case CAM_INTF_PARM_EXPOSURE_COMPENSATION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_EXPOSURE_COMPENSATION, metadata);
    case CAM_INTF_PARM_EV_STEP:
      return SIZE_OF_PARAM(CAM_INTF_PARM_EV_STEP, metadata);
    case CAM_INTF_PARM_AEC_LOCK:
      return SIZE_OF_PARAM(CAM_INTF_PARM_AEC_LOCK, metadata);
    case CAM_INTF_PARM_FPS_RANGE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FPS_RANGE, metadata);
    case CAM_INTF_PARM_AWB_LOCK:
      return SIZE_OF_PARAM(CAM_INTF_PARM_AWB_LOCK, metadata);
    case CAM_INTF_PARM_BESTSHOT_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_BESTSHOT_MODE, metadata);
    case CAM_INTF_PARM_DIS_ENABLE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_DIS_ENABLE, metadata);
    case CAM_INTF_PARM_LED_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_LED_MODE, metadata);
    case CAM_INTF_META_LED_MODE_OVERRIDE:
      return SIZE_OF_PARAM(CAM_INTF_META_LED_MODE_OVERRIDE, metadata);
    case CAM_INTF_PARM_QUERY_FLASH4SNAP:
      return SIZE_OF_PARAM(CAM_INTF_PARM_QUERY_FLASH4SNAP, metadata);
    case CAM_INTF_PARM_EXPOSURE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_EXPOSURE, metadata);
    case CAM_INTF_PARM_SHARPNESS:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SHARPNESS, metadata);
    case CAM_INTF_PARM_CONTRAST:
      return SIZE_OF_PARAM(CAM_INTF_PARM_CONTRAST, metadata);
    case CAM_INTF_PARM_SATURATION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SATURATION, metadata);
    case CAM_INTF_PARM_BRIGHTNESS:
      return SIZE_OF_PARAM(CAM_INTF_PARM_BRIGHTNESS, metadata);
    case CAM_INTF_PARM_ISO:
      return SIZE_OF_PARAM(CAM_INTF_PARM_ISO, metadata);
    case CAM_INTF_PARM_EXPOSURE_TIME:
      return SIZE_OF_PARAM(CAM_INTF_PARM_EXPOSURE_TIME, metadata);
    case CAM_INTF_PARM_USERZOOM:
      return SIZE_OF_PARAM(CAM_INTF_PARM_USERZOOM, metadata);
    case CAM_INTF_PARM_ROLLOFF:
      return SIZE_OF_PARAM(CAM_INTF_PARM_ROLLOFF, metadata);
    case CAM_INTF_PARM_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_MODE, metadata);
    case CAM_INTF_PARM_AEC_ALGO_TYPE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_AEC_ALGO_TYPE, metadata);
    case CAM_INTF_PARM_FOCUS_ALGO_TYPE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FOCUS_ALGO_TYPE, metadata);
    case CAM_INTF_PARM_AEC_ROI:
      return SIZE_OF_PARAM(CAM_INTF_PARM_AEC_ROI, metadata);
    case CAM_INTF_PARM_AF_ROI:
      return SIZE_OF_PARAM(CAM_INTF_PARM_AF_ROI, metadata);
    case CAM_INTF_PARM_SCE_FACTOR:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SCE_FACTOR, metadata);
    case CAM_INTF_PARM_FD:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FD, metadata);
    case CAM_INTF_PARM_MCE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_MCE, metadata);
    case CAM_INTF_PARM_HFR:
      return SIZE_OF_PARAM(CAM_INTF_PARM_HFR, metadata);
    case CAM_INTF_PARM_REDEYE_REDUCTION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_REDEYE_REDUCTION, metadata);
    case CAM_INTF_PARM_WAVELET_DENOISE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_WAVELET_DENOISE, metadata);
    case CAM_INTF_PARM_TEMPORAL_DENOISE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_TEMPORAL_DENOISE, metadata);
    case CAM_INTF_PARM_HISTOGRAM:
      return SIZE_OF_PARAM(CAM_INTF_PARM_HISTOGRAM, metadata);
    case CAM_INTF_PARM_ASD_ENABLE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_ASD_ENABLE, metadata);
    case CAM_INTF_PARM_RECORDING_HINT:
      return SIZE_OF_PARAM(CAM_INTF_PARM_RECORDING_HINT, metadata);
    case CAM_INTF_PARM_HDR:
      return SIZE_OF_PARAM(CAM_INTF_PARM_HDR, metadata);
    case CAM_INTF_PARM_FRAMESKIP:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FRAMESKIP, metadata);
    case CAM_INTF_PARM_ZSL_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_ZSL_MODE, metadata);
    case CAM_INTF_PARM_HDR_NEED_1X:
      return SIZE_OF_PARAM(CAM_INTF_PARM_HDR_NEED_1X, metadata);
    case CAM_INTF_PARM_LOCK_CAF:
      return SIZE_OF_PARAM(CAM_INTF_PARM_LOCK_CAF, metadata);
    case CAM_INTF_PARM_VIDEO_HDR:
      return SIZE_OF_PARAM(CAM_INTF_PARM_VIDEO_HDR, metadata);
    case CAM_INTF_PARM_VT:
      return SIZE_OF_PARAM(CAM_INTF_PARM_VT, metadata);
    case CAM_INTF_META_USE_AV_TIMER:
      return SIZE_OF_PARAM(CAM_INTF_META_USE_AV_TIMER, metadata);
    case CAM_INTF_META_USERZOOM:
      return SIZE_OF_PARAM(CAM_INTF_META_USERZOOM, metadata);
    case CAM_INTF_PARM_SET_AUTOFOCUSTUNING:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SET_AUTOFOCUSTUNING, metadata);
    case CAM_INTF_PARM_SET_VFE_COMMAND:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SET_VFE_COMMAND, metadata);
    case CAM_INTF_PARM_SET_PP_COMMAND:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SET_PP_COMMAND, metadata);
    case CAM_INTF_PARM_MAX_DIMENSION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_MAX_DIMENSION, metadata);
    case CAM_INTF_PARM_RAW_DIMENSION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_RAW_DIMENSION, metadata);
    case CAM_INTF_PARM_TINTLESS:
      return SIZE_OF_PARAM(CAM_INTF_PARM_TINTLESS, metadata);
    case CAM_INTF_PARM_WB_MANUAL:
      return SIZE_OF_PARAM(CAM_INTF_PARM_WB_MANUAL, metadata);
    case CAM_INTF_PARM_EZTUNE_CMD:
      return SIZE_OF_PARAM(CAM_INTF_PARM_EZTUNE_CMD, metadata);
    case CAM_INTF_PARM_INT_EVT:
      return SIZE_OF_PARAM(CAM_INTF_PARM_INT_EVT, metadata);
    case CAM_INTF_PARM_RDI_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_RDI_MODE, metadata);
    case CAM_INTF_PARM_CDS_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_CDS_MODE, metadata);
    case CAM_INTF_PARM_BURST_NUM:
      return SIZE_OF_PARAM(CAM_INTF_PARM_BURST_NUM, metadata);
    case CAM_INTF_PARM_RETRO_BURST_NUM:
      return SIZE_OF_PARAM(CAM_INTF_PARM_RETRO_BURST_NUM, metadata);
    case CAM_INTF_PARM_BURST_LED_ON_PERIOD:
      return SIZE_OF_PARAM(CAM_INTF_PARM_BURST_LED_ON_PERIOD, metadata);
    case CAM_INTF_PARM_LONGSHOT_ENABLE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_LONGSHOT_ENABLE, metadata);
    case CAM_INTF_META_STREAM_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_STREAM_INFO, metadata);
    case CAM_INTF_META_AEC_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_AEC_MODE, metadata);
    case CAM_INTF_META_TOUCH_AE_RESULT:
      return SIZE_OF_PARAM(CAM_INTF_META_TOUCH_AE_RESULT, metadata);
    case CAM_INTF_META_AEC_PRECAPTURE_TRIGGER:
      return SIZE_OF_PARAM(CAM_INTF_META_AEC_PRECAPTURE_TRIGGER, metadata);
    case CAM_INTF_META_AF_TRIGGER:
      return SIZE_OF_PARAM(CAM_INTF_META_AF_TRIGGER, metadata);
    case CAM_INTF_META_CAPTURE_INTENT:
      return SIZE_OF_PARAM(CAM_INTF_META_CAPTURE_INTENT, metadata);
    case CAM_INTF_META_DEMOSAIC:
      return SIZE_OF_PARAM(CAM_INTF_META_DEMOSAIC, metadata);
    case CAM_INTF_META_SHARPNESS_STRENGTH:
      return SIZE_OF_PARAM(CAM_INTF_META_SHARPNESS_STRENGTH, metadata);
    case CAM_INTF_META_GEOMETRIC_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_GEOMETRIC_MODE, metadata);
    case CAM_INTF_META_GEOMETRIC_STRENGTH:
      return SIZE_OF_PARAM(CAM_INTF_META_GEOMETRIC_STRENGTH, metadata);
    case CAM_INTF_META_LENS_SHADING_MAP_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_LENS_SHADING_MAP_MODE, metadata);
    case CAM_INTF_META_ISP_SENSITIVITY:
      return SIZE_OF_PARAM(CAM_INTF_META_ISP_SENSITIVITY, metadata);
    case CAM_INTF_META_SHADING_STRENGTH:
      return SIZE_OF_PARAM(CAM_INTF_META_SHADING_STRENGTH, metadata);
    case CAM_INTF_META_TONEMAP_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_TONEMAP_MODE, metadata);
    case CAM_INTF_META_AWB_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_AWB_INFO, metadata);
    case CAM_INTF_META_FOCUS_POSITION:
      return SIZE_OF_PARAM(CAM_INTF_META_FOCUS_POSITION, metadata);
    case CAM_INTF_META_STREAM_ID:
      return SIZE_OF_PARAM(CAM_INTF_META_STREAM_ID, metadata);
    case CAM_INTF_PARM_STATS_DEBUG_MASK:
      return SIZE_OF_PARAM(CAM_INTF_PARM_STATS_DEBUG_MASK, metadata);
    case CAM_INTF_PARM_STATS_AF_PAAF:
      return SIZE_OF_PARAM(CAM_INTF_PARM_STATS_AF_PAAF, metadata);
    case CAM_INTF_PARM_FOCUS_BRACKETING:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FOCUS_BRACKETING, metadata);
    case CAM_INTF_PARM_FLASH_BRACKETING:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FLASH_BRACKETING, metadata);
    case CAM_INTF_META_JPEG_GPS_COORDINATES:
      return SIZE_OF_PARAM(CAM_INTF_META_JPEG_GPS_COORDINATES, metadata);
    case CAM_INTF_META_JPEG_GPS_PROC_METHODS:
      return SIZE_OF_PARAM(CAM_INTF_META_JPEG_GPS_PROC_METHODS, metadata);
    case CAM_INTF_META_JPEG_GPS_TIMESTAMP:
      return SIZE_OF_PARAM(CAM_INTF_META_JPEG_GPS_TIMESTAMP, metadata);
    case CAM_INTF_META_OTP_WB_GRGB:
      return SIZE_OF_PARAM(CAM_INTF_META_OTP_WB_GRGB, metadata);
    case CAM_INTF_META_JPEG_QUALITY:
      return SIZE_OF_PARAM(CAM_INTF_META_JPEG_QUALITY, metadata);
    case CAM_INTF_META_JPEG_THUMB_QUALITY:
      return SIZE_OF_PARAM(CAM_INTF_META_JPEG_THUMB_QUALITY, metadata);
    case CAM_INTF_META_JPEG_THUMB_SIZE:
      return SIZE_OF_PARAM(CAM_INTF_META_JPEG_THUMB_SIZE, metadata);
    case CAM_INTF_META_JPEG_ORIENTATION:
      return SIZE_OF_PARAM(CAM_INTF_META_JPEG_ORIENTATION, metadata);
    case CAM_INTF_META_PROFILE_TONE_CURVE:
      return SIZE_OF_PARAM(CAM_INTF_META_PROFILE_TONE_CURVE, metadata);
    case CAM_INTF_META_NEUTRAL_COL_POINT:
      return SIZE_OF_PARAM(CAM_INTF_META_NEUTRAL_COL_POINT, metadata);
    case CAM_INTF_META_SENSOR_ROLLING_SHUTTER_SKEW:
      return SIZE_OF_PARAM(CAM_INTF_META_SENSOR_ROLLING_SHUTTER_SKEW,
                           metadata);
    case CAM_INTF_PARM_CAC:
      return SIZE_OF_PARAM(CAM_INTF_PARM_CAC, metadata);
    case CAM_INTF_META_IMG_HYST_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_IMG_HYST_INFO, metadata);
    case CAM_INTF_META_CAC_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_CAC_INFO, metadata);
    case CAM_INTF_META_TEST_PATTERN_DATA:
      return SIZE_OF_PARAM(CAM_INTF_META_TEST_PATTERN_DATA, metadata);
    case CAM_INTF_PARM_UPDATE_DEBUG_LEVEL:
      return SIZE_OF_PARAM(CAM_INTF_PARM_UPDATE_DEBUG_LEVEL, metadata);
    case CAM_INTF_PARM_ROTATION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_ROTATION, metadata);
    case CAM_INTF_PARM_FLIP:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FLIP, metadata);
    case CAM_INTF_PARM_TONE_MAP_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_TONE_MAP_MODE, metadata);
    case CAM_INTF_META_IMGLIB:
      return SIZE_OF_PARAM(CAM_INTF_META_IMGLIB, metadata);
    case CAM_INTF_PARM_CAPTURE_FRAME_CONFIG:
      return SIZE_OF_PARAM(CAM_INTF_PARM_CAPTURE_FRAME_CONFIG, metadata);
#ifdef ASF_OSD
    case CAM_INTF_META_ASF_TRIGGER_REGION:
      return SIZE_OF_PARAM(CAM_INTF_META_ASF_TRIGGER_REGION, metadata);
#endif
    case CAM_INTF_META_SNAP_CROP_INFO_SENSOR:
      return SIZE_OF_PARAM(CAM_INTF_META_SNAP_CROP_INFO_SENSOR, metadata);
    case CAM_INTF_META_SNAP_CROP_INFO_CAMIF:
      return SIZE_OF_PARAM(CAM_INTF_META_SNAP_CROP_INFO_CAMIF, metadata);
    case CAM_INTF_META_SNAP_CROP_INFO_ISP:
      return SIZE_OF_PARAM(CAM_INTF_META_SNAP_CROP_INFO_ISP, metadata);
    case CAM_INTF_META_SNAP_CROP_INFO_CPP:
      return SIZE_OF_PARAM(CAM_INTF_META_SNAP_CROP_INFO_CPP, metadata);
    case CAM_INTF_PARM_CUSTOM:
      return SIZE_OF_PARAM(CAM_INTF_PARM_CUSTOM, metadata);
    case CAM_INTF_PARM_RELATED_SENSORS_CALIBRATION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_RELATED_SENSORS_CALIBRATION,
                           metadata);
    case CAM_INTF_META_AF_FOCAL_LENGTH_RATIO:
      return SIZE_OF_PARAM(CAM_INTF_META_AF_FOCAL_LENGTH_RATIO, metadata);
    case CAM_INTF_META_DCRF:
      return SIZE_OF_PARAM(CAM_INTF_META_DCRF, metadata);
    case CAM_INTF_BUF_DIVERT_INFO:
      return SIZE_OF_PARAM(CAM_INTF_BUF_DIVERT_INFO, metadata);
    case CAM_INTF_META_LOW_LIGHT:
      return SIZE_OF_PARAM(CAM_INTF_META_LOW_LIGHT, metadata);
    case CAM_INTF_META_IMG_DYN_FEAT:
      return SIZE_OF_PARAM(CAM_INTF_META_IMG_DYN_FEAT, metadata);
    case CAM_INTF_AF_STATE_TRANSITION:
      return SIZE_OF_PARAM(CAM_INTF_AF_STATE_TRANSITION, metadata);
    case CAM_INTF_PARM_DUAL_LED_CALIBRATION:
      return SIZE_OF_PARAM(CAM_INTF_PARM_DUAL_LED_CALIBRATION, metadata);
    case CAM_INTF_PARM_INITIAL_EXPOSURE_INDEX:
      return SIZE_OF_PARAM(CAM_INTF_PARM_INITIAL_EXPOSURE_INDEX, metadata);
    case CAM_INTF_PARM_SENSOR_HDR:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SENSOR_HDR, metadata);
    case CAM_INTF_PARM_INSTANT_AEC:
      return SIZE_OF_PARAM(CAM_INTF_PARM_INSTANT_AEC, metadata);
    case CAM_INTF_PARM_ADV_CAPTURE_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_ADV_CAPTURE_MODE, metadata);
    case CAM_INTF_META_VIDEO_STAB_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_VIDEO_STAB_MODE, metadata);
    case CAM_INTF_META_FOCUS_DEPTH_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_FOCUS_DEPTH_INFO, metadata);
    case CAM_INTF_META_IR_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_IR_MODE, metadata);
    case CAM_INTF_META_AEC_CONVERGENCE_SPEED:
      return SIZE_OF_PARAM(CAM_INTF_META_AEC_CONVERGENCE_SPEED, metadata);
    case CAM_INTF_META_AWB_CONVERGENCE_SPEED:
      return SIZE_OF_PARAM(CAM_INTF_META_AWB_CONVERGENCE_SPEED, metadata);
    case CAM_INTF_META_RAW:
      return SIZE_OF_PARAM(CAM_INTF_META_RAW, metadata);
    case CAM_INTF_PARM_QUADRA_CFA:
      return SIZE_OF_PARAM(CAM_INTF_PARM_QUADRA_CFA, metadata);
    case CAM_INTF_META_STREAM_INFO_FOR_PIC_RES:
      return SIZE_OF_PARAM(CAM_INTF_META_STREAM_INFO_FOR_PIC_RES, metadata);
    case CAM_INTF_META_DC_SAC_OUTPUT_INFO:
      return SIZE_OF_PARAM(CAM_INTF_META_DC_SAC_OUTPUT_INFO, metadata);
    case CAM_INTF_META_DC_LOW_POWER_ENABLE:
      return SIZE_OF_PARAM(CAM_INTF_META_DC_LOW_POWER_ENABLE, metadata);
    case CAM_INTF_PARM_FOV_COMP_ENABLE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_FOV_COMP_ENABLE, metadata);
    case CAM_INTF_META_LED_CALIB_RESULT:
      return SIZE_OF_PARAM(CAM_INTF_META_LED_CALIB_RESULT, metadata);
    case CAM_INTF_PARM_SYNC_DC_PARAMETERS:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SYNC_DC_PARAMETERS, metadata);
    case CAM_INTF_META_BINNING_CORRECTION_MODE:
      return SIZE_OF_PARAM(CAM_INTF_META_BINNING_CORRECTION_MODE, metadata);
    case CAM_INTF_META_AEC_LUX_INDEX:
      return SIZE_OF_PARAM(CAM_INTF_META_AEC_LUX_INDEX, metadata);
    case CAM_INTF_META_AF_OBJ_DIST_CM:
      return SIZE_OF_PARAM(CAM_INTF_META_AF_OBJ_DIST_CM, metadata);
    case CAM_INTF_PARAM_BOKEH_BLUR_LEVEL:
      return SIZE_OF_PARAM(CAM_INTF_PARAM_BOKEH_BLUR_LEVEL, metadata);
    case CAM_INTF_META_RTB_DATA:
      return SIZE_OF_PARAM(CAM_INTF_META_RTB_DATA, metadata);
    case CAM_INTF_META_DC_CAPTURE:
      return SIZE_OF_PARAM(CAM_INTF_META_DC_CAPTURE, metadata);
    case CAM_INTF_PARM_VFE1_RESERVED_RDI:
      return SIZE_OF_PARAM(CAM_INTF_PARM_VFE1_RESERVED_RDI, metadata);
    case CAM_INTF_META_AF_FOCUS_POS:
      return SIZE_OF_PARAM(CAM_INTF_META_AF_FOCUS_POS, metadata);
    case CAM_INTF_PARM_SKIP_FINE_SCAN:
      return SIZE_OF_PARAM(CAM_INTF_PARM_SKIP_FINE_SCAN, metadata);
    case CAM_INTF_META_OIS_READ_DATA:
      return SIZE_OF_PARAM(CAM_INTF_META_OIS_READ_DATA, metadata);
    case CAM_INTF_PARM_BOKEH_MODE:
      return SIZE_OF_PARAM(CAM_INTF_PARM_BOKEH_MODE, metadata);
    case CAM_INTF_PARM_CLOSE_HINT:
      return SIZE_OF_PARAM(CAM_INTF_PARM_CLOSE_HINT, metadata);
    case CAM_INTF_PARAM_COUNTER:
      return SIZE_OF_PARAM(CAM_INTF_PARAM_COUNTER, metadata);
    default:
      CLOGD(CAM_MCT_MODULE, "case is not implemented for param_id %d",
        param_id);
      return 0;
    }
    return 0;
}
