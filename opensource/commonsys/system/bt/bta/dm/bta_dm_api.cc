/******************************************************************************
 *
 *  Copyright (C) 2003-2014 Broadcom Corporation
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

/******************************************************************************
 *
 *  This is the API implementation file for the BTA device manager.
 *
 ******************************************************************************/
#include <base/bind_helpers.h>
#include <string.h>

#include "bt_common.h"
#include "bta_api.h"
#include "bta_closure_api.h"
#include "bta_dm_int.h"
#include "bta_sys.h"
#include "bta_sys_int.h"
#include "btm_api.h"
#include "btm_int.h"
#include "osi/include/osi.h"
#include "utl.h"

using bluetooth::Uuid;

/*****************************************************************************
 *  Constants
 ****************************************************************************/

static const tBTA_SYS_REG bta_dm_reg = {bta_dm_sm_execute, bta_dm_sm_disable};

static const tBTA_SYS_REG bta_dm_search_reg = {bta_dm_search_sm_execute,
                                               bta_dm_search_sm_disable};

/*******************************************************************************
 *
 * Function         BTA_EnableBluetooth
 *
 * Description      Enables bluetooth service.  This function must be
 *                  called before any other functions in the BTA API are called.
 *
 *
 * Returns          tBTA_STATUS
 *
 ******************************************************************************/
tBTA_STATUS BTA_EnableBluetooth(tBTA_DM_SEC_CBACK* p_cback) {
  /* Bluetooth disabling is in progress */
  if (bta_dm_cb.disabling) return BTA_FAILURE;

  bta_sys_register(BTA_ID_DM, &bta_dm_reg);
  bta_sys_register(BTA_ID_DM_SEARCH, &bta_dm_search_reg);

  /* if UUID list is not provided as static data */
  bta_sys_eir_register(bta_dm_eir_update_uuid);

  tBTA_DM_API_ENABLE* p_msg =
      (tBTA_DM_API_ENABLE*)osi_malloc(sizeof(tBTA_DM_API_ENABLE));
  p_msg->hdr.event = BTA_DM_API_ENABLE_EVT;
  p_msg->p_sec_cback = p_cback;

  bta_sys_sendmsg(p_msg);

  return BTA_SUCCESS;
}

/*******************************************************************************
 *
 * Function         BTA_DisableBluetooth
 *
 * Description      Disables bluetooth service.  This function is called when
 *                  the application no longer needs bluetooth service
 *
 * Returns          void
 *
 ******************************************************************************/
tBTA_STATUS BTA_DisableBluetooth(void) {
  BT_HDR* p_msg = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  p_msg->event = BTA_DM_API_DISABLE_EVT;

  bta_sys_sendmsg(p_msg);

  return BTA_SUCCESS;
}

/*******************************************************************************
 *
 * Function         BTA_EnableTestMode
 *
 * Description      Enables bluetooth device under test mode
 *
 *
 * Returns          tBTA_STATUS
 *
 ******************************************************************************/
tBTA_STATUS BTA_EnableTestMode(void) {
  BT_HDR* p_msg = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  APPL_TRACE_API("%s", __func__);

  p_msg->event = BTA_DM_API_ENABLE_TEST_MODE_EVT;
  bta_sys_sendmsg(p_msg);

  return BTA_SUCCESS;
}

/*******************************************************************************
 *
 * Function         BTA_DisableTestMode
 *
 * Description      Disable bluetooth device under test mode
 *
 *
 * Returns          None
 *
 ******************************************************************************/
