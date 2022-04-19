/******************************************************************************
 *
 *  Copyright (C) 2003-2012 Broadcom Corporation
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
 *  This is the private interface file for the BTA device manager.
 *
 ******************************************************************************/
#ifndef BTA_DM_INT_H
#define BTA_DM_INT_H

#include <memory>
#include "bt_target.h"
#include "bta_sys.h"

#include "bta_gatt_api.h"

/*****************************************************************************
 *  Constants and data types
 ****************************************************************************/

#define BTA_COPY_DEVICE_CLASS(coddst, codsrc)          \
  {                                                    \
    ((uint8_t*)(coddst))[0] = ((uint8_t*)(codsrc))[0]; \
    ((uint8_t*)(coddst))[1] = ((uint8_t*)(codsrc))[1]; \
    ((uint8_t*)(coddst))[2] = ((uint8_t*)(codsrc))[2]; \
  }

#define BTA_DM_MSG_LEN 50

#define BTA_SERVICE_ID_TO_SERVICE_MASK(id) (1 << (id))

/* DM events */
enum {
  /* device manager local device API events */
  BTA_DM_API_ENABLE_EVT = BTA_SYS_EVT_START(BTA_ID_DM),
  BTA_DM_API_DISABLE_EVT,
  BTA_DM_API_SET_NAME_EVT,
  BTA_DM_API_SET_VISIBILITY_EVT,

  BTA_DM_ACL_CHANGE_EVT,
  BTA_DM_API_ADD_DEVICE_EVT,
  BTA_DM_API_REMOVE_ACL_EVT,

  /* security API events */
  BTA_DM_API_BOND_EVT,
  BTA_DM_API_BOND_CANCEL_EVT,
  BTA_DM_API_PIN_REPLY_EVT,

  /* power manger events */
  BTA_DM_PM_BTM_STATUS_EVT,
  BTA_DM_PM_TIMER_EVT,

  /* simple pairing events */
  BTA_DM_API_CONFIRM_EVT,

  BTA_DM_API_SET_ENCRYPTION_EVT,

  BTA_DM_API_LOC_OOB_EVT,
  BTA_DM_CI_IO_REQ_EVT,
  BTA_DM_CI_RMT_OOB_EVT,

  BTA_DM_API_ADD_BLEKEY_EVT,
  BTA_DM_API_ADD_BLEDEVICE_EVT,
  BTA_DM_API_BLE_PASSKEY_REPLY_EVT,
  BTA_DM_API_BLE_CONFIRM_REPLY_EVT,
  BTA_DM_API_BLE_SEC_GRANT_EVT,
  BTA_DM_API_BLE_SET_BG_CONN_TYPE,
  BTA_DM_API_BLE_CONN_PARAM_EVT,
  BTA_DM_API_BLE_CONN_SCAN_PARAM_EVT,
  BTA_DM_API_BLE_OBSERVE_EVT,
  BTA_DM_API_UPDATE_CONN_PARAM_EVT,
#if (BLE_PRIVACY_SPT == TRUE)
  BTA_DM_API_LOCAL_PRIVACY_EVT,
#endif
  BTA_DM_API_SET_DATA_LENGTH_EVT,
  BTA_DM_API_BLE_ENERGY_INFO_EVT,

  BTA_DM_API_ENABLE_TEST_MODE_EVT,
  BTA_DM_API_DISABLE_TEST_MODE_EVT,
  BTA_DM_API_EXECUTE_CBACK_EVT,
  BTA_DM_API_REMOVE_ALL_ACL_EVT,
  BTA_DM_API_REMOVE_DEVICE_EVT,
  BTA_DM_API_HCI_RAW_COMMAND_EVT,
  BTA_DM_API_SET_WIFI_STATE_EVT,
  BTA_DM_API_SET_PWR_BACKOFF_EVT,
  BTA_DM_API_IOT_REPORT_EVT,
  BTA_DM_API_BREDR_CLEANUP_EVT,
  BTA_DM_API_BREDR_STARTUP_EVT,
  BTA_DM_API_RST_PAIR_FLAG_EVT,
  BTA_DM_MAX_EVT
};

/* DM search events */
enum {
  /* DM search API events */
  BTA_DM_API_SEARCH_EVT = BTA_SYS_EVT_START(BTA_ID_DM_SEARCH),
  BTA_DM_API_SEARCH_CANCEL_EVT,
  BTA_DM_API_DISCOVER_EVT,
  BTA_DM_INQUIRY_CMPL_EVT,
  BTA_DM_REMT_NAME_EVT,
  BTA_DM_SDP_RESULT_EVT,
  BTA_DM_SEARCH_CMPL_EVT,
  BTA_DM_DISCOVERY_RESULT_EVT,
  BTA_DM_API_DI_DISCOVER_EVT,
  BTA_DM_DISC_CLOSE_TOUT_EVT

};

