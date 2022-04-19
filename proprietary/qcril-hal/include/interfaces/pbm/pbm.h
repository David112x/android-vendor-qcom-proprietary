/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <interfaces/common.h>

namespace qcril {
namespace interfaces {

typedef struct {
  int                       max_adn_num;
  int                       valid_adn_num;
  int                       max_email_num;
  int                       valid_email_num;
  int                       max_ad_num;
  int                       valid_ad_num;
  int                       max_name_len;
  int                       max_number_len;
  int                       max_email_len;
  int                       max_anr_len;
} qcril_pbm_adn_count_info;

class AdnRecordInfo {
  public:
    int       record_id;
    string    name;
    string    number;
    int       email_elements;
    string    email[RIL_MAX_NUM_EMAIL_COUNT];
    int       anr_elements;
    string    ad_number[RIL_MAX_NUM_AD_COUNT];
};

class AdnRecords {
  public:
    int           record_elements;
    AdnRecordInfo adn_record_info[RIL_NUM_ADN_RECORDS];
};

// Message Callback structure
class AdnCountInfoResp : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(qcril_pbm_adn_count_info, AdnCountInfo);

 public:
  AdnCountInfoResp() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(AdnCountInfo);
  }
  AdnCountInfoResp(const AdnCountInfoResp &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, AdnCountInfo);
  }
  ~AdnCountInfoResp() {}
};

class AdnRecordUpdatedResp : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int32_t, RecordIndex);

 public:
  AdnRecordUpdatedResp() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(RecordIndex);
  }
  AdnRecordUpdatedResp(const AdnRecordUpdatedResp &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, RecordIndex);
  }
  ~AdnRecordUpdatedResp() {}
};

} // namespace interfaces
} // namespace qcril
