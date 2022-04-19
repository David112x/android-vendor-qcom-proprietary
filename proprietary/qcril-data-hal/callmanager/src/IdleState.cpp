/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>
#include "framework/Log.h"

#include "CallState.h"
#include "CallManager.h"

#include "modules/nas/NasEmbmsEnableDataConMessage.h"
#include "EmbmsCallHandler.h"

using namespace SM;
using namespace rildata;

void IdleState::enter(void) {
  Log::getInstance().d("[IdleState] enter");
}

void IdleState::exit(void) {
  Log::getInstance().d("[IdleState] exit");
}

int IdleState::handleEvent(const CallEventType &event) {
  Log::getInstance().d("IdleState::handleEvent event id = "
    + std::to_string((int)event.callEvent));
  logBuffer.addLogWithTimestamp("[SM]: " + getName() + " handling " + event.getEventName());

  int nextState = _eIdle;
  CallInfo *pCallInstance = nullptr;
  DsiEventDataType *dsiEvent = nullptr;
  dsi_net_evt_t netEvent = DSI_EVT_INVALID;

  if (event.callEvent == _eDsiEvent) {
    dsiEvent = (DsiEventDataType *)event.callEventData;
    netEvent = dsiEvent->netEvent;
    pCallInstance = (CallInfo *)dsiEvent->eventData;
  }
  else {
    pCallInstance = (CallInfo *)event.callEventData;
  }

  switch (event.callEvent) {
    case  _eConnectEvent:
      nextState = _eConnecting;
      if(pCallInstance->isEmbmsCall())
      {
        Log::getInstance().d("[IdleState] EMBMS call");
        if(DSI_SUCCESS != dsi_embms_enable(pCallInstance->getDsiHandle())) {
          Log::getInstance().d("[IdleState] dsi_embms_enable failed");
          qcril_embms_enable_response_payload_type enableResp;
          memset(&enableResp, 0, sizeof(enableResp));
          enableResp.call_id = pCallInstance->getCid();
          enableResp.cause = RIL_E_GENERIC_FAILURE;;
          enableResp.resp_code = (int32_t)EmbmsDataCallFailCause_t::ERROR_UNKNOWN;

          auto msg = std::make_shared<NasEmbmsEnableDataConMessage>(enableResp);
          msg->dispatch();

          nextState = _eError;
          break;
        }
      }
      else if (DSI_SUCCESS != dsi_start_data_call(pCallInstance->getDsiHandle())) {
        Log::getInstance().d("[IdleState] dsi_start_data_call failed");
        nextState = _eError;
        break;
      }
      break;
    default:
      Log::getInstance().d("[IdleState] Unhandled call event = " + std::to_string(event.callEvent));
      break;
  }

  return nextState;
}
