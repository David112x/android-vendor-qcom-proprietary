ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
ifneq ($(TARGET_NO_TELEPHONY), true)
ifneq ($(TARGET_HAS_LOW_RAM),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += $ src/com/qualcomm/qti/qtisystemservice/ICallStateListener.aidl \
    src/com/qualcomm/qti/qtisystemservice/ISystemService.aidl \
    src/com/qualcomm/qti/qtisystemservice/ITelecomCallService.aidl

LOCAL_PACKAGE_NAME := QtiSystemService
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

LOCAL_CERTIFICATE := platform
include $(BUILD_PACKAGE)

endif
endif
endif # TARGET_NO_TELEPHONY
