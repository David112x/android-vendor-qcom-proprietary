/* test_isp2.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <dlfcn.h>

/* mctl headers */
#include "mct_module.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"

/* isp headers */
#include "isp_module.h"
#include "isp_log.h"

#undef LOG_TAG
#define LOG_TAG "test-isp2"

#define WIDTH  640
#define HEIGHT 480

static mct_module_t *module = NULL;

static int32_t test_isp2()
{
  ISP_ERR("E");
  mct_event_t event;
  uint32_t i = 0;
  mct_stream_info_t mct_stream_info;
  mct_port_t *sink_port = NULL, *source_port = NULL;
  mct_port_t peer_port;
  uint32_t identity = 0;
  int32_t rc;
  void *chromatix_handle = NULL;
  void *chromatix_common_handle = NULL;
  void *(*open_lib)(void);
  modulesChromatix_t module_chromatix;
  chromatix_parms_type chromatixData;
  chromatix_VFE_common_type common_chromatixData;
  void *chromatixPtr;
  void *chromatixComPtr;

  module = module_isp_init("isp");
  if (!module) {
      ISP_ERR("failed");
      return 0;
  }
  identity = pack_identity(1, 1);
  /* Call set mod */
  module->set_mod(module, MCT_MODULE_FLAG_INDEXABLE, identity);
  if (module->start_session) {
    module->start_session(module, 1);
  }

  /* mct stream info */
  mct_stream_info.fmt = CAM_FORMAT_YUV_420_NV12;
  mct_stream_info.identity = identity;
  mct_stream_info.dim.width = WIDTH;
  mct_stream_info.dim.height = HEIGHT;
  mct_stream_info.stream_type = CAM_STREAM_TYPE_PREVIEW;
  mct_stream_info.streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;

  sink_port = MCT_PORT_CAST(MCT_MODULE_SINKPORTS(module)->data);

  ISP_ERR("caps_reserve: port=%p", sink_port);
  if(sink_port->check_caps_reserve(sink_port, NULL, &mct_stream_info) == TRUE) {
    /* add identity to port children */
    rc = mct_port_add_child(identity, sink_port);
    if (rc == FALSE) {
      ISP_ERR("failed to add child");
      return -1;
    }
  }

  ISP_ERR("link: port=%p", sink_port);
  sink_port->ext_link(identity, sink_port, &peer_port);

  source_port = MCT_PORT_CAST(MCT_MODULE_SRCPORTS(module)->data);

  ISP_ERR("caps_reserve: port=%p", source_port);
  if(source_port->check_caps_reserve(source_port, NULL, &mct_stream_info) == TRUE) {
    /* add identity to port children */
    rc = mct_port_add_child(identity, source_port);
    if (rc == FALSE) {
      ISP_ERR("failed to add child");
      return -1;
    }
  }

  ISP_ERR("link: port=%p", source_port);
  source_port->ext_link(identity, source_port, &peer_port);

  /* Open new chromatix library */
  /* Load chromatix ptr */
  chromatix_handle = dlopen("libchromatix_imx135_preview.so", RTLD_NOW);
  if (!chromatix_handle) {
    ISP_DBG("Failed to dlopen %s: %s", "libchromatix_imx135_preview.so",
      dlerror());
    return 0;
  }

  ISP_DBG("");
  /* Find function pointer to open */
  *(void **)&open_lib = dlsym(chromatix_handle, "load_chromatix");
  if (!open_lib) {
    ISP_DBG("Failed to find symbol: %s: :%s", "libchromatix_imx135_preview.so",
      dlerror());
    return 0;
  }

  ISP_DBG("");
  /* Call open on chromatix library and get chromatix pointer */
  chromatixPtr = open_lib();
  if (!chromatixPtr) {
    ISP_ERR("failed");
    return 0;
  }

  ISP_DBG("");
  /* common chromatix prt */
  chromatix_common_handle = dlopen("libchromatix_imx135_common.so", RTLD_NOW);
  if (!chromatix_common_handle) {
    ISP_DBG("Failed to dlopen %s: %s", "libchromatix_imx135_common.so",
      dlerror());
    return 0;
  }

  ISP_DBG("");
  /* Find function pointer to open */
  *(void **)&open_lib = dlsym(chromatix_common_handle, "load_chromatix");
  if (!open_lib) {
    ISP_DBG("Failed to find symbol: %s: :%s", "libchromatix_imx135_common.so",
      dlerror());
    return 0;
  }

  ISP_DBG("");
  /* Call open on chromatix library and get chromatix pointer */
  chromatixComPtr = open_lib();
  if (!chromatixComPtr) {
    ISP_ERR("failed");
    return 0;
  }

  ISP_DBG("");
  memcpy(&chromatixData, chromatixPtr, sizeof(chromatixData));
  ISP_DBG("");
  memcpy(&common_chromatixData, chromatixComPtr, sizeof(common_chromatixData));
  ISP_DBG("");
  module_chromatix.chromatixPtr = &chromatixData;
  module_chromatix.chromatixComPtr = &common_chromatixData;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_SET_CHROMATIX_PTR;
  event.u.module_event.module_event_data  = (void *)&module_chromatix;
  sink_port->event_func(sink_port, &event);

  ISP_ERR("stream on");
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  event.u.ctrl_event.control_event_data = (void *)&mct_stream_info;
  sink_port->event_func(sink_port, &event);

  //sleep(5);

  ISP_ERR("stream off");
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  sink_port->event_func(sink_port, &event);
  ISP_ERR("unlink");
  sink_port->un_link(identity, sink_port, &peer_port);

  ISP_ERR("caps_unreserve: port=%p", sink_port);
  sink_port->check_caps_unreserve(sink_port, identity);

  source_port->un_link(identity, source_port, &peer_port);

  ISP_ERR("caps_unreserve: port=%p", source_port);
  source_port->check_caps_unreserve(source_port, identity);
  ISP_ERR("");
  if (module->stop_session) {
    module->stop_session(module, 1);
  }
  ISP_ERR("");
  module_isp_deinit(module);
  ISP_ERR("");

  ISP_ERR("X");
  return 0;
}

int main(int argc __unused, char *argv[] __unused)
{
  ISP_ERR("test_isp2: E");
  test_isp2();
  ISP_ERR("test_isp2: X");
  return 0;
}
