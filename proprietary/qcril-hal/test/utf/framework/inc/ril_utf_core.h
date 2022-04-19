/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_core.h
  @brief   RIL UTF core declarations

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_CORE_H
#define _RIL_UTF_CORE_H

#include "ril_utf_common_includes.h"
#include "ril_utf_defs.h"
#include "ril_utf_log.h"
#include "control_service_v01.h"
#include "imsIF.pb.h"

#ifdef RIL_UTF_IOE
    #include "voice_service_v02.h"
    #include "qmi_csi.h"
#else
    #include "qmi_ims_vt_v01.h"
    #include <pb_decode.h>
    #include <pb_encode.h>
#endif

/*===================================================================================
QCRIL UNITTEST specific definitions
===================================================================================*/
#ifdef QMI_RIL_HAL_UTF
#ifdef __cplusplus
extern "C" {
#endif
#endif
#define EXEC_Q_LOCK()    { pthread_mutex_lock(&exec_q_access); }
#define EXEC_Q_UNLOCK()  { pthread_mutex_unlock(&exec_q_access); }
#define MAIN_THRD_DISPATCH_Q_LOCK()  { pthread_mutex_lock(&main_thread_q_access); }
#define MAIN_THRD_DISPATCH_Q_UNLOCK()  { pthread_mutex_unlock(&main_thread_q_access); }
#define MAIN_THRD_RIL_REQUEST_Q_LOCK()  { pthread_mutex_lock(&main_thread_ril_q_access); }
#define MAIN_THRD_RIL_REQUEST_Q_UNLOCK()  { pthread_mutex_unlock(&main_thread_ril_q_access); }

#ifdef RIL_UTF_IOE
    #define IOE_TABLE_LOCK() { pthread_mutex_lock(&ioe_table_access); }
    #define IOE_TABLE_UNLOCK() { pthread_mutex_unlock(&ioe_table_access); }
    #define UTF_IOE_SERVER_SOCKET_ID -1
#endif
#define QCRIL_UNITTEST_MAX_QMI_CLIENTS 100
#define QCRIL_UNITTEST_MAX_QMI_CLIENT_ID 20

#define QCRIL_UNITTEST_LOG_TEST_START() do{\
               RIL_UTF_ESSENTIAL("\n======================================");\
               RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL:start test %s", __func__);\
               RIL_UTF_ESSENTIAL("\n======================================");\
               } while(0);


#define QCRIL_UNITTEST_LOG_TEST_END()   do{\
               RIL_UTF_ESSENTIAL("\n======================================");\
               RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL:end test %s", __func__);\
               RIL_UTF_ESSENTIAL("\n======================================");\
               } while(0);

#define WAIT_INDEFINITELY()  do{\
               } while(1)

#define RIL_UTF_IMS_SOCKET_MAX_BUF_SIZE (1024*8)
#define RIL_UTF_PROXY_SOCKET_MAX_BUF_SIZE (1024*8)
#define RIL_UTF_OEM_SOCKET_MAX_BUF_SIZE (1024*8)
#define RIL_UTF_IOE_CLIENT_SOCKET_MAX_BUF_SIZE (1024*8)
#define RIL_UTF_RILD_SOCKET_MAX_BUF_SIZE (1024*8)
#define QCRIL_UNITTTEST_NO_ERR 0
#define QCRIL_UNITTEST_GEN_ERR -1
#define QCRIL_UNITTEST_TIMOUT_ERR -2
#define QCRIL_UNITTEST_TARGET_UNEXPECTED_MESSAGE 1
#define MAX_SIGNALS 200
#define MAX_EXPECTS 2000
#define UTF_DEFAULT_TIMEOUT 300000
#define EXPECT_TIMEOUT 30000
#define INJECT_TIMEOUT 30000

#define MICROSECONDS 1000000
#define MAX_LIB_ARGS 16

#define QMI_RIL_UTL

#define RIL_SHLIB

#define ERANGE 34

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define EXEC_MAIN_THREAD 1
#define EXEC_QMI_THREAD 1
#define EXEC_DSI_THREAD 1
#define EXEC_IMS_THREAD 1
#define EXEC_MSIM_THREAD 1
#define EXEC_PROXY_THREAD 1
#define EXEC_OEM_THREAD 1
#define EXEC_IOE_CLIENT_THREAD 1
#define EXEC_RILD_THREAD 1
#define QCRIL_UNITTEST_DEFAULT_QMI_CLIENT 0xFF
#define QCRIL_UNITTTEST_INVALID_EXPECTATION 1
#define QCRIL_UNITTTEST_VALID_EXPECTATION   2
#define QCRIL_TEST_PROPERTY_KEY_LEN 100
#define QCRIL_TEST_PROPERTY_LEN 100
#define QCRIL_TEST_PROPERTY_VALUE_LEN 100
#define QCRIL_TEST_MAX_PROPERTIES 200

