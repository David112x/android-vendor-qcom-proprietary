/*=============================================================================
Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/**
 * @file sp_jc_msg.c
 * @brief - Implementation of APIs to be called by sp
 *          UIM REMOTE client to send JavaCard formatted
 *          messages to the iUICC on the SP.
 *
 */

/*-------------------------------------------------------------------------
 * Include Files
 * ----------------------------------------------------------------------*/
#include "string.h"
#include <errno.h>
#include <spcomlib.h>
#include <sp_jc_msg.h>
#include <log/log.h>

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ----------------------------------------------------------------------*/
#define SPCOM_SYNC_CALL_TIMEOUT_MS 4000

#ifdef PRINT_LOG_TO_STDOUT
    #define LOGD(fmt, x...) printf("sp_jc_msg: dbg: %s: " fmt "\n", __func__,##x)
    #define LOGI(fmt, x...) printf("sp_jc_msg: dbg: %s: " fmt "\n", __func__,##x)
    #define LOGE(fmt, x...) printf("sp_jc_msg: err: %s: " fmt "\n", __func__,##x)
#else // print to system log a.k.a logcat
    #undef LOG_TAG
    #undef LOGD
    #undef LOGI
    #undef LOGE
    #define LOG_TAG "sp_jc_msg"
    #define LOGD(fmt, x...) SLOGD("[%s] " fmt, __func__, ##x)
    #define LOGI(fmt, x...) SLOGI("[%s] " fmt, __func__, ##x)
    #define LOGE(fmt, x...) SLOGE("[%s] " fmt, __func__, ##x)
#endif

#define JC_SPCOM_REQ_APDU_HDR_SIZE   5
#define JC_SPCOM_REQ_APDU_CLA_INDEX  0
#define JC_SPCOM_REQ_APDU_INS_INDEX  1
#define JC_SPCOM_REQ_APDU_P1_INDEX   2
#define JC_SPCOM_REQ_APDU_P2_INDEX   3
#define JC_SPCOM_REQ_APDU_P3_INDEX   4
#define JC_SPCOM_REQ_APDU_DATA_INDEX 5

/*-------------------------------------------------------------------------
 * Function Implementations
 * ----------------------------------------------------------------------*/
/**
 * memscpy
 * Utility memscpy function
 *
 * @param dst      destination pointer
 *        dst_size destination size
 *        src      source pointer
 *        src_size source size
 *
 */
static inline size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
    size_t min_size = (dst_size < src_size) ? dst_size : src_size;
    memcpy(dst, src, min_size);
    return min_size;
}

/**
 * sp_jc_send_pup_cmd_and_wait_on_resp
 * This function sends JavaCard formatted power up command to iUICC using
 * SPCOM interface
 *
 * @param spcom_client_handle_ptr  Pointer to the spcom client
 *
 */
jc_spcom_result_type sp_jc_send_pup_cmd_and_wait_on_resp(void* spcom_client_handle_ptr)
{
    int                num_of_bytes_sent = 0;
    jc_spcom_msg_type* cmd_ptr           = NULL;
    jc_spcom_msg_type* rsp_ptr           = NULL;
    char               jc_spcom_req_buf[SPCOM_MAX_REQUEST_SIZE] = {0};
    char               jc_spcom_rsp_buf[SPCOM_MAX_RESPONSE_SIZE] = {0};

    if (sizeof(jc_spcom_msg_type) > SPCOM_MAX_RESPONSE_SIZE) {
        return JC_SPCOM_TOT_CMD_SIZE_ERR;
    }

    if (spcom_client_handle_ptr == NULL) {
        return JC_SPCOM_NULL_POINTER_ERR;
    }

    cmd_ptr = (jc_spcom_msg_type*) jc_spcom_req_buf;
    rsp_ptr = (jc_spcom_msg_type*) jc_spcom_rsp_buf;

    cmd_ptr->header = JC_POWER_UP_CMD;
    cmd_ptr->length = 0;

    num_of_bytes_sent =  spcom_client_send_message_sync((struct spcom_client*)spcom_client_handle_ptr,
                                                        (char *)cmd_ptr, sizeof(jc_spcom_msg_type),
                                                        (char *)rsp_ptr, sizeof(jc_spcom_msg_type),
                                                        (uint16_t)SPCOM_SYNC_CALL_TIMEOUT_MS);

    if(num_of_bytes_sent == -ETIMEDOUT) {
        LOGE("SPCOM timed out!");
        return JC_SPCOM_TX_ERR;
    }

    if (num_of_bytes_sent != sizeof(jc_spcom_msg_type)) {
        LOGE("SPCOM tx error %d   %d", num_of_bytes_sent, (int) sizeof(jc_spcom_msg_type));
        return JC_SPCOM_TX_ERR;
    }

    /* Extract rsp and verify the same */
    if (rsp_ptr->header != JC_POWER_UP_RSP) {
        return JC_SPCOM_CMD_FAILED_ERR;
    }

    return JC_SPCOM_SUCCESS;
} /* sp_jc_send_pup_cmd_and_wait_on_resp */


