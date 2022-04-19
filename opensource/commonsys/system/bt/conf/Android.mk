# Cannot convert to Android.bp as resource copying has not
# yet implemented for soong as of 12/16/2016
LOCAL_PATH := $(call my-dir)

# Bluetooth bt_stack.conf config file
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := bt_stack.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/bluetooth
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# Bluetooth bt_did.conf config file
# ========================================================
#include $(CLEAR_VARS)
#LOCAL_MODULE := bt_did.conf
#LOCAL_MODULE_CLASS := ETC
#LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/bluetooth
#LOCAL_MODULE_TAGS := optional
#LOCAL_SRC_FILES := $(LOCAL_MODULE)
#include $(BUILD_PREBUILT)

# Bluetooth interop_database.conf config file
# ========================================================
include $(CLEAR_VARS)
LOCAL_PATH := vendor/qcom/opensource/commonsys/bluetooth_ext/system_bt_ext/conf/
LOCAL_MODULE := interop_database.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/bluetooth
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# Bluetooth bt_profile.conf config file
# ========================================================
include $(CLEAR_VARS)
LOCAL_PATH := vendor/qcom/opensource/commonsys/bluetooth_ext/system_bt_ext/conf/
LOCAL_MODULE := bt_profile.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/bluetooth
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
