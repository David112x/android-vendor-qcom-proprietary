/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_db_if.h
  @brief   sqlite3 db API's for test case dev

  DESCRIPTION

  APIs for querying and modifying QCRIL sqlite3 DB in RIL UTF tests.
  "Inject" should be used for modifying DB (inserting, deleting, etc)
  "Expect" should be used for querying DB
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_DB_IF_H_
#define RIL_UTF_DB_IF_H_

#include "ril_utf_if.h"


//RIL interfaces extracted from ril_utf_if.h
typedef class ril_utf_db_if : public ril_utf_node_base_t {
  void set_mcc(const char* str);
  void set_mnc(const char* str);
  void set_emergency_num(const char* str);
  void set_service_state(const char* str);
  void set_service(const char* str);
  void set_ims_address(const char* str);
  void set_iin(const char* str);
  void set_roam(const char* str);
  void set_property_name(const char* str);
  void set_value(const char* str);
  void set_source(qmi_ril_custom_emergency_numbers_source_type val);
  void set_operator_type(const char* str);
  void set_action(ril_utf_db_action_t val);
  void set_expected_ret(int ret);
  void update_default_db_request();
  void update_default_db_response();
public:
  void utf_db_is_mcc_part_of_emergency_numbers_table
  (
      qmi_ril_custom_emergency_numbers_source_type source,
      const char *mcc,
      const char *emergency_num,
      int expected_ret
  );
  void utf_db_query_operator_type
  (
      const char *mcc,
      const char *mnc,
      char operator_type[QCRIL_DB_MAX_OPERATOR_TYPE_LEN]
  );
  void utf_db_insert_operator_type
  (
      const char *mcc,
      const char *mnc,
      const char operator_type[QCRIL_DB_MAX_OPERATOR_TYPE_LEN],
      int expected_ret
  );
  void utf_db_insert_properties_table
  (
      const char *property_name,
      const char *value,
      int expected_ret
  );
} DBTestnode;

#endif /* RIL_UTF_DB_IF_H_*/
