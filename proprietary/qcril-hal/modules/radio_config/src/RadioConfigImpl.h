/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "RadioConfigFactory.h"
#include <android/hardware/radio/config/1.0/IRadioConfig.h>
#include <android/hardware/radio/config/1.0/IRadioConfigIndication.h>
#include <android/hardware/radio/config/1.0/IRadioConfigResponse.h>
#include <android/hardware/radio/config/1.0/types.h>
#include <telephony/ril.h>
#include <framework/Module.h>
#include <hidl/HidlTransportSupport.h>
#include <inttypes.h>
#include <QtiMutex.h>
#include <modules/uim/UimSlotStatusInd.h>
#include "RadioConfigUtil.h"

using android::hardware::radio::config::V1_0::IRadioConfig;
using android::hardware::radio::config::V1_0::IRadioConfigIndication;
using android::hardware::radio::config::V1_0::IRadioConfigResponse;
using android::sp;
using android::hardware::hidl_death_recipient;
using android::hardware::Return;
using android::hardware::hidl_vec;
using android::hardware::hidl_string;

struct RadioConfigImpl : public IRadioConfig, public hidl_death_recipient {
  Module* mModule;

  sp<IRadioConfigIndication> mIndicationCb;
  sp<IRadioConfigResponse> mResponseCb;

  qtimutex::QtiSharedMutex mCallbackLock;

  RadioConfigImpl();
  sp<IRadioConfigResponse> getResponseCallback();
  sp<IRadioConfigIndication> getIndicationCallback();

  virtual void clearCallbacks();
  virtual Module* getRadioConfigModule();
  virtual void createRadioConfigModule();
  static const QcrildConfigVersion &getVersion();
  virtual const char *getDescriptor() { return descriptor; }

  virtual ::android::status_t registerAsService(const std::string &serviceName);
  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

  virtual Return<void> setResponseFunctions_nolock(const sp<IRadioConfigResponse>& radioConfigResponse,
                                                   const sp<IRadioConfigIndication>& radioConfigIndication);
  Return<void> setResponseFunctions(const sp<IRadioConfigResponse>& radioConfigResponse,
                                    const sp<IRadioConfigIndication>& radioConfigIndication);
  Return<void> getSimSlotsStatus(int32_t serial);
  Return<void> setSimSlotsMapping(int32_t serial, const hidl_vec<uint32_t>& slotMap);
  virtual void sendSlotStatusIndication(const std::shared_ptr<UimSlotStatusInd> msg_ptr);
};

sp<RadioConfigImpl> getRadioConfig();
