/*
 * Copyright (C) 2016 The Linux Foundation. All rights reserved
 * Not a Contribution.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 * * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/************************************************************************************
 *
 *  Filename:      btif_tws_plus.cc
 *
 *  Description:   Vendor Bluetooth Interface
 *
 *
 ***********************************************************************************/

#define LOG_TAG "bt_btif_tws_plus"

#include "btif_config.h"
#include "btif_tws_plus.h"
#include "advertise_data_parser.h"
#include "osi/include/log.h"
#include "bta_tws_plus_api.h"
#include "btif_common.h"
#include "btif_storage.h"
#include "btif_util.h"
#include "osi/include/properties.h"

extern void bond_state_changed(bt_status_t status, const RawAddress& bd_addr,
                               bt_bond_state_t state);

/*******************************************************************************
** VENDOR INTERFACE FUNCTIONS
*******************************************************************************/


bt_status_t btif_tws_plus_replace_earbud ( RawAddress *addr,
                                    RawAddress* peer_bd_addr) {
    bt_status_t status = BT_STATUS_FAIL;

    LinkKey link_key;
    size_t size = OCTET16_LEN;
    uint8_t *key = &link_key[0];
    if (btif_config_get_bin(addr->ToString().c_str(), "LinkKey",
                                                key, &size)) {
        if(btif_tws_plus_derive_link_key( *addr, *peer_bd_addr, link_key,
                                     LK_DERIVATION_REASON_REPLACE_EB)) {
          status = BT_STATUS_SUCCESS;
        }
    }
    LOG_INFO(LOG_TAG,"replace_peer_earbud");
    return status;
}


/*******************************************************************************
** LOCAL FUNCTIONS
*******************************************************************************/

static bool btif_tws_plus_update_rmt_dev_props(RawAddress* bd_addr,
                                            RawAddress* peer_bd_addr) {
  bt_property_t remote_properties[8], properties_update[8];
  uint32_t num_props = 0, num_props_u = 0;

  bt_bdname_t name;
  uint32_t cod, devtype;
  bluetooth::Uuid remote_uuids[BT_MAX_NUM_UUIDS];
  memset(&name, 0, sizeof(bt_bdname_t));

  memset(remote_properties, 0, sizeof(remote_properties));

  if (*bd_addr != *peer_bd_addr) {

    BTIF_STORAGE_FILL_PROPERTY(&remote_properties[num_props], BT_PROPERTY_BDNAME,
                                sizeof(name), &name);
    if(BT_STATUS_SUCCESS != btif_storage_get_remote_device_property(peer_bd_addr,
                                          &remote_properties[num_props])) {
        LOG_ERROR(LOG_TAG,"name not found , fetching from other ear bud");
        BTIF_STORAGE_FILL_PROPERTY(&remote_properties[num_props], BT_PROPERTY_BDNAME,
                                sizeof(name), &name);
        btif_storage_get_remote_device_property(bd_addr,
                                          &remote_properties[num_props]);
        LOG_ERROR(LOG_TAG,"fetching from other ear bud %s", name.name);
        memcpy(&properties_update[num_props_u++], &remote_properties[num_props], sizeof remote_properties[0]);
    }
    num_props++;
  }

  BTIF_STORAGE_FILL_PROPERTY(&remote_properties[num_props],
                             BT_PROPERTY_CLASS_OF_DEVICE, sizeof(cod), &cod);
  btif_storage_get_remote_device_property(bd_addr,
                                          &remote_properties[num_props]);

  if (*bd_addr != *peer_bd_addr) {
      memcpy(&properties_update[num_props_u++], &remote_properties[num_props], sizeof remote_properties[0]);
  }
  num_props++;

  BTIF_STORAGE_FILL_PROPERTY(&remote_properties[num_props],
                             BT_PROPERTY_TYPE_OF_DEVICE, sizeof(devtype),
                             &devtype);
  btif_storage_get_remote_device_property(bd_addr,
                                          &remote_properties[num_props]);

  if (*bd_addr != *peer_bd_addr) {
      memcpy(&properties_update[num_props_u++], &remote_properties[num_props], sizeof remote_properties[0]);
  }
  num_props++;

  BTIF_STORAGE_FILL_PROPERTY(&remote_properties[num_props], BT_PROPERTY_UUIDS,
                             sizeof(remote_uuids), remote_uuids);
  btif_storage_get_remote_device_property(bd_addr,
                                          &remote_properties[num_props]);

  if (*bd_addr != *peer_bd_addr) {
      memcpy(&properties_update[num_props_u++], &remote_properties[num_props], sizeof remote_properties[0]);
  }
  num_props++;

  BTIF_TRACE_DEBUG("%s: remote device properties update: %d", __func__, num_props_u);
  btif_storage_set_remote_device_properties(peer_bd_addr, properties_update, num_props_u);

  HAL_CBACK(bt_hal_cbacks, remote_device_properties_cb, BT_STATUS_SUCCESS,
            peer_bd_addr, num_props, remote_properties);

  return true;
}


