/******************************************************************************
#  @file    UimModem.h
#  @brief   Base class header file for UIM module
#
#  ---------------------------------------------------------------------------
#
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#  ---------------------------------------------------------------------------
#******************************************************************************/

#pragma once
#include "framework/Module.h"
#include "framework/PendingMessageList.h"
#include "modules/gstk/CatModemEndPoint.h"
#include "modules/gstk/GstkReportSTKIsRunningMsg.h"
#include "modules/gstk/GstkSendTerminalResponseRequestMsg.h"
#include "modules/gstk/GstkSendEnvelopeRequestMsg.h"
#include "modules/gstk/GstkSendSetupCallResponseRequestMsg.h"
#include "modules/gstk/GstkQmiCatRequestMsg.h"
#include "modules/gstk/GstkQmiCatIndicationMsg.h"
#include "modules/gstk/GstkQmiCatResponseMsg.h"
#include "modules/uim/UimCardStatusIndMsg.h"
#include "GstkTimerExpiryMsg.h"
#include<queue>

class GstkModule : public Module
{
  private:
    bool                                               mReady;
    std::shared_ptr<CatModemEndPoint>                  mCatModemEndPoint;
    std::queue<std::shared_ptr<GstkQmiCatRequestMsg>>  mQmiPendingList;
    PendingMessageList                                 gstk_req_list;
    uint8_t                                            mQmiAsyncMsgCount;

  public:
    GstkModule();
    ~GstkModule();
    void init();
    void qcrilHalGstkModuleCleanup();
    int GstkSendQmiCatRequest(qcril_gstk_request_type                        msg_id,
                              std::shared_ptr<qcril_gstk_request_data_type>  params_ptr,
                              std::shared_ptr<Message>                       req_ptr);
  private:
    void GstkProcessRilInit(std::shared_ptr<Message> msg_ptr);
    void GstkProcessQmiCatCallback(std::shared_ptr<GstkQmiCatResponseMsg> msg);
    void GstkProcessQmiCatInd(std::shared_ptr<GstkQmiCatIndicationMsg> msg);
    void GstkProcessCardStatusMsg(std::shared_ptr<UimCardStatusIndMsg> msg);
    void GstkProcessStkRunningMsg(std::shared_ptr<GstkReportSTKIsRunningMsg> msg);
    void GstkHandleEnvelopeRequest(std::shared_ptr<GstkSendEnvelopeRequestMsg> req_ptr);
    void GstkHandleTerminalResponseRequest(std::shared_ptr<GstkSendTerminalResponseRequestMsg> req_ptr);
    void GstkHandleSetupCallResponseRequest(std::shared_ptr<GstkSendSetupCallResponseRequestMsg> req_ptr);
    void GstkHandleTimerRequest(std::shared_ptr<Message> msg_ptr);
    void GstkHandleTimerExpiry(std::shared_ptr<GstkTimerExpiryMsg> msg_ptr);
};

GstkModule *getGstkModule();
