//
// Copyright 2016 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <sys/types.h>
#include <sys/stat.h>
#include <android/hardware/bluetooth/1.0/IBluetoothHci.h>
#include <hidl/LegacySupport.h>
#include <hwbinder/ProcessState.h>

#ifdef QCOM_FM_SUPPORTED
#include <vendor/qti/hardware/fm/1.0/IFmHci.h>
#endif
#ifdef QCOM_ANT_SUPPORTED
#include <com/qualcomm/qti/ant/1.0/IAntHci.h>
#endif

#ifdef QTI_BT_SAR_SUPPORTED
#include <vendor/qti/hardware/bluetooth_sar/1.0/IBluetoothSar.h>
#endif

#ifdef QTI_BT_CONFIGSTORE_SUPPORTED
#include <vendor/qti/hardware/btconfigstore/1.0/IBTConfigStore.h>
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#ifdef LAZY_SERVICE
#define LOG_TAG "android.hardware.bluetooth@1.0-service-lazy"
#else
#define LOG_TAG "android.hardware.bluetooth@1.0-service"
#endif

// Generated HIDL files
using android::hardware::bluetooth::V1_0::IBluetoothHci;

#ifdef QCOM_FM_SUPPORTED
using vendor::qti::hardware::fm::V1_0::IFmHci;
#endif

#ifdef QCOM_ANT_SUPPORTED
using com::qualcomm::qti::ant::V1_0::IAntHci;
#endif

#ifdef QTI_BT_SAR_SUPPORTED
using vendor::qti::hardware::bluetooth_sar::V1_0::IBluetoothSar;
#endif

#ifdef QTI_BT_CONFIGSTORE_SUPPORTED
using ::vendor::qti::hardware::btconfigstore::V1_0::IBTConfigStore;
#endif

using android::hardware::registerLazyPassthroughServiceImplementation;
using android::hardware::registerPassthroughServiceImplementation;
using android::hardware::defaultPassthroughServiceImplementation;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::OK;

#ifdef QTI_VND_FWK_DETECT_SUPPORTED
#ifdef __cplusplus
extern "C" {
#include "vndfwk-detect.h"
}
#endif
#endif

static const int BT_TX_RT_PRIORITY = 1;

int main() {
    (void)umask(S_IWGRP | S_IWOTH);

    struct sched_param rt_params;
    rt_params.sched_priority = BT_TX_RT_PRIORITY;
/*  TBD: For future
    if (sched_setscheduler(gettid(), SCHED_FIFO, &rt_params)) {
     ALOGE("%s unable to set SCHED_FIFO for pid %d, tid %d, error %s", __func__,
            getpid(), gettid(), strerror(errno));
    }
*/

    //Initialize Mmap size the moment the process starts
    android::hardware::ProcessState::initWithMmapSize((size_t)(256144));

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    bool isVendorEnhancedFramework;

#ifdef QTI_VND_FWK_DETECT_SUPPORTED
    /* 0: Pure AOSP for both system and odm
       1: Pure AOSP for system and QC Value-adds for odm
       2: QC value-adds for system and Pure AOSP for odm
       3: QC value-adds for both system and odm
    */
    int vendorEnhancedInfo = getVendorEnhancedInfo();
    ALOGI("vendorEnhancedInfo: %d", vendorEnhancedInfo);
    isVendorEnhancedFramework = (vendorEnhancedInfo & 1) ? true : false;
#else
    isVendorEnhancedFramework = false;
#endif

    ALOGI("isVendorEnhancedFramework: %d", isVendorEnhancedFramework);
    ALOGI("Registering BT Service");

    android::status_t status;

#ifdef LAZY_SERVICE
    status = registerLazyPassthroughServiceImplementation<IBluetoothHci>();
#else
    status = registerPassthroughServiceImplementation<IBluetoothHci>();
#endif

    if (status != OK)
      ALOGI("Error while registering BT service: %d", status);

#ifdef QCOM_FM_SUPPORTED
    if (isVendorEnhancedFramework) {
      ALOGI("Registering FM Service");

#ifdef LAZY_SERVICE
      status = registerLazyPassthroughServiceImplementation<IFmHci>();
#else
      status = registerPassthroughServiceImplementation<IFmHci>();
#endif

      if (status != OK)
        ALOGI("Error while registering FM service: %d", status);
    }
#endif

#ifdef QCOM_ANT_SUPPORTED
    if (isVendorEnhancedFramework) {
      ALOGI("Registering ANT Service");

#ifdef LAZY_SERVICE
      status = registerLazyPassthroughServiceImplementation<IAntHci>();
#else
      status = registerPassthroughServiceImplementation<IAntHci>();
#endif

      if (status != OK)
        ALOGI("Error while registering ANT service: %d", status);
    }
#endif

#ifdef QTI_BT_SAR_SUPPORTED
    ALOGI("Registering SAR service");

#ifdef LAZY_SERVICE
    status = registerLazyPassthroughServiceImplementation<IBluetoothSar>();
#else
    status = registerPassthroughServiceImplementation<IBluetoothSar>();
#endif

    if (status != OK)
      ALOGI("Error while registering BT SAR service: %d", status);
#endif

#ifdef QTI_BT_CONFIGSTORE_SUPPORTED
    if (isVendorEnhancedFramework) {
      ALOGI("Registering BT Config store Service");

#ifdef LAZY_SERVICE
      status = registerLazyPassthroughServiceImplementation<IBTConfigStore>();
#else
      status = registerPassthroughServiceImplementation<IBTConfigStore>();
#endif
      if (status != OK)
        ALOGI("Error while BT Config store service: %d", status);
    }
#endif

    joinRpcThreadpool();

    return status;
}
