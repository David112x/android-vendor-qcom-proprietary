/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __PPROC_PORT_H__
#define __PPROC_PORT_H__

#include "mct_stream.h"
#include "utils/pp_log.h"

#define PPROC_MAX_STREAM_PER_PORT 4
#define PPROC_MAX_SUBMODS 20

#ifdef MULTIPASS
//Use reproc count in filename for 2 step reprocess
#define CREATE_DUMP_FILENAME(buf, timebuf, pp_config, meta_frame_count,\
  stream_type_str, frame_id) \
  snprintf(buf, sizeof(buf), "%s_%d_%d_Metadata_%s_%d.bin", \
  timeBuf, pp_config.cur_reproc_count, meta_frame_count, \
  stream_type_str, frame_id);
#else
#define CREATE_DUMP_FILENAME(buf, timebuf, pp_config, meta_frame_count,\
  stream_type_str, frame_id) \
    snprintf(buf, sizeof(buf), "%s%d_Metadata_%s_%d.bin", \
    timeBuf, meta_frame_count, stream_type_str, frame_id);
#endif


#define INSERT_LLVD_AT_TOP(submodarr, num_submods, llvd) \
{ \
  if (llvd) { \
    int32_t index = num_submods, k = 0; \
    for (k = index; k > 0; k--) { \
      submodarr[k] = submodarr[k - 1]; \
      PP_DBG("k =%d, name %s, %s", k, \
        MCT_OBJECT_NAME(submodarr[k -1]), \
        MCT_OBJECT_NAME(submodarr[k])); \
    } \
    submodarr[k] = llvd; \
    num_submods++; \
    PP_DBG("after k =%d, name %s", k, MCT_OBJECT_NAME(submodarr[k])); \
  } \
}


#define IS_LLVD_BEFORE_TMOD(meta_stream_info, stream_info, llvd_before_tmod) \
{ \
  uint32_t i = 0; \
  bool flag1 = false, flag2 = false; \
  if (meta_stream_info && \
    ((stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW)|| \
    (stream_info->stream_type == CAM_STREAM_TYPE_VIDEO))) { \
    for (i = 0; i < meta_stream_info->num_streams; i++) { \
        PP_DBG("META_STREAM_INFO type = %d w x h : %d x %d pp_mask = 0x%x", \
          meta_stream_info->type[i], \
          meta_stream_info->stream_sizes[i].width, \
          meta_stream_info->stream_sizes[i].height, \
          meta_stream_info->postprocess_mask[i]); \
        if (EIS2_LLVD_ON_PREVIEW(meta_stream_info, i)) {\
          PP_DBG("EIS2_LLVD_ON_PREVIEW true"); \
          flag1 = true; \
        } \
        if (EIS3_LLVD_ON_VIDEO(meta_stream_info, i)) {\
          PP_DBG("EIS3_LLVD_ON_VIDEO true"); \
          flag2 = true; \
        } \
    } \
    if (flag1 && flag2)  { \
      PP_DBG("llvd_before_tmod true"); \
      llvd_before_tmod = true; \
    } \
  } \
}

#define EIS2_LLVD_ON_PREVIEW(meta_stream_info, i) \
  (((meta_stream_info->type[i] == CAM_STREAM_TYPE_PREVIEW) && \
    (meta_stream_info->is_type[i] == IS_TYPE_EIS_2_0) && \
    (meta_stream_info->postprocess_mask[i] & CAM_QCOM_FEATURE_LLVD)) \
    ? 1 : 0 \
    )

#define EIS3_LLVD_ON_VIDEO(meta_stream_info, i) \
  (((meta_stream_info->type[i] == CAM_STREAM_TYPE_VIDEO) && \
    (meta_stream_info->is_type[i] == IS_TYPE_EIS_3_0) && \
    (meta_stream_info->postprocess_mask[i] & CAM_QCOM_FEATURE_LLVD)) \
    ? 1 : 0 \
    )

mct_port_t *pproc_port_init(const char *name);
void pproc_port_deinit(mct_port_t *port);

boolean pproc_port_check_identity_in_port(void *data1, void *data2);
boolean pproc_port_check_port(mct_port_t *port, uint32_t identity);
mct_port_t *pproc_port_get_reserved_port(mct_module_t *module,
  uint32_t identity);
mct_stream_info_t *pproc_port_get_attached_stream_info(mct_port_t *port,
  uint32_t identity);
mct_port_t *pproc_port_resrv_port_on_module(mct_module_t *submod,
  mct_stream_info_t *stream_info, mct_port_direction_t direction,
  mct_port_t *pproc_port);
boolean pproc_port_check_meta_data_dump(cam_stream_type_t stream_type);

#endif /* __PPROC_PORT_H__ */
