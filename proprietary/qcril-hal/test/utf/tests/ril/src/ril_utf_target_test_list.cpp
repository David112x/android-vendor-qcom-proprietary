/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_target_test_list.cpp
  @brief   RIL UTF test case function pointer table for on target testing

  DESCRIPTION
  An array of function pointer type
  All the test case names should be in this table in order to be called
  All test case declarations should go to ril_utf_target_test_qmi_ril.h
  All test case definitions should go to ril_utf_target_test_qmi_ril.cpp
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_target_test_qmi_ril.h"

func_ptr_t qmi_ril_target_test_table[] =
{
  ril_utf_target_mo_call_001,
  //ril_utf_target_mt_call_001,
  //ril_utf_target_ims_mo_call_001,
  //ril_utf_target_ims_mt_call_001,
  //ril_utf_target_ims_L_mo_call_001,
  //ril_utf_target_ims_L_mo_call_002,
  //ril_utf_target_L_ims_mt_call_001,
  NULL // used to mark the end of the table
};
