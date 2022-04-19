/******************************************************************************
  @file    ril_utf_i_qti_oem_hook_sim.cpp
  @brief   RIL UTF OEMHOOK HIDL implementation

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_qmi_oemhook_service.h"
#include "vendor/qti/hardware/radio/qcrilhook/1.0/IQtiOemHook.h"
#include "ril_utf_hidl_services.h"
#ifdef QMI_RIL_HAL_UTF
#include "ril_utf_core.h"
#endif

extern "C" {
#ifndef QMI_RIL_HAL_UTF
#include "ril_utf_core.h"
#endif
#include "ril_utf_oem_sim.h"
}

using namespace vendor::qti::hardware::radio::qcrilhook::V1_0;
using namespace vendor::qti::hardware::radio::qcrilhook::V1_0::implementation;
using ::android::hardware::Void;


void send_i_oemhook_enqueue_oem_expect(int size, void *payload, int msg_type, int msg_token, uint32_t oem_error) {
  int my_expect_slot;

  // acquire mutex
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: OEM recv thread locking exec_q_access in %s", __func__);
  EXEC_Q_LOCK();

  // Grab next available slot in expect table
  for (int i = 0; i < MAX_EXPECTS; ++i)
  {
    if (expect_table[i].in_use == 0)
    {
      my_expect_slot = i;
      expect_table[my_expect_slot].in_use = 1;
      break;
    }
  }

  // verify that we are allowed to proceed
  while ( expect_table[my_expect_slot].expect_ready != 1 )
  {
    struct timeval e_timeout;
    struct timespec thread_timeout;
    e_timeout.tv_sec = 0;
    e_timeout.tv_usec = EXPECT_TIMEOUT;
    calc_timeout(&thread_timeout, &e_timeout);
    pthread_cond_timedwait(&expect_ready_sig, &exec_q_access, &thread_timeout);
  }

  // Decode message and enqueue
  handle_oem_msg(size, (uint8_t *)payload, msg_type, msg_token, oem_error);

  // signal test thread.
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Setting expect_received to true @ %s %s %d ",__FILE__, __func__, __LINE__);
  expect_table[my_expect_slot].expect_received = 1;
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Signalling Expectation");
  pthread_cond_signal(&signal_exptectation);

  // release mutex
  EXEC_Q_UNLOCK();
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. released exec_q_mutex.", __func__);
}

class IQtiOemHookResponseClient : public IQtiOemHookResponse {
  public:
    ::android::hardware::Return<void> oemHookRawResponse(int32_t serial, RadioError error, const ::android::hardware::hidl_vec<uint8_t>& data) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: oemHookRawResponse");
      const uint8_t *payload = data.data();
      send_i_oemhook_enqueue_oem_expect(data.size(), (void *)payload, 0, serial, 0);
      return Void();
    }
};

class IQtiOemHookIndicationClient : public IQtiOemHookIndication {
  public:
    ::android::hardware::Return<void> oemHookRawIndication(const ::android::hardware::hidl_vec<uint8_t>& data) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: oemHookRawIndication");
      const uint8_t *payload = data.data();
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: data.size = %d", data.size());
      send_i_oemhook_enqueue_oem_expect(data.size(), (void *)payload, 1, 0, 0);
      return Void();
    }
};

static sp<IQtiOemHookResponseClient> mRespClient;
static sp<IQtiOemHookIndicationClient> mIndClient;

extern "C" void reset_i_qti_oem_hook() {
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: reset_i_qti_oem_hook");

  ::android::sp<IQtiOemHook> utfOemhook = IQtiOemHook::getService("oemhook0");
  if (utfOemhook != nullptr) {
    hidl_death_recipient *deathRecipient = dynamic_cast<hidl_death_recipient *>(utfOemhook.get());
    deathRecipient->serviceDied(0, utfOemhook);
  }
}

extern "C" void connect_to_i_qti_oem_hook() {
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: connect_to_i_qti_oem_hook");

  while (1) {
  ::android::sp<IQtiOemHook> utfOemhook = IQtiOemHook::getService("oemhook0");
    if (utfOemhook != nullptr) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: calling setCallback");
      mRespClient = new IQtiOemHookResponseClient();
      mIndClient = new IQtiOemHookIndicationClient();

      utfOemhook->setCallback(mRespClient, mIndClient);
      break;
    }
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: get(oemhook0) returned null");
    usleep(1);
  }
}

extern "C" void send_i_oemhook_request(int serial, char *msg, int len) {
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: send_i_oemhook_request");
  ::android::sp<IQtiOemHook> utfOemhook = IQtiOemHook::getService("oemhook0");
  if (utfOemhook != nullptr) {
    ::android::hardware::hidl_vec<uint8_t> data;
    data.setToExternal((uint8_t *)msg, len);
    //static int serial = 0;

    utfOemhook->oemHookRawRequest(serial, data);
  }
}
