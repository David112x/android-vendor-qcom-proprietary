/* csid.c
 *
 * Copyright (c) 2012-2015, 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "csid.h"
#include "sensor_common.h"

/** csid_get_version: get CSID version
 *
 *  @csid_ctrl: CSID control handle
 *  @data: handle where CSID version is filled
 *
 *  Get CSID version.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csid_get_version(void *csid_ctrl, void *data)
{
  sensor_csid_data_t *ctrl;
  uint32_t           *csid_version;

  RETURN_ERROR_ON_NULL(csid_ctrl);
  RETURN_ERROR_ON_NULL(data);
  ctrl = (sensor_csid_data_t *)csid_ctrl;
  csid_version = (uint32_t *)data;

  *csid_version = ctrl->csid_version;

  return SENSOR_SUCCESS;
}

/** csid_seccam_send_topology: send topology to TZ
 *
 *  @csid_ctrl: CSID control handle
 *  @data: struct containing CSID cfg
 *
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csid_seccam_send_topology(void *csid_ctrl, void *data)
{
  int32_t                       rc = 0;
  sensor_csid_data_t            *ctrl;
  struct msm_camera_csid_params csid_params_k;
  uint32_t                      *topology;
  struct csid_cfg_data          cfg;
  sensor_csi_config             *csi_cfg;

  RETURN_ERROR_ON_NULL(csid_ctrl);
  RETURN_ERROR_ON_NULL(data);
  ctrl = (sensor_csid_data_t *)csid_ctrl;
  csi_cfg = (sensor_csi_config *)data;

  memset(&csid_params_k, 0, sizeof(struct msm_camera_csid_params));

  csid_params_k.phy_sel = csi_cfg->phy_sel;
#ifdef SECURE_CAM_RST_MODULES
  csid_params_k.topology = csi_cfg->topology |
    (1 << csi_cfg->camera_csi_params->csid_core);
  SLOW("phy_sel %d topology %d", csid_params_k.phy_sel, csid_params_k.topology);

  cfg.cfgtype = CSID_SECCAM_TOPOLOGY;
  cfg.cfg.csid_params = &csid_params_k;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
#endif
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed rc :%d",rc);
    return SENSOR_FAILURE;
  }

  return SENSOR_SUCCESS;
}

/** csid_set_cfg: send reset cfg to TZ
 *
 *  @csid_ctrl: CSID control handle
 *  @data: struct containing CSID cfg
 *
 *  Pass CSID cfg to kernel and apply on CSID hw.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csid_seccam_reset_pipeline(void *csid_ctrl, void *data)
{
  int32_t                       rc = 0;
  sensor_csid_data_t            *ctrl;
  struct csid_cfg_data          cfg;
  sensor_csi_config             *csi_cfg;
  struct msm_camera_csid_params csid_params_k;

  RETURN_ERROR_ON_NULL(csid_ctrl);
  RETURN_ERROR_ON_NULL(data);
  ctrl = (sensor_csid_data_t *)csid_ctrl;
  csi_cfg = (sensor_csi_config *)data;

  memset(&csid_params_k, 0, sizeof(struct msm_camera_csid_params));

  csid_params_k.phy_sel = csi_cfg->phy_sel;
#ifdef SECURE_CAM_RST_MODULES
  csid_params_k.is_streamon = csi_cfg->is_stream_on;
  SLOW("phy_sel %d is_streamnon %d", csid_params_k.phy_sel, csid_params_k.is_streamon);
  cfg.cfgtype = CSID_SECCAM_RESET;
  cfg.cfg.csid_params = &csid_params_k;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
#endif
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed rc :%d",rc);
    return SENSOR_FAILURE;
  }

  return SENSOR_SUCCESS;
}

/** csid_set_cfg: set CSID cfg
 *
 *  @csid_ctrl: CSID control handle
 *  @data: struct containing CSID cfg
 *
 *  Pass CSID cfg to kernel and apply on CSID hw.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csid_set_cfg(void *csid_ctrl, void *data)
{
  int32_t                       rc = 0;
  sensor_csid_data_t            *ctrl;
  struct sensor_csi_params      *csi_params;
  sensor_csi_config             *csi_cfg;
  struct sensor_csid_lut_params *lut_params;
  struct csid_cfg_data          cfg;
  struct msm_camera_csid_params csid_params_k;
  struct msm_camera_csid_vc_cfg *csid_vc_cfg = NULL;
  int32_t i = 0;

  RETURN_ERROR_ON_NULL(csid_ctrl);
  RETURN_ERROR_ON_NULL(data);
  ctrl = (sensor_csid_data_t *)csid_ctrl;
  csi_cfg = (sensor_csi_config *)data;

  csi_params = csi_cfg->csi_params;
  lut_params = csi_cfg->lut_params;

  RETURN_ERROR_ON_NULL(csi_params);
  RETURN_ERROR_ON_NULL(lut_params);

  if ((csi_params == ctrl->cur_csi_cfg.csi_params) &&
      (csi_cfg->phy_sel == ctrl->cur_csi_cfg.phy_sel) &&
      (csi_cfg->lut_params == ctrl->cur_csi_cfg.lut_params) &&
      (csi_cfg->lut_params->vc_cfg_a[0].decode_format ==
        ctrl->cur_csi_cfg.lut_params->vc_cfg_a[0].decode_format)) {
    SLOW("same csid params");
    return SENSOR_SUCCESS;
  }

  /* Translate csid params from uspace structure to kernel struct */
  csid_vc_cfg = (struct msm_camera_csid_vc_cfg *)malloc(
    sizeof(*csid_vc_cfg) * lut_params->num_cid);
  RETURN_ERROR_ON_NULL(csid_vc_cfg);

  memset(&csid_params_k, 0, sizeof(struct msm_camera_csid_params));
  memset(csid_vc_cfg, 0, sizeof(struct msm_camera_csid_vc_cfg));

  translate_sensor_csid_params(
    &csid_params_k, csi_params, csid_vc_cfg, lut_params,
    csi_cfg->camera_csi_params);

  /* Value set to 0 so that it is taken from kernel */
  csid_params_k.csi_clk = 0;
  csid_params_k.phy_sel = csi_cfg->phy_sel;

  SLOW("csi params lane cnt %d phy_sel %d lane_assign %x csi_clk %d secure %d",
    csid_params_k.lane_cnt,
    csid_params_k.phy_sel,
    csid_params_k.lane_assign,
    csid_params_k.csi_clk,
#ifdef SECURE_CAMERA
    csid_params_k.is_secure
#else
    FALSE
#endif
    );

  for (i = 0; i < csid_params_k.lut_params.num_cid; i++) {
    SLOW("lut[%d] cid %d dt %x decode format %x", i,
      csid_params_k.lut_params.vc_cfg[i]->cid,
      csid_params_k.lut_params.vc_cfg[i]->dt,
      csid_params_k.lut_params.vc_cfg[i]->decode_format);
  }

  cfg.cfgtype = CSID_CFG;
  cfg.cfg.csid_params = &csid_params_k;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed rc :%d",rc);
  }

  ctrl->cur_csi_cfg = *csi_cfg;

  free(csid_vc_cfg);
  return SENSOR_SUCCESS;
}

