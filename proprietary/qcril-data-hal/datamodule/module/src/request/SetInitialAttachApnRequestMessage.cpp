/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include <sstream>
#include "request/SetInitialAttachApnRequestMessage.h"
#include "modules/android/version.h"
#include "telephony/ril.h"

#ifndef QCRIL_RIL_VERSION
#error "undefined QCRIL_RIL_VERSION!"
#endif

bool SetInitialAttachApnRequestMessage::allocate_and_copy_data(const char *src, char **des)
{
    if (!des)
      return false;

    bool res = true;
    size_t len = src? (strlen(src) + 1) : 0;
    if(len > 0) {
        *des = new char[len];
        if (*des) {
            memcpy(*des, src, len);
        } else {
            res = false;
        }
    } else {
        *des = nullptr;
    }

    return res;
}

SetInitialAttachApnRequestMessage::~SetInitialAttachApnRequestMessage() {
  if (params != nullptr) {
#if (QCRIL_RIL_VERSION < 15)
    RIL_InitialAttachApn *apninfo = (RIL_InitialAttachApn*)params->data;
#else
    RIL_InitialAttachApn_v15 *apninfo = (RIL_InitialAttachApn_v15*)params->data;
#endif

    if(apninfo) {
        if(apninfo->apn) {
            delete []apninfo->apn;
        }

        if(apninfo->protocol) {
            delete []apninfo->protocol;
        }

        if(apninfo->username) {
            delete []apninfo->username;
        }

        if(apninfo->password) {
            delete []apninfo->password;
        }

#if (QCRIL_RIL_VERSION >= 15)
        if (apninfo->roamingProtocol) {
            delete []apninfo->roamingProtocol;
        }
        if (apninfo->mvnoType) {
            delete []apninfo->mvnoType;
        }
        if (apninfo->mvnoMatchData) {
            delete []apninfo->mvnoMatchData;
        }
#endif
        delete apninfo;
    }

    delete params;
  }
}

