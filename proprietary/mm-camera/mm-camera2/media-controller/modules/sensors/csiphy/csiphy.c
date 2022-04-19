/* csiphy.c
 *
 * Copyright (c) 2012-2015, 2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "csiphy.h"
#include "sensor_common.h"

/** csiphy_set_cfg: set CSIPHY cfg
 *
 *  @csiphy_ctrl: CSIPHY control handle
 *  @data: struct containing CSIPHY cfg
 *
 *  Pass CSIPHY cfg to kernel and apply on CSIPHY hw.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csiphy_set_cfg(void *csiphy_ctrl, void *data)
{
  int32_t                         rc = 0;
  sensor_csiphy_data_t           *ctrl;
  sensor_csi_config              *csi_cfg;
  struct sensor_csi_params       *csi_params;
  struct msm_camera_csiphy_params csi_phy_params;
  struct csiphy_cfg_data          cfg;

  SDBG("Enter");
  RETURN_ERROR_ON_NULL(csiphy_ctrl);
  RETURN_ERROR_ON_NULL(data);
  ctrl = (sensor_csiphy_data_t *)csiphy_ctrl;
  csi_cfg = (sensor_csi_config *)data;
  csi_params = csi_cfg->csi_params;

  if ((csi_params == ctrl->cur_csi_params) &&
    (csi_cfg->camera_csi_params == ctrl->cur_camera_csi_params) &&
    (csi_cfg->data_rate == ctrl->data_rate)) {
    SLOW("same csiphy params");
    return SENSOR_SUCCESS;
  }

  /* Translate csi phy params from uspace structure to kernel struct */
  translate_sensor_csiphy_params(&csi_phy_params, csi_params,
    csi_cfg->camera_csi_params, csi_cfg->data_rate);

  /* Value set to 0 so that it is taken from kernel */
  csi_phy_params.csiphy_clk = 0;

  SLOW("csi params lane cnt %d settle cnt %x lane mask %d combo %d",
    csi_phy_params.lane_cnt,
    csi_phy_params.settle_cnt,
    csi_phy_params.lane_mask,
    csi_phy_params.combo_mode);
  SLOW("csi params csid_core %d csiphy_clk %d csi_3phase %d",
    csi_phy_params.csid_core,
    csi_phy_params.csiphy_clk,
    csi_phy_params.csi_3phase);

  cfg.cfgtype = CSIPHY_CFG;
  cfg.cfg.csiphy_params = &csi_phy_params;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSIPHY_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSIPHY_IO_CFG failed: rc = %d",rc);
  }

  ctrl->cur_csi_params = csi_params;
  ctrl->cur_camera_csi_params = csi_cfg->camera_csi_params;
  ctrl->data_rate = csi_cfg->data_rate;

  return SENSOR_SUCCESS;
}

/** csiphy_open: CSIPHY open sub module
 *
 *  @csiphy_ctrl: handle to fill CSIPHY control pointer
 *  @subdev_name: CSIPHY v4l subdev name
 *
 *  1) Allocate CSIPHY control structure.
 *
 *  2) Open CSIPHY v4l subdev.
 *
 *  3) Initialize CSIPHY control structure.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csiphy_open(void **csiphy_ctrl,  void *data)
{
  int32_t                 rc = 0;
  sensor_csiphy_data_t    *ctrl = NULL;
  struct csiphy_cfg_data  cfg;
  char                    subdev_string[32];
  sensor_submodule_info_t *info =
      (sensor_submodule_info_t *)data;

  SDBG("Enter");

  RETURN_ERROR_ON_NULL(csiphy_ctrl);
  RETURN_ERROR_ON_NULL(info);
  RETURN_ERROR_ON_NULL(info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);

  ctrl = malloc(sizeof(sensor_csiphy_data_t));
  RETURN_ERROR_ON_NULL(ctrl);

  memset(ctrl, 0, sizeof(sensor_csiphy_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s",
      info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);

  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("failed");
    goto ERROR;
  }

  cfg.cfgtype = CSIPHY_INIT;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSIPHY_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSIPHY_IO_CFG failed: rc  = %d", rc);
    goto ERROR;
  }

  *csiphy_ctrl = (void *)ctrl;

  return SENSOR_SUCCESS;

ERROR:
  free(ctrl);
  return SENSOR_FAILURE;
}

/** csiphy_process: CSIPHY process function
 *
 *  @csiphy_ctrl: handle to CSIPHY control pointer
 *  @event: type of sensor_submodule_event_type_t
 *  @data: data specific to event
 *
 *  This function call config function based on event type.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csiphy_process(void *csiphy_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  RETURN_ERROR_ON_NULL(csiphy_ctrl);

  switch (event) {
  case CSIPHY_SET_CFG:
    csiphy_set_cfg(csiphy_ctrl, data);
    break;
  default:
    SHIGH("invalid event %d", event);
    return SENSOR_FAILURE;
    break;
  }

  return SENSOR_SUCCESS;
}

/** csiphy_close: CSIPHY close sub module
 *
 *  @csiphy_ctrl: handle to CSIPHY control pointer
 *
 *  This function closes subdev and frees control structure memory.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csiphy_close(void *csiphy_ctrl)
{
  int32_t                           rc = 0;
  int32_t                           ret = SENSOR_SUCCESS;
  sensor_csiphy_data_t              *ctrl;
  struct csiphy_cfg_data            cfg;
  struct msm_camera_csi_lane_params csi_lane_params;

  SDBG("Enter");

  RETURN_ERROR_ON_NULL(csiphy_ctrl);
  ctrl = (sensor_csiphy_data_t *)csiphy_ctrl;

  if (ctrl->cur_csi_params) {
    csi_lane_params.csi_lane_mask = ctrl->cur_camera_csi_params->lane_mask;
  }

  cfg.cfgtype = CSIPHY_RELEASE;
  cfg.cfg.csi_lane_params = &csi_lane_params;
  rc = ioctl(ctrl->fd, __user(VIDIOC_MSM_CSIPHY_IO_CFG), &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSIPHY_IO_CFG failed: rc  = %d", rc);
    ret = SENSOR_FAILURE;
  }

  /* close subdev */
  close(ctrl->fd);
  free(ctrl);

  return ret;
}

/** csiphy_sub_module_init:
 *
 *  @func_tbl: pointer to sensor function table
 *
 *  Initialize function table for CSIPHY device to be used.
 *  This function executes in sensor module context.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
int32_t csiphy_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  RETURN_ERROR_ON_NULL(func_tbl);

  func_tbl->open = csiphy_open;
  func_tbl->process = csiphy_process;
  func_tbl->close = csiphy_close;

  return SENSOR_SUCCESS;
}