/* data type for BTA_DM_API_ENABLE_EVT */
typedef struct {
  BT_HDR hdr;
  tBTA_DM_SEC_CBACK* p_sec_cback;
} tBTA_DM_API_ENABLE;

/* data type for BTA_DM_API_SET_NAME_EVT */
typedef struct {
  BT_HDR hdr;
  BD_NAME name; /* max 248 bytes name, plus must be Null terminated */
} tBTA_DM_API_SET_NAME;

/* data type for BTA_DM_API_SET_VISIBILITY_EVT */
typedef struct {
  BT_HDR hdr;
  tBTA_DM_DISC disc_mode;
  tBTA_DM_CONN conn_mode;
  uint8_t pair_mode;
  uint8_t conn_paired_only;
} tBTA_DM_API_SET_VISIBILITY;

/* data type for BTA_DM_API_HCI_RAW_COMMAND_EVT */
typedef struct
{
  BT_HDR hdr;
  uint16_t opcode;
  uint8_t param_len;
  uint8_t *p_param_buf;
  tBTA_RAW_CMPL_CBACK *p_cback;
} tBTA_DM_API_RAW_COMMAND;
enum {
  BTA_DM_RS_NONE, /* straight API call */
  BTA_DM_RS_OK,   /* the role switch result - successful */
  BTA_DM_RS_FAIL  /* the role switch result - failed */
};
typedef uint8_t tBTA_DM_RS_RES;

/* data type for BTA_DM_API_SEARCH_EVT */
typedef struct {
  BT_HDR hdr;
  tBTA_DM_INQ inq_params;
  tBTA_SERVICE_MASK services;
  tBTA_DM_SEARCH_CBACK* p_cback;
  tBTA_DM_RS_RES rs_res;
  uint8_t num_uuid;
  bluetooth::Uuid* p_uuid;
} tBTA_DM_API_SEARCH;

/* data type for BTA_DM_API_DISCOVER_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTA_SERVICE_MASK services;
  tBTA_DM_SEARCH_CBACK* p_cback;
  bool sdp_search;
  tBTA_TRANSPORT transport;
  uint8_t num_uuid;
  bluetooth::Uuid* p_uuid;
  bluetooth::Uuid uuid;
} tBTA_DM_API_DISCOVER;

/* data type for BTA_DM_API_DI_DISC_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTA_DISCOVERY_DB* p_sdp_db;
  uint32_t len;
  tBTA_DM_SEARCH_CBACK* p_cback;
} tBTA_DM_API_DI_DISC;

/* data type for BTA_DM_API_BOND_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTA_TRANSPORT transport;
} tBTA_DM_API_BOND;

/* data type for BTA_DM_API_BOND_CANCEL_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTA_TRANSPORT transport;
} tBTA_DM_API_BOND_CANCEL;

/* data type for BTA_DM_API_PIN_REPLY_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  bool accept;
  uint8_t pin_len;
  uint8_t p_pin[PIN_CODE_LEN];
} tBTA_DM_API_PIN_REPLY;

/* data type for BTA_DM_API_LOC_OOB_EVT */
typedef struct { BT_HDR hdr; } tBTA_DM_API_LOC_OOB;

/* data type for BTA_DM_API_CONFIRM_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  bool accept;
} tBTA_DM_API_CONFIRM;

/* data type for BTA_DM_CI_IO_REQ_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTA_IO_CAP io_cap;
  tBTA_OOB_DATA oob_data;
  tBTA_AUTH_REQ auth_req;
} tBTA_DM_CI_IO_REQ;

/* data type for BTA_DM_CI_RMT_OOB_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  Octet16 c;
  Octet16 r;
  bool accept;
} tBTA_DM_CI_RMT_OOB;

/* data type for BTA_DM_REMT_NAME_EVT */
typedef struct {
  BT_HDR hdr;
  tBTA_DM_SEARCH result;
} tBTA_DM_REM_NAME;

/* data type for tBTA_DM_DISC_RESULT */
typedef struct {
  BT_HDR hdr;
  tBTA_DM_SEARCH result;
} tBTA_DM_DISC_RESULT;

/* data type for BTA_DM_INQUIRY_CMPL_EVT */
typedef struct {
  BT_HDR hdr;
  uint8_t num;
} tBTA_DM_INQUIRY_CMPL;

/* data type for BTA_DM_SDP_RESULT_EVT */
typedef struct {
  BT_HDR hdr;
  uint16_t sdp_result;
} tBTA_DM_SDP_RESULT;

