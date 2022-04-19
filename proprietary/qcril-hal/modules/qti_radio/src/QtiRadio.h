/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/Module.h"
#include "DataRegistrationStateMessage.h"
#include "DataNrIconTypeIndMessage.h"
#include "modules/nas/Nas5gConnectionIndMessage.h"
#include "modules/nas/Nas5gStatusIndMessage.h"
#include "modules/nas/NasEndcDcnrIndMessage.h"
#include "modules/nas/Nas5gSignalStrengthIndMessage.h"
#include "modules/nas/Nas5gConfigInfoIndMessage.h"
#include "modules/nas/NasUpperLayerIndInfoIndMessage.h"

class QtiRadio : public Module {
  public:
    QtiRadio();
    ~QtiRadio();
    void init();

  private:
      void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
      void handleNas5gConnectionIndMessage(
              std::shared_ptr<Nas5gConnectionIndMessage> msg);
      void handleNas5gStatusIndMessage(
              std::shared_ptr<Nas5gStatusIndMessage> msg);
      void handleNasEndcDcnrIndMessage(
              std::shared_ptr<NasEndcDcnrIndMessage> msg);
      void handleNas5gSignalStrengthIndMessage(
              std::shared_ptr<Nas5gSignalStrengthIndMessage> msg);
      void handleNas5gConfigInfoIndMessage(
              std::shared_ptr<Nas5gConfigInfoIndMessage> msg);
      void handleNasUpperLayerIndInfoIndMessage(
              std::shared_ptr<NasUpperLayerIndInfoIndMessage> msg);
#ifndef QMI_RIL_UTF
      void handleDataNrIconTypeIndMessage(
              std::shared_ptr<rildata::DataNrIconTypeIndMessage> msg);
#endif
};


