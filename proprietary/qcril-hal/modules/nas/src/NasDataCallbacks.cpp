/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "modules/nas/qcril_qmi_nas.h"
#include "framework/Log.h"
#include "framework/legacy.h"
#include "modules/nas/NasDataCallbacks.h"
#include "modules/nas/NasModule.h"
#include "RequestDdsSwitchMessage.h"
#include <string>

//NasDataGetDdsSubIdCallback
void NasDataGetDdsSubIdCallback::onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
        std::shared_ptr<DDSSubIdInfo> responseDataPtr) {
   RIL_Errno res = RIL_E_INTERNAL_ERR;
   boolean send_resp = FALSE;
   DDSSubIdInfo *dds_sub;
   DDSSubIdInfo new_dds_sub;

   if(status == Message::Callback::Status::SUCCESS) {
      Log::getInstance().d("[NasDataGetDdsSubIdCallback]: Callback[msg = " +
        solicitedMsg->dump() + "] executed. client data = " +
        mClientToken + " status = Message::Callback::Status::SUCCESS");

      dds_sub = (responseDataPtr.get());
      res =  qcril_qmi_nas_update_dds_sub_id(dds_sub);

      if(from_allow_data) {
        //request is made from allow data
        Log::getInstance().d("[NasDataGetDdsSubIdCallback]: request is made from allow data");

        if(qcril_qmi_nas_is_dds_change()) {
#ifndef QMI_RIL_UTF
          //send DDS switch request
          qcril_qmi_nas_dds_to_be_updated(&new_dds_sub);
          std::shared_ptr<rildata::RequestDdsSwitchMessage> msg =
              std::make_shared<rildata::RequestDdsSwitchMessage>(new_dds_sub);
          if(msg) {
              NasDataRequestDdsSwitchCallback cb("set-cb-token", mMsgToken);
              msg->setCallback(&cb);
              msg->dispatch();

              //response will be sent in the callback.
              res = RIL_E_SUCCESS;
          } else {
              res = RIL_E_NO_MEMORY;
              send_resp = TRUE;
          }
#endif
        } else {
          send_resp = TRUE;
          res = RIL_E_SUCCESS;
          Log::getInstance().d("[NasDataGetDdsSubIdCallback] Already it is DDS sub, no need to update to modem");
      }
    } else {
      qcril_qmi_nas_notify_embms_status();
    }
  } else {
    Log::getInstance().d("[NasDataGetDdsSubIdCallback]: Callback[msg = " +
      solicitedMsg->dump() + "] executed. client data = " +
      mClientToken + " FAILURE !!");
  }

  if( from_allow_data && mMsgToken != NasDataGetDdsSubIdCallback::INVALID_MSG_TOKEN
        && (send_resp || res != RIL_E_SUCCESS)) {
    // send resp
    Log::getInstance().d("[NasDataGetDdsSubIdCallback]: PS attach/detach response");

    auto msg = getNasModule().getPendingMessageList().extract(static_cast<uint16_t>(mMsgToken));
    if (msg) {
        auto rilMsg = std::static_pointer_cast<QcRilRequestMessage>(msg);
        auto respData = std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr);
        rilMsg->sendResponse(rilMsg, Message::Callback::Status::SUCCESS, respData);
    }
  }
}

Message::Callback *NasDataGetDdsSubIdCallback::clone() {
  NasDataGetDdsSubIdCallback *clone = new NasDataGetDdsSubIdCallback(mClientToken, mMsgToken, from_allow_data);
  return clone;
}

//NasDataRequestDdsSwitchCallback
void NasDataRequestDdsSwitchCallback::onResponse(
    std::shared_ptr<Message> solicitedMsg, Status status,
    std::shared_ptr<RIL_Errno> responseDataPtr) {

    RIL_Errno   res = RIL_E_INTERNAL_ERR;

    if (status == Message::Callback::Status::SUCCESS) {
      Log::getInstance().d("[NasDataRequestDdsSwitchCallback]: Callback[msg = " +
      solicitedMsg->dump() + "] executed. client data = " +
      mClientToken + " status = Message::Callback::Status::SUCCESS");
      res = *(responseDataPtr.get());

      if(res != RIL_E_SUCCESS)
      {
        Log::getInstance().d("[NasDataRequestDdsSwitchCallback]: set dds through data layer FAILED");
      }
      else
      {
         res = qcril_qmi_nas_set_dds_start_timer();
      }
  } else {
    Log::getInstance().d("[NasDataRequestDdsSwitchCallback]: Callback[msg = " +
    solicitedMsg->dump() + "] executed. client data = " +
    mClientToken + " FAILURE !!");
  }

  if(res != RIL_E_SUCCESS) {
     Log::getInstance().d("[NasDataGetDdsSubIdCallback]: PS attach/detach response");
     qcril_qmi_nas_set_attch_state(QMI_RIL_NAS_ATTCH_NONE);

     if (mMsgToken != NasDataGetDdsSubIdCallback::INVALID_MSG_TOKEN) {
        auto msg = getNasModule().getPendingMessageList().extract(static_cast<uint16_t>(mMsgToken));
        if (msg) {
            auto rilMsg = std::static_pointer_cast<QcRilRequestMessage>(msg);
            auto respData = std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr);
            rilMsg->sendResponse(rilMsg, Message::Callback::Status::SUCCESS, respData);
        }
     }
   }
}

Message::Callback *NasDataRequestDdsSwitchCallback::clone() {
  NasDataRequestDdsSwitchCallback *clone = new NasDataRequestDdsSwitchCallback(mClientToken, mMsgToken);
  return clone;
}
