/******************************************************************************
#  Copyright (c) 2014 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril_uim.cpp
  @brief   RIL UTF test case definitions for UIM tests.

  DESCRIPTION
  All UIM-related test case definitions should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_test_qmi_ril.h"

uint8_t aid_value_1[16] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
uint8_t aid_value_2[16] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x04,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
uint8_t aid_value_3[16] = {0xA0,0x00,0x00,0x03,0x43,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x02,0x00,0x00,0xFF};

void load_qmi_card_status(uim_status_change_ind_msg_v01 *init_card_status, Testcase *t, QMIRILTestnode *QMI_node, AndroidTelTestnode *RIL_node)
{
   //Set new status for the next test:

     //Inject: QMI_UIM_STATUS_CHANGE_IND_V01
     uim_status_change_ind_msg_v01* c_s = QMI_node->update_default_qmi_uim_status_change_ind_v01();
     memcpy(c_s, init_card_status, sizeof(uim_status_change_ind_msg_v01));
     t->inject(*QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     RIL_node->update_default_ril_unsol_response_sim_status_changed();
     t->expect(*RIL_node);

     if (init_card_status->card_status.card_info[0].card_state != UIM_CARD_STATE_PRESENT_V01 && init_card_status->card_status_valid ||
         init_card_status->extended_card_status.card_info[0].card_state != UIM_EXTENDED_CARD_STATE_PRESENT_V01 && init_card_status->extended_card_status_valid)
     {
        //Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ
        QMI_node->update_default_qmi_dms_get_current_prl_info_req_v01();
        t->expect(*QMI_node);

        // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
        QMI_node->update_default_qmi_dms_get_current_prl_info_resp_v01();
        QMI_node->set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t->inject(*QMI_node);
     }
     free(init_card_status);
}
/*
loads qmi card status with present card state containing no applications
*/
void set_qmi_card_status_change_ind_state1(uim_status_change_ind_msg_v01 *status)
{
   status->extended_card_status_valid = TRUE;
   status->extended_card_status.index_gw_len = 0;
   status->extended_card_status.index_1x_len = 0;
   status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
   status->extended_card_status.card_info_len = 1;
}

/*
loads qmi card status with USIM, ISIM, and CSIM applications, each with app_state DETECTED
*/
void set_qmi_card_status_change_ind_state2(uim_status_change_ind_msg_v01 *status)
{
   status->extended_card_status_valid = TRUE;
   status->extended_card_status.card_info_len = 1;
   status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
   status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
   status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
   status->extended_card_status.card_info[0].app_info_len = 0x03;
   status->extended_card_status.index_gw[0] = 0x0;
   status->extended_card_status.index_gw_len = 1;
   status->extended_card_status.index_1x[0] = 0x2;
   status->extended_card_status.index_1x_len = 1;
   // App 0
   status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
   status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_DETECTED_V01;
   status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
   memcpy(status->extended_card_status.card_info[0].app_info[0].aid_value, aid_value_1, 16);

   status->extended_card_status.card_info[0].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
   status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[0].pin2.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].pin2.puk_retries = 0x00;

   // App 1
   status->extended_card_status.card_info[0].app_info[1].app_type = UIM_APP_TYPE_ISIM_V01;
   status->extended_card_status.card_info[0].app_info[1].app_state = UIM_APP_STATE_DETECTED_V01;
   status->extended_card_status.card_info[0].app_info[1].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[1].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[1].perso_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].perso_unblock_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].aid_value_len = 0x10;
   memcpy(status->extended_card_status.card_info[0].app_info[1].aid_value, aid_value_2, 16);
   status->extended_card_status.card_info[0].app_info[1].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
   status->extended_card_status.card_info[0].app_info[1].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[1].pin1.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].pin1.puk_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[1].pin2.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].pin2.puk_retries = 0x00;

   // App 2
   status->extended_card_status.card_info[0].app_info[2].app_type = UIM_APP_TYPE_CSIM_V01;
   status->extended_card_status.card_info[0].app_info[2].app_state = UIM_APP_STATE_DETECTED_V01;
   status->extended_card_status.card_info[0].app_info[2].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[2].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[2].perso_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].perso_unblock_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].aid_value_len = 0x10;
   memcpy(status->extended_card_status.card_info[0].app_info[2].aid_value, aid_value_3, 16);
   status->extended_card_status.card_info[0].app_info[2].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
   status->extended_card_status.card_info[0].app_info[2].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
   status->extended_card_status.card_info[0].app_info[2].pin1.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].pin1.puk_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[2].pin2.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].pin2.puk_retries = 0x00;
}

/*
loads qmi card status with USIM, ISIM, and CSIM applications, each with app_state DETECTED
*/
void set_qmi_card_status_response_state2(uim_get_card_status_resp_msg_v01 *status)
{
   status->extended_card_status_valid = TRUE;
   status->extended_card_status.card_info_len = 1;
   status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
   status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
   status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
   status->extended_card_status.card_info[0].app_info_len = 0x03;
   status->extended_card_status.index_gw[0] = 0x0;
   status->extended_card_status.index_gw_len = 1;
   status->extended_card_status.index_1x[0] = 0x2;
   status->extended_card_status.index_1x_len = 1;
   // App 0
   status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
   status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_DETECTED_V01;
   status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
   memcpy(status->extended_card_status.card_info[0].app_info[0].aid_value, aid_value_1, 16);

   status->extended_card_status.card_info[0].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
   status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[0].pin2.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[0].pin2.puk_retries = 0x00;

   // App 1
   status->extended_card_status.card_info[0].app_info[1].app_type = UIM_APP_TYPE_ISIM_V01;
   status->extended_card_status.card_info[0].app_info[1].app_state = UIM_APP_STATE_DETECTED_V01;
   status->extended_card_status.card_info[0].app_info[1].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[1].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[1].perso_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].perso_unblock_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].aid_value_len = 0x10;
   memcpy(status->extended_card_status.card_info[0].app_info[1].aid_value, aid_value_2, 16);
   status->extended_card_status.card_info[0].app_info[1].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
   status->extended_card_status.card_info[0].app_info[1].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[1].pin1.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].pin1.puk_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[1].pin2.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[1].pin2.puk_retries = 0x00;

   // App 2
   status->extended_card_status.card_info[0].app_info[2].app_type = UIM_APP_TYPE_CSIM_V01;
   status->extended_card_status.card_info[0].app_info[2].app_state = UIM_APP_STATE_DETECTED_V01;
   status->extended_card_status.card_info[0].app_info[2].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[2].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[2].perso_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].perso_unblock_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].aid_value_len = 0x10;
   memcpy(status->extended_card_status.card_info[0].app_info[2].aid_value, aid_value_3, 16);
   status->extended_card_status.card_info[0].app_info[2].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
   status->extended_card_status.card_info[0].app_info[2].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
   status->extended_card_status.card_info[0].app_info[2].pin1.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].pin1.puk_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
   status->extended_card_status.card_info[0].app_info[2].pin2.pin_retries = 0x00;
   status->extended_card_status.card_info[0].app_info[2].pin2.puk_retries = 0x00;
}