void BTA_DisableTestMode(void) {
  BT_HDR* p_msg = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  APPL_TRACE_API("%s", __func__);

  p_msg->event = BTA_DM_API_DISABLE_TEST_MODE_EVT;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmSetDeviceName
 *
 * Description      This function sets the Bluetooth name of local device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmSetDeviceName(char* p_name) {
  tBTA_DM_API_SET_NAME* p_msg =
      (tBTA_DM_API_SET_NAME*)osi_malloc(sizeof(tBTA_DM_API_SET_NAME));

  p_msg->hdr.event = BTA_DM_API_SET_NAME_EVT;
  strlcpy((char*)p_msg->name, p_name, BD_NAME_LEN+1);

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmSetVisibility
 *
 * Description      This function sets the Bluetooth connectable,
 *                  discoverable, pairable and conn paired only modes of local
 *                  device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmSetVisibility(tBTA_DM_DISC disc_mode, tBTA_DM_CONN conn_mode,
                         uint8_t pairable_mode, uint8_t conn_filter) {
  tBTA_DM_API_SET_VISIBILITY* p_msg =
      (tBTA_DM_API_SET_VISIBILITY*)osi_malloc(sizeof(tBTA_DM_MSG));

  p_msg->hdr.event = BTA_DM_API_SET_VISIBILITY_EVT;
  p_msg->disc_mode = disc_mode;
  p_msg->conn_mode = conn_mode;
  p_msg->pair_mode = pairable_mode;
  p_msg->conn_paired_only = conn_filter;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
**
** Function         BTA_DmHciRawCommand
**
** Description      This function sends the HCI Raw  command
**                  to the controller
**
**
** Returns          tBTA_STATUS
**
*******************************************************************************/
tBTA_STATUS BTA_DmHciRawCommand (uint16_t opcode, uint8_t param_len,
                                         uint8_t *p_param_buf,
                                         tBTA_RAW_CMPL_CBACK *p_cback)
{

  tBTA_DM_API_RAW_COMMAND *p_msg;
  uint16_t size;

  size = sizeof (tBTA_DM_API_RAW_COMMAND) + param_len;
  p_msg = (tBTA_DM_API_RAW_COMMAND *) osi_malloc(size);
  if (p_msg != NULL) {
    p_msg->hdr.event = BTA_DM_API_HCI_RAW_COMMAND_EVT;
    p_msg->opcode = opcode;
    p_msg->param_len = param_len;
    p_msg->p_param_buf = (uint8_t *)(p_msg + 1);
    p_msg->p_cback = p_cback;

    memcpy (p_msg->p_param_buf, p_param_buf, param_len);

    bta_sys_sendmsg(p_msg);
  }
  return BTA_SUCCESS;

}

/*******************************************************************************
 *
 * Function         BTA_DmSearch
 *
 * Description      This function searches for peer Bluetooth devices. It
 *                  performs an inquiry and gets the remote name for devices.
 *                  Service discovery is done if services is non zero
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmSearch(tBTA_DM_INQ* p_dm_inq, tBTA_SERVICE_MASK services,
                  tBTA_DM_SEARCH_CBACK* p_cback) {
  tBTA_DM_API_SEARCH* p_msg =
      (tBTA_DM_API_SEARCH*)osi_calloc(sizeof(tBTA_DM_API_SEARCH));

  p_msg->hdr.event = BTA_DM_API_SEARCH_EVT;
  memcpy(&p_msg->inq_params, p_dm_inq, sizeof(tBTA_DM_INQ));
  p_msg->services = services;
  p_msg->p_cback = p_cback;
  p_msg->rs_res = BTA_DM_RS_NONE;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmSearchCancel
 *
 * Description      This function  cancels a search initiated by BTA_DmSearch
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmSearchCancel(void) {
  BT_HDR* p_msg = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  p_msg->event = BTA_DM_API_SEARCH_CANCEL_EVT;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmPowerBackOff
 *
 * Description      This function sets/resets PowerBackOff
 *
 * Returns          void
 *
 *******************************************************************************/
void BTA_DmPowerBackOff(bool status) {
  tBTA_DM_API_SET_PWR_BACKOFF *p_msg =
      (tBTA_DM_API_SET_PWR_BACKOFF *)osi_calloc(sizeof(tBTA_DM_API_SET_PWR_BACKOFF));

  p_msg->hdr.event = BTA_DM_API_SET_PWR_BACKOFF_EVT;
  p_msg->status = status;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmSetWifiState
 *
 * Description      This function sets wifi on/off state
 *
 * Returns          void
 *
 *******************************************************************************/
void BTA_DmSetWifiState(bool status) {
  tBTA_DM_API_SET_WIFI_STATE *p_msg =
      (tBTA_DM_API_SET_WIFI_STATE *)osi_calloc(sizeof(tBTA_DM_API_SET_WIFI_STATE));

  p_msg->hdr.event = BTA_DM_API_SET_WIFI_STATE_EVT;
  p_msg->status = status;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBredrCleanup
 *
 * Description      This function do br/edr cleanup
 *
 * Returns          void
 *
 *******************************************************************************/
void BTA_DmBredrCleanup(void) {
  tBTA_DM_API_BREDR_CLEANUP *p_msg =
      (tBTA_DM_API_BREDR_CLEANUP*)osi_calloc(sizeof(tBTA_DM_API_BREDR_CLEANUP));

  p_msg->hdr.event = BTA_DM_API_BREDR_CLEANUP_EVT;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBredrStartup
 *
 * Description      This function do br/edr startup
 *
 * Returns          void
 *
 *******************************************************************************/
void BTA_DmBredrStartup(void) {
  tBTA_DM_API_BREDR_STARTUP *p_msg =
      (tBTA_DM_API_BREDR_STARTUP*)osi_calloc(sizeof(tBTA_DM_API_BREDR_STARTUP));

  p_msg->hdr.event = BTA_DM_API_BREDR_STARTUP_EVT;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
**
 * Function         BTA_DmDiscover
 *
 * Description      This function does service discovery for services of a
 *                  peer device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmDiscover(const RawAddress& bd_addr, tBTA_SERVICE_MASK services,
                    tBTA_DM_SEARCH_CBACK* p_cback, bool sdp_search) {
  tBTA_DM_API_DISCOVER* p_msg =
      (tBTA_DM_API_DISCOVER*)osi_calloc(sizeof(tBTA_DM_API_DISCOVER));

  p_msg->hdr.event = BTA_DM_API_DISCOVER_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->services = services;
  p_msg->p_cback = p_cback;
  p_msg->sdp_search = sdp_search;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmDiscoverUUID
 *
 * Description      This function does service discovery for services of a
 *                  peer device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmDiscoverUUID(const RawAddress& bd_addr, const Uuid& uuid,
                        tBTA_DM_SEARCH_CBACK* p_cback, bool sdp_search) {
  tBTA_DM_API_DISCOVER* p_msg =
      (tBTA_DM_API_DISCOVER*)osi_malloc(sizeof(tBTA_DM_API_DISCOVER));

  p_msg->hdr.event = BTA_DM_API_DISCOVER_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->services = BTA_USER_SERVICE_MASK;  // Not exposed at API level
  p_msg->p_cback = p_cback;
  p_msg->sdp_search = sdp_search;

  p_msg->num_uuid = 0;
  p_msg->p_uuid = NULL;
  p_msg->uuid = uuid;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBond
 *
 * Description      This function initiates a bonding procedure with a peer
 *                  device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBond(const RawAddress& bd_addr) {
  tBTA_DM_API_BOND* p_msg =
      (tBTA_DM_API_BOND*)osi_malloc(sizeof(tBTA_DM_API_BOND));

  p_msg->hdr.event = BTA_DM_API_BOND_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->transport = BTA_TRANSPORT_UNKNOWN;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBondByTransports
 *
 * Description      This function initiates a bonding procedure with a peer
 *                  device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBondByTransport(const RawAddress& bd_addr,
                           tBTA_TRANSPORT transport) {
  tBTA_DM_API_BOND* p_msg =
      (tBTA_DM_API_BOND*)osi_malloc(sizeof(tBTA_DM_API_BOND));

  p_msg->hdr.event = BTA_DM_API_BOND_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->transport = transport;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBondCancel
 *
 * Description      This function cancels the bonding procedure with a peer
 *                  device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBondCancel(const RawAddress& bd_addr) {
  tBTA_DM_API_BOND_CANCEL* p_msg =
      (tBTA_DM_API_BOND_CANCEL*)osi_malloc(sizeof(tBTA_DM_API_BOND_CANCEL));

  p_msg->hdr.event = BTA_DM_API_BOND_CANCEL_EVT;
  p_msg->bd_addr = bd_addr;

  bta_sys_sendmsg(p_msg);
}


/*******************************************************************************
 *
 * Function         BTA_DmResetPairingflag
 *
 * Description      This function reset pairing flag
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmResetPairingflag(const RawAddress& bd_addr) {
  tBTA_DM_API_RST_PAIR_FLAG* p_msg =
      (tBTA_DM_API_RST_PAIR_FLAG*)osi_malloc(sizeof(tBTA_DM_API_RST_PAIR_FLAG));

  p_msg->hdr.event = BTA_DM_API_RST_PAIR_FLAG_EVT;
  p_msg->bd_addr = bd_addr;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmPinReply
 *
 * Description      This function provides a pincode for a remote device when
 *                  one is requested by DM through BTA_DM_PIN_REQ_EVT
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmPinReply(const RawAddress& bd_addr, bool accept, uint8_t pin_len,
                    uint8_t* p_pin)

{
  tBTA_DM_API_PIN_REPLY* p_msg =
      (tBTA_DM_API_PIN_REPLY*)osi_malloc(sizeof(tBTA_DM_API_PIN_REPLY));

  p_msg->hdr.event = BTA_DM_API_PIN_REPLY_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->accept = accept;
  if (accept) {
    p_msg->pin_len = pin_len;
    memcpy(p_msg->p_pin, p_pin, pin_len);
  }

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmLocalOob
 *
 * Description      This function retrieves the OOB data from local controller.
 *                  The result is reported by:
 *                  - bta_dm_co_loc_oob_ext() if device supports secure
 *                    connections (SC)
 *                  - bta_dm_co_loc_oob() if device doesn't support SC
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmLocalOob(void) {
  tBTA_DM_API_LOC_OOB* p_msg =
      (tBTA_DM_API_LOC_OOB*)osi_malloc(sizeof(tBTA_DM_API_LOC_OOB));

  p_msg->hdr.event = BTA_DM_API_LOC_OOB_EVT;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmConfirm
 *
 * Description      This function accepts or rejects the numerical value of the
 *                  Simple Pairing process on BTA_DM_SP_CFM_REQ_EVT
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmConfirm(const RawAddress& bd_addr, bool accept) {
  tBTA_DM_API_CONFIRM* p_msg =
      (tBTA_DM_API_CONFIRM*)osi_malloc(sizeof(tBTA_DM_API_CONFIRM));

  p_msg->hdr.event = BTA_DM_API_CONFIRM_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->accept = accept;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmAddDevice
 *
 * Description      This function adds a device to the security database list of
 *                  peer device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmAddDevice(const RawAddress& bd_addr, DEV_CLASS dev_class,
                     const LinkKey& link_key, tBTA_SERVICE_MASK trusted_mask,
                     bool is_trusted, uint8_t key_type, tBTA_IO_CAP io_cap,
                     uint8_t pin_length) {

  tBTA_DM_API_ADD_DEVICE* p_msg =
      (tBTA_DM_API_ADD_DEVICE*)osi_calloc(sizeof(tBTA_DM_API_ADD_DEVICE));

  p_msg->hdr.event = BTA_DM_API_ADD_DEVICE_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->tm = trusted_mask;
  p_msg->is_trusted = is_trusted;
  p_msg->io_cap = io_cap;
  p_msg->key_type = key_type;
  p_msg->link_key_known = true;
  p_msg->link_key = link_key;

  /* Load device class if specified */
  if (dev_class) {
    p_msg->dc_known = true;
    memcpy(p_msg->dc, dev_class, DEV_CLASS_LEN);
  }

  memset(p_msg->bd_name, 0, BD_NAME_LEN + 1);
  memset(p_msg->features, 0, sizeof(p_msg->features));
  p_msg->pin_length = pin_length;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmRemoveDevice
 *
 * Description      This function removes a device fromthe security database
 *                  list of peer device. It manages unpairing even while
 *                  connected.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
tBTA_STATUS BTA_DmRemoveDevice(const RawAddress& bd_addr) {
  tBTA_DM_API_REMOVE_DEVICE* p_msg =
      (tBTA_DM_API_REMOVE_DEVICE*)osi_calloc(sizeof(tBTA_DM_API_REMOVE_DEVICE));

  p_msg->hdr.event = BTA_DM_API_REMOVE_DEVICE_EVT;
  p_msg->bd_addr = bd_addr;

  bta_sys_sendmsg(p_msg);

  return BTA_SUCCESS;
}

/*******************************************************************************
 *
 * Function         BTA_GetEirService
 *
 * Description      This function is called to get BTA service mask from EIR.
 *
 * Parameters       p_eir - pointer of EIR significant part
 *                  p_services - return the BTA service mask
 *
 * Returns          None
 *
 ******************************************************************************/
extern const uint16_t bta_service_id_to_uuid_lkup_tbl[];
void BTA_GetEirService(uint8_t* p_eir, size_t eir_len,
                       tBTA_SERVICE_MASK* p_services) {
  uint8_t xx, yy;
  uint8_t num_uuid, max_num_uuid = 32;
  uint8_t uuid_list[32 * Uuid::kNumBytes16];
  uint16_t* p_uuid16 = (uint16_t*)uuid_list;
  tBTA_SERVICE_MASK mask;

  BTM_GetEirUuidList(p_eir, eir_len, Uuid::kNumBytes16, &num_uuid, uuid_list,
                     max_num_uuid);
  for (xx = 0; xx < num_uuid; xx++) {
    mask = 1;
    for (yy = 0; yy < BTA_MAX_SERVICE_ID; yy++) {
      if (*(p_uuid16 + xx) == bta_service_id_to_uuid_lkup_tbl[yy]) {
        *p_services |= mask;
        break;
      }
      mask <<= 1;
    }

    /* for HSP v1.2 only device */
    if (*(p_uuid16 + xx) == UUID_SERVCLASS_HEADSET_HS)
      *p_services |= BTA_HSP_SERVICE_MASK;

    if (*(p_uuid16 + xx) == UUID_SERVCLASS_HDP_SOURCE)
      *p_services |= BTA_HL_SERVICE_MASK;

    if (*(p_uuid16 + xx) == UUID_SERVCLASS_HDP_SINK)
      *p_services |= BTA_HL_SERVICE_MASK;
  }
}

/*******************************************************************************
 *
 * Function         BTA_DmGetConnectionState
 *
 * Description      Returns whether the remote device is currently connected.
 *
 * Returns          0 if the device is NOT connected.
 *
 ******************************************************************************/
uint16_t BTA_DmGetConnectionState(const RawAddress& bd_addr) {
  tBTA_DM_PEER_DEVICE* p_dev = bta_dm_find_peer_device(bd_addr);
  return (p_dev && p_dev->conn_state == BTA_DM_CONNECTED);
}

/*******************************************************************************
 *                   Device Identification (DI) Server Functions
 ******************************************************************************/
/*******************************************************************************
 *
 * Function         BTA_DmSetLocalDiRecord
 *
 * Description      This function adds a DI record to the local SDP database.
 *
 * Returns          BTA_SUCCESS if record set sucessfully, otherwise error code.
 *
 ******************************************************************************/
tBTA_STATUS BTA_DmSetLocalDiRecord(tBTA_DI_RECORD* p_device_info,
                                   uint32_t* p_handle) {
  tBTA_STATUS status = BTA_FAILURE;

  if (bta_dm_di_cb.di_num < BTA_DI_NUM_MAX) {
    if (SDP_SetLocalDiRecord((tSDP_DI_RECORD*)p_device_info, p_handle) ==
        SDP_SUCCESS) {
      if (!p_device_info->primary_record) {
        bta_dm_di_cb.di_handle[bta_dm_di_cb.di_num] = *p_handle;
        bta_dm_di_cb.di_num++;
      }

      bta_sys_add_uuid(UUID_SERVCLASS_PNP_INFORMATION);
      status = BTA_SUCCESS;
    }
  }

  return status;
}

/*******************************************************************************
 *
 * Function         bta_dmexecutecallback
 *
 * Description      This function will request BTA to execute a call back in the
 *                  context of BTU task.
 *                  This API was named in lower case because it is only intended
 *                  for the internal customers(like BTIF).
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dmexecutecallback(tBTA_DM_EXEC_CBACK* p_callback, void* p_param) {
  tBTA_DM_API_EXECUTE_CBACK* p_msg =
      (tBTA_DM_API_EXECUTE_CBACK*)osi_malloc(sizeof(tBTA_DM_MSG));

  p_msg->hdr.event = BTA_DM_API_EXECUTE_CBACK_EVT;
  p_msg->p_param = p_param;
  p_msg->p_exec_cback = p_callback;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmAddBleKey
 *
 * Description      Add/modify LE device information.  This function will be
 *                  normally called during host startup to restore all required
 *                  information stored in the NVRAM.
 *
 * Parameters:      bd_addr          - BD address of the peer
 *                  p_le_key         - LE key values.
 *                  key_type         - LE SMP key type.
 *
 * Returns          BTA_SUCCESS if successful
 *                  BTA_FAIL if operation failed.
 *
 ******************************************************************************/
void BTA_DmAddBleKey(const RawAddress& bd_addr, tBTA_LE_KEY_VALUE* p_le_key,
                     tBTA_LE_KEY_TYPE key_type) {
  tBTA_DM_API_ADD_BLEKEY* p_msg =
      (tBTA_DM_API_ADD_BLEKEY*)osi_calloc(sizeof(tBTA_DM_API_ADD_BLEKEY));

  p_msg->hdr.event = BTA_DM_API_ADD_BLEKEY_EVT;
  p_msg->key_type = key_type;
  p_msg->bd_addr = bd_addr;
  memcpy(&p_msg->blekey, p_le_key, sizeof(tBTA_LE_KEY_VALUE));

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmAddBleDevice
 *
 * Description      Add a BLE device.  This function will be normally called
 *                  during host startup to restore all required information
 *                  for a LE device stored in the NVRAM.
 *
 * Parameters:      bd_addr          - BD address of the peer
 *                  dev_type         - Remote device's device type.
 *                  addr_type        - LE device address type.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmAddBleDevice(const RawAddress& bd_addr, tBLE_ADDR_TYPE addr_type,
                        tBT_DEVICE_TYPE dev_type) {
  tBTA_DM_API_ADD_BLE_DEVICE* p_msg = (tBTA_DM_API_ADD_BLE_DEVICE*)osi_calloc(
      sizeof(tBTA_DM_API_ADD_BLE_DEVICE));

  p_msg->hdr.event = BTA_DM_API_ADD_BLEDEVICE_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->addr_type = addr_type;
  p_msg->dev_type = dev_type;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBlePasskeyReply
 *
 * Description      Send BLE SMP passkey reply.
 *
 * Parameters:      bd_addr          - BD address of the peer
 *                  accept           - passkey entry sucessful or declined.
 *                  passkey          - passkey value, must be a 6 digit number,
 *                                     can be lead by 0.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBlePasskeyReply(const RawAddress& bd_addr, bool accept,
                           uint32_t passkey) {
  tBTA_DM_API_PASSKEY_REPLY* p_msg =
      (tBTA_DM_API_PASSKEY_REPLY*)osi_calloc(sizeof(tBTA_DM_API_PASSKEY_REPLY));

  p_msg->hdr.event = BTA_DM_API_BLE_PASSKEY_REPLY_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->accept = accept;

  if (accept) p_msg->passkey = passkey;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBleConfirmReply
 *
 * Description      Send BLE SMP SC user confirmation reply.
 *
 * Parameters:      bd_addr          - BD address of the peer
 *                  accept           - numbers to compare are the same or
 *                                     different.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBleConfirmReply(const RawAddress& bd_addr, bool accept) {
  tBTA_DM_API_CONFIRM* p_msg =
      (tBTA_DM_API_CONFIRM*)osi_calloc(sizeof(tBTA_DM_API_CONFIRM));

  p_msg->hdr.event = BTA_DM_API_BLE_CONFIRM_REPLY_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->accept = accept;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBleSecurityGrant
 *
 * Description      Grant security request access.
 *
 * Parameters:      bd_addr          - BD address of the peer
 *                  res              - security grant status.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBleSecurityGrant(const RawAddress& bd_addr,
                            tBTA_DM_BLE_SEC_GRANT res) {
  tBTA_DM_API_BLE_SEC_GRANT* p_msg =
      (tBTA_DM_API_BLE_SEC_GRANT*)osi_calloc(sizeof(tBTA_DM_API_BLE_SEC_GRANT));

  p_msg->hdr.event = BTA_DM_API_BLE_SEC_GRANT_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->res = res;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmSetBlePrefConnParams
 *
 * Description      This function is called to set the preferred connection
 *                  parameters when default connection parameter is not desired.
 *
 * Parameters:      bd_addr          - BD address of the peripheral
 *                  scan_interval    - scan interval
 *                  scan_window      - scan window
 *                  min_conn_int     - minimum preferred connection interval
 *                  max_conn_int     - maximum preferred connection interval
 *                  slave_latency    - preferred slave latency
 *                  supervision_tout - preferred supervision timeout
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmSetBlePrefConnParams(const RawAddress& bd_addr,
                                uint16_t min_conn_int, uint16_t max_conn_int,
                                uint16_t slave_latency,
                                uint16_t supervision_tout) {
  tBTA_DM_API_BLE_CONN_PARAMS* p_msg = (tBTA_DM_API_BLE_CONN_PARAMS*)osi_calloc(
      sizeof(tBTA_DM_API_BLE_CONN_PARAMS));

  p_msg->hdr.event = BTA_DM_API_BLE_CONN_PARAM_EVT;
  p_msg->peer_bda = bd_addr;
  p_msg->conn_int_max = max_conn_int;
  p_msg->conn_int_min = min_conn_int;
  p_msg->slave_latency = slave_latency;
  p_msg->supervision_tout = supervision_tout;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmSetBleConnScanParams
 *
 * Description      This function is called to set scan parameters used in
 *                  BLE connection request
 *
 * Parameters:      scan_interval    - scan interval
 *                  scan_window      - scan window
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmSetBleConnScanParams(uint32_t scan_interval, uint32_t scan_window) {
  tBTA_DM_API_BLE_SCAN_PARAMS* p_msg = (tBTA_DM_API_BLE_SCAN_PARAMS*)osi_calloc(
      sizeof(tBTA_DM_API_BLE_SCAN_PARAMS));

  p_msg->hdr.event = BTA_DM_API_BLE_CONN_SCAN_PARAM_EVT;
  p_msg->scan_int = scan_interval;
  p_msg->scan_window = scan_window;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         bta_dm_discover_send_msg
 *
 * Description      This function send discover message to BTA task.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_discover_send_msg(const RawAddress& bd_addr,
                                     tBTA_SERVICE_MASK_EXT* p_services,
                                     tBTA_DM_SEARCH_CBACK* p_cback,
                                     bool sdp_search,
                                     tBTA_TRANSPORT transport) {
  const size_t len =
      p_services
          ? (sizeof(tBTA_DM_API_DISCOVER) + sizeof(Uuid) * p_services->num_uuid)
          : sizeof(tBTA_DM_API_DISCOVER);
  tBTA_DM_API_DISCOVER* p_msg = (tBTA_DM_API_DISCOVER*)osi_calloc(len);

  p_msg->hdr.event = BTA_DM_API_DISCOVER_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->p_cback = p_cback;
  p_msg->sdp_search = sdp_search;
  p_msg->transport = transport;

  if (p_services != NULL) {
    p_msg->services = p_services->srvc_mask;
    p_msg->num_uuid = p_services->num_uuid;
    if (p_services->num_uuid != 0) {
      p_msg->p_uuid = (Uuid*)(p_msg + 1);
      memcpy(p_msg->p_uuid, p_services->p_uuid,
             sizeof(Uuid) * p_services->num_uuid);
    }
  }

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmDiscoverByTransport
 *
 * Description      This function does service discovery on particular transport
 *                  for services of a
 *                  peer device. When services.num_uuid is 0, it indicates all
 *                  GATT based services are to be searched; otherwise a list of
 *                  UUID of interested services should be provided through
 *                  p_services->p_uuid.
 *
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmDiscoverByTransport(const RawAddress& bd_addr,
                               tBTA_SERVICE_MASK_EXT* p_services,
                               tBTA_DM_SEARCH_CBACK* p_cback, bool sdp_search,
                               tBTA_TRANSPORT transport) {
  bta_dm_discover_send_msg(bd_addr, p_services, p_cback, sdp_search, transport);
}

/*******************************************************************************
 *
 * Function         BTA_DmDiscoverExt
 *
 * Description      This function does service discovery for services of a
 *                  peer device. When services.num_uuid is 0, it indicates all
 *                  GATT based services are to be searched; other wise a list of
 *                  UUID of interested services should be provided through
 *                  p_services->p_uuid.
 *
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmDiscoverExt(const RawAddress& bd_addr,
                       tBTA_SERVICE_MASK_EXT* p_services,
                       tBTA_DM_SEARCH_CBACK* p_cback, bool sdp_search) {
  bta_dm_discover_send_msg(bd_addr, p_services, p_cback, sdp_search,
                           BTA_TRANSPORT_UNKNOWN);
}

/*******************************************************************************
 *
 * Function         BTA_DmSearchExt
 *
 * Description      This function searches for peer Bluetooth devices. It
 *                  performs an inquiry and gets the remote name for devices.
 *                  Service discovery is done if services is non zero
 *
 * Parameters       p_dm_inq: inquiry conditions
 *                  p_services: if service is not empty, service discovery will
 *                              be done. For all GATT based service conditions,
 *                              put num_uuid, and p_uuid is the pointer to the
 *                              list of UUID values.
 *                  p_cback: callback function when search is completed.
 *
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmSearchExt(tBTA_DM_INQ* p_dm_inq, tBTA_SERVICE_MASK_EXT* p_services,
                     tBTA_DM_SEARCH_CBACK* p_cback) {
  const size_t len =
      p_services
          ? (sizeof(tBTA_DM_API_SEARCH) + sizeof(Uuid) * p_services->num_uuid)
          : sizeof(tBTA_DM_API_SEARCH);
  tBTA_DM_API_SEARCH* p_msg = (tBTA_DM_API_SEARCH*)osi_calloc(len);

  p_msg->hdr.event = BTA_DM_API_SEARCH_EVT;
  memcpy(&p_msg->inq_params, p_dm_inq, sizeof(tBTA_DM_INQ));
  p_msg->p_cback = p_cback;
  p_msg->rs_res = BTA_DM_RS_NONE;

  if (p_services != NULL) {
    p_msg->services = p_services->srvc_mask;
    p_msg->num_uuid = p_services->num_uuid;

    if (p_services->num_uuid != 0) {
      p_msg->p_uuid = (Uuid*)(p_msg + 1);
      memcpy(p_msg->p_uuid, p_services->p_uuid,
             sizeof(Uuid) * p_services->num_uuid);
    } else {
      p_msg->p_uuid = NULL;
    }
  }

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBleUpdateConnectionParam
 *
 * Description      Update connection parameters, can only be used when
 *                  connection is up.
 *
 * Parameters:      bd_addr          - BD address of the peer
 *                  min_int   -     minimum connection interval,
 *                                  [0x0004 ~ 0x4000]
 *                  max_int   -     maximum connection interval,
 *                                  [0x0004 ~ 0x4000]
 *                  latency   -     slave latency [0 ~ 500]
 *                  timeout   -     supervision timeout [0x000a ~ 0xc80]
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBleUpdateConnectionParams(const RawAddress& bd_addr,
                                     uint16_t min_int, uint16_t max_int,
                                     uint16_t latency, uint16_t timeout,
                                     uint16_t min_ce_len, uint16_t max_ce_len) {
  tBTA_DM_API_UPDATE_CONN_PARAM* p_msg =
      (tBTA_DM_API_UPDATE_CONN_PARAM*)osi_calloc(
          sizeof(tBTA_DM_API_UPDATE_CONN_PARAM));

  p_msg->hdr.event = BTA_DM_API_UPDATE_CONN_PARAM_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->min_int = min_int;
  p_msg->max_int = max_int;
  p_msg->latency = latency;
  p_msg->timeout = timeout;
  p_msg->min_ce_len = min_ce_len;
  p_msg->max_ce_len = max_ce_len;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBleConfigLocalPrivacy
 *
 * Description      Enable/disable privacy on the local device
 *
 * Parameters:      privacy_enable   - enable/disabe privacy on remote device.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBleConfigLocalPrivacy(bool privacy_enable) {
#if (BLE_PRIVACY_SPT == TRUE)
  tBTA_DM_API_LOCAL_PRIVACY* p_msg = (tBTA_DM_API_LOCAL_PRIVACY*)osi_calloc(
      sizeof(tBTA_DM_API_ENABLE_PRIVACY));

  p_msg->hdr.event = BTA_DM_API_LOCAL_PRIVACY_EVT;
  p_msg->privacy_enable = privacy_enable;

  bta_sys_sendmsg(p_msg);
#else
  UNUSED(privacy_enable);
#endif
}

/*******************************************************************************
 *
 * Function         BTA_DmBleGetEnergyInfo
 *
 * Description      This function is called to obtain the energy info
 *
 * Parameters       p_cmpl_cback - Command complete callback
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBleGetEnergyInfo(tBTA_BLE_ENERGY_INFO_CBACK* p_cmpl_cback) {
  const size_t len = sizeof(tBTA_DM_API_ENERGY_INFO) + sizeof(tBLE_BD_ADDR);
  tBTA_DM_API_ENERGY_INFO* p_msg = (tBTA_DM_API_ENERGY_INFO*)osi_calloc(len);

  APPL_TRACE_API("%s", __func__);

  p_msg->hdr.event = BTA_DM_API_BLE_ENERGY_INFO_EVT;
  p_msg->p_energy_info_cback = p_cmpl_cback;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBleUpdateConnectionParams
 *
 * Description      Update connection parameters, can only be used when
 *                  connection is up.
 *
 * Parameters:      bd_addr   - BD address of the peer
 *                  min_int   -     minimum connection interval,
 *                                  [0x0004 ~ 0x4000]
 *                  max_int   -     maximum connection interval,
 *                                  [0x0004 ~ 0x4000]
 *                  latency   -     slave latency [0 ~ 500]
 *                  timeout   -     supervision timeout [0x000a ~ 0xc80]
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmBleUpdateConnectionParams(const RawAddress& bd_addr,
                                     uint16_t min_int, uint16_t max_int,
                                     uint16_t latency, uint16_t timeout) {
  tBTA_DM_API_UPDATE_CONN_PARAM* p_msg =
      (tBTA_DM_API_UPDATE_CONN_PARAM*)osi_calloc(
          sizeof(tBTA_DM_API_UPDATE_CONN_PARAM));

  p_msg->hdr.event = BTA_DM_API_UPDATE_CONN_PARAM_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->min_int = min_int;
  p_msg->max_int = max_int;
  p_msg->latency = latency;
  p_msg->timeout = timeout;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBleSetDataLength
 *
 * Description      This function is to set maximum LE data packet size
 *
 * Returns          void
 *
 *
 ******************************************************************************/
void BTA_DmBleSetDataLength(const RawAddress& remote_device,
                            uint16_t tx_data_length) {
  tBTA_DM_API_BLE_SET_DATA_LENGTH* p_msg =
      (tBTA_DM_API_BLE_SET_DATA_LENGTH*)osi_malloc(
          sizeof(tBTA_DM_API_BLE_SET_DATA_LENGTH));

  p_msg->remote_bda = remote_device;
  p_msg->hdr.event = BTA_DM_API_SET_DATA_LENGTH_EVT;
  p_msg->tx_data_length = tx_data_length;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmSetEncryption
 *
 * Description      This function is called to ensure that connection is
 *                  encrypted.  Should be called only on an open connection.
 *                  Typically only needed for connections that first want to
 *                  bring up unencrypted links, then later encrypt them.
 *
 * Parameters:      bd_addr       - Address of the peer device
 *                  transport     - transport of the link to be encruypted
 *                  p_callback    - Pointer to callback function to indicat the
 *                                  link encryption status
 *                  sec_act       - This is the security action to indicate
 *                                  what kind of BLE security level is required
 *                                  for the BLE link if BLE is supported.
 *                                  Note: This parameter is ignored for the
 *                                        BR/EDR or if BLE is not supported.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmSetEncryption(const RawAddress& bd_addr, tBTA_TRANSPORT transport,
                         tBTA_DM_ENCRYPT_CBACK* p_callback,
                         tBTA_DM_BLE_SEC_ACT sec_act) {
  tBTA_DM_API_SET_ENCRYPTION* p_msg = (tBTA_DM_API_SET_ENCRYPTION*)osi_calloc(
      sizeof(tBTA_DM_API_SET_ENCRYPTION));

  APPL_TRACE_API("%s", __func__);

  p_msg->hdr.event = BTA_DM_API_SET_ENCRYPTION_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->transport = transport;
  p_msg->p_callback = p_callback;
  p_msg->sec_act = sec_act;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmCloseACL
 *
 * Description      This function force to close an ACL connection and remove
 *                  the device from the security database list of known devices.
 *
 * Parameters:      bd_addr       - Address of the peer device
 *                  remove_dev    - remove device or not after link down
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmCloseACL(const RawAddress& bd_addr, bool remove_dev,
                    tBTA_TRANSPORT transport) {
  tBTA_DM_API_REMOVE_ACL* p_msg =
      (tBTA_DM_API_REMOVE_ACL*)osi_calloc(sizeof(tBTA_DM_API_REMOVE_ACL));

  APPL_TRACE_API("%s", __func__);

  p_msg->hdr.event = BTA_DM_API_REMOVE_ACL_EVT;
  p_msg->bd_addr = bd_addr;
  p_msg->remove_dev = remove_dev;
  p_msg->transport = transport;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_DmBleObserve
 *
 * Description      This procedure keep the device listening for advertising
 *                  events from a broadcast device.
 *
 * Parameters       start: start or stop observe.
 *
 * Returns          void

 *
 * Returns          void.
 *
 ******************************************************************************/
extern void BTA_DmBleObserve(bool start, uint8_t duration,
                             tBTA_DM_SEARCH_CBACK* p_results_cb) {
  tBTA_DM_API_BLE_OBSERVE* p_msg =
      (tBTA_DM_API_BLE_OBSERVE*)osi_calloc(sizeof(tBTA_DM_API_BLE_OBSERVE));

  APPL_TRACE_API("%s:start = %d ", __func__, start);

  p_msg->hdr.event = BTA_DM_API_BLE_OBSERVE_EVT;
  p_msg->start = start;
  p_msg->duration = duration;
  p_msg->p_cback = p_results_cb;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         BTA_VendorInit
 *
 * Description      This function initializes vendor specific
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_VendorInit(void) { APPL_TRACE_API("BTA_VendorInit"); }

/*******************************************************************************
 *
 * Function         BTA_VendorCleanup
 *
 * Description      This function frees up Broadcom specific VS specific dynamic
 *                  memory
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_VendorCleanup(void) {
  tBTM_BLE_VSC_CB cmn_ble_vsc_cb;
  BTM_BleGetVendorCapabilities(&cmn_ble_vsc_cb);

  if (cmn_ble_vsc_cb.max_filter > 0) {
    btm_ble_adv_filter_cleanup();
#if (BLE_PRIVACY_SPT == TRUE)
    btm_ble_resolving_list_cleanup();
#endif
  }

  if (cmn_ble_vsc_cb.tot_scan_results_strg > 0) btm_ble_batchscan_cleanup();

  if (cmn_ble_vsc_cb.adv_inst_max > 0) btm_ble_multi_adv_cleanup();
}

/*******************************************************************************
 *
 * Function         BTA_DmProcessQueuedServiceDiscovery
 *
 * Description      This function processes queued service discovery
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_DmProcessQueuedServiceDiscovery(void) {
  uint8_t transport = BT_DEVICE_TYPE_BREDR;

  APPL_TRACE_API("BTA_DmProcessQueuedServiceDiscovery");

  while (!bta_dm_search_cb.p_disc_queue.empty()) {
    APPL_TRACE_API("Processing queued service discovery");

    tBTA_DM_MSG* p_data =(tBTA_DM_MSG*) bta_dm_search_cb.p_disc_queue.front();
    bta_dm_search_cb.p_disc_queue.pop();
    if (p_data != NULL) {
      RawAddress bda = p_data->discover.bd_addr;

      if (p_data->discover.transport != BT_TRANSPORT_INVALID) {
        transport = p_data->discover.transport;
      }
      if (BTM_IsAclConnectionUp(bda, transport)) {
        bta_sys_sendmsg(p_data);
        break;
      }
      else {
        osi_free(p_data);
      }
    }
  }
}
