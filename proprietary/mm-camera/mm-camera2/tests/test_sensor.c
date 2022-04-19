/*============================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "mct_module.h"
#include "module_sensor.h"
#include "camera_dbg.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "sensor_debug.h"

#undef LOG_TAG
#define LOG_TAG "mm-camera-test-sensor"

#define STD_LOG(fmt, args...) SHIGH(fmt, ##args); printf(fmt"\n", ##args);

#define STREAM_WIDTH    640
#define STREAM_HEIGHT   480

#define STREAMING_TIME  5

mct_module_t *module = NULL;

int32_t test_sensor()
{
  SHIGH("E");
  mct_pipeline_cap_t query_buf;
  mct_event_t event;
  uint32_t i = 0;
  cam_stream_info_t stream_info;
  mct_stream_info_t mct_stream_info;
  mct_port_t *port=NULL;
  mct_port_t peer_port;
  uint32_t identity = 0;
  int32_t rc;

  module = module_sensor_init("sensor");
  if (!module) {
      SERR("failed");
      return 0;
  }

  identity = pack_identity(1, 1);
  /* Call set mod */
  module->set_mod(module, MCT_MODULE_FLAG_SOURCE, identity);
  module->start_session(module, 1);

  /* Call query capabilities */
  module->query_mod(module, &query_buf, 1);
  SERR("caps: mode %d position %d mount angle %d\n",
    query_buf.sensor_cap.modes_supported, query_buf.sensor_cap.position,
    query_buf.sensor_cap.sensor_mount_angle);
  SERR("caps: focal length %f hor view angle %f ver view angle %f",
    query_buf.sensor_cap.focal_length, query_buf.sensor_cap.hor_view_angle,
    query_buf.sensor_cap.ver_view_angle);

  /* cam stream info */
  stream_info.stream_type = CAM_STREAM_TYPE_PREVIEW;
  stream_info.dim.width = STREAM_WIDTH;
  stream_info.dim.height = STREAM_HEIGHT;

  /* mct stream info */
  mct_stream_info.fmt = CAM_FORMAT_YUV_420_NV12;
  mct_stream_info.identity = identity;
  mct_stream_info.dim.width = STREAM_WIDTH;
  mct_stream_info.dim.height = STREAM_HEIGHT;
  mct_stream_info.stream_type = CAM_STREAM_TYPE_PREVIEW;
  mct_stream_info.streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;


  port = MCT_PORT_CAST(MCT_MODULE_SRCPORTS(module)->data);

  STD_LOG("caps_reserve: port=%p", port);
  if(port->check_caps_reserve(port, NULL, &mct_stream_info) == TRUE) {
    /* add identity to port children */
    rc = mct_port_add_child(identity, port);
    if (rc == FALSE) {
      SERR("failed to add child");
      return -1;
    }
  }

  STD_LOG("link: port=%p", port);
  port->ext_link(identity, port, NULL);
  STD_LOG("stream on");
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  module->process_event(module, &event);

  STD_LOG("streaimng for %d seconds...", STREAMING_TIME);
  sleep(STREAMING_TIME);

  STD_LOG("stream off");
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  module->process_event(module, &event);
  STD_LOG("unlink");
  port->un_link(identity, port, NULL);

  STD_LOG("caps_unreserve: port=%p", port);
  port->check_caps_unreserve(port, identity);

  module->stop_session(module, 1);
  module_sensor_deinit(module);

  SHIGH("X");
  return 0;
}

int main(int argc __unused, char *argv[] __unused)
{
  STD_LOG("---- camera sensor test app ----");
  test_sensor();
  STD_LOG("test_sensor: done");
  return 0;
}
