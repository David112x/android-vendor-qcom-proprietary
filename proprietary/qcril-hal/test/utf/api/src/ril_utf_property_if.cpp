/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_property_if.cpp
  @brief   Property API for property verification

  DESCRIPTION

  "Inject" should be used for modifying property
  "Expect" should be used for verifying property
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_property_if.h"
using namespace std;

void ril_utf_property_if::utf_property_update_property(const char* key, const char* value)
{
  payload = malloc(sizeof(property_node_t));
  payload_len = sizeof(property_node_t);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_property_msg;

  property_node_t* ptr = (property_node_t*) payload;
  strlcpy(ptr->key, key, sizeof(ptr->key));
  strlcpy(ptr->value, value, sizeof(ptr->value));
}
