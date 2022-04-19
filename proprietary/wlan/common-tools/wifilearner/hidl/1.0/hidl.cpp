/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include <android/hidl/manager/1.2/IClientCallback.h>
#include <android/hidl/manager/1.2/IServiceManager.h>

#include "wifistats.h"

extern "C"
{
#include "hidl.h"
#include "wifilearner.h"
}

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using vendor::qti::hardware::wifi::wifilearner::V1_0::IWifiStats;
using vendor::qti::hardware::wifi::wifilearner::V1_0::implementation::WifiStats;

using android::hardware::Return;
using android::hardware::Status;

using android::hidl::manager::V1_2::IServiceManager;
using android::hidl::manager::V1_2::IClientCallback;
using android::hardware::defaultServiceManager1_2;

#define WIFI_STATS_SERVICE_NAME "wifiStats"
#define WIFI_STATS_FQ_SERVICE_NAME "vendor.qti.hardware.wifi.wifilearner@1.0::IWifiStats"

class WifiStatsClientCallback : public IClientCallback {
	Return<void> onClients(const android::sp<IBase>& base, bool clients) override {
		LOG(INFO) << "OnClients: " << clients;
		// we have no clients after having clients at least once
		if (!clients) {
			exit(EXIT_SUCCESS);
		}
		return Status::ok();
	}
};

/*
 * Function     : wifilearner_hidl_process
 * Description  : registers as server instance for IWifiStats service
 * Input params : pointer to wifilearner_ctx
 * Return       : SUCCESS/FAILURE
 *
 */
int wifilearner_hidl_process(struct wifilearner_ctx *wlc)
{
	android::base::InitLogging(NULL,
			android::base::LogdLogger(android::base::SYSTEM));
	LOG(INFO) << "Wifi Learner Hal is booting up...";

	android::sp<IWifiStats> service_wifistats = new WifiStats(wlc);
	configureRpcThreadpool(1, true /* callerWillJoin */);
	if (service_wifistats->registerAsService(WIFI_STATS_SERVICE_NAME) != android::OK) {
		LOG(ERROR) << "Cannot register WifiStats HAL service";
		return 1;
	}

	LOG(INFO) << "Registering client callback";
	android::sp<IServiceManager> manager = defaultServiceManager1_2();
	bool ret = manager->registerClientCallback(WIFI_STATS_FQ_SERVICE_NAME,
						   WIFI_STATS_SERVICE_NAME,
						   service_wifistats, new WifiStatsClientCallback);

	if(!ret) {
		LOG(ERROR) << "failed to register client callback";
	}

	joinRpcThreadpool();
	LOG(INFO) << "wifilearner is terminating...";

	return 0;
}