/* data type for BTA_DM_ACL_CHANGE_EVT */
typedef struct {
  BT_HDR hdr;
  tBTM_BL_EVENT event;
  uint8_t busy_level;
  uint8_t busy_level_flags;
  bool is_new;
  uint8_t new_role;
  RawAddress bd_addr;
  uint8_t hci_status;
  uint16_t pkt_type;
  uint16_t soc_log_id;
  uint16_t handle;
  tBT_TRANSPORT transport;
} tBTA_DM_ACL_CHANGE;

/* data type for BTA_DM_API_IOT_REPORT_EVT*/
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  uint16_t error_type;
  uint16_t error_info;
  uint32_t event_mask;
  uint8_t event_power_level;
  int8_t event_rssi;
  uint8_t event_link_quality;
  uint16_t event_glitch_count;
} tBTA_DM_VND_IOT_REPORT;

/* data type for BTA_DM_PM_BTM_STATUS_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTM_PM_STATUS status;
  uint16_t value;
  uint8_t hci_status;

} tBTA_DM_PM_BTM_STATUS;

/* data type for BTA_DM_PM_TIMER_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTA_DM_PM_ACTION pm_request;
} tBTA_DM_PM_TIMER;

/* data type for BTA_DM_API_ADD_DEVICE_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  DEV_CLASS dc;
  LinkKey link_key;
  tBTA_SERVICE_MASK tm;
  bool is_trusted;
  uint8_t key_type;
  tBTA_IO_CAP io_cap;
  bool link_key_known;
  bool dc_known;
  BD_NAME bd_name;
  uint8_t
      features[BTA_FEATURE_BYTES_PER_PAGE * (BTA_EXT_FEATURES_PAGE_MAX + 1)];
  uint8_t pin_length;
} tBTA_DM_API_ADD_DEVICE;

/* data type for BTA_DM_API_REMOVE_ACL_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
} tBTA_DM_API_REMOVE_DEVICE;

/* data type for BTA_DM_API_RST_PAIR_FLAG_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
} tBTA_DM_API_RST_PAIR_FLAG;

/* data type for BTA_DM_API_SET_WIFI_STATE_EVT */
typedef struct
{
  BT_HDR hdr;
  bool status;
} tBTA_DM_API_SET_WIFI_STATE;

/* data type for tBTA_DM_API_SET_PWR_BACKOFF */
typedef struct
{
  BT_HDR hdr;
  bool status;
} tBTA_DM_API_SET_PWR_BACKOFF;

/* data type for BTA_DM_API_BREDR_CLEANUP_EVT */
typedef struct
{
  BT_HDR hdr;
} tBTA_DM_API_BREDR_CLEANUP;

/* data type for BTA_DM_API_BREDR_STARTUP_EVT */
typedef struct
{
  BT_HDR hdr;
} tBTA_DM_API_BREDR_STARTUP;

/* data type for BTA_DM_API_EXECUTE_CBACK_EVT */
typedef struct {
  BT_HDR hdr;
  void* p_param;
  tBTA_DM_EXEC_CBACK* p_exec_cback;
} tBTA_DM_API_EXECUTE_CBACK;

/* data type for tBTA_DM_API_SET_ENCRYPTION */
typedef struct {
  BT_HDR hdr;
  tBTA_TRANSPORT transport;
  tBTA_DM_ENCRYPT_CBACK* p_callback;
  tBTA_DM_BLE_SEC_ACT sec_act;
  RawAddress bd_addr;
} tBTA_DM_API_SET_ENCRYPTION;

typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTA_LE_KEY_VALUE blekey;
  tBTA_LE_KEY_TYPE key_type;

} tBTA_DM_API_ADD_BLEKEY;

typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBT_DEVICE_TYPE dev_type;
  tBLE_ADDR_TYPE addr_type;

} tBTA_DM_API_ADD_BLE_DEVICE;

typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  bool accept;
  uint32_t passkey;
} tBTA_DM_API_PASSKEY_REPLY;

typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  tBTA_DM_BLE_SEC_GRANT res;
} tBTA_DM_API_BLE_SEC_GRANT;

/* set prefered BLE connection parameters for a device */
typedef struct {
  BT_HDR hdr;
  RawAddress peer_bda;
  uint16_t conn_int_min;
  uint16_t conn_int_max;
  uint16_t supervision_tout;
  uint16_t slave_latency;

} tBTA_DM_API_BLE_CONN_PARAMS;

typedef struct {
  BT_HDR hdr;
  RawAddress peer_bda;
  bool privacy_enable;

} tBTA_DM_API_ENABLE_PRIVACY;