/**
 * sp_jc_send_apdu_cmd_and_wait_on_resp
 *
 * This function sends Java Card formatted APDU command to iUICC
 * using SPCOMM interface
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
                                                          char* SW2_ptr)
{
    int                num_of_bytes_sent  = 0;
    jc_spcom_msg_type* cmd_ptr            = NULL;
    jc_spcom_msg_type* rsp_ptr            = NULL;
    char               jc_spcom_req_buf[SPCOM_MAX_REQUEST_SIZE] = {0};
    char               jc_spcom_rsp_buf[SPCOM_MAX_RESPONSE_SIZE] = {0};

    if (sizeof(jc_spcom_msg_type) > SPCOM_MAX_RESPONSE_SIZE) {
        return JC_SPCOM_TOT_CMD_SIZE_ERR;
    }

    if (spcom_client_handle_ptr == NULL || cmd_buf_ptr == NULL ||
        (rsp_buf_size > 0 && rsp_buf_ptr == NULL) ||
        (SW1_ptr == NULL || SW2_ptr == NULL)) {
        return JC_SPCOM_NULL_POINTER_ERR;
    }

    if (cmd_len < JC_SPCOM_REQ_APDU_HDR_SIZE || cmd_len > (JC_SPCOM_MAX_REQ_APDU_PAYLOAD+JC_SPCOM_REQ_APDU_HDR_SIZE) || cmd_buf_size < cmd_len) {
        return JC_SPCOM_CMD_LEN_ERR;
    }

    cmd_ptr = (jc_spcom_msg_type*) jc_spcom_req_buf;
    rsp_ptr = (jc_spcom_msg_type*) jc_spcom_rsp_buf;

    cmd_ptr->header = JC_APDU_CMD;
    cmd_ptr->body.apdu_cmd.CLA = cmd_buf_ptr[JC_SPCOM_REQ_APDU_CLA_INDEX];
    cmd_ptr->body.apdu_cmd.INS = cmd_buf_ptr[JC_SPCOM_REQ_APDU_INS_INDEX];
    cmd_ptr->body.apdu_cmd.P1  = cmd_buf_ptr[JC_SPCOM_REQ_APDU_P1_INDEX];
    cmd_ptr->body.apdu_cmd.P2  = cmd_buf_ptr[JC_SPCOM_REQ_APDU_P2_INDEX];
    cmd_ptr->length = (uint16_t)(6+(cmd_len-JC_SPCOM_REQ_APDU_HDR_SIZE));

    /* Within we use T=0 mapping of APDUs this specifices that
       P3 depends on the case of instruction(either LC LE or 00).
       When there is data going to the card the fifth byte of the header
       (from uim) is LC. When no data is going to the card the fifth
       byte of the header from UIM is LE - and LC is 00 */
    if (cmd_len > JC_SPCOM_REQ_APDU_HDR_SIZE) {
        if(cmd_buf_ptr[JC_SPCOM_REQ_APDU_P3_INDEX] != (cmd_len - JC_SPCOM_REQ_APDU_HDR_SIZE)) {
            return JC_SPCOM_APDU_LC_ERR;
        }
        cmd_ptr->body.apdu_cmd.LC = (cmd_len - JC_SPCOM_REQ_APDU_HDR_SIZE);
        (void) memscpy(cmd_ptr->body.apdu_cmd.lc_bytes,
                       (size_t)sizeof(cmd_ptr->body.apdu_cmd.lc_bytes),
                       &cmd_buf_ptr[JC_SPCOM_REQ_APDU_DATA_INDEX],
                       (size_t)sizeof(cmd_ptr->body.apdu_cmd.lc_bytes));
        cmd_ptr->body.apdu_cmd.LE = 0;
    } else {
        cmd_ptr->body.apdu_cmd.LC = 0;
        cmd_ptr->body.apdu_cmd.LE = cmd_buf_ptr[JC_SPCOM_REQ_APDU_P3_INDEX];
    }

    num_of_bytes_sent =  spcom_client_send_message_sync((struct spcom_client*)spcom_client_handle_ptr,
                                                        (char *)cmd_ptr,
                                                        sizeof(jc_spcom_msg_type),
                                                        (char *)rsp_ptr,
                                                        sizeof(jc_spcom_msg_type),
                                                        (uint16_t)SPCOM_SYNC_CALL_TIMEOUT_MS);

    if(num_of_bytes_sent == -ETIMEDOUT) {
        LOGE("SPCOM timed out!");
    }

    if (num_of_bytes_sent != sizeof(jc_spcom_msg_type)) {
        LOGE("SPCOM tx error %d   %d", num_of_bytes_sent, (int) sizeof(jc_spcom_msg_type));
        return JC_SPCOM_TX_ERR;
    }

    /* Extract rsp and verify the same */
    if (rsp_ptr->header != JC_APDU_RSP) {
        return JC_SPCOM_CMD_FAILED_ERR;
    }

    *rsp_len_ptr = rsp_ptr->length;
    if (rsp_ptr->length > 0) {
        (void) memscpy(rsp_buf_ptr,
                       (size_t)rsp_buf_size,
                       rsp_ptr->body.apdu_rsp.rsp_bytes,
                       rsp_ptr->length);
    }

    *SW1_ptr = rsp_ptr->body.apdu_rsp.SW1;
    *SW2_ptr = rsp_ptr->body.apdu_rsp.SW2;

    return JC_SPCOM_SUCCESS;
} /* sp_jc_send_apdu_cmd_and_wait_on_resp */


