/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_property_if.h
  @brief   Property API for property verification

  DESCRIPTION

  "Inject" should be used for modifying property
  "Expect" should be used for verifying property
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_PROPERTY_IF_H_
#define RIL_UTF_PROPERTY_IF_H_

#include "ril_utf_if.h"


//RIL interfaces extracted from ril_utf_if.h
typedef class ril_utf_property_if : public ril_utf_node_base_t {

public:
  void utf_property_update_property(const char* key, const char* value);
} PropertyTestnode;

#endif /* RIL_UTF_PROPERTY_IF_H_*/