/** csid_set_test_mode_params: set CSID test mode cfg
 *
 *  @csid_ctrl: CSID control handle
 *  @data: struct containing CSID test mode cfg
 *
 *  Pass CSID cfg to kernel and apply on CSID hw.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csid_set_test_mode_params(void *csid_ctrl, void *data)
{
  int32_t                               rc = 0;
  sensor_csid_data_t                    *ctrl;
  struct sensor_csid_testmode_parms     *csid_params;
  struct csid_cfg_data                  cfg;
  struct msm_camera_csid_testmode_parms csid_params_k;

  RETURN_ERROR_ON_NULL(csid_ctrl);
  RETURN_ERROR_ON_NULL(data);
  ctrl = (sensor_csid_data_t *)csid_ctrl;
  csid_params = (struct sensor_csid_testmode_parms *)data;

  if (csid_params->num_bytes_per_line == 0 &&
    csid_params->num_lines == 0) {
    SLOW("not a csid test mode");
    return SENSOR_SUCCESS;
  }

  /* Translate csid test params from uspace structure to kernel struct */
  translate_sensor_csid_test_params(&csid_params_k, csid_params);

  cfg.cfgtype = CSID_TESTMODE_CFG;
  cfg.cfg.csid_testmode_params = &csid_params_k;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed: rc: %d", rc);
  }

  return SENSOR_SUCCESS;
}

