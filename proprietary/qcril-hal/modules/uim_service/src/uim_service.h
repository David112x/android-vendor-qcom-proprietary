/******************************************************************************
@file    uim_service.h
@brief   qcril uim service

DESCRIPTION
Implements the server side of the IUim interface.
Handles RIL requests and responses and indications to be received
and sent to client respectively

---------------------------------------------------------------------------

Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
---------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "framework/Module.h"
#include <framework/QcrilInitMessage.h>
#include "modules/uim_service/UimGetRilPropertySyncMsg.h"
#include "modules/uim_service/UimSetRilPropertySyncMsg.h"

class UimServiceModule : public Module
{
  public:
    UimServiceModule();
    ~UimServiceModule();
    void init();
    void handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg);
    void handleQcrilUimGetProperty(std::shared_ptr<UimGetRilPropertySyncMsg> msg);
    void handleQcrilUimSetProperty(std::shared_ptr<UimSetRilPropertySyncMsg> msg);
};