typedef struct {
  BT_HDR hdr;
  bool privacy_enable;
} tBTA_DM_API_LOCAL_PRIVACY;

/* set scan parameter for BLE connections */
typedef struct {
  BT_HDR hdr;
  tGATT_IF client_if;
  uint32_t scan_int;
  uint32_t scan_window;
  tBLE_SCAN_MODE scan_mode;
  tBLE_SCAN_PARAM_SETUP_CBACK scan_param_setup_cback;
} tBTA_DM_API_BLE_SCAN_PARAMS;

/* set scan parameter for BLE connections */
typedef struct {
  BT_HDR hdr;
  uint16_t scan_int;
  uint16_t scan_window;
} tBTA_DM_API_BLE_CONN_SCAN_PARAMS;

/* Data type for start/stop observe */
typedef struct {
  BT_HDR hdr;
  bool start;
  uint16_t duration;
  tBTA_DM_SEARCH_CBACK* p_cback;
} tBTA_DM_API_BLE_OBSERVE;

typedef struct {
  BT_HDR hdr;
  RawAddress remote_bda;
  uint16_t tx_data_length;
} tBTA_DM_API_BLE_SET_DATA_LENGTH;

/* set adv parameter for BLE advertising */

typedef struct {
  BT_HDR hdr;
  bool enable;

} tBTA_DM_API_BLE_FEATURE;

typedef struct {
  BT_HDR hdr;
  tBTA_BLE_ENERGY_INFO_CBACK* p_energy_info_cback;
} tBTA_DM_API_ENERGY_INFO;

/* data type for BTA_DM_API_REMOVE_ACL_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  bool remove_dev;
  tBTA_TRANSPORT transport;
} tBTA_DM_API_REMOVE_ACL;

/* data type for BTA_DM_API_REMOVE_ALL_ACL_EVT */
typedef struct {
  BT_HDR hdr;
  tBTA_DM_LINK_TYPE link_type;

} tBTA_DM_API_REMOVE_ALL_ACL;
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  uint16_t min_int;
  uint16_t max_int;
  uint16_t latency;
  uint16_t timeout;
  uint16_t min_ce_len;
  uint16_t max_ce_len;
} tBTA_DM_API_UPDATE_CONN_PARAM;

/* union of all data types */
typedef union {
  /* GKI event buffer header */
  BT_HDR hdr;
  tBTA_DM_API_ENABLE enable;

  tBTA_DM_API_SET_NAME set_name;

  tBTA_DM_API_SET_VISIBILITY set_visibility;

  tBTA_DM_API_ADD_DEVICE add_dev;

  tBTA_DM_API_REMOVE_DEVICE remove_dev;

  tBTA_DM_API_SEARCH search;

  tBTA_DM_API_DISCOVER discover;

  tBTA_DM_API_BOND bond;

  tBTA_DM_API_BOND_CANCEL bond_cancel;

  tBTA_DM_API_PIN_REPLY pin_reply;

  tBTA_DM_API_LOC_OOB loc_oob;
  tBTA_DM_API_CONFIRM confirm;
  tBTA_DM_CI_IO_REQ ci_io_req;
  tBTA_DM_CI_RMT_OOB ci_rmt_oob;

  tBTA_DM_REM_NAME rem_name;

  tBTA_DM_DISC_RESULT disc_result;

  tBTA_DM_INQUIRY_CMPL inq_cmpl;

  tBTA_DM_SDP_RESULT sdp_event;

  tBTA_DM_ACL_CHANGE acl_change;

  tBTA_DM_PM_BTM_STATUS pm_status;

  tBTA_DM_PM_TIMER pm_timer;

  tBTA_DM_API_DI_DISC di_disc;

  tBTA_DM_API_EXECUTE_CBACK exec_cback;

  tBTA_DM_API_SET_ENCRYPTION set_encryption;

  tBTA_DM_API_ADD_BLEKEY add_ble_key;
  tBTA_DM_API_ADD_BLE_DEVICE add_ble_device;
  tBTA_DM_API_PASSKEY_REPLY ble_passkey_reply;
  tBTA_DM_API_BLE_SEC_GRANT ble_sec_grant;
  tBTA_DM_API_BLE_CONN_PARAMS ble_set_conn_params;
  tBTA_DM_API_BLE_CONN_SCAN_PARAMS ble_set_conn_scan_params;
  tBTA_DM_API_BLE_OBSERVE ble_observe;
  tBTA_DM_API_ENABLE_PRIVACY ble_remote_privacy;
  tBTA_DM_API_LOCAL_PRIVACY ble_local_privacy;
  tBTA_DM_API_UPDATE_CONN_PARAM ble_update_conn_params;
  tBTA_DM_API_BLE_SET_DATA_LENGTH ble_set_data_length;

  tBTA_DM_API_ENERGY_INFO ble_energy_info;

  tBTA_DM_API_REMOVE_ACL remove_acl;
  tBTA_DM_API_REMOVE_ALL_ACL remove_all_acl;
  tBTA_DM_API_RAW_COMMAND btc_command;
  tBTA_DM_API_SET_WIFI_STATE wifi_state;
  tBTA_DM_API_SET_PWR_BACKOFF pwr_backoff_state;
  tBTA_DM_VND_IOT_REPORT iot_info;
  tBTA_DM_API_BREDR_CLEANUP  bredr_cleanup;
  tBTA_DM_API_BREDR_STARTUP  bredr_startup;
  tBTA_DM_API_RST_PAIR_FLAG pair_state;
} tBTA_DM_MSG;

