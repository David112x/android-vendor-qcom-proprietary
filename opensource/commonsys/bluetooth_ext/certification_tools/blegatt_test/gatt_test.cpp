/******************************************************************************
 ** Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
 *
 *  Not a Contribution.
 *
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
 *  Filename:      gatt_tool.c
 *
 *  Description:   Bluedroid GATT TOOL application
 *
 ***********************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <map>
//#include <sys/time.h>
//#include <signal.h>
//#include <time.h>
#include "bt_target.h"
#include "l2c_api.h"
#include "bta_api.h"
#include "l2c_int.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <private/android_filesystem_config.h>
#include <android/log.h>

#include <hardware/hardware.h>
#include <hardware/bluetooth.h>
#include <hardware/vendor.h>
#include <hardware/bt_gatt.h>
#include <hardware/bt_gatt_client.h>
#include <hardware/bt_gatt_server.h>
#include <hardware/bt_gatt_types.h>
//#include "../../osi/include/allocator.h"
#include <bt_testapp.h>

#include <signal.h>
#include <time.h>

#include <base/bind.h>
#include <base/callback.h>

using bluetooth::Uuid;
#define bluetooth_module_t "bluetooth_test"

typedef struct {
  uint16_t advertising_event_properties;
  uint32_t adv_int_min;
  uint32_t adv_int_max;
  tBTM_BLE_ADV_CHNL_MAP channel_map;
  tBTM_BLE_AFP adv_filter_policy;
  int8_t tx_power;
  uint8_t primary_advertising_phy;
  uint8_t secondary_advertising_phy;
  uint8_t scan_request_notification_enable;
} tBTM_BLE_ADV_PARAMS;

typedef struct {
  uint8_t enable;
  uint16_t min_interval;
  uint16_t max_interval;
  uint16_t periodic_advertising_properties;
} tBLE_PERIODIC_ADV_PARAMS;


#ifdef TEST_APP_INTERFACE
/************************************************************************************
**  Constants & Macros
************************************************************************************/

#ifndef TRUE
#define     TRUE       1
#endif
#ifndef FALSE
#define     FALSE      0
#endif

#define PID_FILE "/data/.bdt_pid"

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#define CASE_RETURN_STR(const) case const: return #const;


#define INTERVAL_LOW     1
#define INTERVAL_MEDIUM  2
#define INTERVAL_HIGH    3

#define L2CAP_LE_DEFAULT_MTU 512
#define L2CAP_LE_DEFAULT_MPS 230
#define L2CAP_LE_MAX_CREDIT 65535
#define L2CAP_LE_DEFAULT_CREDIT 1
#define LE_CHAR_MAX_LEN_VAL 512

/************************************************************************************
**  Local type definitions
************************************************************************************/
static void register_client_cb(int status, int client_if, const Uuid& app_uuid);

static void register_server_cb(int status, int server_if, const Uuid& app_uuid);

/************************************************************************************
**  Static variables
************************************************************************************/

static unsigned char main_done = 0;
static int status;
typedef struct
{
    uint16_t      result;                 /* Only used in confirm messages */
    uint16_t      credits;                /* used to send the outstanding credits */
    uint16_t      le_psm;
    uint16_t      le_mps;
    uint16_t      le_mtu;
    uint16_t      init_credits;          /* initial credits */
} tL2CAP_LE_CONN_INFO;

typedef struct
{
    bool                 in_use;
    uint16_t                  psm;
    uint16_t                  lcid;
    tL2CAP_LE_CONN_INFO     loc_conn_info;
    tL2CAP_LE_CONN_INFO     rmt_conn_info;
    bool                 is_server;
} t_le_chnl_info;

t_le_chnl_info le_chnl_conn_info[MAX_L2CAP_CLIENTS];
#define LE_ACL_MAX_BUFF_SIZE 4096
static int num_frames = 1;
static unsigned long g_delay = 1; /* Default delay before data transfer */
static int count = 1;
static uint16_t g_BleEncKeySize = 16;
//static int g_omps = 0;
//static int rcv_count = 0;
static int g_le_coc_if = 0;
static int rcv_itration = 0;
static volatile bool cong_status = FALSE;
/* Control channel LE-L2CAP default options */
static tL2CAP_LE_CONN_INFO le_conn_info;
static tL2CAP_LE_CFG_INFO local_coc_cfg;

/* Main API */
const bt_interface_t* sBtInterface = NULL;

static gid_t groups[] = { AID_NET_BT, AID_INET, AID_NET_BT_ADMIN,
                          AID_SYSTEM, AID_MISC, AID_SDCARD_RW,
                          AID_NET_ADMIN, AID_VPN};

enum {
    DISCONNECT,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
};
static unsigned char bt_enabled = 0;
static int  g_ConnectionState   = DISCONNECT;
static int  g_AdapterState      = BT_STATE_OFF;
static int  g_PairState         = BT_BOND_STATE_NONE;

static int  g_conn_id        = 0;
static int  g_client_if      = 0;
static int  g_server_if      = 0;
static int  g_client_if_scan = 0;
static int  g_server_if_scan = 0;

const btgatt_test_interface_t     *sGattInterface = NULL;
const  btgatt_interface_t   *sGattIfaceScan = NULL;
const btsmp_interface_t    *sSmpIface             = NULL;
const btgap_interface_t    *sGapInterface         = NULL;
const btl2cap_interface_t *sL2capInterface = NULL;
const btvendor_interface_t *btvendorInterface = NULL;


int  Btif_gatt_layer = TRUE;
RawAddress* remote_bd_address;

static uint16_t g_SecLevel = 0;
static bool g_ConnType = TRUE;//DUT is initiating connection
static bool g_Fcr_Present = FALSE;
static uint8_t g_Fcr_Mode = L2CAP_FCR_BASIC_MODE;
static uint8_t g_Ertm_AllowedMode = (L2CAP_FCR_CHAN_OPT_BASIC | L2CAP_FCR_CHAN_OPT_ERTM | L2CAP_FCR_CHAN_OPT_STREAM);

//GATT server UUIDs
std::string IMMEDIATE_ALERT_UUID = "00001802-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID = "00002a06-0000-1000-8000-00805f9b34fb";


//Services
// std::string IMMEDIATE_ALERT_UUID = UUID.fromString("00001200-0000-1000-8000-00805f9b34fb");
std::string IMMEDIATE_ALERT_UUID1 = "00001201-0000-1000-8000-00805f9b34fb";
std::string IMMEDIATE_ALERT_UUID2 = "00001202-0000-1000-8000-00805f9b34fb";
std::string IMMEDIATE_ALERT_UUID3 = "00001203-0000-1000-8000-00805f9b34fb";
std::string IMMEDIATE_ALERT_UUID4 = "00001204-0000-1000-8000-00805f9b34fb";
std::string IMMEDIATE_ALERT_UUID5 = "00001205-0000-1000-8000-00805f9b34fb";
std::string IMMEDIATE_ALERT_UUID6 = "00001206-0000-1000-8000-00805f9b34fb";

//Characterstics
std::string ALERT_LEVEL_UUID1 = "00001100-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID2 = "00001101-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID3 = "00001102-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID4 = "00001103-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID5 = "00001104-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID6 = "00001105-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID7 = "00001106-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID8 = "00001107-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID9 = "00001108-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID10 = "00001109-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID11 = "00001110-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID12 = "00001111-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID13 = "00001112-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID14 = "00001113-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID15 = "00001114-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID16 = "00001115-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID17 = "00001116-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_WRITENORESPONSEWITHREADABLE = "00001117-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_WRITENORESPONSEWITHREADABLE2 = "00001118-0000-1000-8000-00805f9b34fb";
std::string AUTHENTICATION_LEVEL_UUID1 = "00001119-0000-1000-8000-00805f9b34fb";
std::string AUTHENTICATION_LEVEL_UUID2 = "00001120-0000-1000-8000-00805f9b34fb";
std::string AUTHENTICATION_LEVEL_UUID3 = "00001121-0000-1000-8000-00805f9b34fb";
std::string AUTHENTICATION_LEVEL_UUID4 = "00001122-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID18 = "00001123-0000-1000-8000-00805f9b34fb";
std::string ALERT_LEVEL_UUID19 = "00001124-0000-1000-8000-00805f9b34fb";

//Decscriptors
std::string ClientCharConfigUUID = "00002902-0000-1000-8000-00805f9b34fb";
std::string DISC_LEVEL_UUID1 = "00001301-0000-1000-8000-00805f9b34fb";
std::string DISC_LEVEL_UUID2 = "00001302-0000-1000-8000-00805f9b34fb";
std::string DISC_LEVEL_UUID3 = "00001303-0000-1000-8000-00805f9b34fb";
std::string DISC_LEVEL_UUID4 = "00001304-0000-1000-8000-00805f9b34fb";
std::string DISC_LEVEL_UUID5 = "00001305-0000-1000-8000-00805f9b34fb";
std::string DISC_LEVEL_UUID6 = "00001306-0000-1000-8000-00805f9b34fb";
std::string DISC_LEVEL_UUID7 = "00001307-0000-1000-8000-00805f9b34fb";
std::string AUTHENTICATION_LEVEL_DESC_UUID1 = "00001400-0000-1000-8000-00805f9b34fb";
std::string AUTHENTICATION_LEVEL_DESC_UUID2 = "00001401-0000-1000-8000-00805f9b34fb";
std::string AUTHENTICATION_LEVEL_DESC_UUID3 = "00001402-0000-1000-8000-00805f9b34fb";
std::string AUTHENTICATION_LEVEL_DESC_UUID4 = "00001403-0000-1000-8000-00805f9b34fb";

std::string CharacteristicExtendedProperties = "00002900-0000-1000-8000-00805f9b34fb";
std::string CharacteristicUserDescription = "00002901-0000-1000-8000-00805f9b34fb";
std::string ServerCharacteristicConfiguration = "00002903-0000-1000-8000-00805f9b34fb";
std::string CharacteristicFormat = "00002904-0000-1000-8000-00805f9b34fb";
std::string CharacteristicAggregateFormat = "00002905-0000-1000-8000-00805f9b34fb";

static uint8_t attr_value[BTGATT_MAX_ATTR_LEN];//600

