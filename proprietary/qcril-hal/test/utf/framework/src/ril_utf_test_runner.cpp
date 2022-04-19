/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_runner.cpp
  @brief   define all functions for the outer class to run all test cases

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include <queue>
#include <algorithm>
#include <ctime>
#include <sys/time.h>
#include <locale>
#include "ril_utf_test_runner.h"
#include "ril_utf_test_if.h"
#ifndef RIL_UTF_IOE
#include "ril_utf_test_qmi_ril.h"
#endif

extern int GEN_CALL_FLOW;
extern int shutdown_after_execution;
extern int is_boot_up;

using namespace std;


/*========================================================
 * constructor for test_runner class
 * avalue is for the attributes
 * ivalue is for the test id
========================================================*/
test_runner::test_runner()
{
  // Init the output file
  utf_log_file = stdout;

  avalue = NULL;
  ivalue = NULL;
}

/*========================================================
 * destructor for test_runner class
========================================================*/
test_runner::~test_runner()
{
  if (avalue != NULL)
  {
    delete[] avalue;
  }
  if (ivalue != NULL)
  {
    delete[] ivalue;
  }
}

/*========================================================
 * check if arguments are legal
 * supported flags are -a, -i, -f and -h
 * if yes, return true, and store the parsed avalue and ivalue
 * else, return false
========================================================*/
bool test_runner::check_arg(int argc, char* argv[])
{
  ifstream fs;
  string temp_str;
  char* buffer;
  if (argc < 2)
  {
    RIL_UTF_ESSENTIAL ("UTF supports command line arguments.\nBy default it runs all the test cases.\nSee 'test -h' for help.\n");
    return true;
  }

  // we assume the one flag match one argument, so argc %2 must equal to 1, except for -h
  if (((argv[1][1] != 'h') && (argv[1][1] != 'g')) && (argc % 2 != 1))
  {
    RIL_UTF_ESSENTIAL("Illegal argument format.\nSee 'test -h' for help.\n");
    return false;
  }

  for (int i = 1; i < argc; i+=2)
  {
    switch (argv[i][1])
    {
    case 'a':
      avalue = new char[(strlen(argv[i+1])+1)]();
      memcpy(avalue, argv[i+1], strlen(argv[i+1]));
      break;
    case 'i':
      ivalue = new char[(strlen(argv[i+1])+1)]();
      memcpy(ivalue, argv[i+1], strlen(argv[i+1]));
      break;
    case 'g':
      GEN_CALL_FLOW = TRUE;
      break;
    case 'f':
      {
        fs.open(argv[i+1]);
        char *tmp = NULL;
        char *lastPtr = NULL;
        getline(fs, temp_str);
        tmp = strdup(temp_str.c_str());
        fs.close();
        buffer = strtok_r(tmp, " ", &lastPtr);
        while (buffer != NULL) {
          if (strcmp(buffer, "-a") == 0)
          {
            buffer = strtok_r(NULL, " ", &lastPtr);
            avalue = new char[(strlen(buffer)+1)]();
            memcpy(avalue, buffer, strlen(buffer));
          }
          else if (strcmp(buffer, "-i") == 0)
          {
            buffer = strtok_r(NULL, " ", &lastPtr);
            ivalue = new char[(strlen(buffer)+1)]();
            memcpy(ivalue, buffer, strlen(buffer));
          }
          else
          {
            printf ("Illegal argument format in file.\nSee 'test -h' for help.\n");
            return false;
          }
          buffer = strtok_r(NULL, " ", &lastPtr);
        }
      }
      break;
    case 'h':
      RIL_UTF_ESSENTIAL ("\nUsage: test [OPTION1] args1 [OPTION2] args2\n\n");
      RIL_UTF_ESSENTIAL ("\t-g (this signals to UTF to generate call flows in the ./doc folder and not run tests \n");
      RIL_UTF_ESSENTIAL ("Available options:\n");
      RIL_UTF_ESSENTIAL ("\t-a: attributes\n");
      RIL_UTF_ESSENTIAL ("\t-i: IDs\n");
      RIL_UTF_ESSENTIAL ("\t-f: filename\n");
      RIL_UTF_ESSENTIAL ("\nExamples:\n");
      RIL_UTF_ESSENTIAL ("\ttest -a NAS,VOICE\n");
      RIL_UTF_ESSENTIAL ("\ttest -i 1,3,5\n");
      RIL_UTF_ESSENTIAL ("\ttest -i 1,2 -a DMS,WMS\n");
      RIL_UTF_ESSENTIAL ("\ttest -f input.txt\n");
      RIL_UTF_ESSENTIAL ("\nNo space allowed before and after comma.\n"
          "The content in the file should be the same as the input from the command line.\n"
          "See framework/src/ril_utf_attributes.h for all the possible attributes.\n\n");
      return false;
      break;
    default:
      RIL_UTF_ESSENTIAL("Illegal argument format.\nSee 'test -h' for help.\n");
      return false;
    }
  }
  return true;
}