#define BTA_DM_NUM_PEER_DEVICE MAX_L2CAP_LINKS

#define BTA_DM_NOT_CONNECTED 0
#define BTA_DM_CONNECTED 1
#define BTA_DM_UNPAIRING 2
typedef uint8_t tBTA_DM_CONN_STATE;

#define BTA_DM_DI_NONE 0x00 /* nothing special */
#define BTA_DM_DI_USE_SSR \
  0x10 /* set this bit if ssr is supported for this link */
#define BTA_DM_DI_AV_ACTIVE \
  0x20 /* set this bit if AV is active for this link */
#define BTA_DM_DI_SET_SNIFF \
  0x01 /* set this bit if call BTM_SetPowerMode(sniff) */
#define BTA_DM_DI_INT_SNIFF \
  0x02 /* set this bit if call BTM_SetPowerMode(sniff) & enter sniff mode */
#define BTA_DM_DI_ACP_SNIFF 0x04 /* set this bit if peer init sniff */
typedef uint8_t tBTA_DM_DEV_INFO;

/* set power mode request type */
#define BTA_DM_PM_RESTART 1
#define BTA_DM_PM_NEW_REQ 2
#define BTA_DM_PM_EXECUTE 3
typedef uint8_t tBTA_DM_PM_REQ;

typedef struct {
  RawAddress peer_bdaddr;
  uint16_t link_policy;
  tBTA_DM_CONN_STATE conn_state;
  tBTA_PREF_ROLES pref_role;
  bool in_use;
  tBTA_DM_DEV_INFO info;
  tBTA_DM_ENCRYPT_CBACK* p_encrypt_cback;
#if (BTM_SSR_INCLUDED == TRUE)
  tBTM_PM_STATUS prev_low; /* previous low power mode used */
#endif
  tBTA_DM_PM_ACTION pm_mode_attempted;
  tBTA_DM_PM_ACTION pm_mode_failed;
  bool remove_dev_pending;
  uint16_t conn_handle;
  tBT_TRANSPORT transport;
} tBTA_DM_PEER_DEVICE;

/* structure to store list of
  active connections */
typedef struct {
  tBTA_DM_PEER_DEVICE peer_device[BTA_DM_NUM_PEER_DEVICE];
  uint8_t count;
  uint8_t le_count;
} tBTA_DM_ACTIVE_LINK;

typedef struct {
  RawAddress peer_bdaddr;
  tBTA_SYS_ID id;
  uint8_t app_id;
  tBTA_SYS_CONN_STATUS state;
  bool new_request;

} tBTA_DM_SRVCS;

#ifndef BTA_DM_NUM_CONN_SRVS
#define BTA_DM_NUM_CONN_SRVS 30
#endif

typedef struct {
  uint8_t count;
  tBTA_DM_SRVCS conn_srvc[BTA_DM_NUM_CONN_SRVS];

} tBTA_DM_CONNECTED_SRVCS;

typedef struct {
#define BTA_DM_PM_SNIFF_TIMER_IDX 0
#define BTA_DM_PM_PARK_TIMER_IDX 1
#define BTA_DM_PM_SUSPEND_TIMER_IDX 2
#define BTA_DM_PM_MODE_TIMER_MAX 3
  /*
   * Keep three different timers for PARK, SNIFF and SUSPEND if TBFC is
   * supported.
   */
  alarm_t* timer[BTA_DM_PM_MODE_TIMER_MAX];

  uint8_t srvc_id[BTA_DM_PM_MODE_TIMER_MAX];
  uint8_t pm_action[BTA_DM_PM_MODE_TIMER_MAX];
  uint8_t active; /* number of active timer */

  RawAddress peer_bdaddr;
  bool in_use;
} tBTA_PM_TIMER;

extern tBTA_DM_CONNECTED_SRVCS bta_dm_conn_srvcs;

