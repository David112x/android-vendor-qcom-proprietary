/******************************************************************************

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

#******************************************************************************/
#ifdef FEATURE_DATA_LQE
#pragma once
#include "over_the_top_v01.h"
#include "modules/qmi/ModemEndPoint.h"
#include "modules/qmi/ModemEndPointModule.h"
#include "MessageCommon.h"

using namespace rildata;

class OTTModemEndPointModule : public ModemEndPointModule
{
private:

  qmi_idl_service_object_type getServiceObject() override;

public:
  OTTModemEndPointModule(string name, ModemEndPoint& owner);
  virtual ~OTTModemEndPointModule();
  void init();
};
#endif /* FEATURE_DATA_LQE */
