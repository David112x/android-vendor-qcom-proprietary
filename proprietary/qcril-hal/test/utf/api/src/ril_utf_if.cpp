/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ril_utf_if.h"
#ifdef QMI_RIL_HAL_UTF
#include "qcril_qmi_oem_reqlist.h"
#endif
using namespace std;


/*========================================================
  FUNCTION: ril_utf_node_base_t
  - default constructor
========================================================*/
ril_utf_node_base_t::ril_utf_node_base_t(void)
{
 set_default_values();
}

/*========================================================
  FUNCTION: ril_utf_node_base_t
  - deep copies the payload data and updates all other
    data from provided object
========================================================*/
void ril_utf_node_base_t::copy_values_from(const ril_utf_node_base_t&n)
{
  payload_len = n.payload_len;
  if(n.payload_len == 0 || n.payload == NULL)
  {
    payload_len = 0;
    payload = NULL;
  }
  else
  {
    payload = (void *) calloc(1,payload_len+1);
    memcpy(payload, n.payload, payload_len);
  }
  payload_type = n.payload_type;
  node_type = n.node_type;
  msg_id = n.msg_id;
  service_id = n.service_id;
  client_id = n.client_id;
  status = n.status;
  token_id = n.token_id;
  location = n.location;
#ifndef RIL_UTF_IOE
  error = n.error;
  ims_error = n.ims_error;
  oem_command_id = n.oem_command_id;
  rild_instance_id = n.rild_instance_id;
  my_func = n.my_func;
  my_service = n.my_service;
  block = n.block;
#endif
  in_exp_set = n.in_exp_set;
  in_loop_set = n.in_loop_set;
  in_callflow_summary = n.in_callflow_summary;
  is_failure_fatal = n.is_failure_fatal;
  callflow_tag = n.callflow_tag;
  milli_sec_timeout = n.milli_sec_timeout;
  micro_sec_timeout = n.micro_sec_timeout;
  description = n.description;
  verify_payload = n.verify_payload;//default: don't skip
  next = NULL;
  prev = NULL;
  timeout = n.timeout;
}

/*========================================================
  FUNCTION: ril_utf_node_base_t
  - copy constructor
  - deep copies the payload data.
========================================================*/
ril_utf_node_base_t::ril_utf_node_base_t(const ril_utf_node_base_t&n)
{
 copy_values_from(n);
}

/*========================================================
  FUNCTION: operator=
  - overloaded = operator
  - deep copies the payload data.
========================================================*/
ril_utf_node_base_t& ril_utf_node_base_t::operator=(const ril_utf_node_base_t&n)
{
  copy_values_from(n);
  return *this;
}