#define QMI_NO_ERROR 0
#define QMI_TIMEOUT_ERROR -3

#define HEX_MSG_BUFFER_LEN 100
#define HEX_MSG_NUMBER_PER_LINE 9

#define QMI_HEADER_SIZE_DEFAULT 13
#define QMI_HEADER_SIZE_CTL     12

#define UTF_MAX_QMI_SEND_MSG_BUF 8192

#define UTF_QMI_PDU_CONTROL_POINT  1
#define UTF_QMI_PDU_SERVICE_POINT  0
#define UTF_QMI_SDU_CTL_REQUEST    0
#define UTF_QMI_SDU_CTL_RESPONSE   2
#define UTF_QMI_SDU_CTL_INDICATION 4
#define UTF_QMI_QMUX_IF_LEN        1

#define UTF_IOE_CLIENT_CONNECT_CB_ID 0x00
#define UTF_IOE_CLIENT_DISCONNECT_CB_ID 0x01
#define UTF_IOE_CLIENT_REQ_CB_ID 0x02
#define UTF_IOE_CLIENT_RESP_CB_ID 0x03
#define UTF_IOE_CLIENT_IND_CB_ID 0x04
#define UTF_IOE_MAX_CLIENTS 10
#define UTF_IOE_REQ_CB_BUFFER_MIN_FIXED_LEN 10
#define UTF_IOE_RESP_IND_BUFFER_MIN_FIXED_LEN 11
#define UTF_IOE_CONNECT_CB_BUFFER_FIXED_LEN 2
#define UTF_IOE_DEFAULT_CLIENT_HANDLE 0


#ifdef RIL_UTF_IOE
#define MAX_IOE_SERVERS 5
#define MAX_IOE_REQUESTS 10
#endif

#ifndef QCRIL_DB_MAX_OPERATOR_TYPE_LEN
#define QCRIL_DB_MAX_OPERATOR_TYPE_LEN (6)
#endif

//===============================================================
// QCRIL UNITTEST specific structs
//===============================================================
#ifndef RIL_UTF_IOE
struct timed_cb_t {
  RIL_TimedCallback callback;
  void *param;
  struct timespec   timeout;
  struct timed_cb_t *next;
  struct timed_cb_t *prev;
};
#endif

struct qcril_test_property_type
{
   char key[QCRIL_TEST_PROPERTY_KEY_LEN];
   char prop_val[QCRIL_TEST_PROPERTY_VALUE_LEN];
};

struct qmi_client_init_t {
  const char                                *conn_id;
  qmi_idl_service_object_type               service_obj;
  qmi_client_ind_cb                         ind_cb;
  void                                      *ind_cb_data;
  qmi_client_type                           user_handle;
  int                                       is_used;
};

#ifdef RIL_UTF_IOE
struct ril_utf_csi_data_t {
   qmi_idl_service_object_type               service_obj;
   qmi_csi_connect                           service_connect;
   qmi_csi_disconnect                        service_disconnect;
   qmi_csi_process_req                       service_process_req;
   void                                      *service_cookie;
   qmi_csi_os_params                         *os_params;
   qmi_csi_service_handle                    *service_provider;
   void                                      *connection_handle;
   int                                       client_sd;
   int                                       *client_array;
   int                                       client_array_size;
   int                                       is_used;
};

struct ril_utf_req_handle_table_t{
    int req_handle;
    int service_handle;
    int client_handle;
    int is_used;
};
#endif

enum ril_utf_q_element_t {
  ril_utf_ril_request,
  ril_utf_ril_response,
  ril_utf_ril_unsol_response,
  ril_utf_qmi_request,
  ril_utf_qmi_response,
  ril_utf_qmi_indication,
  ril_utf_ims_request,
  ril_utf_ims_response,
  ril_utf_ims_indication,
  ril_utf_oem_request,
  ril_utf_oem_response,
  ril_utf_oem_indication,
  ril_utf_ioe_request,
  ril_utf_ioe_response,
  ril_utf_ioe_indication,
  ril_utf_msim_send_msg,
  ril_utf_msim_recv_msg,
  ril_utf_db_request,
  ril_utf_db_response,
  ril_utf_ssr_error_cb,
  ril_utf_ssr_notify_cb,
  ril_utf_property_msg,
  ril_utf_dsi_indication,
  ril_utf_dsi_response,
  ril_utf_dsi_request,
};

