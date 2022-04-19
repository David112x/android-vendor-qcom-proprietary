/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "qcril_qmi_qti_radio_service.h"
#include "modules/qti_radio/RilRequestNrMessageCallback.h"
#include "GetDataNrIconTypeMessage.h"
#include "framework/Log.h"
#include "framework/legacy.h"
#include "qcril_log.h"
#include <telephony/ril.h>
#define TAG "RILQ"


void QtiRadioGetDataNrIconTypeMessageCb::onResponse(
    std::shared_ptr<Message> solicitedMsg, Status status,
    std::shared_ptr<rildata::NrIconType_t> responseDataPtr) {

    RIL_Errno err_num = RIL_E_INTERNAL_ERR;
    five_g_icon_type rilIconType = FIVE_G_ICON_TYPE_INVALID;

    if (status == Message::Callback::Status::SUCCESS) {
      Log::getInstance().d("[QtiRadioGetDataNrIconTypeMessageCb]: Callback[msg = " +
      solicitedMsg->dump() + "] executed. status = Message::Callback::Status::SUCCESS");

      if(responseDataPtr) {
          if(responseDataPtr->isNone()) {
              rilIconType = FIVE_G_ICON_TYPE_NONE;
          } else if(responseDataPtr->isBasic()) {
              rilIconType = FIVE_G_ICON_TYPE_BASIC;
          } else if(responseDataPtr->isUwb()) {
              rilIconType = FIVE_G_ICON_TYPE_UWB;
          } else {
              rilIconType = FIVE_G_ICON_TYPE_INVALID;
          }
      }
      err_num = RIL_E_SUCCESS;
      implementation::queryNrIconTypeResponse(getNrIconToken(), err_num, rilIconType);
    } else {
      Log::getInstance().d("[QtiRadioGetDataNrIconTypeMessageCb]: Callback[msg = " +
      solicitedMsg->dump() + "] executed. FAILURE !!");
      implementation::queryNrIconTypeResponse(getNrIconToken(), err_num, rilIconType);
    }
}

Message::Callback *QtiRadioGetDataNrIconTypeMessageCb::clone() {
    QtiRadioGetDataNrIconTypeMessageCb *clone =
            new QtiRadioGetDataNrIconTypeMessageCb(nrIconToken);
    return clone;
}
