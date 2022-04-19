/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "framework/Log.h"
#include "framework/legacy.h"
#include "event/DataGetMccMncCallback.h"

void DataGetMccMncCallback::onResponse(
    std::shared_ptr<Message> solicitedMsg, Status status,
    std::shared_ptr<qcril_uim_mcc_mnc_info_type> responseDataPtr) {
  mStatus = status;
  if (mStatus == Message::Callback::Status::SUCCESS) {
    Log::getInstance().d("[DataGetMccMncCallback]: Callback[msg = " +
        solicitedMsg->dump() + "] executed. client data = " +
        mClientToken + " status = Message::Callback::Status::SUCCESS");
    mResponseData = responseDataPtr;
    std::shared_ptr<qcril_uim_mcc_mnc_info_type> resp = std::static_pointer_cast<qcril_uim_mcc_mnc_info_type>(responseDataPtr);
    if(resp)
    {
      qcril_request_params_type params_ptr;
      qcril_request_return_type ret_ptr;
      std::memset(&ret_ptr, 0, sizeof(ret_ptr));
      std::memset(&params_ptr, 0, sizeof(params_ptr));
      params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;
      params_ptr.data = static_cast<void*>(new char[sizeof(qcril_uim_mcc_mnc_info_type)]());
      if(params_ptr.data != nullptr) {
          memcpy(params_ptr.data, resp.get(), sizeof(qcril_uim_mcc_mnc_info_type));
      }

      rildata::qcrilDataprocessMccMncInfo(&params_ptr, &ret_ptr);

  } else {
      Log::getInstance().d("[DataGetMccMncCallback]: resp is NULL..not processing ");
    }
} else {
    Log::getInstance().d("[DataGetMccMncCallback]: Callback[msg = " +
        solicitedMsg->dump() + "] executed. client data = " +
        mClientToken + " FAILURE !!");
  }
}

Message::Callback::Status DataGetMccMncCallback::getStatus() {
  if (mStatus == Message::Callback::Status::SUCCESS) {
    Log::getInstance().d("[DataGetMccMncCallback]: Message::Callback::Status::SUCCESS");
  } else {
    Log::getInstance().d("[DataGetMccMncCallback]: not SUCCESS");
  }
  return mStatus;
}

Message::Callback *DataGetMccMncCallback::clone() {
  DataGetMccMncCallback *clone = new DataGetMccMncCallback(mClientToken);
  return clone;
}
