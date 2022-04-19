/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_qmi_api.h
  @brief   qmi API's for test case dev

  DESCRIPTION

  API's for modifying QMI messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _H_QCRIL_UNITTEST_QMI_DEFS_
#define _H_QCRIL_UNITTEST_QMI_DEFS_

#include "ril_utf_defs.h"
#include "ril_utf_core.h"
#include "voice_service_v02.h"
#include "device_management_service_v01.h"
#include "phonebook_manager_service_v01.h"

/*
   QMI_VOICE_INDICATION_REGISTER_REQ
*/
void update_default_qmi_voice_ind_reg_req_msg(elnptr node);
void set_qmi_voice_ind_req_reg_voice_privacy_events(elnptr node, int val);
void set_qmi_voice_ind_req_ext_brst_intl_events(elnptr node, int val);
void set_qmi_voice_ind_req_speech_events(elnptr node, int val);

/*
   QMI_VOICE_INDICATION_REGISTER_RESP
*/
void update_default_qmi_voice_ind_reg_resp_msg(elnptr node);

/* QMI_DMS_SET_EVENT_REPORT_REQ_V01
*/
void update_default_qmi_dms_set_event_report_req_msg(elnptr node);
void set_qmi_dsm_set_event_report_req_report_oprt_mode_state(elnptr node, int val);
void set_qmi_dsm_set_event_report_req_report_prl_init(elnptr node, int val);

/* QMI_DMS_SET_EVENT_REPORT_RESP_V01
*/
void update_default_qmi_dms_set_event_report_resp_msg(elnptr node);

/*
   QMI_DMS_GET_OPERATING_MODE_REQ_V01
*/
void update_default_qmi_dms_get_operating_mode_req_msg(elnptr node);

/*
   QMI_DMS_GET_OPERATING_MODE_RESP_V01
*/
void update_default_qmi_dms_get_operating_mode_resp_msg(elnptr node);
void set_qmi_dms_operating_mode_resp_operating_mode(elnptr node,dms_operating_mode_enum_v01 mode);

/*
  QMI_PBM_INDICATION_REGISTER_REQ_V01
*/
void update_default_qmi_pbm_indication_register_req_msg(elnptr node);
void set_qmi_pbm_indication_register_mask(elnptr node, long int mask);
#endif