uint8_t long_byte_value[] = {31, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
uint8_t short_byte_value[] = {31, 2};

int len_long_char = LE_CHAR_MAX_LEN_VAL;
int len_short_char = 2;

int curr_char_val_len =0;
int curr_handle = 0;

std::map<int, std::vector<uint8_t>> cccd_value_map;

int exec_write_status = BT_STATUS_SUCCESS;
int invalid_offset = 0x07;
int invalid_attribute_value_len = 0x0D;
int application_error = 0x80;

/* Default mtu */
static int g_imtu = 672;
static int g_omtu = 0;

int start_advertising_set_advertiser_id = -1;
int start_advertising_set_tx_power = -1;
int start_advertising_set_status = -1;

enum {
L2CAP_NOT_CONNECTED,
L2CAP_CONN_SETUP,
L2CAP_CONNECTED
};

//static int L2cap_conn_state = L2CAP_NOT_CONNECTED;
static tL2CAP_CFG_INFO tl2cap_cfg_info;
static long data_size = -1;
static uint16_t           g_PSM           = 0;
static uint16_t           g_lcid          = 0;


enum {
    SEND,
    RECEIVE,
    WAITANDSEND,
    PAIR,
    PING,
    CONNECT,
};


/* Control channel eL2CAP default options */
tL2CAP_FCR_OPTS ertm_fcr_opts_def = {
    L2CAP_FCR_ERTM_MODE,
    3, /* Tx window size */
    20, /* Maximum transmissions before disconnecting */
    2000, /* Retransmission timeout (2 secs) */
    12000, /* Monitor timeout (12 secs) */
    100 /* MPS segment size */
};

tL2CAP_FCR_OPTS stream_fcr_opts_def = {
    L2CAP_FCR_STREAM_MODE,
    3,/* Tx window size */
    20, /* Maximum transmissions before disconnecting */
    2000, /* Retransmission timeout (2 secs) */
    12000, /* Monitor timeout (12 secs) */
    100 /* MPS segment size */
};
static tL2CAP_ERTM_INFO t_ertm_info = {0, 0, 0, 0, 0, 0};


/************************************************************************************
**  Static functions
************************************************************************************/

static void process_cmd(char *p, unsigned char is_job);
//static void job_handler(void *param);
static void bdt_log(const char *fmt_str, ...);
static void l2c_connect(RawAddress bd_addr);
static uint16_t do_l2cap_connect(RawAddress bd_addr);



int GetBdAddr(char *p, RawAddress* pbd_addr);
int reg_inst_id = -1;
int reg_status = -1;
/* LE L2CAP functions */
static t_le_chnl_info *le_allocate_conn_info(uint16_t psm, bool is_server);
static t_le_chnl_info *le_get_conn_info(uint16_t psm, bool is_server);
static t_le_chnl_info *le_get_conn_info_by_lcid(uint16_t lcid);
static bool le_release_conn_info(t_le_chnl_info *le_conn_info);
uint8_t do_l2cap_DataWrite(uint16_t chnl_id, char *p , uint32_t len);
static int Send_Data();
static int send_file(char *p);
static void le_l2cap_coc_connect(char *svr);
uint16_t do_le_l2cap_coc_connect(char *p);
static void le_l2cap_coc_flow_ctrl(char *p);
uint16_t do_le_l2cap_coc_flow_ctrl(char *p);
static void do_le_coc_disconnect(char *p);
int GetFileName(char *p, char *filename);
static void le_l2cap_listen(char *p);
static void do_start_advertisment(char *p);
static int send_file(char *p);
static void send_data_on_le_coc(char *svr);
static void do_send_file(char *svr);

/************************************************************************************
**  GATT Client Callbacks
************************************************************************************/
static void register_client_cb(int status, int client_if, const Uuid& app_uuid)
{
    printf("%s:: status=%d, client_if=%d \n", __FUNCTION__, status, client_if);
    if(0 == status)    g_client_if_scan = client_if;
}

static void connect_cb(int conn_id, int status, int client_if, const RawAddress& remote_bd_addr)
{
    printf("%s:: remote_bd_addr=%02x:%02x:%02x:%02x:%02x:%02x, conn_id=0x%x, status=%d, client_if=%d\n",  __FUNCTION__,
    remote_bd_addr.address[0], remote_bd_addr.address[1], remote_bd_addr.address[2],
    remote_bd_addr.address[3], remote_bd_addr.address[4], remote_bd_addr.address[5], conn_id, status, client_if);

    g_conn_id = conn_id;
    sGapInterface->Gap_BleAttrDBUpdate(remote_bd_addr.address, 50, 70, 0, 1000);

}

static btgatt_client_callbacks_t sGattClient_cb =
{
    register_client_cb,
    connect_cb,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, //register_for_notification_cb,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

/************************************************************************************
**  Scanner Callbacks
************************************************************************************/

static void scan_result_cb(uint16_t event_type, uint8_t addr_type,
                           RawAddress *bda, uint8_t primary_phy,
                           uint8_t secondary_phy,
                           uint8_t advertising_sid, int8_t tx_power,
                           int8_t rssi, uint16_t periodic_adv_int,
                           std::vector<uint8_t> adv_data)
{
    printf("%s:: event_type=%d, bda=%02x%02x%02x%02x%02x%02x \n", __FUNCTION__, event_type, bda->address[0],
           bda->address[1], bda->address[2], bda->address[3], bda->address[4], bda->address[5]);
}

static btgatt_scanner_callbacks_t  sScanner_cb =
{
    scan_result_cb,
    NULL,
    NULL,
    NULL
};

/************************************************************************************
**  GATT Server Callbacks
************************************************************************************/
static void register_server_cb(int status, int server_if, const Uuid& app_uuid)
{
    printf("%s:: status=%d, server_if=%d \n", __FUNCTION__, status, server_if);
    if(0 == status)    g_server_if_scan = server_if;

    int  Ret = 0;
    bool is_valid = false;

    std::vector<btgatt_db_element_t> service1;
    //1st service
    btgatt_db_element_t svc1 = {0};
    svc1.uuid = Uuid::FromString(IMMEDIATE_ALERT_UUID, &is_valid);
    svc1.type = BTGATT_DB_PRIMARY_SERVICE;
    service1.push_back(svc1);

    //1st char
    btgatt_db_element_t char1 = {0};
    char1.uuid = Uuid::FromString(ALERT_LEVEL_UUID1, &is_valid);
    char1.type = BTGATT_DB_CHARACTERISTIC;
    char1.properties = 58;
    char1.permissions = 17;
    //use shortByteValue byte array for this char
    service1.push_back(char1);

    //1st desc
    btgatt_db_element_t desc1 = {0};
    desc1.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc1.type = BTGATT_DB_DESCRIPTOR;
    desc1.permissions = 17;
    //use shortByteValue byte array for this desc
    service1.push_back(desc1);

    //2nd char
    btgatt_db_element_t char2 = {0};
    char2.uuid = Uuid::FromString(ALERT_LEVEL_UUID2, &is_valid);
    char2.type = BTGATT_DB_CHARACTERISTIC;
    char2.properties = 58;
    char2.permissions = 34;
    //use shortByteValue byte array for this char
    service1.push_back(char2);

    //2nd desc
    btgatt_db_element_t desc2 = {0};
    desc2.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc2.type = BTGATT_DB_DESCRIPTOR;
    desc2.permissions = 34;
    //use shortByteValue byte array for this desc
    service1.push_back(desc2);


    //3rd char
    btgatt_db_element_t char3 = {0};
    char3.uuid = Uuid::FromString(ALERT_LEVEL_UUID3, &is_valid);
    char3.type = BTGATT_DB_CHARACTERISTIC;
    char3.properties = 58;
    char3.permissions = 68;
    //use shortByteValue byte array for this char
    service1.push_back(char3);

    //3rd desc
    btgatt_db_element_t desc3 = {0};
    desc3.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc3.type = BTGATT_DB_DESCRIPTOR;
    desc3.permissions = 68;
    //use shortByteValue byte array for this desc
    service1.push_back(desc3);


    //4th char
    btgatt_db_element_t char4 = {0};
    char4.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_UUID1, &is_valid);
    char4.type = BTGATT_DB_CHARACTERISTIC;
    char4.properties = 58;
    char4.permissions = 34;
    //use shortByteValue byte array for this char
    service1.push_back(char4);

    //4th desc
    btgatt_db_element_t desc4 = {0};
    desc4.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_DESC_UUID1, &is_valid);
    desc4.type = BTGATT_DB_DESCRIPTOR;
    desc4.permissions = 34;
    //use shortByteValue byte array for this desc
    service1.push_back(desc4);


    //5th char
    btgatt_db_element_t char5 = {0};
    char5.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_UUID2, &is_valid);
    char5.type = BTGATT_DB_CHARACTERISTIC;
    char5.properties = 58;
    char5.permissions = 68;
    //use shortByteValue byte array for this char
    service1.push_back(char5);

    //5th desc
    btgatt_db_element_t desc5 = {0};
    desc5.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_DESC_UUID2, &is_valid);
    desc5.type = BTGATT_DB_DESCRIPTOR;
    desc5.permissions = 68;
    //use shortByteValue byte array for this desc
    service1.push_back(desc5);

    //6th char
    btgatt_db_element_t char6 = {0};
    char6.uuid = Uuid::FromString(ALERT_LEVEL_UUID4, &is_valid);
    char6.type = BTGATT_DB_CHARACTERISTIC;
    char6.properties = 4;
    char6.permissions = 16;
    //use shortByteValue byte array for this char
    service1.push_back(char6);

    //6th desc
    btgatt_db_element_t desc6 = {0};
    desc6.uuid = Uuid::FromString(DISC_LEVEL_UUID1, &is_valid);
    desc6.type = BTGATT_DB_DESCRIPTOR;
    desc6.permissions = 16;
    //use shortByteValue byte array for this desc
    service1.push_back(desc6);

    //7th char
    btgatt_db_element_t char7 = {0};
    char7.uuid = Uuid::FromString(ALERT_LEVEL_UUID5, &is_valid);
    char7.type = BTGATT_DB_CHARACTERISTIC;
    char7.properties = 114;
    char7.permissions = 129;
    //use shortByteValue byte array for this char
    service1.push_back(char7);

    //7th desc
    btgatt_db_element_t desc7 = {0};
    desc7.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc7.type = BTGATT_DB_DESCRIPTOR;
    desc7.permissions = 129;
    //use shortByteValue byte array for this desc
    service1.push_back(desc7);

    //8th char
    btgatt_db_element_t char8 = {0};
    char8.uuid = Uuid::FromString(ALERT_LEVEL_UUID6, &is_valid);
    char8.type = BTGATT_DB_CHARACTERISTIC;
    char8.properties = 66;
    char8.permissions = 129;
    //use shortByteValue byte array for this char
    service1.push_back(char8);

    //8th desc
    btgatt_db_element_t desc8 = {0};
    desc8.uuid = Uuid::FromString(DISC_LEVEL_UUID2, &is_valid);
    desc8.type = BTGATT_DB_DESCRIPTOR;
    desc8.permissions = 129;
    //use shortByteValue byte array for this desc
    service1.push_back(desc8);

    //9th char
    btgatt_db_element_t char9 = {0};
    char9.uuid = Uuid::FromString(ALERT_LEVEL_WRITENORESPONSEWITHREADABLE2, &is_valid);
    char9.type = BTGATT_DB_CHARACTERISTIC;
    char9.properties = 6;
    char9.permissions = 17;
    //use shortByteValue byte array for this char
    service1.push_back(char9);

    //9th desc
    btgatt_db_element_t desc9 = {0};
    desc9.uuid = Uuid::FromString(DISC_LEVEL_UUID7, &is_valid);
    desc9.type = BTGATT_DB_DESCRIPTOR;
    desc9.permissions = 17;
    //use shortByteValue byte array for this desc
    service1.push_back(desc9);

    //10th char
    btgatt_db_element_t char10 = {0};
    char10.uuid = Uuid::FromString(ALERT_LEVEL_UUID18, &is_valid);
    char10.type = BTGATT_DB_CHARACTERISTIC;
    char10.properties = 58;
    char10.permissions = 17;
    //use shortByteValue byte array for this char
    service1.push_back(char10);

    //10th desc
    btgatt_db_element_t desc10 = {0};
    desc10.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc10.type = BTGATT_DB_DESCRIPTOR;
    desc10.permissions = 17;
    //use shortByteValue byte array for this desc
    service1.push_back(desc10);

    //11th char
    btgatt_db_element_t char11 = {0};
    char11.uuid = Uuid::FromString(ALERT_LEVEL_UUID19, &is_valid);
    char11.type = BTGATT_DB_CHARACTERISTIC;
    char11.properties = 58;
    char11.permissions = 17;
    //use shortByteValue byte array for this char
    service1.push_back(char11);

    //11th desc
    btgatt_db_element_t desc11 = {0};
    desc11.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc11.type = BTGATT_DB_DESCRIPTOR;
    desc11.permissions = 17;
    //use shortByteValue byte array for this desc
    service1.push_back(desc11);

    Ret = sGattIfaceScan->server->add_service(g_server_if_scan, service1);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );


    //Second Service - longvalue service
    std::vector<btgatt_db_element_t> service2;
    //2nd service
    btgatt_db_element_t svc2 = {0};
    svc2.uuid = Uuid::FromString(IMMEDIATE_ALERT_UUID1, &is_valid);
    svc2.type = BTGATT_DB_PRIMARY_SERVICE;
    service2.push_back(svc2);

    //1st char
    char1.uuid = Uuid::FromString(ALERT_LEVEL_UUID7, &is_valid);
    char1.type = BTGATT_DB_CHARACTERISTIC;
    char1.properties = 58;
    char1.permissions = 17;
    //use longByteValue byte array for this char
    service2.push_back(char1);

    //1st desc
    desc1.uuid = Uuid::FromString(DISC_LEVEL_UUID3, &is_valid);
    desc1.type = BTGATT_DB_DESCRIPTOR;
    desc1.permissions = 17;
    //use longByteValue byte array for this desc
    service2.push_back(desc1);


    //2nd char
    char2.uuid = Uuid::FromString(ALERT_LEVEL_UUID8, &is_valid);
    char2.type = BTGATT_DB_CHARACTERISTIC;
    char2.properties = 58;
    char2.permissions = 34;
    //use longByteValue byte array for this char
    service2.push_back(char2);

    //2nd desc
    desc2.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc2.type = BTGATT_DB_DESCRIPTOR;
    desc2.permissions = 34;
    //use longByteValue byte array for this desc
    service2.push_back(desc2);



    //3rd char
    char3.uuid = Uuid::FromString(ALERT_LEVEL_UUID9, &is_valid);
    char3.type = BTGATT_DB_CHARACTERISTIC;
    char3.properties = 58;
    char3.permissions = 68;
    //use longByteValue byte array for this char
    service2.push_back(char3);

    //3rd desc
    desc3.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc3.type = BTGATT_DB_DESCRIPTOR;
    desc3.permissions = 68;
    //use longByteValue byte array for this desc
    service2.push_back(desc3);


    //4th char
    char4.uuid = Uuid::FromString(ALERT_LEVEL_UUID10, &is_valid);
    char4.type = BTGATT_DB_CHARACTERISTIC;
    char4.properties = 4;
    char4.permissions = 16;
    //use longByteValue byte array for this char
    service2.push_back(char4);

    //4th desc
    desc4.uuid = Uuid::FromString(DISC_LEVEL_UUID3, &is_valid);
    desc4.type = BTGATT_DB_DESCRIPTOR;
    desc4.permissions = 16;
    //use longByteValue byte array for this desc
    service2.push_back(desc4);


    //5th char
    char5.uuid = Uuid::FromString(ALERT_LEVEL_UUID11, &is_valid);
    char5.type = BTGATT_DB_CHARACTERISTIC;
    char5.properties = 114;
    char5.permissions = 129;
    //use longByteValue byte array for this char
    service2.push_back(char5);

    //5th desc
    desc5.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc5.type = BTGATT_DB_DESCRIPTOR;
    desc5.permissions = 129;
    //use longByteValue byte array for this desc
    service2.push_back(desc5);


    //6th char
    char6.uuid = Uuid::FromString(ALERT_LEVEL_UUID12, &is_valid);
    char6.type = BTGATT_DB_CHARACTERISTIC;
    char6.properties = 66;
    char6.permissions = 129;
    //use longByteValue byte array for this char
    service2.push_back(char6);

    //6th desc
    desc6.uuid = Uuid::FromString(DISC_LEVEL_UUID4, &is_valid);
    desc6.type = BTGATT_DB_DESCRIPTOR;
    desc6.permissions = 129;
    //use longByteValue byte array for this desc
    service2.push_back(desc6);

    //7th char
    char7.uuid = Uuid::FromString(ALERT_LEVEL_WRITENORESPONSEWITHREADABLE, &is_valid);
    char7.type = BTGATT_DB_CHARACTERISTIC;
    char7.properties = 6;
    char7.permissions = 17;
    //use longByteValue byte array for this char
    service2.push_back(char7);

    //7th desc
    desc7.uuid = Uuid::FromString(DISC_LEVEL_UUID6, &is_valid);
    desc7.type = BTGATT_DB_DESCRIPTOR;
    desc7.permissions = 17;
    //use longByteValue byte array for this desc
    service2.push_back(desc7);

    //8th char
    char8.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_UUID3, &is_valid);
    char8.type = BTGATT_DB_CHARACTERISTIC;
    char8.properties = 58;
    char8.permissions = 34;
    //use longByteValue byte array for this char
    service2.push_back(char8);

    //8th desc
    desc8.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_DESC_UUID3, &is_valid);
    desc8.type = BTGATT_DB_DESCRIPTOR;
    desc8.permissions = 34;
    //use longByteValue byte array for this desc
    service2.push_back(desc8);

    //9th char
    char9.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_UUID4, &is_valid);
    char9.type = BTGATT_DB_CHARACTERISTIC;
    char9.properties = 58;
    char9.permissions = 68;
    //use longByteValue byte array for this char
    service2.push_back(char9);

    //9th desc
    desc9.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_DESC_UUID4, &is_valid);
    desc9.type = BTGATT_DB_DESCRIPTOR;
    desc9.permissions = 68;
    //use longByteValue byte array for this desc
    service2.push_back(desc9);


    Ret = sGattIfaceScan->server->add_service(g_server_if_scan, service2);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );



    //3rd service
    std::vector<btgatt_db_element_t> service3;

    btgatt_db_element_t svc3 = {0};
    svc3.uuid = Uuid::FromString(IMMEDIATE_ALERT_UUID2, &is_valid);
    svc3.type = BTGATT_DB_PRIMARY_SERVICE;
    service3.push_back(svc3);

    //1st char
    char1.uuid = Uuid::FromString(ALERT_LEVEL_UUID13, &is_valid);
    char1.type = BTGATT_DB_CHARACTERISTIC;
    char1.properties = 10;
    char1.permissions = 17;
    //use shortByteValue byte array for this char
    service3.push_back(char1);

    //1st desc
    desc1.uuid = Uuid::FromString(DISC_LEVEL_UUID5, &is_valid);
    desc1.type = BTGATT_DB_DESCRIPTOR;
    desc1.permissions = 17;
    //use shortByteValue byte array for this desc
    service3.push_back(desc1);


    //Added this characteristic for Insufficient Encryption key size test case
    //2nd char
    char2.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_UUID4, &is_valid);
    char2.type = BTGATT_DB_CHARACTERISTIC;
    char2.properties = 58;
    char2.permissions = 0x2007;
    //use shortByteValue byte array for this char
    service3.push_back(char2);

    //2nd desc
    desc2.uuid = Uuid::FromString(AUTHENTICATION_LEVEL_DESC_UUID4, &is_valid);
    desc2.type = BTGATT_DB_DESCRIPTOR;
    desc2.permissions = 0x2007;
    //use shortByteValue byte array for this desc
    service3.push_back(desc2);


    Ret = sGattIfaceScan->server->add_service(g_server_if_scan, service3);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );



    //4th service with no value set
    std::vector<btgatt_db_element_t> service4;

    btgatt_db_element_t svc4 = {0};
    svc4.uuid = Uuid::FromString(IMMEDIATE_ALERT_UUID3, &is_valid);
    svc4.type = BTGATT_DB_PRIMARY_SERVICE;
    service4.push_back(svc4);

    //1st char
    char1.uuid = Uuid::FromString(ALERT_LEVEL_UUID14, &is_valid);
    char1.type = BTGATT_DB_CHARACTERISTIC;
    char1.properties = 10;
    char1.permissions = 17;
    //use shortByteValue byte array for this char
    service4.push_back(char1);


    Ret = sGattIfaceScan->server->add_service(g_server_if_scan, service4);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );


    //5th service will all kinds of descriptors
    std::vector<btgatt_db_element_t> service5;

    btgatt_db_element_t svc5 = {0};
    svc5.uuid = Uuid::FromString(IMMEDIATE_ALERT_UUID6, &is_valid);
    svc5.type = BTGATT_DB_PRIMARY_SERVICE;
    service5.push_back(svc5);

    //1st char
    char1.uuid = Uuid::FromString(ALERT_LEVEL_UUID17, &is_valid);
    char1.type = BTGATT_DB_CHARACTERISTIC;
    char1.properties = 58;
    char1.permissions = 17;
    service5.push_back(char1);

    //1st desc
    desc1.uuid = Uuid::FromString(CharacteristicExtendedProperties, &is_valid);
    desc1.type = BTGATT_DB_DESCRIPTOR;
    desc1.permissions = 17;
    //use shortByteValue byte array for this char
    service5.push_back(desc1);

    //2nd desc
    desc2.uuid = Uuid::FromString(CharacteristicUserDescription, &is_valid);
    desc2.type = BTGATT_DB_DESCRIPTOR;
    desc2.permissions = 17;
    //use shortByteValue byte array for this desc
    service5.push_back(desc2);

    //3rd desc
    desc3.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc3.type = BTGATT_DB_DESCRIPTOR;
    desc3.permissions = 17;
    //use shortByteValue byte array for this desc
    service5.push_back(desc3);

    //4th desc
    desc4.uuid = Uuid::FromString(ServerCharacteristicConfiguration, &is_valid);
    desc4.type = BTGATT_DB_DESCRIPTOR;
    desc4.permissions = 17;
    //use shortByteValue byte array for this desc
    service5.push_back(desc4);

    //5th desc
    desc5.uuid = Uuid::FromString(CharacteristicFormat, &is_valid);
    desc5.type = BTGATT_DB_DESCRIPTOR;
    desc5.permissions = 17;
    //use shortByteValue byte array for this desc
    service5.push_back(desc5);

    //6th desc
    desc6.uuid = Uuid::FromString(CharacteristicAggregateFormat, &is_valid);
    desc6.type = BTGATT_DB_DESCRIPTOR;
    desc6.permissions = 17;
    //use shortByteValue byte array for this desc
    service5.push_back(desc6);

    //7th desc
    desc7.uuid = Uuid::FromString(CharacteristicExtendedProperties, &is_valid);
    desc7.type = BTGATT_DB_DESCRIPTOR;
    desc7.permissions = 17;
    //use longByteValue byte array for this desc
    service5.push_back(desc7);

    //8th desc
    desc8.uuid = Uuid::FromString(CharacteristicUserDescription, &is_valid);
    desc8.type = BTGATT_DB_DESCRIPTOR;
    desc8.permissions = 17;
    //use longByteValue byte array for this desc
    service5.push_back(desc8);

    //9th desc
    desc9.uuid = Uuid::FromString(ClientCharConfigUUID, &is_valid);
    desc9.type = BTGATT_DB_DESCRIPTOR;
    desc9.permissions = 17;
    //use longByteValue byte array for this desc
    service5.push_back(desc9);

    //10th desc
    desc10.uuid = Uuid::FromString(ServerCharacteristicConfiguration, &is_valid);
    desc10.type = BTGATT_DB_DESCRIPTOR;
    desc10.permissions = 17;
    //use longByteValue byte array for this desc
    service5.push_back(desc10);

    //11th desc
    desc11.uuid = Uuid::FromString(CharacteristicFormat, &is_valid);
    desc11.type = BTGATT_DB_DESCRIPTOR;
    desc11.permissions = 17;
    //use longByteValue byte array for this desc
    service5.push_back(desc11);

    //12th desc
    btgatt_db_element_t desc12 = {0};
    desc12.uuid = Uuid::FromString(CharacteristicAggregateFormat, &is_valid);
    desc12.type = BTGATT_DB_DESCRIPTOR;
    desc12.permissions = 17;
    //use longByteValue byte array for this desc
    service5.push_back(desc12);

    Ret = sGattIfaceScan->server->add_service(g_server_if_scan, service5);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );

    for(int i=0; i< 512; i++)
    {
        attr_value[i] = 0x02;
    }
}

