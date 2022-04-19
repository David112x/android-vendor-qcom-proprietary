/* ---------------------------------------------------------------------------
Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------------- */
#pragma once
#include "framework/Module.h"
#include <framework/QcrilInitMessage.h>
#include "modules/uim/UimSapStatusIndMsg.h"

class UimRemoteServerModule : public Module
{
  public:
    UimRemoteServerModule();
    ~UimRemoteServerModule();
    void init();
    void handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg);
    void handleSAPInd(std::shared_ptr<UimSapStatusIndMsg> msg);
};