typedef struct {
  int number;
  const char* name;
} ril_utf_q_element_t_type;

static const ril_utf_q_element_t_type ril_utf_q_element_t_name[] = {
    {ril_utf_ril_request, "RIL_REQUEST"},
    {ril_utf_ril_response, "RIL_RESPONSE"},
    {ril_utf_ril_unsol_response, "RIL_UNSOL_RESPONSE"},
    {ril_utf_qmi_request, "QMI_REQUEST"},
    {ril_utf_qmi_response, "QMI_RESPONSE"},
    {ril_utf_qmi_indication, "QMI_INDICATION"},
    {ril_utf_ims_request, "IMS_REQUEST"},
    {ril_utf_ims_response, "IMS_RESPONSE"},
    {ril_utf_ims_indication, "IMS_INDICATION"},
    {ril_utf_oem_request, "OEM_REQUEST"},
    {ril_utf_oem_response, "OEM_RESPONSE"},
    {ril_utf_oem_indication, "OEM_INDICATION"},
    {ril_utf_ioe_request, "IOE_REQUEST"},
    {ril_utf_ioe_response, "IOE_RESPONSE"},
    {ril_utf_ioe_indication, "IOE_INDICATION"},
    {ril_utf_msim_send_msg, "MSIM_IPC_SEND"},
    {ril_utf_msim_recv_msg, "MSIM_IPC_RECV"},
};

enum ril_utf_expectation_t {
  not_met,
  met
};

enum qmi_send_msg_t_e {
  sync_req,
  async_req
};

// Note: match these values with
// actual QMI srv id's
enum ril_utf_qmi_srv_id_t {
  QMI_CTL    = 0x00,
  QMI_WDS    = 0x01,
  QMI_DMS    = 0x02,
  QMI_NAS    = 0x03,
  QMI_WMS    = 0x05,
  QMI_AUTH   = 0x07,
  QMI_VOICE  = 0x09,
  QMI_CAT    = 0x0A,
  QMI_UIM    = 0x0B,
  QMI_PBM    = 0x0C,
  QMI_IMSS   = 0x12,
  QMI_RF_SAR = 0x17,
  QMI_CSVT   = 0x1d,
  QMI_IMSP   = 0x1f,
  QMI_IMSA   = 0x21,
  QMI_PDC    = 0x24,
  QMI_RFRPE  = 0x29,
  QMI_DSD    = 0x2A,
  QMI_LTE    = 0x46,
  QMI_UIM_HTTP = 0x47
};

enum ril_utf_node_t {
  INJECT = 0x01,
  EXPECT = 0x02,
  TIMEOUT_NODE = 0x03,
  CANARY_NODE = 0x04,
  CALLFLOW_SUMMARY = 0x05,
  FUNC_EXEC_NODE = 0x06,
  BLOCK_CLIENT_INIT = 0x07,
};

struct ril_utf_msg_data_type_t {
  int is_valid;
  int msg_id; //common for ril/qmi messages
  enum ril_utf_qmi_srv_id_t qmi_srv_id; //valid only for qmi msgs
  int is_sync_msg;// is qmi sync resp?
  int client_id;
};

struct ril_utf_expecatation_t {
  int is_valid;
  int msg_id;
  int verify_payload;
  enum ril_utf_expectation_t status;
  struct timeval timeout; //timeout for timed waits
};

/*
  Used to pass data from stubs to
  the test thread
*/
#ifndef RIL_UTF_IOE // For Android & QMI Proxy
struct ril_utf_msg_t {
  void *payload;
  int   payload_len;
  enum ril_utf_q_element_t type;//could be ril request/resp, qmi req/resp, qmi ind, etc...
  int msg_id;
  // RIL ATEL vars
  int token_id;
  RIL_Errno error_val;
  ims_Error ims_error;
  int oem_command_id;
  int rild_instance_id;
  // QMI service vars
  int service_id;
  int client_id;

  struct ril_utf_msg_t *next;
  struct ril_utf_msg_t *prev;
};
#else //For IoE
struct ril_utf_msg_t {
  void *payload;
  int   payload_len;
  enum ril_utf_q_element_t type;//could be ril request/resp, qmi req/resp, qmi ind
  int msg_id;
  // RIL ATEL vars
  int token_id;
  // QMI service vars
  int service_id;
  int client_id;

