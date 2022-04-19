/* mct_profile.c
 *
 * This file contains the helper APIs for adding systrace entries
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <unwind.h>
#include <sys/types.h>
#include <inttypes.h>
#include "string.h"
#include "camera_dbg.h"
#include "mct_profiler.h"

#define TRACE_STR_LEN 256

// ============================================================================
// profiler functions
// ============================================================================

/*  mct_profiler_add
 *
 *  Description: This function cats given string plus VA arguments
 *  to one string and calls systrace with this string
 *
 **/
void mct_profiler_add(const char *string, ...)
{
  int digits;
  int length = 0;
  va_list args;
  char *string_next;
  char trace_string[TRACE_STR_LEN];

  va_start(args, string);

  if (strlcpy(trace_string, string, sizeof(trace_string)) >= TRACE_STR_LEN) {
    goto ERROR;
  }
  while ((string_next = va_arg(args, char *))) {
    if (strlcat(trace_string, ", ", sizeof(trace_string)) >= TRACE_STR_LEN)
      goto TRACE;

    if (strlcat(trace_string, string_next, sizeof(trace_string)) >=
      TRACE_STR_LEN)
      goto TRACE;

    length = strlen(trace_string);
    digits = va_arg(args, int);

    if (length < (TRACE_STR_LEN - 16)) {
      snprintf(&trace_string[length], 16, "[%d]", digits);
    }

  TRACE:
    ATRACE_BEGIN(trace_string);

    va_end(args);

  }
  return;
ERROR:
   CLOGE(CAM_MCT_MODULE, "String %s is too long", trace_string);
  return;
}


