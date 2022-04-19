/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_HARDWARE_FACTORY_V1_0_IFACTORY_H
#define VENDOR_QTI_HARDWARE_FACTORY_V1_0_IFACTORY_H
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string>
#include <string.h>
#include <vendor/qti/hardware/factory/1.1/IFactory.h>
#include <hardware/hardware.h>
#include <hidl/Status.h>
#include <hidl/MQDescriptor.h>
#include <cutils/properties.h>
#include <vector>

#define MMID_SERVICE_STATUS "init.svc.vendor.mmid"
#define PACK_SIZE 1024

namespace vendor {
namespace qti {
namespace hardware {
namespace factory {
namespace V1_0 {
namespace implementation {

using ::vendor::qti::hardware::factory::V1_1::IFactory;
using ::vendor::qti::hardware::factory::V1_0::FactoryResult;
using ::vendor::qti::hardware::factory::V1_0::IResultType;
using ::vendor::qti::hardware::factory::V1_0::WriteFileReq;
using ::vendor::qti::hardware::factory::V1_0::ReadFileResult;
using ::vendor::qti::hardware::factory::V1_0::ReadFileReq;

using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::hardware::Void;

using ::std::string;
using ::std::vector;

struct Factory : public IFactory {
    Factory();
    ~Factory();
    Return<void> runApp(const hidl_string& name,const hidl_string& params, bool isStart, runApp_cb _hidl_cb) override;
    Return<void> getSmbStatus(getSmbStatus_cb _hidl_cb) override;
    Return<bool> enterShipMode() override;
    Return<bool> chargerEnable(bool enable) override;
    Return<bool> wifiEnable(bool enable) override;

    Return<IResultType> eraseAllFiles(const hidl_string& path) override;
    Return<IResultType> dirListFileNameToFile(const hidl_string& path, const hidl_string& name) override;
    Return<void> readFile(const hidl_string& path, const ReadFileReq& req, readFile_cb _hidl_cb) override;
    Return<IResultType> writeFile(const hidl_string& path, const WriteFileReq& req) override;
    Return<void> delegate(const hidl_string& cmd,const hidl_string& value, delegate_cb _hidl_cb) override;

private:
    V1_1::FactoryResult set_prop(const char* prop, const char* value);
    V1_1::FactoryResult get_prop(const char* prop);
    vector<string> split_string(const string& str, const string& delim);
};

extern "C" IFactory* HIDL_FETCH_IFactory(const char* name);

}
}  // namespace V1_0
}  // namespace factory
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif // VENDOR_QTI_HARDWARE_FACTORY_V1_0_IFACTORY_H
