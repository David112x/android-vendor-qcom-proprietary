ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res

LOCAL_STATIC_ANDROID_LIBRARIES := \
    android-support-v7-appcompat \
    android-support-v7-cardview \

LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_USE_AAPT2 := true

LOCAL_PACKAGE_NAME := SimContact
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MIN_SDK_VERSION := 21

include $(BUILD_PACKAGE)
endif#TARGET_FWK_SUPPORTS_FULL_VALUEADDS
