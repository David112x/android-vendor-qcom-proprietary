/******************************************************************************
#  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <vector>
#include "modules/qmi/ModemEndPoint.h"
#include "DSDModemEndPointModule.h"
#include "framework/Log.h"
#include "DataCommon.h"

class DSDModemEndPoint : public ModemEndPoint
{
private:
  bool mReportLimitedSysIndicationChange;
public:
  static constexpr const char *NAME = "DSDModemEndPoint";
  DSDModemEndPoint() : ModemEndPoint(NAME)
  {
    mModule = new DSDModemEndPointModule("DSDModemEndPointModule", *this);
    mModule->init();
    mReportLimitedSysIndicationChange = false;
    Log::getInstance().d("[DSDModemEndPoint]: xtor");
  }
  ~DSDModemEndPoint()
  {
      Log::getInstance().d("[DSDModemEndPoint]: destructor");
    //mModule->killLooper();
    delete mModule;
    mModule = nullptr;
  }

  void requestSetup(string clientToken, GenericCallback<string>* cb);

  /**
   * @brief      Sets the apn information synchronously
   *
   * @param[in]  apn_name       The apn name
   * @param[in]  apn_type_mask  The apn type mask
   *
   * @return     { description_of_the_return_value }
   */
  boolean isApnTypeFound(const RIL_ApnTypes &apn_type, const uint8_t &apn_types_mask);

  void sendApnInfoQmi( const std::string &apn_name, dsd_apn_type_enum_v01 apn_type);

  Message::Callback::Status setApnInfoSync(const std::string apn_name,
   const uint8_t apn_type_mask);

  Message::Callback::Status sendAPAssistIWLANSupportedSync( );

  Message::Callback::Status registerForSystemStatusSync( );

  Message::Callback::Status registerForCurrentDDSInd( );

  Message::Callback::Status registerForAPAsstIWlanIndsSync(bool toRegister );

  Message::Callback::Status setV2Capabilities( );

  Message::Callback::Status setApnPreferredSystemChangeSync(const std::string apnName, const int32_t prefRat);

  void generateDsdSystemStatusInd();

  Message::Callback::Status getCurrentDDSSync(DDSSubIdInfo &subId);

  Message::Callback::Status triggerDDSSwitchSync(int subId, int &error, dsd_dds_switch_type_enum_v01 switch_type);

  int toggleLimitedSysIndicationChangeReport(bool report);

  void handleSegmentTimeout(std::shared_ptr<Message> m);
};
