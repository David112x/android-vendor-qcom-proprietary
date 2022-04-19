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
#include "btif/include/btif_storage.h"
#include "btif/include/btif_util.h"
#include "btm_int_types.h"
#include "btm_ble_api.h"


/*******************************************************************************
*
* Function         btm_iot_save_remote_properties
*
* Description      Store remote basic properties to iot conf file
*
* Returns          void
*
*******************************************************************************/
void btm_iot_save_remote_properties(tACL_CONN* p_acl_cb) {
  int name_length = 0;
  char value[1024];
  BD_NAME bd_name;
  bt_property_t prop_name;
  uint32_t cod = 0;
  tBT_DEVICE_TYPE dev_type;
  tBLE_ADDR_TYPE  addr_type;

  //save remote name to iot conf file
  if (strlen((const char *)p_acl_cb->remote_name))
  {
    name_length = strlen((char *)p_acl_cb->remote_name) > BTM_MAX_REM_BD_NAME_LEN ?
            BTM_MAX_REM_BD_NAME_LEN : strlen((char *)p_acl_cb->remote_name) + 1;
    strlcpy(value, (char*)p_acl_cb->remote_name, name_length);
    device_iot_config_addr_set_str(p_acl_cb->remote_addr,
            IOT_CONF_KEY_REMOTE_NAME, value);
  } else {
    if (BTM_GetRemoteDeviceName(p_acl_cb->remote_addr, bd_name))
    {
      device_iot_config_addr_set_str(p_acl_cb->remote_addr,
              IOT_CONF_KEY_REMOTE_NAME, (char *)bd_name);
    }
  }

  //save remote dev class to iot conf file
  cod = devclass2uint(p_acl_cb->remote_dc);
  if (cod == 0) {
    /* Try to retrieve cod from storage */
    BTIF_STORAGE_FILL_PROPERTY(&prop_name,
            BT_PROPERTY_CLASS_OF_DEVICE, sizeof(cod), &cod);
    if (btif_storage_get_remote_device_property(&p_acl_cb->remote_addr, &prop_name)
            == BT_STATUS_SUCCESS)
      BTIF_TRACE_DEBUG("%s cod retrieved from storage is 0x%06x", __func__, cod);
    if (cod == 0) {
      BTIF_TRACE_DEBUG("%s cod is 0, set as unclassified", __func__);
      cod = (0x1F) << 8;
    }
  }
  device_iot_config_addr_set_int(p_acl_cb->remote_addr,
          IOT_CONF_KEY_DEVCLASS, (int)cod);

  BTM_ReadDevInfo(p_acl_cb->remote_addr, &dev_type, &addr_type);

  //save remote dev type to iot conf file
  device_iot_config_addr_set_int(p_acl_cb->remote_addr,
          IOT_CONF_KEY_DEVTYPE, (int)dev_type);

  //save remote addr type to iot conf file
  device_iot_config_addr_set_int(p_acl_cb->remote_addr,
          IOT_CONF_KEY_ADDRTYPE, (int)addr_type);

  //save default recorded value to iot conf file
  device_iot_config_addr_set_int(p_acl_cb->remote_addr,
          IOT_CONF_KEY_RECORDED, IOT_CONF_VAL_RECORDED_DEFAULT);
}

/*******************************************************************************
*
* Function         btm_iot_save_remote_versions
*
* Description      Store remote versions to iot conf file
*
* Returns          void
*
*******************************************************************************/
void btm_iot_save_remote_versions(tACL_CONN* p_acl_cb) {
  device_iot_config_addr_set_int(p_acl_cb->remote_addr,
          IOT_CONF_KEY_MANUFACTURER, p_acl_cb->manufacturer);
  device_iot_config_addr_set_int(p_acl_cb->remote_addr,
          IOT_CONF_KEY_LMPVER, p_acl_cb->lmp_version);
  device_iot_config_addr_set_int(p_acl_cb->remote_addr,
          IOT_CONF_KEY_LMPSUBVER, p_acl_cb->lmp_subversion);
}

#endif
