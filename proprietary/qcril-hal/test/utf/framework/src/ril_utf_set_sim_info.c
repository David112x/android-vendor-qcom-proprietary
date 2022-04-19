/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_set_sim_info.c
  @brief   RIL UTF set sim info functions

  DESCRIPTION
  Used to set sim info for mbn related test cases
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_set_sim_info.h"
#ifndef RIL_UTF_SET_SIM_INFO_H
#define RIL_UTF_SET_SIM_INFO_H

#define RIL_UTF_SIM_INFO_FILE "data/vendor/misc/radio/iccid"

void ril_utf_set_sim_info (char *iccid, char *mcc, char *mnc, int slot)
{
  char file_name[100];
  char final_string[100];

  snprintf(file_name, sizeof(file_name),
                 "%s_%d", RIL_UTF_SIM_INFO_FILE, slot);
  snprintf(final_string, sizeof(final_string),
                 "%s,%s,%s",
                 iccid? iccid: "null",
                 mcc? mcc: "null",
                 mnc? mnc: "null");
  qcril_file_write_a_string_to_file(file_name, final_string);
  RIL_UTF_DEBUG("file : %s", file_name);
  RIL_UTF_DEBUG("final_string : %s", final_string);
}

void ril_utf_reset_sim_info()
{
  ril_utf_set_sim_info("8900310","310","480",0);
  ril_utf_set_sim_info("8900310","310","480",1);
}

#endif