  struct ril_utf_msg_t *next;
  struct ril_utf_msg_t *prev;
};
#endif

/*
  For each node in the list only one of
  e.is_valid or i.is_valid will be true.
*/
struct ril_utf_q_t {
  void *payload;
  int   payload_len;
  enum ril_utf_q_element_t type;//could be ril request/resp, qmi req/resp, qmi ind
  int token_id;
  int node_id; // used to match unordered expects with injects
  int service_id;  // used only for qmi

  struct ril_utf_expecatation_t   e; // could be qmi_req or ril_resp
  struct ril_utf_msg_data_type_t  i; // could be ril_req, qmi_resp or qmi_ind

  struct ril_utf_q_t *next;
  struct ril_utf_q_t *prev;
};

struct ril_request_q_t {
  struct ril_utf_msg_t *head;
  struct ril_utf_msg_t *tail;
};

struct sig_expect_pool_t {
  int in_use;
  int msg_id;
  int service_id;
  int client_id;
  pthread_cond_t signal_exptectation;
};

struct expect_timing_t {
  int in_use;
  int expect_ready;
  int expect_received;
  int is_sync;
  int msg_id;
  int service_id;
  int client_id;
};

typedef struct ril_utf_q_t* elnptr;

struct ril_utf_qmi_resp_pending_q_t {
  qmi_client_type                 user_handle;
  unsigned long                   msg_id;
  void                            *resp_c_struct;
  int                             resp_c_struct_len;
  qmi_client_recv_msg_async_cb    resp_cb;
  void                            *resp_cb_data;
  enum qmi_send_msg_t_e            qmi_send_msg_type;

  struct ril_utf_qmi_resp_pending_q_t  *next;
  struct ril_utf_qmi_resp_pending_q_t  *prev;
};

enum ril_utf_db_action_t {
  UTF_DB_NONE,
  UTF_DB_IS_MCC_PART_OF_EMERGENCY_NUMBER_TABLE,
  UTF_DB_IS_MCC_PART_OF_EMERGENCY_NUMBER_TABLE_WITH_SERVICE_STATE,
  UTF_DB_IS_NUMBER_MCC_PART_OF_EMERGENCY_NUMBER_TABLE,
  UTF_DB_IS_NUMBER_MCC_PART_OF_EMERGENCY_TABLE_WITH_SERVICE_STATE,
  UTF_DB_IS_IMS_ADDRESS_FOR_MCC_EMERGENCY_NUMBER_PART_OF_EMERGENCY_NUMBER_TABLE,
  UTF_DB_QUERY_ESCV_TYPE,
  UTF_DB_QUERY_PROPERTIES_TABLE,
  UTF_DB_INSERT_PROPERTIES_TABLE,
  UTF_DB_QUERY_OPERATOR_TYPE,
  UTF_DB_INSERT_OPERATOR_TYPE,
  UTF_DB_ADD,
  UTF_DB_REMOVE,
  UTF_DB_QUERY
};

typedef struct {
  char *mcc;
  char *mnc;
  char *emergency_num;
  char *service_state;
  char *service;
  char *ims_address;
  char *iin;
  char *roam;
  char *property_name;
  char *value;
  qmi_ril_custom_emergency_numbers_source_type source;
  char operator_type[QCRIL_DB_MAX_OPERATOR_TYPE_LEN];
  enum ril_utf_db_action_t action;
  int expected_ret;
} db_node_t;

typedef struct {
  char key[QCRIL_TEST_PROPERTY_KEY_LEN];
  char value[QCRIL_TEST_PROPERTY_VALUE_LEN];
} property_node_t;

// defined in ril_utf.c
extern qmi_sys_event_client_data_type qmi_client_info;
extern struct qmi_client_init_t ril_utf_qmi_client_list[QCRIL_UNITTEST_MAX_QMI_CLIENT_ID][QCRIL_UNITTEST_MAX_QMI_CLIENTS];
extern pthread_t qmit_id;
#ifndef QMI_RIL_HAL_UTF
extern void *qmi_thrd_start(void *param);
#endif
extern pthread_mutex_t exec_q_access;