#define BTA_DM_NUM_PM_TIMER 7

/* DM control block */
typedef struct {
  bool is_bta_dm_active;
  int bta_dm_hw_status;
  tBTA_DM_ACTIVE_LINK device_list;
  tBTA_DM_SEC_CBACK* p_sec_cback;
  tBTA_BLE_ENERGY_INFO_CBACK* p_energy_info_cback;
  uint16_t state;
  bool disabling;
  alarm_t* disable_timer;
  alarm_t* bond_retrail_timer;
  uint32_t wbt_sdp_handle; /* WIDCOMM Extensions SDP record handle */
  uint8_t wbt_scn;         /* WIDCOMM Extensions SCN */
  uint8_t num_master_only;
  uint8_t pm_id;
  tBTA_PM_TIMER pm_timer[BTA_DM_NUM_PM_TIMER];
  uint32_t
      role_policy_mask;   /* the bits set indicates the modules that wants to
                             remove role switch from the default link policy */
  uint16_t cur_policy;    /* current default link policy */
  uint16_t rs_event;      /* the event waiting for role switch */
  uint8_t cur_av_count;   /* current AV connecions */
  bool disable_pair_mode; /* disable pair mode or not */
  bool conn_paired_only;  /* allow connectable to paired device only or not */
  tBTA_DM_API_SEARCH search_msg;
  uint16_t page_scan_interval;
  uint16_t page_scan_window;
  uint16_t inquiry_scan_interval;
  uint16_t inquiry_scan_window;

  /* Storage for pin code request parameters */
  RawAddress pin_bd_addr;
  DEV_CLASS pin_dev_class;
  tBTA_DM_SEC_EVT pin_evt;
  tBTA_IO_CAP                 loc_io_caps;    /* IO Capabilities of local device */
  tBTA_IO_CAP                 rmt_io_caps;    /* IO Capabilities of remote device */
  tBTA_AUTH_REQ               loc_auth_req;   /* Authentication required for local device */
  tBTA_AUTH_REQ               rmt_auth_req;   /* Authentication required for remote device */
  uint32_t num_val; /* the numeric value for comparison. If just_works, do not
                       show this number to UI */
  bool just_works;  /* true, if "Just Works" association model */
#if (BTA_EIR_CANNED_UUID_LIST != TRUE)
  /* store UUID list for EIR */
  uint32_t eir_uuid[BTM_EIR_SERVICE_ARRAY_SIZE];
#if (BTA_EIR_SERVER_NUM_CUSTOM_UUID > 0)
  bluetooth::Uuid custom_uuid[BTA_EIR_SERVER_NUM_CUSTOM_UUID];
#endif

#endif

  tBTA_DM_ENCRYPT_CBACK* p_encrypt_cback;
  alarm_t* switch_delay_timer;

} tBTA_DM_CB;

/* DM search control block */
typedef struct {
  tBTA_DM_SEARCH_CBACK* p_search_cback;
  tBTM_INQ_INFO* p_btm_inq_info;
  tBTA_SERVICE_MASK services;
  tBTA_SERVICE_MASK services_to_search;
  tBTA_SERVICE_MASK services_found;
  tSDP_DISCOVERY_DB* p_sdp_db;
  uint16_t state;
  RawAddress peer_bdaddr;
  bool name_discover_done;
  BD_NAME peer_name;
  alarm_t* search_timer;
  uint8_t service_index;
  tBTA_DM_MSG* p_search_queue; /* search or discover commands during search
                                  cancel stored here */
  bool wait_disc;
  bool sdp_results;
  bluetooth::Uuid uuid;
  uint8_t peer_scn;
  bool sdp_search;
  bool cancel_pending; /* inquiry cancel is pending */
  tBTA_TRANSPORT transport;
  tBTA_DM_SEARCH_CBACK* p_scan_cback;
  tGATT_IF client_if;
  uint8_t num_uuid;
  bluetooth::Uuid* p_srvc_uuid;
  uint8_t uuid_to_search;
  bool gatt_disc_active;
  uint16_t conn_id;
  uint8_t* p_ble_rawdata;
  uint32_t ble_raw_size;
  uint32_t ble_raw_used;
  alarm_t* gatt_close_timer; /* GATT channel close delay timer */
  RawAddress pending_close_bda; /* pending GATT channel remote device address */
  std::queue<tBTA_DM_MSG *> p_disc_queue;
} tBTA_DM_SEARCH_CB;

/* DI control block */
typedef struct {
  tSDP_DISCOVERY_DB* p_di_db;         /* pointer to the DI discovery database */
  uint8_t di_num;                     /* total local DI record number */
  uint32_t di_handle[BTA_DI_NUM_MAX]; /* local DI record handle, the first one
                                         is primary record */
} tBTA_DM_DI_CB;

