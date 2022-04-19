ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res

LOCAL_AAPT_FLAGS := --auto-add-overlay

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_AIDL_INCLUDES += system/bt/binder

LOCAL_PACKAGE_NAME := BATestApp
LOCAL_CERTIFICATE := platform

LOCAL_MODULE_OWNER := qti

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_PACKAGE)
endif