#ifdef RIL_UTF_IOE
extern pthread_mutex_t ioe_table_access;
#endif
extern struct expect_timing_t expect_table[MAX_EXPECTS];
extern int inject_complete;
#ifndef QMI_RIL_HAL_UTF
extern struct ril_utf_q_t *ril_utf_rec_exp_head;
extern struct ril_utf_q_t *ril_utf_rec_exp_tail;
extern struct ril_utf_q_t *ril_utf_exp_head;
extern struct ril_utf_q_t *ril_utf_exp_tail;
#endif
extern int req_id;
extern pthread_cond_t signal_exptectation;
extern pthread_cond_t inject_complete_sig;
extern pthread_cond_t expect_ready_sig;
extern struct ril_utf_q_t *qmi_resp_head_ptr;
extern struct ril_utf_q_t *qmi_resp_tail_ptr;
extern pthread_cond_t wakeup_qmi_sync_req;
extern struct sig_expect_pool_t wakeup_qmi_sync_signals[MAX_SIGNALS];
#ifndef QMI_RIL_HAL_UTF
extern void  add_to_pending_qmi_resp_q
(
    qmi_client_type                 user_handle,
    unsigned long                   msg_id,
    void                            *resp_c_struct,
    int                             resp_c_struct_len,
    qmi_client_recv_msg_async_cb    resp_cb,
    void                            *resp_cb_data,
    enum qmi_send_msg_t_e                qmi_send_msg_type
);
#endif

#ifdef RIL_UTF_IOE
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif

void print_msg_in_hex( const void *response, size_t responselen );

void print_msg_in_tlv(int msg_id, enum ril_utf_qmi_srv_id_t qmi_srv_id, int client_id, enum ril_utf_q_element_t payload_type,
 const void *message, size_t message_len);

void print_db_msg(void* payload, int payload_len);

void print_header(void);

void ril_utf_qmi_init();

void ril_utf_init_mutexes();

void ril_utf_init_sig_pool();

void add_to_execute_list(struct ril_utf_q_t *node);

void calc_timeout(struct timespec *wait_timeout, struct timeval *timeout);

void print_expectation( struct ril_utf_msg_t *test_step);

int send_ril_utf_ril_request(struct ril_utf_msg_t *test_step);

int send_ril_utf_qmi_response(struct ril_utf_msg_t *test_step);

int send_ril_utf_qmi_indication(struct ril_utf_msg_t *test_step);

#ifndef QMI_RIL_HAL_UTF
int send_ril_utf_oem_request( struct ril_utf_msg_t *test_step );
#endif

int send_ril_utf_rild_socket_request( struct ril_utf_msg_t *test_step );

int send_ril_utf_proxy_qmi_request( struct ril_utf_msg_t *test_step );

#ifdef RIL_UTF_IOE
int send_ril_utf_ioe_request( struct ril_utf_msg_t *test_step );
void ril_utf_connect_to_ioe_server(void);
#endif

int send_ril_utf_db_request (struct ril_utf_msg_t *test_step);

int send_ril_utf_db_response (void* payload, int payload_len);

int send_ril_utf_property_request (struct ril_utf_msg_t *test_step);

int send_ril_utf_property_response (void* payload, int payload_len);

int pack_qmi_msg_header(uint8_t *send_buf, struct ril_utf_msg_t *test_step, int tlv_stream_len);

void clear_exec_list();

elnptr get_exec_list_node();

void remove_ril_utf_q_node( struct ril_utf_msg_t* node,
                            struct ril_utf_msg_t** head,
                            struct ril_utf_msg_t** tail);

void enqueue_ril_utf_q_node( struct ril_utf_msg_t* node,
                             struct ril_utf_msg_t** head,
                             struct ril_utf_msg_t** tail);
struct ril_utf_msg_t* pop_front_ril_utf_q_node
  ( struct ril_utf_msg_t** head,
    struct ril_utf_msg_t** tail );

int clear_inject_expect_queues();

void ril_utf_init();

void ioe_utf_init();

void qmi_proxy_utf_init();

#ifndef QMI_RIL_HAL_UTF
void start_rild_thread();

void start_main_thread();
#endif
void start_qmi_proxy();

void utf_start_ims_threads();
void utf_start_msim_threads();

#ifdef RIL_UTF_IOE
int start_ioe_server();
void utf_start_ioe_server(void);
#endif

void utf_start_oem_threads();

void utf_start_rild_threads();

void utf_start_proxy_threads();

int utf_property_init();

void print_all_properties();

int utf_join_threads();
#ifdef QMI_RIL_HAL_UTF
#ifdef __cplusplus
}
#endif
#endif

uint32_t qcril_get_time_milliseconds();

int send_ril_utf_dsi_indication(struct ril_utf_msg_t *test_step);

int send_ril_utf_dsi_response(struct ril_utf_msg_t *test_step);
#endif //_RIL_UTF_CORE_H
