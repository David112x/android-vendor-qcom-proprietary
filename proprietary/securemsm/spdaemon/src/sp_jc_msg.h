/*=============================================================================
Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/**
 * @file sp_jc_msg.h
 * @brief - APIs to be called by sp UIM REMOTE client
 *          to send JavaCard formatted messages to the iUICC on the SP.
 *
 */

#ifndef _SP_JC_MSG_H_
#define _SP_JC_MSG_H_

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ----------------------------------------------------------------------*/
#define JC_SPCOM_MAX_ATR_BYTES        32
#define JC_SPCOM_MAX_REQ_APDU_PAYLOAD 255
#define JC_SPCOM_MAX_RESP_APDU_LENGTH 256

/*-------------------------------------------------------------------------
 * Structures and enums
 * ----------------------------------------------------------------------*/
typedef enum {
    JC_SPCOM_INTERNAL_ERR,
    JC_SPCOM_TOT_CMD_SIZE_ERR,
    JC_SPCOM_CMD_BUF_SIZE_ERR,
    JC_SPCOM_RSP_BUF_SIZE_ERR,
    JC_SPCOM_CMD_TYPE_ERR,
    JC_SPCOM_CMD_LEN_ERR,
    JC_SPCOM_APDU_LC_ERR,
    JC_SPCOM_APDU_LE_ERR,
    JC_SPCOM_NULL_POINTER_ERR,
    JC_SPCOM_CMD_FAILED_ERR,
    JC_SPCOM_TX_ERR,
    JC_SPCOM_RX_ERR,
    JC_SPCOM_SUCCESS
} jc_spcom_result_type;

typedef enum {
    JC_APDU_CMD,
    JC_APDU_RSP,
    JC_POWER_UP_CMD,
    JC_POWER_UP_RSP,
    JC_POWER_DOWN_CMD,
    JC_POWER_DOWN_RSP,
    JC_RST_CMD,
    JC_RST_RSP,
    JC_LOAD_NVM_CMD,
    JC_LOAD_NVM_RSP,
    JC_ERR
} jc_spcom_cmd_rsp_type;

typedef struct {
    char CLA;
    char INS;
    char P1;
    char P2;
    char LC;
    char LE;
    char lc_bytes[JC_SPCOM_MAX_REQ_APDU_PAYLOAD];
} apdu_cmd_type;

typedef struct {
    char SW1;
    char SW2;
    char rsp_bytes[JC_SPCOM_MAX_RESP_APDU_LENGTH];
} apdu_rsp_type;

typedef struct {
    char atr[32];
} reset_rsp_type;

typedef struct {
    char error_bytes[JC_SPCOM_MAX_RESP_APDU_LENGTH];
} error_type;

typedef union {
    apdu_cmd_type      apdu_cmd;
    apdu_rsp_type      apdu_rsp;
    reset_rsp_type     reset_rsp;
    error_type         error;
} jc_spcom_cmd_body;

/* SPCOM has a max payload of 268 bytes.
   The iUICC spcom cmd structure is shown below.
   Max size of this struct is
   4 bytes for jcio_spcom_cmd_type(enum type)
   2 bytes for length (short type)
   262 bytes(256+6) max size of the union for jcio_spcom_cmd_body.
   */
typedef struct __attribute__((packed)) {
    uint32_t               header;
    uint16_t               length;
    jc_spcom_cmd_body      body;
} jc_spcom_msg_type;

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/
/**
 * sp_jc_send_pup_cmd_and_wait_on_resp
 *
 * This function sends JavaCard formatted power up command to iUICC using
 * SPCOM interface
 *
 * @param spcom_client_handle_ptr  Pointer to the spcom client
 */
jc_spcom_result_type sp_jc_send_pup_cmd_and_wait_on_resp(void* spcom_client_handle_ptr);

/**
 * sp_jc_send_apdu_cmd_and_wait_on_resp
 *
 * This function sends Java Card formatted APDU command to iUICC
 * using SPCOM interface
 *
 * @param spcom_client_handle_ptr Pointer to the spcom client.
          cmd_buf_ptr             APDU cmd buffer
          cmd_buf_size            APDU cmd size
          cmd_len                 APDU cmd len
          rsp_buf_ptr             APDU Response buffer
          rsp_buf_size            APDU Response size
          rsp_len_ptr             APDU Response len ptr
 */
jc_spcom_result_type sp_jc_send_apdu_cmd_and_wait_on_resp(void* spcom_client_handle_ptr,
                                                          char* cmd_buf_ptr,
                                                          uint16_t cmd_buf_size,
                                                          uint16_t cmd_len,
                                                          char* rsp_buf_ptr,
                                                          uint16_t rsp_buf_size,
                                                          uint16_t* rsp_len_ptr,
                                                          char* SW1_ptr,
                                                          char* SW2_ptr);

/**
 * sp_jc_send_rst_cmd_and_wait_on_resp
 *
 * This function sends Java Card formatted reset command to
 * iUICC using SPCOM interface
 *
 * @param spcom_client_handle_ptr Pointer to the spcom client.
          rsp_buf_ptr             ATR Response buffer
          rsp_buf_size            ATR Response size
 *        rsp_len_ptr             ATR Response len ptr
 */
jc_spcom_result_type sp_jc_send_rst_cmd_and_wait_on_resp(void* spcom_client_handle_ptr,
                                                         char* rsp_buf_ptr,
                                                         uint16_t rsp_buf_size,
                                                         uint16_t* rsp_len_ptr);

/**
 * sp_jc_send_pdown_cmd_and_wait_on_resp
 *
 * This function sends JavaCard formatted power down command to
 * iUICC using SPCOM interface
 *
 * @param spcom_client_handle_ptr Pointer to the spcom client.
 *        timeout_msec            Timeout value for the sync spcom API
 */
jc_spcom_result_type sp_jc_send_pdown_cmd_and_wait_on_resp(void* spcom_client_handle_ptr);
#endif /* _SP_JC_MSG_H_ */
