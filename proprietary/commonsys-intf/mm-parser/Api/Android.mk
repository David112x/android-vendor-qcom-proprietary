LOCAL_PATH := $(call my-dir)

BUILD_PARSER :=

ifeq ($(TARGET_FWK_SUPPORTS_AV_VALUEADDS),true)
BUILD_PARSER := true
endif

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)
BUILD_PARSER := true
endif

ifeq ($(BUILD_PARSER),true)
include $(CLEAR_VARS)

LOCAL_MODULE := libmmparser_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc

include $(BUILD_HEADER_LIBRARY)
endif

include $(CLEAR_VARS)

LOCAL_MODULE := libmmparser_proprietary_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_HEADER_LIBRARY)
