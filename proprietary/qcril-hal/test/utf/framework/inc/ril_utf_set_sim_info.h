/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_set_sim_info.h
  @brief   RIL UTF set sim info functions

  DESCRIPTION
  Used to set sim info for mbn related test cases
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_common_includes.h"
#include "ril_utf_core.h"
#include "qcril_file_utils.h"

void ril_utf_set_sim_info (char *iccid, char *mcc, char *mnc, int slot);

void ril_utf_reset_sim_info();
