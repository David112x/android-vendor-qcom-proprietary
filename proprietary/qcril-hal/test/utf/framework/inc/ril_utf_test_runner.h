/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_runner.cpp
  @brief   declare the class to run all test cases and all member functions

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_TEST_RUNNER_H_
#define RIL_UTF_TEST_RUNNER_H_

#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include "ril_utf_test_if.h"



class test_runner
{
  char* avalue; // value for attributes
  char* ivalue; // value for ID

public:
  // constructor
  test_runner(void);

  // destructor
  ~test_runner(void);

  // check if the arguments satisfies the requirement
  // return true if it does, false otherwise
  bool check_arg(int argc, char* argv[]);

  // execute all test cases according to the test case table
  // must be called after check_arg()
  int execute_tests(func_ptr_t func_ptr_table[]);
  int ril_utf_test_code_shutdown();
};

#endif /* RIL_UTF_TEST_RUNNER_H_ */
