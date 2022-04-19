
/* eebin_interface.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "sensor_common.h"
#include "eebin.h"
#include "eebin_interface.h"

/** eebin_control:
 *    @eebin_hdl: pointer to eeprom bin handle
 *
 * Initialize EEPROM BIN device to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SUCCESS
 * Failure - FAILURE
 **/
int32_t eebin_interface_control(void *eebin_hdl, eebin_ctl_t *bin_ctl)
{
  int32_t rc = 0;
  eebin_hdl_t *ctrl = NULL;

  SDBG("Enter");

  RETURN_ERROR_ON_NULL(eebin_hdl);
  RETURN_ERROR_ON_NULL(bin_ctl);

  ctrl = eebin_hdl;

  SLOW("cmd = %d", bin_ctl->cmd);

  switch(bin_ctl->cmd) {
  case EEPROM_BIN_GET_BIN_DATA:
    rc = eebin_get_bin_data(ctrl);
    break;

  case EEPROM_BIN_GET_NUM_DEV:
    rc = eebin_num_dev(eebin_hdl, bin_ctl->ctl.q_num.type,
      &bin_ctl->ctl.q_num.num_devs);
    break;

  case EEPROM_BIN_GET_DEV_DATA:
    rc = eebin_dev_data(eebin_hdl, &bin_ctl->ctl.dev_data);
    break;

  case EEPROM_BIN_GET_NAME_DATA:
    rc = eebin_name_data(eebin_hdl, &bin_ctl->ctl.name_data);
    break;

  case EEPROM_BIN_GET_LIB_NAME_DATA:
    rc = eebin_lib_name_data(eebin_hdl, &bin_ctl->ctl.name_data);
    break;

  default:
    break;
  }

  SDBG("Exit");

  return rc;
}
/** eebin_init:
 *    @hdl: pointer to eeprom bin handle
 *
 * Initialize EEPROM BIN device to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SUCCESS
 * Failure - FAILURE
 **/
int32_t eebin_interface_init(void **hdl)
{
  int32_t rc = 0;
  eebin_hdl_t *eebin_hdl = NULL;

  SDBG("Enter");

  RETURN_ERR_ON_NULL(hdl, -EINVAL);

  eebin_hdl = malloc(sizeof(eebin_hdl_t));
  RETURN_ERR_ON_NULL(eebin_hdl, -EINVAL);

  memset(eebin_hdl, 0, sizeof(eebin_hdl_t));

  eebin_hdl->ready = 0;

  *hdl = (void *)eebin_hdl;
  SDBG("Exit");

  return rc;
}

/** eebin_init:
 *    @eebin_hdl: pointer to eeprom bin handle
 *
 * Initialize EEPROM BIN device to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SUCCESS
 * Failure - FAILURE
 **/
int32_t eebin_interface_deinit(void *eebin_hdl)
{
  int32_t rc = 0;

  SDBG("Enter");

  RETURN_ERR_ON_NULL(eebin_hdl, -EINVAL);

  free(((eebin_hdl_t*)eebin_hdl)->out_buff);
  free(eebin_hdl);
  SDBG("Exit");

  return rc;
}