static void server_connection_cb(int conn_id, int server_if, int connected, const RawAddress& bda)
{
    printf("%s:: conn_id=%d, server_if=%d \n", __FUNCTION__, conn_id, server_if);
    g_conn_id = conn_id;
}

static void request_read_cb(int conn_id, int trans_id, const RawAddress& bda,
                            int attr_handle, int offset, bool is_long)
{
    printf("%s:: conn_id=%d, attr_handle=%d \n", __FUNCTION__, conn_id, attr_handle);
    bt_status_t        Ret;
    int len = len_short_char;
    btgatt_response_t  gatt_resp;
    int status = BT_STATUS_SUCCESS;
    std::vector<uint8_t> cccd_val;
    uint8_t val[2] = {};
    gatt_resp.handle = attr_handle;
    gatt_resp.attr_value.handle = attr_handle;
    gatt_resp.attr_value.offset = offset;
    gatt_resp.attr_value.auth_req = 0;
    gatt_resp.attr_value.len = 1;

    printf("%s:: offset=%d, is_long =%d \n", __FUNCTION__, offset, is_long);

    if(attr_handle == 66)
    {
        printf("%s:: Invalid transport access over LE \n", __FUNCTION__);
        status = application_error;
    }
    else if(attr_handle == 104)
    {
        printf("%s:: Invalid transport access over BR/EDR \n", __FUNCTION__);
        status = application_error;
    }
    else if((attr_handle >=74 && attr_handle <=101) || is_long)
    {
        len = len_long_char;
        if(offset > len)
        {
            printf("%s:: Invalid offset\n", __FUNCTION__);
            status = invalid_offset;
        }
        else
        {
            memcpy(gatt_resp.attr_value.value, &attr_value[offset] , (len - offset));
            gatt_resp.attr_value.len = (len - offset);
        }
    }
    //Client char configuration descriptor
    else if(attr_handle == 43)
    {
        cccd_val = cccd_value_map[conn_id];
        for(int i=0; i< cccd_val.size(); i++) {
            val[i] = cccd_val[i];
        }
        memcpy(gatt_resp.attr_value.value, &val, 2);
        gatt_resp.attr_value.len = 2;
    }
    else
    {
        memcpy(gatt_resp.attr_value.value, attr_value, 300);
    }

    g_conn_id = conn_id;
    Ret = sGattIfaceScan->server->send_response(conn_id, trans_id, status, gatt_resp);
}


static void request_write_cb(int conn_id, int trans_id, const RawAddress& bda,
                             int attr_handle, int offset, bool need_rsp,
                             bool is_prep, std::vector<uint8_t> value)
{
    printf("%s:: conn_id=%d, trans_id=%d, attr_handle=%d \n", __FUNCTION__, conn_id, trans_id, attr_handle);
    bt_status_t        Ret;
    int status = BT_STATUS_SUCCESS;
    uint8_t cccd_val[2] = {};
    btgatt_response_t  gatt_resp;
    gatt_resp.handle = attr_handle;
    gatt_resp.attr_value.handle = attr_handle;
    gatt_resp.attr_value.offset = offset;
    gatt_resp.attr_value.auth_req = 0;
    gatt_resp.attr_value.len = 1;
    exec_write_status = BT_STATUS_SUCCESS;

    printf("%s:: value size=%d, offset=%d \n", __FUNCTION__, value.size(), offset);

    if(is_prep)
    {
        if((value.size()+offset) > len_long_char)
        {
            printf("%s:: Invalid attribute value length for long char/desc \n", __FUNCTION__);
            exec_write_status = invalid_attribute_value_len;
        }
        if(offset > len_long_char)
        {
            printf("%s:: Invalid offset for long char/desc \n", __FUNCTION__);
            exec_write_status = invalid_offset;
        }
    }
    else
    {
        if((curr_handle == attr_handle) && ((value.size()+offset) > curr_char_val_len))
        {
            printf("%s:: Invalid attribute value length for short char/desc \n", __FUNCTION__);
            status = invalid_attribute_value_len;
        }
    }

    for(int i=0; i< value.size(); i++) {
        attr_value[i+offset] = value[i];
    }

    //Client char configuration descriptor
    if(attr_handle == 43)
    {
        for(int i=0; i< value.size(); i++) {
            cccd_val[i] = value[i];
        }
        //cccd_value_map.insert(std::make_pair(conn_id, cccd_val));
        cccd_value_map[conn_id] = value;
        memcpy(gatt_resp.attr_value.value, &cccd_val, value.size());
        gatt_resp.attr_value.len = value.size();
    }
    else
    {
        memcpy(gatt_resp.attr_value.value, &attr_value[offset] , value.size());
        gatt_resp.attr_value.len = value.size();
    }

    curr_char_val_len = value.size();
    curr_handle = attr_handle;

    g_conn_id = conn_id;

    Ret = sGattIfaceScan->server->send_response(conn_id, trans_id, status, gatt_resp);
}

static void request_exec_write_cb(int conn_id, int trans_id, const RawAddress& bda,
                                  int exec_write)
{
    printf("%s:: conn_id=%d, trans_id=%d \n", __FUNCTION__, conn_id, trans_id);
    bt_status_t        Ret;
    int status = BT_STATUS_SUCCESS;
    btgatt_response_t  gatt_resp;

    gatt_resp.handle = 1;
    gatt_resp.attr_value.handle = 1;
    gatt_resp.attr_value.offset = 0;
    gatt_resp.attr_value.auth_req = 0;
    gatt_resp.attr_value.len = 1;
    gatt_resp.attr_value.value[0] = 1;

    if(g_conn_id == conn_id)
    {
        status = exec_write_status;
    }
    g_conn_id = conn_id;

    //cancel all prepare writes
    if(exec_write == 0)
    {
        /*assign the attr value to the old value(0x02) which will be read later by PTS
        and not the value written in the previous prepare write request by PTS*/
        for(int i=0; i< 300; i++)
        {
            attr_value[i] = 0x02;
        }
    }

    Ret = sGattIfaceScan->server->send_response(conn_id, trans_id, status, gatt_resp);
}

static void response_confirmation_cb(int status, int handle)
{
    printf("%s:: status=%d, handle =%d\n", __FUNCTION__, status, handle);
}

static void indication_sent_cb(int conn_id, int status)
{
    printf("%s:: status=%d, conn_id =%d\n", __FUNCTION__, status, conn_id);
}

static btgatt_server_callbacks_t     sGattServer_cb =
{
    register_server_cb,
    server_connection_cb, //connection_callback             connection_cb;
    NULL, //service_added_callback          service_added_cb;
    NULL, //included_service_added_callback included_service_added_cb;
    NULL, //characteristic_added_callback   characteristic_added_cb;
    request_read_cb, // request_read_callback           request_read_characteristic_cb
    request_read_cb, // request_read_callback           request_read_characteristic_cb
    request_write_cb, //request_write_callback          request_write_cb;
    request_write_cb, //request_write_callback          request_write_cb;
    request_exec_write_cb, // request_exec_write_callback     request_exec_write_cb;
    response_confirmation_cb, //response_confirmation_callback  response_confirmation_cb;
    indication_sent_cb, // indication_sent_callback        indication_sent_cb;
    NULL,
    NULL,
    NULL,
    NULL
};


/************************************************************************************
**  GATT Callbacks
************************************************************************************/
static void DiscoverRes_cb (uint16_t conn_id, tGATT_DISC_TYPE disc_type, tGATT_DISC_RES *p_data)
{
    printf("%s:: conn_id=%d, disc_type=%d\n", __FUNCTION__, conn_id, disc_type);
}

static void DiscoverCmpl_cb (uint16_t conn_id, tGATT_DISC_TYPE disc_type, tGATT_STATUS status)
{
    printf("%s:: conn_id=%d, disc_type=%d, status=%d\n", __FUNCTION__, conn_id, disc_type, status);
}

static void  OperationCmpl_cb(uint16_t conn_id, tGATTC_OPTYPE op, tGATT_STATUS status, tGATT_CL_COMPLETE *p_data)
{
    printf("%s:: conn_id=%d, op=%d, status=%d\n", __FUNCTION__, conn_id, op, status);
}

static void Connection_cb (tGATT_IF gatt_if, const RawAddress& bda, uint16_t conn_id, bool connected, tGATT_DISCONN_REASON reason,tBT_TRANSPORT transport)
{
    printf("%s:: remote_bd_addr=%02x:%02x:%02x:%02x:%02x:%02x, conn_id=0x%x, connected=%d, reason=%d, gatt_if=%d \n", __FUNCTION__,
            bda.address[0], bda.address[1], bda.address[2], bda.address[3], bda.address[4], bda.address[5],
            conn_id, connected, reason, gatt_if);
    g_conn_id = conn_id;
}

static void AttributeReq_cb(uint16_t conn_id, uint32_t trans_id, tGATTS_REQ_TYPE type, tGATTS_DATA *p_data)
{
    printf("%s:: conn_id=%d, trans_id=%d, type=%u\n", __FUNCTION__, conn_id, trans_id, type);
}


static tGATT_CBACK sGattCB =
{
    Connection_cb,
    OperationCmpl_cb,
    DiscoverRes_cb,
    DiscoverCmpl_cb,
    AttributeReq_cb,
    NULL,
    NULL,
    NULL,
    NULL
};

/************************************************************************************
**  GAP Callbacks
************************************************************************************/
/*
static void gap_ble_s_attr_request_cback (uint16_t conn_id, uint32_t trans_id, tGATTS_REQ_TYPE op_code, tGATTS_DATA *p_data)
{
    printf("%s:: conn_id=%d, trans_id=%d, op_code=%u\n", __FUNCTION__, conn_id, trans_id, op_code);
}

// client connection callback

static void  gap_ble_c_connect_cback (tGATT_IF gatt_if, BD_ADDR bda, uint16_t conn_id, bool connected, tGATT_DISCONN_REASON reason,tBT_TRANSPORT transport)
{
    printf("%s:: gatt_if=%d, remote_bd_addr=%02x:%02x:%02x:%02x:%02x:%02x, conn_id=%d, connected=%d, reason=%d\n", __FUNCTION__,
    gatt_if, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], conn_id, connected, reason);
    g_conn_id = conn_id;
}

static void  gap_ble_c_cmpl_cback (uint16_t conn_id, tGATTC_OPTYPE op, tGATT_STATUS status, tGATT_CL_COMPLETE *p_data)
{
    printf("%s:: conn_id=%d, op=%d, status=%d\n", __FUNCTION__, conn_id, op, status);
}


static tGATT_CBACK gap_cback =
{
    gap_ble_c_connect_cback,
    gap_ble_c_cmpl_cback,
    NULL,
    NULL,
    gap_ble_s_attr_request_cback,
    NULL,
    NULL
};
*/


/************************************************************************************
**  SMP Callbacks
************************************************************************************/
static uint8_t SMP_cb (tSMP_EVT event, const RawAddress& bda, tSMP_EVT_DATA *p_data)
{
    printf("%s:: event=%d(1-SMP_IO_CAP_REQ_EVT, 2-SMP_SEC_REQUEST_EVT,    \
                   3-SMP_PASSKEY_NOTIF_EVT, 4-SMP_PASSKEY_REQ_EVT, 6-SMP_COMPLT_EVT),   \
                  \nremote_bd_addr=%02x:%02x:%02x:%02x:%02x:%02x, PassKey=%u \n", __FUNCTION__, event,
            bda.address[0], bda.address[1], bda.address[2], bda.address[3], bda.address[4], bda.address[5], p_data->passkey);
    switch(event)
    {
    case SMP_IO_CAP_REQ_EVT:
        printf("Io_Caps=%d, auth_req=%d, max_key_size=%d, init_keys=%d, resp_keys=%d \n", p_data->io_req.io_cap, p_data->io_req.auth_req, p_data->io_req.max_key_size, p_data->io_req.init_keys, p_data->io_req.resp_keys);
        break;

    case SMP_PASSKEY_REQ_EVT:
    case SMP_PASSKEY_NOTIF_EVT:
        printf("passkey value=%u\n", p_data->passkey);
        sSmpIface->PasskeyReply(bda, SMP_SUCCESS, p_data->passkey);
        break;
    case SMP_OOB_REQ_EVT:
        //p_dev_rec->sec_flags |= BTM_SEC_LINK_KEY_AUTHED;
        break;
    case SMP_SEC_REQUEST_EVT:
    case SMP_COMPLT_EVT:
        printf("SMP Complete Event:: Reason=%d \n", p_data->cmplt.reason);
        if(p_data->cmplt.reason == SMP_SUCCESS)
        {
            sSmpIface->SecurityGrant(bda, p_data->cmplt.reason);
            printf("Granting Security \n");
        }
        break;
    }
    return 0;
}


/************************************************************************************
**  Shutdown helper functions
************************************************************************************/

static void bdt_shutdown(void)
{
    bdt_log("shutdown bdroid test app\n");
    main_done = 1;
}


/*****************************************************************************
** Android's init.rc does not yet support applying linux capabilities
*****************************************************************************/

static void config_permissions(void)
{
    struct __user_cap_header_struct header;
    struct __user_cap_data_struct cap[2];

    bdt_log("set_aid_and_cap : pid %d, uid %d gid %d", getpid(), getuid(), getgid());

    header.pid = 0;

    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);

    setuid(AID_BLUETOOTH);
    setgid(AID_BLUETOOTH);

    header.version = _LINUX_CAPABILITY_VERSION_3;

    cap[CAP_TO_INDEX(CAP_NET_RAW)].permitted |= CAP_TO_MASK(CAP_NET_RAW);
    cap[CAP_TO_INDEX(CAP_NET_ADMIN)].permitted |= CAP_TO_MASK(CAP_NET_ADMIN);
    cap[CAP_TO_INDEX(CAP_NET_BIND_SERVICE)].permitted |= CAP_TO_MASK(CAP_NET_BIND_SERVICE);
    cap[CAP_TO_INDEX(CAP_SYS_RAWIO)].permitted |= CAP_TO_MASK(CAP_SYS_RAWIO);
    cap[CAP_TO_INDEX(CAP_SYS_NICE)].permitted |= CAP_TO_MASK(CAP_SYS_NICE);
    cap[CAP_TO_INDEX(CAP_SETGID)].permitted |= CAP_TO_MASK(CAP_SETGID);
    cap[CAP_TO_INDEX(CAP_WAKE_ALARM)].permitted |= CAP_TO_MASK(CAP_WAKE_ALARM);

    cap[CAP_TO_INDEX(CAP_NET_RAW)].effective |= CAP_TO_MASK(CAP_NET_RAW);
    cap[CAP_TO_INDEX(CAP_NET_ADMIN)].effective |= CAP_TO_MASK(CAP_NET_ADMIN);
    cap[CAP_TO_INDEX(CAP_NET_BIND_SERVICE)].effective |= CAP_TO_MASK(CAP_NET_BIND_SERVICE);
    cap[CAP_TO_INDEX(CAP_SYS_RAWIO)].effective |= CAP_TO_MASK(CAP_SYS_RAWIO);
    cap[CAP_TO_INDEX(CAP_SYS_NICE)].effective |= CAP_TO_MASK(CAP_SYS_NICE);
    cap[CAP_TO_INDEX(CAP_SETGID)].effective |= CAP_TO_MASK(CAP_SETGID);
    cap[CAP_TO_INDEX(CAP_WAKE_ALARM)].effective |= CAP_TO_MASK(CAP_WAKE_ALARM);

    capset(&header, &cap[0]);
    setgroups(sizeof(groups)/sizeof(groups[0]), groups);
}


/*****************************************************************************
**   Logger API
*****************************************************************************/

void bdt_log(const char *fmt_str, ...)
{
    static char buffer[1024];
    va_list ap;

    va_start(ap, fmt_str);
    vsnprintf(buffer, 1024, fmt_str, ap);
    va_end(ap);

    fprintf(stdout, "%s\n", buffer);
}

/*******************************************************************************
 ** Misc helper functions
 *******************************************************************************/
static const char* dump_bt_status(int status)
{
    switch(status)
    {
        CASE_RETURN_STR(BT_STATUS_SUCCESS)
        CASE_RETURN_STR(BT_STATUS_FAIL)
        CASE_RETURN_STR(BT_STATUS_NOT_READY)
        CASE_RETURN_STR(BT_STATUS_NOMEM)
        CASE_RETURN_STR(BT_STATUS_BUSY)
        CASE_RETURN_STR(BT_STATUS_UNSUPPORTED)

        default:
            return "unknown status code";
    }
}


/*******************************************************************************
 ** Console helper functions
 *******************************************************************************/

void skip_blanks(char **p)
{
    while (**p == ' ')
    (*p)++;
}

