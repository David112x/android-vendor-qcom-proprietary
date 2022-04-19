/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril.h
  @brief   RIL UTF test case declarations

  DESCRIPTION
  All test case declarations should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_TEST_IOE_RIL
#define _RIL_UTF_TEST_IOE_RIL

#include "ril_utf_test_includes.h"

//==================================================
// Bootup tests
//  Description:
//   "Boot" here refers to the sequence of actions
//   performed after the rild, main, and qmi_cb threads
//   are started and qmi client initialized has been
//   completed
//==================================================
test_result ioe_ril_test_bootup_001(user_specified_attrib_t attr);

test_result ioe_ril_test_send_sms_001(user_specified_attrib_t attr);

test_result ioe_ril_test_get_radio_mode_001(user_specified_attrib_t attr);

#endif //_RIL_UTF_TEST_QMI_RIL