bool btif_tws_plus_auto_connect(RawAddress *addr, bool enable) {
  bt_vendor_property_t vnd_props[2];
  uint32_t num_vnd_props = 0;
  /* Peer earbud BD_ADDR */
  BTIF_STORAGE_FILL_PROPERTY(&vnd_props[num_vnd_props],
                   BT_VENDOR_PROPERTY_TWS_PLUS_AUTO_CONNECT,
                   sizeof(uint8_t), &enable);
  num_vnd_props++;

  HAL_CBACK(bt_vendor_callbacks, rmt_dev_prop_cb, BT_STATUS_SUCCESS,
                                    addr, num_vnd_props, vnd_props);
  return true;
}

bool btif_tws_plus_set_peer_eb_addr(RawAddress *addr, RawAddress *peer_addr) {
  bt_vendor_property_t vnd_props[3];
  uint32_t num_vnd_props = 0;

  if(!addr || !peer_addr) {
      LOG_ERROR(LOG_TAG," Invalid input address");
      return false;
  }
  /* Peer earbud BD_ADDR */
  BTIF_STORAGE_FILL_PROPERTY(&vnd_props[num_vnd_props],
                   BT_VENDOR_PROPERTY_TWS_PLUS_PEER_ADDR,
                   sizeof(RawAddress), peer_addr);
  num_vnd_props++;
  BTIF_TRACE_DEBUG("%s() Bd addr  src %s  dst %s ", __func__,
          addr->ToString().c_str(), peer_addr->ToString().c_str());

  BTA_TwsPlusUpdatePeerEbAddr (*addr, *peer_addr);

  HAL_CBACK(bt_vendor_callbacks, rmt_dev_prop_cb, BT_STATUS_SUCCESS,
                                    addr, num_vnd_props, vnd_props);
  if (*peer_addr != RawAddress::kEmpty) {
    return btif_config_set_str(addr->ToString().c_str(),
                                BTIF_STORAGE_PATH_TWS_PLUS_PEER_ADDR,
                                (char*) peer_addr->ToString().c_str());
  } else {
    return btif_config_remove(addr->ToString().c_str(),
                        BTIF_STORAGE_PATH_TWS_PLUS_PEER_ADDR);
  }
}

bool btif_tws_plus_get_peer_eb_addr(RawAddress *remote_bd_addr,
                                        RawAddress *peer_bd_addr) {
  char val[PROPERTY_VALUE_MAX] = "";
  int len = PROPERTY_VALUE_MAX;
  if(btif_config_get_str(remote_bd_addr->ToString().c_str(),
                         BTIF_STORAGE_PATH_TWS_PLUS_PEER_ADDR,
                                  (char*) val, &len)) {
    RawAddress::FromString(val, *peer_bd_addr);
    return true;
  }
  return false;
}

bool btif_tws_plus_set_dev_type(RawAddress *addr, int tws_plus_dev_type) {
  if(btif_config_set_int(addr->ToString().c_str(),
                         BTIF_STORAGE_PATH_TWS_PLUS_DEV_TYPE,
                         tws_plus_dev_type))  {
    btif_config_flush();
    return true;
  } else {
    return false;
  }
}

