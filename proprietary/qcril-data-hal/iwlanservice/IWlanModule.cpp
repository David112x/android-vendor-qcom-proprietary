/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "IWlanImpl.h"
#include <cstring>
#include <cutils/properties.h>
#include <framework/QcrilInitMessage.h>
#include "IWlanModule.h"
#include <framework/Log.h>
#include <framework/ModuleLooper.h>

using namespace vendor::qti::hardware::data::iwlan;

static load_module<IWlanModule> sIWlanModule;

IWlanModule *getIWlanModule() {
    return &(sIWlanModule.get_module());
}

/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
IWlanModule::IWlanModule() {
  mName = "IWlanModule";
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
    HANDLER(QcrilInitMessage, IWlanModule::handleQcrilInit),
    HANDLER(IWlanDataRegistrationStateChangeIndMessage, IWlanModule::handleDataRegistrationStateChange),
    HANDLER(IWlanDataCallListChangeIndMessage, IWlanModule::handleDataCallListChange),
    HANDLER(QualifiedNetworksChangeIndMessage, IWlanModule::handleQualifiedNetworksChange),
    HANDLER(SetupDataCallIWlanResponseIndMessage, IWlanModule::handleSetupDataCallIWlanResponseIndMessage),
    HANDLER(DeactivateDataCallIWlanResponseIndMessage, IWlanModule::handleDeactivateDataCallIWlanResponseIndMessage),
  };
}

/* Follow RAII.
*/
IWlanModule::~IWlanModule() {
  mLooper = nullptr;
}

/*
 * Module specific initialization that does not belong to RAII .
 */
void IWlanModule::init() {
  Module::init();
}

string IWlanModule::readProperty(string name, string defaultValue) {
  char cPropValue[256] = {'\0'};
  property_get(name.c_str(), cPropValue, defaultValue.c_str());
  return string(cPropValue);
}

/*
 * List of individual private handlers for the subscribed messages.
 */
void IWlanModule::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  string propValue = readProperty("ro.telephony.iwlan_operation_mode", "default");
  if(propValue.compare("legacy") == 0) {
    Log::getInstance().d("[" + mName + "]: IWlan service not required");
  }
  else {
    Log::getInstance().d("[" + mName + "]: get_instance_id = "
          + std::to_string(msg->get_instance_id()));
    /* Init qti_radio hidl services.*/
#ifndef QMI_RIL_UTF
    IWlanServiceInit (msg->get_instance_id ());
#endif
  }
}

void IWlanModule::handleDataRegistrationStateChange(std::shared_ptr<IWlanDataRegistrationStateChangeIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  onDataRegistrationStateChange ();
}

void IWlanModule::handleDataCallListChange(std::shared_ptr<IWlanDataCallListChangeIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  onDataCallListChange (msg->getDCList());
}

void IWlanModule::handleQualifiedNetworksChange(std::shared_ptr<QualifiedNetworksChangeIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  onQualifiedNetworksChange (msg->getQualifiedNetworks());
}

void IWlanModule::handleSetupDataCallIWlanResponseIndMessage(std::shared_ptr<SetupDataCallIWlanResponseIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  onSetupDataCallIWlanResponseIndMessage (msg->getResponse(), msg->getSerial(), msg->getStatus());
}

void IWlanModule::handleDeactivateDataCallIWlanResponseIndMessage(std::shared_ptr<DeactivateDataCallIWlanResponseIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  onDeactivateDataCallIWlanResponseIndMessage (msg->getResponse(), msg->getSerial(), msg->getStatus());
}


