/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_attributess.h
  @brief   declare all attributes and attribute classes used in RIL UTF

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_ATTRIBUTES_H_
#define RIL_UTF_ATTRIBUTES_H_

#include <string>
#include <list>
#include <algorithm>
#include <stdio.h>
#include <stdint.h>
#include <iostream>

#define UINT32_MAX 0xFFFFFFFF

// structure to match attribute name to attribute type
typedef struct
{
  std::string name;
  int type;
} attributes_table_entry_t;

// enumeration of all possible attributes
enum attributes
{
  ALL = 0,
  DMS,
  VOICE,
  PBM,
  NAS,
  WMS,
  IMSP,
  IMSS,
  IMSA,
  IMS,
  OEM,
  MSIM,
  PDC,
  UIM,
  SMS,
  LTED,
  eMBMS,
  NV,
  cdma_preference,
  pref_network_type,
  Voice_Radio_Tech,
  Voice_Registration_State,
  Data_Registration_State,
  nw_scan,
  Operator_Name,
  screen_state,
  lte_apn_init_attach,
  Property,
  AllowData,
};

int search_for_attribute_type (std::string name);

// attributes for the overall requirement
class user_specified_attrib_t
{
public:
  std::list<std::string> ID_list;
  uint32_t attribute;
  user_specified_attrib_t();
  void setID(std::string ID);
  void setAttribute(std::string str);
};

// attributes in each test case
class test_attribute_t
{
  bool is_attribute_set; // become true if the attribute (not id) is set by test writer
  std::string ID;
  uint32_t attribute;
public:
  test_attribute_t(void);
  test_attribute_t(std::string myTestID);
  void setID(std::string myTestID);
  void setAttribute (std::string str);
  bool isAttributeMatch (user_specified_attrib_t newattr);
};

#endif /* QMI_RIL_TEST_ATTRIBUTES_H_ */