bool btif_tws_plus_get_dev_type(RawAddress *addr, int *tws_plus_dev_type) {
  return btif_config_get_int(addr->ToString().c_str(),
                             BTIF_STORAGE_PATH_TWS_PLUS_DEV_TYPE,
                             tws_plus_dev_type);
}

bool btif_is_tws_plus_device(const RawAddress *remote_bd_addr) {
  return btif_config_exist(remote_bd_addr->ToString().c_str(),
                        BTIF_STORAGE_PATH_TWS_PLUS_PEER_ADDR);
}

void btif_tws_plus_reverse_addr(RawAddress * addr) {
  uint8_t i = 0, tmp, len = sizeof(RawAddress);

  for (i = 0; i < len / 2; i++) {
    tmp = addr->address[i];
    addr->address[i] = addr->address[len - 1 - i];
    addr->address[len - 1 - i] = tmp;
  }
}

bool btif_tws_plus_process_eir(tBTA_DM_SEARCH *p_search_data,
                                        RawAddress *peer_eb_bd_addr)  {
  const uint8_t* p_eir_manufacure_data = NULL;
  uint8_t manufacure_data_len = 0;
  uint8_t peer_eb_addr_offset = 3;
  /* Check EIR for TWS_PLUS info */
  if (p_search_data->inq_res.p_eir) {

    p_eir_manufacure_data = AdvertiseDataParser::GetFieldByType(
          p_search_data->inq_res.p_eir, p_search_data->inq_res.eir_len,
          BTM_EIR_MANUFACTURER_SPECIFIC_TYPE, &manufacure_data_len);

    if (p_eir_manufacure_data) {
      LOG_ERROR(LOG_TAG,"process manudata: %x %x bd_addr: %x",
        p_eir_manufacure_data[0], p_eir_manufacure_data[1],
        manufacure_data_len - peer_eb_addr_offset);
      if (p_eir_manufacure_data[0] == 0x0A && p_eir_manufacure_data[1] == 0x00
           && ((manufacure_data_len - peer_eb_addr_offset) == 6)) {
        LOG_ERROR(LOG_TAG,"p_eir_manufacure_data found");
        memcpy(&peer_eb_bd_addr->address[0], p_eir_manufacure_data + peer_eb_addr_offset,
                                sizeof(RawAddress));
        btif_tws_plus_reverse_addr(peer_eb_bd_addr);
        LOG_ERROR(LOG_TAG,"btif_tws_plus_process_eir earbud: %s",
            peer_eb_bd_addr->ToString().c_str());
        return TRUE;
      }
    } else {
      LOG_ERROR(LOG_TAG,"p_eir_manufacure_data not found");
      return FALSE;
    }
  }
  return FALSE;
}

bool btif_tws_plus_get_services(RawAddress *bd_addr) {
    BTA_TwsPlusSdpSearch(*bd_addr);
    return true;
}

bool btif_tws_plus_load_tws_devices(void) {
  RawAddress bd_addr;
  RawAddress peer_bd_addr;
  char val[PROPERTY_VALUE_MAX] = "";
  int len = PROPERTY_VALUE_MAX;
  for (const btif_config_section_iter_t* iter = btif_config_section_begin();
       iter != btif_config_section_end();
       iter = btif_config_section_next(iter)) {
    const char* name = btif_config_section_name(iter);
    if (!RawAddress::IsValidAddress(name)) continue;

    BTIF_TRACE_DEBUG(" %s Remote device:%s", __func__, name);
    if (btif_config_get_str(name, BTIF_STORAGE_PATH_TWS_PLUS_PEER_ADDR,
                                  (char*) val, &len)) {
       BTIF_TRACE_DEBUG("%s() Bd addr  src %s  dst %s ", __func__,
              name, val);
       RawAddress::FromString(val, peer_bd_addr);
       RawAddress::FromString(name, bd_addr);
       btif_tws_plus_set_peer_eb_addr(&bd_addr, &peer_bd_addr);
    }
  }
  return true;
}

