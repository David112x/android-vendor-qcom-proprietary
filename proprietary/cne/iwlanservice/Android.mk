ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := android.hardware.radio-V1.0-java \
              android.hardware.radio-V1.1-java \
              android.hardware.radio-V1.4-java
LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.data.iwlan-V1.0-java

LOCAL_MODULE_OWNER := qti
LOCAL_PACKAGE_NAME := IWlanService

LOCAL_VENDOR_MODULE := true
LOCAL_CERTIFICATE := platform
LOCAL_SDK_VERSION := system_current
include $(BUILD_PACKAGE)
endif
