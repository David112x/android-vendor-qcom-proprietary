LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
        $(call all-java-files-under, src) \
        src/org/codeaurora/bluetooth/pxpservice/IPxpService.aidl

LOCAL_PACKAGE_NAME := pxp-monitor
LOCAL_CERTIFICATE := platform

LOCAL_MODULE_OWNER := qcom

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