uint32_t get_int(char **p, int DefaultValue)
{
    uint32_t Value = 0;
    unsigned char   UseDefault;

    UseDefault = 1;
    skip_blanks(p);

    while ( ((**p)<= '9' && (**p)>= '0') )
    {
        Value = Value * 10 + (**p) - '0';
        UseDefault = 0;
        (*p)++;
    }
   if (UseDefault)
       return DefaultValue;
   else
       return Value;
}

int get_signed_int(char **p, int DefaultValue)
{
    int    Value = 0;
    unsigned char   UseDefault;
    unsigned char  NegativeNum = 0;

    UseDefault = 1;
    skip_blanks(p);

    if ((**p) == '-')
    {
        NegativeNum = 1;
        (*p)++;
    }
    while ( ((**p)<= '9' && (**p)>= '0') )
    {
        Value = Value * 10 + (**p) - '0';
        UseDefault = 0;
        (*p)++;
    }

    if (UseDefault)
        return DefaultValue;
    else
        return ((NegativeNum == 0)? Value : -Value);
}

void get_str(char **p, char *Buffer)
{
    skip_blanks(p);
    while (**p != 0 && **p != ' ')
    {
        *Buffer = **p;
        (*p)++;
        Buffer++;
    }

    *Buffer = 0;
}

uint32_t get_hex_any(char **p, int DefaultValue, unsigned int NumOfNibble)
{
    uint32_t Value = 0;
    unsigned char   UseDefault;

    UseDefault = 1;
    skip_blanks(p);

    while ((NumOfNibble) && (((**p)<= '9' && (**p)>= '0') ||
          ((**p)<= 'f' && (**p)>= 'a') ||
          ((**p)<= 'F' && (**p)>= 'A')) )
    {
        if (**p >= 'a')
            Value = Value * 16 + (**p) - 'a' + 10;
        else if (**p >= 'A')
            Value = Value * 16 + (**p) - 'A' + 10;
        else
        Value = Value * 16 + (**p) - '0';
        UseDefault = 0;
        (*p)++;
        NumOfNibble--;
    }

    if (UseDefault)
        return DefaultValue;
    else
        return Value;
}
uint32_t get_hex(char **p, int DefaultValue)
{
    return (get_hex_any(p, DefaultValue, 8));
}
uint32_t get_hex_byte(char **p, int DefaultValue)
{
    return (get_hex_any(p, DefaultValue, 2));
}

std::string get_uuid_str(char **p, int uuid_len_bytes)
{
    std::string uuid_str, temp;
    skip_blanks(p);
    switch(uuid_len_bytes)
    {
        case 2: //16 bit uuid
            for(int i=0; i<4; i++)
            {
                temp = temp + (**p);
                (*p)++;
            }
            uuid_str = "0000"+temp+"-0000-1000-8000-00805F9B34FB";
            break;

        case 4: //32 bit uuid
            for(int i=0; i<8; i++)
            {
                temp = temp + (**p);
                (*p)++;
            }
            uuid_str = temp+"-0000-1000-8000-00805F9B34FB";
            break;

        case 16: //128 bit uuid
            for(int i=0; i<32; i++)
            {
                temp = temp + (**p);
                (*p)++;
                if (i == 7 || i == 11 || i == 15 || i == 19)
                    temp = temp + "-";
            }
            uuid_str = temp;
            break;

        default:
            printf("%s:: ERROR: no matching uuid \n", __FUNCTION__);
    }
    printf("%s:: uuid_str = %s \n", __FUNCTION__, uuid_str.c_str());
    return uuid_str;
}


#define is_cmd(str) ((strlen(str) == strlen(cmd)) && strncmp((const char *)&cmd, str, strlen(str)) == 0)
#define if_cmd(str)  if (is_cmd(str))

typedef void (t_console_cmd_handler) (char *p);

typedef struct {
    const char *name;
    t_console_cmd_handler *handler;
    const char *help;
    unsigned char is_job;
} t_cmd;

void do_help(char *p);
void do_quit(char *p);
void do_init(char *p);
void do_enable(char *p);
void do_disable(char *p);
void do_dut_mode_configure(char *p);
void do_le_test_mode(char *p);
void do_cleanup(char *p);
void do_le_client_register(char *p);
void do_le_client_deregister(char *p);
void do_le_client_connect (char *p);
void do_le_client_refresh (char *p);
void do_le_conn_param_update(char *p);
void do_le_client_connect_auto (char *p);
void do_le_client_disconnect (char *p);
void do_le_client_scan_start (char *p);
void do_le_client_scan_stop (char *p);
void do_le_client_set_adv_data(char *p);
void do_le_client_set_adv_mode(char *p);
void do_le_client_multi_adv_set_inst_data(char *p);
void do_le_client_adv_update(char *p);
void do_le_client_adv_enable(char *p);
void do_le_client_adv_disable(char *p);
void do_le_client_configureMTU(char *p);
void do_le_client_discover(char *p);
void do_le_client_read(char *p);
void do_le_client_write(char *p);
void do_le_execute_write(char *p);
void do_le_set_idle_timeout(char *p);
void do_le_server_register(char *p);
void do_le_server_deregister(char *p);
void do_le_server_add_service(char *p);
void do_le_server_connect (char *p);
void do_le_server_connect_auto (char *p);
void do_le_server_disconnect (char *p);
void do_le_server_send_indication(char *p);
void do_smp_init(char *p);
void do_smp_pair(char *p);
void do_smp_pair_cancel(char *p);
void do_smp_security_grant(char *p);
void do_smp_passkey_reply(char *p);
void do_smp_encrypt(char *p);
void do_le_gap_conn_param_update(char *p);
void do_le_gap_attr_init(char *p);
void do_le_set_char_len(char *p);
void do_pairing(char *p);
void do_l2cap_send_data_cid(char *p);
static void do_set_localname(char *p);
void do_start_adv_set(char *p);
//void do_register_adv(char *p);
void do_unregister_adv_set(char *p);

/*******************************************************************
 *
 *  CONSOLE COMMAND TABLE
 *
*/

const t_cmd console_cmd_list[] =
{
    /*
     * INTERNAL
     */

    { "help", do_help, "lists all available console commands", 0 },
    { "quit", do_quit, "", 0},

    /*
     * API CONSOLE COMMANDS
     */

     /* Init and Cleanup shall be called automatically */
    { "enable", do_enable, ":: enables bluetooth", 0 },
    { "disable", do_disable, ":: disables bluetooth", 0 },
    { "dut_mode_configure", do_dut_mode_configure, ":: DUT mode - 1 to enter,0 to exit", 0 },
    { "c_register", do_le_client_register, "::UUID: 1<1111..> 2<12323..> 3<321111..>", 0 },
    { "c_deregister", do_le_client_deregister, "::UUID: 1<1111..> 2<12323..> 3<321111..>", 0 },
    { "c_connect", do_le_client_connect, ":: transport-type<0,1...> , BdAddr<00112233445566>", 0 },
    { "c_refresh", do_le_client_refresh, ":: BdAddr<00112233445566>", 0 },
    { "c_conn_param_update", do_le_conn_param_update, ":: min_interval(hex), max_interval(hex), latency(hex), timeout(hex), BdAddr<00112233445566>", 0 },
    { "c_connect_auto", do_le_client_connect_auto, ":: transport-type<0,1...> , BdAddr<00112233445566>", 0 },
    { "c_disconnect", do_le_client_disconnect, ":: transport-type<0,1...>, BdAddr<00112233445566>", 0 },
    { "c_configureMTU", do_le_client_configureMTU, ":: int mtu_size", 0 },
    { "c_discover", do_le_client_discover, "type(1-PrimaryService, 2-PrimaryService using UUID, 3-Included Service, 4-Characteristic, 5-Characteristic Descriptor) \
                                            \n\t s.handle(hex) e.handle(hex) UUIDLen(16/32/128) UUID(hex)", 0 },
    { "c_read", do_le_client_read, "Type(1-ByType, 2-ByHandle, 3-ByMultiple, 4-CharValue, 5-Partial (blob)) Auth_Req \
                                    \n\t ByType       :: s.handle(hex) e.handle(hex) UUIDLen(16/32/128) UUID(hex) \
                                    \n\t ByHandle     :: Handle(hex) \
                                    \n\t ByMultiple   :: NumOfHandle<1-10> Handle_1(hex) Handle_2(hex) ... Handle_N(hex) \
                                    \n\t CharValue    :: s.handle(hex) e.handle(hex) UUIDLen(16/32/128) UUID(hex) \
                                    \n\t Partial/Blob :: Handle(hex) Offset(hex)", 0 },
    { "c_write", do_le_client_write, "Type(1-No response, 2-write, 3-prepare write) Auth_req Handle Offset Len(0-600) Value(hex)", 0 },
    { "c_execute_write", do_le_execute_write, "is_execute", 0 },
    { "c_scan_start", do_le_client_scan_start, "::", 0 },
    { "c_scan_stop", do_le_client_scan_stop, "::", 0 },
    //{ "c_listen_start", do_le_client_listen_start, "::", 0 },
    //{ "c_listen_stop", do_le_client_listen_stop, "::", 0 },
   /* { "c_set_adv_mode", do_le_client_set_adv_mode, ":: Discoverability mode,Connectable_mode,", 0 },
    { "c_set_adv_data", do_le_client_set_adv_data, "::EnableScanrsp<0/1>, IncludeName<0/1> IncludeTxPower<0/1>,min_conn_interval,int  max_conn_interval", 0 },
    { "c_set_multi_adv_data", do_le_client_multi_adv_set_inst_data, "::EnableScanrsp<0/1>, IncludeName<0/1> IncludeTxPower<0/1>", 0 },
    { "start_advertising", do_le_client_adv_enable, "::int client_if,int min_interval,int max_interval,int adv_type,int chnl_map, int tx_power timeout",0},
    { "c_adv_update", do_le_client_adv_update, "::int client_if, int min_interval,int max_interval,int adv_type,int chnl_map, int tx_power, int timeout",0},
    { "stop_advertising", do_le_client_adv_disable, "::int adv_if",0},*/
    { "c_set_idle_timeout", do_le_set_idle_timeout, "bd_addr, time_out(int)", 0 },
    { "c_gap_attr_init", do_le_gap_attr_init, "::", 0 },
    { "c_gap_conn_param_update", do_le_gap_conn_param_update, "::", 0 },
    { "c_set_char_len", do_le_set_char_len, ":: <Default value: 512>", 0},

    { "s_register", do_le_server_register, "::UUID: 1<1111..> 2<12323..> 3<321111..>", 0 },
    { "s_connect", do_le_server_connect, ":: transport, BdAddr<00112233445566>", 0 },
    { "s_connect_auto", do_le_server_connect_auto, ":: BdAddr<00112233445566>", 0 },
    { "s_disconnect", do_le_server_disconnect, ":: transport, BdAddr<00112233445566>", 0 },
    { "s_add_service", do_le_server_add_service, "::", 0 },
    { "s_send_indication", do_le_server_send_indication, "::handle(hex), confirm (1 for indication, 0 for notification)", 0 },

    { "pair", do_pairing, ":: BdAddr<00112233445566>", 0 },

    { "smp_init", do_smp_init, "::", 0 }, //Here itself we will register.
    { "smp_pair", do_smp_pair, ":: BdAddr<00112233445566>", 0 },
    { "smp_pair_cancel", do_smp_pair_cancel, ":: BdAddr<00112233445566>", 0 },
    { "smp_security_grant", do_smp_security_grant, ":: BdAddr<00112233445566>, res<>", 0 },
    { "smp_passkey_reply", do_smp_passkey_reply, ":: BdAddr<00112233445566>, res<>, passkey<>", 0 },
    //{ "smp_encrypt", do_smp_encrypt, "::", 0 },
    { "l2cap_send_data_cid", do_l2cap_send_data_cid, ":: BdAddr<00112233445566>, CID<>", 0 },

    { "set_local_name", do_set_localname, ":: setName<name>", 0 },
    /* add here */
    //{ "register_advertiser", do_register_adv, ":: RegisterAdvertiser", 0 },
    { "start_adv_set", do_start_adv_set, ":: adv_evt_properties(hex eg:13 for connectable and scannable legacy adv), "
            "\n interval(1-interval low, 2-interval medium, 3-interval high),"
            "\n primary phy(1 for LE 1M, 3 for LE Coded),"
            "\n secondary phy (1 for LE 1M, 2 for LE 2M and 3 for LE Coded)", 0 },
    { "unregister_advertiser", do_unregister_adv_set, ":: UnregisterAdvertiser", 0 },

     /* LE-L2CAP cmds */
    { " ", NULL, "\n\t\t\033[0m\033[34mLE L2CAP CoC Commands\033[0m", 0 },
    { " ", NULL, "\033[0m\033[34mCommands\t\t\tParameters\033[0m", 0 },

    { "start_adv", do_start_advertisment, "\t\t::\tuuid [0 - register"\
        "none (or) 1 - register uuid] , \n\t\t\t\tflag [0- stop adv (or) "\
        "1 - start adv] \n  " , 0},

    { "le_l2cap_listen", le_l2cap_listen, "\t::\tle_psm [1 to 255], "\
        "\n\t\t\t\tle_mtu [23 to 65535], \n\t\t\t\tle_mps [23 to 65533],"\
        " \n\t\t\t\tinit_credits [0 to 65535], \n\t\t\t\tsec_level "\
        "[0 - None, 1 - Authentication, 2 - Auth and Encryption]", 0},

    { "le_l2cap_coc_connect", le_l2cap_coc_connect, "\t::\tle_psm [128 to 255],"\
        "\n\t\t\t\tle_mtu [23 to 65535], \n\t\t\t\tle_mps [23 to 65533], "\
        "\n\t\t\t\tinit_credits [0 to 65535], \n\t\t\t\tsec_level [0 - None,"\
        " 1 - Authentication, 2 - Encryption], \n\t\t\t\tbd_addr [001122334455] ", 0},

    { "le_l2cap_coc_flow_ctrl", le_l2cap_coc_flow_ctrl, "\t::\tchnl_id [chnl id"\
        "info from conn_ind or conn_cnf], \n\t\t\t\tcredits [1 to 65535]", 0},

    { "send_data_on_le_coc", send_data_on_le_coc, "\t::\tchnl_id [chnl id info "\
        "from conn_ind or conn_cnf] , \n\t\t\t\tdata_type [0 - Unsegmented data ,"\
        "1 - Segmented data]", 0},

    { "send_file", do_send_file, "\t\t::\tchnl_id [chnl_id info from conn_ind"\
        "or conn_cnf], \n\t\t\t\tfile_name", 0},

    { "le_coc_disconnect", do_le_coc_disconnect, "\t::\tchnl_id [chnl_id info"\
        "from conn_ind or conn_cnf]", 0},

    /* last entry */
    {NULL, NULL, "", 0},
};


static int console_cmd_maxlen = 0;

static void *cmdjob_handler(void *param)
{
    char *job_cmd = (char*)param;

    bdt_log("cmdjob starting (%s)", job_cmd);

    process_cmd(job_cmd, 1);

    bdt_log("cmdjob terminating");

    free(job_cmd);
    return NULL;
}

static int create_cmdjob(char *cmd)
{
    pthread_t thread_id;
    char *job_cmd;

    job_cmd = (char*)calloc(1, strlen(cmd)+1); /* freed in job handler */
    if (job_cmd) {
       strlcpy(job_cmd, cmd,(strlen(cmd)+1));
       if (pthread_create(&thread_id, NULL, cmdjob_handler, (void *)job_cmd) != 0)
      /*if (pthread_create(&thread_id, NULL,
                       (void*)cmdjob_handler, (void*)job_cmd) !=0)*/
         perror("pthread_create");
      return 0;
    }
    else
       perror("create_Cmdjob malloc failed ");
    return -1;
}

/*******************************************************************************
 ** Load stack lib
 *******************************************************************************/
#define BLUETOOTH_LIBRARY_NAME "libbluetooth_qti.so"
int load_bt_lib(const bt_interface_t** interface) {
  const char* sym = BLUETOOTH_INTERFACE_STRING;
  bt_interface_t* itf = nullptr;

  // Always try to load the default Bluetooth stack on GN builds.
  const char* path = BLUETOOTH_LIBRARY_NAME;
  void* handle = dlopen(path, RTLD_NOW);
  if (!handle) {
    //const char* err_str = dlerror();
    printf("failed to load Bluetooth library\n");
    goto error;
  }

  // Get the address of the bt_interface_t.
  itf = (bt_interface_t*)dlsym(handle, sym);
  if (!itf) {
    printf("failed to load symbol from Bluetooth library\n");
    goto error;
  }

  // Success.
  printf(" loaded HAL Success\n");
  *interface = itf;
  return 0;

error:
  *interface = NULL;
  if (handle) dlclose(handle);

  return -EINVAL;
}

int HAL_load(void)
{
    if (load_bt_lib((bt_interface_t const**)&sBtInterface)) {
        printf("No Bluetooth Library found\n");
        return -1;
    }
    return 0;
}

int HAL_unload(void)
{
    int err = 0;

    bdt_log("Unloading HAL lib");

    sBtInterface = NULL;

    bdt_log("HAL library unloaded (%s)", strerror(err));

    return err;
}

/*******************************************************************************
 ** HAL test functions & callbacks
 *******************************************************************************/

void setup_test_env(void)
{
    int i = 0;

    while (console_cmd_list[i].name != NULL)
    {
        console_cmd_maxlen = MAX(console_cmd_maxlen, (int)strlen(console_cmd_list[i].name));
        i++;
    }
}

