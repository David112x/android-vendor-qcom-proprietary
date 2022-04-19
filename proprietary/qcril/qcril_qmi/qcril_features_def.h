/******************************************************************************
  @file    qcril_features_def.h
  @brief   qcril feature definition information

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2015, 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------


  ---------------------------------------------------------------------------
  Not a Contribution

  Copyright (C) 2006 The Android Open Source Project

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  ---------------------------------------------------------------------------
******************************************************************************/



#ifndef QCRIL_FEATURES_DEF_H
#define QCRIL_FEATURES_DEF_H

#define EXTENDED_FAIL_ERROR_CAUSE_FOR_VOICE_CALL (1)
#define HOLD_RESUME_ERROR_CODES (1)
// Max number of RIL_REQUESTs supported.
// Keeping the value as RIL_REQUEST_STOP_KEEPALIVE (max RIL_REQUEST ID as per ril.h) + 10 (add extra
// buffer 10 in case of customers having extra requests defined by themselfs).
#define QCRIL_FEATURES_DEF_MAX_REQUEST_ID      155
// Max number of RIL_UNSOLs supported.
// Keeping the value as RIL_UNSOL_KEEPALIVE_STATUS (max RIL_UNSOL ID as per ril.h) + 10 (add extra
// buffer 10 in case of customers having extra unsols defined by themselfs).
#define QCRIL_FEATURES_DEF_MAX_UNSOL_ID       1060
#define QCRIL_FEATURES_DEF_MAX_ERROR_ID        525
#define RIL_REQUEST_GET_ADN_RECORD (300)
#define RIL_REQUEST_UPDATE_ADN_RECORD (301)
#define RIL_UNSOL_RESPONSE_ADN_INIT_DONE (1046)
#define RIL_UNSOL_RESPONSE_ADN_RECORDS (1047)
#define RIL_NUM_ADN_RECORDS (10)
#define RIL_MAX_NUM_AD_COUNT (4)
#define RIL_MAX_NUM_EMAIL_COUNT (2)
typedef struct {
    int       record_id;
    char*     name;
    char*     number;
    int       email_elements;
    char*     email[RIL_MAX_NUM_EMAIL_COUNT];
    int       anr_elements;
    char*     ad_number[RIL_MAX_NUM_AD_COUNT];
} RIL_AdnRecordInfo;
typedef struct {
    int               record_elements;
    RIL_AdnRecordInfo adn_record_info[RIL_NUM_ADN_RECORDS];
} RIL_AdnRecord_v1;

#endif /* QCRIL_FEATURES_DEF_H */
