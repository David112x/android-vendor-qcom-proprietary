/*!
 *
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include "SpcomSSREvent.h"
#include "log.h"

SPUHidlClientTest* SpcomSSREvent::sTestFw = nullptr;

SpcomSSREvent::SpcomSSREvent(SPUHidlClientTest* testFw) {
    sTestFw = testFw;
}

SpcomSSREvent::~SpcomSSREvent() { }

// SSR callback to be called by SPU service when SPU SSR event occurs
Return<void> SpcomSSREvent::callback() {
    ALOGD("SSR Callback was called");
    sTestFw->mSSROccured = true;
    return Void();
}