void check_return_status(int status)
{
    if (status != BT_STATUS_SUCCESS)
    {
        bdt_log("HAL REQUEST FAILED status : %d (%s)", status, dump_bt_status(status));
    }
    else
    {
        bdt_log("HAL REQUEST SUCCESS");
    }
}
/*
static void do_set_adv_params(char *p)
{
    bt_bdaddr_t bd_addr = {{0}};
    int int_min = 0x0, int_max = 0x0, addr_type = 0;
    int_max = get_int(&p, -1);
    int_min = get_int(&p, -1);
    if(int_max < int_min)
        return;
//    if(FALSE == GetBdAddr(p, &bd_addr))    return;
//    sBtInterface->le_set_adv_params(int_min, int_max, &bd_addr, addr_type);
}
*/

static void do_set_localname(char *p)
{
    printf("set name in progress: %s\n", p);
    bt_property_t property = {BT_PROPERTY_BDNAME, static_cast<int>(strlen(p)), p};
    status =  sBtInterface->set_adapter_property(&property);
}

static void adapter_state_changed(bt_state_t state)
{
    int V1 = 1000, V2=2;
    char V3[] = "GATTTOOL";
    bt_property_t property = {(bt_property_type_t)9 /*BT_PROPERTY_DISCOVERY_TIMEOUT*/, 4, &V1};
    bt_property_t property1 = {(bt_property_type_t)7 /*SCAN*/, 2, &V2};
    bt_property_t property2 ={(bt_property_type_t)1,9, &V3};
    printf("ADAPTER STATE UPDATED : %s\n", (state == BT_STATE_OFF)?"OFF":"ON");

    g_AdapterState = state;

    if (state == BT_STATE_ON) {
        bt_enabled = 1;
        status = sBtInterface->set_adapter_property(&property1);
        status = sBtInterface->set_adapter_property(&property);
        status = sBtInterface->set_adapter_property(&property2);
    } else {
        bt_enabled = 0;
    }
}

static void adapter_properties_changed(bt_status_t status, int num_properties, bt_property_t *properties)
{
 char Bd_addr[15] = {0};
    if(NULL == properties)
    {
        printf("properties is null\n");
        return;
    }
    switch(properties->type)
    {
    case BT_PROPERTY_BDADDR:
        memcpy(Bd_addr, properties->val, properties->len);
        break;
//    case BT_PROPERTY_ADAPTER_BLE_ADV_MODE:
 //       printf("Set in advertisement mode\n");
 //       break;
    default:
        printf("property type not used\n");
    }
    return;
}

static void discovery_state_changed(bt_discovery_state_t state)
{
    printf("Discovery State Updated : %s\n", (state == BT_DISCOVERY_STOPPED)?"STOPPED":"STARTED");
}


static void pin_request_cb(RawAddress* remote_bd_addr, bt_bdname_t *bd_name, uint32_t cod, bool min_16_digit )
{
    remote_bd_address = remote_bd_addr;
    //bt_pin_code_t pincode = {{0x31, 0x32, 0x33, 0x34}};
    printf("Enter the pin key displayed in the remote device and terminate the key entry with .\n");

    /*if(BT_STATUS_SUCCESS != sBtInterface->pin_reply(remote_bd_addr, TRUE, 4, &pincode))
    {
        printf("Pin Reply failed\n");
    }*/
}
static void ssp_request_cb(RawAddress* remote_bd_addr, bt_bdname_t *bd_name,
                           uint32_t cod, bt_ssp_variant_t pairing_variant, uint32_t pass_key)
{
    printf("ssp_request_cb : name=%s variant=%d passkey=%u\n", bd_name->name, pairing_variant, pass_key);
    if(BT_STATUS_SUCCESS != sBtInterface->ssp_reply(remote_bd_addr, pairing_variant, TRUE, pass_key))
    {
        printf("SSP Reply failed\n");
    }
}

static void bond_state_changed_cb(bt_status_t status, RawAddress* remote_bd_addr, bt_bond_state_t state)
{
    g_PairState = state;
}

static void acl_state_changed(bt_status_t status, RawAddress* remote_bd_addr, bt_acl_state_t state)
{
    printf("acl_state_changed : remote_bd_addr=%02x:%02x:%02x:%02x:%02x:%02x, acl status=%s \n",
    remote_bd_addr->address[0], remote_bd_addr->address[1], remote_bd_addr->address[2],
    remote_bd_addr->address[3], remote_bd_addr->address[4], remote_bd_addr->address[5],
    (state == BT_ACL_STATE_CONNECTED)?"ACL Connected" :"ACL Disconnected"
    );
}
static void dut_mode_recv(uint16_t opcode, uint8_t *buf, uint8_t len)
{
    bdt_log("DUT MODE RECV : NOT IMPLEMENTED");
}

static void le_test_mode(bt_status_t status, uint16_t packet_count)
{
    bdt_log("LE TEST MODE END status:%s number_of_packets:%d", dump_bt_status(status), packet_count);
}

extern int timer_create (clockid_t, struct sigevent *__restrict, timer_t *__restrict);
extern int timer_settime (timer_t, int, const struct itimerspec *__restrict, struct itimerspec *__restrict);

static bool set_wake_alarm(uint64_t delay_millis, bool should_wake, alarm_cb cb, void *data)
{

   static timer_t timer;
   static bool timer_created;

   if (!timer_created) {
      struct sigevent sigevent;
      memset(&sigevent, 0, sizeof(sigevent));
      sigevent.sigev_notify = SIGEV_THREAD;
      sigevent.sigev_notify_function = (void (*)(union sigval))cb;
      sigevent.sigev_value.sival_ptr = data;
      timer_create(CLOCK_MONOTONIC, &sigevent, &timer);
      timer_created = true;
   }

   struct itimerspec new_value;
   new_value.it_value.tv_sec = delay_millis / 1000;
   new_value.it_value.tv_nsec = (delay_millis % 1000) * 1000 * 1000;
   new_value.it_interval.tv_sec = 0;
   new_value.it_interval.tv_nsec = 0;
   timer_settime(timer, 0, &new_value, NULL);

  return TRUE;
}

static int acquire_wake_lock(const char *lock_name)
{
    return BT_STATUS_SUCCESS;
}

static int release_wake_lock(const char *lock_name)
{
    return BT_STATUS_SUCCESS;
}

static bt_callbacks_t bt_callbacks = {
    sizeof(bt_callbacks_t),
    adapter_state_changed,
    adapter_properties_changed, /*adapter_properties_cb */
    NULL, /* remote_device_properties_cb */
    NULL, /* device_found_cb */
    discovery_state_changed, /* discovery_state_changed_cb */
    pin_request_cb, /* pin_request_cb  */
    ssp_request_cb, /* ssp_request_cb  */
    bond_state_changed_cb, /*bond_state_changed_cb */
    acl_state_changed, /* acl_state_changed_cb */
    NULL, /* thread_evt_cb */
    dut_mode_recv, /*dut_mode_recv_cb */
    le_test_mode, /* le_test_mode_cb */
    NULL      /*energy_info_cb*/
};

static bt_os_callouts_t bt_os_callbacks = {
     sizeof(bt_os_callouts_t),
     set_wake_alarm,
     acquire_wake_lock,
     release_wake_lock
};

static void l2test_l2c_connect_ind_cb(const RawAddress& bd_addr, uint16_t lcid, uint16_t psm, uint8_t id)
{

    uint16_t result;

    local_coc_cfg.credits = L2CAP_LE_DEFAULT_CREDIT;
    local_coc_cfg.mtu = L2CAP_LE_DEFAULT_MTU;
    local_coc_cfg.mps = L2CAP_LE_DEFAULT_MPS;
    /* Verify if LE PSM  */
   if (L2C_IS_VALID_LE_PSM(psm))
   {
       if (psm == 200)
       {
           printf("No Resources Available\n");
           result = L2CAP_LE_RESULT_NO_RESOURCES;
           sL2capInterface->LeConnectRsp (bd_addr, id, lcid, result,L2CAP_LE_RESULT_CONN_OK,&local_coc_cfg);
       }
       else if(psm == 201)
       {
           printf("L2CAP_LE_CONN_INSUFFI_AUTHORIZATION \n");
           result = L2CAP_LE_RESULT_INSUFFICIENT_AUTHORIZATION;
           sL2capInterface->LeConnectRsp (bd_addr, id, lcid, result,L2CAP_LE_RESULT_CONN_OK,&local_coc_cfg);
       }
       else
       {

           result = L2CAP_LE_RESULT_CONN_OK;
           sL2capInterface->LeConnectRsp (bd_addr, id, lcid, result,L2CAP_LE_RESULT_CONN_OK,&local_coc_cfg);
       }
       return;
   }
    if((L2CAP_FCR_ERTM_MODE == g_Fcr_Mode) || (L2CAP_FCR_STREAM_MODE == g_Fcr_Mode)) {
        sL2capInterface->ErtmConnectRsp(bd_addr, id, lcid, L2CAP_CONN_OK, L2CAP_CONN_OK, &t_ertm_info);
    } else {
        sL2capInterface->ConnectRsp(bd_addr, id, lcid, L2CAP_CONN_OK, L2CAP_CONN_OK);
    }
    {
        tL2CAP_CFG_INFO cfg;
        memcpy (&cfg ,&tl2cap_cfg_info,sizeof(tl2cap_cfg_info));
        if ((!sL2capInterface->ConfigReq (lcid, &cfg)) && cfg.fcr_present
              && cfg.fcr.mode != L2CAP_FCR_BASIC_MODE) {
            cfg.fcr.mode = L2CAP_FCR_BASIC_MODE;
            cfg.fcr_present = FALSE;
            sL2capInterface->ConfigReq (lcid, &cfg);
        }
    }
    g_ConnectionState = CONNECT;
    g_lcid = lcid;
}

static void l2test_l2c_connect_cfm_cb(uint16_t lcid, uint16_t result)
{
    t_le_chnl_info *le_conn_info = le_get_conn_info_by_lcid(lcid);
    if (le_conn_info&&L2C_IS_VALID_LE_PSM(le_conn_info->psm))
    {

        if (result == L2CAP_LE_RESULT_CONN_OK) {
            g_ConnectionState = CONNECT;
        }
        else if(le_conn_info && !le_conn_info->is_server)
        {
            le_release_conn_info(le_conn_info);
        }
        return;
    }
#if 0
    if (result == L2CAP_CONN_OK) {
        L2cap_conn_state = L2CAP_CONN_SETUP;
        tL2CAP_CFG_INFO cfg;
        memcpy (&cfg ,&tl2cap_cfg_info,sizeof(tl2cap_cfg_info));
        sL2capInterface->ConfigReq (lcid, &cfg);
        g_imtu = cfg.mtu;
        g_ConnectionState = CONNECT;
        g_lcid = lcid;
    }
#endif
}

static void l2test_l2c_connect_pnd_cb(uint16_t lcid)
{
    g_ConnectionState = CONNECTING;
}
static void l2test_l2c_config_ind_cb(uint16_t lcid, tL2CAP_CFG_INFO *p_cfg)
{
    p_cfg->result = L2CAP_CFG_OK;
    p_cfg->fcr_present = FALSE;
    if(p_cfg->mtu_present) g_omtu = p_cfg->mtu;
    else g_omtu = L2CAP_DEFAULT_MTU;
    sL2capInterface->ConfigRsp (lcid, p_cfg);
    return;
}

static void l2test_l2c_config_cfm_cb(uint16_t lcid, tL2CAP_CFG_INFO *p_cfg)
{

    /* For now, always accept configuration from the other side */
    if (p_cfg->result == L2CAP_CFG_OK) {
        printf("\nl2test_l2c_config_cfm_cb Success\n");
    } else {

     /* If peer has rejected FCR and suggested basic then try basic */
    if (p_cfg->fcr_present) {
        tL2CAP_CFG_INFO cfg;
        memcpy (&cfg ,&tl2cap_cfg_info,sizeof(tl2cap_cfg_info));
        cfg.fcr_present = FALSE;
        sL2capInterface->ConfigReq (lcid, &cfg);
        // Remain in configure state
        return;
    }
    sL2capInterface->DisconnectReq(lcid);
    }
    if(0 == g_omtu) g_omtu = L2CAP_DEFAULT_MTU;
}

static void l2test_l2c_disconnect_ind_cb(uint16_t lcid, bool ack_needed)
{
    t_le_chnl_info *le_conn_info = le_get_conn_info_by_lcid(lcid);
    /* release the conn info entry if it'a  client */
    if(le_conn_info &&  !le_conn_info->is_server)
    {
        le_release_conn_info(le_conn_info);
    }
    printf("l2test_le_l2c_disconnect_ind_cb, cid=0x%x, acks=%d\n", lcid, ack_needed);
    if (ack_needed)
    {
        /* send L2CAP disconnect response */
        sL2capInterface->DisconnectRsp(lcid);
    }
    g_ConnectionState = DISCONNECTING;
    g_lcid = 0;
}
static void l2test_l2c_disconnect_cfm_cb(uint16_t lcid, uint16_t result)
{
    t_le_chnl_info *le_conn_info = le_get_conn_info_by_lcid(lcid);
    /* release the conn info entry if it'a  client */
    if(le_conn_info &&  !le_conn_info->is_server)
    {
        le_release_conn_info(le_conn_info);
    }

    printf("l2test_le_l2c_disconnect_cfm_cb, cid=0x%x, result=%d\n", lcid, result);
    g_ConnectionState = DISCONNECT;
    g_lcid = 0;
}
static void l2test_l2c_QoSViolationInd(const RawAddress& bd_addr)
{
    printf("l2test_l2c_QoSViolationInd\n");
}
static void l2test_l2c_data_ind_cb(uint16_t lcid, BT_HDR *p_buf)
{
    rcv_itration++;
    printf("l2test_l2c_data_ind_cb:: itration=%d, event=%u, len=%u, "\
            "offset=%u, layer_specific=%u\n",rcv_itration, p_buf->event,
            p_buf->len, p_buf->offset, p_buf->layer_specific);
    sL2capInterface->LeFreeBuf(p_buf);
     printf("l2test_l2c_data_ind_cb:: event=%u, len=%u, offset=%u, layer_specific=%u\n", p_buf->event, p_buf->len, p_buf->offset, p_buf->layer_specific);
}
static void l2test_l2c_congestion_ind_cb(uint16_t lcid, bool is_congested)
{
    cong_status = is_congested;
    printf("l2test_l2c_congestion_ind_cb is_congested %d\n ", is_congested);
}

static void l2test_l2c_tx_complete_cb (uint16_t lcid, uint16_t NoOfSDU)
{
    printf("l2test_l2c_tx_complete_cb, cid=0x%x, SDUs=%u\n", lcid, NoOfSDU);
}

/*
static void l2c_echo_rsp_cb(uint16_t p)
{
    printf("Ping Response = %s\n", (L2CAP_PING_RESULT_OK==p) ?"Ping Reply OK" :(L2CAP_PING_RESULT_NO_LINK==p) ?"Link Could Not be setup" :"Remote L2cap did not reply");
}
*/

/* L2CAP callback function structure */
static tL2CAP_APPL_INFO l2test_l2c_appl = {
  //  sizeof(l2test_l2c_appl),
    l2test_l2c_connect_ind_cb,
    l2test_l2c_connect_cfm_cb,
    l2test_l2c_connect_pnd_cb,
    l2test_l2c_config_ind_cb,
    l2test_l2c_config_cfm_cb,
    l2test_l2c_disconnect_ind_cb,
    l2test_l2c_disconnect_cfm_cb,
    l2test_l2c_QoSViolationInd,
    l2test_l2c_data_ind_cb,
    l2test_l2c_congestion_ind_cb,
    l2test_l2c_tx_complete_cb
};


void bdt_init(void)
{
    bdt_log("INIT BT ");
    status = sBtInterface->init(&bt_callbacks, false, false);
    if (status == BT_STATUS_SUCCESS) {
        // Get Vendor Interface
        btvendorInterface = (btvendor_interface_t *)sBtInterface->get_profile_interface(BT_PROFILE_VENDOR_ID);
        if (!btvendorInterface) {
            bdt_log("Error in loading vendor interface ");
            exit(0);
        }
        //status = (bt_status_t)sBtInterface->set_os_callouts(&callouts);
        status = sBtInterface->set_os_callouts(&bt_os_callbacks);
    }
    check_return_status(status);
}

void bdt_enable(void)
{
    bdt_log("ENABLE BT");
    if (bt_enabled) {
        bdt_log("Bluetooth is already enabled");
        return;
    }
    status = sBtInterface->enable();

    check_return_status(status);
}

void bdt_disable(void)
{
    bdt_log("DISABLE BT");
    if (!bt_enabled) {
        bdt_log("Bluetooth is already disabled");
        return;
    }
    status = sBtInterface->disable();

    check_return_status(status);
}

void do_pairing(char *p)
{
    RawAddress bd_addr = {{0}};
    int transport = GATT_TRANSPORT_LE;
    if(FALSE == GetBdAddr(p, &bd_addr))    return;    // arg1
    if(BT_STATUS_SUCCESS != sBtInterface->create_bond(&bd_addr, transport))
    {
        printf("Failed to Initiate Pairing \n");
        return;
    }
}

