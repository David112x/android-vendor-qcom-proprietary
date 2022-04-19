TOP_LOCAL_PATH := $(call my-dir)
#include HIDL
include $(TOP_LOCAL_PATH)/hidl/Android.mk

#include Daemon
include $(TOP_LOCAL_PATH)/daemon/Android.mk

LOCAL_PATH:= $(TOP_LOCAL_PATH)

# ==========================================================================
# Install the Library
# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.light.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.factory.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)