/* DM search state */
enum {

  BTA_DM_SEARCH_IDLE,
  BTA_DM_SEARCH_ACTIVE,
  BTA_DM_SEARCH_CANCELLING,
  BTA_DM_DISCOVER_ACTIVE

};

/* DM HW status event state */
enum {

  BTA_DM_HW_OFF = 1,
  BTA_DM_HW_TURNING_ON,
  BTA_DM_HW_ON,
  BTA_DM_HW_TURNING_OFF,

};

typedef struct {
  DEV_CLASS dev_class; /* local device class */
  uint16_t
      policy_settings;   /* link policy setting hold, sniff, park, MS switch */
  uint16_t page_timeout; /* timeout for page in slots */
  uint16_t link_timeout; /* link supervision timeout in slots */
  bool avoid_scatter; /* true to avoid scatternet when av is streaming (be the
                         master) */

} tBTA_DM_CFG;

extern const uint32_t bta_service_id_to_btm_srv_id_lkup_tbl[];

typedef struct {
  uint8_t id;
  uint8_t app_id;
  uint8_t cfg;

} tBTA_DM_RM;

extern tBTA_DM_CFG* p_bta_dm_cfg;
extern tBTA_DM_RM* p_bta_dm_rm_cfg;

typedef struct {
  uint8_t id;
  uint8_t app_id;
  uint8_t spec_idx; /* index of spec table to use */

} tBTA_DM_PM_CFG;

typedef struct {
  tBTA_DM_PM_ACTION power_mode;
  uint16_t timeout;

} tBTA_DM_PM_ACTN;

typedef struct {
  uint8_t allow_mask; /* mask of sniff/hold/park modes to allow */
#if (BTM_SSR_INCLUDED == TRUE)
  uint8_t ssr; /* set SSR on conn open/unpark */
#endif
  tBTA_DM_PM_ACTN actn_tbl[BTA_DM_PM_NUM_EVTS][2];

} tBTA_DM_PM_SPEC;

typedef struct {
  uint16_t max_lat;
  uint16_t min_rmt_to;
  uint16_t min_loc_to;
} tBTA_DM_SSR_SPEC;

typedef struct {
  uint16_t manufacturer;
  uint16_t lmp_sub_version;
  uint8_t lmp_version;
} tBTA_DM_LMP_VER_INFO;

extern const uint16_t bta_service_id_to_uuid_lkup_tbl[];

extern tBTA_DM_PM_CFG* p_bta_dm_pm_cfg;
extern tBTA_DM_PM_SPEC* p_bta_dm_pm_spec;
extern tBTM_PM_PWR_MD* p_bta_dm_pm_md;
#if (BTM_SSR_INCLUDED == TRUE)
extern tBTA_DM_SSR_SPEC* p_bta_dm_ssr_spec;
#endif

/* update dynamic BRCM Aware EIR data */
extern const tBTA_DM_EIR_CONF bta_dm_eir_cfg;
extern tBTA_DM_EIR_CONF* p_bta_dm_eir_cfg;

/* DM control block */
extern tBTA_DM_CB bta_dm_cb;

/* DM search control block */
extern tBTA_DM_SEARCH_CB bta_dm_search_cb;

/* DI control block */
extern tBTA_DM_DI_CB bta_dm_di_cb;

extern bool bta_dm_sm_execute(BT_HDR* p_msg);
extern void bta_dm_sm_disable(void);
extern bool bta_dm_search_sm_execute(BT_HDR* p_msg);
extern void bta_dm_search_sm_disable(void);

extern void bta_dm_enable(tBTA_DM_MSG* p_data);
extern void bta_dm_disable(tBTA_DM_MSG* p_data);
extern void bta_dm_init_cb(void);
extern void bta_dm_set_dev_name(tBTA_DM_MSG* p_data);
extern void bta_dm_set_visibility(tBTA_DM_MSG* p_data);

extern void bta_dm_set_scan_config(tBTA_DM_MSG* p_data);
extern void bta_dm_vendor_spec_command(tBTA_DM_MSG* p_data);
extern void bta_dm_set_wifi_state(tBTA_DM_MSG *p_data);
extern void bta_dm_power_back_off(tBTA_DM_MSG *p_data);
extern void bta_dm_bredr_cleanup(tBTA_DM_MSG *p_data);
extern void bta_dm_bredr_startup(tBTA_DM_MSG *p_data);
extern void bta_dm_bond(tBTA_DM_MSG* p_data);
extern void bta_dm_bond_cancel(tBTA_DM_MSG* p_data);
extern void bta_dm_pin_reply(tBTA_DM_MSG* p_data);
extern void bta_dm_acl_change(tBTA_DM_MSG* p_data);
extern void bta_dm_process_iot_report(tBTA_DM_MSG *p_data);
extern void bta_dm_process_ssr(void);
extern void bta_dm_add_device(tBTA_DM_MSG* p_data);
extern void bta_dm_remove_device(tBTA_DM_MSG* p_data);
extern void bta_dm_close_acl(tBTA_DM_MSG* p_data);
extern void bta_dm_reset_pairing_flag(tBTA_DM_MSG* p_data);

