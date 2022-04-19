/*===========================================================================

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <stdint.h>
#include <set>
#include <unordered_map>

#include "legacy/qcril_data.h"
#include "event/RilEventDataCallback.h"
#include "event/CallManagerEventMessage.h"

#include "LocalLogBuffer.h"
#include "DsiWrapper.h"

#include "legacy/qmi_embms_v01.h"


#define EMBMS_TMGI_LEN 6
/**
 * @brief EMBMS information class
 * @details EMBMS information class
 */
namespace ril_data {

typedef struct
{
  int                      cId;
  qcril_instance_id_e_type instance_id;
  RIL_Token                pend_tok;
  qcril_evt_e_type         pend_req;
  int                      event_id_android;
  char                     session_id_valid;
  int                      session_id;
  embms_tmgi_type_v01      tmgi;
  embms_tmgi_type_v01      deact_tmgi;
} tmgiRequestType;

}

class EmbmsRilResponse {
public:
  EmbmsRilResponse(qcril_evt_e_type requestId, qcril_instance_id_e_type instanceId, int requestIdAndroid, RIL_Token token, RIL_Errno rilResponse){
    mRequestId = requestId;
    mInstanceId = instanceId;
    mRequestIdAndroid = requestIdAndroid;
    mRiltoken = token;
    mRilResponse = rilResponse;
    mRespPacket = nullptr;
    mRespPktLen = 0;
  }
  ~EmbmsRilResponse(){
    if (mRespPacket != nullptr)
      free(mRespPacket);
      mRespPacket = nullptr;
  }

  void sendActivateTmgiResponse(embms_tmgi_type_v01 *tmgi_info, int32_t dbg_trace_id,
                                uint8_t call_id, EmbmsDataCallFailCause_t failCause);
  void sendDeactivateTmgiResponse(embms_tmgi_type_v01 *tmgi_info, int32_t dbg_trace_id,
                                  uint8_t call_id, EmbmsDataCallFailCause_t failCause);
  void sendActivateDeactivateTmgiResponse(embms_tmgi_type_v01 *act_tmgi_info, embms_tmgi_type_v01 *deact_tmgi_info,
                                 int32_t dbg_trace_id, uint8_t call_id, EmbmsDataCallFailCause_t failCause);
  void sendContentDescUpdateResponse(int32_t dbg_trace_id, EmbmsDataCallFailCause_t failCause);
  void sendGetActiveTmgiResponse(int32_t dbg_trace_id, wds_embms_tmgi_list_query_resp_msg_v01 *embms_tmgi,
                                 EmbmsDataCallFailCause_t failCause);
  void sendGetAvailableTmgiResponse(int32_t dbg_trace_id, wds_embms_tmgi_list_query_resp_msg_v01 *embms_tmgi,
                                    EmbmsDataCallFailCause_t failCause);
  void sendInterestedListResponse(int32_t dbg_trace_id);

private:
  qcril_evt_e_type mRequestId;
  qcril_instance_id_e_type mInstanceId;
  int mRequestIdAndroid;
  RIL_Token mRiltoken;
  RIL_Errno mRilResponse;
  void *mRespPacket;
  int mRespPktLen;
  void sendRilResponse();
};

class EmbmsCallEndReason {
public:
  static EmbmsDataCallFailCause_t getFailCause(int dsiReason);
private:
  static unordered_map<int, EmbmsDataCallFailCause_t> dsiCallEndReasonMap;
};

class EmbmsCallHandler {
public:
  EmbmsCallHandler(LocalLogBuffer& logBuffer);
  ~EmbmsCallHandler();

  void activateTmgi(qcril_request_params_type *params_ptr);
  void deactivateTmgi(qcril_request_params_type *params_ptr);
  void activateDeactivateTmgi(qcril_request_params_type *params_ptr);
  void contentDescUpdate(qcril_request_params_type *params_ptr);
  void sendInterestedList(qcril_request_params_type *params_ptr);
  void getActiveTmgi(qcril_request_params_type *params_ptr);
  void getAvailableTmgi(qcril_request_params_type *params_ptr);
  void checkIsActive();
  void processEvents(dsi_net_evt_t evt, dsi_embms_tmgi_info_type *embms_payload);
  void setDsiHandle(dsi_hndl_t handle){dsiHandle = handle;}
  void setInstanceId(int instance_id){instanceId = instance_id;}
  int isEmbmsActive(void);

private:
  LocalLogBuffer& logBuffer;
  dsi_hndl_t dsiHandle;
  int instanceId;
  std::list<std::shared_ptr<ril_data::tmgiRequestType>> txnList;
  void processTmgiDeactivatedEvent(dsi_embms_tmgi_info_type *embms_payload);
  void processTmgiActivatedEvent(dsi_embms_tmgi_info_type *embms_payload);
  void processTmgiActivatedDeactivatedEvent(dsi_embms_tmgi_info_type *embms_payload);
};


