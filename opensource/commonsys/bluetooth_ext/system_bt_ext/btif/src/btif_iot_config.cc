/******************************************************************************
 *  Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * *  Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * *  Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#include "bt_target.h"
#if (BT_IOT_LOGGING_ENABLED == TRUE)

#include "device_iot_config.h"
#include "bta_api.h"
#include "bt_target.h"
#include "btif_storage.h"

/*******************************************************************************
 *  Constants & Macros
 ******************************************************************************/
#define COD_UNCLASSIFIED ((0x1F) << 8)


/*******************************************************************************
*
* Function         btif_iot_save_pair_type
*
* Description      Store remote pair type to iot conf file
*
* Returns          void
*
*******************************************************************************/
static void btif_iot_save_pair_type(const RawAddress& bdaddr, bool is_ble, bool is_ssp) {
  if (is_ssp) {
    if (!is_ble)
      device_iot_config_addr_set_int(bdaddr,
              IOT_CONF_KEY_PAIRTYPE, IOT_CONF_VAL_PAIR_TYPE_SSP);
    else
      device_iot_config_addr_set_int(bdaddr,
              IOT_CONF_KEY_LE_PAIRTYPE, IOT_CONF_VAL_LE_PAIRTYPE_SECURE);
  } else {
    if (!is_ble)
      device_iot_config_addr_set_int(bdaddr,
              IOT_CONF_KEY_PAIRTYPE, IOT_CONF_VAL_PAIR_TYPE_LEGACY);
    else
      device_iot_config_addr_set_int(bdaddr,
              IOT_CONF_KEY_LE_PAIRTYPE, IOT_CONF_VAL_LE_PAIRTYPE_LEGACY);
  }
}

/*******************************************************************************
*
* Function         btif_iot_update_remote_info
*
* Description      Store remote dev info to iot conf file
*
* Returns          void
*
*******************************************************************************/
void btif_iot_update_remote_info(tBTA_DM_AUTH_CMPL* p_auth_cmpl, bool is_ble, bool is_ssp) {
  int name_length = 0;
  char value[1024];
  BD_NAME bd_name;
  int num_properties = 0;
  bt_property_t properties[2];
  uint32_t cod = 0;
  uint8_t lmp_ver = 0;
  uint16_t lmp_subver = 0;
  uint16_t mfct_set = 0;
  tBTM_STATUS btm_status;

  //save remote name to iot conf file
  if (strlen((const char *)p_auth_cmpl->bd_name))
  {
    name_length = strlen((char *)p_auth_cmpl->bd_name) > BTM_MAX_LOC_BD_NAME_LEN ?
            BTM_MAX_LOC_BD_NAME_LEN : strlen((char *)p_auth_cmpl->bd_name) + 1;
    strlcpy(value, (char*)p_auth_cmpl->bd_name, name_length);
    device_iot_config_addr_set_str(p_auth_cmpl->bd_addr,
            IOT_CONF_KEY_REMOTE_NAME, value);
  } else {
    if (BTM_GetRemoteDeviceName(p_auth_cmpl->bd_addr, bd_name))
    {
      device_iot_config_addr_set_str(p_auth_cmpl->bd_addr,
              IOT_CONF_KEY_REMOTE_NAME, (char *)bd_name);
    }
  }

  //save remote dev class to iot conf file
  //Try to retrieve cod from storage
  BTIF_STORAGE_FILL_PROPERTY(&properties[num_properties],
          BT_PROPERTY_CLASS_OF_DEVICE, sizeof(cod), &cod);
  if (btif_storage_get_remote_device_property(&p_auth_cmpl->bd_addr, &properties[num_properties])
          == BT_STATUS_SUCCESS)
    BTIF_TRACE_DEBUG("%s cod retrieved from storage is 0x%06x", __func__, cod);
  if (cod == 0) {
    BTIF_TRACE_DEBUG("%s cod is 0, set as unclassified", __func__);
    cod = COD_UNCLASSIFIED;
  }
  device_iot_config_addr_set_int(p_auth_cmpl->bd_addr,
          IOT_CONF_KEY_DEVCLASS, (int)cod);
  num_properties++;

  //save remote dev type to iot conf file
  bt_device_type_t dev_type;
  uint8_t remote_dev_type;
  BTIF_STORAGE_FILL_PROPERTY(&properties[num_properties],
          BT_PROPERTY_TYPE_OF_DEVICE, sizeof(uint8_t), &remote_dev_type);
  if (btif_storage_get_remote_device_property(&p_auth_cmpl->bd_addr, &properties[num_properties])
          == BT_STATUS_SUCCESS) {
    BTIF_TRACE_DEBUG("%s retrieve dev type from storage", __func__);
    dev_type = (bt_device_type_t)(remote_dev_type | p_auth_cmpl->dev_type);
  } else {
    dev_type = (bt_device_type_t)(p_auth_cmpl->dev_type);
  }
  device_iot_config_addr_set_int(p_auth_cmpl->bd_addr,
          IOT_CONF_KEY_DEVTYPE, (int)dev_type);

  //save remote addr type to iot conf file
  device_iot_config_addr_set_int(p_auth_cmpl->bd_addr,
          IOT_CONF_KEY_ADDRTYPE, (int)p_auth_cmpl->addr_type);

  //save remote versions to iot conf file
  btm_status = BTM_ReadRemoteVersion(p_auth_cmpl->bd_addr, &lmp_ver,
          &mfct_set, &lmp_subver);

  if (btm_status == BTM_SUCCESS)
  {
    device_iot_config_addr_set_int(p_auth_cmpl->bd_addr,
            IOT_CONF_KEY_MANUFACTURER, mfct_set);
    device_iot_config_addr_set_int(p_auth_cmpl->bd_addr,
            IOT_CONF_KEY_LMPVER, lmp_ver);
    device_iot_config_addr_set_int(p_auth_cmpl->bd_addr,
            IOT_CONF_KEY_LMPSUBVER, lmp_subver);
  }

  //save remote pair type to iot conf file
  btif_iot_save_pair_type(p_auth_cmpl->bd_addr, is_ble, is_ssp);

  device_iot_config_flush();
}

#endif
