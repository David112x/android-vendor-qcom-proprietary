LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src/com/qualcomm/qti/sensors/core)
LOCAL_SRC_FILES +=  $(call all-java-files-under, src/com/qualcomm/qti/sensors/ui)
ifeq ($(call is-platform-sdk-version-at-least,29),true)
LOCAL_SRC_FILES :=  $(filter-out src/com/qualcomm/qti/sensors/ui/qsensortest/TabControl.java, $(LOCAL_SRC_FILES))
else
LOCAL_SRC_FILES :=  $(filter-out src/com/qualcomm/qti/sensors/ui/qsensortest/legacy/TabControl.java,$(LOCAL_SRC_FILES))
endif

ifeq ($(call is-platform-sdk-version-at-least,29),true)
LOCAL_STATIC_ANDROID_LIBRARIES := \
    android-support-v7-appcompat \
    android-support-v4

LOCAL_RESOURCE_DIR := \
  $(LOCAL_PATH)/res \
  frameworks/support/v7/appcompat/res \
  frameworks/support/v4/res
endif

LOCAL_PACKAGE_NAME := QSensorTest
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_FLAG_FILES := proguard.flags
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_SDK_VERSION := system_current
include $(BUILD_PACKAGE)
