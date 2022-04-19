#Disabling Presence App for Android Go
ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := PresenceApp
LOCAL_CERTIFICATE := platform
LOCAL_PRODUCT_MODULE := true

LOCAL_JAVA_LIBRARIES += telephony-common ims-common
include $(BUILD_PACKAGE)
endif
