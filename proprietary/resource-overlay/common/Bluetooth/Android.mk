LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CERTIFICATE := platform

LOCAL_SRC_FILES := $(call all-subdir-java-files)

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/qva/res
else
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/generic/res
endif #TARGET_FWK_SUPPORTS_FULL_VALUEADDS

LOCAL_PACKAGE_NAME := BluetoothResCommon
LOCAL_SDK_VERSION := current

include $(BUILD_RRO_PACKAGE)
