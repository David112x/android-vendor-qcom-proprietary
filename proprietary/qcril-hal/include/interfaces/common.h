/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <framework/message_translator.h>
#include <telephony/ril.h>
#include <string>

/*============================================================================
                QCRIL Interface Common macros
============================================================================*/

/*
 * QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR
 */
#define QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(type, field)                                      \
private:                                                                                      \
  bool _##field##_valid;                                                                      \
  type _##field;                                                                              \
                                                                                              \
public:                                                                                       \
  void set##field(type in) {                                                                  \
    _##field##_valid = true;                                                                  \
    _##field = in;                                                                            \
  }                                                                                           \
  bool has##field() const { return _##field##_valid; }                                        \
  type get##field() const { return _##field; }                                                \
  type& get##field##Ref() { return  _##field; }

/*
 * QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR
 */
#define QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(type, field)                           \
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::shared_ptr<type>, field);

/*
 * QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR
 */
#define QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(type, field)                               \
private:                                                                                      \
  std::vector<type> _##field;                                                                 \
                                                                                              \
public:                                                                                       \
  void set##field(std::vector<type> in) { _##field = in; }                                    \
  void addTo##field(type val) { _##field.push_back(val); }                                    \
  std::vector<type> get##field() const { return _##field; }                                   \
  std::vector<type>& get##field##Ref() { return _##field; }

/*
 * QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR
 */
#define QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(field) _##field##_valid = false;

/*
 * QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR
 */
#define QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, field)                                 \
  _##field##_valid = from._##field##_valid;                                                   \
  _##field = from._##field;

/*
 * QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR
 */
#define QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(field)                           \
  _##field##_valid = false;                                                                   \
  _##field = nullptr;

/*
 * QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR
 */
#define QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, field)                      \
  _##field##_valid = from._##field##_valid;                                                   \
  _##field = from._##field;

/*
 * QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR
 */
#define QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(field) _##field.clear();

/*
 * QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR
 */
#define QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, field) _##field = from._##field;

/*============================================================================
                QCRIL Interface Common data types
============================================================================*/
namespace qcril {
namespace interfaces {

/*
 * Common Base class for all interface classes
 */
class BasePayload {};

}  // namespace interfaces
}  // namespace qcril