/*========================================================
  FUNCTION: set_default_values
  - set default values for the node
========================================================*/
void ril_utf_node_base_t::set_default_values(void)
{
  payload = NULL;
  payload_len = 0;
  node_type = INJECT;
  payload_type = ril_utf_ril_request;
  msg_id = 0;
  service_id = 0;
  client_id = 0;
  token_id = -1;
  status = not_met;
  verify_payload = true;//default: don't skip
#ifndef RIL_UTF_IOE
  error = RIL_E_SUCCESS;
  ims_error = ims_Error_E_SUCCESS;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_REQ_GENERIC;
#else
  oem_command_id = QCRIL_EVT_HOOK_REQ_GENERIC;
#endif
  rild_instance_id = 0;
#endif
  in_exp_set = false;
  in_loop_set = false;
  in_callflow_summary = false;
  is_failure_fatal = true;
  callflow_tag = "";
  next = NULL;
  prev = NULL;

  if( !suspend_timeouts )
  {
    set_expect_timeout(400); // default 40 msec
  }
  else
  {
    set_expect_timeout(global_timeout_max * MICROSECONDS);
  }

}
/*========================================================
  FUNCTION: get/set_expect_verification_status
  - if verification status set to 1, then

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_expect_skip_verification_status(bool status)
{
  verify_payload = status;
}

bool ril_utf_node_base_t::get_expect_skip_verification_status(void)
{
  return verify_payload;
}

#ifndef RIL_UTF_IOE
/*========================================================
  FUNCTION: get/set_ril_error_value

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_ril_error_value(RIL_Errno error_val)
{
  error = error_val;
}
RIL_Errno ril_utf_node_base_t::get_ril_error_value(void)
{
  return error;
}
/*========================================================
  FUNCTION: get/set_ims_error_value

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_ims_error_value(ims_Error error_val)
{
  ims_error = error_val;
}
ims_Error ril_utf_node_base_t::get_ims_error_value(void)
{
  return ims_error;
}

/*========================================================
  FUNCTION: get/set_rild_instance_id

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_rild_instance_id(int instance_id)
{
  rild_instance_id = instance_id;
}
int ril_utf_node_base_t::get_rild_instance_id(void)
{
  return rild_instance_id;
}

/*========================================================
  FUNCTION: get_oem_command_id

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
int ril_utf_node_base_t::get_oem_command_id(void)
{
  return oem_command_id;
}
#endif
/*========================================================
  FUNCTION: clear_payload
  - clears the payload, releases memory and sets len to 0

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::clear_payload(void)
{
  free(payload);
  set_default_values();
}

/*========================================================
  FUNCTION: set_next
  - update the next ptr of node

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_next(ril_utf_node_base_t *n)
{
  next = n;
}

ril_utf_node_base_t * ril_utf_node_base_t::get_next(void)
{
  return next;
}

/*========================================================
  FUNCTION: set_prev
  - update the prev ptr of node

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_prev(ril_utf_node_base_t *n)
{
  prev = n;
}
ril_utf_node_base_t * ril_utf_node_base_t::get_prev(void)
{
  return prev;
}

/*========================================================
  FUNCTION: set_expect_timeout
  - update the next ptr of node

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_expect_timeout(int msec)
{
  int sec = msec / MICROSECONDS;

  timeout.tv_sec = sec;
  timeout.tv_usec = msec * 1000;
}

struct timeval ril_utf_node_base_t::get_expect_timeout(void)
{
  return timeout;
}

/*========================================================
  FUNCTION: set/get_payload_type
  - set the payload type to one of the following
        ril_utf_ril_request,
        ril_utf_ril_response,
        ril_utf_qmi_request,
        ril_utf_qmi_response,
        ril_utf_qmi_indication

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_payload_type(enum ril_utf_q_element_t p_t)
{
  payload_type = p_t;
}

enum ril_utf_q_element_t ril_utf_node_base_t::get_payload_type(void)
{
  return payload_type;
}

/*========================================================
  FUNCTION: set/get_node_type
  - Node type can be either inject or expect.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_node_type(enum ril_utf_node_t n_t)
{
  node_type = n_t;
}

enum ril_utf_node_t ril_utf_node_base_t::get_node_type(void)
{
  return node_type;
}

/*========================================================
  FUNCTION: get/set_qmi_srv_id
  - QMI service id. (example QMI_NAS = 0x03).
  - Note this service id need not be the same as the actual
    value for QMI service id, although the values may
    coincide.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_service_id(int new_service_id)
{
  service_id = new_service_id;
}

int ril_utf_node_base_t::get_service_id()
{
  return service_id;
}

/*========================================================
  FUNCTION: get/set_in_exp_set
  - status of node in unorder set of nodes

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_in_exp_set(bool val)
{
  in_exp_set = val;
}
bool ril_utf_node_base_t::get_in_exp_set(void)
{
  return in_exp_set;
}

/*========================================================
  FUNCTION: get/set_in_loop_set
  - status of node in a loop used to poll status

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_in_loop_set(bool val)
{
  in_loop_set = val;
}
bool ril_utf_node_base_t::get_in_loop_set(void)
{
  return in_loop_set;
}


/*========================================================
  FUNCTION: get/set_in_callflow_summary
  - status of node in callflow summary

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_in_callflow_summary(bool val)
{
  in_callflow_summary = val;
}
bool ril_utf_node_base_t::get_in_callflow_summary(void)
{
  return in_callflow_summary;
}

/*========================================================
  FUNCTION: get/set_client_id
  - get/set the qmi service client_id.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_client_id(int id)
{
  client_id = id;
}

int ril_utf_node_base_t::get_client_id(void)
{
  return client_id;
}
/*========================================================
  FUNCTION: get/set_token_id
  - get/set the ril atel token_id

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_token_id(int id)
{
  token_id = id;
}
int ril_utf_node_base_t::get_token_id(void)
{
  return token_id;
}

/*========================================================
  FUNCTION: get/set_expect_status
  - Gets/Sets the expectation status.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_expect_status(enum ril_utf_expectation_t s)
{
  status = s;
}

enum ril_utf_expectation_t ril_utf_node_base_t::get_expect_status(void)
{
  return status;
}

/*========================================================
  FUNCTION: print_expectation
  - prints relevant details about the expecatation including
    service id, msg id, and the message itself.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::print_expectation(void)
{
  RIL_UTF_HTML_LOG(" Function: %s", location.function);
  RIL_UTF_HTML_LOG(" File: %s Line: %d", location.file, location.line_number);
  switch ( payload_type )
  {
    case ril_utf_ril_request:
    case ril_utf_ril_unsol_response:
    case ril_utf_ril_response:
      print_ril_message_name(msg_id);
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: %d, Token ID: %d, Type: %s", msg_id, token_id, ril_utf_q_element_t_name[payload_type].name);
      print_msg_in_hex(payload, payload_len);
      break;

    case ril_utf_qmi_request:
    case ril_utf_qmi_response:
    case ril_utf_qmi_indication:
      print_qmi_message_name(service_id, msg_id, payload_type);
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: 0x%04x, Type: %s", msg_id, ril_utf_q_element_t_name[payload_type].name);
      print_msg_in_tlv(msg_id, (enum ril_utf_qmi_srv_id_t)service_id, client_id, payload_type, payload, payload_len);
      break;

#ifndef RIL_UTF_IOE
    case ril_utf_ims_request:
    case ril_utf_ims_response:
    case ril_utf_ims_indication:
      print_ims_message_name(msg_id);
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: %d, Token ID: %d, Type: %s", msg_id, token_id, ril_utf_q_element_t_name[payload_type].name);
      break;

    case ril_utf_oem_request:
    case ril_utf_oem_response:
    case ril_utf_oem_indication:
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: 0x%04x, Type: %s", msg_id, ril_utf_q_element_t_name[payload_type].name);
      print_oem_message_name(service_id, msg_id, oem_command_id, payload_type);
      break;
    case ril_utf_msim_send_msg:
    case ril_utf_msim_recv_msg:
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: %d, Type: %s", msg_id, ril_utf_q_element_t_name[payload_type].name);
      print_msim_message_name(msg_id);
      print_msg_in_hex(payload, payload_len);
      break;
#endif
    case ril_utf_db_response:
      print_db_msg(payload, payload_len);
      break;
    default:
      RIL_UTF_HTML_LOG("     Unsupported Message Type %d", payload_type);
      break;
  }
}

/*========================================================
  FUNCTION: print_injection
  - prints relevant details about the injection including
    service id, msg id, and the message itself.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::print_injection(void)
{
  switch ( payload_type )
  {
    case ril_utf_ril_request:
    case ril_utf_ril_unsol_response:
    case ril_utf_ril_response:
      print_ril_message_name(msg_id);
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Client ID: %d, Message ID: %d, Type: %s", client_id, msg_id, ril_utf_q_element_t_name[payload_type].name);
      print_msg_in_hex(payload, payload_len);
      break;

    case ril_utf_qmi_request:
    case ril_utf_qmi_response:
    case ril_utf_qmi_indication:
      //TODO: decode qmi msg first
      print_qmi_message_name(service_id, msg_id, payload_type);
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Client ID: %d, Message ID: 0x%04x, Type: %s", client_id, msg_id, ril_utf_q_element_t_name[payload_type].name);
      print_msg_in_tlv(msg_id, (enum ril_utf_qmi_srv_id_t)service_id, client_id, payload_type, payload, payload_len);
      break;
    case ril_utf_db_request:
      print_db_msg(payload, payload_len);
      break;
    case ril_utf_ims_request:
    case ril_utf_ims_response:
    case ril_utf_ims_indication:
      print_ims_message_name(msg_id);
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: %d, Token ID: %d, Type: %s", msg_id, token_id, ril_utf_q_element_t_name[payload_type].name);
      break;
    case ril_utf_oem_request:
    case ril_utf_oem_response:
    case ril_utf_oem_indication:
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: 0x%04x, Type: %s", msg_id, ril_utf_q_element_t_name[payload_type].name);
      print_oem_message_name(service_id, msg_id, oem_command_id, payload_type);
      break;
    default:
      RIL_UTF_HTML_LOG("     Unsupported Message Type %d", payload_type);
      break;
  }
}

void ril_utf_node_base_t::set_source_location(const source_location_t &location)
{
    this->location = location;
}

const source_location_t &ril_utf_node_base_t::get_source_location(void)
{
    return this->location;
}

/*========================================================
  FUNCTION: get_msg_id
  - get/set the messate id. The message could be either
    RIL or QMI.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_msg_id(int id)
{
  msg_id = id;
}

int ril_utf_node_base_t::get_msg_id()
{
  return msg_id;
}


/*========================================================
  FUNCTION:  get_payload / status
  - return a pointer to the payload.
  Note: The data pointed to by payload should ideally not
  be modified outside of the class member functions. If
  accesed, behaviour of framework will be undefined.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void * ril_utf_node_base_t::get_payload(void)
{
  return payload;
}

int  ril_utf_node_base_t::get_payload_len(void)
{
  return payload_len;
}


void ril_utf_node_base_t::add_callflow_tag(char *tag)
{
  callflow_tag = tag;
}
const char *ril_utf_node_base_t::get_callflow_tag()
{
  return callflow_tag.c_str();
}


/*========================================================
  FUNCTION: verify_payload
  - If not set, payload of expect will not be verified

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_verify_payload(bool v)
{
  verify_payload = v;
}
bool ril_utf_node_base_t::is_verify_payload(void)
{
  return verify_payload;
}

/*========================================================
  FUNCTION: is_fatal
  - If set to "fatal", then the test case will terminate
    on this failure.

  Critical section access:
  - All access to node members will happen from the context
    of the "main" test execution thread. So no concurrency
    issues here.
========================================================*/
void ril_utf_node_base_t::set_fatal_on_failure(bool f)
{
  is_failure_fatal = f;
}