void bdt_dut_mode_configure(char *p)
{
    int32_t mode = -1;

    bdt_log("BT DUT MODE CONFIGURE");
    if (!bt_enabled) {
        bdt_log("Bluetooth must be enabled for test_mode to work.");
        return;
    }
    mode = get_signed_int(&p, mode);
    if ((mode != 0) && (mode != 1)) {
        bdt_log("Please specify mode: 1 to enter, 0 to exit");
        return;
    }
    status = sBtInterface->dut_mode_configure(mode);

    check_return_status(status);
}

#define HCI_LE_RECEIVER_TEST_OPCODE 0x201D
#define HCI_LE_TRANSMITTER_TEST_OPCODE 0x201E
#define HCI_LE_END_TEST_OPCODE 0x201F

void bdt_le_test_mode(char *p)
{
    int cmd;
    unsigned char buf[3];
    int arg1, arg2, arg3;

    bdt_log("BT LE TEST MODE");
    if (!bt_enabled) {
        bdt_log("Bluetooth must be enabled for le_test to work.");
        return;
    }

    memset(buf, 0, sizeof(buf));
    cmd = get_int(&p, 0);
    switch (cmd)
    {
        case 0x1: /* RX TEST */
           arg1 = get_int(&p, -1);
           if (arg1 < 0) bdt_log("%s Invalid arguments", __FUNCTION__);
           buf[0] = arg1;
           status = sBtInterface->le_test_mode(HCI_LE_RECEIVER_TEST_OPCODE, buf, 1);
           break;
        case 0x2: /* TX TEST */
            arg1 = get_int(&p, -1);
            arg2 = get_int(&p, -1);
            arg3 = get_int(&p, -1);
            if ((arg1 < 0) || (arg2 < 0) || (arg3 < 0))
                bdt_log("%s Invalid arguments", __FUNCTION__);
            buf[0] = arg1;
            buf[1] = arg2;
            buf[2] = arg3;
            status = sBtInterface->le_test_mode(HCI_LE_TRANSMITTER_TEST_OPCODE, buf, 3);
           break;
        case 0x3: /* END TEST */
            status = sBtInterface->le_test_mode(HCI_LE_END_TEST_OPCODE, buf, 0);
           break;
        default:
            bdt_log("Unsupported command");
            return;
            break;
    }
    if (status != BT_STATUS_SUCCESS)
    {
        bdt_log("%s Test 0x%x Failed with status:0x%x", __FUNCTION__, cmd, status);
    }
    return;
}

void bdt_cleanup(void)
{
    bdt_log("CLEANUP");
    sBtInterface->cleanup();
}

/*******************************************************************************
 ** Console commands
 *******************************************************************************/

void do_help(char *p)
{
    int i = 0;
    char line[128];
//    int pos = 0;

    while (console_cmd_list[i].name != NULL)
    {
        snprintf(line, 128,"%s", (char*)console_cmd_list[i].name);
        bdt_log("%s %s\n", (char*)line, (char*)console_cmd_list[i].help);
        i++;
    }
}

void do_quit(char *p)
{
    bdt_shutdown();
}

/*******************************************************************
 *
 *  BT TEST  CONSOLE COMMANDS
 *
 *  Parses argument lists and passes to API test function
 *
*/

void do_init(char *p)
{
    bdt_init();
}

void do_enable(char *p)
{
    bdt_enable();
}

void do_disable(char *p)
{
    bdt_disable();
}
void do_dut_mode_configure(char *p)
{
    bdt_dut_mode_configure(p);
}

void do_le_test_mode(char *p)
{
    bdt_le_test_mode(p);
}

void do_cleanup(char *p)
{
    bdt_cleanup();
}


void do_le_client_register(char *p)
{
    bt_status_t        Ret;
    int Idx;
    Uuid uuid;
    Uuid bt_uuid;
    bool is_valid = false;

    skip_blanks(&p);
    Idx = atoi(p);

    switch(Idx)
    {
    case 1:
        uuid = Uuid::FromString("0000A00C-0000-0000-01234-56789ABCDEF", &is_valid);//0000A00C-0000-0000-0123-456789ABCDEF
        bt_uuid = Uuid::FromString("0000A00C-0000-0000-0123-456789ABCDEF", &is_valid); //0000A00C-0000-0000-0123-456789ABCDEF
        break;
    case 2:
        uuid = Uuid::FromString("1122A00C-0000-0000-0123-456789ABCDEF", &is_valid);//1122A00C-0000-0000-0123-456789ABCDEF
        bt_uuid = Uuid::FromString("1122A00C-0000-0000-0123-456789ABCDEF", &is_valid);//1122A00C-0000-0000-0123-456789ABCDEF*/
        break;
    default:
        printf("%s:: ERROR: no matching uuid \n", __FUNCTION__);
        return;
    }
    if(Btif_gatt_layer)
    {
        Ret = sGattIfaceScan->client->register_client(bt_uuid);
        printf("%s:: ret value %d\n", __FUNCTION__,Ret);
    }
    else
    {
        g_client_if = sGattInterface->Register(uuid, &sGattCB);
        sleep(2);
        sGattInterface->StartIf(g_client_if);
    }
}

void do_le_client_deregister(char *p)
{
    bt_status_t        Ret;

    if(Btif_gatt_layer)
    {
        if(0 == g_client_if_scan)
        {
            printf("%s:: ERROR: no application registered\n", __FUNCTION__);
            return;
        }
        Ret = sGattIfaceScan->client->unregister_client(g_client_if_scan);
        printf("%s:: Ret=%d\n", __FUNCTION__, Ret);
    }
    else
    {
        if(0 == g_client_if)
        {
            printf("%s:: ERROR: no application registered\n", __FUNCTION__);
            return;
        }
        sGattInterface->Deregister(g_client_if);
    }
}

void do_le_client_connect (char *p)
{
    bool        Ret = false;
    RawAddress  bd_addr = {{0}};
    int transport = BT_TRANSPORT_BR_EDR;
    transport = get_int(&p, -1);
    if(FALSE == GetBdAddr(p, &bd_addr))    return;

    if(Btif_gatt_layer)
    {
        //TODO need to add phy parameter as 0x07 for connection to all types of Phys
        Ret = sGattIfaceScan->client->connect(g_client_if_scan, bd_addr, TRUE, transport, FALSE, 0x01);
    }
    else if(transport == BT_TRANSPORT_BR_EDR) {
        //Outgoing Connection

        //    g_SecLevel |= BTM_SEC_OUT_AUTHENTICATE;
        //     g_SecLevel |= BTM_SEC_OUT_ENCRYPT ;
        g_PSM= 1;
        g_SecLevel = 0;
        printf("g_SecLevel = %d \n", g_SecLevel);
        sL2capInterface->RegisterPsm(g_PSM, g_ConnType, g_SecLevel );
        sleep(3);

        l2c_connect(bd_addr);
    }
    else
    {
        Ret = sGattInterface->Connect(g_client_if, bd_addr.address, TRUE, transport);
    }
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}

void do_le_client_refresh (char *p)
{
    bool        Ret;
    RawAddress  bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return;

    if(Btif_gatt_layer)
    {
        Ret = sGattIfaceScan->client->refresh(g_client_if_scan, bd_addr);
        printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
    }
}

void do_le_conn_param_update(char *p)
{
    bool        Ret;
    RawAddress bd_addr = {{0}};
    int min_interval = 24;
    int max_interval = 40;
    int latency = 0;
    int timeout = 2000;

    min_interval =  get_hex(&p, -1);
    max_interval =  get_hex(&p, -1);
    latency      =  get_hex(&p, -1);
    timeout      =  get_hex(&p, -1);
    if(FALSE == GetBdAddr(p, &bd_addr))    return;

    Ret = sGattIfaceScan->client->conn_parameter_update(bd_addr, min_interval,
                max_interval, latency, timeout, 0, 0);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);

}

void do_le_client_connect_auto (char *p)
{
    bool        Ret;
    RawAddress bd_addr = {{0}};
    int transport = BT_TRANSPORT_BR_EDR;
    transport = get_int(&p, -1);
    if(FALSE == GetBdAddr(p, &bd_addr))    return;

    if(Btif_gatt_layer)
    {
        Ret = sGattIfaceScan->client->connect(g_client_if_scan, bd_addr, FALSE, transport, FALSE, 0x01);
    }
    else
    {
        Ret = sGattInterface->Connect(g_client_if, bd_addr.address, FALSE,BT_TRANSPORT_LE);
    }
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}


void do_le_client_disconnect (char *p)
{
    int        Ret = -1;
    bool return_status;
    RawAddress bd_addr = {{0}};
    int transport = BT_TRANSPORT_BR_EDR;
    transport = get_int(&p, -1);
    if(FALSE == GetBdAddr(p, &bd_addr))    return;

    if(Btif_gatt_layer)
    {
        Ret = sGattIfaceScan->client->disconnect(g_client_if_scan, bd_addr, g_conn_id);
    }
    else if(transport == BT_TRANSPORT_BR_EDR)
    {
        return_status = sL2capInterface->DisconnectReq(g_lcid);
    }
    else
    {
        Ret = sGattInterface->Disconnect(g_conn_id);
    }
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}

void do_le_client_scan_start (char *p)
{
    sGattIfaceScan->scanner->Scan(true);
}

void do_le_client_scan_stop (char *p)
{
    sGattIfaceScan->scanner->Scan(false);
}

/*void do_le_client_set_adv_data(char *p)
{
    bt_status_t        Ret;
    bool              SetScanRsp        = FALSE;
    bool              IncludeName        = TRUE;
    bool              IncludeTxPower    = FALSE;
    int               min_conn_interval = 100;
    int               max_conn_interval = 1000;

    SetScanRsp         = get_int(&p, -1);  // arg1  Other than zero will be considered as true.
    IncludeName     = get_int(&p, -1);  // arg2  Other than zero will be considered as true.
    IncludeTxPower     = get_int(&p, -1);  // arg3  Other than zero will be considered as true.
    min_conn_interval     = get_int(&p, -1);  // arg3  Other than zero will be considered as true.
    max_conn_interval     = get_int(&p, -1);  // arg3  Other than zero will be considered as true.

    //To start with we are going with hard-code values.
    Ret = sGattIfaceScan->client->set_adv_data(g_server_if_scan , SetScanRsp, IncludeName, IncludeTxPower, min_conn_interval, max_conn_interval, 0,8, "QUALCOMM", 0, NULL,0,NULL);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}
void do_le_client_set_adv_mode(char *p)
{
    tBTA_DM_DISC disc_mode;
    tBTA_DM_CONN conn_mode;

    disc_mode = get_int(&p,-1);
    conn_mode = get_int(&p,-1);
    printf("%s:: discoverable  mode=%d  connectable _mode=%d \n", __FUNCTION__,disc_mode,conn_mode );
    sGattInterface->cSetVisibility(disc_mode,conn_mode);

}

void do_le_client_multi_adv_set_inst_data(char *p)
{
    bt_status_t        Ret;
    bool              SetScanRsp        = FALSE;
    bool              IncludeName        = TRUE;
    bool              IncludeTxPower    = TRUE;

    SetScanRsp         = get_int(&p, -1);  // arg1  Other than zero will be considered as true.
    IncludeName     = get_int(&p, -1);  // arg2  Other than zero will be considered as true.
    IncludeTxPower     = get_int(&p, -1);  // arg3  Other than zero will be considered as true.

    //To start with we are going with hard-code values.
    Ret = sGattIfaceScan->client->multi_adv_set_inst_data(g_client_if_scan , SetScanRsp, IncludeName, IncludeTxPower,0,8, "QUALCOMM", 0, NULL,0,NULL);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}

void do_le_client_adv_update(char *p)
{
    bt_status_t        Ret;
    int               TxPower    = 3;
    int               chnlMap    = 7;
    int               min_interval = 160;
    int               max_interval = 240;
    int               adv_type    = 3 ;//non-connectable undirect
    int               adv_if   =  g_server_if_scan;
    int               timeout_s =   30;

    adv_if       =  get_int(&p, -1);
    min_interval =  get_int(&p, -1);
    max_interval =  get_int(&p, -1);
    adv_type     =  get_int(&p, -1);
    chnlMap      =  get_int(&p, -1);
    TxPower      =  get_int(&p, -1);
    timeout_s    =  get_int(&p, -1);
    //To start with we are going with hard-code values.
    Ret = sGattIfaceScan->client->multi_adv_update(adv_if, min_interval, max_interval,adv_type,chnlMap,TxPower, timeout_s);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}

void do_le_client_adv_enable(char *p)
{
    bt_status_t       Ret;
    int               TxPower    = 4;
    int               chnlMap    = 7;
    int               min_interval = 30;
    int               max_interval = 60;
    int               adv_type    = 0; //connectable undirect
    int               adv_if   =  g_server_if_scan;
    int               timeout_s = 30;

    adv_if       =  get_int(&p, -1);
    min_interval =  get_int(&p, -1);
    max_interval =  get_int(&p, -1);
    adv_type     =  get_int(&p, -1);
    chnlMap      =  get_int(&p, -1);
    TxPower      =  get_int(&p, -1);
    timeout_s    =  get_int(&p, -1);
    Ret = sGattIfaceScan->client->multi_adv_enable(adv_if,min_interval,max_interval,adv_type,chnlMap,TxPower, timeout_s);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}

void do_le_client_adv_disable(char *p)
{
    bt_status_t   Ret;
    int           adv_if = g_server_if_scan;

    adv_if = get_int(&p, -1);
    Ret = sGattIfaceScan->client->multi_adv_disable(adv_if);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}*/

void DoNothing2(uint8_t, uint8_t) {}

void StartAdvertisingSetCb(uint8_t advertiser_id, int8_t tx_power,
                                uint8_t status) {
        start_advertising_set_advertiser_id = advertiser_id;
        start_advertising_set_tx_power = tx_power;
        start_advertising_set_status = status;
}

void do_start_adv_set(char *p)
{
    std::vector<uint8_t> scan_resp;
    AdvertiseParameters params;
    PeriodicAdvertisingParameters periodic_params;
    periodic_params.enable = false;
    std::vector<uint8_t> periodic_data;
    int interval;

    //Adv params
    params.advertising_event_properties = get_hex(&p, -1);
    interval = get_int(&p, -1);

    switch(interval)
    {
        case INTERVAL_LOW:
            params.min_interval = 160;
            params.max_interval = 210;
            break;
        case INTERVAL_MEDIUM:
            params.min_interval = 400;
            params.max_interval = 450;
            break;
        case INTERVAL_HIGH:
            params.min_interval = 1600;
            params.max_interval = 1650;
            break;
    }

    params.primary_advertising_phy = get_int(&p, -1);
    params.secondary_advertising_phy = get_int(&p, -1);
    params.channel_map = 0x07;
    params.tx_power = -7;

    params.scan_request_notification_enable = false;

    //adv data
    uint8_t arr[] ={10,9,'G','A','T','T','-','T','O','O','L'};
    std::vector<uint8_t> adv_data(arr, arr + (sizeof(arr)/sizeof(arr[0])));

    sGattIfaceScan->advertiser->StartAdvertisingSet(base::Bind(&StartAdvertisingSetCb),
            params, adv_data, scan_resp, periodic_params, periodic_data,
            0 , 0 ,  base::Bind(DoNothing2));
}
void do_unregister_adv_set(char *p)
{
    sGattIfaceScan->advertiser->Unregister(start_advertising_set_advertiser_id);
}

void do_le_client_configureMTU(char *p)
{
    tGATT_STATUS Ret =0;
    uint16_t mtu = 23;

    mtu  = get_int(&p, -1);
    printf("%s:: mtu :%d\n", __FUNCTION__, mtu);
    Ret = sGattInterface->cConfigureMTU(g_conn_id, mtu);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}

void do_le_client_discover(char *p)
{
    int        uuid_len = 0, uuid_len_bytes = 0;
    tGATT_STATUS Ret =0;
    tGATT_DISC_TYPE disc_type; //GATT_DISC_SRVC_ALL , GATT_DISC_SRVC_BY_UUID
    bool is_valid = false;

    disc_type = get_int(&p, -1);  // arg1
    uint16_t s_handle = get_hex(&p, -1);  // arg2
    uint16_t e_handle = get_hex(&p, -1);  // arg3

    uuid_len    = get_int(&p, -1);  // arg4 - Size in bits for the uuid (16, 32, or 128)
    if((16==uuid_len) || (32==uuid_len) || (128==uuid_len))
    {
        uuid_len_bytes = uuid_len/8;
    }
    else
    {
        printf("%s::ERROR - Invalid Parameter. UUID Len should be either 16/32/128 \n",__FUNCTION__);
        return;
    }

    std::string uuid_str = get_uuid_str(&p, uuid_len_bytes);
    Uuid service = Uuid::FromString(uuid_str, &is_valid);

    printf("%s:: disc_type = %d is_valid = %d\n", __FUNCTION__, disc_type, is_valid);

    //if(FALSE == GetDiscType(p, &disc_type))    return;        //TODO - add the function if user input is needed
    Ret = sGattInterface->cDiscover(g_conn_id, disc_type, s_handle, e_handle, service);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}


