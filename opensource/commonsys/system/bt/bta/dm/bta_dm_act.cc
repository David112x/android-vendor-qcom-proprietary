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
 *  This file contains the action functions for device manager state
 *  machine.
 *
 ******************************************************************************/

#define LOG_TAG "bt_bta_dm"

#include <base/bind.h>
#include <base/callback.h>
#include <base/logging.h>
#include <string.h>

#include "bt_common.h"
#include "bt_target.h"
#include "bt_types.h"
#include "bta_api.h"
#include "bta_dm_api.h"
#include "bta_dm_co.h"
#include "bta_dm_int.h"
#include "bta_sys.h"
#include "btif/include/btif_storage.h"
#include "btm_api.h"
#include "btm_int.h"
#include "btu.h"
#include "gap_api.h" /* For GAP_BleReadPeerPrefConnParams */
#include "l2c_api.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"
#include "sdp_api.h"
#include "bta_sdp_api.h"
#include "stack/gatt/connection_manager.h"
#include "stack/include/gatt_api.h"
#include "utl.h"
#include "device/include/interop_config.h"
#include "device/include/profile_config.h"
#include "device/include/interop.h"
#include "stack/sdp/sdpint.h"
#include <inttypes.h>
#include "btif/include/btif_config.h"
#include "device/include/device_iot_config.h"
#include <btcommon_interface_defs.h>
#include <controller.h>

#if (GAP_INCLUDED == TRUE)
#include "gap_api.h"
#endif

using bluetooth::Uuid;

static void bta_dm_inq_results_cb(tBTM_INQ_RESULTS* p_inq, uint8_t* p_eir,
                                  uint16_t eir_len);
static void bta_dm_inq_cmpl_cb(void* p_result);

static void bta_dm_service_search_remname_cback(const RawAddress& bd_addr,
                                                DEV_CLASS dc, BD_NAME bd_name);

static void bta_dm_rem_name_cback(const RawAddress& bd_addr,
                                        DEV_CLASS dc, BD_NAME bd_name);
static void bta_dm_remname_cback(void* p);
static void bta_dm_find_services(const RawAddress& bd_addr);
static void bta_dm_discover_next_device(void);
static void bta_dm_sdp_callback(uint16_t sdp_status);
static uint8_t bta_dm_authorize_cback(const RawAddress& bd_addr,
                                      DEV_CLASS dev_class, BD_NAME bd_name,
                                      uint8_t* service_name, uint8_t service_id,
                                      bool is_originator);
static uint8_t bta_dm_pin_cback(const RawAddress& bd_addr, DEV_CLASS dev_class,
                                BD_NAME bd_name, bool min_16_digit);
static uint8_t bta_dm_new_link_key_cback(const RawAddress& bd_addr,
                                         DEV_CLASS dev_class, BD_NAME bd_name,
                                         const LinkKey& key, uint8_t key_type);
static uint8_t bta_dm_authentication_complete_cback(const RawAddress& bd_addr,
                                                    DEV_CLASS dev_class,
                                                    BD_NAME bd_name,
                                                    int result);
static void bta_dm_local_name_cback(void * p_name);
static bool bta_dm_check_av(uint16_t event);
static void bta_dm_bl_change_cback(tBTM_BL_EVENT_DATA* p_data);

static void bta_dm_policy_cback(tBTA_SYS_CONN_STATUS status, uint8_t id,
                                uint8_t app_id, const RawAddress& peer_addr);

/* Extended Inquiry Response */
#if (BTM_LOCAL_IO_CAPS != BTM_IO_CAP_NONE)
static uint8_t bta_dm_sp_cback(tBTM_SP_EVT event, tBTM_SP_EVT_DATA* p_data);
#endif

static void bta_dm_set_eir(char* local_name);

static void bta_dm_eir_search_services(tBTM_INQ_RESULTS* p_result,
                                       tBTA_SERVICE_MASK* p_services_to_search,
                                       tBTA_SERVICE_MASK* p_services_found);

static void bta_dm_search_timer_cback(void* data);
static void bta_dm_disable_conn_down_timer_cback(void* data);
static void bta_dm_bond_retrail_cback(void* data);
static void bta_dm_rm_cback(tBTA_SYS_CONN_STATUS status, uint8_t id,
                            uint8_t app_id, const RawAddress& peer_addr);
static void bta_dm_adjust_roles(bool delay_role_switch);
static void bta_dm_set_tech_based_max_power(bool status);
static char* bta_dm_get_remname(void);
static void bta_dm_bond_cancel_complete_cback(tBTM_STATUS result);

static bool bta_dm_read_remote_device_name(const RawAddress& bd_addr,
                                           tBT_TRANSPORT transport);
static void bta_dm_discover_device(const RawAddress& remote_bd_addr);

static void bta_dm_sys_hw_cback(tBTA_SYS_HW_EVT status);
static void bta_dm_disable_search_and_disc(void);

static uint8_t bta_dm_ble_smp_cback(tBTM_LE_EVT event, const RawAddress& bda,
                                    tBTM_LE_EVT_DATA* p_data);
static void bta_dm_ble_id_key_cback(uint8_t key_type,
                                    tBTM_BLE_LOCAL_KEYS* p_key);
static void bta_dm_gattc_register(void);
static void btm_dm_start_gatt_discovery(const RawAddress& bd_addr);
static void bta_dm_cancel_gatt_discovery(const RawAddress& bd_addr);
static void bta_dm_gattc_callback(tBTA_GATTC_EVT event, tBTA_GATTC* p_data);
extern tBTA_DM_CONTRL_STATE bta_dm_pm_obtain_controller_state(void);

#if (BLE_VND_INCLUDED == TRUE)
static void bta_dm_ctrl_features_rd_cmpl_cback(tBTM_STATUS result);
#endif

#ifndef BTA_DM_BLE_ADV_CHNL_MAP
#define BTA_DM_BLE_ADV_CHNL_MAP \
  (BTM_BLE_ADV_CHNL_37 | BTM_BLE_ADV_CHNL_38 | BTM_BLE_ADV_CHNL_39)
#endif

/* Bluetooth Technologies */
#define HCI_VS_SET_MAX_RADIATED_POWER_SUB_OPCODE 0x0E
#define BR_TECH_VALUE 0x00
#define EDR_TECH_VALUE 0x01
#define BLE_TECH_VALUE 0x02

/* Disable timer interval (in milliseconds) */
#ifndef BTA_DM_DISABLE_TIMER_MS
#define BTA_DM_DISABLE_TIMER_MS 5000
#endif

/* Disable timer retrial interval (in milliseconds) */
#ifndef BTA_DM_DISABLE_TIMER_RETRIAL_MS
#define BTA_DM_DISABLE_TIMER_RETRIAL_MS 1500
#endif

/* bond retrial interval (in milliseconds) */
#ifndef BTA_DM_BOND_TIMER_RETRIAL_MS
#define BTA_DM_BOND_TIMER_RETRIAL_MS 100
#endif

/* Disable connection down timer (in milliseconds) */
#ifndef BTA_DM_DISABLE_CONN_DOWN_TIMER_MS
#define BTA_DM_DISABLE_CONN_DOWN_TIMER_MS 1000
#endif

/* Switch delay timer (in milliseconds) */
#ifndef BTA_DM_SWITCH_DELAY_TIMER_MS
#define BTA_DM_SWITCH_DELAY_TIMER_MS 500
#endif

static void bta_dm_reset_sec_dev_pending(const RawAddress& remote_bd_addr);
static void bta_dm_remove_sec_dev_entry(const RawAddress& remote_bd_addr);
static void bta_dm_observe_results_cb(tBTM_INQ_RESULTS* p_inq, uint8_t* p_eir,
                                      uint16_t eir_len);
static void bta_dm_observe_cmpl_cb(void* p_result);
static void bta_dm_delay_role_switch_cback(void* data);
static void bta_dm_disable_timer_cback(void* data);
static void bta_dm_vnd_info_report_cback(uint8_t evt_len, uint8_t *p_data);

const uint16_t bta_service_id_to_uuid_lkup_tbl[BTA_MAX_SERVICE_ID] = {
    UUID_SERVCLASS_PNP_INFORMATION,       /* Reserved */
    UUID_SERVCLASS_SERIAL_PORT,           /* BTA_SPP_SERVICE_ID */
    UUID_SERVCLASS_DIALUP_NETWORKING,     /* BTA_DUN_SERVICE_ID */
    UUID_SERVCLASS_AUDIO_SOURCE,          /* BTA_A2DP_SOURCE_SERVICE_ID */
    UUID_SERVCLASS_LAN_ACCESS_USING_PPP,  /* BTA_LAP_SERVICE_ID */
    UUID_SERVCLASS_HEADSET,               /* BTA_HSP_HS_SERVICE_ID */
    UUID_SERVCLASS_HF_HANDSFREE,          /* BTA_HFP_HS_SERVICE_ID */
    UUID_SERVCLASS_OBEX_OBJECT_PUSH,      /* BTA_OPP_SERVICE_ID */
    UUID_SERVCLASS_OBEX_FILE_TRANSFER,    /* BTA_FTP_SERVICE_ID */
    UUID_SERVCLASS_CORDLESS_TELEPHONY,    /* BTA_CTP_SERVICE_ID */
    UUID_SERVCLASS_INTERCOM,              /* BTA_ICP_SERVICE_ID */
    UUID_SERVCLASS_IRMC_SYNC,             /* BTA_SYNC_SERVICE_ID */
    UUID_SERVCLASS_DIRECT_PRINTING,       /* BTA_BPP_SERVICE_ID */
    UUID_SERVCLASS_IMAGING_RESPONDER,     /* BTA_BIP_SERVICE_ID */
    UUID_SERVCLASS_PANU,                  /* BTA_PANU_SERVICE_ID */
    UUID_SERVCLASS_NAP,                   /* BTA_NAP_SERVICE_ID */
    UUID_SERVCLASS_GN,                    /* BTA_GN_SERVICE_ID */
    UUID_SERVCLASS_SAP,                   /* BTA_SAP_SERVICE_ID */
    UUID_SERVCLASS_AUDIO_SINK,            /* BTA_A2DP_SERVICE_ID */
    UUID_SERVCLASS_AV_REMOTE_CONTROL,     /* BTA_AVRCP_SERVICE_ID */
    UUID_SERVCLASS_HUMAN_INTERFACE,       /* BTA_HID_SERVICE_ID */
    UUID_SERVCLASS_VIDEO_SINK,            /* BTA_VDP_SERVICE_ID */
    UUID_SERVCLASS_PBAP_PSE,              /* BTA_PBAP_SERVICE_ID */
    UUID_SERVCLASS_HEADSET_AUDIO_GATEWAY, /* BTA_HSP_SERVICE_ID */
    UUID_SERVCLASS_AG_HANDSFREE,          /* BTA_HFP_SERVICE_ID */
    UUID_SERVCLASS_MESSAGE_ACCESS,        /* BTA_MAP_SERVICE_ID */
    UUID_SERVCLASS_MESSAGE_NOTIFICATION,  /* BTA_MN_SERVICE_ID */
    UUID_SERVCLASS_HDP_PROFILE,           /* BTA_HDP_SERVICE_ID */
    UUID_SERVCLASS_PBAP_PCE,              /* BTA_PCE_SERVICE_ID */
    UUID_PROTOCOL_ATT                     /* BTA_GATT_SERVICE_ID */
};

/*
 * NOTE : The number of element in bta_service_id_to_btm_srv_id_lkup_tbl should
 * be matching with the value BTA_MAX_SERVICE_ID in bta_api.h
 *
 * i.e., If you add new Service ID for BTA, the correct security ID of the new
 * service from Security service definitions (btm_api.h) should be added to
 * this lookup table.
 */
const uint32_t bta_service_id_to_btm_srv_id_lkup_tbl[BTA_MAX_SERVICE_ID] = {
    0,                             /* Reserved */
    BTM_SEC_SERVICE_SERIAL_PORT,   /* BTA_SPP_SERVICE_ID */
    BTM_SEC_SERVICE_DUN,           /* BTA_DUN_SERVICE_ID */
    BTM_SEC_SERVICE_AVDTP,         /* BTA_AUDIO_SOURCE_SERVICE_ID */
    BTM_SEC_SERVICE_LAN_ACCESS,    /* BTA_LAP_SERVICE_ID */
    BTM_SEC_SERVICE_HEADSET_AG,    /* BTA_HSP_SERVICE_ID */
    BTM_SEC_SERVICE_AG_HANDSFREE,  /* BTA_HFP_SERVICE_ID */
    BTM_SEC_SERVICE_OBEX,          /* BTA_OPP_SERVICE_ID */
    BTM_SEC_SERVICE_OBEX_FTP,      /* BTA_FTP_SERVICE_ID */
    BTM_SEC_SERVICE_CORDLESS,      /* BTA_CTP_SERVICE_ID */
    BTM_SEC_SERVICE_INTERCOM,      /* BTA_ICP_SERVICE_ID */
    BTM_SEC_SERVICE_IRMC_SYNC,     /* BTA_SYNC_SERVICE_ID */
    BTM_SEC_SERVICE_BPP_JOB,       /* BTA_BPP_SERVICE_ID */
    BTM_SEC_SERVICE_BIP,           /* BTA_BIP_SERVICE_ID */
    BTM_SEC_SERVICE_BNEP_PANU,     /* BTA_PANU_SERVICE_ID */
    BTM_SEC_SERVICE_BNEP_NAP,      /* BTA_NAP_SERVICE_ID */
    BTM_SEC_SERVICE_BNEP_GN,       /* BTA_GN_SERVICE_ID */
    BTM_SEC_SERVICE_SAP,           /* BTA_SAP_SERVICE_ID */
    BTM_SEC_SERVICE_AVDTP,         /* BTA_A2DP_SERVICE_ID */
    BTM_SEC_SERVICE_AVCTP,         /* BTA_AVRCP_SERVICE_ID */
    BTM_SEC_SERVICE_HIDH_SEC_CTRL, /* BTA_HID_SERVICE_ID */
    BTM_SEC_SERVICE_AVDTP,         /* BTA_VDP_SERVICE_ID */
    BTM_SEC_SERVICE_PBAP,          /* BTA_PBAP_SERVICE_ID */
    BTM_SEC_SERVICE_HEADSET,       /* BTA_HSP_HS_SERVICE_ID */
    BTM_SEC_SERVICE_HF_HANDSFREE,  /* BTA_HFP_HS_SERVICE_ID */
    BTM_SEC_SERVICE_MAP,           /* BTA_MAP_SERVICE_ID */
    BTM_SEC_SERVICE_MAP,           /* BTA_MN_SERVICE_ID */
    BTM_SEC_SERVICE_HDP_SNK,       /* BTA_HDP_SERVICE_ID */
    BTM_SEC_SERVICE_PBAP,          /* BTA_PCE_SERVICE_ID */
    BTM_SEC_SERVICE_ATT            /* BTA_GATT_SERVICE_ID */
};

/* bta security callback */
const tBTM_APPL_INFO bta_security = {&bta_dm_authorize_cback,
                                     &bta_dm_pin_cback,
                                     &bta_dm_new_link_key_cback,
                                     &bta_dm_authentication_complete_cback,
                                     &bta_dm_bond_cancel_complete_cback,
#if (BTM_LOCAL_IO_CAPS != BTM_IO_CAP_NONE)
                                     &bta_dm_sp_cback,
#else
                                     NULL,
#endif
                                     &bta_dm_ble_smp_cback,
                                     &bta_dm_ble_id_key_cback};

#define MAX_DISC_RAW_DATA_BUF (4096)
uint8_t g_disc_raw_data_buf[MAX_DISC_RAW_DATA_BUF];

extern DEV_CLASS local_device_default_class;

// Stores the local Input/Output Capabilities of the Bluetooth device.
static uint8_t btm_local_io_caps;

/** Initialises the BT device manager */
void bta_dm_enable(tBTA_DM_MSG* p_data) {
  tBTA_DM_ENABLE enable_event;
  /* if already in use, return an error */
  if (bta_dm_cb.is_bta_dm_active == true) {
    APPL_TRACE_WARNING("%s Device already started by another application",
                       __func__);
    memset(&enable_event, 0, sizeof(tBTA_DM_ENABLE));
    enable_event.status = BTA_FAILURE;
    if (p_data->enable.p_sec_cback != NULL)
      p_data->enable.p_sec_cback(BTA_DM_ENABLE_EVT,
                                 (tBTA_DM_SEC*)&enable_event);
    return;
  }

  /* first, register our callback to SYS HW manager */
  bta_sys_hw_register(BTA_SYS_HW_BLUETOOTH, bta_dm_sys_hw_cback);

  /* Register callback with btm layer for remote name */
  BTM_SecAddRmtNameNotifyCallback(&bta_dm_rem_name_cback);

  /* make sure security callback is saved - if no callback, do not erase the
  previous one,
  it could be an error recovery mechanism */
  if (p_data->enable.p_sec_cback != NULL)
    bta_dm_cb.p_sec_cback = p_data->enable.p_sec_cback;
  /* notify BTA DM is now active */
  bta_dm_cb.is_bta_dm_active = true;
  bta_dm_cb.bta_dm_hw_status = BTA_DM_HW_OFF;

  /* send a message to BTA SYS */
  tBTA_SYS_HW_MSG* sys_enable_event =
      (tBTA_SYS_HW_MSG*)osi_malloc(sizeof(tBTA_SYS_HW_MSG));
  sys_enable_event->hdr.event = BTA_SYS_API_ENABLE_EVT;
  sys_enable_event->hw_module = BTA_SYS_HW_BLUETOOTH;

  bta_sys_sendmsg(sys_enable_event);

  btm_local_io_caps = btif_storage_get_local_io_caps();
}

/*******************************************************************************
 *
 * Function         bta_dm_init_cb
 *
 * Description      Initializes the bta_dm_cb control block
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_init_cb(void) {
  memset(&bta_dm_cb, 0, sizeof(bta_dm_cb));
  bta_dm_cb.disable_timer = alarm_new("bta_dm.disable_timer");
  bta_dm_cb.bond_retrail_timer = alarm_new("bta_dm.bond_retrail_timer");
  bta_dm_cb.switch_delay_timer = alarm_new("bta_dm.switch_delay_timer");
  for (size_t i = 0; i < BTA_DM_NUM_PM_TIMER; i++) {
    for (size_t j = 0; j < BTA_DM_PM_MODE_TIMER_MAX; j++) {
      bta_dm_cb.pm_timer[i].timer[j] = alarm_new("bta_dm.pm_timer");
    }
  }
  btm_register_iot_info_cback(bta_dm_vnd_info_report_cback);
  btm_register_ssr_cback(bta_dm_process_ssr);
}

/*******************************************************************************
 *
 * Function         bta_dm_deinit_cb
 *
 * Description      De-initializes the bta_dm_cb control block
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_deinit_cb(void) {
  /*
   * TODO: Should alarm_free() the bta_dm_cb timers during graceful
   * shutdown.
   */
  alarm_free(bta_dm_cb.disable_timer);
  alarm_free(bta_dm_cb.bond_retrail_timer);
  alarm_free(bta_dm_cb.switch_delay_timer);
  for (size_t i = 0; i < BTA_DM_NUM_PM_TIMER; i++) {
    for (size_t j = 0; j < BTA_DM_PM_MODE_TIMER_MAX; j++) {
      alarm_free(bta_dm_cb.pm_timer[i].timer[j]);
    }
  }
  memset(&bta_dm_cb, 0, sizeof(bta_dm_cb));
}

/*******************************************************************************
 *
 * Function         bta_dm_sys_hw_cback
 *
 * Description     callback register to SYS to get HW status updates
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_sys_hw_cback(tBTA_SYS_HW_EVT status) {
  DEV_CLASS dev_class;
  tBTA_DM_SEC_CBACK* temp_cback;
  uint8_t key_mask = 0;
  tBTA_BLE_LOCAL_ID_KEYS id_key;
  tBTA_DM_MSG* p_data;

  APPL_TRACE_DEBUG("%s with event: %i", __func__, status);

  /* On H/W error evt, report to the registered DM application callback */
  if (status == BTA_SYS_HW_ERROR_EVT) {
    if (bta_dm_cb.p_sec_cback != NULL)
      bta_dm_cb.p_sec_cback(BTA_DM_HW_ERROR_EVT, NULL);
    return;
  }

  if (status == BTA_SYS_HW_OFF_EVT) {

    if (bta_dm_cb.bta_dm_hw_status != BTA_DM_HW_ON) {
      APPL_TRACE_ERROR("BLuetooth HW chip is already OFF, ignoring BTA_SYS_HW_OFF_EVT");
      return;
    }

    bta_dm_cb.bta_dm_hw_status = BTA_DM_HW_TURNING_OFF;
    if (bta_dm_cb.p_sec_cback != NULL)
      bta_dm_cb.p_sec_cback(BTA_DM_DISABLE_EVT, NULL);

    /* reinitialize the control block */
    bta_dm_deinit_cb();

    /* hw is ready, go on with BTA DM initialization */
    alarm_free(bta_dm_search_cb.search_timer);
    alarm_free(bta_dm_search_cb.gatt_close_timer);

    while (!bta_dm_search_cb.p_disc_queue.empty()) {
      p_data =(tBTA_DM_MSG*) bta_dm_search_cb.p_disc_queue.front();
      if(p_data)
        osi_free_and_reset((void**)&p_data);
      bta_dm_search_cb.p_disc_queue.pop();
    }

    memset(&bta_dm_search_cb, 0, sizeof(bta_dm_search_cb));

    /* unregister from SYS */
    bta_sys_hw_unregister(BTA_SYS_HW_BLUETOOTH);
    /* notify BTA DM is now unactive */
    bta_dm_cb.is_bta_dm_active = false;
    bta_dm_cb.bta_dm_hw_status = BTA_DM_HW_OFF;
  } else if (status == BTA_SYS_HW_ON_EVT) {
    /* FIXME: We should not unregister as the SYS shall invoke this callback on
     * a H/W error.
     * We need to revisit when this platform has more than one BLuetooth H/W
     * chip
     */
    // bta_sys_hw_unregister( BTA_SYS_HW_BLUETOOTH);

    if (bta_dm_cb.bta_dm_hw_status != BTA_DM_HW_OFF) {
        APPL_TRACE_ERROR("BLuetooth HW chip is already on, ignoring BTA_SYS_HW_ON_EVT");
        return;
    }
    bta_dm_cb.bta_dm_hw_status = BTA_DM_HW_TURNING_ON;
    /* save security callback */
    temp_cback = bta_dm_cb.p_sec_cback;
    /* make sure the control block is properly initialized */
    bta_dm_init_cb();
    /* and retrieve the callback */
    bta_dm_cb.p_sec_cback = temp_cback;
    bta_dm_cb.is_bta_dm_active = true;

    /* hw is ready, go on with BTA DM initialization */
    alarm_free(bta_dm_search_cb.search_timer);
    alarm_free(bta_dm_search_cb.gatt_close_timer);
    while (!bta_dm_search_cb.p_disc_queue.empty()) {
      p_data =(tBTA_DM_MSG*) bta_dm_search_cb.p_disc_queue.front();
      if(p_data)
        osi_free_and_reset((void**)&p_data);
      bta_dm_search_cb.p_disc_queue.pop();
    }
    memset(&bta_dm_search_cb, 0, sizeof(bta_dm_search_cb));
    /*
     * TODO: Should alarm_free() the bta_dm_search_cb timers during
     * graceful shutdown.
     */
    bta_dm_search_cb.search_timer = alarm_new("bta_dm_search.search_timer");
    bta_dm_search_cb.gatt_close_timer =
        alarm_new("bta_dm_search.gatt_close_timer");

    memset(&bta_dm_conn_srvcs, 0, sizeof(bta_dm_conn_srvcs));
    memset(&bta_dm_di_cb, 0, sizeof(tBTA_DM_DI_CB));

    memcpy(dev_class, p_bta_dm_cfg->dev_class, sizeof(dev_class));
    BTM_SetDeviceClass(dev_class);

    /* load BLE local information: ID keys, ER if available */
    Octet16 er;
    bta_dm_co_ble_load_local_keys(&key_mask, &er, &id_key);

    if (key_mask & BTA_BLE_LOCAL_KEY_TYPE_ER) {
      BTM_BleLoadLocalKeys(BTA_BLE_LOCAL_KEY_TYPE_ER,
                           (tBTM_BLE_LOCAL_KEYS*)&er);
    }
    if (key_mask & BTA_BLE_LOCAL_KEY_TYPE_ID) {
      BTM_BleLoadLocalKeys(BTA_BLE_LOCAL_KEY_TYPE_ID,
                           (tBTM_BLE_LOCAL_KEYS*)&id_key);
    }
    bta_dm_search_cb.conn_id = GATT_INVALID_CONN_ID;

    BTM_SecRegister((tBTM_APPL_INFO*)&bta_security);
    BTM_SetDefaultLinkSuperTout(p_bta_dm_cfg->link_timeout);
    BTM_WritePageTimeout(p_bta_dm_cfg->page_timeout);
    bta_dm_cb.cur_policy = p_bta_dm_cfg->policy_settings;
    BTM_SetDefaultLinkPolicy(bta_dm_cb.cur_policy);
    BTM_RegBusyLevelNotif(bta_dm_bl_change_cback, NULL,
                          BTM_BL_UPDATE_MASK | BTM_BL_ROLE_CHG_MASK);

#if (BLE_VND_INCLUDED == TRUE)
    BTM_BleReadControllerFeatures(bta_dm_ctrl_features_rd_cmpl_cback);
#else
    /* If VSC multi adv commands are available, advertising will be initialized
     * when capabilities are read. If they are not avaliable, initialize
     * advertising here */
    btm_ble_adv_init();
#endif

    /* Earlier, we used to invoke BTM_ReadLocalAddr which was just copying the
       bd_addr
       from the control block and invoking the callback which was sending the
       DM_ENABLE_EVT.
       But then we have a few HCI commands being invoked above which were still
       in progress
       when the ENABLE_EVT was sent. So modified this to fetch the local name
       which forces
       the DM_ENABLE_EVT to be sent only after all the init steps are complete
       */
    BTM_ReadLocalDeviceNameFromController(
        (tBTM_CMPL_CB*)bta_dm_local_name_cback);

    bta_sys_rm_register((tBTA_SYS_CONN_CBACK*)bta_dm_rm_cback);

    /* initialize bluetooth low power manager */
    bta_dm_init_pm();

    bta_sys_policy_register((tBTA_SYS_CONN_CBACK*)bta_dm_policy_cback);

    bta_dm_gattc_register();
    bta_dm_cb.bta_dm_hw_status = BTA_DM_HW_ON;
  } else
    APPL_TRACE_DEBUG(" --- ignored event");
}

