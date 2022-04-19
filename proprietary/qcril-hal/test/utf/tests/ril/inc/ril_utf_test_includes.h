/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_includes.h
  @brief   RIL UTF test function includes

  DESCRIPTION
  Includes all tests written in the test sourc files in src folder.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_TEST_INCLUDES_H
#define _RIL_UTF_TEST_INCLUDES_H
#ifdef QMI_RIL_HAL_UTF
#include "ril_utf_core.h"
#endif

extern "C" {
#ifndef QMI_RIL_HAL_UTF
#include "ril_utf_core.h"
#endif
#include "ril_utf_rild_sim.h"
#include "ril_utf_set_sim_info.h"
}

#include "ril_utf_test_if.h"
#include "ril_utf_qmiril_if.h"
#include "ril_utf_androidTel_if.h"
#include "ril_utf_oem_if.h"
#include "ril_utf_ims_if.h"
#include "ril_utf_msim_if.h"
#include "ril_utf_db_if.h"
#include "ril_utf_ssr_if.h"
#include "ril_utf_property_if.h"
#include "ril_utf_utility_if.h"
#include "ril_utf_qmi_msg_analyzer.h"

#ifdef QMI_RIL_HAL_UTF
extern "C" int property_set(const char *key, const char *value);
#endif
#endif //_RIL_UTF_TEST_INCLUDES_H