void insert_qmi_uim_get_label_calls(QMIRILTestnode *QMI_node, Testcase *t)
{
     //Expect: QMI_UIM_GET_LABEL_REQ
     uim_get_label_req_msg_v01 *label_req1 = QMI_node->update_default_qmi_uim_get_label_req();
     label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
     label_req1->application_information.aid_len = 0x10;
     memcpy(label_req1->application_information.aid, aid_value_1, 16);//sizeof(label_req->application_information.aid));
     t->expect(*QMI_node);

     //Inject: QMI_UIM_GET_LABEL_RESP
     uim_get_label_resp_msg_v01 *response1 = QMI_node->update_default_qmi_uim_get_label_resp();
     response1->resp.result = QMI_RESULT_SUCCESS_V01;
     strcpy(response1->label, "USIM");
     response1->label_len = 4;
     response1->label_valid = TRUE;
     t->inject(*QMI_node);

     //Expect: QMI_UIM_GET_LABEL_REQ
     uim_get_label_req_msg_v01 *label_req2 = QMI_node->update_default_qmi_uim_get_label_req();
     label_req2->application_information.slot = (uim_slot_enum_v01)0x01;
     label_req2->application_information.aid_len = 0x10;
     memcpy(label_req2->application_information.aid, aid_value_2, 16);//sizeof(label_req->application_information.aid));
     t->expect(*QMI_node);

     //Inject: QMI_UIM_GET_LABEL_RESP
     uim_get_label_resp_msg_v01 *response2 = QMI_node->update_default_qmi_uim_get_label_resp();
     response2->resp.result = QMI_RESULT_SUCCESS_V01;
     strcpy(response2->label, "ISIM");
     response2->label_len = 4;
     response2->label_valid = TRUE;
     t->inject(*QMI_node);

     //Expect: QMI_UIM_GET_LABEL_REQ
     uim_get_label_req_msg_v01 *label_req3 = QMI_node->update_default_qmi_uim_get_label_req();
     label_req3->application_information.slot = (uim_slot_enum_v01)0x01;
     label_req3->application_information.aid_len = 0x10;
     memcpy(label_req3->application_information.aid, aid_value_3, 16);//sizeof(label_req->application_information.aid));
     t->expect(*QMI_node);

     //Inject: QMI_UIM_GET_LABEL_RESP
     uim_get_label_resp_msg_v01 *response3 = QMI_node->update_default_qmi_uim_get_label_resp();
     response3->resp.result = QMI_RESULT_SUCCESS_V01;
     strcpy(response3->label, "CSIM");
     response3->label_len = 4;
     response3->label_valid = TRUE;
     t->inject(*QMI_node);
}
/**==
@testname: ril_utf_test_get_card_status_1_1_1
@description: Power up with card present during initialization.
==**/
test_result ril_utf_test_get_card_status_1_1_1(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_1_1");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Set up initial card status for test case
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state1(init_status);
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = (RIL_PinState)0x0;
     status_ptr->cdma_subscription_app_index = -1;
     status_ptr->gsm_umts_subscription_app_index = -1;
     status_ptr->ims_subscription_app_index = -1;
     t.expect(TEL_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}
/**==
@testname: ril_utf_test_get_card_status_1_1_2
@description: Power up with card present then changed to error - no_atr.
==**/
test_result ril_utf_test_get_card_status_1_1_2(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;

     t.set_test_id("ril_utf_test_get_card_status_1_1_2");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Inject: QMI_UIM_STATUS_CHANGE_IND_V01
     uim_status_change_ind_msg_v01* ptr = QMI_node.update_default_qmi_uim_status_change_ind_v01();
     memset(ptr, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state1(ptr);
     ptr->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ERROR_V01;
     ptr->extended_card_status.card_info[0].error_code = UIM_CARD_ERROR_CODE_NO_ATR_RECEIVED_V01;
     t.inject(QMI_node);


     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //The QMI_DMS_GET_CURRENT_PRL_INFO req/resp is for handling a message received upon card error

     //Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ
     QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
     t.expect(QMI_node);

     // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
     QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
     QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     t.inject(QMI_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_ABSENT; //QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED maps to card_absent in qcril_uim_convert_card_state
     status_ptr->universal_pin_state = (RIL_PinState)0x0;
     status_ptr->cdma_subscription_app_index = -1;
     status_ptr->gsm_umts_subscription_app_index = -1;
     status_ptr->ims_subscription_app_index = -1;
     t.expect(TEL_node);

     // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
     t.expect(QMI_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_test_get_card_status_1_1_3
@description: Power up with card error - POWER_DOWN during initialization.
==**/
test_result ril_utf_test_get_card_status_1_1_3(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;

     t.set_test_id("ril_utf_test_get_card_status_1_1_3");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Set bootup initialization to Error - Power Down
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state1(init_status);
     init_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ERROR_V01;
     init_status->extended_card_status.card_info[0].error_code = UIM_CARD_ERROR_CODE_POWER_DOWN_V01;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_ABSENT;
     status_ptr->universal_pin_state = (RIL_PinState)0x0;
     status_ptr->cdma_subscription_app_index = -1;
     status_ptr->gsm_umts_subscription_app_index = -1;
     status_ptr->ims_subscription_app_index = -1;
     t.expect(TEL_node);

     // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
     t.expect(QMI_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_test_get_card_status_1_1_4
@description: Power up with card error - TECHNICAL_PROBLEMS during initialization.
==**/
test_result ril_utf_test_get_card_status_1_1_4(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;

     t.set_test_id("ril_utf_test_get_card_status_1_1_4");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Set bootup initialization to Error - Technical Problems
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state1(init_status);
     init_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ERROR_V01;
     init_status->extended_card_status.card_info[0].error_code = UIM_CARD_ERROR_CODE_SIM_TECHNICAL_PROBLEMS_V01;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_ERROR;
     status_ptr->universal_pin_state = (RIL_PinState)0x0;
     status_ptr->gsm_umts_subscription_app_index = -1;
     status_ptr->cdma_subscription_app_index = -1;
     status_ptr->ims_subscription_app_index = -1;
     t.expect(TEL_node);

     // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
     t.expect(QMI_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_test_get_card_status_1_1_5
@description: Send a status change indication with an invalid extended_card_status_valid TLV.
@             Verify that no further indications occur inside RIL.
==**/
test_result ril_utf_test_get_card_status_1_1_5(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;

     t.set_test_id("ril_utf_test_get_card_status_1_1_5");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Inject: QMI_UIM_STATUS_CHANGE_IND
     init_status = QMI_node.update_default_qmi_uim_status_change_ind_v01();
     set_qmi_card_status_change_ind_state1(init_status);
     init_status->extended_card_status_valid = FALSE;
     t.inject(QMI_node);

     /* Note: there shouldn't be any more messages here. If for some reason RIL does respond
        to the invalid indication, a new "expect" point will be created and show up as a failed
        test point inside the UTF. It will show "Error!! Message received when none was expected"
     */

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_test_get_card_status_1_1_6
Power up with card absent during initialization
==**/
test_result ril_utf_test_get_card_status_1_1_6(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;

     t.set_test_id("ril_utf_test_get_card_status_1_1_6");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Set bootup initialization to Absent
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state1(init_status);
     init_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ABSENT_V01;

     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);
     //TODO: Fix status-retrieval callflow inside test case

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_ABSENT;
     status_ptr->universal_pin_state = (RIL_PinState)0x0;
     status_ptr->cdma_subscription_app_index = -1;
     status_ptr->gsm_umts_subscription_app_index = -1;
     status_ptr->ims_subscription_app_index = -1;
     t.expect(TEL_node);

     // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
     t.expect(QMI_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_test_get_card_status_1_1_7
@description: Power up with card present then changed to absent.
==**/
test_result ril_utf_test_get_card_status_1_1_7(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;


     t.set_test_id("ril_utf_test_get_card_status_1_1_7");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Inject: QMI_UIM_STATUS_CHANGE_IND
     uim_status_change_ind_msg_v01* ptr = QMI_node.update_default_qmi_uim_status_change_ind_v01();
     set_qmi_card_status_change_ind_state1(ptr);
     ptr->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ABSENT_V01; //0x00
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ
     QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
     t.expect(QMI_node);

     //Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP
     QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
     QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     t.inject(QMI_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_ABSENT; //0x00
     status_ptr->universal_pin_state = (RIL_PinState)0x0;
     status_ptr->cdma_subscription_app_index = -1;
     status_ptr->gsm_umts_subscription_app_index = -1;
     status_ptr->ims_subscription_app_index = -1;
     t.expect(TEL_node);

     // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
     t.expect(QMI_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_test_get_card_status_1_1_8
@description: Power up with card present then changed to error - POLL_ERROR.
==**/
test_result ril_utf_test_get_card_status_1_1_8(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;

     t.set_test_id("ril_utf_test_get_card_status_1_1_8");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Inject: QMI_UIM_STATUS_CHANGE_IND_V01
     uim_status_change_ind_msg_v01* ptr = QMI_node.update_default_qmi_uim_status_change_ind_v01();
     set_qmi_card_status_change_ind_state1(ptr);
     ptr->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ERROR_V01; //0x02
     ptr->extended_card_status.card_info[0].error_code = UIM_CARD_ERROR_CODE_POLL_ERROR_V01; //0x02
     t.inject(QMI_node);


     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ
     QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
     t.expect(QMI_node);

     //Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
     QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
     QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     t.inject(QMI_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_ERROR; //0x02
     status_ptr->universal_pin_state = (RIL_PinState)0x0;
     status_ptr->cdma_subscription_app_index = -1;
     status_ptr->gsm_umts_subscription_app_index = -1;
     status_ptr->ims_subscription_app_index = -1;
     t.expect(TEL_node);

     // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
     t.expect(QMI_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_test_get_card_status_1_1_9
@description: Power up with card absent then changed to present.
==**/
test_result ril_utf_test_get_card_status_1_1_9(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;

     t.set_test_id("ril_utf_test_get_card_status_1_1_9");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Set bootup initialization to Absent
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state1(init_status);
     init_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ABSENT_V01;

     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: QMI_UIM_STATUS_CHANGE_IND_V01
     uim_status_change_ind_msg_v01* ptr = QMI_node.update_default_qmi_uim_status_change_ind_v01();
     memset(ptr, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state1(ptr);
     t.inject(QMI_node);

     //Expect: QMI_DMS_UIM_GET_ICCID_REQ
     QMI_node.update_default_qmi_dms_get_iccid_req();
     t.expect(QMI_node);

     //Inject: QMI_DMS_UIM_GET_ICCID_RESP
     QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ
     QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
     t.expect(QMI_node);

     // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
     QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
     QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     t.inject(QMI_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT; //0x02
     status_ptr->cdma_subscription_app_index = -1;
     status_ptr->gsm_umts_subscription_app_index = -1;
     status_ptr->ims_subscription_app_index = -1;
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_test_get_card_status_1_3_1
@description: State change of USIM from DETECTED to PIN1 REQUIRED.
==**/

test_result ril_utf_test_get_card_status_1_3_1(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     t.set_test_id("ril_utf_test_get_card_status_1_3_1");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //TODO: Fix status-retrieval callflow inside test case
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;

     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: QMI_UIM_STATUS_CHANGE_IND_V01
     changed_status = QMI_node.update_default_qmi_uim_status_change_ind_v01();
     set_qmi_card_status_change_ind_state2(changed_status);

     changed_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PIN1_OR_UPIN_REQ_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x3;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0xa;

     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_PIN;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[1].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_test_get_card_status_1_3_2
@description: State change of USIM from PIN1 REQURED to READY
              (successful PIN verification)
==**/

test_result ril_utf_test_get_card_status_1_3_2(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_3_2");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Set initial bootup state
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);
     init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PIN1_OR_UPIN_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;

     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_PIN;
     status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: RIL_REQUEST_ENTER_SIM_PIN
     char **enter_pin_sim_pin_req = TEL_node.update_default_ril_request_enter_sim_pin();
     strcpy(enter_pin_sim_pin_req[0], "3512");
     strcpy(enter_pin_sim_pin_req[1], "a0000000871002f310ffff89080000ff");
     t.inject(TEL_node);

     //Expect: QMI_UIM_VERIFY_PIN_REQ
     uim_verify_pin_req_msg_v01 *verify_pin_req = QMI_node.update_default_qmi_uim_verify_pin_req();
     verify_pin_req->verify_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
     strcpy(verify_pin_req->verify_pin.pin_value, "3512");
     verify_pin_req->verify_pin.pin_value_len = 4;
     t.expect(QMI_node);

     //Inject: QMI_UIM_VERIFY_PIN_RESP
     uim_verify_pin_resp_msg_v01 *verify_pin_resp = QMI_node.update_default_qmi_uim_verify_pin_resp();
     verify_pin_resp->resp.result = QMI_RESULT_SUCCESS_V01;
     verify_pin_resp->resp.error = QMI_ERR_NONE_V01;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_CARD_STATUS_REQ
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(TRUE);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_CARD_STATUS_RESP
     uim_get_card_status_resp_msg_v01 *qmi_resp = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     set_qmi_card_status_response_state2(qmi_resp);
     qmi_resp->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_VERIFIED_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_SERVICE_STATUS_REQ
     uim_session_information_type_v01 session_info;
     memset(&session_info, 0x0, sizeof(uim_session_information_type_v01));
     session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
     QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_SERVICE_STATUS_RESP
     QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: RIL_REQUEST_ENTER_SIM_PIN_RESPONSE
     int *enter_sim_pin_resp = TEL_node.update_default_ril_request_enter_sim_pin_resp();
     *enter_sim_pin_resp = 0x00; //used to verify that PIN verification was successful
     TEL_node.set_ril_error_value(RIL_E_SUCCESS);
     t.expect(TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_READY;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_VERIFIED;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "CSIM");
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_test_get_card_status_1_3_3
@description: USIM = PIN1 REQUIRED, PIN verification unsuccessful.
==**/

test_result ril_utf_test_get_card_status_1_3_3(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_3_3");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);
     init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PIN1_OR_UPIN_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;

     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_PIN;
     status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: RIL_REQUEST_ENTER_SIM_PIN
     char **enter_pin_sim_pin_req = TEL_node.update_default_ril_request_enter_sim_pin();
     enter_pin_sim_pin_req[0] = "3512";
     enter_pin_sim_pin_req[1] = "a0000000871002f310ffff89080000ff";
     t.inject(TEL_node);

     //Expect: QMI_UIM_VERIFY_PIN_REQ
     uim_verify_pin_req_msg_v01 *verify_pin_req = QMI_node.update_default_qmi_uim_verify_pin_req();
     verify_pin_req->verify_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
     strcpy(verify_pin_req->verify_pin.pin_value, "3512");
     verify_pin_req->verify_pin.pin_value_len = 4;
     t.expect(QMI_node);

     //Inject: QMI_UIM_VERIFY_PIN_RESP
     uim_verify_pin_resp_msg_v01 *verify_pin_resp = QMI_node.update_default_qmi_uim_verify_pin_resp();
     verify_pin_resp->resp.result = QMI_RESULT_FAILURE_V01;
     verify_pin_resp->resp.error = QMI_ERR_INCORRECT_PIN_V01;
     verify_pin_resp->retries_left.verify_left = 2;
     verify_pin_resp->retries_left.unblock_left = 10;
     verify_pin_resp->retries_left_valid = TRUE;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_CARD_STATUS_REQ
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(TRUE);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_CARD_STATUS_RESP
     uim_get_card_status_resp_msg_v01 *qmi_resp = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     set_qmi_card_status_response_state2(qmi_resp);
     qmi_resp->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PIN1_OR_UPIN_REQ_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x02;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: RIL_REQUEST_ENTER_SIM_PIN_RESPONSE
     int *enter_sim_pin_resp = TEL_node.update_default_ril_request_enter_sim_pin_resp();
     *enter_sim_pin_resp = 2;
     TEL_node.set_ril_error_value(RIL_E_PASSWORD_INCORRECT);
     t.expect(TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_PIN;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "CSIM");
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_test_get_card_status_1_3_4
@description: USIM = PIN1 REQUIRED, PIN BLOCKED by unsuccessful verification.
==**/

test_result ril_utf_test_get_card_status_1_3_4(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_3_4");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);
     init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PIN1_OR_UPIN_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;

     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_PIN;
     status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: RIL_REQUEST_ENTER_SIM_PIN
     char **enter_pin_sim_pin_req = TEL_node.update_default_ril_request_enter_sim_pin();
     enter_pin_sim_pin_req[0] = "3512";
     enter_pin_sim_pin_req[1] = "a0000000871002f310ffff89080000ff";
     t.inject(TEL_node);

     //Expect: QMI_UIM_VERIFY_PIN_REQ
     uim_verify_pin_req_msg_v01 *verify_pin_req = QMI_node.update_default_qmi_uim_verify_pin_req();
     verify_pin_req->verify_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
     strcpy(verify_pin_req->verify_pin.pin_value, "3512");
     verify_pin_req->verify_pin.pin_value_len = 4;
     t.expect(QMI_node);

     //Inject: QMI_UIM_VERIFY_PIN_RESP
     uim_verify_pin_resp_msg_v01 *verify_pin_resp = QMI_node.update_default_qmi_uim_verify_pin_resp();
     verify_pin_resp->resp.result = QMI_RESULT_FAILURE_V01;
     verify_pin_resp->resp.error = QMI_ERR_PIN_BLOCKED_V01;
     verify_pin_resp->retries_left.unblock_left = 10;
     verify_pin_resp->retries_left.verify_left = 0;
     verify_pin_resp->retries_left_valid = TRUE;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_CARD_STATUS_REQ
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(TRUE);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_CARD_STATUS_RESP
     uim_get_card_status_resp_msg_v01 *qmi_resp = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     set_qmi_card_status_response_state2(qmi_resp);
     qmi_resp->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PUK1_OR_PUK_REQ_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_BLOCKED_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: RIL_REQUEST_ENTER_SIM_PIN_RESPONSE
     int *enter_sim_pin_resp = TEL_node.update_default_ril_request_enter_sim_pin_resp();
     *enter_sim_pin_resp = 0;
     TEL_node.set_ril_error_value(RIL_E_PASSWORD_INCORRECT);
     t.expect(TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_PUK;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_BLOCKED;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "CSIM");
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**==
@testname: ril_utf_test_get_card_status_1_3_5
@description: USIM = PUK; Successfully unblock PIN.
==**/

test_result ril_utf_test_get_card_status_1_3_5(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_3_5");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);
     init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PUK1_OR_PUK_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_BLOCKED_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;

     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_PUK;
     status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_BLOCKED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: RIL_REQUEST_ENTER_SIM_PUK
     char **enter_pin_sim_puk_req = TEL_node.update_default_ril_request_enter_sim_puk();
     enter_pin_sim_puk_req[0] = "1337";
     enter_pin_sim_puk_req[1] = "3512";
     enter_pin_sim_puk_req[2] = "a0000000871002f310ffff89080000ff";
     t.inject(TEL_node);

     //Expect: QMI_UIM_UNBLOCK_PIN_REQ
     uim_unblock_pin_req_msg_v01 *unblock_pin_req = QMI_node.update_default_qmi_uim_unblock_pin_req();
     unblock_pin_req->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
     strcpy(unblock_pin_req->unblock_pin.puk_value, "1337");
     strcpy(unblock_pin_req->unblock_pin.new_pin_value, "3512");
     unblock_pin_req->unblock_pin.puk_value_len = 4;
     unblock_pin_req->unblock_pin.new_pin_value_len = 4;
     unblock_pin_req->unblock_pin.pin_id = (uim_pin_id_enum_v01)1;
     t.expect(QMI_node);

     //Inject: QMI_UIM_UNBLOCK_PIN_RESP
     uim_unblock_pin_resp_msg_v01 *unblock_pin_resp = QMI_node.update_default_qmi_uim_unblock_pin_resp();
     unblock_pin_resp->resp.result = QMI_RESULT_SUCCESS_V01;
     unblock_pin_resp->resp.error = QMI_ERR_NONE_V01;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_CARD_STATUS_REQ
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(TRUE);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_CARD_STATUS_RESP
     uim_get_card_status_resp_msg_v01 *qmi_resp = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     set_qmi_card_status_response_state2(qmi_resp);
     qmi_resp->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_VERIFIED_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_SERVICE_STATUS_REQ
     uim_session_information_type_v01 session_info;
     memset(&session_info, 0x0, sizeof(uim_session_information_type_v01));
     session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
     QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_SERVICE_STATUS_RESP
     QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: RIL_REQUEST_ENTER_SIM_PUK_RESPONSE
     int *enter_sim_puk_resp = TEL_node.update_default_ril_request_enter_sim_puk_resp();
     TEL_node.set_ril_error_value(RIL_E_SUCCESS);
     t.expect(TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_READY;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_VERIFIED;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "CSIM");
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_test_get_card_status_1_3_6
@description: USIM = PUK required; UNBLOCK PIN failure
==**/

test_result ril_utf_test_get_card_status_1_3_6(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_3_6");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);
     init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PUK1_OR_PUK_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_BLOCKED_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x02;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;

     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_PUK;
     status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_BLOCKED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: RIL_REQUEST_ENTER_SIM_PUK
     char **enter_pin_sim_puk_req = TEL_node.update_default_ril_request_enter_sim_puk();
     enter_pin_sim_puk_req[0] = "1336";
     enter_pin_sim_puk_req[1] = "3512";
     enter_pin_sim_puk_req[2] = "a0000000871002f310ffff89080000ff";
     t.inject(TEL_node);

     //Expect: QMI_UIM_UNBLOCK_PIN_REQ
     uim_unblock_pin_req_msg_v01 *unblock_pin_req = QMI_node.update_default_qmi_uim_unblock_pin_req();
     unblock_pin_req->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
     strcpy(unblock_pin_req->unblock_pin.puk_value, "1336");
     strcpy(unblock_pin_req->unblock_pin.new_pin_value, "3512");
     unblock_pin_req->unblock_pin.puk_value_len = 4;
     unblock_pin_req->unblock_pin.new_pin_value_len = 4;
     unblock_pin_req->unblock_pin.pin_id = (uim_pin_id_enum_v01)1;
     t.expect(QMI_node);

     //Inject: QMI_UIM_UNBLOCK_PIN_RESP
     uim_unblock_pin_resp_msg_v01 *unblock_pin_resp = QMI_node.update_default_qmi_uim_unblock_pin_resp();
     unblock_pin_resp->resp.result = QMI_RESULT_FAILURE_V01;
     unblock_pin_resp->resp.error = QMI_ERR_PIN_BLOCKED_V01;
     unblock_pin_resp->retries_left.unblock_left = 0x01;
     unblock_pin_resp->retries_left_valid = TRUE;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_CARD_STATUS_REQ
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(TRUE);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_CARD_STATUS_RESP
     uim_get_card_status_resp_msg_v01 *qmi_resp = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     set_qmi_card_status_response_state2(qmi_resp);
     qmi_resp->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PUK1_OR_PUK_REQ_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_BLOCKED_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x01;
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: RIL_REQUEST_ENTER_SIM_PUK_RESPONSE
     int *enter_sim_puk_resp = TEL_node.update_default_ril_request_enter_sim_puk_resp();
     *enter_sim_puk_resp = 0x01;
     TEL_node.set_ril_error_value(RIL_E_PASSWORD_INCORRECT);
     t.expect(TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_PUK;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_BLOCKED;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[1].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[2].app_label_ptr, "CSIM");
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_test_get_card_status_1_3_7
@description: USIM = PUK required; UNBLOCK PIN failure then PIN PERM BLOCKED
==**/

test_result ril_utf_test_get_card_status_1_3_7(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_3_7");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PUK1_OR_PUK_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_BLOCKED_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x01;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;
     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_PUK;
     status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_BLOCKED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: RIL_REQUEST_ENTER_SIM_PUK
     char **enter_pin_sim_puk_req = TEL_node.update_default_ril_request_enter_sim_puk();
     enter_pin_sim_puk_req[0] = "1336";
     enter_pin_sim_puk_req[1] = "3512";
     enter_pin_sim_puk_req[2] = "a0000000871002f310ffff89080000ff";
     t.inject(TEL_node);

     //Expect: QMI_UIM_UNBLOCK_PIN_REQ
     uim_unblock_pin_req_msg_v01 *unblock_pin_req = QMI_node.update_default_qmi_uim_unblock_pin_req();
     unblock_pin_req->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
     strcpy(unblock_pin_req->unblock_pin.puk_value, "1336");
     strcpy(unblock_pin_req->unblock_pin.new_pin_value, "3512");
     unblock_pin_req->unblock_pin.puk_value_len = 4;
     unblock_pin_req->unblock_pin.new_pin_value_len = 4;
     unblock_pin_req->unblock_pin.pin_id = (uim_pin_id_enum_v01)1;
     t.expect(QMI_node);

     //Inject: QMI_UIM_UNBLOCK_PIN_RESP
     uim_unblock_pin_resp_msg_v01 *unblock_pin_resp = QMI_node.update_default_qmi_uim_unblock_pin_resp();
     unblock_pin_resp->resp.result = QMI_RESULT_FAILURE_V01;
     unblock_pin_resp->resp.error = QMI_ERR_PIN_PERM_BLOCKED_V01;
     unblock_pin_resp->retries_left.unblock_left = 0x00;
     unblock_pin_resp->retries_left_valid = TRUE;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_CARD_STATUS_REQ
     QMI_node.update_default_qmi_uim_get_card_status_req();
     QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(TRUE);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_CARD_STATUS_RESP
     uim_get_card_status_resp_msg_v01 *qmi_resp = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     set_qmi_card_status_response_state2(qmi_resp);
     qmi_resp->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PIN1_PERM_BLOCKED_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_PERMANENTLY_BLOCKED_V01;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
     qmi_resp->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x00;
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Expect: RIL_REQUEST_ENTER_SIM_PUK_RESPONSE
     int *enter_sim_puk_resp = TEL_node.update_default_ril_request_enter_sim_puk_resp();
     *enter_sim_puk_resp = 0x00;
     TEL_node.set_ril_error_value(RIL_E_PASSWORD_INCORRECT);
     t.expect(TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_PUK;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_PERM_BLOCKED;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[1].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[2].app_label_ptr, "CSIM");
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**==
@testname: ril_utf_test_get_card_status_1_3_8
@description: State change of USIM from CHECK_PERSO to READY
              (successful perso verification)
==**/

test_result ril_utf_test_get_card_status_1_3_8(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_3_8");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Set initial bootup state
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);
     init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PERSON_CHECK_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_VERIFIED_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     init_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_CODE_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_GW_NETWORK_V01;
     init_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x03;
     init_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x0A;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;

     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_SUBSCRIPTION_PERSO;
     status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_VERIFIED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_SIM_NETWORK;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ
     char **enter_pin_sim_pin_req = TEL_node.update_default_ril_request_enter_depersonalization_code();
     strcpy(enter_pin_sim_pin_req[0], "3");
     strcpy(enter_pin_sim_pin_req[1], "12345678");
     t.inject(TEL_node);

     //Expect: QMI_UIM_DEPERSONALIZATION_REQ
     uim_depersonalization_req_msg_v01 *deperso_req = QMI_node.update_default_qmi_uim_depersonalization_req();
     deperso_req->depersonalization.feature = UIM_PERSO_FEATURE_GW_NETWORK_V01;
     strcpy(deperso_req->depersonalization.ck_value, "12345678");
     deperso_req->depersonalization.ck_value_len = 0x08;
     t.expect(QMI_node);

     //Inject: QMI_UIM_DEPERSONALIZATION_RESP
     uim_depersonalization_resp_msg_v01 *deperso_resp = QMI_node.update_default_qmi_uim_depersonalization_resp();
     deperso_resp->resp.result = QMI_RESULT_SUCCESS_V01;
     deperso_resp->resp.error = QMI_ERR_NONE_V01;
     t.inject(QMI_node);

     //Expect: QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ response
     TEL_node.update_default_ril_request_enter_depersonalization_code_resp();
     TEL_node.set_ril_error_value(RIL_E_SUCCESS);
     t.expect(TEL_node);

     //Inject: QMI_UIM_STATUS_CHANGE_IND_V01
     changed_status = QMI_node.update_default_qmi_uim_status_change_ind_v01();
     set_qmi_card_status_change_ind_state2(changed_status);
     changed_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_VERIFIED_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     changed_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_GW_NETWORK_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x03;
     changed_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x0A;
     t.inject(QMI_node);

     //Expect: QMI_UIM_GET_SERVICE_STATUS_REQ
     uim_session_information_type_v01 session_info;
     memset(&session_info, 0x0, sizeof(uim_session_information_type_v01));
     session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
     QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
     t.expect(QMI_node);

     //Inject: QMI_UIM_GET_SERVICE_STATUS_RESP
     QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //TODO: The messages for the ENTER_DEPERSONALIZATION_CODE and
     //GET_SIM_STATUS are somehow getting combined

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_READY;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_VERIFIED;
     new_status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_READY;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "CSIM");
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_test_get_card_status_1_3_9
@description: State change of USIM from CHECK_PERSO to
              (successful perso verification)
==**/

test_result ril_utf_test_get_card_status_1_3_9(user_specified_attrib_t attr)
{
     Testcase t;
     QMIRILTestnode QMI_node;
     AndroidTelTestnode TEL_node;
     uim_status_change_ind_msg_v01 *init_status;
     uim_status_change_ind_msg_v01 *changed_status;

     uim_get_label_req_msg_v01 *label_req;

     t.set_test_id("ril_utf_test_get_card_status_1_3_9");
     t.set_test_attribute("UIM");
     if (!t.isAttributeMatch(attr)) {
       test_result ret;
       return ret;
     }

     //Set initial bootup state
     init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
     memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
     set_qmi_card_status_change_ind_state2(init_status);
     init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PERSON_CHECK_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_VERIFIED_V01;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
     init_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     init_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_CODE_REQ_V01;
     init_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_GW_NETWORK_V01;
     init_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x03;
     init_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x0A;
     load_qmi_card_status(init_status, &t, &QMI_node, &TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
     status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     status_ptr->gsm_umts_subscription_app_index = 0;
     status_ptr->cdma_subscription_app_index = 2;
     status_ptr->ims_subscription_app_index = -1;
     status_ptr->num_applications = 3;

     status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     status_ptr->applications[0].app_state = RIL_APPSTATE_SUBSCRIPTION_PERSO;
     status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_VERIFIED;
     status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_SIM_NETWORK;
     status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

     status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

     status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

     t.expect(TEL_node);

     //Inject: QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ
     char **enter_pin_sim_pin_req = TEL_node.update_default_ril_request_enter_depersonalization_code();
     strcpy(enter_pin_sim_pin_req[0], "3");
     strcpy(enter_pin_sim_pin_req[1], "87654321");
     t.inject(TEL_node);

     //Expect: QMI_UIM_DEPERSONALIZATION_REQ
     uim_depersonalization_req_msg_v01 *deperso_req = QMI_node.update_default_qmi_uim_depersonalization_req();
     deperso_req->depersonalization.feature = UIM_PERSO_FEATURE_GW_NETWORK_V01;
     strcpy(deperso_req->depersonalization.ck_value, "87654321");
     deperso_req->depersonalization.ck_value_len = 0x08;
     t.expect(QMI_node);

     //Inject: QMI_UIM_DEPERSONALIZATION_RESP
     uim_depersonalization_resp_msg_v01 *deperso_resp = QMI_node.update_default_qmi_uim_depersonalization_resp();
     deperso_resp->resp.result = QMI_RESULT_FAILURE_V01;
     deperso_resp->resp.error = QMI_ERR_INCORRECT_PIN_V01;
     deperso_resp->retries_left.verify_left = 0x02;
     deperso_resp->retries_left.unblock_left = 0x0A;
     deperso_resp->retries_left_valid = TRUE;
     t.inject(QMI_node);

     //Expect: QCRIL_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ response
     int *ril_deperso_resp = TEL_node.update_default_ril_request_enter_depersonalization_code_resp();
     *ril_deperso_resp = 0x02;
     TEL_node.set_ril_error_value(RIL_E_GENERIC_FAILURE);
     t.expect(TEL_node);

     //Inject: QMI_UIM_STATUS_CHANGE_IND_V01
     changed_status = QMI_node.update_default_qmi_uim_status_change_ind_v01();
     set_qmi_card_status_change_ind_state2(changed_status);
     changed_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_PERSON_CHECK_REQ_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_ENABLED_VERIFIED_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
     changed_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
     changed_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_CODE_REQ_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_GW_NETWORK_V01;
     changed_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x02;
     changed_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x0A;
     t.inject(QMI_node);

     //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
     TEL_node.update_default_ril_unsol_response_sim_status_changed();
     t.expect(TEL_node);

     //Inject: RIL_REQUEST_GET_SIM_STATUS
     TEL_node.update_default_ril_request_get_sim_status();
     t.inject(TEL_node);

     //Inject/Expect: 3 x QMI_UIM_GET_LABEL_REQ/RESP
     insert_qmi_uim_get_label_calls(&QMI_node, &t);

     //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
     RIL_CardStatus_v6 *new_status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();

     new_status_ptr->card_state = RIL_CARDSTATE_PRESENT;
     new_status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
     new_status_ptr->gsm_umts_subscription_app_index = 0;
     new_status_ptr->cdma_subscription_app_index = 2;
     new_status_ptr->ims_subscription_app_index = -1;
     new_status_ptr->num_applications = 3;

     new_status_ptr->applications[0].app_state = RIL_APPSTATE_SUBSCRIPTION_PERSO;
     new_status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
     new_status_ptr->applications[0].pin1 = RIL_PINSTATE_ENABLED_VERIFIED;
     new_status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_SIM_NETWORK;
     new_status_ptr->applications[0].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
     new_status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "USIM");

     new_status_ptr->applications[1].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
     new_status_ptr->applications[1].aid_ptr = (char *)malloc(33);
     strcpy(new_status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
     new_status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "ISIM");

     new_status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
     new_status_ptr->applications[2].app_state = RIL_APPSTATE_DETECTED;
     new_status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
     new_status_ptr->applications[2].aid_ptr = (char *)malloc(33);
     new_status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
     strcpy(new_status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
     new_status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
     strcpy(new_status_ptr->applications[0].app_label_ptr, "CSIM");
     t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**=
 @testname:     ril_utf_test_send_status_command_testcase_1
 @description : In this test case, we try to send a status command to the
                card expecting only status words and see that response is
                propagated from RIL to Telephony accordingly
==**/
test_result ril_utf_test_send_status_command_testcase_1(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode TEL_node;
  RIL_SIM_IO_v6                    * sim_io_req_data_ptr;
  RIL_SIM_IO_Response              * sim_io_resp_ptr;
  uim_send_status_cmd_req_msg_v01  * status_command_req;
  uim_send_status_cmd_resp_msg_v01 * qmi_response;

  t.set_test_id("ril_utf_test_send_status_command_testcase_1");
  t.set_test_attribute("UIM");
  if (!t.isAttributeMatch(attr)) {
   test_result ret;
   return ret;
  }

  sim_io_req_data_ptr = TEL_node.update_default_ril_request_sim_io();

  sim_io_req_data_ptr->command = 242;
  sim_io_req_data_ptr->p1 = 0;
  sim_io_req_data_ptr->p2 = 12;

  //Inject
  t.inject(TEL_node);

  status_command_req = QMI_node.update_default_qmi_uim_send_status_req_v01();

  status_command_req->session_information.session_type =
    (uim_session_type_enum_v01)UIM_SESSION_TYPE_PRIMARY_GW_V01;

  status_command_req->status_cmd_mode_valid = TRUE;
  status_command_req->status_cmd_mode       =
    (uim_status_cmd_mode_enum_v01)UIM_STATUS_CMD_NO_INDICATION_V01;

  status_command_req->status_cmd_resp_valid = TRUE;
  status_command_req->status_cmd_resp       =
    (uim_status_cmd_resp_enum_v01)UIM_STATUS_CMD_NO_DATA_RESP_V01;

  //Expect
  t.expect(QMI_node);

  qmi_response = QMI_node.update_default_qmi_uim_send_status_resp_v01();

  qmi_response->resp.result       = QMI_RESULT_SUCCESS_V01;
  qmi_response->card_result_valid = TRUE;
  qmi_response->card_result.sw1   = 0x90;
  qmi_response->card_result.sw2   = 0x00;

  //Inject
  t.inject(QMI_node);

  sim_io_resp_ptr      = TEL_node.update_default_ril_request_sim_io_resp();
  sim_io_resp_ptr->sw1 = 0x90;
  sim_io_resp_ptr->sw2 = 0x00;

  //Expect
  t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**=
 @testname:     ril_utf_test_refresh_init_full_fcn_non_prov_1
 @description : In this test case, we try to send init full fcn refresh indication
                for non prov session and expect refresh complete request.
==**/
test_result ril_utf_test_refresh_init_full_fcn_non_prov_1(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode TEL_node;
  OEMTestnode OEM_node;

  uim_refresh_ind_msg_v01           * refresh_ind_ptr;
  uim_refresh_complete_req_msg_v01  * refresh_complete_ptr;
  uim_refresh_complete_resp_msg_v01 * refresh_complete_resp_ptr;
  RIL_Hook_SimRefreshResponse       * oem_refresh_indication_ptr;

  t.set_test_id("ril_utf_test_refresh_init_full_fcn_non_prov_1");
  t.set_test_attribute("UIM");
  if (!t.isAttributeMatch(attr)) {
   test_result ret;
   return ret;
  }

  refresh_ind_ptr = QMI_node.update_default_qmi_uim_refresh_ind_msg_v01();

  refresh_ind_ptr->refresh_event_valid = TRUE;
  refresh_ind_ptr->refresh_event.stage = UIM_REFRESH_STAGE_START_V01;
  refresh_ind_ptr->refresh_event.mode  = UIM_REFRESH_MODE_INIT_FCN_V01;
  refresh_ind_ptr->refresh_event.session_type = UIM_SESSION_TYPE_NONPROVISIONING_SLOT_1_V01;
  refresh_ind_ptr->refresh_event.aid_len = 0x10;
  memcpy(refresh_ind_ptr->refresh_event.aid, aid_value_2, 16);

  //Inject
  t.inject(QMI_node);

  refresh_complete_ptr = QMI_node.update_default_qmi_uim_refresh_complete_req_msg_v01();
  refresh_complete_ptr->session_information.session_type = UIM_SESSION_TYPE_NONPROVISIONING_SLOT_1_V01;
  refresh_complete_ptr->session_information.aid_len = 0x10;
  memcpy(refresh_complete_ptr->session_information.aid, aid_value_2, 16);
  refresh_complete_ptr->refresh_success = 1;
  //Expect
  t.expect(QMI_node);

  refresh_complete_resp_ptr = QMI_node.update_default_qmi_uim_refresh_complete_resp_msg_v01();

  refresh_complete_resp_ptr->resp.result = QMI_RESULT_SUCCESS_V01;
  //Inject
  t.inject(QMI_node);

  oem_refresh_indication_ptr           = OEM_node.update_default_sim_refresh_unsol_event();
  oem_refresh_indication_ptr->result   = SIM_INIT;
  oem_refresh_indication_ptr->app_type = RIL_APPTYPE_ISIM;
  oem_refresh_indication_ptr->aid_len  = 32;
  memcpy(oem_refresh_indication_ptr->aid, "a0000000871004f310ffff89080000ff", 32);
  t.expect(OEM_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**=
 @testname:     ril_utf_test_get_card_status_1
 @description : In this test case, we try to see if RIL_REQUEST_GET_SIM_STATUS is working
                as expected assuming modem does not have support to provide ICCID and ATR
                as part of uim_status_change_ind.
==**/
test_result ril_utf_test_get_card_status_1(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode TEL_node;

  uim_get_label_req_msg_v01*  label_req_ptr;
  uim_get_label_resp_msg_v01* label_resp_ptr;


  t.set_test_id("ril_utf_test_get_card_status_1");
  t.set_test_attribute("UIM");
  if (!t.isAttributeMatch(attr)) {
   test_result ret;
   return ret;
  }

  //Inject: RIL_REQUEST_GET_SIM_STATUS
  TEL_node.update_default_ril_request_get_sim_status();
  t.inject(TEL_node);

  label_req_ptr = QMI_node.update_default_qmi_uim_get_label_req();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  label_resp_ptr = QMI_node.update_default_qmi_uim_get_label_resp();
  label_resp_ptr->resp.result = QMI_RESULT_FAILURE_V01;
  label_resp_ptr->resp.error  =QMI_ERR_INTERNAL_V01;
  t.inject(QMI_node);

  label_req_ptr = QMI_node.update_default_qmi_uim_get_label_req();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  label_resp_ptr = QMI_node.update_default_qmi_uim_get_label_resp();
  label_resp_ptr->resp.result = QMI_RESULT_FAILURE_V01;
  label_resp_ptr->resp.error  =QMI_ERR_INTERNAL_V01;
  t.inject(QMI_node);

  label_req_ptr = QMI_node.update_default_qmi_uim_get_label_req();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  label_resp_ptr = QMI_node.update_default_qmi_uim_get_label_resp();
  label_resp_ptr->resp.result = QMI_RESULT_FAILURE_V01;
  label_resp_ptr->resp.error  =QMI_ERR_INTERNAL_V01;
  t.inject(QMI_node);

  label_req_ptr = QMI_node.update_default_qmi_uim_get_label_req();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  label_resp_ptr = QMI_node.update_default_qmi_uim_get_label_resp();
  label_resp_ptr->resp.result = QMI_RESULT_FAILURE_V01;
  label_resp_ptr->resp.error  =QMI_ERR_INTERNAL_V01;
  t.inject(QMI_node);

  //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
  RIL_CardStatus_v6* status_ptr = TEL_node.update_default_ril_request_get_sim_status_resp();
  status_ptr->card_state = RIL_CARDSTATE_PRESENT;
  status_ptr->atr = "3b9f97803fc7828031e073fe211f64060100829000680000000000000000000";
  status_ptr->iccid = "981099091008000028f2";
  status_ptr->universal_pin_state = (RIL_PinState)0x0;
  status_ptr->cdma_subscription_app_index = 2;
  status_ptr->gsm_umts_subscription_app_index = 0;
  status_ptr->ims_subscription_app_index = 1;
  status_ptr->num_applications = 4;

  status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
  status_ptr->applications[0].app_state = RIL_APPSTATE_READY;
  status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_READY;
  status_ptr->applications[0].aid_ptr = "a0000000871002f310ffff89080000ff";
  status_ptr->applications[0].app_label_ptr = NULL;
  status_ptr->applications[0].pin1_replaced = 0;
  status_ptr->applications[0].pin1 = RIL_PINSTATE_DISABLED;
  status_ptr->applications[0].pin2 =  RIL_PINSTATE_ENABLED_NOT_VERIFIED;

  status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
  status_ptr->applications[1].app_state = RIL_APPSTATE_READY;
  status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
  status_ptr->applications[1].aid_ptr = "a0000000871004f310ffff89080000ff";
  status_ptr->applications[1].app_label_ptr = NULL;
  status_ptr->applications[1].pin1_replaced = 0;
  status_ptr->applications[1].pin1 = RIL_PINSTATE_DISABLED;
  status_ptr->applications[1].pin2 =  RIL_PINSTATE_UNKNOWN;

  status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
  status_ptr->applications[2].app_state = RIL_APPSTATE_READY;
  status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_READY;
  status_ptr->applications[2].aid_ptr = "a0000003431002f310ffff89020000ff";
  status_ptr->applications[2].app_label_ptr = NULL;
  status_ptr->applications[2].pin1_replaced = 0;
  status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
  status_ptr->applications[2].pin2 =  RIL_PINSTATE_ENABLED_NOT_VERIFIED;

  status_ptr->applications[3].app_type = RIL_APPTYPE_UNKNOWN;
  status_ptr->applications[3].app_state = RIL_APPSTATE_DETECTED;
  status_ptr->applications[3].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
  status_ptr->applications[3].aid_ptr = "a000000063504b43532d3135";
  status_ptr->applications[3].app_label_ptr = NULL;
  status_ptr->applications[3].pin1_replaced = 0;
  status_ptr->applications[3].pin1 = RIL_PINSTATE_UNKNOWN;
  status_ptr->applications[3].pin2 =  RIL_PINSTATE_UNKNOWN;

  t.expect(TEL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_test_card_status_change
@description: card state changes from present to absent.
==**/
test_result ril_utf_test_card_status_change(user_specified_attrib_t attr)
{
    Testcase t;
    QMIRILTestnode QMI_node;
    AndroidTelTestnode TEL_node;
    OEMTestnode OEM_node;

    t.set_test_id("ril_utf_test_card_status_change");
    t.set_test_attribute("UIM");
    if (!t.isAttributeMatch(attr)) {
        test_result ret;
        return ret;
    }

    t.set_start();

    //Inject: QMI_UIM_STATUS_CHANGE_IND_V01
    uim_status_change_ind_msg_v01* ptr = QMI_node.update_default_qmi_uim_status_change_ind_v01();
    set_qmi_card_status_change_ind_state1(ptr);
    ptr->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ABSENT_V01;
    t.inject(QMI_node);

    //Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ
    QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
    t.expect(QMI_node);

    // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
    QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
    QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    // Expect: QCRIL_EVT_HOOK_UNSOL_CARD_STATE_CHANGE_IND
    OEM_node.update_default_qcril_evt_hook_unsol_card_state_change_ind();
    OEM_node.set_expect_skip_verification_status(false);
    t.expect(OEM_node);

    //Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
    TEL_node.update_default_ril_unsol_response_sim_status_changed();
    t.expect(TEL_node);

    t.set_end();

    t.execute();
    t.print_summary();

    return t.get_test_result();
}

/**==
@testname:    ril_utf_test_open_logical_channel_request_1

@description: In this test case we issue a open logical channel request and
              verify that status words sent by QMI UIM in case of failure case are
              propagated to Telephony.
==**/
test_result ril_utf_test_open_logical_channel_request_1(
              user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode TEL_node;

  uint8_t                                status_words[2]                           = {0x69, 0x85};
  uim_open_logical_channel_req_msg_v01*  qmi_open_logical_channel_req_ptr          = NULL;
  uim_open_logical_channel_resp_msg_v01* qmi_open_logical_channel_resp_ptr         = NULL;
  RIL_OpenChannelParams*                 ril_open_logical_channel_req_payload_ptr  = NULL;
  uint8_t*                               ril_open_logical_channel_resp_payload_ptr = NULL;


  t.set_test_id("ril_utf_test_open_logical_channel_request_1");
  t.set_test_attribute("UIM");
  if (!t.isAttributeMatch(attr)) {
      test_result ret;
      return ret;
  }

  //Inject: RIL_REQUEST_OPEN_LOGICAL_CHANNEL
  ril_open_logical_channel_req_payload_ptr =
      TEL_node.update_default_ril_request_open_logical_channel();
  /* Any dummy AID */
  ril_open_logical_channel_req_payload_ptr->aidPtr = "A000000063504B43532D3135";
  /* Return FCP template */
  ril_open_logical_channel_req_payload_ptr->p2     = 4;
  t.inject(TEL_node);

  qmi_open_logical_channel_req_ptr = QMI_node.update_default_qmi_uim_open_channel_req();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  qmi_open_logical_channel_resp_ptr = QMI_node.update_default_qmi_uim_open_channel_resp();
  qmi_open_logical_channel_resp_ptr->resp.result        = QMI_RESULT_FAILURE_V01;
  qmi_open_logical_channel_resp_ptr->resp.error         = QMI_ERR_SIM_FILE_NOT_FOUND_V01;
  qmi_open_logical_channel_resp_ptr->card_result_valid  = true;
  qmi_open_logical_channel_resp_ptr->card_result.sw1    = status_words[0];
  qmi_open_logical_channel_resp_ptr->card_result.sw2    = status_words[1];
  t.inject(QMI_node);

  ril_open_logical_channel_resp_payload_ptr =
      TEL_node.update_default_ril_request_open_logical_channel_resp(3);
  TEL_node.set_ril_error_value(RIL_E_NO_SUCH_ELEMENT);

  /* Channel ID */
  ril_open_logical_channel_resp_payload_ptr[0] = 0;
  /* Status Words*/
  ril_open_logical_channel_resp_payload_ptr[1] = status_words[0];
  ril_open_logical_channel_resp_payload_ptr[2] = status_words[1];
  t.expect(TEL_node);

  t.execute();
  t.print_summary();
   return t.get_test_result();
}