extern void bta_dm_pm_btm_status(tBTA_DM_MSG* p_data);
extern void bta_dm_pm_timer(tBTA_DM_MSG* p_data);
extern void bta_dm_add_ampkey(tBTA_DM_MSG* p_data);

extern void bta_dm_add_blekey(tBTA_DM_MSG* p_data);
extern void bta_dm_add_ble_device(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_passkey_reply(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_confirm_reply(tBTA_DM_MSG* p_data);
extern void bta_dm_security_grant(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_set_bg_conn_type(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_set_conn_params(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_set_conn_scan_params(tBTA_DM_MSG* p_data);
extern void bta_dm_close_gatt_conn(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_observe(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_update_conn_params(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_config_local_privacy(tBTA_DM_MSG* p_data);
extern void bta_dm_ble_set_adv_params(uint16_t adv_int_min,
                                      uint16_t adv_int_max,
                                      tBLE_BD_ADDR* p_dir_bda);

extern void bta_dm_ble_set_data_length(tBTA_DM_MSG* p_data);

extern void bta_dm_ble_get_energy_info(tBTA_DM_MSG* p_data);

extern void bta_dm_set_encryption(tBTA_DM_MSG* p_data);
extern void bta_dm_confirm(tBTA_DM_MSG* p_data);
extern void bta_dm_loc_oob(tBTA_DM_MSG* p_data);
extern void bta_dm_ci_io_req_act(tBTA_DM_MSG* p_data);
extern void bta_dm_ci_rmt_oob_act(tBTA_DM_MSG* p_data);

extern void bta_dm_init_pm(void);
extern void bta_dm_disable_pm(void);

extern uint8_t bta_dm_get_av_count(void);
extern void bta_dm_search_start(tBTA_DM_MSG* p_data);
extern void bta_dm_search_cancel(tBTA_DM_MSG* p_data);
extern void bta_dm_discover(tBTA_DM_MSG* p_data);
extern void bta_dm_di_disc(tBTA_DM_MSG* p_data);
extern void bta_dm_inq_cmpl(tBTA_DM_MSG* p_data);
extern void bta_dm_rmt_name(tBTA_DM_MSG* p_data);
extern void bta_dm_sdp_result(tBTA_DM_MSG* p_data);
extern void bta_dm_search_cmpl(tBTA_DM_MSG* p_data);
extern void bta_dm_free_sdp_db(tBTA_DM_MSG* p_data);
extern void bta_dm_disc_result(tBTA_DM_MSG* p_data);
extern void bta_dm_search_result(tBTA_DM_MSG* p_data);
extern void bta_dm_discovery_cmpl(tBTA_DM_MSG* p_data);
extern void bta_dm_queue_search(tBTA_DM_MSG* p_data);
extern void bta_dm_queue_disc(tBTA_DM_MSG* p_data);
extern void bta_dm_search_clear_queue(tBTA_DM_MSG* p_data);
extern void bta_dm_search_cancel_cmpl(tBTA_DM_MSG* p_data);
extern void bta_dm_search_cancel_notify(tBTA_DM_MSG* p_data);
extern void bta_dm_search_cancel_transac_cmpl(tBTA_DM_MSG* p_data);
extern void bta_dm_disc_rmt_name(tBTA_DM_MSG* p_data);
extern tBTA_DM_PEER_DEVICE* bta_dm_find_peer_device(
    const RawAddress& peer_addr);

void bta_dm_eir_update_uuid(uint16_t uuid16, bool adding);

extern void bta_dm_enable_test_mode(tBTA_DM_MSG* p_data);
extern void bta_dm_disable_test_mode(tBTA_DM_MSG* p_data);
extern void bta_dm_execute_callback(tBTA_DM_MSG* p_data);

extern void bta_dm_remove_all_acl(tBTA_DM_MSG* p_data);
extern void bta_dm_hci_raw_command(tBTA_DM_MSG *p_data);

extern void bta_dm_queue_service_disc(tBTA_DM_MSG* p_data);
#endif /* BTA_DM_INT_H */
