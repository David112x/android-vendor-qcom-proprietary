/******************************************************************************
  @file    ril_utf_i_ims_radio_sim.cpp
  @brief   RIL UTF IMS HIDL implementation

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_service.h"
#include "modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_utils.h"
#include "modules/android_ims_radio/hidl_impl/1.4/qcril_qmi_ims_radio_service_1_4.h"
#include "modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_service_1_5.h"
#include "modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_utils_1_5.h"
#include "modules/android_ims_radio/hidl_impl/1.6/qcril_qmi_ims_radio_service_1_6.h"
#include "ril_utf_hidl_services.h"
#include "ril_utf_ims_if.h"
#include "ril_utf_i_ims_radio_sim_utils.h"
#ifdef QMI_RIL_HAL_UTF
#include "ril_utf_core.h"
#include "ril_utf_ims_sim.h"
#endif
extern "C" {
#ifndef QMI_RIL_HAL_UTF
#include "ril_utf_core.h"
#endif
}

using namespace vendor::qti::hardware::radio::ims;
using namespace vendor::qti::hardware::radio::ims::V1_0;
using namespace vendor::qti::hardware::radio::ims::V1_0::implementation;
using namespace vendor::qti::hardware::radio::ims::V1_0::utils;

using ::android::hardware::Void;

void send_i_ims_radio_enqueue_ims_expect(int size, void *payload, int msg_id, int token,
                                         enum ril_utf_q_element_t type, ims_Error error) {
  int my_expect_slot;

  // acquire mutex
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: IMS recv thread locking exec_q_access in %s", __func__);
  EXEC_Q_LOCK();
  // Grab next available slot in expect table
  for (int i = 0; i < MAX_EXPECTS; ++i) {
    if (expect_table[i].in_use == 0) {
      my_expect_slot = i;
      expect_table[my_expect_slot].in_use = 1;
      break;
    }
  }

  // verify that we are allowed to proceed
  while (expect_table[my_expect_slot].expect_ready != 1) {
    struct timeval e_timeout;
    struct timespec thread_timeout;
    e_timeout.tv_sec = 0;
    e_timeout.tv_usec = EXPECT_TIMEOUT;
    calc_timeout(&thread_timeout, &e_timeout);
    pthread_cond_timedwait(&expect_ready_sig, &exec_q_access, &thread_timeout);
  }

  enqueue_ims_expect(size, payload, msg_id, token, type, error);

  // signal test thread.
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Setting expect_received to true @ %s %s %d ", __FILE__, __func__,
                __LINE__);
  expect_table[my_expect_slot].expect_received = 1;
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Signalling Expectation");
  pthread_cond_signal(&signal_exptectation);

  // release mutex
  EXEC_Q_UNLOCK();
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. released exec_q_mutex.", __func__);
}

class IImsRadioResponseClient : public V1_6::IImsRadioResponse {
 public:
  ::android::hardware::Return<void> dialResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: dialResponse");

    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_DIAL, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);

    return Void();
  }
  ::android::hardware::Return<void> answerResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: answerResponse");

    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_ANSWER, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);

    return Void();
  }
  ::android::hardware::Return<void> hangupResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: hangupResponse");

    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_HANGUP, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> requestRegistrationChangeResponse(int32_t token,
                                                                      uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: requestRegistrationChangeResponse");

    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_IMS_REG_STATE_CHANGE, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> queryServiceStatusResponse(
      int32_t token, uint32_t errorCode,
      const ::android::hardware::hidl_vec<
          ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo>& srvStatusList) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: queryServiceStatusResponse: no op: expecting "
        "queryServiceStatusResponse_1_6!!!");
    return Void();
  }
  ::android::hardware::Return<void> queryServiceStatusResponse_1_6(
      int32_t token, uint32_t errorCode,
      const ::android::hardware::hidl_vec<
          ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo>& srvStatusList) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: queryServiceStatusResponse_1_6");
    ims_SrvStatusList *msg_data = (ims_SrvStatusList *)qcril_malloc(sizeof(ims_SrvStatusList));
    V1_6::utils::convertHidlToProtoServiceInfoList(srvStatusList, *msg_data);

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_SrvStatusList), msg_data,
                                        ims_MsgId_REQUEST_QUERY_SERVICE_STATUS, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> setServiceStatusResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: setServiceStatusResponse");

    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_SET_SERVICE_STATUS, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> holdResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo &sipError) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: holdResponse");

    ims_SipErrorInfo *msg_data = nullptr;
    if (errorCode != ims_Error_E_SUCCESS) {
      std::string logStr = toString(sipError);
      QCRIL_LOG_INFO("sipError = %s", logStr.c_str());
      msg_data = (ims_SipErrorInfo *)qcril_malloc(sizeof(ims_SipErrorInfo));
      convertHidlToProtoSipErrorInfo(sipError, *msg_data);
    }

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_SipErrorInfo), msg_data, ims_MsgId_REQUEST_HOLD,
                                        token, ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }

  ::android::hardware::Return<void> resumeResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo &sipError) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: resumeResponse");
    ims_SipErrorInfo *msg_data = nullptr;
    if (errorCode != ims_Error_E_SUCCESS) {
      msg_data = (ims_SipErrorInfo *)qcril_malloc(sizeof(ims_SipErrorInfo));
      convertHidlToProtoSipErrorInfo(sipError, *msg_data);
    }

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_SipErrorInfo), msg_data,
                                        ims_MsgId_REQUEST_RESUME, token, ril_utf_ims_response,
                                        (ims_Error)errorCode);
    return Void();
  }

  ::android::hardware::Return<void> setConfigResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo &config) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: setConfigResponse: no op: expecting setConfigResponse_1_6!!!");
    return Void();
  }
  ::android::hardware::Return<void> setConfigResponse_1_6(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo& config) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: setConfigResponse_1_6");

    ims_ConfigMsg *msg_data = (ims_ConfigMsg *)qcril_malloc(sizeof(ims_ConfigMsg));
    V1_6::utils::convertHidlToProtoConfig(config, *msg_data);

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_ConfigMsg), msg_data,
                                        ims_MsgId_REQUEST_SET_IMS_CONFIG, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> getConfigResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo &config) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getConfigResponse");

    ims_ConfigMsg *msg_data = (ims_ConfigMsg *)qcril_malloc(sizeof(ims_ConfigMsg));
    convertHidlToProtoConfig(config, *msg_data);

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_ConfigMsg), msg_data,
                                        ims_MsgId_REQUEST_GET_IMS_CONFIG, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> getRegistrationResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo &registration) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: getRegistrationResponse: no op: expecting "
        "getRegistrationResponse_1_6!!!");
    return Void();
  }
  ::android::hardware::Return<void> getRegistrationResponse_1_6(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& registration) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getRegistrationResponse_1_6");

    ims_Registration *msg_data = (ims_Registration *)qcril_malloc(sizeof(ims_Registration));
    V1_6::utils::convertHidlToProtoRegistrationInfo(registration, *msg_data);

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_Registration), msg_data,
                                        ims_MsgId_REQUEST_IMS_REGISTRATION_STATE, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);

    return Void();
  }
  ::android::hardware::Return<void> suppServiceStatusResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus &suppServiceStatus) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: suppServiceStatusResponse: no op: expecting "
        "suppServiceStatusResponse_1_3!!!");
    return Void();
  }
  ::android::hardware::Return<void> conferenceResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo &errorInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: conferenceResponse");

    ims_SipErrorInfo *msg_data = nullptr;
    if (errorCode != ims_Error_E_SUCCESS) {
      msg_data = (ims_SipErrorInfo *)qcril_malloc(sizeof(ims_SipErrorInfo));
      convertHidlToProtoSipErrorInfo(errorInfo, *msg_data);
    }

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_SipErrorInfo), msg_data,
                                        ims_MsgId_REQUEST_CONFERENCE, token, ril_utf_ims_response,
                                        (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> getClipResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus &clipProvisionStatus) {
    std::string logStr = toString(clipProvisionStatus);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getClipResponse %d, %d, %s", token, errorCode, logStr.c_str());
    ims_ClipProvisionStatus *msg_data =
        (ims_ClipProvisionStatus *)qcril_malloc(sizeof(ims_ClipProvisionStatus));

    if (errorCode == ims_Error_E_SUCCESS) {
      msg_data->has_clip_status = TRUE;
      switch (clipProvisionStatus.clipStatus) {
        case ClipStatus::NOT_PROVISIONED:
          msg_data->clip_status = ims_ClipStatus_NOT_PROVISIONED;
          break;
        case ClipStatus::PROVISIONED:
          msg_data->clip_status = ims_ClipStatus_PROVISIONED;
          break;
        case ClipStatus::STATUS_UNKNOWN:
          msg_data->clip_status = ims_ClipStatus_STATUS_UNKNOWN;
          break;
        case ClipStatus::INVALID:
        default:
          msg_data->has_clip_status = FALSE;
          break;
      }
    }
    if (errorCode != ims_Error_E_SUCCESS && clipProvisionStatus.hasErrorDetails) {
      msg_data->has_errorDetails = clipProvisionStatus.hasErrorDetails;
      convertHidlToProtoSipErrorInfo(clipProvisionStatus.errorDetails, msg_data->errorDetails);
    }

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_ClipProvisionStatus), msg_data,
                                        ims_MsgId_REQUEST_QUERY_CLIP, token, ril_utf_ims_response,
                                        (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> getClirResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo &clirInfo, bool hasClirInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getClirResponse");
    ims_Clir *msg_data = (ims_Clir *)qcril_malloc(sizeof(ims_Clir));
    if (hasClirInfo) {
      convertHidlToProtoClirInfo(clirInfo, *msg_data);
    }
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_Clir), msg_data, ims_MsgId_REQUEST_GET_CLIR,
                                        token, ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> setClirResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: setClirResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_SET_CLIR, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> getColrResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo &colrInfo) {
    return Void();
  }
  ::android::hardware::Return<void> exitEmergencyCallbackModeResponse(int32_t token,
                                                                      uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: exitEmergencyCallbackModeResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE,
                                        token, ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> sendDtmfResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: sendDtmfResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_DTMF, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> startDtmfResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: startDtmfResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_DTMF_START, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> stopDtmfResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: stopDtmfResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_DTMF_STOP, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> setUiTTYModeResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: setUiTTYModeResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_SEND_UI_TTY_MODE, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> modifyCallInitiateResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: modifyCallInitiateResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_MODIFY_CALL_INITIATE, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> modifyCallConfirmResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: modifyCallConfirmResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_MODIFY_CALL_CONFIRM, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> queryCallForwardStatusResponse(
      int32_t token, uint32_t errorCode,
      const ::android::hardware::hidl_vec<
          ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo> &callForwardInfoList,
      const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo &errorDetails) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: queryCallForwardStatusResponse");

    ims_CallForwardInfoList *msg_data =
        (ims_CallForwardInfoList *)qcril_malloc(sizeof(ims_CallForwardInfoList));

    int len = callForwardInfoList.size();
    if (len > 0) {
      ims_CallForwardInfoList_CallForwardInfo **dptr =
          (ims_CallForwardInfoList_CallForwardInfo **)qcril_malloc(
              sizeof(ims_CallForwardInfoList_CallForwardInfo *) * (len + 1));
      for (int i = 0; i < len; i++) {
        dptr[i] = (ims_CallForwardInfoList_CallForwardInfo *)qcril_malloc(
            sizeof(ims_CallForwardInfoList_CallForwardInfo));
        convertHidlToProtoCallForwardInfo(callForwardInfoList[i], *dptr[i]);
      }
      msg_data->info.arg = dptr;
    }
    msg_data->has_errorDetails = true;
    convertHidlToProtoSipErrorInfo(errorDetails, msg_data->errorDetails);

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_CallForwardInfoList), msg_data,
                                        ims_MsgId_REQUEST_QUERY_CALL_FORWARD_STATUS, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);

    return Void();
  }
  ::android::hardware::Return<void> getCallWaitingResponse(
      int32_t token, uint32_t errorCode,
      ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus,
      uint32_t serviceClass,
      const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo &errorDetails) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getCallWaitingResponse");

    ims_CallWaitingInfo *msg_data =
        (ims_CallWaitingInfo *)qcril_malloc(sizeof(ims_CallWaitingInfo));

    if (serviceStatus != ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::INVALID) {
      msg_data->has_service_status = TRUE;
      msg_data->service_status = convertHidlToProtoServiceClassStatus(serviceStatus);
    }
    if (serviceClass != INT32_MAX) {
      msg_data->has_service_class = TRUE;
      msg_data->service_class.has_service_Class = TRUE;
      msg_data->service_class.service_Class = serviceClass;
    }

    if (errorCode != ims_Error_E_SUCCESS) {
      msg_data->has_errorDetails = true;
      convertHidlToProtoSipErrorInfo(errorDetails, msg_data->errorDetails);
    }

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_CallWaitingInfo), msg_data,
                                        ims_MsgId_REQUEST_QUERY_CALL_WAITING, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> explicitCallTransferResponse(int32_t token,
                                                                 uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: explicitCallTransferResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_EXPLICIT_CALL_TRANSFER, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> setSuppServiceNotificationResponse(
      int32_t token, uint32_t errorCode,
      ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: setSuppServiceNotificationResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_SET_SUPP_SVC_NOTIFICATION,
                                        token, ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> getRtpStatisticsResponse(int32_t token, uint32_t errorCode,
                                                             uint64_t packetCount) {
    return Void();
  }
  ::android::hardware::Return<void> getRtpErrorStatisticsResponse(int32_t token, uint32_t errorCode,
                                                                  uint64_t packetErrorCount) {
    return Void();
  }
  ::android::hardware::Return<void> addParticipantResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: addParticipantResponse");
    ims_SipErrorInfo *msg_data = (ims_SipErrorInfo *)qcril_malloc(sizeof(ims_SipErrorInfo));
    // TODO: there is no errorInfo in the interface!!!
    // convertHidlToProtoSipErrorInfo(errorInfo, *msg_data);
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_SipErrorInfo), msg_data,
                                        ims_MsgId_REQUEST_ADD_PARTICIPANT, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);

    return Void();
  }
  ::android::hardware::Return<void> deflectCallResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: deflectCallResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_DEFLECT_CALL, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> sendGeolocationInfoResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: sendGeolocationInfoResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> getImsSubConfigResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo &subConfigInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getImsSubConfigResponse");
    ims_ImsSubConfig *msg_data = nullptr;
    if (errorCode == ims_Error_E_SUCCESS) {
      msg_data = (ims_ImsSubConfig *)qcril_malloc(sizeof(ims_ImsSubConfig));
      convertHidlToProtoImsSubConfig(subConfigInfo, *msg_data);
    }
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_ImsSubConfig), msg_data,
                                        ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> sendRttMessageResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: sendRttMessageResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_SEND_RTT_MSG, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> cancelModifyCallResponse(int32_t token, uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: cancelModifyCallResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_CANCEL_MODIFY_CALL, token,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> sendImsSmsResponse(
      int32_t token, uint32_t msgRef,
      ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult smsStatus,
      ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason reason) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: sendImsSmsResponse");

    ims_Error errorCode = convertHidlToInternalImsSmsResponse(smsStatus, reason);
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_SEND_SMS,
                token, ril_utf_ims_response, errorCode);
    return Void();
  }
  ::android::hardware::Return<void> suppServiceStatusResponse_1_3(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus &suppServiceStatus) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: suppServiceStatusResponse_1_3");
    // Same suppServiceStatusResponse for used for all of the below Messages
    // ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS  // Not used from v1_5 onwards
    // ims_MsgId_REQUEST_SET_CALL_WAITING
    // ims_MsgId_REQUEST_SET_COLR
    // ims_MsgId_REQUEST_SUPP_SVC_STATUS
    uint32_t origToken = (token & 0x0000FFFF);
    uint32_t msgId = (token & 0xFFFF0000) >> 16;

    ims_SuppSvcResponse *msg_data = nullptr;
    if ((msgId == ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS && errorCode != ims_Error_E_SUCCESS) ||
        (msgId != ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS)) {
      msg_data = (ims_SuppSvcResponse *)qcril_malloc(sizeof(ims_SuppSvcResponse));
      convertHidlToProtoSuppServiceStatus(suppServiceStatus, *msg_data);
    }
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_SuppSvcResponse), msg_data, msgId, origToken,
                                        ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> getColrResponse_1_3(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo &colrInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getColrResponse_1_3");
    ims_Colr *msg_data = (ims_Colr *)qcril_malloc(sizeof(ims_Colr));

    if (errorCode == ims_Error_E_SUCCESS) {
      if (colrInfo.status != ServiceClassStatus::INVALID) {
        msg_data->has_status = TRUE;
        msg_data->status = convertHidlToProtoServiceClassStatus(colrInfo.status);
      }
      if (colrInfo.provisionStatus !=
          ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::INVALID) {
        msg_data->has_provisionStatus = TRUE;
        msg_data->provisionStatus =
            convertHidlToProtoServiceClassProvisionStatus(colrInfo.provisionStatus);
      }
      if (colrInfo.presentation != IpPresentation::IP_PRESENTATION_INVALID) {
        msg_data->has_presentation = TRUE;
        msg_data->presentation = convertHidlToProtoPresentation(colrInfo.presentation);
      }
    }
    if (errorCode != ims_Error_E_SUCCESS && colrInfo.hasErrorDetails) {
      msg_data->has_errorDetails = TRUE;
      convertHidlToProtoSipErrorInfo(colrInfo.errorDetails, msg_data->errorDetails);
    }

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_Colr), msg_data, ims_MsgId_REQUEST_GET_COLR,
                                        token, ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> registerMultiIdentityLinesResponse(int32_t token,
                                                                       uint32_t errorCode) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: registerMultiIdentityLinesResponse");
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES,
                                        token, ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> queryVirtualLineInfoResponse(
      int32_t token, const ::android::hardware::hidl_string &msisdn,
      const ::android::hardware::hidl_vec<::android::hardware::hidl_string> &virtualLineInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: queryVirtualLineInfoResponse");
    ims_VirtualLineInfoResp *msg_data =
      (ims_VirtualLineInfoResp *)qcril_malloc(sizeof(ims_VirtualLineInfoResp));
    if (!msisdn.empty()) {
      strlcpy(msg_data->msisdn, msisdn.c_str(), msisdn.size()+1);
    }
    if (virtualLineInfo.size()) {
      msg_data->lines_len = virtualLineInfo.size();
      for(uint32_t i = 0; i < 10 && i < virtualLineInfo.size(); i++) {
        if (!virtualLineInfo[i].empty()) {
          strlcpy(msg_data->lines[i], virtualLineInfo[i].c_str(), virtualLineInfo[i].size()+1);
        }
      }
    }
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_VirtualLineInfoResp), msg_data,
                                        ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO,
                                        token, ril_utf_ims_response, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> setCallForwardStatusResponse(
      int32_t token, uint32_t errorCode,
      const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo &callForwardStatus) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: setCallForwardStatusResponse");
    // Same suppServiceStatusResponse for used for all of the below Messages
    // ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS
    uint32_t origToken = (token & 0x0000FFFF);
    uint32_t msgId = (token & 0xFFFF0000) >> 16;

    ims_CallForwardStatusInfo *msg_data = nullptr;
    msg_data = (ims_CallForwardStatusInfo *)qcril_malloc(sizeof(ims_CallForwardStatusInfo));
    V1_5::utils::convertHidlToProtoCallForwardStatusInfo(callForwardStatus, *msg_data);
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_CallForwardStatusInfo), msg_data, msgId,
                                        origToken, ril_utf_ims_response, (ims_Error)errorCode);
    return Void();
  }
  ::android::hardware::Return<void> sendImsSmsResponse_1_5(
      int32_t token, uint32_t msgRef,
      ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult smsStatus,
      ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason reason) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: sendImsSmsResponse_1_5");

    ims_Error errorCode = convertHidlToInternalImsSmsResponse(smsStatus, reason);
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_REQUEST_SEND_SMS,
                token, ril_utf_ims_response, errorCode);
    return Void();
  }

};

class IImsRadioIndicationClient : public V1_6::IImsRadioIndication {
 public:
  ::android::hardware::Return<void> onCallStateChanged(
      const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CallInfo>
          &callList) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onCallStateChanged: no op: expecting onCallStateChanged_1_5!!!");
    return Void();
  }
  ::android::hardware::Return<void> onCallStateChanged_1_1(
      const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_1::CallInfo>
          &callList) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: onCallStateChanged_1_1: no op: expecting onCallStateChanged_1_5!!!");
    return Void();
  }
  ::android::hardware::Return<void> onCallStateChanged_1_2(
      const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_2::CallInfo>
          &callList) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: onCallStateChanged_1_2: no op: expecting onCallStateChanged_1_5!!!");
    return Void();
  }
  ::android::hardware::Return<void> onCallStateChanged_1_3(
      const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_3::CallInfo>
          &callList) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: onCallStateChanged_1_3: no op: expecting onCallStateChanged_1_5!!!");
    return Void();
  }
  ::android::hardware::Return<void> onCallStateChanged_1_4(
      const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_4::CallInfo>
          &callList) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: onCallStateChanged_1_4: no op: expecting onCallStateChanged_1_5!!!");
    return Void();
  }
  ::android::hardware::Return<void> onCallStateChanged_1_5(
      const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_5::CallInfo>&
          callList) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: onCallStateChanged_1_5: no op: expecting onCallStateChanged_1_6!!!");
    return Void();
  }
  ::android::hardware::Return<void> onCallStateChanged_1_6(
      const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_6::CallInfo>&
          callList) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onCallStateChanged_1_6");

    ims_CallList* msg_data = (ims_CallList*)qcril_malloc(sizeof(ims_CallList));
    V1_6::utils::convertHidlToProtoCallList(callList, *msg_data);

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_CallList), msg_data,
                                        ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }
  ::android::hardware::Return<void> onRing() {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onRing");
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_UNSOL_CALL_RING, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }
  ::android::hardware::Return<void> onRingbackTone(
      ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation tone) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onRingbackTone");
    ims_RingBackTone *msg_data = (ims_RingBackTone *)qcril_malloc(sizeof(ims_RingBackTone));

    msg_data->has_flag = (tone != ToneOperation::INVALID);
    if (msg_data->has_flag) {
      msg_data->flag = convertHidlToProtoToneOperation(tone);
    }

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_UNSOL_RINGBACK_TONE, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }
  ::android::hardware::Return<void> onRegistrationChanged(
      const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& registration) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: onRegistrationChanged: no op: expecting onRegistrationChanged_1_6!!!");
    return Void();
  }
  ::android::hardware::Return<void> onRegistrationChanged_1_6(
      const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& registration) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onRegistrationChanged");

    ims_Registration *msg_data = (ims_Registration *)qcril_malloc(sizeof(ims_Registration));
    V1_6::utils::convertHidlToProtoRegistrationInfo(registration, *msg_data);

    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_Registration), msg_data,
                                        ims_MsgId_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }
  ::android::hardware::Return<void> onHandover(
      const ::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo& handover) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onHandover: no op: expecting onHandover_1_6!!!");
    return Void();
  }
  ::android::hardware::Return<void> onHandover_1_6(
      const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& handover) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onHandover_1_6");
    ims_Handover* msg_data = (ims_Handover*)qcril_malloc(sizeof(ims_Handover));

    V1_6::utils::convertHidlToProtoHandoverInfo(handover, *msg_data);

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_Handover), msg_data,
                                        ims_MsgId_UNSOL_RESPONSE_HANDOVER, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }
  ::android::hardware::Return<void> onServiceStatusChanged(
      const ::android::hardware::hidl_vec<
          ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo>& srvStatusList) {
    RIL_UTF_DEBUG(
        "\n QMI_RIL_UTL: onServiceStatusChanged: no op: expecting onServiceStatusChanged_1_6!!!");
    return Void();
  }
  ::android::hardware::Return<void> onServiceStatusChanged_1_6(
      const ::android::hardware::hidl_vec<
          ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo>& srvStatusList) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onServiceStatusChanged_1_6");

    ims_SrvStatusList *msg_data = (ims_SrvStatusList *)qcril_malloc(sizeof(ims_SrvStatusList));

    V1_6::utils::convertHidlToProtoServiceInfoList(srvStatusList, *msg_data);

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_SrvStatusList), msg_data,
                                        ims_MsgId_UNSOL_SRV_STATUS_UPDATE, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }

  ::android::hardware::Return<void> onRadioStateChanged(
      ::vendor::qti::hardware::radio::ims::V1_0::RadioState radioState) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onRadioStateChanged");

    ims_RadioStateChanged *msg_data =
        (ims_RadioStateChanged *)qcril_malloc(sizeof(ims_RadioStateChanged));
    msg_data->has_state = true;
    msg_data->state = ims_RadioState_RADIO_STATE_ON;

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_RadioStateChanged), msg_data,
                                        ims_MsgId_UNSOL_RADIO_STATE_CHANGED, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }

  ::android::hardware::Return<void> onEnterEmergencyCallBackMode() {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onEnterEmergencyCallBackMode");
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE,
                                        -1, ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onExitEmergencyCallBackMode() {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onExitEmergencyCallBackMode");
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE,
                                        -1, ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onTtyNotification(
      const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo &ttyInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onTtyNotification");
    ims_TtyNotify *msg_data = (ims_TtyNotify *)qcril_malloc(sizeof(ims_TtyNotify));
    int len = ttyInfo.userData.size();
    RIL_UTF_DEBUG("\n ttyInfo.userData.size() = %d", len);
    if (len) {
      qcril_binary_data_type *userDataPtr =
        (qcril_binary_data_type *)qcril_malloc(sizeof(qcril_binary_data_type));
      userDataPtr->len = len;
      userDataPtr->data = (uint8_t *)ttyInfo.userData.data();
      msg_data->userData.arg = userDataPtr;
    }
    msg_data->has_mode = FALSE;
    if (ttyInfo.mode  != TtyMode::TTY_MODE_INVALID) {
      msg_data->mode = convertHidlToProtoTtyMode(ttyInfo.mode);
      msg_data->has_mode = TRUE;
    }
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_TtyNotify), msg_data,
                                        ims_MsgId_UNSOL_TTY_NOTIFICATION, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onRefreshConferenceInfo(
      const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo &conferenceInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onRefreshConferenceInfo");
    ims_ConfInfo *msg_data = (ims_ConfInfo *)qcril_malloc(sizeof(ims_ConfInfo));
    int len = conferenceInfo.confInfoUri.size();
    RIL_UTF_DEBUG("\n conferenceInfo.confInfoUri.size() = %d", len);
    if (len) {
      qcril_binary_data_type *confInfoUriPtr =
        (qcril_binary_data_type *)qcril_malloc(sizeof(qcril_binary_data_type));
      confInfoUriPtr->len = len;
      confInfoUriPtr->data = (uint8_t *)qcril_malloc(len);
      memcpy(confInfoUriPtr->data, conferenceInfo.confInfoUri.data(), len);
      msg_data->conf_info_uri.arg = confInfoUriPtr;
    }
    msg_data->has_confCallState = TRUE;
    if (conferenceInfo.conferenceCallState  == ConferenceCallState::RINGING) {
      msg_data->confCallState = ims_ConfCallState_RINGING;
    } else if (conferenceInfo.conferenceCallState  == ConferenceCallState::FOREGROUND) {
      msg_data->confCallState = ims_ConfCallState_FOREGROUND;
    } else if (conferenceInfo.conferenceCallState  == ConferenceCallState::BACKGROUND) {
      msg_data->confCallState = ims_ConfCallState_BACKGROUND;
    } else {
      msg_data->has_confCallState = FALSE;
    }
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_ConfInfo), msg_data,
                                        ims_MsgId_UNSOL_REFRESH_CONF_INFO, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onRefreshViceInfo(
      const ::vendor::qti::hardware::radio::ims::V1_0::ViceInfo &viceInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onRefreshViceInfo");
    ims_ViceInfo *msg_data = (ims_ViceInfo *)qcril_malloc(sizeof(ims_ViceInfo));
    int len = viceInfo.viceInfoUri.size();
    RIL_UTF_DEBUG("\n viceInfo.viceInfoUri.size() = %d", len);
    if (len) {
      qcril_binary_data_type *viceInfoUriPtr =
        (qcril_binary_data_type *)qcril_malloc(sizeof(qcril_binary_data_type));
      viceInfoUriPtr->len = len;
      viceInfoUriPtr->data = (uint8_t *)qcril_malloc(len);
      memcpy(viceInfoUriPtr->data, viceInfo.viceInfoUri.data(), len);
      msg_data->vice_info_uri.arg = viceInfoUriPtr;
    }
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_ConfInfo), msg_data,
                                        ims_MsgId_UNSOL_REFRESH_VICE_INFO, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onModifyCall(
      const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo &callModifyInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onModifyCall");

    ims_CallModify *msg_data = (ims_CallModify *)qcril_malloc(sizeof(ims_CallModify));

    convertHidlToProtoCallModifyInfo(callModifyInfo, *msg_data);

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_CallModify), msg_data,
                                        ims_MsgId_UNSOL_MODIFY_CALL, -1, ril_utf_ims_indication,
                                        ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onSuppServiceNotification(
      const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification
          &suppServiceNotification) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onSuppServiceNotification");
    ims_SuppSvcNotification *msg_data =
        (ims_SuppSvcNotification *)qcril_malloc(sizeof(ims_SuppSvcNotification));
    convertHidlToProtoSuppServiceNotification(suppServiceNotification, *msg_data);
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_SuppSvcNotification), msg_data,
                                        ims_MsgId_UNSOL_SUPP_SVC_NOTIFICATION, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onMessageWaiting(
      const ::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication
          &messageWaitingIndication) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onMessageWaiting");
    ims_Mwi *msg_data = (ims_Mwi *)qcril_malloc(sizeof(ims_Mwi));
    // TODO: Add support for validating the MWI
    //convertHidlToProtoMessageWaitingIndication(messageWaitingIndication, *msg_data);
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_Mwi), msg_data,
                                        ims_MsgId_UNSOL_MWI, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onGeolocationInfoRequested(double lat, double lon) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onGeolocationInfoRequested");

    ims_GeoLocationInfo *msg_data =
        (ims_GeoLocationInfo *)qcril_malloc(sizeof(ims_GeoLocationInfo));
    msg_data->has_lat = true;
    msg_data->lat = lat;
    msg_data->has_lon = true;
    msg_data->lon = lon;

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_GeoLocationInfo), msg_data,
                                        ims_MsgId_UNSOL_REQUEST_GEOLOCATION, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onImsSubConfigChanged(
      const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo &config) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onImsSubConfigChanged");
    ims_ImsSubConfig *msg_data = (ims_ImsSubConfig *)qcril_malloc(sizeof(ims_ImsSubConfig));
    convertHidlToProtoImsSubConfig(config, *msg_data);
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_ImsSubConfig), msg_data,
                                        ims_MsgId_UNSOL_IMS_SUB_CONFIG_CHANGED, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onParticipantStatusInfo(
      const ::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo
          &participantStatusInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onParticipantStatusInfo");
    ims_ParticipantStatusInfo *msg_data =
        (ims_ParticipantStatusInfo *)qcril_malloc(sizeof(ims_ParticipantStatusInfo));
    convertHidlToProtoParticipantStatusInfo(participantStatusInfo, *msg_data);
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_ParticipantStatusInfo), msg_data,
                                        ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onRegistrationBlockStatus(
      bool hasBlockStatusOnWwan,
      const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus &blockStatusOnWwan,
      bool hasBlockStatusOnWlan,
      const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus &blockStatusOnWlan) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onParticipantStatusInfo");
    // Decode message and enqueue
    ims_RegistrationBlockStatus *msg_data =
        (ims_RegistrationBlockStatus *)qcril_malloc(sizeof(ims_RegistrationBlockStatus));
    msg_data->has_imsBlockStatusOnWwan = hasBlockStatusOnWwan;
    convertHidlToProtoBlockStatus(blockStatusOnWwan, msg_data->imsBlockStatusOnWwan);
    msg_data->has_imsBlockStatusOnWlan = hasBlockStatusOnWlan;
    convertHidlToProtoBlockStatus(blockStatusOnWlan, msg_data->imsBlockStatusOnWlan);
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_RegistrationBlockStatus), msg_data,
                                        ims_MsgId_UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onRttMessageReceived(
      const ::android::hardware::hidl_string &message) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onRttMessageReceived");
    ims_RttMessage *msg_data =
        (ims_RttMessage *)qcril_malloc(sizeof(ims_RttMessage));
     convertHidlToProtoString(message, (char*&)(msg_data->rttMessage.arg));
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onRttMessageReceived %s ",message.c_str());
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_RttMessage), msg_data,
                                        ims_MsgId_UNSOL_RESPONSE_RTT_MSG_RECEIVED, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }
  ::android::hardware::Return<void> onVoWiFiCallQuality(
      ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo voWiFiCallQualityInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onVoWiFiCallQuality");

    ims_VoWiFiCallQuality *msg_data =
        (ims_VoWiFiCallQuality *)qcril_malloc(sizeof(ims_VoWiFiCallQuality));

    msg_data->has_quality = TRUE;
    switch (voWiFiCallQualityInfo) {
      case VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_EXCELLENT:
        msg_data->quality = ims_voWiFiQuality_VOWIFI_QUALITY_EXCELLENT;
        break;
      case VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_FAIR:
        msg_data->quality = ims_voWiFiQuality_VOWIFI_QUALITY_FAIR;
        break;
      case VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_BAD:
        msg_data->quality = ims_voWiFiQuality_VOWIFI_QUALITY_BAD;
        break;
      case VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_NONE:
        msg_data->quality = ims_voWiFiQuality_VOWIFI_QUALITY_NONE;
        break;
      case VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_INVALID:
      default:
        msg_data->has_quality = FALSE;
        break;
    }

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_CallModify), msg_data,
                                        ims_MsgId_UNSOL_VOWIFI_CALL_QUALITY, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onSupplementaryServiceIndication(
      const ::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult &ss) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onSupplementaryServiceIndication");
    StkCcUnsolSsResult *msg_data = (StkCcUnsolSsResult *)qcril_malloc(sizeof(StkCcUnsolSsResult));
    //convertHidlToProtoStkCcUnsolSsResult(ss, *msg_data);
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(StkCcUnsolSsResult), msg_data,
                                        ims_MsgId_UNSOL_ON_SS, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onImsSmsStatusReport(
      const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport &smsStatusReport) {
    return Void();
  }
  ::android::hardware::Return<void> onIncomingImsSms(
      const ::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms &imsSms) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onIncomingImsSms");

    utf_ims_mt_sms_t* smsMsg = (utf_ims_mt_sms_t*)qcril_malloc(sizeof(utf_ims_mt_sms_t));
    assert(smsMsg != nullptr);
    convertHidlToInternalIncomingSmsResult(imsSms, *smsMsg);

    send_i_ims_radio_enqueue_ims_expect(sizeof(utf_ims_mt_sms_t), smsMsg,
                                        ims_MsgId_UNSOL_INCOMING_IMS_SMS, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onVopsChanged(bool isVopsEnabled) { return Void(); }
  ::android::hardware::Return<void> onIncomingCallAutoRejected(
      const ::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo
          &autoCallRejectionInfo) {
    return Void();
  }
  ::android::hardware::Return<void> onVoiceInfoChanged(
      ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo voiceInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onVoiceInfoChanged");
    ims_VoiceInfoData *msg_data =
          (ims_VoiceInfoData*)qcril_malloc(sizeof(ims_VoiceInfoData));
    msg_data->voiceInfo = (ims_VoiceInfo)voiceInfo;
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_VoiceInfoData), msg_data,
                                        ims_MsgId_UNSOL_VOICE_INFO, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onMultiIdentityRegistrationStatusChange(
      const ::android::hardware::hidl_vec<
          ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal> &info) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onMultiIdentityRegistrationStatusChange");
    ims_MultiIdentityStatus *msg_data =
          (ims_MultiIdentityStatus*)qcril_malloc(sizeof(ims_MultiIdentityStatus));
    V1_4::utils::convertMultiIdentityLineInfo(info, *msg_data);
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_MultiIdentityStatus), msg_data,
                                        ims_MsgId_UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onMultiIdentityInfoPending() {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onMultiIdentityInfoPending");
    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(0, nullptr, ims_MsgId_UNSOL_MULTI_IDENTITY_PENDING_INFO,
                                        -1, ril_utf_ims_indication, ims_Error_E_SUCCESS);
    return Void();
  }
  ::android::hardware::Return<void> onIncomingCallAutoRejected_1_5(
      const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo
          &autoCallRejectionInfo) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onIncomingCallAutoRejected_1_5");

    ims_AutoCallRejectionInfo *msg_data =
              (ims_AutoCallRejectionInfo *)qcril_malloc(sizeof(ims_AutoCallRejectionInfo));
    V1_5::utils::convertHidlToProtoAutoCallRejectionInfo(autoCallRejectionInfo, *msg_data);

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_AutoCallRejectionInfo), msg_data,
                                        ims_MsgId_UNSOL_AUTO_CALL_REJECTION_IND, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }

  ::android::hardware::Return<void> onModemSupportsWfcRoamingModeConfiguration(
      bool wfcRoamingConfigurationSupport) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onModemSupportsWfcRoamingModeConfiguration");

    ims_WfcRoamingConfigSupportInfo *msg_data =
        (ims_WfcRoamingConfigSupportInfo *)qcril_malloc(sizeof(ims_WfcRoamingConfigSupportInfo));
    msg_data->wfcRoamingConfigSupport = true;

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_WfcRoamingConfigSupportInfo), msg_data,
                                        ims_MsgId_UNSOL_WFC_ROAMING_MODE_CONFIG_SUPPORT, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }

  ::android::hardware::Return<void> onUssdMessageFailed(
      ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType type,
      const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorDetails) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: onUssdMessageFailed");

    ims_UssdMessageFailedInfo *msg_data =
        (ims_UssdMessageFailedInfo *)qcril_malloc(sizeof(ims_UssdMessageFailedInfo));
    assert(msg_data != nullptr);

    switch(type) {
      case ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOTIFY:
        msg_data->type = ims_UssdModeType_NOTIFY;
        break;
      case ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::REQUEST:
        msg_data->type = ims_UssdModeType_REQUEST;
        break;
      case ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_RELEASE:
        msg_data->type = ims_UssdModeType_NW_RELEASE;
        break;
      case ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::LOCAL_CLIENT:
        msg_data->type = ims_UssdModeType_LOCAL_CLIENT;
        break;
      case ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOT_SUPPORTED:
        msg_data->type = ims_UssdModeType_NOT_SUPPORTED;
        break;
      case ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_TIMEOUT:
        msg_data->type = ims_UssdModeType_NW_TIMEOUT;
        break;
    }
    convertHidlToProtoSipErrorInfo(errorDetails, msg_data->errorDetails);

    // Decode message and enqueue
    send_i_ims_radio_enqueue_ims_expect(sizeof(ims_UssdMessageFailedInfo), msg_data,
                                        ims_MsgId_UNSOL_USSD_FAILED, -1,
                                        ril_utf_ims_indication, ims_Error_E_SUCCESS);

    return Void();
  }
};

static sp<IImsRadioResponseClient> mRespClient;
static sp<IImsRadioIndicationClient> mIndClient;

void reset_i_ims_radio() {
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: reset_i_ims_radio");

  ::android::sp<vendor::qti::hardware::radio::ims::V1_6::IImsRadio> utfImsRadio =
    vendor::qti::hardware::radio::ims::V1_6::IImsRadio::getService("imsradio0");
  if (utfImsRadio != nullptr) {
    hidl_death_recipient *deathRecipient = dynamic_cast<hidl_death_recipient *>(utfImsRadio.get());
    deathRecipient->serviceDied(0, utfImsRadio);
  }
}

void connect_to_i_ims_radio() {
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: connect_to_i_ims_radio");

  while (1) {
    ::android::sp<vendor::qti::hardware::radio::ims::V1_6::IImsRadio> utfImsRadio =
      vendor::qti::hardware::radio::ims::V1_6::IImsRadio::getService("imsradio0");
    if (utfImsRadio != nullptr) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: calling setCallback");
      mRespClient = new IImsRadioResponseClient();
      mIndClient = new IImsRadioIndicationClient();

      utfImsRadio->setCallback(mRespClient, mIndClient);
      break;
    }
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: get(imsradio0) returned null");
    usleep(1);
  }
}

//extern "C"
void send_i_ims_radio_request(ims_MsgType type, ims_MsgId msgId, uint32_t token, void *msg) {
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: send_i_ims_radio_request");

  ::android::sp<vendor::qti::hardware::radio::ims::V1_6::IImsRadio> utfImsRadio =
    vendor::qti::hardware::radio::ims::V1_6::IImsRadio::getService("imsradio0");
  if (utfImsRadio != nullptr) {
    switch (msgId) {
      case ims_MsgId_REQUEST_DIAL: {
        V1_4::DialRequest req = {};
        // DialRequest req = {};
        ims_Dial *i_req = (ims_Dial *)msg;

        V1_4::utils::convertProtoToHidlDial(*i_req, req);
        if (i_req->isEmergencyDial) {
          ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory> categories = i_req->emergencyDialExtra.categories;
          ::android::hardware::hidl_vec<::android::hardware::hidl_string> urns;
          urns.resize(0);
          ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute route = (::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute)(i_req->emergencyDialExtra.route);
          bool hasKnownUserIntentEmergency = i_req->emergencyDialExtra.hasKnownUserIntentEmergency;
          bool isTesting = i_req->emergencyDialExtra.isTesting;
          utfImsRadio->emergencyDial(token, req, categories, urns, route, hasKnownUserIntentEmergency, isTesting);
        } else {
          utfImsRadio->dial_1_4(token, req);
        }
        // utfImsRadio->dial(token, req);
      } break;
      case ims_MsgId_REQUEST_ADD_PARTICIPANT: {
        V1_4::DialRequest req = {};
        ims_Dial *i_req = (ims_Dial *)msg;

        V1_4::utils::convertProtoToHidlDial(*i_req, req);
        utfImsRadio->addParticipant_1_4(token, req);
      } break;
      case ims_MsgId_REQUEST_ANSWER: {
        ims_Answer *i_req = (ims_Answer *)msg;
        CallType callType = CallType::CALL_TYPE_INVALID;
        if (i_req->has_call_type) {
          callType = convertProtoToHidlCallType(i_req->call_type);
        }
        IpPresentation presentation = IpPresentation::IP_PRESENTATION_INVALID;
        if (i_req->has_presentation) {
          presentation = convertProtoToHidlPresentation(i_req->presentation);
        }
        RttMode mode = RttMode::RTT_MODE_INVALID;
        if (i_req->has_rttMode) {
          mode = convertProtoToHidlRttMode(i_req->rttMode);
        }
        utfImsRadio->answer(token, callType, presentation, mode);
      } break;
      case ims_MsgId_REQUEST_HANGUP: {
        V1_3::HangupRequestInfo req = {};
        ims_Hangup *i_req = (ims_Hangup *)msg;

        convertProtoToHidlHangup(*i_req, req);
        utfImsRadio->hangup_1_3(token, req);
      } break;
      case ims_MsgId_REQUEST_IMS_REGISTRATION_STATE: {
        utfImsRadio->getImsRegistrationState(token);
      } break;
      case ims_MsgId_REQUEST_SET_SERVICE_STATUS: {
        std::vector<V1_6::ServiceStatusInfo> req;
        ims_SetServiceStatusType *i_req = (ims_SetServiceStatusType *)msg;
        if (i_req->has_volteEnabled) {
          V1_6::ServiceStatusInfo ssInfo = {};
          ssInfo.callType = V1_0::CallType::CALL_TYPE_VOICE;
          ssInfo.accTechStatus.resize(1);
          ssInfo.accTechStatus[0].networkMode = V1_6::RadioTechType::RADIO_TECH_LTE;
          ssInfo.accTechStatus[0].status = i_req->volteEnabled ? V1_0::StatusType::STATUS_ENABLED
                                                               : V1_0::StatusType::STATUS_DISABLED;
          req.push_back(ssInfo);
        }
        if (i_req->has_videoTelephonyEnabled) {
          V1_6::ServiceStatusInfo ssInfo = {};
          ssInfo.callType = V1_0::CallType::CALL_TYPE_VT;
          ssInfo.accTechStatus.resize(1);
          ssInfo.accTechStatus[0].networkMode = V1_6::RadioTechType::RADIO_TECH_LTE;
          ssInfo.accTechStatus[0].status = i_req->videoTelephonyEnabled
                                               ? V1_0::StatusType::STATUS_ENABLED
                                               : V1_0::StatusType::STATUS_DISABLED;
          req.push_back(ssInfo);
        }
        if (i_req->has_wifiCallingEnabled) {
          V1_6::ServiceStatusInfo ssInfo = {};
          ssInfo.callType = V1_0::CallType::CALL_TYPE_VOICE;
          ssInfo.accTechStatus.resize(1);
          ssInfo.accTechStatus[0].networkMode = V1_6::RadioTechType::RADIO_TECH_WIFI;
          ssInfo.accTechStatus[0].status = i_req->wifiCallingEnabled
                                               ? V1_0::StatusType::STATUS_ENABLED
                                               : V1_0::StatusType::STATUS_DISABLED;
          req.push_back(ssInfo);
        }
        if (i_req->has_utEnabled) {
          V1_6::ServiceStatusInfo ssInfo = {};
          ssInfo.callType = V1_0::CallType::CALL_TYPE_UT;
          ssInfo.accTechStatus.resize(1);
          ssInfo.accTechStatus[0].networkMode = V1_6::RadioTechType::RADIO_TECH_LTE;
          ssInfo.accTechStatus[0].status = i_req->utEnabled ? V1_0::StatusType::STATUS_ENABLED
                                                            : V1_0::StatusType::STATUS_DISABLED;
          req.push_back(ssInfo);
        }
        utfImsRadio->setServiceStatus_1_6(token, req);
      } break;
      case ims_MsgId_REQUEST_QUERY_CALL_FORWARD_STATUS: {
        CallForwardInfo req = {};
        ims_CallForwardInfoList *i_req = (ims_CallForwardInfoList *)msg;
        ims_CallForwardInfoList_CallForwardInfo **ptr =
            (ims_CallForwardInfoList_CallForwardInfo **)i_req->info.arg;
        if (ptr && ptr[0]) {
          convertProtoToHidlCallForwardInfo(*ptr[0], req);
        }
        utfImsRadio->queryCallForwardStatus(token, req);
      } break;
      case ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS: {
        uint32_t extToken =
            (ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS << 16 | (0x0000FFFF & token));

        CallForwardInfo req = {};
        ims_CallForwardInfoList *i_req = (ims_CallForwardInfoList *)msg;
        ims_CallForwardInfoList_CallForwardInfo **ptr =
            (ims_CallForwardInfoList_CallForwardInfo **)i_req->info.arg;
        if (ptr && ptr[0]) {
          convertProtoToHidlCallForwardInfo(*ptr[0], req);
        }
        utfImsRadio->setCallForwardStatus(extToken, req);
      } break;
      case ims_MsgId_REQUEST_QUERY_CALL_WAITING: {
        uint32_t serviceClass = 0;
        ims_ServiceClass *i_req = (ims_ServiceClass *)msg;
        if (i_req->has_service_Class) {
          serviceClass = i_req->service_Class;
        }
        utfImsRadio->getCallWaiting(token, serviceClass);
      } break;
      case ims_MsgId_REQUEST_SET_SUPP_SVC_NOTIFICATION: {
        ServiceClassStatus status = ServiceClassStatus::INVALID;
        ims_SuppSvcStatus *i_req = (ims_SuppSvcStatus *)msg;
        if (i_req->has_status) {
          if (i_req->status == ims_ServiceClassStatus_ENABLED) {
            status = ServiceClassStatus::ENABLED;
          } else {
            status = ServiceClassStatus::DISABLED;
          }
        }
        utfImsRadio->setSuppServiceNotification(token, status);
      } break;
      case ims_MsgId_REQUEST_EXPLICIT_CALL_TRANSFER: {
        ExplicitCallTransferInfo ectInfo = {};
        ims_ExplicitCallTransfer *i_req = (ims_ExplicitCallTransfer *)msg;
        convertProtoToHidlEctInfo(ectInfo, *i_req);
        utfImsRadio->explicitCallTransfer(token, ectInfo);
      } break;
      case ims_MsgId_REQUEST_DEFLECT_CALL: {
        DeflectRequestInfo deflectInfo = { .connIndex=INT32_MAX, .number=""};
        ims_DeflectCall *i_req = (ims_DeflectCall *)msg;
        convertProtoToHidlDeflectRequestInfo(deflectInfo, *i_req);
        utfImsRadio->deflectCall(token, deflectInfo);
      } break;
      case ims_MsgId_REQUEST_DTMF: {
        DtmfInfo dtmfInfo = {};
        ims_Dtmf *i_req = (ims_Dtmf *)msg;
        if (i_req->dtmf.arg) {
          dtmfInfo.dtmf = (char *)(i_req->dtmf.arg);
        }
        utfImsRadio->sendDtmf(token, dtmfInfo);
      } break;
      case ims_MsgId_REQUEST_DTMF_START: {
        DtmfInfo dtmfInfo = {};
        ims_Dtmf *i_req = (ims_Dtmf *)msg;
        if (i_req->dtmf.arg) {
          dtmfInfo.dtmf = (char *)(i_req->dtmf.arg);
        }
        utfImsRadio->startDtmf(token, dtmfInfo);
      } break;
      case ims_MsgId_REQUEST_DTMF_STOP: {
        utfImsRadio->stopDtmf(token);
      } break;
      case ims_MsgId_REQUEST_IMS_REG_STATE_CHANGE: {
        RegState req = RegState::INVALID;
        ims_Registration *i_req = (ims_Registration *)msg;
        if (i_req->has_state) {
          req = convertProtoToHidlRegState(i_req->state);
        }
        utfImsRadio->requestRegistrationChange(token, req);
      } break;
      case ims_MsgId_REQUEST_HOLD: {
        ims_Hold *i_req = (ims_Hold *)msg;
        uint32_t callId = INT32_MAX;
        if (i_req->has_callId) {
          callId = i_req->callId;
        }
        utfImsRadio->hold(token, callId);
      } break;
      case ims_MsgId_REQUEST_RESUME: {
        ims_Resume *i_req = (ims_Resume *)msg;
        uint32_t callId = INT32_MAX;
        if (i_req->has_callId) {
          callId = i_req->callId;
        }
        utfImsRadio->resume(token, callId);
      } break;
      case ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO: {
        ims_GeoLocationInfo *i_req = (ims_GeoLocationInfo *)msg;
        V1_2::AddressInfo addressInfo = {};
        convertProtoToHidlGeolocation(*i_req, addressInfo);
        utfImsRadio->sendGeolocationInfo_1_2(token, i_req->lat, i_req->lon, addressInfo);
      } break;
      case ims_MsgId_REQUEST_CONFERENCE: {
        utfImsRadio->conference(token);
      } break;
      case ims_MsgId_REQUEST_QUERY_CLIP: {
        utfImsRadio->getClip(token);
      } break;
      case ims_MsgId_REQUEST_GET_CLIR: {
        utfImsRadio->getClir(token);
      } break;
      case ims_MsgId_REQUEST_GET_COLR: {
        utfImsRadio->getColr(token);
      } break;
      case ims_MsgId_REQUEST_SET_CLIR: {
        ClirInfo req = {};
        ims_Clir *i_req = (ims_Clir *)msg;
        convertProtoToHidlClirInfo(*i_req, req);
        utfImsRadio->setClir(token, req);
      } break;
      case ims_MsgId_REQUEST_SET_CALL_WAITING: {
        uint32_t extToken = (ims_MsgId_REQUEST_SET_CALL_WAITING << 16 | (0x0000FFFF & token));
        ServiceClassStatus status = ServiceClassStatus::INVALID;
        uint32_t serviceClass = INT32_MAX;
        ims_CallWaitingInfo *i_req = (ims_CallWaitingInfo *)msg;
        if (i_req->has_service_class && i_req->service_class.has_service_Class) {
          serviceClass = i_req->service_class.service_Class;
        }
        if (i_req->has_service_status) {
          if (i_req->service_status == ims_ServiceClassStatus_ENABLED) {
            status = ServiceClassStatus::ENABLED;
          } else {
            status = ServiceClassStatus::DISABLED;
          }
        }
        utfImsRadio->setCallWaiting(extToken, status, serviceClass);
      } break;
      case ims_MsgId_REQUEST_SET_COLR: {
        uint32_t extToken = (ims_MsgId_REQUEST_SET_COLR << 16 | (0x0000FFFF & token));
        V1_3::ColrInfo req;
        ims_Colr *i_req = (ims_Colr *)msg;
        convertProtoToHidlColrInfo(*i_req, req);
        utfImsRadio->setColr_1_3(extToken, req);
      } break;
      case ims_MsgId_REQUEST_SUPP_SVC_STATUS: {
        uint32_t extToken = (ims_MsgId_REQUEST_SUPP_SVC_STATUS << 16 | (0x0000FFFF & token));

        ims_SuppSvcRequest *i_req = (ims_SuppSvcRequest *)msg;
        uint32_t operationType = INT32_MAX;
        if (i_req->has_operationType) {
          operationType = i_req->operationType;
        }
        FacilityType facilityType = FacilityType::FACILITY_INVALID;
        if (i_req->has_facilityType) {
          facilityType = convertProtoToHidlFacilityType(i_req->facilityType);
        }
        CbNumListInfo cbNumListInfo = {};
        if (i_req->has_cbNumListType) {
          convertProtoToHidlNumListInfo(i_req->cbNumListType, cbNumListInfo);
        }
        hidl_string password = i_req->password;
        utfImsRadio->suppServiceStatus_1_6(extToken, operationType, facilityType, cbNumListInfo,
            password);
      } break;
      case ims_MsgId_REQUEST_QUERY_SERVICE_STATUS: {
        utfImsRadio->queryServiceStatus(token);
      } break;
      case ims_MsgId_REQUEST_SEND_UI_TTY_MODE: {
        ims_TtyNotify *i_req = (ims_TtyNotify *)msg;
        TtyInfo ttyInfo = {.mode = TtyMode::TTY_MODE_INVALID};
        if (i_req->has_mode) {
          ttyInfo.mode = convertProtoToHidlTtyMode(i_req->mode);
        }
        utfImsRadio->setUiTtyMode(token, ttyInfo);
      } break;
      case ims_MsgId_REQUEST_SET_IMS_CONFIG: {
        ims_ConfigMsg *i_req = (ims_ConfigMsg *)msg;
        V1_6::ConfigInfo config = {};
        V1_6::utils::convertProtoToHidlConfig(*i_req, config);
        utfImsRadio->setConfig_1_6(token, config);
      } break;
      case ims_MsgId_REQUEST_GET_IMS_CONFIG: {
        ims_ConfigMsg *i_req = (ims_ConfigMsg *)msg;
        ConfigInfo config = {};
        convertProtoToHidlConfig(*i_req, config);
        utfImsRadio->getConfig(token, config);
      } break;
      case ims_MsgId_REQUEST_MODIFY_CALL_INITIATE: {
        ims_CallModify *i_req = (ims_CallModify *)msg;
        CallModifyInfo modifyInfo = {};
        convertProtoToHidlCallModifyInfo(*i_req, modifyInfo);
        utfImsRadio->modifyCallInitiate(token, modifyInfo);
      } break;
      case ims_MsgId_REQUEST_MODIFY_CALL_CONFIRM: {
        ims_CallModify *i_req = (ims_CallModify *)msg;
        CallModifyInfo modifyInfo = {};
        convertProtoToHidlCallModifyInfo(*i_req, modifyInfo);
        utfImsRadio->modifyCallConfirm(token, modifyInfo);
      } break;
      case ims_MsgId_REQUEST_CANCEL_MODIFY_CALL: {
        ims_CancelModifyCall *i_req = (ims_CancelModifyCall *)msg;
        uint32_t callId = i_req->callId;
        utfImsRadio->cancelModifyCall(token, callId);
      } break;
      case ims_MsgId_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE: {
        utfImsRadio->exitEmergencyCallbackMode(token);
      } break;
      case ims_MsgId_REQUEST_SEND_SMS: {
        auto i_req = static_cast<RIL_IMS_SMS_Message*>(msg);
        V1_2::ImsSmsMessage imsSms = {};
        convertInternalToHidlImsSendSms(*i_req, imsSms);
        utfImsRadio->sendImsSms(token, imsSms);
      } break;
      case ims_MsgId_REQUEST_ACK_SMS: {
        auto i_req = static_cast<utf_ims_ack_sms_t*>(msg);
        utfImsRadio->acknowledgeSms_1_5(token, i_req->message_id,
            static_cast<V1_5::ImsSmsDeliverStatusResult>(i_req->delivery_result));
      } break;
      case ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES: {
        ims_MultiIdentityStatus *i_req = (ims_MultiIdentityStatus *)msg;
        hidl_vec<V1_4::MultiIdentityLineInfoHal> lines;
        V1_4::utils::convertMultiIdentityLineInfo(*i_req, lines);
        utfImsRadio->registerMultiIdentityLines(token, lines);
      } break;
      case ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO: {
        ims_VirtualLineInfoReq *i_req = (ims_VirtualLineInfoReq *)msg;
        hidl_string msisdn = i_req->msisdn;
        utfImsRadio->queryVirtualLineInfo(token, msisdn);
      } break;
      case ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG: {
        utfImsRadio->getImsSubConfig(token);
      } break;
      case ims_MsgId_REQUEST_SEND_RTT_MSG: {
        ims_RttMessage *i_req = (ims_RttMessage *)msg;
        hidl_string rttMessage;
        convertProtoToHidlString((const char*)(i_req->rttMessage.arg),rttMessage);
        utfImsRadio->sendRttMessage(token, rttMessage);
        break;
      }
      default:
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: unrecognized msgId: %d\n", msgId);
        break;
    }
  } else {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: get(imsradio0) returned null");
  }
}