/*========================================================
 * executed after the argument check is successful
 * run all the test cases defined in the function pointer table
========================================================*/
int test_runner::execute_tests(func_ptr_t func_ptr_table[])
{
  user_specified_attrib_t attr;
  queue<test_result> results;
  int tests_passed = 0;
  int tests_failed = 0;
  double total_tests_runtime = 0.0;

  // set attribute
  char* a_buffer;

  utf_log_file = nullptr;

#ifndef UTF_TARGET_BUILD
  // Initialize Properties for first test
  #ifndef RIL_UTF_IOE
  utf_property_init();
  #endif
#endif

  char *lastPtr = NULL;

  // parse the attribute value and store them as attributes
  // since attributes are separated by ',' without space
  a_buffer = strtok_r(avalue, ",", &lastPtr);
  while (a_buffer != NULL)
  {
    string tmp = a_buffer;
    // transform all attribute names into upper case
    // transform(tmp.begin(), tmp.end(), tmp.begin(), std::toupper);  // Not currently functional need to reimplement

    // set attribute
    attr.setAttribute(tmp);

    // parse the next attribute
    a_buffer = strtok_r(NULL, ",", &lastPtr);
  }

  // parse id in similar way
  char* i_buffer = strtok_r(ivalue, ",", &lastPtr);
  while (i_buffer != NULL)
  {
    string tmp = i_buffer;
    attr.setID(tmp);
    i_buffer = strtok_r(NULL, ",", &lastPtr);
  }

  // execute all the test cases defined in the function pointer table
  int i;
  for (i = 0; func_ptr_table[i] != NULL; ++i)
  {

    if (GEN_CALL_FLOW == TRUE)
    {
      // Only generate call flow diagram
      (func_ptr_table[i])(attr);
    }
    else
    {
      //// Execute test ////
      struct timeval start_time, end_time, diff;
      gettimeofday(&start_time, NULL);

      // Run test
      test_result ret = (func_ptr_table[i])(attr);

      if (ret.status == STATUS_COMPLETED) {
        gettimeofday(&end_time, NULL);
        timersub(&end_time, &start_time, &diff);
        ret.sec_execution = diff.tv_sec + (diff.tv_usec / 1000000.0);
        results.push(ret);
      }

      // Shutdown test code
      if (shutdown_after_execution) {
        ril_utf_test_code_shutdown();
        is_boot_up = 0;
        // Reinitialize property set
        utf_property_init();
      }
    }
  }
  if (!shutdown_after_execution) {
    ril_utf_test_code_shutdown();
    is_boot_up = 0;
  }
  utf_log_file = stdout;

  if (GEN_CALL_FLOW == TRUE)
  {
    RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    RIL_UTF_ESSENTIAL("\n      QMI RIL UTF test callflow diagrams generated");
    RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

    RIL_UTF_ESSENTIAL("\n   Please consult the ./doc directory for output .pngs");
    RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
  }
  else
  {
    string cond;

    RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    RIL_UTF_ESSENTIAL("\n                  QMI RIL UTF tests");
    RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

    RIL_UTF_ESSENTIAL("\n%-70s\t%-5s\t%-4s\t%-4s\t%-4s\t%-15s",
                      "----------Test ID----------", "Result", "Passed", "Failed", "Total", "Exec time (sec)");

    while ( results.empty() == false )
    {
      test_result ret = results.front();
      results.pop();
      int passed = ret.test_points_passed;
      int failed = ret.test_points_failed;
      int tot = ret.num_test_points;
      double exec_time = ret.sec_execution;
      string id = ret.test_id;

      // Add to pass / fail talley
      if ( failed == 0 )
      {
        cond = "PASS";
        ++tests_passed;
      }
      else
      {
        cond = "FAIL";
        ++tests_failed;
      }
      // Add to total timer
      total_tests_runtime += exec_time;

      RIL_UTF_ESSENTIAL("\n%-70s\t%-6s\t%-6d\t%-6d\t%d\t%-6.3f%-s",
                         id.c_str(), cond.c_str(), passed, failed, tot,
                         exec_time, (exec_time > 0.5) ? "***" : "");
    }

    RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    RIL_UTF_ESSENTIAL("\n%d Testcases PASSED\t\t\tTotal runtime: %.3f sec", tests_passed, total_tests_runtime);
    if ( tests_failed != 0 )
    {
      RIL_UTF_ESSENTIAL("\n%d Testcases FAILED (please see errors above)", tests_failed);
    }
    RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
  } // end GEN_CALL_FLOW

  if (utf_log_file) {
    fclose(utf_log_file);
  }

  return tests_failed;
}


int test_runner::ril_utf_test_code_shutdown()
{
  #ifndef RIL_UTF_IOE
  #ifndef UTF_TARGET_BUILD
  // dump all logging from shutdown (not needed)
  pthread_mutex_lock(&ril_utf_log_lock_mutex);
  if (utf_log_file) {
    fclose(utf_log_file);
    utf_log_file = nullptr;
    //utf_log_file = fopen("/dev/null", "w");
  }
  pthread_mutex_unlock(&ril_utf_log_lock_mutex);
  #endif

  // SHUTDOWN QMI_RIL
  qmi_ril_utf_shutdown();

#ifndef UTF_TARGET_BUILD
  // Close log file
  pthread_mutex_lock(&ril_utf_log_lock_mutex);
  //fclose(utf_log_file);
  //utf_log_file = stdout;
  pthread_mutex_unlock(&ril_utf_log_lock_mutex);
#endif
  #endif

  return 0;
}

