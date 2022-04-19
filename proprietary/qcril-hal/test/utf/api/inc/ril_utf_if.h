/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef _RIL_UTF_INTERFACE_H
#define _RIL_UTF_INTERFACE_H

#include <string>

#ifdef QMI_RIL_HAL_UTF
#include "ril_utf_core.h"
#endif
extern "C" {
#ifndef QMI_RIL_HAL_UTF
#include "ril_utf_core.h"
#endif
#include "device_management_service_v01.h"
#include "phonebook_manager_service_v01.h"
#include "network_access_service_v01.h"
#include "wireless_messaging_service_v01.h"
#include "user_identity_module_v01.h"
#include "user_identity_module_http_v01.h"
#ifndef RIL_UTF_IOE
#include "ril_utf_ril_api.h"
#include "ril_utf_qmi_sim.h"
#include "ril_utf_msim_rild.h"
#include "qmi_embms_v01.h"
#include "ril_utf_timed_cb_sim.h"
#include "ril_utf_oem_sim.h"
#endif
#include "ip_multimedia_subsystem_presence_v01.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "ip_multimedia_subsystem_application_v01.h"
#include "ril_utf_message_printer.h"
#include "circuit_switched_video_telephony_v01.h"
#include "card_application_toolkit_v02.h"
#include "data_system_determination_v01.h"
#include "persistent_device_configuration_v01.h"
#include "qcril_db.h"
#include <sqlite3.h>
extern int global_timeout_max;
extern int suspend_timeouts;
}

#include "modules/sms/qcril_qmi_sms.h"

//===============================================================
//  RIL UTF classes
//===============================================================

#define MAX_ID_SIZE 256
#define MAX_FILENAME_SIZE 256
struct source_location_t {
  const char *function;
  const char * file;
  size_t line_number;
};

typedef class ril_utf_node_base_t {
protected:
  void *payload;
  int   payload_len;
  enum ril_utf_node_t node_type; // inject, expect

  enum ril_utf_q_element_t payload_type; // could be ril request/resp, qmi req/resp, qmi ind

  int msg_id; // common for ril/qmi messages
  //enum ril_utf_qmi_srv_id_t qmi_srv_id; // valid only for qmi msgs
  int service_id;
  int client_id; // qmi_client id
  int token_id;
  bool in_exp_set;
  bool in_loop_set;
  bool in_callflow_summary;
  int oem_command_id;
#ifndef RIL_UTF_IOE
  RIL_Errno error; // ATEL message error
  ims_Error ims_error;
  int rild_instance_id;
  void (*my_func)();
#endif
  int milli_sec_timeout; // used for timeout subclass
  int micro_sec_timeout; // used for canary node subclass
  enum ril_utf_qmi_srv_id_t my_service;
  int block;

  std::string description; // used for callflow subclass
  source_location_t location;


  // expectation specific params
  enum ril_utf_expectation_t status;
  struct timeval timeout; // timeout for timed waits
  bool  verify_payload;
  bool is_failure_fatal;

  std::string callflow_tag;

  ril_utf_node_base_t *next;
  ril_utf_node_base_t *prev;

  void set_default_values(void);
  void copy_values_from(const ril_utf_node_base_t&n);

public:

  //ctor
  ril_utf_node_base_t(void);

  //copy ctor : need due to deep copy requirements
  // for payload data
  ril_utf_node_base_t(const ril_utf_node_base_t &n);

  //overload assignment
  ril_utf_node_base_t& operator=(const ril_utf_node_base_t&n);

  //==================================================
  // Class member accessor functions
  //==================================================
  void set_next(ril_utf_node_base_t *n);
  ril_utf_node_base_t * get_next(void);

  void set_prev(ril_utf_node_base_t *n);
  ril_utf_node_base_t * get_prev(void);

  void set_expect_timeout(int msec); //msec
  struct timeval get_expect_timeout(void);

  void set_expect_skip_verification_status(bool status);
  bool  get_expect_skip_verification_status(void);

#ifndef RIL_UTF_IOE
  void set_ril_error_value(RIL_Errno error_val);
  RIL_Errno get_ril_error_value(void);
  void set_ims_error_value(ims_Error error_val);
  ims_Error get_ims_error_value(void);
  void set_rild_instance_id(int instance_id);
  int get_rild_instance_id(void);

  int get_oem_command_id(void);
#endif

  void set_payload_type(enum ril_utf_q_element_t p_t);
  enum ril_utf_q_element_t get_payload_type(void);

  void set_node_type(enum ril_utf_node_t n_t);
  enum ril_utf_node_t get_node_type(void);

  void set_service_id(int new_service_id);
  int get_service_id();

  void set_in_exp_set(bool val);
  bool get_in_exp_set(void);

  void set_in_loop_set(bool val);
  bool get_in_loop_set(void);

  void set_in_callflow_summary(bool val);
  bool get_in_callflow_summary(void);

  void set_client_id(int id);
  int get_client_id(void);

  void set_token_id(int id);
  int get_token_id(void);

  void set_expect_status(enum ril_utf_expectation_t s);
  enum ril_utf_expectation_t get_expect_status();

  void set_msg_id(int id);
  int get_msg_id();

  void *get_payload(void);
  int  get_payload_len();

  void add_callflow_tag(char *tag);
  const char *get_callflow_tag();

  void clear_payload(void);
  void print_expectation(void);
  void print_injection(void);
  void set_verify_payload(bool v);
  bool is_verify_payload(void);
  void set_fatal_on_failure(bool f);
  bool is_fatal_on_failure(void);
  void set_source_location(const source_location_t &location);
  const source_location_t &get_source_location(void);


} Testnode;

//  The following node type is used to add delays during
//  test execution, particularly to test timeouts in test code
typedef class ril_utf_timeout_if : public ril_utf_node_base_t {
public:
  ril_utf_timeout_if(void);
  void set_timeout_value(int milli_sec);
  int get_timeout_value(void);
} TimeoutNode;

//  The following node type is used to verify no messages
//  are outstanding and unhandled at the end of the test
typedef class ril_utf_canary_if : public ril_utf_node_base_t {
public:
  ril_utf_canary_if(void);
  void set_timeout_micro(int micro_sec);
  int get_timeout_micro(void);
} CanaryNode;

//  The following node type is used to replace printing of
//  messages in a callflow with a summary
typedef class ril_utf_callflow_summary_if : public ril_utf_node_base_t {
public:
  ril_utf_callflow_summary_if(void);
  void set_description(char *desc);
  const char *get_description();
} CallflowSummary;

//  The following node type is used to block qmi_client_init for a particular service
typedef class ril_utf_block_client_init_if : public ril_utf_node_base_t {
public:
  ril_utf_block_client_init_if(void);
  void set_blocked_service(enum ril_utf_qmi_srv_id_t service);
  enum ril_utf_qmi_srv_id_t get_blocked_service();
  void set_block_state(int state);
  int get_block_state();
} BlockClientInitNode;

// Container for unordered set of expect nodes
struct ril_utf_expect_set_q_t {
  ril_utf_node_base_t *test_step;
  ril_utf_expect_set_q_t *prev;
  ril_utf_expect_set_q_t *next;
};

#endif