void do_le_client_read(char *p)
{
    int i =0;
    int  uuid_len = 0, uuid_len_bytes=0;
    tGATT_STATUS Ret = 0;
    tGATT_READ_TYPE read_type;
    int auth_req;
    tGATT_READ_PARAM readBuf;// = {GATT_AUTH_REQ_NONE, 0x201};
    bool is_valid = false;
    std::string uuid_str;

    //Parse and copy command line arguments
    read_type = get_int(&p, -1); // arg2
    auth_req = get_int(&p, -1); // arg2

    switch(read_type)
    {
    case GATT_READ_BY_TYPE:
    case GATT_READ_CHAR_VALUE:

        readBuf.service.auth_req     = auth_req;
        readBuf.service.s_handle     = get_hex(&p, -1);  // arg2
        readBuf.service.e_handle     = get_hex(&p, -1);  // arg3

        uuid_len    = get_int(&p, -1);  // arg4 - Size in bits for the uuid (16, 32, or 128)
        if((16==uuid_len) || (32==uuid_len) || (128==uuid_len))
        {
            uuid_len_bytes = uuid_len/8;
        }
        else
        {
            printf("%s::ERROR - Invalid Parameter. UUID Len should be either 16/32/128 \n",__FUNCTION__);
            return;
        }

        uuid_str = get_uuid_str(&p, uuid_len_bytes);
        readBuf.service.uuid = Uuid::FromString(uuid_str, &is_valid);
        printf("%s:: read_type = %d is_valid = %d\n", __FUNCTION__, read_type, is_valid);
        break;


    case GATT_READ_BY_HANDLE:
        readBuf.by_handle.handle = get_hex(&p, -1);
        readBuf.by_handle.auth_req = auth_req;
        break;

    case GATT_READ_MULTIPLE:
        readBuf.read_multiple.auth_req = auth_req;
        readBuf.read_multiple.num_handles = get_hex(&p, -1); //arg 2
        if(readBuf.read_multiple.num_handles > 10)
        {
            printf(":: ERROR - invalid param. Max handle value is 10. \n");
            return;
        }
        for(i=0; i<readBuf.read_multiple.num_handles; i++)
        {
            readBuf.read_multiple.handles[i] = get_hex(&p, -1); //arg 3 ... N
        }
        printf("%s:: Read by MultipleHandle \t Number of handles=%04x \n", __FUNCTION__, readBuf.read_multiple.num_handles);
        break;

    case GATT_READ_PARTIAL:
        readBuf.partial.auth_req = auth_req;
        readBuf.partial.handle = get_hex(&p, -1); //arg 2
        readBuf.partial.offset = get_hex(&p, -1); //arg 3
        printf("%s:: Read by Descriptor \t handle=%04x \t offset=%04x \n", __FUNCTION__, readBuf.partial.handle, readBuf.partial.offset);
        break;

    }

    Ret = sGattInterface->cRead(g_conn_id, read_type, &readBuf);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}

void copy_string(char *dest, char *source)
{
   int i = 2;
   while(i)
   {
      *dest = *source;
      source++;
      dest++;
      i--;
   }
   *dest = '\0';
}

void do_le_client_write(char *p)
{
    int i;
    tGATT_STATUS Ret = 0;
    tGATT_WRITE_TYPE write_type;
    int auth_req = 0;
    tGATT_VALUE writeBuf;// = {GATT_AUTH_REQ_NONE, 0x201};

    write_type = get_int(&p, -1); // arg1
    auth_req = get_int(&p, -1); // arg2

    writeBuf.conn_id = g_conn_id;
    writeBuf.auth_req = auth_req;
    writeBuf.handle     = get_hex(&p, -1);  // arg3
    writeBuf.offset     = get_hex(&p, -1);  //arg4
    writeBuf.len         = get_int(&p, -1); //arg5


    if(writeBuf.len > GATT_MAX_ATTR_LEN )
    {
        printf("%s:: ERROR - invalid param. Max length for Write is 600 \n",__FUNCTION__);
        return;
    }
    memset(&(writeBuf.value[0]), 0, GATT_MAX_ATTR_LEN);
    for (i = 0; i < writeBuf.len; i++)
    {
        writeBuf.value[i] = get_hex_byte(&p, 0);
    }

    Ret = sGattInterface->cWrite(g_conn_id, write_type, &writeBuf);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}
void do_le_execute_write(char *p)
{
    bool is_execute;
    tGATT_STATUS Ret = 0;

    is_execute = get_int(&p, -1); // arg1

    printf("%s:: is_execute=%d \n", __FUNCTION__, is_execute);
    Ret = sGattInterface->cExecuteWrite(g_conn_id, is_execute);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}
void do_le_set_idle_timeout(char *p)
{
    int idle_timeout;
    RawAddress bd_addr = {{0}};
        if(FALSE == GetBdAddr(p, &bd_addr))    return;
    idle_timeout = get_int(&p, -1); //arg2
    sGattInterface->cSetIdleTimeout(bd_addr.address, idle_timeout);

}


/*******************************************************************************
 ** GATT SERVER API commands
 *******************************************************************************/
void do_le_server_register(char *p)
{
    bt_status_t        Ret;
    bool is_valid = false;
    int Idx;
    Uuid uuid;
    Uuid bt_uuid;
    skip_blanks(&p);
    Idx = atoi(p);
    switch(Idx)
    {
    case 1:
        uuid = Uuid::FromString("0000A00C-0000-0000-0123-456789ABCDEF", &is_valid);//0000A00C-0000-0000-0123-456789ABCDEF
        bt_uuid = Uuid::FromString("0000A00C-0000-0000-0123-456789ABCDEF", &is_valid); //0000A00C-0000-0000-0123-456789ABCDEF
        break;
    case 2:
        uuid = Uuid::FromString("1122A00C-0000-0000-0123-456789ABCDEF", &is_valid);//1122A00C-0000-0000-0123-456789ABCDEF
        bt_uuid = Uuid::FromString("1122A00C-0000-0000-0123-456789ABCDEF", &is_valid);//1122A00C-0000-0000-0123-456789ABCDEF*/
        break;
    default:
        printf("%s:: ERROR: no matching uuid \n", __FUNCTION__);
        return;
    }

    if(Btif_gatt_layer)
    {
        Ret = sGattIfaceScan->server->register_server(bt_uuid);
        printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
    }
    else
    {
        g_server_if = sGattInterface->Register(uuid, &sGattCB);
        printf("%s:: g_server_if=%d \n", __FUNCTION__, g_server_if);
    }
}

void do_le_server_deregister(char *p)
{
    bt_status_t        Ret;
    if(0 == g_server_if)
    {
        printf("%s:: ERROR: no application registered\n", __FUNCTION__);
        return;
    }
    sGattInterface->Deregister(g_server_if);
    Ret = sGattIfaceScan->server->unregister_server(g_server_if_scan);
    printf("%s::Ret = %d\n", __FUNCTION__,Ret);
}

void do_le_server_add_service(char *p)
{
    int  Ret = 0;
    bool is_valid = false;

    std::vector<btgatt_db_element_t> service;
    //1st service
    btgatt_db_element_t svc1 = {0};
    svc1.uuid = Uuid::FromString("00001800-0000-1000-8000-00805f9b34fb", &is_valid);//00001800-0000-1000-8000-00805f9b34fb
    svc1.type = BTGATT_DB_PRIMARY_SERVICE;
    service.push_back(svc1);

    //2nd service
    btgatt_db_element_t svc2 = {0};
    svc2.uuid = Uuid::FromString("00001801-0000-1000-8000-00805f9b34fb", &is_valid);//00001801-0000-1000-8000-00805f9b34fb
    svc2.type = BTGATT_DB_PRIMARY_SERVICE;
    service.push_back(svc2);

    Ret = sGattIfaceScan->server->add_service(g_server_if_scan, service);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}

void do_le_server_connect (char *p)
{
    bool        Ret;
    RawAddress bd_addr = {{0}};
    int transport = BT_TRANSPORT_BR_EDR;
    transport = get_int(&p, -1);
    if(FALSE == GetBdAddr(p, &bd_addr))    return;
    Ret = sGattIfaceScan->server->connect(g_server_if_scan, bd_addr, TRUE, transport);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}

void do_le_server_connect_auto (char *p)
{
    bool        Ret;
    RawAddress bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return;
    Ret = sGattIfaceScan->server->connect(g_server_if_scan, bd_addr, FALSE, BT_TRANSPORT_LE);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}


void do_le_server_disconnect (char *p)
{
    bt_status_t        Ret;
    RawAddress bd_addr = {{0}};
    int transport = BT_TRANSPORT_BR_EDR;
    transport = get_int(&p, -1);
    if(FALSE == GetBdAddr(p, &bd_addr))    return;
    Ret = sGattIfaceScan->server->disconnect(g_server_if_scan, bd_addr, g_conn_id);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}

void do_le_server_send_indication (char *p)
{
    bt_status_t        Ret;
    int attr_handle;
    int confirm = 0;
    uint8_t arr[] = {1,2,3,4};
    std::vector<uint8_t> value (arr, arr + sizeof(arr) / sizeof(arr[0]));

    attr_handle = get_hex(&p, -1);
    confirm = get_int(&p, -1);
    Ret = sGattIfaceScan->server->send_indication(g_server_if_scan, attr_handle,
                                                  g_conn_id, confirm, value);
    printf("%s:: Ret=%d \n", __FUNCTION__,Ret );
}

/**************************************************
**L2CAP for BR/EDR conn
***************************************************/



void do_l2cap_init(char *p)
{

    memset(&tl2cap_cfg_info, 0, sizeof(tl2cap_cfg_info));
    memset(&le_conn_info, 0, sizeof(le_conn_info));
    //Use macros for the constants
    tl2cap_cfg_info.mtu_present = TRUE;
    tl2cap_cfg_info.mtu = g_imtu;
    tl2cap_cfg_info.flush_to_present = TRUE;
    tl2cap_cfg_info.flush_to = 0xffff;
    //use other param if needed
    tl2cap_cfg_info.fcr_present = g_Fcr_Present;
    tl2cap_cfg_info.fcr.mode = g_Fcr_Mode;
    tl2cap_cfg_info.fcs = 0;
    tl2cap_cfg_info.fcs_present = 1;
    le_conn_info.init_credits = L2CAP_LE_MAX_CREDIT;
    le_conn_info.le_mtu = L2CAP_LE_DEFAULT_MTU;
    le_conn_info.le_mps = L2CAP_LE_DEFAULT_MPS;
    le_conn_info.le_psm = 0x80; //dynamic le psm starts from 0x80
    if(L2CAP_FCR_ERTM_MODE == tl2cap_cfg_info.fcr.mode)
    {
        tl2cap_cfg_info.fcr = ertm_fcr_opts_def;
    }
    else if(L2CAP_FCR_STREAM_MODE == tl2cap_cfg_info.fcr.mode)
    {
        tl2cap_cfg_info.fcr = stream_fcr_opts_def;
    }
    else if(NULL == sL2capInterface)
    {
        printf("Get L2cap testapp interfaces\n");
        sL2capInterface = (btl2cap_interface_t *)btvendorInterface->get_testapp_interface(TEST_APP_L2CAP);
    }
    tl2cap_cfg_info.fcr.tx_win_sz = 3;
    //Initialize ERTM Parameters
    t_ertm_info.preferred_mode = g_Fcr_Mode;
    t_ertm_info.allowed_modes = g_Ertm_AllowedMode;
    t_ertm_info.user_rx_buf_size = BT_DEFAULT_BUFFER_SIZE;
    t_ertm_info.user_tx_buf_size = BT_DEFAULT_BUFFER_SIZE;
    t_ertm_info.fcr_rx_buf_size = BT_DEFAULT_BUFFER_SIZE;
    t_ertm_info.fcr_tx_buf_size = BT_DEFAULT_BUFFER_SIZE;
    //Load L2cap Interface
    sL2capInterface->Init(&l2test_l2c_appl);
}


/*
static int l2c_pair(char *p)
{
    bt_bdaddr_t bd_addr = {{0}};
    int transport = BT_TRANSPORT_BR_EDR;
    transport = get_int(&p, -1);
    GetBdAddr(p, &bd_addr);
    if(BT_STATUS_SUCCESS != sBtInterface->create_bond(&bd_addr,transport))
    {
        printf("Failed to Initiate Pairing \n");
        return FALSE;
    }
     sleep(20);
     return TRUE;
}
*/

static uint16_t do_l2cap_connect(RawAddress bd_addr)
{

    if((L2CAP_FCR_STREAM_MODE == g_Fcr_Mode) || (L2CAP_FCR_ERTM_MODE == g_Fcr_Mode)) {
        return sL2capInterface->ErtmConnectReq(g_PSM, bd_addr, &t_ertm_info);
    } else {
        return sL2capInterface->Connect(g_PSM, &bd_addr);
    }
}

static void l2c_connect(RawAddress bd_addr)
{
    do_l2cap_connect(bd_addr);
}


bool do_l2cap_disconnect(char *p)
{
    return sL2capInterface->DisconnectReq(g_lcid);
}

/* LE-L2CAP functionalities */
static t_le_chnl_info *le_allocate_conn_info(uint16_t psm, bool is_server)
{
    t_le_chnl_info    *p_le_chnl_info = &le_chnl_conn_info[0];
    uint16_t      i;

    for (i = 0; i < MAX_L2CAP_CLIENTS; i++, p_le_chnl_info++)
    {
        if (!p_le_chnl_info->in_use)
        {
            p_le_chnl_info->in_use = TRUE;
            p_le_chnl_info->is_server = is_server;
            p_le_chnl_info->psm    = psm;
            return p_le_chnl_info;
        }
    }
    return (NULL);

}

static t_le_chnl_info *le_get_conn_info(uint16_t psm, bool is_server)
{
    t_le_chnl_info    *p_le_chnl_info = &le_chnl_conn_info[0];
    uint16_t      i;

    for (i = 0; i < MAX_L2CAP_CLIENTS; i++, p_le_chnl_info++)
    {
        if ((p_le_chnl_info->in_use) && (p_le_chnl_info->psm == psm)
                && (is_server == p_le_chnl_info->is_server))
        {
            return p_le_chnl_info;
        }
    }
    return NULL;

}

static t_le_chnl_info *le_get_conn_info_by_lcid(uint16_t lcid)
{
    t_le_chnl_info    *p_le_chnl_info = &le_chnl_conn_info[0];
    uint16_t      i;

    for (i = 0; i < MAX_L2CAP_CLIENTS; i++, p_le_chnl_info++)
    {
        if ((p_le_chnl_info->in_use) && (p_le_chnl_info->lcid == lcid))
        {
            return p_le_chnl_info;
        }
    }
    return NULL;

}

static bool le_release_conn_info(t_le_chnl_info *le_conn_info)
{
    if (le_conn_info && (le_conn_info->in_use))
    {
        le_conn_info->in_use = FALSE;
        le_conn_info->is_server = 0;
        le_conn_info->psm    = 0;
        memset(le_conn_info, 0, sizeof(tL2CAP_LE_CONN_INFO));
        return TRUE;
    }
    return FALSE;
}

static void do_start_advertisment(char *p)
{
	Uuid uuid;
    int option = get_int(&p, -1);
    int start = get_int(&p, -1);
    RawAddress bd_addr = {{0}};
	bool is_valid = false;

	// 128 bit UUID: 1122A00D-0000-0000-0123-456789ABCDEF
    uuid = Uuid::FromString("1122A00D-0000-0000-0123-456789ABCDEF", &is_valid);//1122A00D-0000-0000-0123-456789ABCDEF

    if ((g_le_coc_if == 0) && option)
        g_le_coc_if = sGattInterface->Register(uuid, &sGattCB);
    printf("Gatt Registration Done\n");

    if( option)
    {
        if (start == 1)
            status = sGattInterface->Listen(g_le_coc_if, start, bd_addr);
        else if (start == 0)
            status = sGattInterface->Listen(g_le_coc_if, start, bd_addr);
        else
        {
            printf("Unknown parameter\n");
            return;
        }
    }
    else
    {
        if (start == 1)
            status = sGattInterface->Listen(g_server_if_scan, start, bd_addr);
        else if (start == 0)
            status = sGattInterface->Listen(g_server_if_scan, start, bd_addr);
        else
        {
            printf("Unknown parameter\n");
            return;
        }
    }
    printf("Gatt Listen status is %d\n", status);
}

uint16_t do_le_l2cap_coc_flow_ctrl(char *p)
{
    uint16_t lcid = get_int(&p, -1);

    uint16_t credits = get_int(&p, -1);

    printf("\ndo_le_l2cap_coc_flow_ctrl lcid = %d, credits = %d\n", lcid, credits);
    return sL2capInterface->LeFlowControl(lcid, credits);
}
static void le_l2cap_coc_flow_ctrl(char *p)
{
    printf("In le_l2cap_coc_flow_ctrl\n");
    do_le_l2cap_coc_flow_ctrl(p);
}

