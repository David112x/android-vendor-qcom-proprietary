/******************************************************************************
#  Copyright (c) 2015 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ims_payload.cpp
  @brief   Message print functions and validation for IMS messages

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
extern "C" {
#include "ril_utf_ims_payload.h"
#include <string.h>
}

void print_ims_CallList_Call (ims_CallList_Call *call)
{
  RIL_UTF_ESSENTIAL("\nIMS CALL DUMP:");
  RIL_UTF_ESSENTIAL("\nhas_state %");
}