/*******************************************************************************
 *
 * Function         bta_dm_disable
 *
 * Description      Disables the BT device manager
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_disable(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  bt_soc_type_t soc_type = controller_get_interface()->get_soc_type();

  /* Set l2cap idle timeout to 0 (so BTE immediately disconnects ACL link after
   * last channel is closed) */
  L2CA_SetIdleTimeoutByBdAddr(RawAddress::kAny, 0, BT_TRANSPORT_BR_EDR);
  L2CA_SetIdleTimeoutByBdAddr(RawAddress::kAny, 0, BT_TRANSPORT_LE);

  /* disable all active subsystems */
  bta_sys_disable(BTA_SYS_HW_BLUETOOTH);

  /* De-register callback with btm layer for remote name */
  BTM_SecDeleteRmtNameNotifyCallback(&bta_dm_rem_name_cback);

  BTM_SetDiscoverability(BTM_NON_DISCOVERABLE, 0, 0);
  BTM_SetConnectability(BTM_NON_CONNECTABLE, 0, 0);

  bta_dm_disable_pm();
  bta_dm_disable_search_and_disc();
  bta_dm_cb.disabling = true;

  connection_manager::reset(false);

  /* Disable soc iot info report */
  if ((soc_type == BT_SOC_TYPE_SMD || soc_type == BT_SOC_TYPE_CHEROKEE) &&
      is_iot_info_report_enabled()) {
    btm_enable_soc_iot_info_report(false);
  }

  /* Disable SOC Logging */
  if (soc_type == BT_SOC_TYPE_SMD) {
    uint8_t param[5] = {0x10,0x02,0x00,0x00,0x01};
    BTM_VendorSpecificCommand(HCI_VS_HOST_LOG_OPCODE,5,param,NULL);
  } else if (soc_type == BT_SOC_TYPE_CHEROKEE || soc_type == BT_SOC_TYPE_HASTINGS) {
    uint8_t param[2] = {0x14, 0x00};
    BTM_VendorSpecificCommand(HCI_VS_HOST_LOG_OPCODE, 2, param, NULL);
  }

  if (BTM_GetNumAclLinks() == 0) {
#if (BTA_DISABLE_DELAY > 0)
    /* If BTA_DISABLE_DELAY is defined and greater than zero, then delay the
     * shutdown by
     * BTA_DISABLE_DELAY milliseconds
     */
    APPL_TRACE_WARNING("%s BTA_DISABLE_DELAY set to %d ms", __func__,
                       BTA_DISABLE_DELAY);
    alarm_set_on_mloop(bta_dm_cb.disable_timer, BTA_DISABLE_DELAY,
                       bta_dm_disable_conn_down_timer_cback, NULL);
#else
    bta_dm_disable_conn_down_timer_cback(NULL);
#endif
  } else {
    alarm_set_on_mloop(bta_dm_cb.disable_timer, BTA_DM_DISABLE_TIMER_MS,
                       bta_dm_disable_timer_cback, UINT_TO_PTR(0));
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_disable_timer_cback
 *
 * Description      Called if the disable timer expires
 *                  Used to close ACL connections which are still active
 *
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_disable_timer_cback(void* data) {
  uint8_t i;
  tBT_TRANSPORT transport = BT_TRANSPORT_BR_EDR;
  bool trigger_disc = false;
  uint32_t param = PTR_TO_UINT(data);

  APPL_TRACE_WARNING("%s trial %u", __func__, param);

  if (param == 2) {
    if (BTM_GetNumAclLinks()) {
      for (i = 0; i < bta_dm_cb.device_list.count; i++) {
        transport = bta_dm_cb.device_list.peer_device[i].transport;
        if (BT_TRANSPORT_BR_EDR == transport) {
          btm_remove_acl(bta_dm_cb.device_list.peer_device[i].peer_bdaddr,
                  transport);
        }
      }
    }
  }else if (BTM_GetNumAclLinks() && (param == 0)) {
    for (i = 0; i < bta_dm_cb.device_list.count; i++) {
      transport = bta_dm_cb.device_list.peer_device[i].transport;
      btm_remove_acl(bta_dm_cb.device_list.peer_device[i].peer_bdaddr,
                     transport);
      trigger_disc = true;
    }

    /* Retrigger disable timer in case ACL disconnect failed, DISABLE_EVT still
       need
        to be sent out to avoid jave layer disable timeout */
    if (trigger_disc) {
      alarm_set_on_mloop(bta_dm_cb.disable_timer,
                         BTA_DM_DISABLE_TIMER_RETRIAL_MS,
                         bta_dm_disable_timer_cback, UINT_TO_PTR(1));
    }
  } else {
    bta_dm_cb.disabling = false;

    bta_sys_remove_uuid(UUID_SERVCLASS_PNP_INFORMATION);
    bta_dm_disable_conn_down_timer_cback(NULL);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_set_dev_name
 *
 * Description      Sets local device name
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_set_dev_name(tBTA_DM_MSG* p_data) {
  BTM_SetLocalDeviceName((char*)p_data->set_name.name);
  bta_dm_set_eir((char*)p_data->set_name.name);
}

/*******************************************************************************
 *
 * Function         bta_dm_set_wifi_state
 *
 * Description      Sets wifi state
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_set_wifi_state(tBTA_DM_MSG *p_data) {
  if (BTM_GetWifiState() == p_data->wifi_state.status)
    return;

  BTM_SetWifiState((bool)p_data->wifi_state.status);
  if (p_data->wifi_state.status == true)
    bta_dm_adjust_roles(FALSE);

}

/*******************************************************************************
 *
 * Function         bta_dm_power_back_off
 *
 * Description      Sets/Resets back_off power
 *
 *
 * Returns          void
 *
 ******************************************************************************/

 void bta_dm_power_back_off(tBTA_DM_MSG *p_data) {

  bt_soc_type_t soc_type = controller_get_interface()->get_soc_type();
  if (soc_type < BT_SOC_TYPE_CHEROKEE) {
    APPL_TRACE_WARNING("%s: power_back_off feature not supported", __func__);
    return;
  }

  if (BTM_GetPowerBackOffState() == p_data->pwr_backoff_state.status)
    return;

  BTM_SetPowerBackOffState((bool)p_data->pwr_backoff_state.status);

  bta_dm_set_tech_based_max_power(p_data->pwr_backoff_state.status);

}

/*******************************************************************************
 *
 * Function         bta_dm_bredr_cleanup
 *
 * Description      do bredr cleanup
 *
 *
 * Returns          void
 *
 *****************************************************************************/
void bta_dm_bredr_cleanup(tBTA_DM_MSG *p_data) {
  alarm_set_on_mloop(bta_dm_cb.disable_timer, BTA_DM_DISABLE_TIMER_MS,
                    bta_dm_disable_timer_cback, UINT_TO_PTR(2));
}

/*******************************************************************************
 *
 * Function         bta_dm_bredr_startup
 *
 * Description      do bredr startup
 *
 *
 * Returns          void
 *
 *****************************************************************************/
void bta_dm_bredr_startup(tBTA_DM_MSG *p_data) {
  uint8_t i;
  tBT_TRANSPORT transport = BT_TRANSPORT_BR_EDR;
  if (alarm_is_scheduled(bta_dm_cb.disable_timer)) {
    alarm_cancel(bta_dm_cb.disable_timer);
    if (BTM_GetNumAclLinks()) {
      for (i = 0; i < bta_dm_cb.device_list.count; i++) {
        transport = bta_dm_cb.device_list.peer_device[i].transport;
        if (BT_TRANSPORT_BR_EDR == transport) {
          btm_remove_acl(bta_dm_cb.device_list.peer_device[i].peer_bdaddr,
                         transport);
        }
      }
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_reset_pairing_flag
 *
 * Description      Reset pairing flag
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_reset_pairing_flag(tBTA_DM_MSG *p_data) {
  tBTM_STATUS status;

  APPL_TRACE_EVENT(" bta_dm_reset_pairing_flag ");
  status = BTM_SecResetPairingFlag(p_data->pair_state.bd_addr);

}

/*******************************************************************************
 *
 * Function         bta_dm_set_visibility
 *
 * Description      Sets discoverability, connectability and pairability
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_set_visibility(tBTA_DM_MSG* p_data) {
  uint16_t window, interval;
  uint16_t le_disc_mode = BTM_BleReadDiscoverability();
  uint16_t le_conn_mode = BTM_BleReadConnectability();
  uint16_t disc_mode = BTM_ReadDiscoverability(&window, &interval);
  uint16_t conn_mode = BTM_ReadConnectability(&window, &interval);

  /* set modes for Discoverability and connectability if not ignore */
  if (p_data->set_visibility.disc_mode != (BTA_DM_IGNORE | BTA_DM_LE_IGNORE)) {
    if ((p_data->set_visibility.disc_mode & BTA_DM_LE_IGNORE) ==
        BTA_DM_LE_IGNORE)
      p_data->set_visibility.disc_mode =
          ((p_data->set_visibility.disc_mode & ~BTA_DM_LE_IGNORE) |
           le_disc_mode);
    if ((p_data->set_visibility.disc_mode & BTA_DM_IGNORE) == BTA_DM_IGNORE)
      p_data->set_visibility.disc_mode =
          ((p_data->set_visibility.disc_mode & ~BTA_DM_IGNORE) | disc_mode);

    BTM_SetDiscoverability(p_data->set_visibility.disc_mode,
                           bta_dm_cb.inquiry_scan_window,
                           bta_dm_cb.inquiry_scan_interval);
  }

  if (p_data->set_visibility.conn_mode != (BTA_DM_IGNORE | BTA_DM_LE_IGNORE)) {
    if ((p_data->set_visibility.conn_mode & BTA_DM_LE_IGNORE) ==
        BTA_DM_LE_IGNORE)
      p_data->set_visibility.conn_mode =
          ((p_data->set_visibility.conn_mode & ~BTA_DM_LE_IGNORE) |
           le_conn_mode);
    if ((p_data->set_visibility.conn_mode & BTA_DM_IGNORE) == BTA_DM_IGNORE)
      p_data->set_visibility.conn_mode =
          ((p_data->set_visibility.conn_mode & ~BTA_DM_IGNORE) | conn_mode);

    BTM_SetConnectability(p_data->set_visibility.conn_mode,
                          bta_dm_cb.page_scan_window,
                          bta_dm_cb.page_scan_interval);
  }

  /* Send False or True if not ignore */
  if (p_data->set_visibility.pair_mode != BTA_DM_IGNORE) {
    if (p_data->set_visibility.pair_mode == BTA_DM_NON_PAIRABLE)
      bta_dm_cb.disable_pair_mode = true;
    else
      bta_dm_cb.disable_pair_mode = false;
  }

  /* Send False or True if not ignore */
  if (p_data->set_visibility.conn_paired_only != BTA_DM_IGNORE) {
    if (p_data->set_visibility.conn_paired_only == BTA_DM_CONN_ALL)
      bta_dm_cb.conn_paired_only = false;
    else
      bta_dm_cb.conn_paired_only = true;
  }

  /* Change mode if either mode is not ignore */
  if (p_data->set_visibility.pair_mode != BTA_DM_IGNORE ||
      p_data->set_visibility.conn_paired_only != BTA_DM_IGNORE)
    BTM_SetPairableMode((bool)(!(bta_dm_cb.disable_pair_mode)),
                          bta_dm_cb.conn_paired_only);
}

/*******************************************************************************
**
** Function         bta_dm_hci_raw_command
**
** Description      Send a HCI RAW command to the controller
**
**
** Returns          void
**
*******************************************************************************/
void bta_dm_hci_raw_command (tBTA_DM_MSG *p_data)
{
    tBTM_STATUS status;
    APPL_TRACE_API("bta_dm_hci_raw_command");
    status = BTM_Hci_Raw_Command(p_data->btc_command.opcode,p_data->btc_command.param_len,p_data->btc_command.p_param_buf, p_data->btc_command.p_cback);

}

/*******************************************************************************
 *
 * Function         bta_dm_process_remove_device
 *
 * Description      Removes device, Disconnects ACL link if required.
 ***
 ******************************************************************************/
void bta_dm_process_remove_device(const RawAddress& bd_addr) {
  /* need to remove all pending background connection before unpair */
  BTA_GATTC_CancelOpen(0, bd_addr, false);

  BTM_SecDeleteDevice(bd_addr);

  /* remove all cached GATT information */
  BTA_GATTC_Refresh(bd_addr);

  if (bta_dm_cb.p_sec_cback) {
    tBTA_DM_SEC sec_event;
    sec_event.link_down.bd_addr = bd_addr;
    ;
    /* No connection, set status to success (acl disc code not valid) */
    sec_event.link_down.status = HCI_SUCCESS;
    bta_dm_cb.p_sec_cback(BTA_DM_DEV_UNPAIRED_EVT, &sec_event);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_remove_device
 *
 * Description      Removes device, disconnects ACL link if required.
 ***
 ******************************************************************************/
void bta_dm_remove_device(tBTA_DM_MSG* p_data) {
  tBTA_DM_API_REMOVE_DEVICE* p_dev = &p_data->remove_dev;
  bool continue_delete_other_dev = false;
  if (p_dev == NULL) return;

  RawAddress other_address = p_dev->bd_addr;

  /* If ACL exists for the device in the remove_bond message*/
  bool continue_delete_dev = false;
  uint8_t other_transport = BT_TRANSPORT_INVALID;
  interop_database_remove_addr(INTEROP_DYNAMIC_ROLE_SWITCH, &other_address);

  if (BTM_IsAclConnectionUp(p_dev->bd_addr, BT_TRANSPORT_LE) ||
      BTM_IsAclConnectionUp(p_dev->bd_addr, BT_TRANSPORT_BR_EDR)) {
    APPL_TRACE_DEBUG("%s: ACL Up count  %d", __func__,
                     bta_dm_cb.device_list.count);
    continue_delete_dev = false;

    /* Take the link down first, and mark the device for removal when
     * disconnected */
    for (int i = 0; i < bta_dm_cb.device_list.count; i++) {
      auto& peer_device = bta_dm_cb.device_list.peer_device[i];
      if (peer_device.peer_bdaddr == p_dev->bd_addr) {
        peer_device.conn_state = BTA_DM_UNPAIRING;

        /* Make sure device is not in white list before we disconnect */
        GATT_CancelConnect(0, p_dev->bd_addr, false);

        btm_remove_acl(p_dev->bd_addr, peer_device.transport);
        APPL_TRACE_DEBUG("%s: transport: %d", __func__, peer_device.transport);

        /* save the other transport to check if device is connected on
         * other_transport */
        if (bta_dm_cb.device_list.peer_device[i].transport == BT_TRANSPORT_LE)
          other_transport = BT_TRANSPORT_BR_EDR;
        else
          other_transport = BT_TRANSPORT_LE;

        break;
      }
    }
  } else {
    continue_delete_dev = true;
  }
  // If it is DUMO device and device is paired as different address, unpair that
  // device
  // if different address
  if ((other_transport &&
       (BTM_ReadConnectedTransportAddress(&other_address, other_transport))) ||
      (!other_transport &&
        BTM_ReadConnectedTransportAddress(&other_address, BT_TRANSPORT_INVALID))) {
    continue_delete_other_dev = false;
    /* Take the link down first, and mark the device for removal when
     * disconnected */
    for (int i = 0; i < bta_dm_cb.device_list.count; i++) {
      auto& peer_device = bta_dm_cb.device_list.peer_device[i];
      if (peer_device.peer_bdaddr == other_address) {
        peer_device.conn_state = BTA_DM_UNPAIRING;

        /* Make sure device is not in white list before we disconnect */
        GATT_CancelConnect(0, p_dev->bd_addr, false);

        btm_remove_acl(other_address, peer_device.transport);
        break;
      }
    }
  } else {
    APPL_TRACE_DEBUG("%s: continue to delete the other dev ", __func__);
    continue_delete_other_dev = true;
  }
  /* Delete the device mentioned in the msg */
  if (continue_delete_dev) bta_dm_process_remove_device(p_dev->bd_addr);

  /* Delete the other paired device too */
  if (continue_delete_other_dev && !other_address.IsEmpty())
    bta_dm_process_remove_device(other_address);
}

/*******************************************************************************
 *
 * Function         bta_dm_add_device
 *
 * Description      This function adds a Link Key to an security database entry.
 *                  It is normally called during host startup to restore all
 *                  required information stored in the NVRAM.
 ******************************************************************************/
void bta_dm_add_device(tBTA_DM_MSG* p_data) {
  tBTA_DM_API_ADD_DEVICE* p_dev = &p_data->add_dev;
  uint8_t* p_dc = NULL;
  LinkKey* p_lc = NULL;
  uint32_t trusted_services_mask[BTM_SEC_SERVICE_ARRAY_SIZE];
  uint8_t index = 0;
  uint8_t btm_mask_index = 0;

  memset(trusted_services_mask, 0, sizeof(trusted_services_mask));

  /* If not all zeros, the device class has been specified */
  if (p_dev->dc_known) p_dc = (uint8_t*)p_dev->dc;

  if (p_dev->link_key_known) p_lc = &p_dev->link_key;

  if (p_dev->is_trusted) {
    /* covert BTA service mask to BTM mask */
    while (p_dev->tm && (index < BTA_MAX_SERVICE_ID)) {
      if (p_dev->tm & (uint32_t)(1 << index)) {
        btm_mask_index =
            bta_service_id_to_btm_srv_id_lkup_tbl[index] / BTM_SEC_ARRAY_BITS;
        trusted_services_mask[btm_mask_index] |=
            (uint32_t)(1 << (bta_service_id_to_btm_srv_id_lkup_tbl[index] -
                             (uint32_t)(btm_mask_index * 32)));

        p_dev->tm &= (uint32_t)(~(1 << index));
      }
      index++;
    }
  }

  if (!BTM_SecAddDevice(p_dev->bd_addr, p_dc, p_dev->bd_name, p_dev->features,
                        trusted_services_mask, p_lc, p_dev->key_type,
                        p_dev->io_cap, p_dev->pin_length)) {
    LOG(ERROR) << "BTA_DM: Error adding device " << p_dev->bd_addr;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_close_acl
 *
 * Description      This function forces to close the connection to a remote
 *                  device and optionaly remove the device from security
 *                  database if required.
 ***
 ******************************************************************************/
void bta_dm_close_acl(tBTA_DM_MSG* p_data) {
  tBTA_DM_API_REMOVE_ACL* p_remove_acl = &p_data->remove_acl;
  uint8_t index;

  APPL_TRACE_DEBUG("bta_dm_close_acl");

  if (BTM_IsAclConnectionUp(p_remove_acl->bd_addr, p_remove_acl->transport)) {
    for (index = 0; index < bta_dm_cb.device_list.count; index++) {
      if (bta_dm_cb.device_list.peer_device[index].peer_bdaddr ==
          p_remove_acl->bd_addr)
        break;
    }
    if (index != bta_dm_cb.device_list.count) {
      if (p_remove_acl->remove_dev)
        bta_dm_cb.device_list.peer_device[index].remove_dev_pending = true;
    } else {
      APPL_TRACE_ERROR("unknown device, remove ACL failed");
    }

    /* Make sure device is not in white list before we disconnect */
    GATT_CancelConnect(0, p_remove_acl->bd_addr, false);

    /* Disconnect the ACL link */
    btm_remove_acl(p_remove_acl->bd_addr, p_remove_acl->transport);
  }
  /* if to remove the device from security database ? do it now */
  else if (p_remove_acl->remove_dev) {
    if (!BTM_SecDeleteDevice(p_remove_acl->bd_addr)) {
      APPL_TRACE_ERROR("delete device from security database failed.");
    }
    /* need to remove all pending background connection if any */
    BTA_GATTC_CancelOpen(0, p_remove_acl->bd_addr, false);
    /* remove all cached GATT information */
    BTA_GATTC_Refresh(p_remove_acl->bd_addr);
  }
  /* otherwise, no action needed */
}

/*******************************************************************************
 *
 * Function         bta_dm_remove_all_acl
 *
 * Description      This function forces to close all the ACL links specified by
 *                  link type
 ***
 ******************************************************************************/
void bta_dm_remove_all_acl(tBTA_DM_MSG* p_data) {
  const tBTA_DM_LINK_TYPE link_type = p_data->remove_all_acl.link_type;
  tBT_TRANSPORT transport = BT_TRANSPORT_BR_EDR;

  APPL_TRACE_DEBUG("%s link type = %d", __func__, link_type);

  for (uint8_t i = 0; i < bta_dm_cb.device_list.count; i++) {
    transport = bta_dm_cb.device_list.peer_device[i].transport;
    if ((link_type == BTA_DM_LINK_TYPE_ALL) ||
        ((link_type == BTA_DM_LINK_TYPE_LE) &&
         (transport == BT_TRANSPORT_LE)) ||
        ((link_type == BTA_DM_LINK_TYPE_BR_EDR) &&
         (transport == BT_TRANSPORT_BR_EDR))) {
      /* Disconnect the ACL link */
      btm_remove_acl(bta_dm_cb.device_list.peer_device[i].peer_bdaddr,
                     transport);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_bond
 *
 * Description      Bonds with peer device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_bond(tBTA_DM_MSG* p_data) {
  tBTM_STATUS status;
  tBTA_DM_SEC sec_event;
  char* p_name;
  tBTA_DM_API_BOND* p_msg;

  if (p_data->bond.transport == BTA_TRANSPORT_UNKNOWN)
    status = BTM_SecBond(p_data->bond.bd_addr, 0, NULL, 0);
  else
    status = BTM_SecBondByTransport(p_data->bond.bd_addr,
                                    p_data->bond.transport, 0, NULL, 0);
  if (BTM_BUSY == status) {

    p_msg = (tBTA_DM_API_BOND*)osi_malloc(sizeof(tBTA_DM_API_BOND));
    memcpy(p_msg, p_data, sizeof(tBTA_DM_API_BOND));

    alarm_set_on_mloop(
            bta_dm_cb.bond_retrail_timer, BTA_DM_BOND_TIMER_RETRIAL_MS,
            bta_dm_bond_retrail_cback, p_msg);
    return;
  }
  if (bta_dm_cb.p_sec_cback && (status != BTM_CMD_STARTED)) {
    memset(&sec_event, 0, sizeof(tBTA_DM_SEC));
    sec_event.auth_cmpl.bd_addr = p_data->bond.bd_addr;
    p_name = BTM_SecReadDevName(p_data->bond.bd_addr);
    if (p_name != NULL) {
      memcpy(sec_event.auth_cmpl.bd_name, p_name, BD_NAME_LEN);
      sec_event.auth_cmpl.bd_name[BD_NAME_LEN] = 0;
    }

    /*      taken care of by memset [above]
            sec_event.auth_cmpl.key_present = false;
            sec_event.auth_cmpl.success = false;
    */
    sec_event.auth_cmpl.fail_reason = HCI_ERR_ILLEGAL_COMMAND;
    if (status == BTM_SUCCESS) {
      sec_event.auth_cmpl.success = true;
    } else {
      /* delete this device entry from Sec Dev DB */
      bta_dm_remove_sec_dev_entry(p_data->bond.bd_addr);
    }
    bta_dm_cb.p_sec_cback(BTA_DM_AUTH_CMPL_EVT, &sec_event);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_bond_cancel
 *
 * Description      Cancels bonding with a peer device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_bond_cancel(tBTA_DM_MSG* p_data) {
  tBTM_STATUS status;
  tBTA_DM_SEC sec_event;

  APPL_TRACE_EVENT(" bta_dm_bond_cancel ");
  status = BTM_SecBondCancel(p_data->bond_cancel.bd_addr);

  if (bta_dm_cb.p_sec_cback &&
      (status != BTM_CMD_STARTED && status != BTM_SUCCESS)) {
    sec_event.bond_cancel_cmpl.result = BTA_FAILURE;

    bta_dm_cb.p_sec_cback(BTA_DM_BOND_CANCEL_CMPL_EVT, &sec_event);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_pin_reply
 *
 * Description      Send the pin_reply to a request from BTM
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_pin_reply(tBTA_DM_MSG* p_data) {
  uint32_t trusted_mask[BTM_SEC_SERVICE_ARRAY_SIZE];
  uint32_t* current_trusted_mask;

  current_trusted_mask = BTM_ReadTrustedMask(p_data->pin_reply.bd_addr);

  if (current_trusted_mask) {
    memcpy(trusted_mask, current_trusted_mask, sizeof(trusted_mask));
  } else {
    memset(trusted_mask, 0, sizeof(trusted_mask));
  }

  if (p_data->pin_reply.accept) {
    BTM_PINCodeReply(p_data->pin_reply.bd_addr, BTM_SUCCESS,
                     p_data->pin_reply.pin_len, p_data->pin_reply.p_pin,
                     trusted_mask);
  } else {
    BTM_PINCodeReply(p_data->pin_reply.bd_addr, BTM_NOT_AUTHORIZED, 0, NULL,
                     trusted_mask);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_policy_cback
 *
 * Description      process the link policy changes
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_policy_cback(tBTA_SYS_CONN_STATUS status, uint8_t id,
                                uint8_t app_id, const RawAddress& peer_addr) {
  tBTA_DM_PEER_DEVICE* p_dev = NULL;
  uint16_t policy = app_id;
  uint32_t mask = (uint32_t)(1 << id);

  if (peer_addr != RawAddress::kEmpty) {
    p_dev = bta_dm_find_peer_device(peer_addr);
  }

  APPL_TRACE_DEBUG(" bta_dm_policy_cback cmd:%d, policy:0x%x", status, policy);
  switch (status) {
    case BTA_SYS_PLCY_SET:
      if (!p_dev) return;
      /* restore the default link policy */
      p_dev->link_policy |= policy;
      BTM_SetLinkPolicy(p_dev->peer_bdaddr, &(p_dev->link_policy));
      break;

    case BTA_SYS_PLCY_CLR:
      if (!p_dev) return;

      if (policy & (HCI_ENABLE_SNIFF_MODE | HCI_ENABLE_PARK_MODE)) {
        /* if clearing sniff/park, wake the link */
        bta_dm_pm_active(p_dev->peer_bdaddr);
      }
      /* clear the policy from the default link policy */
      p_dev->link_policy &= (~policy);
      BTM_SetLinkPolicy(p_dev->peer_bdaddr, &(p_dev->link_policy));
      break;

    case BTA_SYS_PLCY_DEF_SET:
      /* want to restore/set the role switch policy */
      bta_dm_cb.role_policy_mask &= ~mask;
      if (0 == bta_dm_cb.role_policy_mask) {
        /* if nobody wants to insist on the role */
        bta_dm_cb.cur_policy |= HCI_ENABLE_MASTER_SLAVE_SWITCH;
        BTM_SetDefaultLinkPolicy(bta_dm_cb.cur_policy);
      }
      break;

    case BTA_SYS_PLCY_DEF_CLR:
      /* want to remove the role switch policy */
      bta_dm_cb.role_policy_mask |= mask;
      bta_dm_cb.cur_policy &= ~HCI_ENABLE_MASTER_SLAVE_SWITCH;
      BTM_SetDefaultLinkPolicy(bta_dm_cb.cur_policy);
      break;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_confirm
 *
 * Description      Send the user confirm request reply in response to a
 *                  request from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_confirm(tBTA_DM_MSG* p_data) {
  tBTM_STATUS res = BTM_NOT_AUTHORIZED;

  if (p_data->confirm.accept) res = BTM_SUCCESS;
  BTM_ConfirmReqReply(res, p_data->confirm.bd_addr);
}

/*******************************************************************************
 *
 * Function         bta_dm_loc_oob
 *
 * Description      Retrieve the OOB data from the local LM
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_loc_oob(UNUSED_ATTR tBTA_DM_MSG* p_data) { BTM_ReadLocalOobData(); }

/*******************************************************************************
 *
 * Function         bta_dm_ci_io_req_act
 *
 * Description      respond to the IO capabilities request from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_ci_io_req_act(tBTA_DM_MSG* p_data) {
  tBTM_AUTH_REQ auth_req = BTM_AUTH_AP_NO;
  if (p_data->ci_io_req.auth_req) auth_req = BTM_AUTH_AP_YES;
  BTM_IoCapRsp(p_data->ci_io_req.bd_addr, p_data->ci_io_req.io_cap,
               p_data->ci_io_req.oob_data, auth_req);
}

/*******************************************************************************
 *
 * Function         bta_dm_ci_rmt_oob_act
 *
 * Description      respond to the OOB data request for the remote device from
 *                  BTM
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_ci_rmt_oob_act(tBTA_DM_MSG* p_data) {
  tBTM_STATUS res = BTM_NOT_AUTHORIZED;

  if (p_data->ci_rmt_oob.accept) res = BTM_SUCCESS;
  BTM_RemoteOobDataReply(res, p_data->ci_rmt_oob.bd_addr, p_data->ci_rmt_oob.c,
                         p_data->ci_rmt_oob.r);
}

/*******************************************************************************
 *
 * Function         bta_dm_search_start
 *
 * Description      Starts an inquiry
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_start(tBTA_DM_MSG* p_data) {
  tBTM_INQUIRY_CMPL result;

  size_t len = sizeof(Uuid) * p_data->search.num_uuid;
  bta_dm_gattc_register();

  APPL_TRACE_DEBUG("%s avoid_scatter=%d", __func__,
                   p_bta_dm_cfg->avoid_scatter);

  if (p_bta_dm_cfg->avoid_scatter &&
      (p_data->search.rs_res == BTA_DM_RS_NONE) &&
      bta_dm_check_av(BTA_DM_API_SEARCH_EVT)) {
    memcpy(&bta_dm_cb.search_msg, &p_data->search, sizeof(tBTA_DM_API_SEARCH));
    return;
  }

  BTM_ClearInqDb(NULL);
  /* save search params */
  bta_dm_search_cb.p_search_cback = p_data->search.p_cback;
  bta_dm_search_cb.services = p_data->search.services;

  osi_free_and_reset((void**)&bta_dm_search_cb.p_srvc_uuid);

  if ((bta_dm_search_cb.num_uuid = p_data->search.num_uuid) != 0 &&
      p_data->search.p_uuid != NULL) {
    bta_dm_search_cb.p_srvc_uuid = (Uuid*)osi_malloc(len);
    *bta_dm_search_cb.p_srvc_uuid = *p_data->search.p_uuid;
  }
  result.status = BTM_StartInquiry((tBTM_INQ_PARMS*)&p_data->search.inq_params,
                                   bta_dm_inq_results_cb,
                                   (tBTM_CMPL_CB*)bta_dm_inq_cmpl_cb);

  APPL_TRACE_EVENT("%s status=%d", __func__, result.status);
  if (result.status != BTM_CMD_STARTED) {
    result.num_resp = 0;
    bta_dm_inq_cmpl_cb((void*)&result);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_search_cancel
 *
 * Description      Cancels an ongoing search for devices
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_cancel(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  tBTA_DM_MSG* p_msg;

  if (BTM_IsInquiryActive()) {
    if (BTM_CancelInquiry() == BTM_SUCCESS) {
      bta_dm_search_cancel_notify(NULL);
      p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));
      p_msg->hdr.event = BTA_DM_SEARCH_CMPL_EVT;
      p_msg->hdr.layer_specific = BTA_DM_API_DISCOVER_EVT;
      bta_sys_sendmsg(p_msg);
    } else {
      /* flag a search cancel is pending */
      bta_dm_search_cb.cancel_pending = true;
    }
  }
  /* If no Service Search going on then issue cancel remote name in case it is
     active */
  else if (!bta_dm_search_cb.name_discover_done) {
    BTM_CancelRemoteDeviceName();

    p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));
    p_msg->hdr.event = BTA_DM_SEARCH_CMPL_EVT;
    p_msg->hdr.layer_specific = BTA_DM_API_DISCOVER_EVT;
    bta_sys_sendmsg(p_msg);
  } else {
    p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));
    p_msg->hdr.event = BTA_DM_INQUIRY_CMPL_EVT;
    p_msg->hdr.layer_specific = BTA_DM_API_DISCOVER_EVT;
    bta_sys_sendmsg(p_msg);
  }

  if (bta_dm_search_cb.gatt_disc_active) {
    bta_dm_cancel_gatt_discovery(bta_dm_search_cb.peer_bdaddr);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_discover
 *
 * Description      Discovers services on a remote device
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_discover(tBTA_DM_MSG* p_data) {
  size_t len = sizeof(Uuid) * p_data->discover.num_uuid;
  APPL_TRACE_EVENT("%s services_to_search=0x%04X, sdp_search=%d", __func__,
                   p_data->discover.services, p_data->discover.sdp_search);

  /* save the search condition */
  bta_dm_search_cb.services = p_data->discover.services;

  bta_dm_gattc_register();
  osi_free_and_reset((void**)&bta_dm_search_cb.p_srvc_uuid);
  if ((bta_dm_search_cb.num_uuid = p_data->discover.num_uuid) != 0 &&
      p_data->discover.p_uuid != NULL) {
    bta_dm_search_cb.p_srvc_uuid = (Uuid*)osi_malloc(len);
    *bta_dm_search_cb.p_srvc_uuid = *p_data->discover.p_uuid;
  }
  bta_dm_search_cb.uuid_to_search = bta_dm_search_cb.num_uuid;

  bta_dm_search_cb.p_search_cback = p_data->discover.p_cback;
  bta_dm_search_cb.sdp_search = p_data->discover.sdp_search;
  bta_dm_search_cb.services_to_search = bta_dm_search_cb.services;
  bta_dm_search_cb.service_index = 0;
  bta_dm_search_cb.services_found = 0;
  bta_dm_search_cb.peer_name[0] = 0;
  bta_dm_search_cb.sdp_search = p_data->discover.sdp_search;
  bta_dm_search_cb.p_btm_inq_info = BTM_InqDbRead(p_data->discover.bd_addr);
  bta_dm_search_cb.transport = p_data->discover.transport;

  bta_dm_search_cb.name_discover_done = false;
  bta_dm_search_cb.uuid = p_data->discover.uuid;
  bta_dm_discover_device(p_data->discover.bd_addr);
}

/*******************************************************************************
 *
 * Function         bta_dm_di_disc_cmpl
 *
 * Description      Sends event to application when DI discovery complete
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_di_disc_cmpl(tBTA_DM_MSG* p_data) {
  tBTA_DM_DI_DISC_CMPL di_disc;

  memset(&di_disc, 0, sizeof(tBTA_DM_DI_DISC_CMPL));
  di_disc.bd_addr = bta_dm_search_cb.peer_bdaddr;

  if ((p_data->hdr.offset == SDP_SUCCESS) ||
      (p_data->hdr.offset == SDP_DB_FULL)) {
    di_disc.num_record = SDP_GetNumDiRecords(bta_dm_di_cb.p_di_db);
  } else
    di_disc.result = BTA_FAILURE;

  bta_dm_di_cb.p_di_db = NULL;
  bta_dm_search_cb.p_search_cback(BTA_DM_DI_DISC_CMPL_EVT,
                                  (tBTA_DM_SEARCH*)&di_disc);
}

/*******************************************************************************
 *
 * Function         bta_dm_di_disc_callback
 *
 * Description      This function queries a remote device for DI information.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_di_disc_callback(uint16_t result) {
  tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));

  p_msg->hdr.event = BTA_DM_SEARCH_CMPL_EVT;
  p_msg->hdr.layer_specific = BTA_DM_API_DI_DISCOVER_EVT;
  p_msg->hdr.offset = result;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         bta_dm_disable_search_and_disc
 *
 * Description      Cancels an ongoing search or discovery for devices in case
 *                  of a Bluetooth disable
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_disable_search_and_disc(void) {
  tBTA_DM_DI_DISC_CMPL di_disc;

  if (bta_dm_search_cb.state != BTA_DM_SEARCH_IDLE) bta_dm_search_cancel(NULL);

  if (bta_dm_di_cb.p_di_db != NULL) {
    memset(&di_disc, 0, sizeof(tBTA_DM_DI_DISC_CMPL));
    di_disc.bd_addr = bta_dm_search_cb.peer_bdaddr;
    di_disc.result = BTA_FAILURE;

    bta_dm_di_cb.p_di_db = NULL;
    bta_dm_search_cb.p_search_cback(BTA_DM_DI_DISC_CMPL_EVT, NULL);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_di_disc
 *
 * Description      This function queries a remote device for DI information.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_di_disc(tBTA_DM_MSG* p_data) {
  uint16_t result = BTA_FAILURE;

  bta_dm_search_cb.p_search_cback = p_data->di_disc.p_cback;
  bta_dm_search_cb.peer_bdaddr = p_data->di_disc.bd_addr;
  bta_dm_di_cb.p_di_db = p_data->di_disc.p_sdp_db;

  bta_dm_search_cb.p_sdp_db =
      (tSDP_DISCOVERY_DB*)osi_malloc(BTA_DM_SDP_DB_SIZE);
  if (SDP_DiDiscover(bta_dm_search_cb.peer_bdaddr, p_data->di_disc.p_sdp_db,
                     p_data->di_disc.len,
                     bta_dm_di_disc_callback) == SDP_SUCCESS) {
    result = BTA_SUCCESS;
  }

  if (result == BTA_FAILURE) {
    tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));

    p_msg->hdr.event = BTA_DM_SEARCH_CMPL_EVT;
    p_msg->hdr.layer_specific = BTA_DM_API_DI_DISCOVER_EVT;
    p_data->hdr.offset = result;
    bta_sys_sendmsg(p_msg);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_read_remote_device_name
 *
 * Description      Initiate to get remote device name
 *
 * Returns          true if started to get remote name
 *
 ******************************************************************************/
static bool bta_dm_read_remote_device_name(const RawAddress& bd_addr,
                                           tBT_TRANSPORT transport) {
  tBTM_STATUS btm_status;

  APPL_TRACE_DEBUG("%s", __func__);

  bta_dm_search_cb.peer_bdaddr = bd_addr;
  bta_dm_search_cb.peer_name[0] = 0;

  btm_status =
      BTM_ReadRemoteDeviceName(bta_dm_search_cb.peer_bdaddr,
                               (tBTM_CMPL_CB*)bta_dm_remname_cback, transport);

  if (btm_status == BTM_CMD_STARTED) {
    APPL_TRACE_DEBUG("%s: BTM_ReadRemoteDeviceName is started", __func__);

    return (true);
  } else if (btm_status == BTM_BUSY) {
    APPL_TRACE_DEBUG("%s: BTM_ReadRemoteDeviceName is busy", __func__);

    /* Remote name discovery is on going now so BTM cannot notify through
     * "bta_dm_remname_cback" */
    /* adding callback to get notified that current reading remore name done */
    BTM_SecAddRmtNameNotifyCallback(&bta_dm_service_search_remname_cback);

    return (true);
  } else {
    APPL_TRACE_WARNING("%s: BTM_ReadRemoteDeviceName returns 0x%02X", __func__,
                       btm_status);

    return (false);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_inq_cmpl
 *
 * Description      Process the inquiry complete event from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_inq_cmpl(tBTA_DM_MSG* p_data) {
  tBTA_DM_SEARCH data;

  APPL_TRACE_DEBUG("bta_dm_inq_cmpl");

  data.inq_cmpl.num_resps = p_data->inq_cmpl.num;
  bta_dm_search_cb.p_search_cback(BTA_DM_INQ_CMPL_EVT, &data);

  bta_dm_search_cb.p_btm_inq_info = BTM_InqDbFirst();
  if (bta_dm_search_cb.p_btm_inq_info != NULL) {
    /* start name and service discovery from the first device on inquiry result
     */
    bta_dm_search_cb.name_discover_done = false;
    bta_dm_search_cb.peer_name[0] = 0;
    bta_dm_search_cb.transport = BTA_TRANSPORT_UNKNOWN;
    bta_dm_discover_device(
        bta_dm_search_cb.p_btm_inq_info->results.remote_bd_addr);
  } else {
    tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));

    /* no devices, search complete */
    bta_dm_search_cb.services = 0;

    p_msg->hdr.event = BTA_DM_SEARCH_CMPL_EVT;
    p_msg->hdr.layer_specific = BTA_DM_API_DISCOVER_EVT;
    bta_sys_sendmsg(p_msg);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_rmt_name
 *
 * Description      Process the remote name result from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_rmt_name(tBTA_DM_MSG* p_data) {
  APPL_TRACE_DEBUG("bta_dm_rmt_name");

  if (p_data->rem_name.result.disc_res.bd_name[0] &&
      bta_dm_search_cb.p_btm_inq_info) {
    bta_dm_search_cb.p_btm_inq_info->appl_knows_rem_name = true;
  }

  bta_dm_discover_device(bta_dm_search_cb.peer_bdaddr);
}

/*******************************************************************************
 *
 * Function         bta_dm_disc_rmt_name
 *
 * Description      Process the remote name result from BTM when application
 *                  wants to find the name for a bdaddr
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_disc_rmt_name(tBTA_DM_MSG* p_data) {
  tBTM_INQ_INFO* p_btm_inq_info;

  APPL_TRACE_DEBUG("bta_dm_disc_rmt_name");

  p_btm_inq_info = BTM_InqDbRead(p_data->rem_name.result.disc_res.bd_addr);
  if (p_btm_inq_info) {
    if (p_data->rem_name.result.disc_res.bd_name[0]) {
      p_btm_inq_info->appl_knows_rem_name = true;
    }
  }

  bta_dm_discover_device(p_data->rem_name.result.disc_res.bd_addr);
}

static void bta_dm_store_profiles_version() {
  tSDP_DISC_REC* sdp_rec = NULL;
  uint16_t profile_version, avdtp_version = 0;
  uint16_t avrcp_features = 0;
  tSDP_DISC_ATTR* p_attr;
  tSDP_PROTOCOL_ELEM elem;
  int i;

  const UINT16 servclass_uuids[] = {
    UUID_SERVCLASS_AUDIO_SINK,
    UUID_SERVCLASS_HF_HANDSFREE,
    UUID_SERVCLASS_AV_REMOTE_CONTROL,
    UUID_SERVCLASS_AV_REM_CTRL_TARGET,
    UUID_SERVCLASS_MESSAGE_NOTIFICATION,
  };

  const UINT16 btprofile_uuids[] = {
    UUID_SERVCLASS_ADV_AUDIO_DISTRIBUTION,
    UUID_SERVCLASS_HF_HANDSFREE,
    UUID_SERVCLASS_AV_REMOTE_CONTROL,
    UUID_SERVCLASS_AV_REMOTE_CONTROL,
    UUID_SERVCLASS_MAP_PROFILE,
  };

  const char* profile_keys[] = {
    A2DP_VERSION_CONFIG_KEY,
    HFP_VERSION_CONFIG_KEY,
    AV_REM_CTRL_VERSION_CONFIG_KEY,
    AV_REM_CTRL_TG_VERSION_CONFIG_KEY,
    MAP_MCE_VERSION_CONFIG_KEY,
  };

#if (BT_IOT_LOGGING_ENABLED == TRUE)
  const char* iot_profile_keys[] = {
    IOT_CONF_KEY_A2DP_VERSION,
    IOT_CONF_KEY_HFP_VERSION,
    IOT_CONF_KEY_AVRCP_CTRL_VERSION,
    IOT_CONF_KEY_AVRCP_TG_VERSION,
  };
#endif

  int profile_num = sizeof(servclass_uuids)/sizeof(servclass_uuids[0]);

  APPL_TRACE_DEBUG("%s", __func__);

  for (i = 0; i < profile_num; i++) {
    profile_version = 0;
    if ((sdp_rec =
         SDP_FindServiceInDb(bta_dm_search_cb.p_sdp_db, servclass_uuids[i], NULL))
         == NULL)
      continue;

    if (SDP_FindAttributeInRec(sdp_rec, ATTR_ID_BT_PROFILE_DESC_LIST) == NULL)
      continue;
    /* get profile version (if failure, version parameter is not updated) */
    SDP_FindProfileVersionInRec(sdp_rec, btprofile_uuids[i], &profile_version);
    if (profile_version != 0) {
      if (servclass_uuids[i] == UUID_SERVCLASS_MESSAGE_NOTIFICATION) {
        APPL_TRACE_DEBUG("%s MCE record found ", __func__);
        check_and_store_mce_profile_version(sdp_rec);
        continue;
      }
      if (btif_config_set_uint16(sdp_rec->remote_bd_addr.ToString().c_str(),
                              profile_keys[i],
                              profile_version)) {
        btif_config_save();
      } else {
        APPL_TRACE_WARNING("%s: Failed to store peer profile version for %s",
                           __func__, sdp_rec->remote_bd_addr.ToString().c_str());
      }
#if (BT_IOT_LOGGING_ENABLED == TRUE)
      device_iot_config_addr_set_hex(sdp_rec->remote_bd_addr,
          iot_profile_keys[i], profile_version, IOT_CONF_BYTE_NUM_2);
#endif
    }

    if (servclass_uuids[i] == UUID_SERVCLASS_AUDIO_SINK) {
      /* get peer AVDTP version */
      if (SDP_FindProtocolListElemInRec(sdp_rec, UUID_PROTOCOL_AVDTP, &elem)) {
        avdtp_version = elem.params[0];
        if (avdtp_version != 0) {
          if (btif_config_set_uint16(sdp_rec->remote_bd_addr.ToString().c_str(),
                              AVDTP_VERSION_CONFIG_KEY,
                              avdtp_version)) {
            btif_config_save();
          } else {
            APPL_TRACE_WARNING("%s: Failed to store avdtp_version version for %s",
                           __func__, sdp_rec->remote_bd_addr.ToString().c_str());
          }
        }
      }
    }
    /* find peer supported features for avrcp profile*/
    if (servclass_uuids[i] == UUID_SERVCLASS_AV_REMOTE_CONTROL) {
      p_attr = SDP_FindAttributeInRec(sdp_rec, ATTR_ID_SUPPORTED_FEATURES);
      if (p_attr != NULL) {
        avrcp_features = p_attr->attr_value.v.u16;
        if (avrcp_features != 0) {
          APPL_TRACE_DEBUG("avrcp_features: 0x%x", avrcp_features);
          if (btif_config_set_uint16(sdp_rec->remote_bd_addr.ToString().c_str(),
                           AV_REM_CTRL_FEATURES_CONFIG_KEY,
                           avrcp_features)) {
            btif_config_save();
          } else {
            APPL_TRACE_WARNING("%s: Failed to store avrcp_features for %s",
                                __func__, sdp_rec->remote_bd_addr.ToString().c_str());
          }
        }
      }
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_store_di_info
 *
 * Description      Store DI info to config file
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_store_di_info(tSDP_DISC_REC* sdp_rec, uint16_t vendor_id,
                            uint16_t product_id, uint16_t version) {

   APPL_TRACE_DEBUG("vendor_id = 0x%2x product_id = 0x%2x version = 0x%2x",
                    vendor_id, product_id, version);

   btif_config_set_uint16(sdp_rec->remote_bd_addr.ToString().c_str(),
              PNP_VENDOR_ID_CONFIG_KEY, vendor_id);
   btif_config_set_uint16(sdp_rec->remote_bd_addr.ToString().c_str(),
              PNP_PRODUCT_ID_CONFIG_KEY, product_id);
   btif_config_set_uint16(sdp_rec->remote_bd_addr.ToString().c_str(),
              PNP_PRODUCT_VERSION_CONFIG_KEY, version);
   btif_config_save();
}

/*******************************************************************************
 *
 * Function         bta_dm_sdp_result
 *
 * Description      Process the discovery result from sdp
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_sdp_result(tBTA_DM_MSG* p_data) {
  tSDP_DISC_REC* p_sdp_rec = NULL;
  tBTA_DM_MSG* p_msg;
  bool scn_found = false;
  uint16_t service = 0xFFFF;
  tSDP_PROTOCOL_ELEM pe;
  tSDP_DI_GET_RECORD di_rec;

  Uuid* p_uuid = bta_dm_search_cb.p_srvc_uuid;
  tBTA_DM_SEARCH result;

  std::vector<Uuid> uuid_list;

  if ((p_data->sdp_event.sdp_result == SDP_SUCCESS) ||
      (p_data->sdp_event.sdp_result == SDP_NO_RECS_MATCH) ||
      (p_data->sdp_event.sdp_result == SDP_DB_FULL)) {
    APPL_TRACE_DEBUG("sdp_result::0x%x", p_data->sdp_event.sdp_result);
    do {
      p_sdp_rec = NULL;
      if (bta_dm_search_cb.service_index == (BTA_USER_SERVICE_ID + 1)) {
        p_sdp_rec = SDP_FindServiceUUIDInDb(bta_dm_search_cb.p_sdp_db,
                                            bta_dm_search_cb.uuid, p_sdp_rec);

        if (p_sdp_rec && SDP_FindProtocolListElemInRec(
                             p_sdp_rec, UUID_PROTOCOL_RFCOMM, &pe)) {
          bta_dm_search_cb.peer_scn = (uint8_t)pe.params[0];
          scn_found = true;
        }
      } else {
        service =
            bta_service_id_to_uuid_lkup_tbl[bta_dm_search_cb.service_index - 1];
        p_sdp_rec =
            SDP_FindServiceInDb(bta_dm_search_cb.p_sdp_db, service, p_sdp_rec);
        if (service == UUID_SERVCLASS_PNP_INFORMATION) {
          APPL_TRACE_DEBUG(" Store DI info to conf file " );
          if (SDP_GetNumDiRecords(bta_dm_search_cb.p_sdp_db) != 0) {
            /* always update information with primary DI record */
            if (SDP_GetDiRecord(1, &di_rec, bta_dm_search_cb.p_sdp_db) == SDP_SUCCESS) {
               bta_dm_store_di_info(p_sdp_rec, di_rec.rec.vendor, di_rec.rec.product,
                                  di_rec.rec.version);
            }
          }
        }
      }

      /* finished with BR/EDR services, now we check the result for GATT based
       * service UUID */
      if (bta_dm_search_cb.service_index == BTA_MAX_SERVICE_ID) {
        if (bta_dm_search_cb.uuid_to_search != 0 && p_uuid != NULL) {
          p_uuid +=
              (bta_dm_search_cb.num_uuid - bta_dm_search_cb.uuid_to_search);
          /* only support 16 bits UUID for now */
          service = p_uuid->As16Bit();
        }
        /* all GATT based services */
        do {
          /* find a service record, report it */
          p_sdp_rec =
              SDP_FindServiceInDb(bta_dm_search_cb.p_sdp_db, 0, p_sdp_rec);
          if (p_sdp_rec) {
            Uuid service_uuid;
            if (SDP_FindServiceUUIDInRec(p_sdp_rec, &service_uuid)) {
              /* send result back to app now, one by one */
              result.disc_ble_res.bd_addr = bta_dm_search_cb.peer_bdaddr;
              strlcpy((char*)result.disc_ble_res.bd_name, bta_dm_get_remname(),
                      BD_NAME_LEN + 1);

              result.disc_ble_res.service = service_uuid;
              bta_dm_search_cb.p_search_cback(BTA_DM_DISC_BLE_RES_EVT, &result);
            }
          }

          if (bta_dm_search_cb.uuid_to_search > 0) break;

        } while (p_sdp_rec);
      } else {
        /* SDP_DB_FULL means some records with the
           required attributes were received */
        if (((p_data->sdp_event.sdp_result == SDP_DB_FULL) &&
             bta_dm_search_cb.services != BTA_ALL_SERVICE_MASK) ||
            (p_sdp_rec != NULL)) {
          if (service != UUID_SERVCLASS_PNP_INFORMATION) {
            bta_dm_search_cb.services_found |=
                (tBTA_SERVICE_MASK)(BTA_SERVICE_ID_TO_SERVICE_MASK(
                    bta_dm_search_cb.service_index - 1));
            uint16_t tmp_svc =
                bta_service_id_to_uuid_lkup_tbl[bta_dm_search_cb.service_index -
                                                1];
            /* Add to the list of UUIDs */
            uuid_list.push_back(Uuid::From16Bit(tmp_svc));
          }
        }
      }

      if (bta_dm_search_cb.services == BTA_ALL_SERVICE_MASK &&
          bta_dm_search_cb.services_to_search == 0) {
        if (bta_dm_search_cb.service_index == BTA_BLE_SERVICE_ID &&
            bta_dm_search_cb.uuid_to_search > 0)
          bta_dm_search_cb.uuid_to_search--;

        if (bta_dm_search_cb.uuid_to_search == 0 ||
            bta_dm_search_cb.service_index != BTA_BLE_SERVICE_ID)
          bta_dm_search_cb.service_index++;
      } else /* regular one service per search or PNP search */
        break;

    } while (bta_dm_search_cb.service_index <= BTA_MAX_SERVICE_ID);

    APPL_TRACE_DEBUG("%s services_found = %04x", __func__,
                     bta_dm_search_cb.services_found);

    /* Collect the 128-bit services here and put them into the list */
    if (bta_dm_search_cb.services == BTA_ALL_SERVICE_MASK) {
      p_sdp_rec = NULL;
      do {
        /* find a service record, report it */
        p_sdp_rec =
            SDP_FindServiceInDb_128bit(bta_dm_search_cb.p_sdp_db, p_sdp_rec);
        if (p_sdp_rec) {
          // SDP_FindServiceUUIDInRec_128bit is used only once, refactor?
          Uuid temp_uuid;
          if (SDP_FindServiceUUIDInRec_128bit(p_sdp_rec, &temp_uuid)) {
            uuid_list.push_back(temp_uuid);
          }
        }
      } while (p_sdp_rec);

      if (bta_dm_search_cb.services_to_search == 0)
         bta_dm_store_profiles_version();

    }
    /* if there are more services to search for */
    if (bta_dm_search_cb.services_to_search) {
      /* Free up the p_sdp_db before checking the next one */
      bta_dm_free_sdp_db(NULL);
      bta_dm_find_services(bta_dm_search_cb.peer_bdaddr);
    } else {
      /* callbacks */
      /* start next bd_addr if necessary */

      BTM_SecDeleteRmtNameNotifyCallback(&bta_dm_service_search_remname_cback);

      p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));
      p_msg->hdr.event = BTA_DM_DISCOVERY_RESULT_EVT;
      p_msg->disc_result.result.disc_res.result = BTA_SUCCESS;
      p_msg->disc_result.result.disc_res.p_raw_data = NULL;
      p_msg->disc_result.result.disc_res.raw_data_size = 0;
      p_msg->disc_result.result.disc_res.num_uuids = uuid_list.size();
      p_msg->disc_result.result.disc_res.p_uuid_list = NULL;
      if (uuid_list.size() > 0) {
        // TODO(jpawlowski): make p_uuid_list into vector, and just copy
        // vectors, but first get rid of bta_sys_sendmsg below.
        p_msg->disc_result.result.disc_res.p_uuid_list =
            (Uuid*)osi_malloc(uuid_list.size() * sizeof(Uuid));
        memcpy(p_msg->disc_result.result.disc_res.p_uuid_list, uuid_list.data(),
               uuid_list.size() * sizeof(Uuid));
      }
      // Copy the raw_data to the discovery result structure
      if (bta_dm_search_cb.p_sdp_db != NULL &&
          bta_dm_search_cb.p_sdp_db->raw_used != 0 &&
          bta_dm_search_cb.p_sdp_db->raw_data != NULL) {
        APPL_TRACE_DEBUG("%s raw_data used = 0x%x raw_data_ptr = 0x%x",
                         __func__, bta_dm_search_cb.p_sdp_db->raw_used,
                         bta_dm_search_cb.p_sdp_db->raw_data);

        p_msg->disc_result.result.disc_res.p_raw_data =
            (uint8_t*)osi_malloc(bta_dm_search_cb.p_sdp_db->raw_used);
        memcpy(p_msg->disc_result.result.disc_res.p_raw_data,
               bta_dm_search_cb.p_sdp_db->raw_data,
               bta_dm_search_cb.p_sdp_db->raw_used);

        p_msg->disc_result.result.disc_res.raw_data_size =
            bta_dm_search_cb.p_sdp_db->raw_used;

        bta_dm_search_cb.p_sdp_db->raw_data =
            NULL;  // no need to free this - it is a global assigned.
        bta_dm_search_cb.p_sdp_db->raw_used = 0;
        bta_dm_search_cb.p_sdp_db->raw_size = 0;
      } else {
        APPL_TRACE_DEBUG("%s raw data size is 0 or raw_data is null!!",
                         __func__);
      }
      /* Done with p_sdp_db. Free it */
      bta_dm_free_sdp_db(NULL);
      p_msg->disc_result.result.disc_res.services =
          bta_dm_search_cb.services_found;

      // Piggy back the SCN over result field
      if (scn_found &&
         (bta_dm_search_cb.services == BTA_USER_SERVICE_MASK)) {
        p_msg->disc_result.result.disc_res.result =
            (3 + bta_dm_search_cb.peer_scn);
        p_msg->disc_result.result.disc_res.services |= BTA_USER_SERVICE_MASK;

        APPL_TRACE_EVENT(" Piggy back the SCN over result field  SCN=%d",
                         bta_dm_search_cb.peer_scn);
      }
      p_msg->disc_result.result.disc_res.bd_addr = bta_dm_search_cb.peer_bdaddr;
      strlcpy((char*)p_msg->disc_result.result.disc_res.bd_name,
              bta_dm_get_remname(), BD_NAME_LEN + 1);

      bta_sys_sendmsg(p_msg);
    }
  } else {
    /* conn failed. No need for timer */
    if (p_data->sdp_event.sdp_result == SDP_CONN_FAILED ||
        p_data->sdp_event.sdp_result == SDP_CONN_REJECTED ||
        p_data->sdp_event.sdp_result == SDP_SECURITY_ERR)
      bta_dm_search_cb.wait_disc = false;

    /* not able to connect go to next device */
    if (bta_dm_search_cb.p_sdp_db)
      osi_free_and_reset((void**)&bta_dm_search_cb.p_sdp_db);

    BTM_SecDeleteRmtNameNotifyCallback(&bta_dm_service_search_remname_cback);

    p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));
    p_msg->hdr.event = BTA_DM_DISCOVERY_RESULT_EVT;
    p_msg->disc_result.result.disc_res.result = BTA_FAILURE;
    p_msg->disc_result.result.disc_res.services =
        bta_dm_search_cb.services_found;
    p_msg->disc_result.result.disc_res.bd_addr = bta_dm_search_cb.peer_bdaddr;
    strlcpy((char*)p_msg->disc_result.result.disc_res.bd_name,
            bta_dm_get_remname(), BD_NAME_LEN + 1);

    bta_sys_sendmsg(p_msg);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_search_cmpl
 *
 * Description      Sends event to application
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_cmpl(tBTA_DM_MSG* p_data) {
  APPL_TRACE_EVENT("%s", __func__);

  osi_free_and_reset((void**)&bta_dm_search_cb.p_srvc_uuid);

  if (p_data->hdr.layer_specific == BTA_DM_API_DI_DISCOVER_EVT)
    bta_dm_di_disc_cmpl(p_data);
  else
    bta_dm_search_cb.p_search_cback(BTA_DM_DISC_CMPL_EVT, NULL);
  BTA_DmProcessQueuedServiceDiscovery();
}

/*******************************************************************************
 *
 * Function         bta_dm_disc_result
 *
 * Description      Service discovery result when discovering services on a
 *                  device
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_disc_result(tBTA_DM_MSG* p_data) {
  APPL_TRACE_EVENT("%s", __func__);

  /* if any BR/EDR service discovery has been done, report the event */
  if ((bta_dm_search_cb.services &
       ((BTA_ALL_SERVICE_MASK | BTA_USER_SERVICE_MASK) &
        ~BTA_BLE_SERVICE_MASK)))
    bta_dm_search_cb.p_search_cback(BTA_DM_DISC_RES_EVT,
                                    &p_data->disc_result.result);

  tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));

  /* send a message to change state */
  p_msg->hdr.event = BTA_DM_SEARCH_CMPL_EVT;
  p_msg->hdr.layer_specific = BTA_DM_API_DISCOVER_EVT;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         bta_dm_search_result
 *
 * Description      Service discovery result while searching for devices
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_result(tBTA_DM_MSG* p_data) {
  APPL_TRACE_DEBUG("%s searching:0x%04x, result:0x%04x", __func__,
                   bta_dm_search_cb.services,
                   p_data->disc_result.result.disc_res.services);

  /* call back if application wants name discovery or found services that
   * application is searching */
  if ((!bta_dm_search_cb.services) ||
      ((bta_dm_search_cb.services) &&
       (p_data->disc_result.result.disc_res.services))) {
    bta_dm_search_cb.p_search_cback(BTA_DM_DISC_RES_EVT,
                                    &p_data->disc_result.result);
  }

  /* if searching did not initiate to create link */
  if (!bta_dm_search_cb.wait_disc) {
    /* if service searching is done with EIR, don't search next device */
    if (bta_dm_search_cb.p_btm_inq_info) bta_dm_discover_next_device();
  } else {
    /* wait until link is disconnected or timeout */
    bta_dm_search_cb.sdp_results = true;
    alarm_set_on_mloop(bta_dm_search_cb.search_timer,
                       1000 * (L2CAP_LINK_INACTIVITY_TOUT + 1),
                       bta_dm_search_timer_cback, NULL);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_search_timer_cback
 *
 * Description      Called when ACL disconnect time is over
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_search_timer_cback(UNUSED_ATTR void* data) {
  APPL_TRACE_EVENT("%s", __func__);
  bta_dm_search_cb.wait_disc = false;

  /* proceed with next device */
  bta_dm_discover_next_device();
}

/*******************************************************************************
 *
 * Function         bta_dm_free_sdp_db
 *
 * Description      Frees SDP data base
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_free_sdp_db(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  osi_free_and_reset((void**)&bta_dm_search_cb.p_sdp_db);
}

/*******************************************************************************
 *
 * Function         bta_dm_queue_search
 *
 * Description      Queues search command while search is being cancelled
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_queue_search(tBTA_DM_MSG* p_data) {
  osi_free(bta_dm_search_cb.p_search_queue);
  bta_dm_search_cb.p_search_queue =
      (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_API_SEARCH));
  memcpy(bta_dm_search_cb.p_search_queue, p_data, sizeof(tBTA_DM_API_SEARCH));
}

/*******************************************************************************
 *
 * Function         bta_dm_queue_disc
 *
 * Description      Queues discovery command while search is being cancelled
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_queue_disc(tBTA_DM_MSG* p_data) {
  osi_free(bta_dm_search_cb.p_search_queue);
  bta_dm_search_cb.p_search_queue =
      (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_API_DISCOVER));
  memcpy(bta_dm_search_cb.p_search_queue, p_data, sizeof(tBTA_DM_API_DISCOVER));
}

/*******************************************************************************
 *
 * Function         bta_dm_search_clear_queue
 *
 * Description      Clears the queue if API search cancel is called
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_clear_queue(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  osi_free_and_reset((void**)&bta_dm_search_cb.p_search_queue);
}

/*******************************************************************************
 *
 * Function         bta_dm_search_cancel_cmpl
 *
 * Description      Search cancel is complete
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_cancel_cmpl(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  if (bta_dm_search_cb.p_search_queue) {
    bta_sys_sendmsg(bta_dm_search_cb.p_search_queue);
    bta_dm_search_cb.p_search_queue = NULL;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_search_cancel_transac_cmpl
 *
 * Description      Current Service Discovery or remote name procedure is
 *                  completed after search cancellation
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_cancel_transac_cmpl(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  osi_free_and_reset((void**)&bta_dm_search_cb.p_sdp_db);
  bta_dm_search_cancel_notify(NULL);
}

/*******************************************************************************
 *
 * Function         bta_dm_search_cancel_notify
 *
 * Description      Notify application that search has been cancelled
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_cancel_notify(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  if (bta_dm_search_cb.p_search_cback) {
    bta_dm_search_cb.p_search_cback(BTA_DM_SEARCH_CANCEL_CMPL_EVT, NULL);
  }
  if (!bta_dm_search_cb.name_discover_done) {
    BTM_CancelRemoteDeviceName();
  }
  if (bta_dm_search_cb.gatt_disc_active) {
    bta_dm_cancel_gatt_discovery(bta_dm_search_cb.peer_bdaddr);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_find_services
 *
 * Description      Starts discovery on a device
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_find_services(const RawAddress& bd_addr) {

  while (bta_dm_search_cb.service_index < BTA_MAX_SERVICE_ID) {
    Uuid uuid = Uuid::kEmpty;
    if (bta_dm_search_cb.services_to_search &
        (tBTA_SERVICE_MASK)(
            BTA_SERVICE_ID_TO_SERVICE_MASK(bta_dm_search_cb.service_index))) {
      bta_dm_search_cb.p_sdp_db =
          (tSDP_DISCOVERY_DB*)osi_malloc(BTA_DM_SDP_DB_SIZE);
      APPL_TRACE_DEBUG("bta_dm_search_cb.services = %04x***********",
                       bta_dm_search_cb.services);
      /* try to search all services by search based on L2CAP UUID */
      if (bta_dm_search_cb.services == BTA_ALL_SERVICE_MASK) {
        LOG_INFO(LOG_TAG, " %s services_to_search= 0x%" PRIx64, __func__,
                bta_dm_search_cb.services_to_search);
        if (bta_dm_search_cb.services_to_search & BTA_RES_SERVICE_MASK) {
          uuid = Uuid::From16Bit(bta_service_id_to_uuid_lkup_tbl[0]);
          bta_dm_search_cb.services_to_search &= ~BTA_RES_SERVICE_MASK;
        } else {
          uuid = Uuid::From16Bit(UUID_PROTOCOL_L2CAP);
          bta_dm_search_cb.services_to_search = 0;
        }
      } else {
        /* for LE only profile */
        if (bta_dm_search_cb.service_index == BTA_BLE_SERVICE_ID) {
          if (bta_dm_search_cb.uuid_to_search > 0 &&
              bta_dm_search_cb.p_srvc_uuid) {
            uuid = *(bta_dm_search_cb.p_srvc_uuid + bta_dm_search_cb.num_uuid -
                     bta_dm_search_cb.uuid_to_search);

            bta_dm_search_cb.uuid_to_search--;
          } else {
            uuid = Uuid::From16Bit(
                bta_service_id_to_uuid_lkup_tbl[bta_dm_search_cb
                                                    .service_index]);
          }

          /* last one? clear the BLE service bit if all discovery has been done
           */
          if (bta_dm_search_cb.uuid_to_search == 0)
            bta_dm_search_cb.services_to_search &=
                (tBTA_SERVICE_MASK)(~(BTA_SERVICE_ID_TO_SERVICE_MASK(
                    bta_dm_search_cb.service_index)));

        } else {
          /* remove the service from services to be searched  */
          bta_dm_search_cb.services_to_search &= (tBTA_SERVICE_MASK)(~(
              BTA_SERVICE_ID_TO_SERVICE_MASK(bta_dm_search_cb.service_index)));
          uuid = Uuid::From16Bit(
              bta_service_id_to_uuid_lkup_tbl[bta_dm_search_cb.service_index]);
        }
      }

      if (bta_dm_search_cb.service_index == BTA_USER_SERVICE_ID) {
        uuid = bta_dm_search_cb.uuid;
      }

      LOG_INFO(LOG_TAG, "%s search UUID = %s", __func__,
               uuid.ToString().c_str());
      SDP_InitDiscoveryDb(bta_dm_search_cb.p_sdp_db, BTA_DM_SDP_DB_SIZE, 1,
                          &uuid, 0, NULL);

      memset(g_disc_raw_data_buf, 0, sizeof(g_disc_raw_data_buf));
      bta_dm_search_cb.p_sdp_db->raw_data = g_disc_raw_data_buf;

      bta_dm_search_cb.p_sdp_db->raw_size = MAX_DISC_RAW_DATA_BUF;

      if (!SDP_ServiceSearchAttributeRequest(bd_addr, bta_dm_search_cb.p_sdp_db,
                                             &bta_dm_sdp_callback)) {
        /*
         * If discovery is not successful with this device, then
         * proceed with the next one.
         */
        osi_free_and_reset((void**)&bta_dm_search_cb.p_sdp_db);
        bta_dm_search_cb.service_index = BTA_MAX_SERVICE_ID;

      } else {
        if (uuid == Uuid::From16Bit(UUID_PROTOCOL_L2CAP)) {
          bool feature = profile_feature_fetch(PBAP_ID, PBAP_0102_SUPPORT);
          LOG_DEBUG(LOG_TAG, " is_pbap_adv_enabled : %d", feature);
          if (feature) {
            LOG_DEBUG(LOG_TAG, "%s SDP search for PBAP Client ", __func__);
            BTA_SdpSearch(bd_addr, Uuid::From16Bit(UUID_SERVCLASS_PBAP_PCE));
          }
        }
        if ((bta_dm_search_cb.service_index == BTA_BLE_SERVICE_ID &&
             bta_dm_search_cb.uuid_to_search == 0) ||
            bta_dm_search_cb.service_index != BTA_BLE_SERVICE_ID)
          bta_dm_search_cb.service_index++;
        return;
      }
    }

    bta_dm_search_cb.service_index++;
  }

  /* no more services to be discovered */
  if (bta_dm_search_cb.service_index >= BTA_MAX_SERVICE_ID) {
    tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));
    /* initialize the data structure - includes p_raw_data and raw_data_size */
    memset(&(p_msg->disc_result.result), 0, sizeof(tBTA_DM_DISC_RES));
    p_msg->hdr.event = BTA_DM_DISCOVERY_RESULT_EVT;
    p_msg->disc_result.result.disc_res.services =
        bta_dm_search_cb.services_found;
    p_msg->disc_result.result.disc_res.bd_addr = bta_dm_search_cb.peer_bdaddr;
    strlcpy((char*)p_msg->disc_result.result.disc_res.bd_name,
            bta_dm_get_remname(), BD_NAME_LEN + 1);

    bta_sys_sendmsg(p_msg);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_discover_next_device
 *
 * Description      Starts discovery on the next device in Inquiry data base
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_discover_next_device(void) {
  APPL_TRACE_DEBUG("bta_dm_discover_next_device");

  /* searching next device on inquiry result */
  bta_dm_search_cb.p_btm_inq_info =
      BTM_InqDbNext(bta_dm_search_cb.p_btm_inq_info);
  if (bta_dm_search_cb.p_btm_inq_info != NULL) {
    bta_dm_search_cb.name_discover_done = false;
    bta_dm_search_cb.peer_name[0] = 0;
    bta_dm_search_cb.transport = BTA_TRANSPORT_UNKNOWN;
    bta_dm_discover_device(
        bta_dm_search_cb.p_btm_inq_info->results.remote_bd_addr);
  } else {
    tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));

    /* no devices, search complete */
    bta_dm_search_cb.services = 0;

    p_msg->hdr.event = BTA_DM_SEARCH_CMPL_EVT;
    p_msg->hdr.layer_specific = BTA_DM_API_DISCOVER_EVT;

    bta_sys_sendmsg(p_msg);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_discover_device
 *
 * Description      Starts name and service discovery on the device
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_discover_device(const RawAddress& remote_bd_addr) {
  tBT_TRANSPORT transport = BT_TRANSPORT_BR_EDR;
  if (bta_dm_search_cb.transport == BTA_TRANSPORT_UNKNOWN) {
    tBT_DEVICE_TYPE dev_type;
    tBLE_ADDR_TYPE addr_type;

    BTM_ReadDevInfo(remote_bd_addr, &dev_type, &addr_type);
    if (dev_type == BT_DEVICE_TYPE_BLE || addr_type == BLE_ADDR_RANDOM)
      transport = BT_TRANSPORT_LE;
  } else {
    transport = bta_dm_search_cb.transport;
  }


  VLOG(1) << __func__ << " BDA: " << remote_bd_addr;

  bta_dm_search_cb.peer_bdaddr = remote_bd_addr;

  APPL_TRACE_DEBUG(
      "%s name_discover_done = %d p_btm_inq_info 0x%x state = %d, transport=%d",
      __func__, bta_dm_search_cb.name_discover_done,
      bta_dm_search_cb.p_btm_inq_info, bta_dm_search_cb.state, transport);

  if (bta_dm_search_cb.p_btm_inq_info) {
    APPL_TRACE_DEBUG("%s appl_knows_rem_name %d", __func__,
                     bta_dm_search_cb.p_btm_inq_info->appl_knows_rem_name);
  }
  if ((bta_dm_search_cb.p_btm_inq_info) &&
      (bta_dm_search_cb.p_btm_inq_info->results.device_type ==
       BT_DEVICE_TYPE_BLE) &&
      (bta_dm_search_cb.state == BTA_DM_SEARCH_ACTIVE)) {
    /* Do not perform RNR for LE devices at inquiry complete*/
    bta_dm_search_cb.name_discover_done = true;
  }
  /* if name discovery is not done and application needs remote name */
  if ((!bta_dm_search_cb.name_discover_done) &&
      ((bta_dm_search_cb.p_btm_inq_info == NULL) ||
       (bta_dm_search_cb.p_btm_inq_info &&
        (!bta_dm_search_cb.p_btm_inq_info->appl_knows_rem_name)))) {
    if (bta_dm_read_remote_device_name(bta_dm_search_cb.peer_bdaddr,
                                       transport) == true) {
      /* Continue the service search on same transport after rnr complete */
      bta_dm_search_cb.transport = transport;
      return;
    }
    /* starting name discovery failed */
    bta_dm_search_cb.name_discover_done = true;
  }

  /* Reset transport state for next discovery */
  bta_dm_search_cb.transport = BTA_TRANSPORT_UNKNOWN;

  /* if application wants to discover service */
  if (bta_dm_search_cb.services) {
    /* initialize variables */
    bta_dm_search_cb.service_index = 0;
    bta_dm_search_cb.services_found = 0;
    bta_dm_search_cb.services_to_search = bta_dm_search_cb.services;
    bta_dm_search_cb.uuid_to_search = bta_dm_search_cb.num_uuid;
    if ((bta_dm_search_cb.p_btm_inq_info != NULL) &&
        bta_dm_search_cb.services != BTA_USER_SERVICE_MASK &&
        (bta_dm_search_cb.sdp_search == false)) {
      /* check if EIR provides the information of supported services */
      bta_dm_eir_search_services(&bta_dm_search_cb.p_btm_inq_info->results,
                                 &bta_dm_search_cb.services_to_search,
                                 &bta_dm_search_cb.services_found);
    }

    /* if seaching with EIR is not completed */
    if (bta_dm_search_cb.services_to_search) {
      /* check whether connection already exists to the device
         if connection exists, we don't have to wait for ACL
         link to go down to start search on next device */
    if (transport == BT_TRANSPORT_BR_EDR) {
      if (BTM_IsAclConnectionUp(bta_dm_search_cb.peer_bdaddr,
                                BT_TRANSPORT_BR_EDR))
        bta_dm_search_cb.wait_disc = false;
      else
        bta_dm_search_cb.wait_disc = true;
    }

      if (bta_dm_search_cb.p_btm_inq_info) {
        APPL_TRACE_DEBUG(
            "%s p_btm_inq_info 0x%x results.device_type 0x%x "
            "services_to_search 0x%x",
            __func__, bta_dm_search_cb.p_btm_inq_info,
            bta_dm_search_cb.p_btm_inq_info->results.device_type,
            bta_dm_search_cb.services_to_search);
      }

      if (transport == BT_TRANSPORT_LE) {
        if (bta_dm_search_cb.services_to_search & BTA_BLE_SERVICE_MASK) {
          // set the raw data buffer here
          memset(g_disc_raw_data_buf, 0, sizeof(g_disc_raw_data_buf));
          bta_dm_search_cb.p_ble_rawdata = g_disc_raw_data_buf;

          bta_dm_search_cb.ble_raw_size = MAX_DISC_RAW_DATA_BUF;
          bta_dm_search_cb.ble_raw_used = 0;

          /* start GATT for service discovery */
          btm_dm_start_gatt_discovery(bta_dm_search_cb.peer_bdaddr);
          return;
        }
      } else {
        bta_dm_search_cb.sdp_results = false;
        bta_dm_find_services(bta_dm_search_cb.peer_bdaddr);
        return;
      }
    }
  }

  /* name discovery and service discovery are done for this device */
  tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));
  p_msg->hdr.event = BTA_DM_DISCOVERY_RESULT_EVT;
  /* initialize the data structure - includes p_raw_data and raw_data_size */
  memset(&(p_msg->disc_result.result), 0, sizeof(tBTA_DM_DISC_RES));
  p_msg->disc_result.result.disc_res.result = BTA_SUCCESS;
  p_msg->disc_result.result.disc_res.services = bta_dm_search_cb.services_found;
  p_msg->disc_result.result.disc_res.bd_addr = bta_dm_search_cb.peer_bdaddr;
  strlcpy((char*)p_msg->disc_result.result.disc_res.bd_name,
          (char*)bta_dm_search_cb.peer_name, BD_NAME_LEN + 1);

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         bta_dm_sdp_callback
 *
 * Description      Callback from sdp with discovery status
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_sdp_callback(uint16_t sdp_status) {
  tBTA_DM_SDP_RESULT* p_msg =
      (tBTA_DM_SDP_RESULT*)osi_malloc(sizeof(tBTA_DM_SDP_RESULT));

  p_msg->hdr.event = BTA_DM_SDP_RESULT_EVT;
  p_msg->sdp_result = sdp_status;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         bta_dm_inq_results_cb
 *
 * Description      Inquiry results callback from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_inq_results_cb(tBTM_INQ_RESULTS* p_inq, uint8_t* p_eir,
                                  uint16_t eir_len) {
  tBTA_DM_SEARCH result;
  tBTM_INQ_INFO* p_inq_info;
  uint16_t service_class;

  result.inq_res.bd_addr = p_inq->remote_bd_addr;
  memcpy(result.inq_res.dev_class, p_inq->dev_class, DEV_CLASS_LEN);
  BTM_COD_SERVICE_CLASS(service_class, p_inq->dev_class);
  result.inq_res.is_limited =
      (service_class & BTM_COD_SERVICE_LMTD_DISCOVER) ? true : false;
  result.inq_res.rssi = p_inq->rssi;

  result.inq_res.ble_addr_type = p_inq->ble_addr_type;
  result.inq_res.inq_result_type = p_inq->inq_result_type;
  result.inq_res.device_type = p_inq->device_type;
  result.inq_res.flag = p_inq->flag;

  /* application will parse EIR to find out remote device name */
  result.inq_res.p_eir = p_eir;
  result.inq_res.eir_len = eir_len;

  p_inq_info = BTM_InqDbRead(p_inq->remote_bd_addr);
  if (p_inq_info != NULL) {
    /* initialize remt_name_not_required to false so that we get the name by
     * default */
    result.inq_res.remt_name_not_required = false;
  }

  if (bta_dm_search_cb.p_search_cback)
    bta_dm_search_cb.p_search_cback(BTA_DM_INQ_RES_EVT, &result);

  if (p_inq_info) {
    /* application indicates if it knows the remote name, inside the callback
     copy that to the inquiry data base*/
    if (result.inq_res.remt_name_not_required)
      p_inq_info->appl_knows_rem_name = true;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_inq_cmpl_cb
 *
 * Description      Inquiry complete callback from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_inq_cmpl_cb(void* p_result) {
  tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));

  APPL_TRACE_DEBUG("%s", __func__);

  if (!bta_dm_search_cb.cancel_pending) {
    p_msg->inq_cmpl.hdr.event = BTA_DM_INQUIRY_CMPL_EVT;
    p_msg->inq_cmpl.num = ((tBTM_INQUIRY_CMPL*)p_result)->num_resp;
  } else {
    bta_dm_search_cb.cancel_pending = false;
    bta_dm_search_cancel_notify(NULL);
    p_msg->hdr.event = BTA_DM_SEARCH_CMPL_EVT;
    p_msg->hdr.layer_specific = BTA_DM_API_DISCOVER_EVT;
  }

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
**
** Function         bta_dm_rem_name_cback
**
** Description      Remote name call back from BTM when remote name is retrieved successfully
**
** Returns          void
**
*******************************************************************************/
static void bta_dm_rem_name_cback (const RawAddress& bd_addr, DEV_CLASS dc, BD_NAME bd_name)
{
  tBTA_DM_SEC sec_event;

  APPL_TRACE_DEBUG("bta_dm_rem_name_cback name=<%s>", bd_name);

  if (strlen((char*)bd_name) > (BD_NAME_LEN)){
    sec_event.rem_name_evt.bd_name[(BD_NAME_LEN)] = 0;
  }
  sec_event.rem_name_evt.bd_addr = bd_addr;
  strlcpy((char*)sec_event.rem_name_evt.bd_name, (char*)bd_name, BD_NAME_LEN + 1);
  if(bta_dm_cb.p_sec_cback){
    bta_dm_cb.p_sec_cback(BTA_DM_REM_NAME_EVT, &sec_event);
  }
}


/*******************************************************************************
 *
 * Function         bta_dm_service_search_remname_cback
 *
 * Description      Remote name call back from BTM during service discovery
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_service_search_remname_cback(const RawAddress& bd_addr,
                                                UNUSED_ATTR DEV_CLASS dc,
                                                BD_NAME bd_name) {
  tBTM_REMOTE_DEV_NAME rem_name;
  tBTM_STATUS btm_status;

  APPL_TRACE_DEBUG("%s name=<%s>", __func__, bd_name);

  /* if this is what we are looking for */
  if (bta_dm_search_cb.peer_bdaddr == bd_addr) {
    rem_name.length = strlen((char*)bd_name);
    if (rem_name.length > BD_NAME_LEN) {
      rem_name.length = BD_NAME_LEN;
      rem_name.remote_bd_name[BD_NAME_LEN] = 0;
    }
    strlcpy((char*)rem_name.remote_bd_name, (char*)bd_name, BD_NAME_LEN + 1);
    rem_name.status = BTM_SUCCESS;

    bta_dm_remname_cback(&rem_name);
  } else {
    /* get name of device */
    btm_status = BTM_ReadRemoteDeviceName(bta_dm_search_cb.peer_bdaddr,
                                          (tBTM_CMPL_CB*)bta_dm_remname_cback,
                                          BT_TRANSPORT_BR_EDR);
    if (btm_status == BTM_BUSY) {
      /* wait for next chance(notification of remote name discovery done) */
      APPL_TRACE_DEBUG("%s: BTM_ReadRemoteDeviceName is busy", __func__);
    } else if (btm_status != BTM_CMD_STARTED) {
      /* if failed to start getting remote name then continue */
      APPL_TRACE_WARNING("%s: BTM_ReadRemoteDeviceName returns 0x%02X",
                         __func__, btm_status);

      rem_name.length = 0;
      rem_name.remote_bd_name[0] = 0;
      rem_name.status = btm_status;
      bta_dm_remname_cback(&rem_name);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_remname_cback
 *
 * Description      Remote name complete call back from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_remname_cback(void* p) {
  tBTM_REMOTE_DEV_NAME* p_remote_name = (tBTM_REMOTE_DEV_NAME*)p;
  APPL_TRACE_DEBUG("bta_dm_remname_cback len = %d name=<%s>",
                   p_remote_name->length, p_remote_name->remote_bd_name);

  if ((p_remote_name->bd_addr != RawAddress::kEmpty) &&
      (p_remote_name->bd_addr != bta_dm_search_cb.peer_bdaddr)) {
    VLOG(1) << "bta_dm_remname_cback ,rnr complete for diff device,return"
    << " search_cb.peer_dbaddr:" << bta_dm_search_cb.peer_bdaddr
    << " p_remote_name_bda=" << p_remote_name->bd_addr;
    return;
  }
  /* remote name discovery is done but it could be failed */
  bta_dm_search_cb.name_discover_done = true;
  strlcpy((char*)bta_dm_search_cb.peer_name,
          (char*)p_remote_name->remote_bd_name, BD_NAME_LEN + 1);

  BTM_SecDeleteRmtNameNotifyCallback(&bta_dm_service_search_remname_cback);

  if (bta_dm_search_cb.transport == BT_TRANSPORT_LE) {
    GAP_BleReadPeerPrefConnParams(bta_dm_search_cb.peer_bdaddr);
  }

  tBTA_DM_REM_NAME* p_msg =
      (tBTA_DM_REM_NAME*)osi_malloc(sizeof(tBTA_DM_REM_NAME));
  p_msg->result.disc_res.bd_addr = bta_dm_search_cb.peer_bdaddr;
  strlcpy((char*)p_msg->result.disc_res.bd_name,
          (char*)p_remote_name->remote_bd_name, BD_NAME_LEN + 1);
  p_msg->hdr.event = BTA_DM_REMT_NAME_EVT;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         bta_dm_authorize_cback
 *
 * Description      cback requesting authorization
 *
 * Returns          void
 *
 ******************************************************************************/
static uint8_t bta_dm_authorize_cback(const RawAddress& bd_addr,
                                      DEV_CLASS dev_class, BD_NAME bd_name,
                                      UNUSED_ATTR uint8_t* service_name,
                                      uint8_t service_id,
                                      UNUSED_ATTR bool is_originator) {
  tBTA_DM_SEC sec_event;
  uint8_t index = 1;

  sec_event.authorize.bd_addr = bd_addr;
  memcpy(sec_event.authorize.dev_class, dev_class, DEV_CLASS_LEN);
  strlcpy((char*)sec_event.authorize.bd_name, (char*)bd_name, BD_NAME_LEN + 1);

#if (BTA_JV_INCLUDED == TRUE)
  sec_event.authorize.service = service_id;
#endif

  while (index < BTA_MAX_SERVICE_ID) {
    /* get the BTA service id corresponding to BTM id */
    if (bta_service_id_to_btm_srv_id_lkup_tbl[index] == service_id) {
      sec_event.authorize.service = index;
      break;
    }
    index++;
  }

  /* if supported service callback otherwise not authorized */
  if (bta_dm_cb.p_sec_cback && (index < BTA_MAX_SERVICE_ID
#if (BTA_JV_INCLUDED == TRUE)
                                /* pass through JV service ID */
                                || (service_id >= BTA_FIRST_JV_SERVICE_ID &&
                                    service_id <= BTA_LAST_JV_SERVICE_ID)
#endif
                                    )) {
    bta_dm_cb.p_sec_cback(BTA_DM_AUTHORIZE_EVT, &sec_event);
    return BTM_CMD_STARTED;
  } else {
    return BTM_NOT_AUTHORIZED;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_pinname_cback
 *
 * Description      Callback requesting pin_key
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_pinname_cback(void* p_data) {
  tBTM_REMOTE_DEV_NAME* p_result = (tBTM_REMOTE_DEV_NAME*)p_data;
  tBTA_DM_SEC sec_event;
  uint32_t bytes_to_copy;
  tBTA_DM_SEC_EVT event = bta_dm_cb.pin_evt;

  if (BTA_DM_SP_CFM_REQ_EVT == event) {
    /* Retrieved saved device class and bd_addr */
    sec_event.cfm_req.bd_addr = bta_dm_cb.pin_bd_addr;
    BTA_COPY_DEVICE_CLASS(sec_event.cfm_req.dev_class, bta_dm_cb.pin_dev_class);

    if (p_result && p_result->status == BTM_SUCCESS) {
      bytes_to_copy = (p_result->length < BD_NAME_LEN)
                          ? p_result->length
                          : BD_NAME_LEN;
      memcpy(sec_event.cfm_req.bd_name, p_result->remote_bd_name,
             bytes_to_copy);
      sec_event.pin_req.bd_name[BD_NAME_LEN] = 0;
    } else /* No name found */
      sec_event.cfm_req.bd_name[0] = 0;

    sec_event.key_notif.passkey =
        bta_dm_cb.num_val; /* get PIN code numeric number */

    /* 1 additional event data fields for this event */
    sec_event.cfm_req.just_works = bta_dm_cb.just_works;

    /* retrieve the loc and rmt caps */
    sec_event.cfm_req.loc_io_caps = bta_dm_cb.loc_io_caps;
    sec_event.cfm_req.rmt_io_caps = bta_dm_cb.rmt_io_caps;
    sec_event.cfm_req.loc_auth_req = bta_dm_cb.loc_auth_req;
    sec_event.cfm_req.rmt_auth_req = bta_dm_cb.rmt_auth_req;
  } else {
    /* Retrieved saved device class and bd_addr */
    sec_event.pin_req.bd_addr = bta_dm_cb.pin_bd_addr;
    BTA_COPY_DEVICE_CLASS(sec_event.pin_req.dev_class, bta_dm_cb.pin_dev_class);

    if (p_result && p_result->status == BTM_SUCCESS) {
      bytes_to_copy = (p_result->length < BD_NAME_LEN)
                          ? p_result->length
                          : BD_NAME_LEN;
      memcpy(sec_event.pin_req.bd_name, p_result->remote_bd_name,
             bytes_to_copy);
      sec_event.pin_req.bd_name[BD_NAME_LEN] = 0;
    } else /* No name found */
      sec_event.pin_req.bd_name[0] = 0;

    event = bta_dm_cb.pin_evt;
    sec_event.key_notif.passkey =
        bta_dm_cb.num_val; /* get PIN code numeric number */
  }

  if (bta_dm_cb.p_sec_cback) bta_dm_cb.p_sec_cback(event, &sec_event);
}

/*******************************************************************************
 *
 * Function         bta_dm_pin_cback
 *
 * Description      Callback requesting pin_key
 *
 * Returns          void
 *
 ******************************************************************************/
static uint8_t bta_dm_pin_cback(const RawAddress& bd_addr, DEV_CLASS dev_class,
                                BD_NAME bd_name, bool min_16_digit) {
  tBTA_DM_SEC sec_event;

  if (!bta_dm_cb.p_sec_cback) return BTM_NOT_AUTHORIZED;

  /* If the device name is not known, save bdaddr and devclass and initiate a
   * name request */
  if (bd_name[0] == 0) {
    bta_dm_cb.pin_evt = BTA_DM_PIN_REQ_EVT;
    bta_dm_cb.pin_bd_addr = bd_addr;
    BTA_COPY_DEVICE_CLASS(bta_dm_cb.pin_dev_class, dev_class);
    if ((BTM_ReadRemoteDeviceName(bd_addr, bta_dm_pinname_cback,
                                  BT_TRANSPORT_BR_EDR)) == BTM_CMD_STARTED)
      return BTM_CMD_STARTED;

    APPL_TRACE_WARNING(
        " bta_dm_pin_cback() -> Failed to start Remote Name Request  ");
  }

  sec_event.pin_req.bd_addr = bd_addr;
  BTA_COPY_DEVICE_CLASS(sec_event.pin_req.dev_class, dev_class);
  strlcpy((char*)sec_event.pin_req.bd_name, (char*)bd_name, BD_NAME_LEN + 1);
  sec_event.pin_req.min_16_digit = min_16_digit;

  bta_dm_cb.p_sec_cback(BTA_DM_PIN_REQ_EVT, &sec_event);
  return BTM_CMD_STARTED;
}

/*******************************************************************************
 *
 * Function         bta_dm_new_link_key_cback
 *
 * Description      Callback from BTM to notify new link key
 *
 * Returns          void
 *
 ******************************************************************************/
static uint8_t bta_dm_new_link_key_cback(const RawAddress& bd_addr,
                                         UNUSED_ATTR DEV_CLASS dev_class,
                                         BD_NAME bd_name, const LinkKey& key,
                                         uint8_t key_type) {
  tBTA_DM_SEC sec_event;
  tBTA_DM_AUTH_CMPL* p_auth_cmpl;
  uint8_t event;

  memset(&sec_event, 0, sizeof(tBTA_DM_SEC));

  /* Not AMP Key type */
  if (key_type != HCI_LKEY_TYPE_AMP_WIFI && key_type != HCI_LKEY_TYPE_AMP_UWB) {
    event = BTA_DM_AUTH_CMPL_EVT;
    p_auth_cmpl = &sec_event.auth_cmpl;

    p_auth_cmpl->bd_addr = bd_addr;

    memcpy(p_auth_cmpl->bd_name, bd_name, BD_NAME_LEN);
    p_auth_cmpl->bd_name[BD_NAME_LEN] = 0;

    p_auth_cmpl->key_present = true;
    p_auth_cmpl->key_type = key_type;
    p_auth_cmpl->success = true;
    p_auth_cmpl->key = key;
    sec_event.auth_cmpl.fail_reason = HCI_SUCCESS;

    // Report the BR link key based on the BR/EDR address and type
    BTM_ReadDevInfo(bd_addr, &sec_event.auth_cmpl.dev_type,
                    &sec_event.auth_cmpl.addr_type);
    if (bta_dm_cb.p_sec_cback) bta_dm_cb.p_sec_cback(event, &sec_event);

    // Setting remove_dev_pending flag to false, where it will avoid deleting
    // the
    // security device record when the ACL connection link goes down in case of
    // reconnection.
    if (bta_dm_cb.device_list.count)
      bta_dm_reset_sec_dev_pending(p_auth_cmpl->bd_addr);
  } else {
    APPL_TRACE_WARNING("%s() Received AMP Key", __func__);
  }

  return BTM_CMD_STARTED;
}

/*******************************************************************************
 *
 * Function         bta_dm_authentication_complete_cback
 *
 * Description      Authentication complete callback from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
static uint8_t bta_dm_authentication_complete_cback(
    const RawAddress& bd_addr, UNUSED_ATTR DEV_CLASS dev_class, BD_NAME bd_name,
    int result) {
  tBTA_DM_SEC sec_event;

  if (result != BTM_SUCCESS) {
    memset(&sec_event, 0, sizeof(tBTA_DM_SEC));
    sec_event.auth_cmpl.bd_addr = bd_addr;

    memcpy(sec_event.auth_cmpl.bd_name, bd_name, BD_NAME_LEN);
    sec_event.auth_cmpl.bd_name[BD_NAME_LEN] = 0;
    sec_event.auth_cmpl.is_sm4_dev = btm_is_sm4_dev(bd_addr);

    // Report the BR link key based on the BR/EDR address and type
    BTM_ReadDevInfo(bd_addr, &sec_event.auth_cmpl.dev_type,
                    &sec_event.auth_cmpl.addr_type);
    sec_event.auth_cmpl.fail_reason = (uint8_t)result;

    if (bta_dm_cb.p_sec_cback)
      bta_dm_cb.p_sec_cback(BTA_DM_AUTH_CMPL_EVT, &sec_event);

    if ((result == HCI_ERR_KEY_MISSING) && btm_is_sm4_dev(bd_addr)) {
      APPL_TRACE_WARNING(
              "bta_dm_authentication_complete_cback: sm4 device dont delete security record");
    } else if (result == HCI_ERR_AUTH_FAILURE || result == HCI_ERR_KEY_MISSING ||
        result == HCI_ERR_HOST_REJECT_SECURITY ||
        result == HCI_ERR_ENCRY_MODE_NOT_ACCEPTABLE) {
      APPL_TRACE_WARNING("%s deleting %s - result: 0x%02x", __func__,
                         bd_addr.ToString().c_str(), result);
      bta_dm_remove_sec_dev_entry(bd_addr);
    }
  }

  return BTM_SUCCESS;
}

#if (BTM_LOCAL_IO_CAPS != BTM_IO_CAP_NONE)
/*******************************************************************************
 *
 * Function         bta_dm_sp_cback
 *
 * Description      simple pairing callback from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
static uint8_t bta_dm_sp_cback(tBTM_SP_EVT event, tBTM_SP_EVT_DATA* p_data) {
  tBTM_STATUS status = BTM_CMD_STARTED;
  tBTA_DM_SEC sec_event;
  tBTA_DM_SEC_EVT pin_evt = BTA_DM_SP_KEY_NOTIF_EVT;

  APPL_TRACE_EVENT("bta_dm_sp_cback: %d", event);
  if (!bta_dm_cb.p_sec_cback) return BTM_NOT_AUTHORIZED;

  /* TODO_SP */
  switch (event) {
    case BTM_SP_IO_REQ_EVT:
      if (btm_local_io_caps != BTM_IO_CAP_NONE) {
        /* translate auth_req */
        bta_dm_co_io_req(p_data->io_req.bd_addr, &p_data->io_req.io_cap,
                         &p_data->io_req.oob_data, &p_data->io_req.auth_req,
                         p_data->io_req.is_orig);
      }
      APPL_TRACE_EVENT("io mitm: %d oob_data:%d", p_data->io_req.auth_req,
                       p_data->io_req.oob_data);
      break;
    case BTM_SP_IO_RSP_EVT:
      if (btm_local_io_caps != BTM_IO_CAP_NONE) {
        bta_dm_co_io_rsp(p_data->io_rsp.bd_addr, p_data->io_rsp.io_cap,
                         p_data->io_rsp.oob_data, p_data->io_rsp.auth_req);
      }
      break;

    case BTM_SP_CFM_REQ_EVT:
      pin_evt = BTA_DM_SP_CFM_REQ_EVT;
      bta_dm_cb.just_works = sec_event.cfm_req.just_works =
          p_data->cfm_req.just_works;
      sec_event.cfm_req.loc_auth_req = p_data->cfm_req.loc_auth_req;
      sec_event.cfm_req.rmt_auth_req = p_data->cfm_req.rmt_auth_req;
      sec_event.cfm_req.loc_io_caps = p_data->cfm_req.loc_io_caps;
      sec_event.cfm_req.rmt_io_caps = p_data->cfm_req.rmt_io_caps;

    /* continue to next case */
    /* Passkey entry mode, mobile device with output capability is very
        unlikely to receive key request, so skip this event */
    /*case BTM_SP_KEY_REQ_EVT: */
      FALLTHROUGH;
    case BTM_SP_KEY_NOTIF_EVT:
      if (btm_local_io_caps == BTM_IO_CAP_NONE &&
          BTM_SP_KEY_NOTIF_EVT == event) {
        status = BTM_NOT_AUTHORIZED;
        break;
      }

      bta_dm_cb.num_val = sec_event.key_notif.passkey =
          p_data->key_notif.passkey;

      if (BTM_SP_CFM_REQ_EVT == event) {
        bta_dm_cb.num_val = sec_event.key_notif.passkey = p_data->cfm_req.num_val;
        /* Due to the switch case falling through below to BTM_SP_KEY_NOTIF_EVT,
           call remote name request using values from cfm_req */
        if (p_data->cfm_req.bd_name[0] == 0) {
          bta_dm_cb.pin_evt = pin_evt;
          bta_dm_cb.pin_bd_addr = p_data->cfm_req.bd_addr;
          bta_dm_cb.rmt_io_caps = sec_event.cfm_req.rmt_io_caps;
          bta_dm_cb.loc_io_caps = sec_event.cfm_req.loc_io_caps;
          bta_dm_cb.rmt_auth_req = sec_event.cfm_req.rmt_auth_req;
          bta_dm_cb.loc_auth_req = sec_event.cfm_req.loc_auth_req;
          BTA_COPY_DEVICE_CLASS(bta_dm_cb.pin_dev_class,
                                p_data->cfm_req.dev_class);
          if ((BTM_ReadRemoteDeviceName(
                  p_data->cfm_req.bd_addr, bta_dm_pinname_cback,
                  BT_TRANSPORT_BR_EDR)) == BTM_CMD_STARTED)
            return BTM_CMD_STARTED;
          APPL_TRACE_WARNING(
              " bta_dm_sp_cback() -> Failed to start Remote Name Request  ");
        } else {
          /* Due to the switch case falling through below to
             BTM_SP_KEY_NOTIF_EVT,
             copy these values into key_notif from cfm_req */
          sec_event.key_notif.bd_addr = p_data->cfm_req.bd_addr;
          BTA_COPY_DEVICE_CLASS(sec_event.key_notif.dev_class,
                                p_data->cfm_req.dev_class);
          strlcpy((char*)sec_event.key_notif.bd_name,
                  (char*)p_data->cfm_req.bd_name, BD_NAME_LEN + 1);
        }
      }

      if (BTM_SP_KEY_NOTIF_EVT == event) {
        bta_dm_cb.num_val = sec_event.key_notif.passkey = p_data->key_notif.passkey;
        /* If the device name is not known, save bdaddr and devclass
           and initiate a name request with values from key_notif */
        if (p_data->key_notif.bd_name[0] == 0) {
          bta_dm_cb.pin_evt = pin_evt;
          /* Store the local and remote io caps */
          bta_dm_cb.loc_io_caps = sec_event.cfm_req.loc_io_caps;
          bta_dm_cb.rmt_io_caps = sec_event.cfm_req.rmt_io_caps;
          bta_dm_cb.pin_bd_addr = p_data->key_notif.bd_addr;
          BTA_COPY_DEVICE_CLASS(bta_dm_cb.pin_dev_class,
                                p_data->key_notif.dev_class);
          if ((BTM_ReadRemoteDeviceName(
                  p_data->key_notif.bd_addr, bta_dm_pinname_cback,
                  BT_TRANSPORT_BR_EDR)) == BTM_CMD_STARTED)
            return BTM_CMD_STARTED;
          APPL_TRACE_WARNING(
              " bta_dm_sp_cback() -> Failed to start Remote Name Request  ");
        } else {
          sec_event.key_notif.bd_addr = p_data->key_notif.bd_addr;
          BTA_COPY_DEVICE_CLASS(sec_event.key_notif.dev_class,
                                p_data->key_notif.dev_class);
          strlcpy((char*)sec_event.key_notif.bd_name,
                  (char*)p_data->key_notif.bd_name, BD_NAME_LEN + 1);
          sec_event.key_notif.bd_name[BD_NAME_LEN] = 0;
        }
      }

      bta_dm_cb.p_sec_cback(pin_evt, &sec_event);

      break;

    case BTM_SP_LOC_OOB_EVT:
      bta_dm_co_loc_oob((bool)(p_data->loc_oob.status == BTM_SUCCESS),
                        p_data->loc_oob.c, p_data->loc_oob.r);
      break;

    case BTM_SP_RMT_OOB_EVT:
      /* If the device name is not known, save bdaddr and devclass and initiate
       * a name request */
      if (p_data->rmt_oob.bd_name[0] == 0) {
        bta_dm_cb.pin_evt = BTA_DM_SP_RMT_OOB_EVT;
        bta_dm_cb.pin_bd_addr = p_data->rmt_oob.bd_addr;
        BTA_COPY_DEVICE_CLASS(bta_dm_cb.pin_dev_class,
                              p_data->rmt_oob.dev_class);
        if ((BTM_ReadRemoteDeviceName(p_data->rmt_oob.bd_addr,
                                      bta_dm_pinname_cback,
                                      BT_TRANSPORT_BR_EDR)) == BTM_CMD_STARTED)
          return BTM_CMD_STARTED;
        APPL_TRACE_WARNING(
            " bta_dm_sp_cback() -> Failed to start Remote Name Request  ");
      }

      sec_event.rmt_oob.bd_addr = p_data->rmt_oob.bd_addr;
      BTA_COPY_DEVICE_CLASS(sec_event.rmt_oob.dev_class,
                            p_data->rmt_oob.dev_class);
      strlcpy((char*)sec_event.rmt_oob.bd_name, (char*)p_data->rmt_oob.bd_name,
              BD_NAME_LEN + 1);

      bta_dm_cb.p_sec_cback(BTA_DM_SP_RMT_OOB_EVT, &sec_event);

      bta_dm_co_rmt_oob(p_data->rmt_oob.bd_addr);
      break;

    case BTM_SP_COMPLT_EVT:
      /* do not report this event - handled by link_key_callback or
       * auth_complete_callback */
      break;

    case BTM_SP_KEYPRESS_EVT:
      memcpy(&sec_event.key_press, &p_data->key_press,
             sizeof(tBTM_SP_KEYPRESS));
      bta_dm_cb.p_sec_cback(BTA_DM_SP_KEYPRESS_EVT, &sec_event);
      break;

    case BTM_SP_UPGRADE_EVT:
      bta_dm_co_lk_upgrade(p_data->upgrade.bd_addr, &p_data->upgrade.upgrade);
      break;

    default:
      status = BTM_NOT_AUTHORIZED;
      break;
  }
  APPL_TRACE_EVENT("dm status: %d", status);
  return status;
}
#endif

/*******************************************************************************
 *
 * Function         bta_dm_local_name_cback
 *
 * Description      Callback from btm after local name is read
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_local_name_cback(UNUSED_ATTR void * p_name) {
  tBTA_DM_SEC sec_event;

  sec_event.enable.status = BTA_SUCCESS;

  if (bta_dm_cb.p_sec_cback)
    bta_dm_cb.p_sec_cback(BTA_DM_ENABLE_EVT, &sec_event);
}

/*******************************************************************************
 *
 * Function         bta_dm_bl_change_cback
 *
 * Description      Callback from btm when acl connection goes up or down
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_bl_change_cback(tBTM_BL_EVENT_DATA* p_data) {
  tBTA_DM_ACL_CHANGE* p_msg =
      (tBTA_DM_ACL_CHANGE*)osi_malloc(sizeof(tBTA_DM_ACL_CHANGE));

  p_msg->event = p_data->event;
  p_msg->is_new = false;

  switch (p_msg->event) {
    case BTM_BL_CONN_EVT:
      p_msg->is_new = true;
      p_msg->bd_addr = *p_data->conn.p_bda;
      p_msg->transport = p_data->conn.transport;
      p_msg->handle = p_data->conn.handle;
      break;
    case BTM_BL_DISCN_EVT:
      p_msg->bd_addr = *p_data->discn.p_bda;
      p_msg->transport = p_data->discn.transport;
      p_msg->handle = p_data->discn.handle;
      break;
    case BTM_BL_UPDATE_EVT:
      p_msg->busy_level = p_data->update.busy_level;
      p_msg->busy_level_flags = p_data->update.busy_level_flags;
      break;
    case BTM_BL_ROLE_CHG_EVT:
      p_msg->new_role = p_data->role_chg.new_role;
      p_msg->hci_status = p_data->role_chg.hci_status;
      p_msg->bd_addr = *p_data->role_chg.p_bda;
      break;
    case BTM_BL_PKT_TYPE_CHG_EVT:
      p_msg->pkt_type = p_data->pkt_type_chg.pkt_type;
      p_msg->bd_addr = p_data->pkt_type_chg.remote_bd_addr;
      break;
    case BTM_BL_SOC_LOGGING_EVT:
      p_msg->soc_log_id = p_data->soc_logging.soc_log_id;
      break;
    case BTM_BL_COLLISION_EVT:
      p_msg->bd_addr = *p_data->conn.p_bda;
      break;
  }

  p_msg->hdr.event = BTA_DM_ACL_CHANGE_EVT;
  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
**
** Function         vnd_get_error_info
**
** Description      Returns error info mask based on soc error type sent by SoC
**
**
** Returns          uint16_t
**
*******************************************************************************/
uint16_t vnd_get_error_info(uint16_t soc_error_type) {
  switch(soc_error_type) {
    case SOC_ERROR_PAGE_TIMEOUT:
      return SOC_PAGE_TIMEOUT;
    case SOC_ERROR_HALF_LSTO:
      return SOC_HALF_LSTO;
    case SOC_ERROR_AUDIO_GLITCH:
      return SOC_A2DP_TRANSMISSION_DELAY;
    case SOC_ERROR_SCO_MISSES:
      return SOC_SCO_MISSES;
    case SOC_ERROR_LSTO:
      return SOC_LSTO;
    case SOC_ERROR_CONN_FAIL:
      return SOC_CONN_FAIL;
    default:
      return 0;
  }
}

/*******************************************************************************
**
** Function         vnd_map_soc_error_type(uint16_t soc_error_type)
**
** Description      Map tSOC_ERROR_TYPE to tBT_ERROR_TYPE
**
**
** Returns          uint16_t
**
*******************************************************************************/
uint16_t vnd_map_soc_error_type(uint16_t soc_error_type) {
  switch(soc_error_type) {
    case SOC_ERROR_PAGE_TIMEOUT:
    case SOC_ERROR_CONN_FAIL:
      return BT_SOC_CONNECTION_FAIL;
    case SOC_ERROR_HALF_LSTO:
    case SOC_ERROR_LSTO:
      return BT_SOC_DISCONNECTION;
    case SOC_ERROR_AUDIO_GLITCH:
      return BT_SOC_A2DP_GLITCH;
    case SOC_ERROR_SCO_MISSES:
      return BT_SOC_VOICE_BREAK;

    default:
      return -1;
  }
}


/*******************************************************************************
**
** Function         bta_dm_vnd_info_report_cback
**
** Description      Called from btm when SoC sends IOT device info report
**
**
** Returns          void
**
*******************************************************************************/
static void bta_dm_vnd_info_report_cback (uint8_t evt_len, uint8_t *p_data) {
  uint8_t iot_info_len;
  uint8_t *p_iot_info;
  uint16_t soc_error_type;
  APPL_TRACE_DEBUG("bta_dm_vnd_info_report_cback");
  tBTA_DM_VND_IOT_REPORT *p_msg =
      (tBTA_DM_VND_IOT_REPORT *)osi_calloc(sizeof(tBTA_DM_VND_IOT_REPORT));

  p_msg->hdr.event = BTA_DM_API_IOT_REPORT_EVT;
  STREAM_TO_UINT16(soc_error_type, p_data);
  p_msg->error_type = vnd_map_soc_error_type(soc_error_type);
  STREAM_TO_UINT8(iot_info_len, p_data);
  p_iot_info = p_data;
  STREAM_TO_BDADDR(p_msg->bd_addr, p_data);
  STREAM_SKIP_UINT16(p_data); // currently connection handle is not required.
  STREAM_TO_UINT32(p_msg->event_mask, p_data);
  STREAM_TO_UINT8(p_msg->event_power_level, p_data);
  STREAM_TO_INT8(p_msg->event_rssi, p_data);
  // for SOC_ERROR_CONN_FAIL error, set event_mask as fail reason.
  if (soc_error_type == SOC_ERROR_CONN_FAIL) {
    STREAM_TO_UINT8(p_msg->event_mask, p_data);
  }

  if (p_msg->error_type == BT_SOC_A2DP_GLITCH) {
    STREAM_TO_UINT8(p_msg->event_link_quality, p_data);
    STREAM_SKIP_UINT16(p_data); // currently overflow count is not required.
  } else {
    p_msg->event_link_quality = 0xFF;
  }

  if ((p_msg->error_type == BT_SOC_A2DP_GLITCH
      || p_msg->error_type == BT_SOC_VOICE_BREAK)
      && ((int)iot_info_len - (p_data - p_iot_info) >= 2)) {
    STREAM_TO_UINT16(p_msg->event_glitch_count, p_data);
  }
  p_msg->error_info = vnd_get_error_info(soc_error_type);

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         bta_dm_rs_cback
 *
 * Description      Receives the role switch complete event
 *
 * Returns
 *
 ******************************************************************************/
static void bta_dm_rs_cback(UNUSED_ATTR void* p1) {
  APPL_TRACE_WARNING("bta_dm_rs_cback:%d", bta_dm_cb.rs_event);
  if (bta_dm_cb.rs_event == BTA_DM_API_SEARCH_EVT) {
    bta_dm_cb.search_msg.rs_res =
        BTA_DM_RS_OK; /* do not care about the result for now */
    bta_dm_cb.rs_event = 0;
    bta_dm_search_start((tBTA_DM_MSG*)&bta_dm_cb.search_msg);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_check_av
 *
 * Description      This function checks if AV is active
 *                  if yes, make sure the AV link is master
 *
 * Returns          bool - true, if switch is in progress
 *
 ******************************************************************************/
static bool bta_dm_check_av(uint16_t event) {
  bool avoid_roleswitch = false;
  bool switching = false;
  uint8_t i;
  tBTA_DM_PEER_DEVICE* p_dev;

#if (BTA_DM_AVOID_A2DP_ROLESWITCH_ON_INQUIRY == TRUE)

  /* avoid role switch upon inquiry if a2dp is actively streaming as it
     introduces an audioglitch due to FW scheduling delays (unavoidable) */
  if (event == BTA_DM_API_SEARCH_EVT) {
    avoid_roleswitch = true;
  }
#endif

  APPL_TRACE_WARNING("bta_dm_check_av:%d", bta_dm_cb.cur_av_count);
  if (bta_dm_cb.cur_av_count) {
    for (i = 0; i < bta_dm_cb.device_list.count; i++) {
      p_dev = &bta_dm_cb.device_list.peer_device[i];
      APPL_TRACE_WARNING("[%d]: state:%d, info:x%x, avoid_rs %d", i,
                         p_dev->conn_state, p_dev->info, avoid_roleswitch);
      if ((p_dev->conn_state == BTA_DM_CONNECTED) &&
          (p_dev->info & BTA_DM_DI_AV_ACTIVE) && (!avoid_roleswitch)) {
        /* make master and take away the role switch policy */
        if (BTM_CMD_STARTED == BTM_SwitchRole(p_dev->peer_bdaddr,
                                              HCI_ROLE_MASTER,
                                              (tBTM_CMPL_CB*)bta_dm_rs_cback)) {
          /* the role switch command is actually sent */
          bta_dm_cb.rs_event = event;
          switching = true;
        }
        /* else either already master or can not switch for some reasons */
        bta_dm_policy_cback(BTA_SYS_PLCY_CLR, 0, HCI_ENABLE_MASTER_SLAVE_SWITCH,
                            p_dev->peer_bdaddr);
        break;
      }
    }
  }
  return switching;
}

/*******************************************************************************
 *
 * Function         bta_dm_acl_change
 *
 * Description      Process BTA_DM_ACL_CHANGE_EVT
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_acl_change(tBTA_DM_MSG* p_data) {
  uint8_t i;
  uint8_t* p;
  tBTA_DM_SEC conn;
  bool is_new = p_data->acl_change.is_new;
  const RawAddress& p_bda = p_data->acl_change.bd_addr;
  bool need_policy_change = false;
  bool issue_unpair_cb = false;

  tBTA_DM_PEER_DEVICE* p_dev;
  memset(&conn, 0, sizeof(tBTA_DM_SEC));

  if(!bta_dm_cb.is_bta_dm_active) {
    APPL_TRACE_ERROR("%s Ignore event: %d, bta dm is not in initialized state.",
            __func__, p_data->acl_change.event);
    return;
  }

  switch (p_data->acl_change.event) {
    case BTM_BL_UPDATE_EVT: /* busy level update */
      if (bta_dm_cb.p_sec_cback) {
        conn.busy_level.level = p_data->acl_change.busy_level;
        conn.busy_level.level_flags = p_data->acl_change.busy_level_flags;
        bta_dm_cb.p_sec_cback(BTA_DM_BUSY_LEVEL_EVT, &conn);
      }
      return;

    case BTM_BL_ROLE_CHG_EVT: /* role change event */
      p_dev = bta_dm_find_peer_device(p_bda);
      if (p_dev) {
        APPL_TRACE_DEBUG(
            "bta_dm_acl_change role chg info:x%x new_role:%d dev count:%d",
            p_dev->info, p_data->acl_change.new_role,
            bta_dm_cb.device_list.count);
        if (p_dev->info & BTA_DM_DI_AV_ACTIVE) {
          /* there's AV activity on this link */
          if ((p_data->acl_change.new_role == HCI_ROLE_SLAVE) &&
              (p_data->acl_change.hci_status == HCI_SUCCESS)) {
            BTM_SetA2dpStreamQoS(p_bda, NULL);
          }
          if (p_data->acl_change.new_role == HCI_ROLE_SLAVE &&
              bta_dm_cb.device_list.count > 1 &&
              p_data->acl_change.hci_status == HCI_SUCCESS) {
            /* more than one connections and the AV connection is role switched
             * to slave
             * switch it back to master and remove the switch policy */
            BTM_SwitchRole(p_bda, BTM_ROLE_MASTER, NULL);
            need_policy_change = true;
          } else if (p_bta_dm_cfg->avoid_scatter &&
                     (p_data->acl_change.new_role == HCI_ROLE_MASTER)) {
            /* if the link updated to be master include AV activities, remove
             * the switch policy */
            need_policy_change = true;
          }

          if (need_policy_change) {
            bta_dm_policy_cback(BTA_SYS_PLCY_CLR, 0,
                                HCI_ENABLE_MASTER_SLAVE_SWITCH,
                                p_dev->peer_bdaddr);
          }
        } else {
          /* there's AV no activity on this link and role switch happened
           * check if AV is active
           * if so, make sure the AV link is master */
          bta_dm_check_av(0);
        }
        bta_sys_notify_role_chg(p_data->acl_change.bd_addr,
                                p_data->acl_change.new_role,
                                p_data->acl_change.hci_status);
        conn.role_chg.bd_addr = p_bda;
        conn.role_chg.new_role = (uint8_t)p_data->acl_change.new_role;
        if (bta_dm_cb.p_sec_cback)
          bta_dm_cb.p_sec_cback(BTA_DM_ROLE_CHG_EVT, (tBTA_DM_SEC*)&conn);
      }
      return;
    case BTM_BL_PKT_TYPE_CHG_EVT:   /* packet type change event */
      conn.pkt_type_chg.remote_bd_addr = p_bda;
      conn.pkt_type_chg.pkt_type = (UINT16) p_data->acl_change.pkt_type;
      if (bta_dm_cb.p_sec_cback )
        bta_dm_cb.p_sec_cback(BTA_DM_PKT_TYPE_CHG_EVT, (tBTA_DM_SEC *)&conn);
      return;
    case BTM_BL_SOC_LOGGING_EVT:   /* packet type change event */
      conn.soc_logging.soc_log_id = (UINT16) p_data->acl_change.soc_log_id;
      if( bta_dm_cb.p_sec_cback )
          bta_dm_cb.p_sec_cback(BTA_DM_SOC_LOGGING_EVT, (tBTA_DM_SEC *)&conn);
      return;
  }

  /* Collision report from Stack: Notify profiles */
  if (p_data->acl_change.event == BTM_BL_COLLISION_EVT) {
    bta_sys_notify_collision(p_bda);
    return;
  }

  if (is_new) {
    for (i = 0; i < bta_dm_cb.device_list.count; i++) {
      if (bta_dm_cb.device_list.peer_device[i].peer_bdaddr == p_bda &&
          bta_dm_cb.device_list.peer_device[i].conn_handle ==
              p_data->acl_change.handle)
        break;
    }

    if (i == bta_dm_cb.device_list.count) {
      if (bta_dm_cb.device_list.count < BTA_DM_NUM_PEER_DEVICE) {
        /* new acl connection,reset new peer device */
        memset(&bta_dm_cb.device_list.peer_device[i], 0, sizeof(bta_dm_cb.device_list.peer_device[i]));
        bta_dm_cb.device_list.peer_device[bta_dm_cb.device_list.count]
            .peer_bdaddr = p_bda;
        bta_dm_cb.device_list.peer_device[bta_dm_cb.device_list.count]
            .link_policy = bta_dm_cb.cur_policy;
        bta_dm_cb.device_list.count++;
        APPL_TRACE_ERROR("%s new acl connetion:count = %d", __func__, bta_dm_cb.device_list.count);
        bta_dm_cb.device_list.peer_device[i].conn_handle =
            p_data->acl_change.handle;
        if (p_data->acl_change.transport == BT_TRANSPORT_LE)
          bta_dm_cb.device_list.le_count++;
      } else {
        APPL_TRACE_ERROR("%s max active connection reached, no resources",
                         __func__);
        return;
      }
    }

    bta_dm_cb.device_list.peer_device[i].conn_state = BTA_DM_CONNECTED;
    bta_dm_cb.device_list.peer_device[i].pref_role = BTA_ANY_ROLE;
    conn.link_up.bd_addr = p_bda;
    bta_dm_cb.device_list.peer_device[i].info = BTA_DM_DI_NONE;
    conn.link_up.link_type = p_data->acl_change.transport;
    bta_dm_cb.device_list.peer_device[i].transport =
        p_data->acl_change.transport;

    if (((NULL != (p = BTM_ReadLocalFeatures())) &&
         HCI_SNIFF_SUB_RATE_SUPPORTED(p)) &&
        ((NULL != (p = BTM_ReadRemoteFeatures(p_bda))) &&
         HCI_SNIFF_SUB_RATE_SUPPORTED(p))) {
      /* both local and remote devices support SSR */
      bta_dm_cb.device_list.peer_device[i].info = BTA_DM_DI_USE_SSR;
    }
    APPL_TRACE_WARNING("%s info: 0x%x", __func__,
                       bta_dm_cb.device_list.peer_device[i].info);

    if (bta_dm_cb.p_sec_cback)
      bta_dm_cb.p_sec_cback(BTA_DM_LINK_UP_EVT, (tBTA_DM_SEC*)&conn);
  } else {
    for (i = 0; i < bta_dm_cb.device_list.count; i++) {
      if (bta_dm_cb.device_list.peer_device[i].peer_bdaddr != p_bda ||
          bta_dm_cb.device_list.peer_device[i].transport !=
              p_data->acl_change.transport)
        continue;

      if (bta_dm_cb.device_list.peer_device[i].conn_state == BTA_DM_UNPAIRING) {
        if (BTM_SecDeleteDevice(
                bta_dm_cb.device_list.peer_device[i].peer_bdaddr))
          issue_unpair_cb = true;

        APPL_TRACE_DEBUG("%s: Unpairing: issue unpair CB = %d ", __func__,
                         issue_unpair_cb);
      }

      conn.link_down.is_removed =
          bta_dm_cb.device_list.peer_device[i].remove_dev_pending;

      // Iterate to the one before the last when shrinking the list,
      // otherwise we memcpy garbage data into the record.
      // Then clear out the last item in the list since we are shrinking.
      for (; i < bta_dm_cb.device_list.count - 1; i++) {
        memcpy(&bta_dm_cb.device_list.peer_device[i],
               &bta_dm_cb.device_list.peer_device[i + 1],
               sizeof(bta_dm_cb.device_list.peer_device[i]));
      }
      if (bta_dm_cb.device_list.count > 0) {
        int clear_index = bta_dm_cb.device_list.count - 1;
        memset(&bta_dm_cb.device_list.peer_device[clear_index], 0,
               sizeof(bta_dm_cb.device_list.peer_device[clear_index]));
      }
      break;
    }
    if (bta_dm_cb.device_list.count) bta_dm_cb.device_list.count--;
    if ((p_data->acl_change.transport == BT_TRANSPORT_LE) &&
        (bta_dm_cb.device_list.le_count))
      bta_dm_cb.device_list.le_count--;
    conn.link_down.link_type = p_data->acl_change.transport;

    if ((p_data->acl_change.transport == BT_TRANSPORT_LE) &&
        bta_dm_search_cb.gatt_disc_active &&
        bta_dm_search_cb.peer_bdaddr == p_bda) {
      APPL_TRACE_WARNING("%s cancel gatt discovery, addr: %s", __func__,
          p_bda.ToString().c_str());
      bta_dm_cancel_gatt_discovery(bta_dm_search_cb.peer_bdaddr);
    }

    if ((p_data->acl_change.transport == BT_TRANSPORT_BR_EDR) &&
        bta_dm_search_cb.wait_disc &&
        bta_dm_search_cb.peer_bdaddr == p_bda) {
      bta_dm_search_cb.wait_disc = false;

      if (bta_dm_search_cb.sdp_results) {
        APPL_TRACE_EVENT(" timer stopped  ");
        alarm_cancel(bta_dm_search_cb.search_timer);
        bta_dm_discover_next_device();
      }
    }

    if (bta_dm_cb.disabling) {
      if (!BTM_GetNumAclLinks()) {
        /*
         * Start a timer to make sure that the profiles
         * get the disconnect event.
         */
        alarm_set_on_mloop(bta_dm_cb.disable_timer,
                           BTA_DM_DISABLE_CONN_DOWN_TIMER_MS,
                           bta_dm_disable_conn_down_timer_cback, NULL);
      }
    }
    if (conn.link_down.is_removed) {
      BTM_SecDeleteDevice(p_bda);
      /* need to remove all pending background connection */
      BTA_GATTC_CancelOpen(0, p_bda, false);
      /* remove all cached GATT information */
      BTA_GATTC_Refresh(p_bda);
    }

    conn.link_down.bd_addr = p_bda;
    conn.link_down.status = (uint8_t)btm_get_acl_disc_reason_code();
    if (bta_dm_cb.p_sec_cback) {
      bta_dm_cb.p_sec_cback(BTA_DM_LINK_DOWN_EVT, &conn);
      if (issue_unpair_cb)
        bta_dm_cb.p_sec_cback(BTA_DM_DEV_UNPAIRED_EVT, &conn);
    }
  }

  bta_dm_adjust_roles(true);
}

/*******************************************************************************
**
** Function         bta_dm_process_iot_report
**
** Description      Sends disable event to application
**
**
** Returns          void
**
*******************************************************************************/
void bta_dm_process_iot_report(tBTA_DM_MSG *p_data)
{
  APPL_TRACE_WARNING("bta_dm_process_iot_report");
  tBTA_DM_VND_IOT_REPORT* iot_info = &p_data->iot_info;
  tBTA_DM_SEC sec_event;
  sec_event.iot_info.bd_addr = iot_info->bd_addr;
  sec_event.iot_info.error_type = iot_info->error_type;
  sec_event.iot_info.error_info = iot_info->error_info;
  sec_event.iot_info.event_mask = iot_info->event_mask;
  sec_event.iot_info.event_power_level = iot_info->event_power_level;
  sec_event.iot_info.event_rssi = iot_info->event_rssi;
  sec_event.iot_info.event_link_quality = iot_info->event_link_quality;
  sec_event.iot_info.event_glitch_count = iot_info->event_glitch_count;

  APPL_TRACE_DEBUG("bta_dm_process_iot_report %s",
                        iot_info->bd_addr.ToString().c_str());

  if( bta_dm_cb.p_sec_cback )
    bta_dm_cb.p_sec_cback(BTA_DM_IOT_INFO_EVT, (tBTA_DM_SEC *)&sec_event);
}

/*******************************************************************************
**
** Function         bta_dm_process_ssr
**
** Description      Notifies SSR event to application
**
**
** Returns          void
**
*******************************************************************************/
void bta_dm_process_ssr(void)
{
  APPL_TRACE_WARNING("bta_dm_process_ssr");

  if( bta_dm_cb.p_sec_cback )
    bta_dm_cb.p_sec_cback(BTA_DM_SSR_EVT, NULL);
}
/*******************************************************************************
 *
 * Function         bta_dm_disable_conn_down_timer_cback
 *
 * Description      Sends disable event to application
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_disable_conn_down_timer_cback(UNUSED_ATTR void* data) {
  tBTA_SYS_HW_MSG* sys_enable_event =
      (tBTA_SYS_HW_MSG*)osi_malloc(sizeof(tBTA_SYS_HW_MSG));

  /* disable the power managment module */
  bta_dm_disable_pm();

  /* register our callback to SYS HW manager */
  bta_sys_hw_register(BTA_SYS_HW_BLUETOOTH, bta_dm_sys_hw_cback);

  /* send a message to BTA SYS */
  sys_enable_event->hdr.event = BTA_SYS_API_DISABLE_EVT;
  sys_enable_event->hw_module = BTA_SYS_HW_BLUETOOTH;
  bta_sys_sendmsg(sys_enable_event);

  bta_dm_cb.disabling = false;
}

/*******************************************************************************
 *
 * Function         bta_dm_rm_cback
 *
 * Description      Role management callback from sys
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_rm_cback(tBTA_SYS_CONN_STATUS status, uint8_t id,
                            uint8_t app_id, const RawAddress& peer_addr) {
  uint8_t j;
  tBTA_PREF_ROLES role;
  tBTA_DM_PEER_DEVICE* p_dev;

  p_dev = bta_dm_find_peer_device(peer_addr);
  if (status == BTA_SYS_CONN_OPEN) {
    if (p_dev) {
      /* Do not set to connected if we are in the middle of unpairing. When AV
       * stream is
       * started it fakes out a SYS_CONN_OPEN to potentially trigger a role
       * switch command.
       * But this should not be done if we are in the middle of unpairing.
       */
      if (p_dev->conn_state != BTA_DM_UNPAIRING)
        p_dev->conn_state = BTA_DM_CONNECTED;

      for (j = 1; j <= p_bta_dm_rm_cfg[0].app_id; j++) {
        if (((p_bta_dm_rm_cfg[j].app_id == app_id) ||
             (p_bta_dm_rm_cfg[j].app_id == BTA_ALL_APP_ID)) &&
            (p_bta_dm_rm_cfg[j].id == id)) {
          role = p_bta_dm_rm_cfg[j].cfg;

          if (role > p_dev->pref_role) p_dev->pref_role = role;
          break;
        }
      }
    }
  }

  if ((BTA_ID_AV == id) || (BTA_ID_AVK == id)) {
    if (status == BTA_SYS_CONN_BUSY) {
      if (p_dev) p_dev->info |= BTA_DM_DI_AV_ACTIVE;
      /* AV calls bta_sys_conn_open with the A2DP stream count as app_id */
      if (BTA_ID_AV == id) bta_dm_cb.cur_av_count = bta_dm_get_av_count();
    } else if (status == BTA_SYS_CONN_IDLE) {
      if (p_dev) p_dev->info &= ~BTA_DM_DI_AV_ACTIVE;

      /* get cur_av_count from connected services */
      if (BTA_ID_AV == id) bta_dm_cb.cur_av_count = bta_dm_get_av_count();
    }
    APPL_TRACE_WARNING("bta_dm_rm_cback:%d, status:%d", bta_dm_cb.cur_av_count,
                       status);
  }

  /* Don't adjust roles for each busy/idle state transition to avoid
     excessive switch requests when individual profile busy/idle status
     changes */
  if ((status != BTA_SYS_CONN_BUSY) && (status != BTA_SYS_CONN_IDLE))
    bta_dm_adjust_roles(false);
}

/*******************************************************************************
 *
 * Function         bta_dm_delay_role_switch_cback
 *
 * Description      Callback from btm to delay a role switch
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_delay_role_switch_cback(UNUSED_ATTR void* data) {
  APPL_TRACE_EVENT("%s: initiating Delayed RS", __func__);
  bta_dm_adjust_roles(false);
}

/*******************************************************************************
 *
 * Function         bta_dm_reset_sec_dev_pending
 *
 * Description      Setting the remove device pending status to false from
 *                  security device DB, when the link key notification
 *                  event comes.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_reset_sec_dev_pending(const RawAddress& remote_bd_addr) {
  for (size_t i = 0; i < bta_dm_cb.device_list.count; i++) {
    if (bta_dm_cb.device_list.peer_device[i].peer_bdaddr == remote_bd_addr) {
      bta_dm_cb.device_list.peer_device[i].remove_dev_pending = false;
      return;
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_remove_sec_dev_entry
 *
 * Description      Removes device entry from Security device DB if ACL
 connection with
 *                  remtoe device does not exist, else schedule for dev entry
 removal upon
                     ACL close
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_remove_sec_dev_entry(const RawAddress& remote_bd_addr) {
  if (BTM_IsAclConnectionUp(remote_bd_addr, BT_TRANSPORT_LE) ||
      BTM_IsAclConnectionUp(remote_bd_addr, BT_TRANSPORT_BR_EDR)) {
    APPL_TRACE_DEBUG(
        "%s ACL is not down. Schedule for  Dev Removal when ACL closes",
        __func__);
    BTM_SecClearSecurityFlags(remote_bd_addr);
    for (int i = 0; i < bta_dm_cb.device_list.count; i++) {
      if (bta_dm_cb.device_list.peer_device[i].peer_bdaddr == remote_bd_addr) {
        bta_dm_cb.device_list.peer_device[i].remove_dev_pending = TRUE;
        break;
      }
    }
  } else {
    // remote_bd_addr comes from security record, which is removed in
    // BTM_SecDeleteDevice.
    RawAddress addr_copy = remote_bd_addr;
    BTM_SecDeleteDevice(addr_copy);
    /* need to remove all pending background connection */
    BTA_GATTC_CancelOpen(0, addr_copy, false);
    /* remove all cached GATT information */
    BTA_GATTC_Refresh(addr_copy);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_adjust_roles
 *
 * Description      Adjust roles
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_adjust_roles(bool delay_role_switch) {
  uint8_t i;
  bool set_master_role = false;
  uint8_t br_count =
      bta_dm_cb.device_list.count - bta_dm_cb.device_list.le_count;
  if (br_count) {
    /* the configuration is no scatternet
     * or AV connection exists and there are more than one ACL link */
    if ((p_bta_dm_rm_cfg[0].cfg == BTA_DM_NO_SCATTERNET) ||
        (bta_dm_cb.cur_av_count && br_count > 1)) {
      L2CA_SetDesireRole(HCI_ROLE_MASTER);
      set_master_role = true;
    }

    for (i = 0; i < bta_dm_cb.device_list.count; i++) {
      if (bta_dm_cb.device_list.peer_device[i].conn_state == BTA_DM_CONNECTED &&
          bta_dm_cb.device_list.peer_device[i].transport ==
              BT_TRANSPORT_BR_EDR) {
        if (!set_master_role &&
            (bta_dm_cb.device_list.peer_device[i].pref_role != BTA_ANY_ROLE) &&
            (p_bta_dm_rm_cfg[0].cfg == BTA_DM_PARTIAL_SCATTERNET)) {
          L2CA_SetDesireRole(HCI_ROLE_MASTER);
          set_master_role = true;
        }

        if ((bta_dm_cb.device_list.peer_device[i].pref_role ==
             BTA_MASTER_ROLE_ONLY) ||
            (br_count > 1)) {
          /* Initiating immediate role switch with certain remote devices
            has caused issues due to role  switch colliding with link encryption
            setup and
            causing encryption (and in turn the link) to fail .  These device .
            Firmware
            versions are stored in a blacklist and role switch with these
            devices are
            delayed to avoid the collision with link encryption setup */

          if (bta_dm_cb.device_list.peer_device[i].pref_role !=
                  BTA_SLAVE_ROLE_ONLY && !delay_role_switch) {
            BTM_SwitchRole(bta_dm_cb.device_list.peer_device[i].peer_bdaddr,
                           HCI_ROLE_MASTER, NULL);
          } else {
            alarm_set_on_mloop(bta_dm_cb.switch_delay_timer,
                               BTA_DM_SWITCH_DELAY_TIMER_MS,
                               bta_dm_delay_role_switch_cback, NULL);
          }
        } else if ((br_count == 1) &&
                   (bta_dm_cb.device_list.peer_device[i].pref_role ==
                    BTA_MASTER_ROLE_PREF)) {
          BTM_SwitchRole(bta_dm_cb.device_list.peer_device[i].peer_bdaddr,
                         HCI_ROLE_MASTER, NULL);
        }
      }
    }
    if (!set_master_role) {
      L2CA_SetDesireRole(L2CAP_DESIRED_LINK_ROLE);
    }

  } else {
    L2CA_SetDesireRole(L2CAP_DESIRED_LINK_ROLE);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_get_remname
 *
 * Description      Returns a pointer to the remote name stored in the DM
 *                  control block if it exists, or from the BTM memory.
 *
 * Returns          char * - Pointer to the remote device name
 ******************************************************************************/
static char* bta_dm_get_remname(void) {
  char* p_name = (char*)bta_dm_search_cb.peer_name;
  char* p_temp;

  /* If the name isn't already stored, try retrieving from BTM */
  if (*p_name == '\0') {
    p_temp = BTM_SecReadDevName(bta_dm_search_cb.peer_bdaddr);
    if (p_temp != NULL) p_name = p_temp;
  }

  return p_name;
}
/*******************************************************************************
 *
 * Function         bta_dm_link_power_cntrl_callback
 *
 * Description      Callback to notify link_power_cntrl cmd status
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_link_power_cntrl_callback(tBTM_VSC_CMPL *param)
 {
   uint8_t status = 0xFF;
   uint8_t* p;

   /* Check status of command complete event */
   CHECK(param->opcode == HCI_VS_LINK_POWER_CTRL_REQ_OPCODE);
   CHECK(param->param_len > 0);

   p = param->p_param_buf;
   STREAM_TO_UINT8(status, p);
   if (status != HCI_SUCCESS) {
     BTM_TRACE_DEBUG("%s: Status = 0x%02x (0 is success)",__func__, status);
     return;
   }

   APPL_TRACE_DEBUG("%s: param->opcode=%x subopcode=%x status=%x",
                      __func__, param->opcode, param->p_param_buf[1],
                      param->p_param_buf[0]);
 }

/*******************************************************************************
 *
 * Function         bta_dm_set_tech_based_max_power
 *
 * Description      limit the maximum output power for particular technology
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_set_tech_based_max_power(bool status) {
  uint8_t param[8] = {0};
  param[0] = HCI_VS_SET_MAX_RADIATED_POWER_SUB_OPCODE;
  uint8_t HCI_VS_LINK_POWER_CTRL_PARAM_SIZE;

  if (status == true) {
    static max_pow_feature_t tech_based_max_power;
    uint8_t tech_count = 0, i = 2;

    APPL_TRACE_DEBUG("%s",__func__);

    int tech_name;
    for (tech_name = BR_MAX_POW_SUPPORT;tech_name <= BLE_MAX_POW_SUPPORT;tech_name++) {
      tech_based_max_power = max_radiated_power_fetch(MAX_POW_ID, (profile_info_t)tech_name);
    }

    if (tech_based_max_power.BR_max_pow_feature == true) {
      param[1] = ++tech_count;
      param[i++] = BR_TECH_VALUE;
      param[i++] = tech_based_max_power.BR_max_pow_support;

      APPL_TRACE_DEBUG("%s BR_max_power fetch from conf file: 0X%x",
            __func__, tech_based_max_power.BR_max_pow_support);
    }

    if (tech_based_max_power.EDR_max_pow_feature == true) {
      param[1] = ++tech_count;
      param[i++] = EDR_TECH_VALUE;
      param[i++] = tech_based_max_power.EDR_max_pow_support;

      APPL_TRACE_DEBUG("%s EDR_max_power fetch from conf file: 0X%x",
            __func__, tech_based_max_power.EDR_max_pow_support);
    }

    if (tech_based_max_power.BLE_max_pow_feature == true) {

      param[1] = ++tech_count;
      param[i++] = BLE_TECH_VALUE;
      param[i++] = tech_based_max_power.BLE_max_pow_support;

      APPL_TRACE_DEBUG("%s BLE_max_power fetch from conf file: 0X%x",
            __func__, tech_based_max_power.BLE_max_pow_support);
    }

    if (!tech_count) {
      APPL_TRACE_DEBUG("%s: max power not configured for any tech ",__func__);
      return;
    } else {
      HCI_VS_LINK_POWER_CTRL_PARAM_SIZE = 2 + (tech_count * 2);
    }

  } else {
    HCI_VS_LINK_POWER_CTRL_PARAM_SIZE = 8;
    param[1] = 0x03;
    param[2] = BR_TECH_VALUE;
    param[3] = 0x80;
    param[4] = EDR_TECH_VALUE;
    param[5] = 0x80;
    param[6] = BLE_TECH_VALUE;
    param[7] = 0x80;
  }

    BTM_VendorSpecificCommand(HCI_VS_LINK_POWER_CTRL_REQ_OPCODE,
                                 HCI_VS_LINK_POWER_CTRL_PARAM_SIZE, param,
                                 bta_dm_link_power_cntrl_callback);
}

/*******************************************************************************
 *
 * Function         bta_dm_bond_cancel_complete_cback
 *
 * Description      Authentication complete callback from BTM
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_bond_cancel_complete_cback(tBTM_STATUS result) {
  tBTA_DM_SEC sec_event;

  if (result == BTM_SUCCESS)
    sec_event.bond_cancel_cmpl.result = BTA_SUCCESS;
  else
    sec_event.bond_cancel_cmpl.result = BTA_FAILURE;

  if (bta_dm_cb.p_sec_cback) {
    bta_dm_cb.p_sec_cback(BTA_DM_BOND_CANCEL_CMPL_EVT, &sec_event);
  }
}

/*******************************************************************************
 *
 * Function         find_utf8_char_boundary
 *
 * Description      This function checks a UTF8 string |utf8str| starting at
 *                  |offset|, moving backwards and returns the offset of the
 *                  next valid UTF8 character boundary found.
 *
 * Returns          Offset of UTF8 character boundary
 *
 ******************************************************************************/
static size_t find_utf8_char_boundary(const char* utf8str, size_t offset) {
  CHECK(utf8str);
  CHECK(offset > 0);

  while (--offset) {
    uint8_t ch = (uint8_t)utf8str[offset];
    if ((ch & 0x80) == 0x00)  // ASCII
      return offset + 1;
    if ((ch & 0xC0) == 0xC0)  // Multi-byte sequence start
      return offset;
  }

  return 0;
}

/*******************************************************************************
 *
 * Function         bta_dm_set_eir
 *
 * Description      This function creates EIR tagged data and writes it to
 *                  controller.
 *
 * Returns          None
 *
 ******************************************************************************/
static void bta_dm_set_eir(char* local_name) {
  uint8_t* p;
  uint8_t* p_length;
#if (BTA_EIR_CANNED_UUID_LIST != TRUE)
  uint8_t* p_type;
  uint8_t max_num_uuid;
#if (BTA_EIR_SERVER_NUM_CUSTOM_UUID > 0)
  uint8_t custom_uuid_idx;
#endif  // BTA_EIR_SERVER_NUM_CUSTOM_UUID
#endif  // BTA_EIR_CANNED_UUID_LIST
#if (BTM_EIR_DEFAULT_FEC_REQUIRED == FALSE)
  uint8_t free_eir_length = HCI_EXT_INQ_RESPONSE_LEN;
#else  // BTM_EIR_DEFAULT_FEC_REQUIRED
  uint8_t free_eir_length = HCI_DM5_PACKET_SIZE;
#endif  // BTM_EIR_DEFAULT_FEC_REQUIRED
  uint8_t num_uuid;
  uint8_t data_type;
  uint8_t local_name_len;

  /* wait until complete to disable */
  if (alarm_is_scheduled(bta_dm_cb.disable_timer)) return;

#if (BTA_EIR_CANNED_UUID_LIST != TRUE)
  /* if local name is not provided, get it from controller */
  if (local_name == NULL) {
    if (BTM_ReadLocalDeviceName(&local_name) != BTM_SUCCESS) {
      APPL_TRACE_ERROR("Fail to read local device name for EIR");
    }
  }
#endif  // BTA_EIR_CANNED_UUID_LIST

  /* Allocate a buffer to hold HCI command */
  BT_HDR* p_buf = (BT_HDR*)osi_malloc(BTM_CMD_BUF_SIZE);
  p = (uint8_t*)p_buf + BTM_HCI_EIR_OFFSET;

  memset(p, 0x00, HCI_EXT_INQ_RESPONSE_LEN);

  APPL_TRACE_DEBUG("BTA is generating EIR");

  if (local_name)
    local_name_len = strlen(local_name);
  else
    local_name_len = 0;

  data_type = BTM_EIR_COMPLETE_LOCAL_NAME_TYPE;
  /* if local name is longer than minimum length of shortened name */
  /* check whether it needs to be shortened or not */
  if (local_name_len > p_bta_dm_eir_cfg->bta_dm_eir_min_name_len) {
/* get number of UUID 16-bit list */
#if (BTA_EIR_CANNED_UUID_LIST == TRUE)
    num_uuid = p_bta_dm_eir_cfg->bta_dm_eir_uuid16_len / Uuid::kNumBytes16;
#else   // BTA_EIR_CANNED_UUID_LIST
    max_num_uuid = (free_eir_length - 2) / Uuid::kNumBytes16;
    data_type = BTM_GetEirSupportedServices(bta_dm_cb.eir_uuid, &p,
                                            max_num_uuid, &num_uuid);
    p = (uint8_t*)p_buf + BTM_HCI_EIR_OFFSET; /* reset p */
#endif  // BTA_EIR_CANNED_UUID_LIST

    /* if UUID doesn't fit remaing space, shorten local name */
    if (local_name_len > (free_eir_length - 4 - num_uuid * Uuid::kNumBytes16)) {
      local_name_len = find_utf8_char_boundary(
          local_name, p_bta_dm_eir_cfg->bta_dm_eir_min_name_len);
      APPL_TRACE_WARNING("%s local name is shortened (%d)", __func__,
                         local_name_len);
      data_type = BTM_EIR_SHORTENED_LOCAL_NAME_TYPE;
    } else {
      data_type = BTM_EIR_COMPLETE_LOCAL_NAME_TYPE;
    }
  }

  UINT8_TO_STREAM(p, local_name_len + 1);
  UINT8_TO_STREAM(p, data_type);

  if (local_name != NULL) {
    memcpy(p, local_name, local_name_len);
    p += local_name_len;
  }
  free_eir_length -= local_name_len + 2;

#if (BTA_EIR_CANNED_UUID_LIST == TRUE)
  /* if UUID list is provided as static data in configuration */
  if ((p_bta_dm_eir_cfg->bta_dm_eir_uuid16_len > 0) &&
      (p_bta_dm_eir_cfg->bta_dm_eir_uuid16)) {
    if (free_eir_length > Uuid::kNumBytes16 + 2) {
      free_eir_length -= 2;

      if (free_eir_length >= p_bta_dm_eir_cfg->bta_dm_eir_uuid16_len) {
        num_uuid = p_bta_dm_eir_cfg->bta_dm_eir_uuid16_len / Uuid::kNumBytes16;
        data_type = BTM_EIR_COMPLETE_16BITS_UUID_TYPE;
      } else /* not enough room for all UUIDs */
      {
        APPL_TRACE_WARNING("BTA EIR: UUID 16-bit list is truncated");
        num_uuid = free_eir_length / Uuid::kNumBytes16;
        data_type = BTM_EIR_MORE_16BITS_UUID_TYPE;
      }
      UINT8_TO_STREAM(p, num_uuid * Uuid::kNumBytes16 + 1);
      UINT8_TO_STREAM(p, data_type);
      memcpy(p, p_bta_dm_eir_cfg->bta_dm_eir_uuid16,
             num_uuid * Uuid::kNumBytes16);
      p += num_uuid * Uuid::kNumBytes16;
      free_eir_length -= num_uuid * Uuid::kNumBytes16;
    }
  }
#else /* (BTA_EIR_CANNED_UUID_LIST == TRUE) */
  /* if UUID list is dynamic */
  if (free_eir_length >= 2) {
    p_length = p++;
    p_type = p++;
    num_uuid = 0;

    max_num_uuid = (free_eir_length - 2) / Uuid::kNumBytes16;
    data_type = BTM_GetEirSupportedServices(bta_dm_cb.eir_uuid, &p,
                                            max_num_uuid, &num_uuid);

    if (data_type == BTM_EIR_MORE_16BITS_UUID_TYPE) {
      APPL_TRACE_WARNING("BTA EIR: UUID 16-bit list is truncated");
    }
#if (BTA_EIR_SERVER_NUM_CUSTOM_UUID > 0)
    else {
      for (custom_uuid_idx = 0;
           custom_uuid_idx < BTA_EIR_SERVER_NUM_CUSTOM_UUID;
           custom_uuid_idx++) {
        const Uuid& curr = bta_dm_cb.custom_uuid[custom_uuid_idx];
        if (curr.GetShortestRepresentationSize() == Uuid::kNumBytes16) {
          if (num_uuid < max_num_uuid) {
            UINT16_TO_STREAM(p, curr.As16Bit());
            num_uuid++;
          } else {
            data_type = BTM_EIR_MORE_16BITS_UUID_TYPE;
            APPL_TRACE_WARNING("BTA EIR: UUID 16-bit list is truncated");
            break;
          }
        }
      }
    }
#endif /* (BTA_EIR_SERVER_NUM_CUSTOM_UUID > 0) */

    UINT8_TO_STREAM(p_length, num_uuid * Uuid::kNumBytes16 + 1);
    UINT8_TO_STREAM(p_type, data_type);
    free_eir_length -= num_uuid * Uuid::kNumBytes16 + 2;
  }
#endif /* (BTA_EIR_CANNED_UUID_LIST == TRUE) */

#if (BTA_EIR_CANNED_UUID_LIST != TRUE && BTA_EIR_SERVER_NUM_CUSTOM_UUID > 0)
  /* Adding 32-bit UUID list */
  if (free_eir_length >= 2) {
    p_length = p++;
    p_type = p++;
    num_uuid = 0;
    data_type = BTM_EIR_COMPLETE_32BITS_UUID_TYPE;

    max_num_uuid = (free_eir_length - 2) / Uuid::kNumBytes32;

    for (custom_uuid_idx = 0; custom_uuid_idx < BTA_EIR_SERVER_NUM_CUSTOM_UUID;
         custom_uuid_idx++) {
      const Uuid& curr = bta_dm_cb.custom_uuid[custom_uuid_idx];
      if (curr.GetShortestRepresentationSize() == Uuid::kNumBytes32) {
        if (num_uuid < max_num_uuid) {
          UINT32_TO_STREAM(p, curr.As32Bit());
          num_uuid++;
        } else {
          data_type = BTM_EIR_MORE_32BITS_UUID_TYPE;
          APPL_TRACE_WARNING("BTA EIR: UUID 32-bit list is truncated");
          break;
        }
      }
    }

    UINT8_TO_STREAM(p_length, num_uuid * Uuid::kNumBytes32 + 1);
    UINT8_TO_STREAM(p_type, data_type);
    free_eir_length -= num_uuid * Uuid::kNumBytes32 + 2;
  }

  /* Adding 128-bit UUID list */
  if (free_eir_length >= 2) {
    p_length = p++;
    p_type = p++;
    num_uuid = 0;
    data_type = BTM_EIR_COMPLETE_128BITS_UUID_TYPE;

    max_num_uuid = (free_eir_length - 2) / Uuid::kNumBytes128;

    for (custom_uuid_idx = 0; custom_uuid_idx < BTA_EIR_SERVER_NUM_CUSTOM_UUID;
         custom_uuid_idx++) {
      const Uuid& curr = bta_dm_cb.custom_uuid[custom_uuid_idx];
      if (curr.GetShortestRepresentationSize() == Uuid::kNumBytes128) {
        if (num_uuid < max_num_uuid) {
          ARRAY16_TO_STREAM(p, curr.To128BitBE().data());
          num_uuid++;
        } else {
          data_type = BTM_EIR_MORE_128BITS_UUID_TYPE;
          APPL_TRACE_WARNING("BTA EIR: UUID 128-bit list is truncated");
          break;
        }
      }
    }

    UINT8_TO_STREAM(p_length, num_uuid * Uuid::kNumBytes128 + 1);
    UINT8_TO_STREAM(p_type, data_type);
    free_eir_length -= num_uuid * Uuid::kNumBytes128 + 2;
  }
#endif /* ( BTA_EIR_CANNED_UUID_LIST != TRUE \
          )&&(BTA_EIR_SERVER_NUM_CUSTOM_UUID > 0) */

  /* if Flags are provided in configuration */
  if ((p_bta_dm_eir_cfg->bta_dm_eir_flag_len > 0) &&
      (p_bta_dm_eir_cfg->bta_dm_eir_flags) &&
      (free_eir_length >= p_bta_dm_eir_cfg->bta_dm_eir_flag_len + 2)) {
    UINT8_TO_STREAM(p, p_bta_dm_eir_cfg->bta_dm_eir_flag_len + 1);
    UINT8_TO_STREAM(p, BTM_EIR_FLAGS_TYPE);
    memcpy(p, p_bta_dm_eir_cfg->bta_dm_eir_flags,
           p_bta_dm_eir_cfg->bta_dm_eir_flag_len);
    p += p_bta_dm_eir_cfg->bta_dm_eir_flag_len;
    free_eir_length -= p_bta_dm_eir_cfg->bta_dm_eir_flag_len + 2;
  }

  /* if Manufacturer Specific are provided in configuration */
  if ((p_bta_dm_eir_cfg->bta_dm_eir_manufac_spec_len > 0) &&
      (p_bta_dm_eir_cfg->bta_dm_eir_manufac_spec) &&
      (free_eir_length >= p_bta_dm_eir_cfg->bta_dm_eir_manufac_spec_len + 2)) {
    p_length = p;

    UINT8_TO_STREAM(p, p_bta_dm_eir_cfg->bta_dm_eir_manufac_spec_len + 1);
    UINT8_TO_STREAM(p, BTM_EIR_MANUFACTURER_SPECIFIC_TYPE);
    memcpy(p, p_bta_dm_eir_cfg->bta_dm_eir_manufac_spec,
           p_bta_dm_eir_cfg->bta_dm_eir_manufac_spec_len);
    p += p_bta_dm_eir_cfg->bta_dm_eir_manufac_spec_len;
    free_eir_length -= p_bta_dm_eir_cfg->bta_dm_eir_manufac_spec_len + 2;

  } else {
    p_length = NULL;
  }

  /* if Inquiry Tx Resp Power compiled */
  if ((p_bta_dm_eir_cfg->bta_dm_eir_inq_tx_power) && (free_eir_length >= 3)) {
    UINT8_TO_STREAM(p, 2); /* Length field */
    UINT8_TO_STREAM(p, BTM_EIR_TX_POWER_LEVEL_TYPE);
    UINT8_TO_STREAM(p, *(p_bta_dm_eir_cfg->bta_dm_eir_inq_tx_power));
    free_eir_length -= 3;
  }

  if (free_eir_length)
    UINT8_TO_STREAM(p, 0); /* terminator of significant part */

  BTM_WriteEIR(p_buf);
}

/*******************************************************************************
 *
 * Function         bta_dm_eir_search_services
 *
 * Description      This function searches services in received EIR
 *
 * Returns          None
 *
 ******************************************************************************/
static void bta_dm_eir_search_services(tBTM_INQ_RESULTS* p_result,
                                       tBTA_SERVICE_MASK* p_services_to_search,
                                       tBTA_SERVICE_MASK* p_services_found) {
  tBTA_SERVICE_MASK service_index = 0;
  tBTM_EIR_SEARCH_RESULT result;

  VLOG(1) << "BTA searching services in EIR of BDA:"
          << p_result->remote_bd_addr;

  APPL_TRACE_DEBUG("    with services_to_search=0x%08X", *p_services_to_search);

  /* always do GATT based service discovery by SDP instead of from EIR    */
  /* if GATT based service is also to be put in EIR, need to modify this  */
  while (service_index < (BTA_MAX_SERVICE_ID - 1)) {
    if (*p_services_to_search &
        (tBTA_SERVICE_MASK)(BTA_SERVICE_ID_TO_SERVICE_MASK(service_index))) {
      result = BTM_HasInquiryEirService(
          p_result, bta_service_id_to_uuid_lkup_tbl[service_index]);

      /* Searching for HSP v1.2 only device */
      if ((result != BTM_EIR_FOUND) &&
          (bta_service_id_to_uuid_lkup_tbl[service_index] ==
           UUID_SERVCLASS_HEADSET)) {
        result = BTM_HasInquiryEirService(p_result, UUID_SERVCLASS_HEADSET_HS);
      }

      if (result == BTM_EIR_FOUND) {
        /* If Plug and Play service record, need to check to see if Broadcom
         * stack */
        /* However, EIR data doesn't have EXT_BRCM_VERSION so just skip it */
        if (bta_service_id_to_uuid_lkup_tbl[service_index] !=
            UUID_SERVCLASS_PNP_INFORMATION) {
          *p_services_found |= (tBTA_SERVICE_MASK)(
              BTA_SERVICE_ID_TO_SERVICE_MASK(service_index));
          /* remove the service from services to be searched  */
          *p_services_to_search &= (tBTA_SERVICE_MASK)(
              ~(BTA_SERVICE_ID_TO_SERVICE_MASK(service_index)));
        }
      } else if (result == BTM_EIR_NOT_FOUND) {
        /* remove the service from services to be searched  */
        *p_services_to_search &= (tBTA_SERVICE_MASK)(
            ~(BTA_SERVICE_ID_TO_SERVICE_MASK(service_index)));
      }
    }

    service_index++;
  }

  APPL_TRACE_ERROR(
      "BTA EIR search result, services_to_search=0x%08X, services_found=0x%08X",
      *p_services_to_search, *p_services_found);
}

#if (BTA_EIR_CANNED_UUID_LIST != TRUE)
/*******************************************************************************
 *
 * Function         bta_dm_eir_update_uuid
 *
 * Description      This function adds or removes service UUID in EIR database.
 *
 * Returns          None
 *
 ******************************************************************************/
void bta_dm_eir_update_uuid(uint16_t uuid16, bool adding) {
  /* if this UUID is not advertised in EIR */
  if (!BTM_HasEirService(p_bta_dm_eir_cfg->uuid_mask, uuid16)) return;

  if (adding) {
    APPL_TRACE_EVENT("Adding UUID=0x%04X into EIR", uuid16);

    BTM_AddEirService(bta_dm_cb.eir_uuid, uuid16);
  } else {
    APPL_TRACE_EVENT("Removing UUID=0x%04X from EIR", uuid16);

    BTM_RemoveEirService(bta_dm_cb.eir_uuid, uuid16);
  }

  bta_dm_set_eir(NULL);

  APPL_TRACE_EVENT("bta_dm_eir_update_uuid UUID bit mask=0x%08X %08X",
                   bta_dm_cb.eir_uuid[1], bta_dm_cb.eir_uuid[0]);
}
#endif

/*******************************************************************************
 *
 * Function         bta_dm_enable_test_mode
 *
 * Description      enable test mode
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_enable_test_mode(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  BTM_EnableTestMode();
}

/*******************************************************************************
 *
 * Function         bta_dm_disable_test_mode
 *
 * Description      disable test mode
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_disable_test_mode(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  BTM_HCI_Reset();
}

/*******************************************************************************
 *
 * Function         bta_dm_execute_callback
 *
 * Description      Just execute a generic call back in the context of the
 *                  BTU/BTA tack
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_execute_callback(tBTA_DM_MSG* p_data) {
  /* sanity check */
  if (p_data->exec_cback.p_exec_cback == NULL) {
    return;
  }

  p_data->exec_cback.p_exec_cback(p_data->exec_cback.p_param);
}

/*******************************************************************************
 *
 * Function         bta_dm_encrypt_cback
 *
 * Description      link encryption complete callback.
 *
 * Returns         None
 *
 ******************************************************************************/
void bta_dm_encrypt_cback(const RawAddress* bd_addr, tBT_TRANSPORT transport,
                          UNUSED_ATTR void* p_ref_data, tBTM_STATUS result) {
  tBTA_STATUS bta_status = BTA_SUCCESS;
  tBTA_DM_ENCRYPT_CBACK* p_callback = NULL;
  uint8_t i;

  for (i = 0; i < bta_dm_cb.device_list.count; i++) {
    if (bta_dm_cb.device_list.peer_device[i].peer_bdaddr == *bd_addr &&
        bta_dm_cb.device_list.peer_device[i].conn_state == BTA_DM_CONNECTED)
      break;
  }

  if (i < bta_dm_cb.device_list.count) {
    p_callback = bta_dm_cb.device_list.peer_device[i].p_encrypt_cback;
    bta_dm_cb.device_list.peer_device[i].p_encrypt_cback = NULL;
  }

  switch (result) {
    case BTM_SUCCESS:
      break;
    case BTM_WRONG_MODE:
      bta_status = BTA_WRONG_MODE;
      break;
    case BTM_NO_RESOURCES:
      bta_status = BTA_NO_RESOURCES;
      break;
    case BTM_BUSY:
      bta_status = BTA_BUSY;
      break;
    default:
      bta_status = BTA_FAILURE;
      break;
  }

  APPL_TRACE_DEBUG("bta_dm_encrypt_cback status =%d p_callback=0x%x",
                   bta_status, p_callback);

  if (p_callback) {
    (*p_callback)(*bd_addr, transport, bta_status);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_set_encryption
 *
 * Description      This function to encrypt the link
 *
 * Returns          None
 *
 ******************************************************************************/
void bta_dm_set_encryption(tBTA_DM_MSG* p_data) {
  uint8_t i;

  APPL_TRACE_DEBUG("bta_dm_set_encryption");  // todo
  if (!p_data->set_encryption.p_callback) {
    APPL_TRACE_ERROR("bta_dm_set_encryption callback is not provided");
    return;
  }
  for (i = 0; i < bta_dm_cb.device_list.count; i++) {
    if (bta_dm_cb.device_list.peer_device[i].peer_bdaddr ==
            p_data->set_encryption.bd_addr &&
        bta_dm_cb.device_list.peer_device[i].conn_state == BTA_DM_CONNECTED)
      break;
  }
  if (i < bta_dm_cb.device_list.count) {
    if (bta_dm_cb.device_list.peer_device[i].p_encrypt_cback) {
      APPL_TRACE_ERROR("earlier enc was not done for same device");
      (*p_data->set_encryption.p_callback)(p_data->set_encryption.bd_addr,
                                           p_data->set_encryption.transport,
                                           BTA_BUSY);
      return;
    }

    if (BTM_SetEncryption(p_data->set_encryption.bd_addr,
                          p_data->set_encryption.transport,
                          bta_dm_encrypt_cback, NULL,
                          p_data->set_encryption.sec_act) == BTM_CMD_STARTED) {
      bta_dm_cb.device_list.peer_device[i].p_encrypt_cback =
          p_data->set_encryption.p_callback;
    }
  }
}

bool bta_dm_check_if_only_hd_connected(const RawAddress& peer_addr) {
  APPL_TRACE_DEBUG("%s: count(%d)", __func__, bta_dm_conn_srvcs.count);

  for (uint8_t j = 0; j < bta_dm_conn_srvcs.count; j++) {
    // Check if profiles other than hid are connected
    if ((bta_dm_conn_srvcs.conn_srvc[j].id != BTA_ID_HD) &&
        bta_dm_conn_srvcs.conn_srvc[j].peer_bdaddr == peer_addr) {
      APPL_TRACE_DEBUG("%s: Another profile (id=%d) is connected", __func__,
                       bta_dm_conn_srvcs.conn_srvc[j].id);
      return false;
    }
  }

  return true;
}

/*******************************************************************************
 *
 * Function         bta_dm_observe_results_cb
 *
 * Description      Callback for BLE Observe result
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_observe_results_cb(tBTM_INQ_RESULTS* p_inq, uint8_t* p_eir,
                                      uint16_t eir_len) {
  tBTA_DM_SEARCH result;
  tBTM_INQ_INFO* p_inq_info;
  APPL_TRACE_DEBUG("bta_dm_observe_results_cb");

  result.inq_res.bd_addr = p_inq->remote_bd_addr;
  result.inq_res.rssi = p_inq->rssi;
  result.inq_res.ble_addr_type = p_inq->ble_addr_type;
  result.inq_res.inq_result_type = p_inq->inq_result_type;
  result.inq_res.device_type = p_inq->device_type;
  result.inq_res.flag = p_inq->flag;
  result.inq_res.ble_evt_type = p_inq->ble_evt_type;
  result.inq_res.ble_primary_phy = p_inq->ble_primary_phy;
  result.inq_res.ble_secondary_phy = p_inq->ble_secondary_phy;
  result.inq_res.ble_advertising_sid = p_inq->ble_advertising_sid;
  result.inq_res.ble_tx_power = p_inq->ble_tx_power;
  result.inq_res.ble_periodic_adv_int = p_inq->ble_periodic_adv_int;

  /* application will parse EIR to find out remote device name */
  result.inq_res.p_eir = p_eir;
  result.inq_res.eir_len = eir_len;

  p_inq_info = BTM_InqDbRead(p_inq->remote_bd_addr);
  if (p_inq_info != NULL) {
    /* initialize remt_name_not_required to false so that we get the name by
     * default */
    result.inq_res.remt_name_not_required = false;
  }

  if (bta_dm_search_cb.p_scan_cback)
    bta_dm_search_cb.p_scan_cback(BTA_DM_INQ_RES_EVT, &result);

  if (p_inq_info) {
    /* application indicates if it knows the remote name, inside the callback
     copy that to the inquiry data base*/
    if (result.inq_res.remt_name_not_required)
      p_inq_info->appl_knows_rem_name = true;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_observe_cmpl_cb
 *
 * Description      Callback for BLE Observe complete
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_observe_cmpl_cb(void* p_result) {
  tBTA_DM_SEARCH data;

  APPL_TRACE_DEBUG("bta_dm_observe_cmpl_cb");

  data.inq_cmpl.num_resps = ((tBTM_INQUIRY_CMPL*)p_result)->num_resp;
  if (bta_dm_search_cb.p_scan_cback) {
    bta_dm_search_cb.p_scan_cback(BTA_DM_INQ_CMPL_EVT, &data);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_ble_smp_cback
 *
 * Description      Callback for BLE SMP
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static uint8_t bta_dm_ble_smp_cback(tBTM_LE_EVT event, const RawAddress& bda,
                                    tBTM_LE_EVT_DATA* p_data) {
  tBTM_STATUS status = BTM_SUCCESS;
  tBTA_DM_SEC sec_event;
  char* p_name = NULL;

  if (!bta_dm_cb.p_sec_cback) return BTM_NOT_AUTHORIZED;

  memset(&sec_event, 0, sizeof(tBTA_DM_SEC));
  switch (event) {
    case BTM_LE_IO_REQ_EVT:
      if (btm_local_io_caps != BTM_IO_CAP_NONE) {
        bta_dm_co_ble_io_req(
            bda, &p_data->io_req.io_cap, &p_data->io_req.oob_data,
            &p_data->io_req.auth_req, &p_data->io_req.max_key_size,
            &p_data->io_req.init_keys, &p_data->io_req.resp_keys);
      }
      APPL_TRACE_EVENT("io mitm: %d oob_data:%d", p_data->io_req.auth_req,
                       p_data->io_req.oob_data);

      break;

    case BTM_LE_SEC_REQUEST_EVT:
      sec_event.ble_req.bd_addr = bda;
      p_name = BTM_SecReadDevName(bda);
      if (p_name != NULL)
        strlcpy((char*)sec_event.ble_req.bd_name, p_name, BD_NAME_LEN + 1);
      else
        sec_event.ble_req.bd_name[0] = 0;
      bta_dm_cb.p_sec_cback(BTA_DM_BLE_SEC_REQ_EVT, &sec_event);
      break;

    case BTM_LE_KEY_NOTIF_EVT:
      sec_event.key_notif.bd_addr = bda;
      p_name = BTM_SecReadDevName(bda);
      if (p_name != NULL)
        strlcpy((char*)sec_event.key_notif.bd_name, p_name, BD_NAME_LEN + 1);
      else
        sec_event.key_notif.bd_name[0] = 0;
      sec_event.key_notif.passkey = p_data->key_notif;
      bta_dm_cb.p_sec_cback(BTA_DM_BLE_PASSKEY_NOTIF_EVT, &sec_event);
      break;

    case BTM_LE_KEY_REQ_EVT:
      sec_event.ble_req.bd_addr = bda;
      bta_dm_cb.p_sec_cback(BTA_DM_BLE_PASSKEY_REQ_EVT, &sec_event);
      break;

    case BTM_LE_OOB_REQ_EVT:
      sec_event.ble_req.bd_addr = bda;
      bta_dm_cb.p_sec_cback(BTA_DM_BLE_OOB_REQ_EVT, &sec_event);
      break;

    case BTM_LE_NC_REQ_EVT:
      sec_event.key_notif.bd_addr = bda;
      p_name = BTM_SecReadDevName(bda);
      if (p_name != NULL)
        strlcpy((char*)sec_event.key_notif.bd_name, p_name, BD_NAME_LEN + 1);
      else
        sec_event.key_notif.bd_name[0] = 0;
      sec_event.key_notif.passkey = p_data->key_notif;
      bta_dm_cb.p_sec_cback(BTA_DM_BLE_NC_REQ_EVT, &sec_event);
      break;

    case BTM_LE_SC_OOB_REQ_EVT:
      sec_event.ble_req.bd_addr = bda;
      bta_dm_cb.p_sec_cback(BTA_DM_BLE_SC_OOB_REQ_EVT, &sec_event);
      break;

    case BTM_LE_KEY_EVT:
      sec_event.ble_key.bd_addr = bda;
      sec_event.ble_key.key_type = p_data->key.key_type;
      sec_event.ble_key.p_key_value = p_data->key.p_key_value;
      bta_dm_cb.p_sec_cback(BTA_DM_BLE_KEY_EVT, &sec_event);
      break;

    case BTM_LE_COMPLT_EVT:
      sec_event.auth_cmpl.bd_addr = bda;
      BTM_ReadDevInfo(bda, &sec_event.auth_cmpl.dev_type,
                      &sec_event.auth_cmpl.addr_type);
      p_name = BTM_SecReadDevName(bda);
      if (p_name != NULL)
        strlcpy((char*)sec_event.auth_cmpl.bd_name, p_name, (BD_NAME_LEN + 1));
      else
        sec_event.auth_cmpl.bd_name[0] = 0;

      if (p_data->complt.reason != 0) {
        sec_event.auth_cmpl.fail_reason =
            BTA_DM_AUTH_CONVERT_SMP_CODE(((uint8_t)p_data->complt.reason));

        if (btm_sec_is_a_bonded_dev(bda) &&
            p_data->complt.reason == SMP_CONN_TOUT) {
          // Bonded device failed to encrypt - to test this remove battery from
          // HID device right after connection, but before encryption is
          // established
          LOG(INFO) << __func__
                    << ": bonded device disconnected when encrypting - no "
                       "reason to unbond";
        } else {
          /* delete this device entry from Sec Dev DB */
          bta_dm_remove_sec_dev_entry(bda);
        }

      } else {
        sec_event.auth_cmpl.success = true;

        sec_event.auth_cmpl.smp_over_br = p_data->complt.smp_over_br;
        if (!p_data->complt.smp_over_br)
          GATT_ConfigServiceChangeCCC(bda, true, BT_TRANSPORT_LE);
      }

      if (bta_dm_cb.p_sec_cback) {
        // bta_dm_cb.p_sec_cback(BTA_DM_AUTH_CMPL_EVT, &sec_event);
        bta_dm_cb.p_sec_cback(BTA_DM_BLE_AUTH_CMPL_EVT, &sec_event);
      }
      break;

    default:
      status = BTM_NOT_AUTHORIZED;
      break;
  }
  return status;
}

/*******************************************************************************
 *
 * Function         bta_dm_ble_id_key_cback
 *
 * Description      Callback for BLE local ID keys
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_ble_id_key_cback(uint8_t key_type,
                                    tBTM_BLE_LOCAL_KEYS* p_key) {
  uint8_t evt;
  tBTA_DM_SEC dm_key;

  switch (key_type) {
    case BTM_BLE_KEY_TYPE_ID:
    case BTM_BLE_KEY_TYPE_ER:
      if (bta_dm_cb.p_sec_cback) {
        memcpy(&dm_key.ble_id_keys, p_key, sizeof(tBTM_BLE_LOCAL_KEYS));

        evt = (key_type == BTM_BLE_KEY_TYPE_ID) ? BTA_DM_BLE_LOCAL_IR_EVT
                                                : BTA_DM_BLE_LOCAL_ER_EVT;
        bta_dm_cb.p_sec_cback(evt, &dm_key);
      }
      break;

    default:
      APPL_TRACE_DEBUG("Unknown key type %d", key_type);
      break;
  }
  return;
}

/*******************************************************************************
 *
 * Function         bta_dm_add_blekey
 *
 * Description      This function adds an BLE Key to an security database entry.
 *                  This function shall only be called AFTER BTA_DmAddBleDevice
 *                  has been called.
 *                  It is normally called during host startup to restore all
 *                  required information stored in the NVRAM.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_add_blekey(tBTA_DM_MSG* p_data) {
  if (!BTM_SecAddBleKey(p_data->add_ble_key.bd_addr,
                        (tBTM_LE_KEY_VALUE*)&p_data->add_ble_key.blekey,
                        p_data->add_ble_key.key_type)) {
    LOG(ERROR) << "BTA_DM: Error adding BLE Key for device "
               << p_data->add_ble_key.bd_addr;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_add_ble_device
 *
 * Description      This function adds an BLE device to an security database
 *                  entry.
 *                  It is normally called during host startup to restore all
 *                  required information stored in the NVRAM.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_add_ble_device(tBTA_DM_MSG* p_data) {
  if (!BTM_SecAddBleDevice(p_data->add_ble_device.bd_addr, NULL,
                           p_data->add_ble_device.dev_type,
                           p_data->add_ble_device.addr_type)) {
    LOG(ERROR) << "BTA_DM: Error adding BLE Device for device "
               << p_data->add_ble_device.bd_addr;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_add_ble_device
 *
 * Description      This function adds an BLE device to an security database
 *                  entry.
 *                  It is normally called during host startup to restore all
 *                  required information stored in the NVRAM.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_passkey_reply(tBTA_DM_MSG* p_data) {
  if (p_data->pin_reply.accept) {
    BTM_BlePasskeyReply(p_data->ble_passkey_reply.bd_addr, BTM_SUCCESS,
                        p_data->ble_passkey_reply.passkey);
  } else {
    BTM_BlePasskeyReply(p_data->ble_passkey_reply.bd_addr, BTM_NOT_AUTHORIZED,
                        p_data->ble_passkey_reply.passkey);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_ble_confirm_reply
 *
 * Description      This is response to SM numeric comparison request submitted
 *                  to application.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_confirm_reply(tBTA_DM_MSG* p_data) {
  if (p_data->confirm.accept) {
    BTM_BleConfirmReply(p_data->confirm.bd_addr, BTM_SUCCESS);
  } else {
    BTM_BleConfirmReply(p_data->ble_passkey_reply.bd_addr, BTM_NOT_AUTHORIZED);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_security_grant
 *
 * Description      This function grant SMP security request access.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_security_grant(tBTA_DM_MSG* p_data) {
  BTM_SecurityGrant(p_data->ble_sec_grant.bd_addr, p_data->ble_sec_grant.res);
}

/*******************************************************************************
 *
 * Function         bta_dm_ble_set_bg_conn_type
 *
 * Description      This function set the BLE background connection type
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_set_bg_conn_type(tBTA_DM_MSG* p_data) {
  //TODO Discard this function or Should return error?
  LOG(INFO) << __func__ <<" %s Shouldnt get called from application";
}

/*******************************************************************************
 *
 * Function         bta_dm_ble_set_conn_params
 *
 * Description      This function set the preferred connection parameters.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_set_conn_params(tBTA_DM_MSG* p_data) {
  BTM_BleSetPrefConnParams(p_data->ble_set_conn_params.peer_bda,
                           p_data->ble_set_conn_params.conn_int_min,
                           p_data->ble_set_conn_params.conn_int_max,
                           p_data->ble_set_conn_params.slave_latency,
                           p_data->ble_set_conn_params.supervision_tout);
}

/*******************************************************************************
 *
 * Function         bta_dm_ble_set_conn_scan_params
 *
 * Description      This function set the preferred connection scan parameters.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_set_conn_scan_params(tBTA_DM_MSG* p_data) {
  BTM_BleSetConnScanParams(p_data->ble_set_conn_scan_params.scan_int,
                           p_data->ble_set_conn_scan_params.scan_window);
}
/*******************************************************************************
 *
 * Function         bta_dm_ble_update_conn_params
 *
 * Description      This function update LE connection parameters.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_update_conn_params(tBTA_DM_MSG* p_data) {
  if (!L2CA_UpdateBleConnParams(p_data->ble_update_conn_params.bd_addr,
                                p_data->ble_update_conn_params.min_int,
                                p_data->ble_update_conn_params.max_int,
                                p_data->ble_update_conn_params.latency,
                                p_data->ble_update_conn_params.timeout)) {
    APPL_TRACE_ERROR("Update connection parameters failed!");
  }
}

#if (BLE_PRIVACY_SPT == TRUE)
/*******************************************************************************
 *
 * Function         bta_dm_ble_config_local_privacy
 *
 * Description      This function set the local device LE privacy settings.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_config_local_privacy(tBTA_DM_MSG* p_data) {
  BTM_BleConfigPrivacy(p_data->ble_local_privacy.privacy_enable);
}
#endif

/*******************************************************************************
 *
 * Function         bta_dm_ble_observe
 *
 * Description      This function set the preferred connection scan parameters.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_observe(tBTA_DM_MSG* p_data) {
  tBTM_STATUS status;
  if (p_data->ble_observe.start) {
    /*Save the  callback to be called when a scan results are available */
    bta_dm_search_cb.p_scan_cback = p_data->ble_observe.p_cback;
    status = BTM_BleObserve(true, p_data->ble_observe.duration,
                            bta_dm_observe_results_cb, bta_dm_observe_cmpl_cb);
    if (status != BTM_CMD_STARTED) {
      tBTA_DM_SEARCH data;
      APPL_TRACE_WARNING(" %s BTM_BleObserve  failed. status %d", __func__,
                         status);
      data.inq_cmpl.num_resps = 0;
      if (bta_dm_search_cb.p_scan_cback) {
        bta_dm_search_cb.p_scan_cback(BTA_DM_INQ_CMPL_EVT, &data);
      }
    }
  } else {
    bta_dm_search_cb.p_scan_cback = NULL;
    BTM_BleObserve(false, 0, NULL, NULL);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_ble_set_data_length
 *
 * Description      This function set the maximum transmission packet size
 *
 * Parameters
 *
 ******************************************************************************/
void bta_dm_ble_set_data_length(tBTA_DM_MSG* p_data) {
  if (BTM_SetBleDataLength(p_data->ble_set_data_length.remote_bda,
                           p_data->ble_set_data_length.tx_data_length) !=
      BTM_SUCCESS) {
    APPL_TRACE_ERROR("%s failed", __func__);
  }
}

/*******************************************************************************
 *
 * Function         bta_ble_enable_scan_cmpl
 *
 * Description      ADV payload filtering enable / disable complete callback
 *
 *
 * Returns          None
 *
 ******************************************************************************/
static void bta_ble_energy_info_cmpl(tBTM_BLE_TX_TIME_MS tx_time,
                                     tBTM_BLE_RX_TIME_MS rx_time,
                                     tBTM_BLE_IDLE_TIME_MS idle_time,
                                     tBTM_BLE_ENERGY_USED energy_used,
                                     tBTM_STATUS status) {
  tBTA_STATUS st = (status == BTM_SUCCESS) ? BTA_SUCCESS : BTA_FAILURE;
  tBTA_DM_CONTRL_STATE ctrl_state = 0;

  if (BTA_SUCCESS == st) ctrl_state = bta_dm_pm_obtain_controller_state();

  if (bta_dm_cb.p_energy_info_cback)
    bta_dm_cb.p_energy_info_cback(tx_time, rx_time, idle_time, energy_used,
                                  ctrl_state, st);
}

/*******************************************************************************
 *
 * Function         bta_dm_ble_get_energy_info
 *
 * Description      This function obtains the energy info
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_ble_get_energy_info(tBTA_DM_MSG* p_data) {
  tBTM_STATUS btm_status = 0;

  bta_dm_cb.p_energy_info_cback = p_data->ble_energy_info.p_energy_info_cback;
  btm_status = BTM_BleGetEnergyInfo(bta_ble_energy_info_cmpl);
  if (BTM_CMD_STARTED != btm_status)
    bta_ble_energy_info_cmpl(0, 0, 0, 0, btm_status);
}

#ifndef BTA_DM_GATT_CLOSE_DELAY_TOUT
#define BTA_DM_GATT_CLOSE_DELAY_TOUT 5000
#endif

/*******************************************************************************
 *
 * Function         bta_dm_gattc_register
 *
 * Description      Register with GATTC in DM if BLE is needed.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_dm_gattc_register(void) {
  if (bta_dm_search_cb.client_if == BTA_GATTS_INVALID_IF) {
    BTA_GATTC_AppRegister(bta_dm_gattc_callback,
                          base::Bind([](uint8_t client_id, uint8_t status) {
                            if (status == GATT_SUCCESS)
                              bta_dm_search_cb.client_if = client_id;
                            else
                              bta_dm_search_cb.client_if = BTA_GATTS_INVALID_IF;

                          }));
  }
}

/*******************************************************************************
 *
 * Function         btm_dm_start_disc_gatt_services
 *
 * Description      This function starts a GATT service search request.
 *
 * Parameters:
 *
 ******************************************************************************/
static void btm_dm_start_disc_gatt_services(uint16_t conn_id) {
  Uuid* p_uuid = bta_dm_search_cb.p_srvc_uuid + bta_dm_search_cb.num_uuid -
                 bta_dm_search_cb.uuid_to_search;

  /* always search for all services */
  BTA_GATTC_ServiceSearchRequest(conn_id, p_uuid);
}

/*******************************************************************************
 *
 * Function         bta_dm_gatt_disc_result
 *
 * Description      This function process the GATT service search result.
 *
 * Parameters:
 *
 ******************************************************************************/
static void bta_dm_gatt_disc_result(tBTA_GATT_ID service_id) {
  tBTA_DM_SEARCH result;

  /*
   * This logic will not work for gatt case.  We are checking against the
   * bluetooth profiles here
   * just copy the GATTID in raw data field and send it across.
   */

  if (bta_dm_search_cb.ble_raw_used + sizeof(tBTA_GATT_ID) <
      bta_dm_search_cb.ble_raw_size) {
    APPL_TRACE_DEBUG(
        "ADDING BLE SERVICE uuid=%s, ble_ptr = 0x%x, ble_raw_used = 0x%x",
        service_id.uuid.ToString().c_str(), bta_dm_search_cb.p_ble_rawdata,
        bta_dm_search_cb.ble_raw_used);

    if (bta_dm_search_cb.p_ble_rawdata) {
      // TODO(jpawlowski): the p_ble_raw data is only sent to btif_dm.cc, but is
      // never used there. Get rid of this code completly, or implement the
      // TODOs from btif_dm.cc
      memcpy((bta_dm_search_cb.p_ble_rawdata + bta_dm_search_cb.ble_raw_used),
             &service_id, sizeof(service_id));

      bta_dm_search_cb.ble_raw_used += sizeof(service_id);
    } else {
      APPL_TRACE_ERROR("p_ble_rawdata is NULL");
    }

  } else {
    APPL_TRACE_ERROR(
        "%s out of room to accomodate more service ids ble_raw_size = %d "
        "ble_raw_used = %d",
        __func__, bta_dm_search_cb.ble_raw_size, bta_dm_search_cb.ble_raw_used);
  }

  LOG_INFO(LOG_TAG, "%s service_id_uuid_len=%zu", __func__,
           service_id.uuid.GetShortestRepresentationSize());
  if (bta_dm_search_cb.state != BTA_DM_SEARCH_IDLE) {
    /* send result back to app now, one by one */
    result.disc_ble_res.bd_addr = bta_dm_search_cb.peer_bdaddr;
    strlcpy((char*)result.disc_ble_res.bd_name, bta_dm_get_remname(),
            BD_NAME_LEN + 1);
    result.disc_ble_res.service = service_id.uuid;

    bta_dm_search_cb.p_search_cback(BTA_DM_DISC_BLE_RES_EVT, &result);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_gatt_disc_complete
 *
 * Description      This function process the GATT service search complete.
 *
 * Parameters:
 *
 ******************************************************************************/
static void bta_dm_gatt_disc_complete(uint16_t conn_id, tGATT_STATUS status) {
  APPL_TRACE_DEBUG("%s conn_id = %d", __func__, conn_id);

  if (bta_dm_search_cb.uuid_to_search > 0) bta_dm_search_cb.uuid_to_search--;

  if (status == GATT_SUCCESS && bta_dm_search_cb.uuid_to_search > 0) {
    btm_dm_start_disc_gatt_services(conn_id);
  } else {
    tBTA_DM_MSG* p_msg = (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_MSG));

    bta_dm_search_cb.uuid_to_search = 0;

    /* no more services to be discovered */
    p_msg->hdr.event = BTA_DM_DISCOVERY_RESULT_EVT;
    p_msg->disc_result.result.disc_res.result =
        (status == GATT_SUCCESS) ? BTA_SUCCESS : BTA_FAILURE;
    APPL_TRACE_DEBUG("%s service found: 0x%08x", __func__,
                     bta_dm_search_cb.services_found);
    p_msg->disc_result.result.disc_res.services =
        bta_dm_search_cb.services_found;
    p_msg->disc_result.result.disc_res.num_uuids = 0;
    p_msg->disc_result.result.disc_res.p_uuid_list = NULL;
    p_msg->disc_result.result.disc_res.bd_addr = bta_dm_search_cb.peer_bdaddr;
    strlcpy((char*)p_msg->disc_result.result.disc_res.bd_name,
            bta_dm_get_remname(), BD_NAME_LEN + 1);

    p_msg->disc_result.result.disc_res.device_type |= BT_DEVICE_TYPE_BLE;
    if (bta_dm_search_cb.ble_raw_used > 0) {
      p_msg->disc_result.result.disc_res.p_raw_data =
          (uint8_t*)osi_malloc(bta_dm_search_cb.ble_raw_used);

      memcpy(p_msg->disc_result.result.disc_res.p_raw_data,
             bta_dm_search_cb.p_ble_rawdata, bta_dm_search_cb.ble_raw_used);

      p_msg->disc_result.result.disc_res.raw_data_size =
          bta_dm_search_cb.ble_raw_used;
    } else {
      p_msg->disc_result.result.disc_res.p_raw_data = NULL;
      bta_dm_search_cb.p_ble_rawdata = 0;
    }

    bta_sys_sendmsg(p_msg);

    if (conn_id != GATT_INVALID_CONN_ID) {
      /* start a GATT channel close delay timer */
      bta_sys_start_timer(bta_dm_search_cb.gatt_close_timer,
                          BTA_DM_GATT_CLOSE_DELAY_TOUT,
                          BTA_DM_DISC_CLOSE_TOUT_EVT, 0);
      bta_dm_search_cb.pending_close_bda = bta_dm_search_cb.peer_bdaddr;
    }
    bta_dm_search_cb.gatt_disc_active = false;
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_close_gatt_conn
 *
 * Description      This function close the GATT connection after delay
 *timeout.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_close_gatt_conn(UNUSED_ATTR tBTA_DM_MSG* p_data) {
  if (bta_dm_search_cb.conn_id != GATT_INVALID_CONN_ID)
    BTA_GATTC_Close(bta_dm_search_cb.conn_id);

  bta_dm_search_cb.pending_close_bda = RawAddress::kEmpty;
  bta_dm_search_cb.conn_id = GATT_INVALID_CONN_ID;
}

/*******************************************************************************
 *
 * Function         bta_dm_queue_service_disc
 *
 * Description      This function queues the service discovery request info
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_queue_service_disc(tBTA_DM_MSG* p_data) {
  APPL_TRACE_DEBUG("bta_dm_queue_service_disc");

  tBTA_DM_MSG* p_service_disc_data =
       (tBTA_DM_MSG*)osi_malloc(sizeof(tBTA_DM_API_DISCOVER));
  memcpy(p_service_disc_data, p_data, sizeof(tBTA_DM_API_DISCOVER));
  bta_dm_search_cb.p_disc_queue.push(p_service_disc_data);
}

/*******************************************************************************
 *
 * Function         btm_dm_start_gatt_discovery
 *
 * Description      This is GATT initiate the service search by open a GATT
 *                  connection first.
 *
 * Parameters:
 *
 ******************************************************************************/
void btm_dm_start_gatt_discovery(const RawAddress& bd_addr) {
  APPL_TRACE_DEBUG("btm_dm_start_gatt_discovery");
  bta_dm_search_cb.gatt_disc_active = true;

  /* connection is already open */
  if (bta_dm_search_cb.pending_close_bda == bd_addr &&
      bta_dm_search_cb.conn_id != GATT_INVALID_CONN_ID) {
    bta_dm_search_cb.pending_close_bda = RawAddress::kEmpty;
    alarm_cancel(bta_dm_search_cb.gatt_close_timer);
    btm_dm_start_disc_gatt_services(bta_dm_search_cb.conn_id);
  } else {
    if (BTM_IsAclConnectionUp(bd_addr, BT_TRANSPORT_LE)) {
      BTA_GATTC_Open(bta_dm_search_cb.client_if, bd_addr, true,
                     GATT_TRANSPORT_LE, true);
    } else {
      //TODO review. Kept qcom Specific change only.
      APPL_TRACE_DEBUG("btm_dm_start_gatt_discovery: ACL is disconnected");
      /* don't create ACL for GATT discovery if ACL already disconnected */
          APPL_TRACE_DEBUG("btm_dm_start_gatt_discovery: Not creating acl"
            " for client_if = %d", bta_dm_search_cb.client_if);
          if (bta_dm_search_cb.gatt_disc_active) {
            bta_dm_cancel_gatt_discovery(bd_addr);
          }
          bta_dm_search_cb.gatt_disc_active = false;
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_cancel_gatt_discovery
 *
 * Description      This is GATT cancel the GATT service search.
 *
 * Parameters:
 *
 ******************************************************************************/
static void bta_dm_cancel_gatt_discovery(const RawAddress& bd_addr) {
  if (bta_dm_search_cb.conn_id == GATT_INVALID_CONN_ID) {
    BTA_GATTC_CancelOpen(bta_dm_search_cb.client_if, bd_addr, true);
  }

  bta_dm_gatt_disc_complete(bta_dm_search_cb.conn_id, (tGATT_STATUS)GATT_ERROR);
}

/*******************************************************************************
 *
 * Function         bta_dm_proc_open_evt
 *
 * Description      process BTA_GATTC_OPEN_EVT in DM.
 *
 * Parameters:
 *
 ******************************************************************************/
void bta_dm_proc_open_evt(tBTA_GATTC_OPEN* p_data) {
  VLOG(1) << "DM Search state= " << bta_dm_search_cb.state
          << " search_cb.peer_dbaddr:" << bta_dm_search_cb.peer_bdaddr
          << " connected_bda=" << p_data->remote_bda;

  APPL_TRACE_DEBUG("BTA_GATTC_OPEN_EVT conn_id = %d client_if=%d status = %d",
                   p_data->conn_id, p_data->client_if, p_data->status);

  bta_dm_search_cb.conn_id = p_data->conn_id;

  if (p_data->status == GATT_SUCCESS) {
    btm_dm_start_disc_gatt_services(p_data->conn_id);
  } else {
    bta_dm_gatt_disc_complete(GATT_INVALID_CONN_ID, p_data->status);
  }
}

/*******************************************************************************
 *
 * Function         bta_dm_gattc_callback
 *
 * Description      This is GATT client callback function used in DM.
 *
 * Parameters:
 *
 ******************************************************************************/
static void bta_dm_gattc_callback(tBTA_GATTC_EVT event, tBTA_GATTC* p_data) {
  APPL_TRACE_DEBUG("bta_dm_gattc_callback event = %d", event);

  switch (event) {
    case BTA_GATTC_OPEN_EVT:
      bta_dm_proc_open_evt(&p_data->open);
      break;

    case BTA_GATTC_SEARCH_RES_EVT:
      bta_dm_gatt_disc_result(p_data->srvc_res.service_uuid);
      break;

    case BTA_GATTC_SEARCH_CMPL_EVT:
      if (bta_dm_search_cb.state != BTA_DM_SEARCH_IDLE)
        bta_dm_gatt_disc_complete(p_data->search_cmpl.conn_id,
                                  p_data->search_cmpl.status);
      break;

    case BTA_GATTC_CLOSE_EVT:
      APPL_TRACE_DEBUG("BTA_GATTC_CLOSE_EVT reason = %d, data conn_id %d, search conn_id %d",
                       p_data->close.reason,p_data->close.conn_id,bta_dm_search_cb.conn_id);
       if(p_data->close.conn_id == bta_dm_search_cb.conn_id)
          bta_dm_search_cb.conn_id = GATT_INVALID_CONN_ID;
      /* in case of disconnect before search is completed */
      if ((bta_dm_search_cb.state != BTA_DM_SEARCH_IDLE) &&
          (bta_dm_search_cb.state != BTA_DM_SEARCH_ACTIVE) &&
          p_data->close.remote_bda == bta_dm_search_cb.peer_bdaddr) {
        bta_dm_gatt_disc_complete((uint16_t)GATT_INVALID_CONN_ID,
                                  (tGATT_STATUS)GATT_ERROR);
      }
      break;

    default:
      break;
  }
}

#if (BLE_VND_INCLUDED == TRUE)
/*******************************************************************************
 *
 * Function         bta_dm_ctrl_features_rd_cmpl_cback
 *
 * Description      callback to handle controller feature read complete
 *
 * Parameters:
 *
 ******************************************************************************/
static void bta_dm_ctrl_features_rd_cmpl_cback(tBTM_STATUS result) {
  APPL_TRACE_DEBUG("%s  status = %d ", __func__, result);
  if (result == BTM_SUCCESS) {
    if (bta_dm_cb.p_sec_cback)
      bta_dm_cb.p_sec_cback(BTA_DM_LE_FEATURES_READ, NULL);
  } else {
    APPL_TRACE_ERROR("%s Ctrl BLE feature read failed: status :%d", __func__,
                     result);
  }
}
#endif /* BLE_VND_INCLUDED */

static void bta_dm_bond_retrail_cback(void* data) {

 APPL_TRACE_EVENT("%s: retry bond", __func__);

 tBTM_STATUS status;
 tBTA_DM_SEC sec_event;
 char* p_name;
 tBTA_DM_MSG* p_data = (tBTA_DM_MSG*) data;

 if (p_data->bond.transport == BTA_TRANSPORT_UNKNOWN)
    status = BTM_SecBond(p_data->bond.bd_addr, 0, NULL, 0);
 else
    status = BTM_SecBondByTransport(p_data->bond.bd_addr,
                             p_data->bond.transport, 0, NULL, 0);

  if (bta_dm_cb.p_sec_cback && (status != BTM_CMD_STARTED)) {
    memset(&sec_event, 0, sizeof(tBTA_DM_SEC));
    sec_event.auth_cmpl.bd_addr = p_data->bond.bd_addr;
    p_name = BTM_SecReadDevName(p_data->bond.bd_addr);
    if (p_name != NULL) {
      memcpy(sec_event.auth_cmpl.bd_name, p_name, BD_NAME_LEN);
      sec_event.auth_cmpl.bd_name[BD_NAME_LEN] = 0;
    }

    sec_event.auth_cmpl.fail_reason = HCI_ERR_ILLEGAL_COMMAND;
    if (status == BTM_SUCCESS) {
      sec_event.auth_cmpl.success = true;
    } else {
     /* delete this device entry from Sec Dev DB */
      bta_dm_remove_sec_dev_entry(p_data->bond.bd_addr);
    }
    bta_dm_cb.p_sec_cback(BTA_DM_AUTH_CMPL_EVT, &sec_event);
  }
  osi_free(data);
}