uint16_t do_le_l2cap_coc_connect(char *p)
{
    int le_initiator_sec_level;
    uint16_t le_coc_seclevel = 0;
    RawAddress bd_addr = {{0}};
    uint16_t le_psm = get_int(&p, -1);


    t_le_chnl_info *le_conn_info = le_allocate_conn_info(le_psm, FALSE);

    if(le_conn_info)
    {
        le_conn_info->loc_conn_info.le_psm = le_psm;
        le_conn_info->loc_conn_info.le_mtu = get_int(&p, -1);
        le_conn_info->loc_conn_info.le_mps = get_int(&p, -1);
        le_conn_info->loc_conn_info.init_credits = get_int(&p, -1);
        le_initiator_sec_level = get_int(&p, -1);
        if(FALSE == GetBdAddr(p, &bd_addr))    return FALSE;
    }
    else
        return FALSE;

    if (le_initiator_sec_level == 0)
    {
        le_coc_seclevel |= BTM_SEC_NONE;
    }
    else if (le_initiator_sec_level == 1)
    {
        le_coc_seclevel |= BTM_SEC_OUT_AUTHENTICATE;
    }
    else if (le_initiator_sec_level == 2)
    {
        le_coc_seclevel |= BTM_SEC_OUT_ENCRYPT;
        le_coc_seclevel |= BTM_SEC_OUT_AUTHENTICATE;
    }
    else
    {
        printf("Security level not supported");
        return FALSE;
    }

    printf("g_SecLevel = %d \n", le_coc_seclevel);
    sL2capInterface->RegisterLePsm(le_conn_info->loc_conn_info.le_psm, TRUE,
            le_coc_seclevel, g_BleEncKeySize);
    sleep(3);
    local_coc_cfg.credits = le_conn_info->loc_conn_info.init_credits;
    local_coc_cfg.mtu = le_conn_info->loc_conn_info.le_mtu;
    local_coc_cfg.mps = le_conn_info->loc_conn_info.le_mps;
    printf("\ndo_l2cap_connect:::::::: psm %d mtu %d mps %d init_credit %d \n",
            le_conn_info->loc_conn_info.le_psm, le_conn_info->loc_conn_info.le_mtu,
            le_conn_info->loc_conn_info.le_mps, le_conn_info->loc_conn_info.init_credits);

    return sL2capInterface->LeConnect(le_conn_info->loc_conn_info.le_psm, bd_addr, &local_coc_cfg);
}

static void le_l2cap_coc_connect(char *svr)
{
    do_le_l2cap_coc_connect(svr);
}

static void le_l2cap_listen(char *p)
{
    int le_rspndr_sec_level;
    uint16_t le_coc_seclevel = 0;
    uint16_t le_psm = get_int(&p, -1);
    t_le_chnl_info *le_conn_info = le_get_conn_info(le_psm, TRUE);

    if(!le_conn_info)
    {
        le_conn_info = le_allocate_conn_info(le_psm, TRUE);
    }
    else
    {
        printf("ALready listening on same channel");
        return;
    }

    if(le_conn_info)
    {
        le_conn_info->loc_conn_info.le_psm = le_psm;
        le_conn_info->loc_conn_info.le_mtu = get_int(&p, -1);
        le_conn_info->loc_conn_info.le_mps = get_int(&p, -1);
        le_conn_info->loc_conn_info.init_credits = get_int(&p, -1);
        le_rspndr_sec_level = get_int(&p, -1);
    }
    else
        return;

    if (le_rspndr_sec_level == 0)
    {
        le_coc_seclevel |= BTM_SEC_NONE;
    }
    else if (le_rspndr_sec_level == 1)
    {
        le_coc_seclevel |= BTM_SEC_IN_AUTHENTICATE;
    }
    else if (le_rspndr_sec_level == 2)
    {
        le_coc_seclevel |= BTM_SEC_IN_ENCRYPT;
        le_coc_seclevel |= BTM_SEC_IN_AUTHENTICATE;
    }
    else
    {
        printf("Security level not supported");
        return ;
    }
    printf("g_SecLevel = %d \n", le_coc_seclevel);

    sL2capInterface->RegisterLePsm(le_conn_info->loc_conn_info.le_psm, FALSE,
                                           le_coc_seclevel, g_BleEncKeySize);

    printf("Waiting for Incoming connection for LE PSM %d... \n",
                              le_conn_info->loc_conn_info.le_psm);
}

uint8_t do_l2cap_DataWrite(uint16_t chnl_id, char *p , uint32_t len)
{
    return sL2capInterface->DataWrite(chnl_id, p, len);
}

static int send_file(char *p)
{
    uint32_t seq = 0, itration = 1;
    int fd, size;
    char filename[] = {0};
    char tmpBuf[LE_ACL_MAX_BUFF_SIZE];
    uint16_t lcid;

    lcid = get_int(&p, -1);
    t_le_chnl_info *le_conn_info = le_get_conn_info_by_lcid(lcid);
    GetFileName(p, filename);

    if(!le_conn_info)
    {
        printf("No conn info, exit \n");
        return FALSE;
    }

    g_omtu =  le_conn_info->rmt_conn_info.le_mtu;

    if(g_omtu < LE_ACL_MAX_BUFF_SIZE)
        data_size = g_omtu;
    else
        data_size = LE_ACL_MAX_BUFF_SIZE;

    printf("data_size(max patload size) = %ld, g_omtu(max ttansmission unit) = %d",
            data_size, g_omtu);

    printf("Filename for input data = %s \n", filename);

    if ((fd = open(filename, O_RDONLY)) < 0)
    {
        printf("Open failed: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }
    while (1)
    {
        while(cong_status)
        {
            usleep(50 * 1000);
        }
        if((size = read(fd, tmpBuf, data_size)) <= 0)
        {
            printf("\n File end ");
            break;
        }
        printf("Sending data :: itration %d, omtu %d, writing data size %d\n",
                itration, g_omtu, size);
        do_l2cap_DataWrite(lcid, tmpBuf, size);
        itration++;
    }

    if (num_frames && g_delay && count && !(seq % count))
        usleep(g_delay);
    return TRUE;
}

static int Send_Data(char *p)
{
    //uint32_t seq =0;
    int send_mode;
    uint16_t lcid;

    lcid = get_int(&p, -1);
    send_mode = get_int(&p, -1);

    char tmpBuffer_1[] = {
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F
    };
    char tmpBuffer_2[] = {
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F
    };

    if(send_mode == 1)  // segmented
    {
        printf("Sending Segmented data...\nData written len %d...\n",
                sizeof(tmpBuffer_1) );
        do_l2cap_DataWrite(lcid, tmpBuffer_1, sizeof(tmpBuffer_1));
    }
    else if( send_mode == 0) // unsegmented
    {
        printf("Sending Unsegmented data...\nData written len %d...\n",
                sizeof(tmpBuffer_2) );
        do_l2cap_DataWrite(lcid, tmpBuffer_2, sizeof(tmpBuffer_2));
    }
    return TRUE;
}

static void send_data_on_le_coc(char *svr)
{
    printf("Sending data on LE L2CAP CoC...\n");
    Send_Data(svr);
}
static void do_send_file(char *svr)
{
    printf("Sending file on LE L2CAP CoC...\n");
    send_file(svr);
}

bool le_coc_disc(char *p)
{
    uint16_t cid = get_int(&p, -1);
    return sL2capInterface->DisconnectReq(cid);
}

static void do_le_coc_disconnect(char *p)
{
    le_coc_disc(p);
}

/*******************************************************************************
 ** SMP API commands
 *******************************************************************************/
void do_smp_init(char *p)
{
    sSmpIface->init();
    sleep(1);
    sSmpIface->Register(SMP_cb);
    sleep(1);
}

void do_smp_pair(char *p)
{
    tSMP_STATUS Ret = 0;
    RawAddress bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return;
    Ret = sSmpIface->Pair(bd_addr.address);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}

void do_smp_pair_cancel(char *p)
{
    bool Ret = 0;
    RawAddress bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return;
    Ret = sSmpIface->PairCancel(bd_addr.address);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);
}

void do_smp_security_grant(char *p)
{
    uint8_t    res;
    RawAddress bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return; //arg1
    res = get_int(&p, -1); // arg2
    sSmpIface->SecurityGrant(bd_addr.address, res);
    printf("%s:: Ret=%d \n", __FUNCTION__,res);
}

void do_smp_passkey_reply(char *p)
{
    uint32_t passkey;
    uint8_t    res;
    RawAddress bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return; //arg1
        printf("get res value\n");
    res = get_int(&p, -1); // arg2
        printf("res value=%d\n", res);
    passkey = get_int(&p, -1); // arg3
        printf("passkey value=%d\n", passkey);
    sSmpIface->PasskeyReply(bd_addr.address, res, passkey);
    printf("%s:: Ret=%d \n", __FUNCTION__,res);
}

void do_smp_encrypt(char *p)
{
    bool Ret = 0;
    uint8_t    res;
    RawAddress bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return; //arg1
    res = get_int(&p, -1); // arg2
    printf("%s:: res =%d Ret=%d \n", __FUNCTION__,res, Ret);
}

void do_le_gap_conn_param_update(char *p)
{
    //attr_uuid = get_int(&p, -1);
    tGAP_BLE_ATTR_VALUE attr_value;
    attr_value.conn_param.int_min = 50;
    attr_value.conn_param.int_max = 70;
    attr_value.conn_param.latency = 0;
    attr_value.conn_param.sp_tout = 10;
    RawAddress bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return; //arg1
    //attr_uuid = get_hex(&p, -1);
    //L2CA_UpdateBleConnParams(bd_addr.address, 50, 70, 0, 1000);
    printf("stage 1\n");
    sGapInterface->Gap_BleAttrDBUpdate(bd_addr.address, attr_value.conn_param.int_min, attr_value.conn_param.int_max,attr_value.conn_param.latency ,attr_value.conn_param.sp_tout);
    printf("%s:: GAP connection parameter Update\n", __FUNCTION__);

}

void do_le_set_char_len(char *p)
{
    int len;
    len = get_int(&p, -1); //arg1
    if (len > LE_CHAR_MAX_LEN_VAL || len < 1) {
      printf("Invalid length. Enter 1 to 512\n");
    } else {
      len_long_char = len;
      printf("long characteristic length is %d\n", len_long_char);
    }
}

void do_le_gap_attr_init(char *p)
{
    sGapInterface->Gap_AttrInit();
    printf("%s:: GAP Initialization\n", __FUNCTION__);

}
/*
void do_le_gap_set_disc(char *p)
{
    uint16_t Ret = 0;
    uint16_t mode;
    uint16_t duration;
    uint16_t interval;

    mode = get_int(&p, -1);
    if(1 == mode)         mode = GAP_NON_DISCOVERABLE;
    else if(2 == mode)  mode = GAP_LIMITED_DISCOVERABLE;
    else                mode = GAP_GENERAL_DISCOVERABLE;

    duration = get_int(&p, -1);
    if((12 > duration) || (duration > 1000))    duration = 0; //if 0 is passed, stack will take 12 as default

    interval = get_int(&p, -1);
    if((12 > interval) || (interval > 1000))    interval = 0; //if 0 is passed, stack will take 800 as default



    sGapInterface->Gap_SetDiscoverableMode(mode, duration, interval);
    printf("%s:: Ret=%d\n", __FUNCTION__, Ret);
}

void do_le_gap_set_conn(char *p)
{
    uint16_t Ret=0;
    uint16_t mode;
    uint16_t duration;
    uint16_t interval;

    mode = get_int(&p, -1);
    if(1 == mode)
           mode = GAP_NON_CONNECTABLE;
    else
           mode = GAP_CONNECTABLE;

    duration = get_int(&p, -1);
        if((12 > duration) || (duration > 1000))    duration = 0; //if 0 is passed, stack will take 12 as default

    interval = get_int(&p, -1);
    if((12 > interval) || (interval > 1000))    interval = 0; //if 0 is passed, stack will take 800 as default

    sGapInterface->Gap_SetConnectableMode(mode, duration, interval);
    printf("%s:: Ret=%d\n", __FUNCTION__, Ret);
}
*/
void do_l2cap_send_data_cid(char *p)
{
    uint16_t        cid        = 0;
    BT_HDR        bt_hdr;
    uint16_t         Ret = 0;
    RawAddress bd_addr = {{0}};
    if(FALSE == GetBdAddr(p, &bd_addr))    return; //arg1
    cid = get_int(&p, -1); // arg2

    bt_hdr.event     = 0;
    bt_hdr.len         = 1;
    bt_hdr.offset     = 0;
    bt_hdr.layer_specific = 0;


    Ret = sL2capInterface->SendFixedChnlData(cid, bd_addr.address, &bt_hdr);
    printf("%s:: Ret=%d \n", __FUNCTION__, Ret);

}

/*
 * Main console command handler
*/

static void process_cmd(char *p, unsigned char is_job)
{
    char cmd[2048];
    int i = 0;
    bt_pin_code_t pincode;
    char *p_saved = p;

    get_str(&p, cmd);

    /* table commands */
    while (console_cmd_list[i].name != NULL)
    {
        if (is_cmd(console_cmd_list[i].name))
        {
            if (!is_job && console_cmd_list[i].is_job)
                create_cmdjob(p_saved);
            else
            {
                console_cmd_list[i].handler(p);
            }
            return;
        }
        i++;
    }
    //pin key
    if(cmd[6] == '.') {
        for(i=0; i<6; i++) {
            pincode.pin[i] = cmd[i];
        }
        if(BT_STATUS_SUCCESS != sBtInterface->pin_reply(remote_bd_address, TRUE, strlen((const char*)pincode.pin), &pincode)) {
            printf("Pin Reply failed\n");
        }
        //flush the char for pinkey
        cmd[6] = 0;
    }
    else {
        bdt_log("%s : unknown command\n", p_saved);
        do_help(NULL);
    }
}

int main (int argc, char * argv[])
{
    static btgatt_callbacks_t    sGatt_cb = {sizeof(btgatt_callbacks_t), &sGattClient_cb, &sGattServer_cb, &sScanner_cb};

    config_permissions();
    bdt_log("\n:::::::::::::::::::::::::::::::::::::::::::::::::::");
    bdt_log(":: Bluedroid test app starting");

    if ( HAL_load() < 0 ) {
        perror("HAL failed to initialize, exit\n");
        unlink(PID_FILE);
        exit(0);
    }

    setup_test_env();

    /* Automatically perform the init */
    bdt_init();
    sleep(5);
    bdt_enable();
    sleep(5);
    bdt_log("Get SMP IF BT Interface = %x \n", sBtInterface);
    sGattInterface   = (btgatt_test_interface_t *)btvendorInterface->get_testapp_interface(TEST_APP_GATT);
    sSmpIface        = (btsmp_interface_t *)btvendorInterface->get_testapp_interface(TEST_APP_SMP);
    bdt_log("Get GAP IF");
    sGapInterface    = (btgap_interface_t *)btvendorInterface->get_testapp_interface(TEST_APP_GAP);

    bdt_log("Get GATT IF");
    sGattIfaceScan     = (btgatt_interface_t *)sBtInterface->get_profile_interface(BT_PROFILE_GATT_ID);

    bdt_log("Get L2CAP IF");
    sL2capInterface        = (btl2cap_interface_t *)btvendorInterface->get_testapp_interface(TEST_APP_L2CAP);

    sGattIfaceScan->init(&sGatt_cb);
    bdt_log("GATT IF INIT Done");

    printf("\n Before l2cap init\n");
    do_l2cap_init(NULL);
    printf("\n after l2cap init\n");

    while(!main_done)
    {
        char line[2048];

        /* command prompt */
        printf( ">" );
        fflush(stdout);

        fgets (line, 2048, stdin);

        if (line[0]!= '\0')
        {
            /* remove linefeed */
            line[strlen(line)-1] = 0;

            process_cmd(line, 0);
            memset(line, '\0', 2048);
        }
    }

    /* FIXME: Commenting this out as for some reason, the application does not exit otherwise*/
    //bdt_cleanup();

    HAL_unload();

    bdt_log(":: Bluedroid test app terminating");

    return 0;
}
int GetFileName(char *p, char *filename)
{
//    uint8_t  i;
    int len;

    skip_blanks(&p);

    printf("Input file name = %s\n", p);

    if (p == NULL)
    {
        printf("\nInvalid File Name... Please enter file name\n");
        return FALSE;
    }
    len = strlen(p);

    memcpy(filename, p, len);
    filename[len] = '\0';

    return TRUE;
}
int GetBdAddr(char *p, RawAddress* pbd_addr)
{
    char Arr[13] = {0};
    char *pszAddr = NULL;
    uint8_t k1 = 0;
    uint8_t k2 = 0;
    uint8_t  i;

    skip_blanks(&p);

    printf("Input=%s\n", p);

    if(12 > strlen(p))
    {
        printf("\nInvalid Bd Address. Format[112233445566]\n");
        return FALSE;
    }
    memcpy(Arr, p, 12);

    for(i=0; i<12; i++)
    {
        Arr[i] = tolower(Arr[i]);
    }
    pszAddr = Arr;

    for(i=0; i<6; i++)
    {
        k1 = (uint8_t) ( (*pszAddr >= 'a') ? ( 10 + (uint8_t)( *pszAddr - 'a' )) : (*pszAddr - '0') );
        pszAddr++;
        k2 = (uint8_t) ( (*pszAddr >= 'a') ? ( 10 + (uint8_t)( *pszAddr - 'a' )) : (*pszAddr - '0') );
        pszAddr++;

        if ( (k1>15)||(k2>15) )
        {
            return FALSE;
        }
        pbd_addr->address[i] = (k1<<4 | k2);
    }
    return TRUE;
}
#endif //TEST_APP_INTERFACE