bool ril_utf_node_base_t::is_fatal_on_failure(void)
{
  return is_failure_fatal;
}

///////////////////////////////////////////////////////////
//                    Timeout Node
///////////////////////////////////////////////////////////
ril_utf_timeout_if::ril_utf_timeout_if(void)
{
  milli_sec_timeout = 20;  // default 20 ms
}

void ril_utf_timeout_if::set_timeout_value(int milli_sec)
{
  milli_sec_timeout = milli_sec;
}

int ril_utf_timeout_if::get_timeout_value(void)
{
  return milli_sec_timeout;
}

///////////////////////////////////////////////////////////
//                    Block Client Init Node
///////////////////////////////////////////////////////////
ril_utf_block_client_init_if::ril_utf_block_client_init_if(void)
{
}

void ril_utf_block_client_init_if::set_blocked_service(enum ril_utf_qmi_srv_id_t service)
{
  my_service = service;
}
enum ril_utf_qmi_srv_id_t ril_utf_block_client_init_if::get_blocked_service()
{
  return my_service;
}
void ril_utf_block_client_init_if::set_block_state(int state)
{
  block = state;
}
int ril_utf_block_client_init_if::get_block_state()
{
  return block;
}
///////////////////////////////////////////////////////////
//                    Canary Node
///////////////////////////////////////////////////////////
ril_utf_canary_if::ril_utf_canary_if(void)
{
  micro_sec_timeout = 1000;
}
void ril_utf_canary_if::set_timeout_micro(int micro_sec)
{
  micro_sec_timeout = micro_sec;
}
int ril_utf_canary_if::get_timeout_micro(void)
{
  return micro_sec_timeout;
}

///////////////////////////////////////////////////////////
//                    Callfow Node
///////////////////////////////////////////////////////////
ril_utf_callflow_summary_if::ril_utf_callflow_summary_if(void)
{
  description = "";
}
void ril_utf_callflow_summary_if::set_description(char *desc)
{
  description = desc;
}

const char *ril_utf_callflow_summary_if::get_description()
{
  return description.c_str();
}
