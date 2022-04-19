#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling the source files
#*******************************************************************************/

ifneq ($(TARGET_NO_TELEPHONY), true)
ifneq ($(TARGET_HAS_LOW_RAM),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under,src)

LOCAL_PACKAGE_NAME := atfwd
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_JAVA_LIBRARIES := qti-telephony-hidl-wrapper
LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.radio.atcmdfwd-V1.0-java
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif
endif # TARGET_NO_TELEPHONY

