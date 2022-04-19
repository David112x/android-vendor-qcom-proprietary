/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ril_utf_test_runner.h"
#include "ril_utf_test_if.h"
#include "ril_utf_test_ioe_ril.h"

int main(int argc, char *argv[])
{
  test_runner t;
  if (t.check_arg(argc, argv))
  {
    ioe_utf_init();
    utf_start_ioe_server();
    //usleep(1000);
    user_specified_attrib_t attr;
    attr.setAttribute("ALL");
    test_result ret = ioe_ril_test_bootup_001(attr);
        int passed = ret.test_points_passed;
    int failed = ret.test_points_failed;
    int tot = ret.num_test_points;
    RIL_UTF_ESSENTIAL("@@@@@@@@@@@@@@@@\nQMI_RIL_UTL: Boot up cases\n Total = %d\n Passed = %d \n failed = %d\n @@@@@@@@@@@@@@@@@@@@@@@@@@@", tot, passed, failed);
    ril_utf_connect_to_ioe_server();

    t.execute_tests(ioe_ril_test_table);
    //usleep(1000);


//  TODO: add graceful framework shutdown mechanism
  //WAIT_INDEFINITELY();
  }

  return 0;
}