void SetInitialAttachApnRequestMessage::deepCopy(const ril_request_type &request){
#if (QCRIL_RIL_VERSION < 15)
  RIL_InitialAttachApn *apninfo = nullptr;
#else
  RIL_InitialAttachApn_v15 *apninfo = nullptr;
#endif
  bool success = false;

  do {
    params = new qcril_request_params_type;
    if (!params)
      break;

    // fill legacy params
    params->instance_id = request.instance_id;
    params->modem_id = QCRIL_DEFAULT_MODEM_ID;
    params->event_id = request.req_id;
    params->t = request.t;
    params->datalen = request.payload_len;
    params->data = nullptr;

#if (QCRIL_RIL_VERSION < 15)
    apninfo = new RIL_InitialAttachApn;
#else
    apninfo = new RIL_InitialAttachApn_v15;
#endif
    if (!apninfo)
    {
      Log::getInstance().d("[SetInitialAttachApnRequestMessage]::apninfo is NULL. Exiting");
      break;
    }
    // reset all pointers as nullptr to avoid unexpcted free when fail
    memset(apninfo, 0, sizeof(*apninfo));
    memset(&attach_params,0, sizeof(attach_params));

#if (QCRIL_RIL_VERSION < 15)
    RIL_InitialAttachApn* payload = (RIL_InitialAttachApn*)request.payload;
#else
    RIL_InitialAttachApn_v15* payload = (RIL_InitialAttachApn_v15*)request.payload;
#endif
    if (!payload)
    {
      Log::getInstance().d("[SetInitialAttachApnRequestMessage]::"
        "RIL_InitialAttachApn/RIL_InitialAttachApn_v15 Payload is NULL. Exiting");
      break;
    }

    apninfo->authtype = payload->authtype;
#if (QCRIL_RIL_VERSION >= 15)
    apninfo->supportedTypesBitmask = payload->supportedTypesBitmask;
    apninfo->bearerBitmask = payload->bearerBitmask;
    apninfo->modemCognitive = payload->modemCognitive;
    apninfo->mtu = payload->mtu;
#endif

    if(allocate_and_copy_data(payload->apn, &apninfo->apn) == false)
      break;

    if(allocate_and_copy_data(payload->protocol, &apninfo->protocol) == false)
      break;

    if(allocate_and_copy_data(payload->password, &apninfo->password) == false)
      break;

    if(allocate_and_copy_data(payload->username, &apninfo->username) == false)
      break;

#if (QCRIL_RIL_VERSION >= 15)

    if (allocate_and_copy_data(payload->roamingProtocol, &apninfo->roamingProtocol) == false)
      break;

    if (allocate_and_copy_data(payload->mvnoType, &apninfo->mvnoType) == false)
      break;

    if (allocate_and_copy_data(payload->mvnoMatchData, &apninfo->mvnoMatchData) == false)
      break;
#endif

  if(payload->apn){
    attach_params.apn.assign(payload->apn,strlen(payload->apn));
  }
  if(payload->username){
    attach_params.username.assign(payload->username,strlen(payload->username));
  }

  if(payload->password){
    attach_params.password.assign(payload->password,strlen(payload->password));
  }

  attach_params.authType =payload->authtype;

  if(payload->protocol){
    attach_params.protocol.assign(payload->protocol,strlen(payload->protocol));
  }

#if (QCRIL_RIL_VERSION >= 15)
  if(payload->mvnoType){
    attach_params.mvnoType.assign(payload->mvnoType,strlen(payload->mvnoType));
  }

  if(payload->mvnoMatchData){
    attach_params.mvnoMatchData.assign(payload->mvnoMatchData,strlen(payload->mvnoMatchData));
  }

  attach_params.modemCognitive = payload->modemCognitive;
  if(payload->roamingProtocol) {
    attach_params.roamingProtocol.assign(payload->roamingProtocol,strlen(payload->roamingProtocol));
  }
  attach_params.apnTypeBitmask = payload->supportedTypesBitmask;
  attach_params.bearerBitmask = payload->bearerBitmask;
  attach_params.mtu = payload->mtu;
#endif
    success = true;
  } while (false);

  if (!success) { // free the allocated memory
    if (apninfo) {
      if (apninfo->apn)
        delete []apninfo->apn;
      if (apninfo->protocol)
        delete []apninfo->protocol;
      if (apninfo->username)
        delete []apninfo->username;
      if (apninfo->password)
        delete []apninfo->password;
#if (QCRIL_RIL_VERSION >= 15)
      if (apninfo->roamingProtocol)
        delete []apninfo->roamingProtocol;
      if (apninfo->mvnoType)
        delete []apninfo->mvnoType;
      if (apninfo->mvnoMatchData)
        delete []apninfo->mvnoMatchData;
#endif
      delete apninfo;
      apninfo = NULL;
    }
  } else {
    params->data = (void*)apninfo;
  }
}

string SetInitialAttachApnRequestMessage::dump()
{
  std::stringstream ss;
  ss << "[" << mName << "] ";
  ss << "apn = " << attach_params.apn << ", ";
  ss << "protocol =" << attach_params.protocol <<",";
  ss << "username =" << attach_params.username <<",";
  ss << "password =" << attach_params.password <<",";
  ss << "authType =" << attach_params.authType <<",";
#if (QCRIL_RIL_VERSION >= 15)
  ss << "mvnoType = " << attach_params.mvnoType << ", ";
  ss << "mvnoMatchData = " << attach_params.mvnoMatchData << ", ";
  ss << "roamingProtocol = " << attach_params.roamingProtocol << ", ";
  ss << "apnTypeBitmask = " << attach_params.apnTypeBitmask << ", ";
  ss << "bearerBitmask = " << attach_params.bearerBitmask << ", ";
  ss << "modemCognitive = " << attach_params.modemCognitive << ", ";
  ss << "mtu = " << attach_params.mtu << ", ";
#endif
  return ss.str();
}
