/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef RIL_UTF_IOE_IF_H_
#define RIL_UTF_IOE_IF_H_

#include "ril_utf_if.h"
#include "mcm_sms_v01.h"
#include "mcm_dm_v01.h"
#include "mcm_common_v01.h"
//QMI interfaces extracted from ril_utf_if.h
typedef class ril_utf_ioe_if : public ril_utf_node_base_t {
public:
  //==================================================
  // IoE interfaces
  //==================================================
  /*MCM_SMS_SEND_MO_MSG_REQ_V01*/
    void update_mcm_sms_send_mo_msg_req_msg(uint32_t format, char * message_content, char *destination);
    void set_mcm_sms_send_mo_msg_req_msg_size_validation(uint8_t size_validation);

    void update_mcm_sms_send_mo_msg_resp_msg(mcm_result_t_v01 result, mcm_error_t_v01 error);

    /*MCM_DM_GET_RADIO_MODE_REQ_V01*/
    void update_mcm_dm_get_radio_mode_req_msg();

    /*MCM_DM_GET_RADIO_MODE_RESP_V01*/
    void update_mcm_dm_get_radio_mode_resp_msg(mcm_result_t_v01 result, mcm_error_t_v01 error);
    void set_mcm_dm_get_radio_mode_resp_msg_radio_mode(uint8_t radio_mode);


} IOETestnode;

#endif /* RIL_UTF_IOE_IF_H_ */
