/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <vendor/qti/hardware/capabilityconfigstore/1.0/ICapabilityConfigStore.h>
#include <hidl/LegacySupport.h>

#include "CapabilityConfigStore.h"

using vendor::qti::hardware::capabilityconfigstore::V1_0::ICapabilityConfigStore;
using android::hardware::defaultPassthroughServiceImplementation;
using android::hardware::registerPassthroughServiceImplementation;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::OK;

int main() {
	ALOGI("vendor::qti::hardware::capabilityconfigstore  service starts");
	android::status_t status;

	configureRpcThreadpool(10, false);
	status = registerPassthroughServiceImplementation<ICapabilityConfigStore>();

	LOG_ALWAYS_FATAL_IF(status != OK, "Error while registering capabilityconfigstore: %d", status);

	joinRpcThreadpool();
	return status;
}

