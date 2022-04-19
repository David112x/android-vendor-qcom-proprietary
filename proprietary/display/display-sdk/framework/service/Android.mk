# @file Android.mk
#
ifneq ($(TARGET_HAS_LOW_RAM), true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under,src)
LOCAL_REQUIRED_MODULES := \
    com.qti.snapdragon.sdk.display
LOCAL_JAVA_LIBRARIES := \
    com.qti.snapdragon.sdk.display
LOCAL_PACKAGE_NAME := colorservice
LOCAL_DEX_PREOPT := false
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_OWNER := qti
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true
include $(BUILD_PACKAGE)
endif
