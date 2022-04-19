/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "ril_utf_test_runner.h"
#include "ril_utf_test_if.h"
#include "ril_utf_test_qmi_ril.h"
#include "ril_utf_target_test_qmi_ril.h"
#include <sanitizer/lsan_interface.h>

int main(int argc, char *argv[])
{
  int ret_val = 0;
  test_runner t;
  if (t.check_arg(argc, argv))
  {

#ifdef UTF_TARGET_BUILD
  ret_val = t.execute_tests(qmi_ril_target_test_table);
#else
  ret_val = t.execute_tests(qmi_ril_test_table);
#endif


  //WAIT_INDEFINITELY();
  }
#ifndef UTF_TEST_COVERAGE
  __lsan_do_leak_check();
#endif
  return ret_val;
}
