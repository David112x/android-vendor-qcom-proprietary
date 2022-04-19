ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(BUILD_TINY_ANDROID),true)

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libqti_resampler_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/resample
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := lib_soundmodel_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/soundmodel
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libqti_vraudio_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/3DAudio
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

endif #ANDROID_P and above

endif # BUILD_TINY_ANDROID
endif # is-board-platform, QCOM
