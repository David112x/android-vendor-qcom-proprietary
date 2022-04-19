/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ril_api.h
  @brief   RIL API's for test case dev

  DESCRIPTION

  API's for modifying RIL messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_ril_api.h"
#include "ril_utf_defs.h"

void update_default_ril_unsol_resp_radio_state_changed(elnptr node)
{
  struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t* tmp = (struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t*) malloc(sizeof(struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t));
  tmp->state = RADIO_STATE_UNAVAILABLE;

  node->payload = tmp;
  node->payload_len = sizeof(struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t);
  node->type = ril_utf_ril_unsol_response;
  node->e.is_valid = 1;
  node->e.msg_id = RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED;

  //use default timeout
}
