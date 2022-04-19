/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_list_qmi_ril.cpp
  @brief   RIL UTF test case function pointer table

  DESCRIPTION
  An array of function pointer type
  All the test case names should be in this table in order to be called
  All test case declarations should go to ril_utf_test_qmi_ril.h
  All test case definitions should go to ril_utf_test_qmi_ril.cpp
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_test_ioe_ril.h"

func_ptr_t ioe_ril_test_table[] =
{
//
  ioe_ril_test_get_radio_mode_001,
    ioe_ril_test_send_sms_001,
  NULL // used to mark the end of the table
};
