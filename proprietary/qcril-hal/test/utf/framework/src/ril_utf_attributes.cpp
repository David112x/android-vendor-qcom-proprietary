/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_attributess.h
  @brief   define all attributes and attribute class functions used in RIL UTF

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_attributes.h"

using namespace std;

/*========================================================
 * Array to match attribute name to attribute type
========================================================*/
attributes_table_entry_t attributes_type_from_name[] =
{
  { "ALL", ALL},
  { "DMS", DMS },
  { "VOICE", VOICE },
  { "PBM", PBM },
  { "NAS", NAS },
  { "WMS", WMS },
  { "IMSP", IMSP },
  { "IMSS",IMSS },
  { "IMSA", IMSA },
  { "IMS", IMS },
  { "OEM", OEM },
  { "MSIM", MSIM },
  { "PDC", PDC },
  { "UIM", UIM },
  { "SMS", SMS },
  { "LTED", LTED },
  { "eMBMS", eMBMS },
  { "NV", NV },
  { "cdma_preference", cdma_preference },
  { "pref_network_type", pref_network_type },
  { "Voice_Radio_Tech", Voice_Radio_Tech },
  { "Voice_Registration_State", Voice_Registration_State },
  { "Data_Registration_State", Data_Registration_State },
  { "nw_scan", nw_scan },
  { "Operator_Name", Operator_Name },
  { "screen_state", screen_state },
  { "lte_apn_init_attach", lte_apn_init_attach },
  { "Property", Property },
  { "AllowData", AllowData },
};

/*========================================================
 * Search attribute type by attribute name from the table
========================================================*/
int search_for_attribute_type(string name)
{
  int attributeNum = sizeof(attributes_type_from_name) / sizeof(attributes_table_entry_t);
  for (int i = 0; i < attributeNum; ++i)
  {
    if (attributes_type_from_name[i].name == name)
    {
      return attributes_type_from_name[i].type;
    }
  }
  // if not found, return -1 for error
  return -1;
}

/*========================================================
 * Attribute is an uint32_t integer (big endian)
 * bit 1 (rightmost bit) stands for "all attributes"
 * Each bit stands for an attribute, starting from bit 2
 * Which bit for what attribute are defined in the attributes enumeration
 * ex. 00000001 stand for "all attributes"
 *     00010010 stand for attribute 2 and 5, which is "DMS" and "WMS"
 * In this way, if (attribute1|attribute2 != 0) then the two attribute list
 *    will have some common attributes
========================================================*/

/*========================================================
 * Constructor for user specified attribute
 * all attributes bit set to 1 by default, since we need to do the & operation,
 * it has to be set to 1 to match the "all attribute" case
========================================================*/
user_specified_attrib_t::user_specified_attrib_t()
{
  attribute = 0;
}

/*========================================================
 * Used to set the user specified ID
========================================================*/
void user_specified_attrib_t::setID(string ID)
{
  ID_list.push_back(ID);
}

/*========================================================
 * Used to set the user specified attribute
========================================================*/
void user_specified_attrib_t::setAttribute(string str)
{
  int ret = search_for_attribute_type(str);
  if (ret > 0)
  {
    attribute |= (1 << (uint32_t)ret);
  }
  // if "all attribute" bit is set by the user
  // start set all bits to 1
  else if (ret == 0)
  {
    attribute = (uint32_t)UINT32_MAX;
  }
  else
  {
    printf("Error, attribute doesn't match anything from table\n");
    exit(0);
  }
}

/*========================================================
 * Constructor for attribute class in test cases
========================================================*/
test_attribute_t::test_attribute_t(void)
{
  ID = "";
  attribute = 1;
  is_attribute_set = false;
}

/*========================================================
 * Constructor for attribute class in test cases
 * Take ID as input
========================================================*/
test_attribute_t::test_attribute_t(string myTestID)
{
  ID = myTestID;
  attribute = 1;
  is_attribute_set = false;
}

/*========================================================
 * Used to set the ID of test cases
========================================================*/
void test_attribute_t::setID(string myTestID)
{
  ID = myTestID;
}

/*========================================================
 * Used to set the attributes of test cases
 * - If no attribute set, set the "all attribute" bit to 1
 *   by default (which is done by the constructor)
 * - Else, set that bit to 0, and set corresponding attributes
========================================================*/
void test_attribute_t::setAttribute (string str)
{
  // if attribute hasn't been set yet, mark the flag to true,
  // and then initialize all bits to 0
  if (!is_attribute_set)
  {
    is_attribute_set = true;
    attribute = 0;
  }

  // search for attributes from the table
  uint32_t ret = search_for_attribute_type(str);
  if (ret > 0) {
    attribute |= (1 << ret);
  }
  else if (ret == 0)
  {
    attribute = (uint32_t)UINT32_MAX;
  }
  else
  {
    printf("Error, attribute doesn't match anything from table\n");
  }
}

/*========================================================
 * Used to see if the attributes or ID in a test case match
 * any of the user specified test cases
 * The logic is as followed:
 * - If user specifies ID, only compare ID
 * - If user hasn't specified ID, then compare the attributes
========================================================*/
bool test_attribute_t::isAttributeMatch (user_specified_attrib_t newattr)
{
  // if this test case need to run all the time
  if (attribute == (uint32_t)UINT32_MAX) {
    return true;
  }

  // No ID or attributes set by user
  if (newattr.ID_list.size() == 0 && newattr.attribute == 0) {
    return true;
  }

  // ID is set by user, compare ID
  if (newattr.ID_list.size()) {
    list<string>::iterator iter = find(newattr.ID_list.begin(), newattr.ID_list.end(), ID);
    if (iter != newattr.ID_list.end()) {
      return true;
    }
  }

  // Compare attribute
  if ((newattr.attribute & attribute) != 0) {
    return true;
  }

  // ID or attribute does not match; return false
  return false;
}