/**
 * sp_jc_send_rst_cmd_and_wait_on_resp
 *
 * This function sends Java Card formatted reset command to
 * iUICC using SPCOMM interface
 *
 * @param spcom_client_handle_ptr Pointer to the spcom client.
          rsp_buf_ptr             ATR Response buffer
          rsp_buf_size            ATR Response size
          rsp_len_ptr             ATR Response len ptr
 */
jc_spcom_result_type sp_jc_send_rst_cmd_and_wait_on_resp(void* spcom_client_handle_ptr,
                                                         char* rsp_buf_ptr,
                                                         uint16_t rsp_buf_size,
                                                         uint16_t* rsp_len_ptr)
{
    int                num_of_bytes_sent = 0;
    jc_spcom_msg_type* cmd_ptr           = NULL;
    jc_spcom_msg_type* rsp_ptr           = NULL;
    char               jc_spcom_req_buf[SPCOM_MAX_REQUEST_SIZE] = {0};
    char               jc_spcom_rsp_buf[SPCOM_MAX_RESPONSE_SIZE] = {0};

    if (sizeof(jc_spcom_msg_type) > SPCOM_MAX_RESPONSE_SIZE) {
        return JC_SPCOM_TOT_CMD_SIZE_ERR;
    }

    if (spcom_client_handle_ptr == NULL) {
        return JC_SPCOM_NULL_POINTER_ERR;
    }

    cmd_ptr = (jc_spcom_msg_type*) jc_spcom_req_buf;
    rsp_ptr = (jc_spcom_msg_type*) jc_spcom_rsp_buf;

    if (rsp_buf_ptr == NULL || rsp_buf_size < sizeof(rsp_ptr->body.reset_rsp.atr)) {
        return JC_SPCOM_RSP_BUF_SIZE_ERR;
    }

    cmd_ptr->header = JC_RST_CMD;
    cmd_ptr->length = 0;

    num_of_bytes_sent =  spcom_client_send_message_sync((struct spcom_client*)spcom_client_handle_ptr,
                                                        (char *)cmd_ptr, sizeof(jc_spcom_msg_type),
                                                        (char *)rsp_ptr, sizeof(jc_spcom_msg_type),
                                                        (uint16_t)SPCOM_SYNC_CALL_TIMEOUT_MS);

    if(num_of_bytes_sent == -ETIMEDOUT) {
        LOGE("SPCOM timed out!");
    }

    if (num_of_bytes_sent != sizeof(jc_spcom_msg_type)) {
        LOGE("SPCOM tx error %d   %d", num_of_bytes_sent, (int) sizeof(jc_spcom_msg_type));
        return JC_SPCOM_TX_ERR;
    }

    /* Extract rsp and verify the same */
    if (rsp_ptr->header != JC_RST_RSP) {
        return JC_SPCOM_CMD_FAILED_ERR;
    }

    *rsp_len_ptr = rsp_ptr->length;
    if (rsp_ptr->length) {
    (void) memscpy(rsp_buf_ptr,
                   (size_t)rsp_buf_size,
                   rsp_ptr->body.reset_rsp.atr,
                       rsp_ptr->length);
    }
    else
    {
        return JC_SPCOM_CMD_FAILED_ERR;
    }

    return JC_SPCOM_SUCCESS;
} /* sp_jc_send_rst_cmd_and_wait_on_resp */


