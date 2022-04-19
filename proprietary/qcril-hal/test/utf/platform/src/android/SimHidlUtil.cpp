/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "platform/android/SimHidlUtil.h"

int convertSimIoPayloadtoHidl(void *data, size_t datalen, V1_0::IccIo& iccIoData)
{
  android::hardware::hidl_string aid_val;

  auto payload = static_cast<RIL_SIM_IO_v6 *>(data);

  iccIoData.command = payload->command;
  iccIoData.fileId  = payload->fileid;

  if (payload->path != NULL)
  {
    android::hardware::hidl_string path_val = {};

    path_val.setToExternal(payload->path, strlen(payload->path));

    iccIoData.path = path_val;
  }

  iccIoData.p1 = payload->p1;
  iccIoData.p2 = payload->p2;
  iccIoData.p3 = payload->p3;

  if (payload->data != NULL)
  {
    android::hardware::hidl_string data_val = {};

    data_val.setToExternal(payload->data, strlen(payload->data));

    iccIoData.data = data_val;
  }

  if (payload->pin2 != NULL)
  {
    android::hardware::hidl_string pin2_val = {};

    pin2_val.setToExternal(payload->pin2, strlen(payload->pin2));

    iccIoData.pin2 = pin2_val;
  }

  if (payload->aidPtr != NULL)
  {
    android::hardware::hidl_string aid_val = {};

    aid_val.setToExternal(payload->aidPtr, strlen(payload->aidPtr));

    iccIoData.aid = aid_val;
  }

  return 0;
}

int convertSimIoRespHidltoRilPayload(const V1_0::IccIoResult& iccIoResult, RIL_SIM_IO_Response& data)
{
  data.sw1 = iccIoResult.sw1;
  data.sw2 = iccIoResult.sw2;

  if (!iccIoResult.simResponse.empty())
  {
    data.simResponse = (char *) malloc(strlen(iccIoResult.simResponse.c_str()) + 1);
    strcpy(data.simResponse, iccIoResult.simResponse.c_str());
  }

  return 0;
}

int convertGetIccCardStatusRespHidltoRilPayload
(
  const V1_2::CardStatus& cardStatusData,
  RIL_CardStatus_v6&      data
)
{
  data.card_state                      = (RIL_CardState)cardStatusData.base.cardState;
  data.universal_pin_state             = (RIL_PinState) cardStatusData.base.universalPinState;
  data.gsm_umts_subscription_app_index = cardStatusData.base.gsmUmtsSubscriptionAppIndex;
  data.cdma_subscription_app_index     = cardStatusData.base.cdmaSubscriptionAppIndex;
  data.ims_subscription_app_index      = cardStatusData.base.imsSubscriptionAppIndex;

  data.physical_slot_id = cardStatusData.physicalSlotId;

  if (data.card_state == RIL_CARDSTATE_PRESENT)
  {
    data.atr = (char *) malloc(strlen(cardStatusData.atr.c_str()) + 1);
    strcpy(data.atr, cardStatusData.atr.c_str());

    data.iccid = (char *) malloc(strlen(cardStatusData.iccid.c_str()) + 1);
    strcpy(data.iccid, cardStatusData.iccid.c_str());
  }

  data.num_applications = cardStatusData.base.applications.size();

  for (uint8_t index = 0; index < cardStatusData.base.applications.size() &&
       index < RIL_CARD_MAX_APPS; index++)
  {
    data.applications[index].app_type       =
      (RIL_AppType) cardStatusData.base.applications[index].appType;
    data.applications[index].app_state      =
      (RIL_AppState) cardStatusData.base.applications[index].appState;
    data.applications[index].perso_substate =
     (RIL_PersoSubstate) cardStatusData.base.applications[index].persoSubstate;

    if (!cardStatusData.base.applications[index].aidPtr.empty())
    {
      data.applications[index].aid_ptr =
        (char *) malloc(strlen(cardStatusData.base.applications[index].aidPtr.c_str()) + 1);
      strcpy(data.applications[index].aid_ptr, cardStatusData.base.applications[index].aidPtr.c_str());
    }

    if (!cardStatusData.base.applications[index].appLabelPtr.empty())
    {
      data.applications[index].app_label_ptr =
        (char *) malloc(strlen(cardStatusData.base.applications[index].appLabelPtr.c_str()) + 1);
      strcpy(data.applications[index].app_label_ptr, cardStatusData.base.applications[index].appLabelPtr.c_str());
    }

    data.applications[index].pin1_replaced = cardStatusData.base.applications[index].pin1Replaced;
    data.applications[index].pin1 = (RIL_PinState) cardStatusData.base.applications[index].pin1;
    data.applications[index].pin2 = (RIL_PinState) cardStatusData.base.applications[index].pin2;
  }

  return 0;
}

int convertGetIccCardStatusRespHidltoRilPayload_1_4
(
  const V1_4::CardStatus& cardStatusData,
  RIL_CardStatus_v6&      data
)
{
  return convertGetIccCardStatusRespHidltoRilPayload(cardStatusData.base, data);
}

void convertOpenLogicalChannelRespHidlToRilPayload
       (const android::hardware::hidl_vec<int8_t>& selectResponse, int32_t channel,
        uint8_t *payload)
{
  payload[0] = (uint8_t) channel;

  if (selectResponse.size() != 0)
  {
    memcpy(&payload[1], &selectResponse[0], selectResponse.size());
  }
}

void convertOpenLogicalChannelPayloadtoHidl(void* data, size_t datalen, uint8_t* p2,
     android::hardware::hidl_string& hidl_aid)
{

  RIL_OpenChannelParams* open_channel_param_ptr = (RIL_OpenChannelParams*)data;
  *p2=open_channel_param_ptr->p2;
  hidl_aid.setToExternal(open_channel_param_ptr->aidPtr, strlen(open_channel_param_ptr->aidPtr));
}
