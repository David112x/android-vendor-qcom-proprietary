ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)

LOCAL_PATH:= $(call my-dir)

# ==========================================================================
# Build Package
# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PRODUCT_MODULE := true

LOCAL_PACKAGE_NAME := PowerOffAlarm
LOCAL_CERTIFICATE := platform

LOCAL_SDK_VERSION := system_current
LOCAL_JAVA_LIBRARIES := \
    android.hidl.base-V1.0-java \

LOCAL_STATIC_JAVA_LIBRARIES := \
    vendor.qti.hardware.alarm-V1.0-java \

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif #TARGET_FWK_SUPPORTS_FULL_VALUEADDS