/**
 * sp_jc_send_pdown_cmd_and_wait_on_resp
 * This function sends JavaCard formatted power down command to
 * iUICC using SPCOMM interface
 *
 * @param spcom_client_handle_ptr Pointer to the spcom client.
 *        timeout_msec            Timeout value for the sync spcom API
 */
jc_spcom_result_type sp_jc_send_pdown_cmd_and_wait_on_resp(void* spcom_client_handle_ptr)
{
    int                num_of_bytes_sent = 0;
    jc_spcom_msg_type* cmd_ptr           = NULL;
    jc_spcom_msg_type* rsp_ptr           = NULL;
    char               jc_spcom_req_buf[SPCOM_MAX_REQUEST_SIZE] = {0};
    char               jc_spcom_rsp_buf[SPCOM_MAX_RESPONSE_SIZE] = {0};

    if (sizeof(jc_spcom_msg_type) > SPCOM_MAX_RESPONSE_SIZE) {
        return JC_SPCOM_TOT_CMD_SIZE_ERR;
    }

    if (spcom_client_handle_ptr == NULL) {
        return JC_SPCOM_NULL_POINTER_ERR;
    }

    cmd_ptr = (jc_spcom_msg_type*) jc_spcom_req_buf;
    rsp_ptr = (jc_spcom_msg_type*) jc_spcom_rsp_buf;

    cmd_ptr->header = JC_POWER_DOWN_CMD;
    cmd_ptr->length = 0;

    num_of_bytes_sent =  spcom_client_send_message_sync((struct spcom_client*)spcom_client_handle_ptr,
                                                        (char *)cmd_ptr, sizeof(jc_spcom_msg_type),
                                                        (char *)rsp_ptr, sizeof(jc_spcom_msg_type),
                                                        (uint16_t)SPCOM_SYNC_CALL_TIMEOUT_MS);

    if(num_of_bytes_sent == -ETIMEDOUT) {
        LOGE("SPCOM timed out!");
    }

    if (num_of_bytes_sent != sizeof(jc_spcom_msg_type)) {
        LOGE("SPCOM tx error %d   %d", num_of_bytes_sent, (int) sizeof(jc_spcom_msg_type));
        return JC_SPCOM_TX_ERR;
    }

    /* Extract rsp and verify the same */
    if (rsp_ptr->header != JC_POWER_DOWN_RSP) {
        return JC_SPCOM_CMD_FAILED_ERR;
    }

    return JC_SPCOM_SUCCESS;
} /* sp_jc_send_pdown_cmd_and_wait_on_resp */