bool btif_tws_plus_derive_link_key ( RawAddress eb_addr, RawAddress peer_eb_addr,
                    LinkKey src_key, tLK_DERIVATION_REASON reason) {
  BTIF_TRACE_DEBUG("%s() Derive link for Bd addr %s ", __func__,
               peer_eb_addr.ToString().c_str());
  BTA_TwsPlusDeriveLinkKey( eb_addr, peer_eb_addr, src_key, reason);
  return true;
}

static void btif_tws_plus_upstreams_evt(uint16_t event, char* p_param) {
  tBTA_TWS_PLUS* p_data = (tBTA_TWS_PLUS*)p_param;
  LinkKey link_key;
  uint8_t *key = &link_key[0];
  size_t size = OCTET16_LEN;

  BTIF_TRACE_EVENT("%s:  event = %d", __func__, event);
  switch (event) {
    case BTA_TWS_PLUS_SDP_SEARCH_COMP_EVT: {
      if(BTA_TWS_PLUS_SUCCESS == p_data->sdp_search_comp.status &&
         (p_data->sdp_search_comp.peer_eb_addr != RawAddress::kEmpty)) {
          btif_tws_plus_reverse_addr(( RawAddress* ) &p_data->sdp_search_comp.peer_eb_addr);
          BTIF_TRACE_DEBUG("%s() Bd addr found from SDP query : %s ", __func__,
          p_data->sdp_search_comp.peer_eb_addr.ToString().c_str());

          btif_tws_plus_set_peer_eb_addr(&p_data->sdp_search_comp.eb_addr,
                                        &p_data->sdp_search_comp.peer_eb_addr);
          if (btif_config_get_bin(p_data->sdp_search_comp.eb_addr.ToString().c_str(),
                                  "LinkKey", key, &size)) {
            btif_tws_plus_derive_link_key(p_data->sdp_search_comp.eb_addr,
                                          p_data->sdp_search_comp.peer_eb_addr,
                                          link_key, LK_DERIVATION_REASON_PAIR);

          }
      } else {
          // update bond state changed for first device
          RawAddress eb_bd_addr = RawAddress::kEmpty;
          bond_state_changed(BT_STATUS_SUCCESS,
                p_data->sdp_search_comp.eb_addr, BT_BOND_STATE_BONDED);
          btif_tws_plus_update_rmt_dev_props(&p_data->sdp_search_comp.eb_addr,
                                            &p_data->sdp_search_comp.eb_addr);
          btif_tws_plus_set_peer_eb_addr(&p_data->sdp_search_comp.eb_addr,
                                         &eb_bd_addr);
      }
      break;
    }
    case BTA_TWS_PLUS_ENABLE_EVT: {
      break;
    }
    case BTA_TWS_PLUS_LK_DERIVED_EVT: {
        if(BTA_TWS_PLUS_SUCCESS == p_data->lk_derived.status) {
            tLK_DERIVATION_REASON reason = p_data->lk_derived.reason;

            BTIF_TRACE_DEBUG("%s:  BTA_TWS_PLUS_LK_DERIVED_EVT ", __func__);

            DEV_CLASS dev_class = {0, 0, 0};
            int pin_length = 0;
            int cod;
            int linkkey_type;
            if (btif_config_get_int(p_data->lk_derived.bd_addr.ToString().c_str(), "LinkKeyType", &linkkey_type)) {
               if (btif_config_get_int(p_data->lk_derived.bd_addr.ToString().c_str(), "DevClass", &cod)) {
                   uint2devclass((uint32_t)cod, dev_class);
                   btif_config_get_int(p_data->lk_derived.bd_addr.ToString().c_str(), "PinLength", &pin_length);

                   BTA_DmAddDevice(p_data->lk_derived.peer_eb_addr, dev_class,
                          p_data->lk_derived.key, 0, 0,
                        (uint8_t)linkkey_type, 0, pin_length);
                   BTIF_TRACE_DEBUG("%s:  BTA_TWS_PLUS_LK_DERIVED_EVT  added bonded device ", __func__);
                   btif_storage_add_bonded_device(
                   ( RawAddress* )  &p_data->lk_derived.peer_eb_addr,
                                     p_data->lk_derived.key,
                                     linkkey_type, pin_length);
                   btif_tws_plus_set_peer_eb_addr( ( RawAddress* ) &p_data->lk_derived.peer_eb_addr,
                                                    ( RawAddress* ) &p_data->lk_derived.bd_addr);
               }
            }
            if(reason == LK_DERIVATION_REASON_PAIR) {
                BTIF_TRACE_DEBUG("%s:  BTA_TWS_PLUS_LK_DERIVED_EVT  update for original ", __func__);
                bond_state_changed(BT_STATUS_SUCCESS,
                      p_data->lk_derived.bd_addr, BT_BOND_STATE_BONDED);
                btif_tws_plus_update_rmt_dev_props( &p_data->lk_derived.bd_addr,
                                                  &p_data->lk_derived.bd_addr);
            } else if(reason == LK_DERIVATION_REASON_REPLACE_EB) {
                RawAddress old_eb_bd_addr;
                if(btif_tws_plus_get_peer_eb_addr(&p_data->lk_derived.bd_addr,
                                               &old_eb_bd_addr)) {
                    BTIF_TRACE_DEBUG("%s:  BTA_TWS_PLUS_LK_DERIVED_EVT  update for old device ", __func__);
                    RawAddress peer_eb_bdaddr  = RawAddress::kEmpty;
                    btif_storage_remove_bonded_device(&old_eb_bd_addr);
                    BTA_DmRemoveDevice(old_eb_bd_addr);
                    bond_state_changed(BT_STATUS_SUCCESS, old_eb_bd_addr, BT_BOND_STATE_NONE);
                    btif_tws_plus_set_peer_eb_addr(&old_eb_bd_addr, &peer_eb_bdaddr);
                }
                btif_tws_plus_set_peer_eb_addr(&p_data->lk_derived.bd_addr,
                                               &p_data->lk_derived.peer_eb_addr);
            }

            BTIF_TRACE_DEBUG("%s:  BTA_TWS_PLUS_LK_DERIVED_EVT  bond state for new  ", __func__);
            bond_state_changed(BT_STATUS_SUCCESS,
                        p_data->lk_derived.peer_eb_addr, BT_BOND_STATE_BONDING);
            bond_state_changed(BT_STATUS_SUCCESS,
                        p_data->lk_derived.peer_eb_addr, BT_BOND_STATE_BONDED);

            if(reason == LK_DERIVATION_REASON_PAIR) {
                btif_tws_plus_auto_connect(&p_data->lk_derived.peer_eb_addr, false);
                btif_tws_plus_update_rmt_dev_props( &p_data->lk_derived.bd_addr,
                                                &p_data->lk_derived.peer_eb_addr);
                btif_tws_plus_auto_connect(&p_data->lk_derived.peer_eb_addr, true);

            } else {
                btif_tws_plus_update_rmt_dev_props( &p_data->lk_derived.bd_addr,
                                                   &p_data->lk_derived.peer_eb_addr);
            }
        }
      break;
    }
    default:
      break;
  }
}

void btif_tws_plus_callback(tBTA_TWS_PLUS_EVT event, tBTA_TWS_PLUS* p_data) {
  /* switch context to btif task context (copy full union size for convenience)
   */
  bt_status_t status = btif_transfer_context(
      btif_tws_plus_upstreams_evt, (uint16_t)event, (char*)p_data,
      sizeof(tBTA_TWS_PLUS), NULL);

  /* catch any failed context transfers */
  ASSERTC(status == BT_STATUS_SUCCESS, "context transfer failed", status);
}

/*******************************************************************************
 *
 * Function         btif_tws_plus_execute_service
 *
 * Description      Initializes/Shuts down the service
 *
 * Returns          BT_STATUS_SUCCESS on success, BT_STATUS_FAIL otherwise
 *
 ******************************************************************************/
bt_status_t btif_tws_plus_execute_service(bool b_enable) {
  BTIF_TRACE_DEBUG("%s enable:%d", __func__, b_enable);

  if (twsplus_enabled == true) {
    if (b_enable) {
      BTA_TwsPlusEnable(btif_tws_plus_callback);
      btif_tws_plus_load_tws_devices();
    } else {
      BTA_TwsPlusDisable();
    }
  }
  return BT_STATUS_SUCCESS;
}
