/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef _RIL_UTF_TEST_IF
#define _RIL_UTF_TEST_IF

#include <string>
#include <iostream>
#include <time.h>
#include <map>
#include "ril_utf_if.h"
#include "ril_utf_func_exec_if.h"
#include "ril_utf_attributes.h"

enum ril_utf_test_type_t {
  REGRESSION = 0x01
};

enum ril_utf_test_result_status_t {
  STATUS_INVALID = 0x00,
  STATUS_COMPLETED = 0x01,
};

struct test_result {
  ril_utf_test_result_status_t status;
  std::string test_id;
  std::string test_description;
  int num_test_points;
  int test_points_passed;
  int test_points_failed;
  double sec_execution;
  test_result() : test_id(""), test_description(""), status(STATUS_INVALID) {}
};

typedef test_result (*func_ptr_t)(user_specified_attrib_t);
extern func_ptr_t qmi_ril_test_table[];
extern func_ptr_t qmi_ril_target_test_table[];
extern func_ptr_t qmi_proxy_test_table[];
extern func_ptr_t ioe_ril_test_table[];

typedef class ril_utf_test_t {
  ril_utf_node_base_t *head;
  ril_utf_node_base_t *tail;
  enum ril_utf_test_type_t type; //TBD:
  int points_passed;
  int points_failed;
  bool in_exp_set; // marks whether testset execution is in unordered expect set
  bool in_callflow_summary;

  // Boot test case (should be null for boot tests)
  test_result (*boot_case)();
  int (*cleanup_case)();
  // Loop set is used to poll for a status Ex: network registration state on target
  bool in_loop_set;
  ril_utf_node_base_t *start_loop;
  bool loop_fail;


  //int curr_ril_id;
  std::map<int, int> outstanding_ril_tokens;
  int curr_oem_id;
  int curr_ioe_id;
  std::map<int, int> outstanding_ims_tokens;

  // to record the test point number in each test
  int testPointNum;

  // used for the input attribute and the return value;
  test_attribute_t attr;
  test_result result;

  bool clear_db;
  void add_node( ril_utf_node_base_t& node);
  int remove_list_node(struct ril_utf_expect_set_q_t* i,
                       struct ril_utf_expect_set_q_t* &head_ptr,
                       struct ril_utf_expect_set_q_t* &tail_ptr);
  int clear_list(struct ril_utf_expect_set_q_t* &head_ptr,
                 struct ril_utf_expect_set_q_t* &tail_ptr);
  int gen_call_flow_diagram(ril_utf_node_base_t* &test_step);
  int handle_loop_set(struct ril_utf_node_base_t* &test_step);
  int handle_expect_step( struct ril_utf_node_base_t* &test_step,
                          int &current_expect_slot);
  bool handle_unordered_exp_set( struct ril_utf_node_base_t* &test_step,
                                struct ril_utf_msg_t *ril_utf_exp_ptr);
  void find_unordered_match( struct ril_utf_node_base_t* &curr_node,
                             struct ril_utf_msg_t *ril_utf_exp_ptr,
                             bool &found_match);
  void move_unordered_exp_set( struct ril_utf_node_base_t* &test_step,
                               struct ril_utf_node_base_t* &curr_node);
  void handle_canary_timeout(int micro_sec, int &current_expect_slot);
  int validate_expect ( ril_utf_node_base_t *exp_node,
                        struct ril_utf_msg_t *data );
  void validate_ims( ril_utf_node_base_t *exp_node,
                     struct ril_utf_msg_t *data);
  void validate_oem( ril_utf_node_base_t *exp_node,
                     struct ril_utf_msg_t *data);
  void validate_ioe_response( ril_utf_node_base_t *exp_node,
                            struct ril_utf_msg_t *data);
  void validate_ioe_indication( ril_utf_node_base_t *exp_node,
                                struct ril_utf_msg_t *data);
  void validate_ims_last_call_fail_cause( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_registration_state( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_srv_status_update( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_mwi( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_get_current_calls( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  bool validate_ims_sip_error_info_util( ims_SipErrorInfo &expect,
                                         ims_SipErrorInfo &recv);
  void validate_ims_sip_error_info_msg( ril_utf_node_base_t *exp_node,
                              struct ril_utf_msg_t *data);
  void validate_ims_get_colr( ril_utf_node_base_t *exp_node,
                              struct ril_utf_msg_t *data);
  void validate_ims_query_call_forward_status( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_query_call_waiting( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_sub_config(ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_query_virtual_line_info(ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_voice_info(ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_multi_identity_registration_status(ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_imsa_geo_loc( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_imsa_vowifi_call_quality( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_request_query_clip_resp( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_config_msg( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_refresh_conf_info ( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_refresh_vice_info ( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_modify_call ( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_supp_svc_notification ( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_response_handover ( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_request_supp_svc_status_resp( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_set_call_forward_status_resp( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_radio_state_changed( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_network_state_changed( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_unsol_participant_status_info( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_wfc_roaming_config_support( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_ussd_failed( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_unsol_incoming_ims_sms( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  bool validate_ims_info_struct_list(ims_Info **exp_info, ims_Info **recv_info);
  void validate_imss_rtt_message( ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_registration_block_status(ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);
  void validate_ims_unsol_auto_call_rejection_ind(ril_utf_node_base_t *exp_node,
      struct ril_utf_msg_t *data);

  void validate_lte_direct_disc_message(ril_utf_node_base_t *exp_node,
                     struct ril_utf_msg_t *data);
  enum ril_utf_expectation_t validate_lte_direct_disc_device_capability(
                     void *exp,
                     void *recv);
  enum ril_utf_expectation_t validate_lte_direct_disc_service_status(
                     void *exp,
                     void *recv);
  enum ril_utf_expectation_t validate_lte_direct_disc_unsol_authorization_result(
                     void *exp,
                     void *recv);
  enum ril_utf_expectation_t validate_lte_direct_disc_unsol_expression_status(
                     void *exp,
                     void *recv);
  enum ril_utf_expectation_t validate_lte_direct_disc_unsol_transmission_status(
                     void *exp,
                     void *recv);
  enum ril_utf_expectation_t validate_lte_direct_disc_unsol_reception_status(
                     void *exp,
                     void *recv);
  enum ril_utf_expectation_t validate_lte_direct_disc_unsol_match_event(
                     void *exp,
                     void *recv);

  void validate_qmi_request( ril_utf_node_base_t *exp_node,
                     struct ril_utf_msg_t *data);
  void validate_qmi_response( ril_utf_node_base_t *exp_node,
                     struct ril_utf_msg_t *data);
  void validate_qmi_indication( ril_utf_node_base_t *exp_node,
                     struct ril_utf_msg_t *data);
  void validate_ril_unsol( ril_utf_node_base_t *exp_node,
                           struct ril_utf_msg_t *data);
  void validate_ril_resp( ril_utf_node_base_t *exp_node,
                           struct ril_utf_msg_t *data);
  void validate_msim_recv( ril_utf_node_base_t *exp_node,
                           struct ril_utf_msg_t *data);
  void verify_ril_ims_registration_state(ril_utf_node_base_t *exp_node,
                                     struct ril_utf_msg_t *data);
  int handle_inject_step( struct ril_utf_node_base_t* &test_step );
  struct ril_utf_msg_t* update_exp_ptr(struct ril_utf_node_base_t *test_step );
  struct ril_utf_msg_t* update_inj_ptr(struct ril_utf_node_base_t *test_step );
  void clear_exec_q(void);
  void append_exec_q(ril_utf_node_base_t *prev, ril_utf_node_base_t *src_p);
  void point_pass(void);
  void point_fail(void);

  public:

  //ctor
  ril_utf_test_t(void);

  ril_utf_test_t& operator=(const ril_utf_test_t& t);
  ril_utf_test_t operator+(ril_utf_test_t& t);

  //dtor
  ~ril_utf_test_t(void);

  void _inject( ril_utf_node_base_t& node, const source_location_t &location);
  void _expect( ril_utf_node_base_t& node, const source_location_t &location);
  void sleep( int milli_sec );
  void block_client_init(enum ril_utf_qmi_srv_id_t service);
  void unblock_client_init();
  void add_canary( ril_utf_canary_if& node );
  void add_func_exec( void (*func)() );
  void execute (void);
  void execute (void(*update_globals)(void));
  void set_start(void);
  void set_end(void);
  void loop_start(void);
  void loop_end(void);
  void callflow_summary_start();
  void callflow_summary_end(char *summary);
  void print_summary(void);
  void set_boot_case(test_result (*func)() );
  void set_cleanup_case(int (*func)());

  void set_test_id(std::string id);
  std::string get_test_id(void);
  void set_test_description(std::string desc);
  std::string get_test_description(void);
  void set_test_type(enum ril_utf_test_type_t t);
  void set_test_attribute(std::string attribute);
  bool isAttributeMatch (user_specified_attrib_t newattr);

  test_result get_test_result(void);

  void print_received_message(ril_utf_msg_t* msg_ptr);
  void print_test_point(void);
  void clear_db_before_execution(void);
  void reset_mbn_sim_info(void);
  void set_mbn_sim_info(char *iccid, char *mcc, char *mnc, int slot);
  void verify_ril_voice_registration_state(ril_utf_node_base_t *exp_node,
                                 struct ril_utf_msg_t *data);
  void verify_ril_data_registration_state(ril_utf_node_base_t *exp_node,
                                 struct ril_utf_msg_t *data);
  void validate_dsi_request_response( ril_utf_node_base_t *exp_node, struct ril_utf_msg_t *data);

} Testcase;
#define inject(node) _inject(node, {__func__, __FILE__, __LINE__})
#define expect(node) _expect(node, {__func__, __FILE__, __LINE__})

#endif //_RIL_UTF_TEST_IF
