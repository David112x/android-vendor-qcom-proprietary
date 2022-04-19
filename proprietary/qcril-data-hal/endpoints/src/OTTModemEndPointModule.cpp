/******************************************************************************

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

#******************************************************************************/
#ifdef FEATURE_DATA_LQE
#include "OTTModemEndPointModule.h"
#include <modules/qmi/QmiIndMessage.h>

using namespace rildata;

OTTModemEndPointModule::OTTModemEndPointModule(string name,
    ModemEndPoint &owner) : ModemEndPointModule(name, owner)
{
  mServiceObject = nullptr;

  using std::placeholders::_1;
}

OTTModemEndPointModule::~OTTModemEndPointModule() {}

void OTTModemEndPointModule::init()
{
  ModemEndPointModule::init();
  mStackId[0] = 0;
}

qmi_idl_service_object_type OTTModemEndPointModule::getServiceObject()
{
  return ott_get_service_object_v01();
}
#endif /* FEATURE_DATA_LQE */