/** csid_open: CSID open sub module
 *
 *  @csid_ctrl: handle to fill CSID control pointer
 *  @subdev_name: CSID v4l subdev name
 *
 *  1) Allocate CSID control structure.
 *
 *  2) Open CSID v4l subdev.
 *
 *  3) Initialize CSID control structure.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csid_open(void **csid_ctrl, void* data)
{
  int32_t               rc = 0;
  sensor_csid_data_t    *ctrl = NULL;
  struct csid_cfg_data  cfg;
  char                  subdev_string[32];
  sensor_submodule_info_t *info =
      (sensor_submodule_info_t *)data;

  SLOW("Enter");

  RETURN_ERROR_ON_NULL(csid_ctrl);
  RETURN_ERROR_ON_NULL(info);
  RETURN_ERROR_ON_NULL(info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);

  ctrl = malloc(sizeof(sensor_csid_data_t));
  RETURN_ERROR_ON_NULL(ctrl);

  memset(ctrl, 0, sizeof(sensor_csid_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s",
      info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);

  SLOW("subdev name %s", subdev_string);

  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("failed");
    goto ERROR;
  }

  cfg.cfgtype = CSID_INIT;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed %s", strerror(errno));
    goto ERROR;
  }

  ctrl->csid_version = cfg.cfg.csid_version;
  *csid_ctrl = (void *)ctrl;

  SLOW("Exit");

  return SENSOR_SUCCESS;

ERROR:
  free(ctrl);
  return SENSOR_FAILURE;
}

/** csid_process: CSID process function
 *
 *  @csid_ctrl: handle to CSID control pointer
 *  @event: type of sensor_submodule_event_type_t
 *  @data: data specific to event
 *
 *  This function call config function based on event type.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csid_process(void *csid_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  RETURN_ERROR_ON_NULL(csid_ctrl);

  switch (event) {
  case CSID_GET_VERSION:
    csid_get_version(csid_ctrl, data);
    break;
  case CSID_SET_CFG:
    csid_set_cfg(csid_ctrl, data);
    break;
  case CSID_SET_TEST_MODE:
    csid_set_test_mode_params(csid_ctrl, data);
    break;
  case CSID_SECCAM_SEND_TOPOLOGY:
    csid_seccam_send_topology(csid_ctrl, data);
    break;
  case CSID_SECCAM_RESET_PIPELINE:
    csid_seccam_reset_pipeline(csid_ctrl, data);
    break;
  default:
    SERR("invalid event %d", event);
    return SENSOR_FAILURE;
  }

  return SENSOR_SUCCESS;
}

/** csid_close: CSID close sub module
 *
 *  @csid_ctrl: handle to CSID control pointer
 *
 *  This function closes subdev and frees control structure memory.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t csid_close(void *csid_ctrl)
{
  int32_t               rc = 0;
  int32_t               ret = SENSOR_SUCCESS;
  struct csid_cfg_data  cfg;
  sensor_csid_data_t    *ctrl;

  RETURN_ERROR_ON_NULL(csid_ctrl);
  ctrl = (sensor_csid_data_t *)csid_ctrl;

  SLOW("Enter");

  cfg.cfgtype = CSID_RELEASE;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed rc:%d",rc);
    ret = SENSOR_FAILURE;
  }

  close(ctrl->fd);
  free(ctrl);

  SDBG("Exit");

  return ret;
}

/** csid_sub_module_init:
 *
 *  @func_tbl: pointer to sensor function table
 *
 *  Initialize function table for CSID device to be used.
 *  This function executes in sensor module context.
 *
 *  Return: TRUE on success and FALSE on failure
 **/
int32_t csid_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  SDBG("Enter");
  RETURN_ERROR_ON_NULL(func_tbl);

  func_tbl->open = csid_open;
  func_tbl->process = csid_process;
  func_tbl->close = csid_close;

  return SENSOR_SUCCESS;
}
