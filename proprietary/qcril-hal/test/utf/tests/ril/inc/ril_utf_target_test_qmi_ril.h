/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_target_test_qmi_ril.h
  @brief   RIL UTF target test case declarations

  DESCRIPTION
  All test case declarations should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_TARGET_TEST_QMI_RIL
#define _RIL_UTF_TARGET_TEST_QMI_RIL

#include "ril_utf_test_includes.h"

//==================================================
// Bootup tests
//  Description:
//   "Boot" here refers to the sequence of actions
//   performed after the rild, main, and qmi_cb threads
//   are started and qmi client initialized has been
//   completed
//==================================================
test_result qmi_ril_target_bootup_001();
test_result ril_utf_target_mo_call_001(user_specified_attrib_t attr);
test_result ril_utf_target_mt_call_001(user_specified_attrib_t attr);
test_result ril_utf_target_ims_mo_call_001(user_specified_attrib_t attr);
test_result ril_utf_target_ims_mt_call_001(user_specified_attrib_t attr);
test_result ril_utf_target_ims_L_mo_call_001(user_specified_attrib_t attr);
test_result ril_utf_target_ims_L_mo_call_002(user_specified_attrib_t attr);
test_result ril_utf_target_L_ims_mt_call_001(user_specified_attrib_t attr);


#endif //_RIL_UTF_TARGET_TEST_QMI_RIL
