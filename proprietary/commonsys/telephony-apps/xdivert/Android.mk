ifneq ($(TARGET_NO_TELEPHONY), true)
ifneq ($(TARGET_HAS_LOW_RAM),true)

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := xdivert
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES := telephony-common qti-telephony-utils qti-telephony-hidl-wrapper
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
endif
endif # TARGET_NO_TELEPHONY
