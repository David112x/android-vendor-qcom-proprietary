/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_qmi_sim.h
  @brief   RIL UTF QMI tests

  DESCRIPTION
   RIL UTF tests go here
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_common_includes.h"
#include "ril_utf_core.h"
#include "ril_utf_test_includes.h"

/*========================================================
  QMI RIL unit test "light" a.k.a UTL
  The unit test framework starts here and performs the
  following:
  - initialize UTL specific params
  - start "main" thread
  - start "rild" thread
  - exectue tests
=========================================================*/
int main()
{

  ril_utf_init();
  start_main_thread();
  start_rild_thread();

  // execute all tests suites
  {
    test_bootup();
  }

  //TODO: add graceful framework shutdown mechanism
  WAIT_INDEFINITELY();

  return 0;
}

