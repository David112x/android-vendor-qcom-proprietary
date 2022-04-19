/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "DSDModemEndPoint.h"
#include "WDSModemEndPoint.h"
#include "AttachProfile.h"
#include "modules/android/version.h"
#include "telephony/ril.h"
#include "request/SetInitialAttachApnRequestMessage.h"
#include "request/SetDataProfileRequestMessage.h"
#include "NasModemEndPointHelper.h"
#include "MessageCommon.h"

#ifndef QCRIL_RIL_VERSION
#error "undefined QCRIL_RIL_VERSION!"
#endif

using namespace rildata;

class ProfileHandler
{
public:
  struct RilRespParams
  {
    qcril_instance_id_e_type instance_id;
    qcril_modem_id_e_type modem_id;
    qcril_evt_e_type event_id;
    RIL_Token t;
  };
private:
  bool mApAssistMode;
  static std::unique_ptr<NasModemEndPointHelper> nas_helper;
  std::shared_ptr<WDSModemEndPoint> wds_endpoint;
  std::shared_ptr<DSDModemEndPoint> dsd_endpoint;
  std::shared_ptr<std::list<uint16_t>> attach_list;
  AttachProfile::params_t ril_params;
  enum Request
  {
   SET_INITIAL_ATTACH,
   SET_DATA_PROFILE
  };

  /**
   * @brief      Is action supported as part of the attach procedure
   */
  enum AttachResult
  {
    NO_DETACH_NEEDED,
    DETACH_NEEDED,
    GENERIC_FAILURE
  };

  static RilRespParams m_resp_params;

  /**
   * @brief      Common implementation agnostic of the version of RIL
   *
   * @return     { description_of_the_return_value }
   */
  bool setAttachListCommon();
  /**
   * @brief      If current profile is in profiles found from attach list, then no
   *             attach required.
   *
   * @param[in]  attach_list  The attach list
   *
   * @return     True if attach required, False otherwise.
   *
   *  NOTE: This uses uint16_t based on the QMI response type
   */
  bool isAttachRequired(const std::list<uint16_t>& attach_list, AttachProfile &ril_profile);

  /**
   * @brief      Sets the attach list.
   *             Additionally, sets action if it is supported.
   *
   * @param[in]  attach_list  The attach list
   *
   * @return     SUPPORTED if action is supported by QMI service
   *             UNSUPPORTED if action is not supported by QMI service
   */
  AttachResult setAttachList();

  /**
   * @brief      Sends a request resp.
   *
   * @return     { description_of_the_return_value }
   */
  static void sendReqResp(const RilRespParams& resp, RIL_Errno e);

#if (QCRIL_RIL_VERSION < 15)
  uint8_t copyParams (const SetInitialAttachApnRequestMessage::base_attach_params_t& p,
                      AttachProfile::params_t &params);
  void dumpParams(const SetInitialAttachApnRequestMessage::base_attach_params_t& params);
#else
  uint8_t copyParams (const SetInitialAttachApnRequestMessage::v15_attach_params_t& p,
                      AttachProfile::params_t &params);
  void dumpParams(const SetInitialAttachApnRequestMessage::v15_attach_params_t& p);
#endif
  void initialAttachHandler();

public:

#if (QCRIL_RIL_VERSION < 15)
  void handleInitialAttachRequest(
              const SetInitialAttachApnRequestMessage::base_attach_params_t& p,
              const RilRespParams& resp_params);
#else
  void handleInitialAttachRequest(
              const SetInitialAttachApnRequestMessage::v15_attach_params_t& p,
              const RilRespParams& resp_params);
  void handleSetDataProfileRequestMessage(std::shared_ptr<Message> msg);
  uint8_t copyParams (const DataProfileInfo_t& p, BaseProfile::params_t &params);
  void dumpParams(const DataProfileInfo_t& p);

#endif

  void qcril_data_nas_detach_attach_ind_hdlr(uint8_t domainPrefValid,
                         nas_srv_domain_pref_enum_type_v01 domainPref );
  static void handleWdsUnSolInd(const wds_set_lte_attach_pdn_list_action_result_ind_msg_v01 *res);
  static void processNasPsAttachDetachResp(RIL_Errno* ret);
  void releaseQdpAttachProfile(void);
  void init(bool apAssistMode);
  ProfileHandler();
  ~ProfileHandler() {};
  bool inited = false;
};
