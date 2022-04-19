/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_HARDWARE_CAPABILITYCONFIGSTORE_V1_0_ICAPABILITYCONFIGSTORE_H
#define VENDOR_QTI_HARDWARE_CAPABILITYCONFIGSTORE_V1_0_ICAPABILITYCONFIGSTORE_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <log/log.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <dirent.h>
#include <string.h>
#include <unordered_map>
#include <string>
#include <utils/String8.h>
#include <string.h>
#include <vector>
#include <cutils/properties.h>
#define LOG_TAG "capabilityconfigstorehal"
#include <vendor/qti/hardware/capabilityconfigstore/1.0/ICapabilityConfigStore.h>
#define CONFIGSTORE_XML_PATH "/vendor/etc/configstore"
#define CAP_CONFIGSTORE_DEBUG_PROP "persist.vendor.cap_configstore_debug"
#define  CONFIGSTORE_ALOGI(...)  {   if(debug_mode) {ALOGI(__VA_ARGS__);}}

#define XML_FILE_EXT ".xml"


using namespace std;
using android::String8;

void set_debug_flag();


namespace vendor {
namespace qti {
namespace hardware {
namespace capabilityconfigstore {
namespace V1_0 {
namespace implementation {


using ::vendor::qti::hardware::capabilityconfigstore::V1_0::ICapabilityConfigStore;
using ::vendor::qti::hardware::capabilityconfigstore::V1_0::CommandResult;
using ::vendor::qti::hardware::capabilityconfigstore::V1_0::Result;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::hardware::Void;

struct CapabilityConfigStore : public ICapabilityConfigStore {
    CapabilityConfigStore();
    ~CapabilityConfigStore();

    Return<void> getConfig(const hidl_string& area, const hidl_string& key, getConfig_cb _hidl_cb) override;

public:
    void parse_capability_data();

};

extern "C" ICapabilityConfigStore* HIDL_FETCH_ICapabilityConfigStore(const char* name);

}
}  // namespace V1_0
}  // namespace capabilityconfigstore
}  // hardware
}  // namespace qti
}  // namespace vendor

#endif // VENDOR_QTI_HARDWARE_CAPABILITYCONFIGSTORE_V1_0_ICAPABILITYCONFIGSTORE_H
