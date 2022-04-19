#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling the source files
#*******************************************************************************/
ifneq ($(TARGET_NO_TELEPHONY), true)
src_java_ims := src/org/codeaurora/ims
src_java_ims += src/com/qualcomm/ims/vt
src_java_ims += src/com/qualcomm/ims/utils

LOCAL_PATH:= $(call my-dir)

# ==========================================================================
# Build the service
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_PACKAGE_NAME := ims
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true
LOCAL_JAVA_LIBRARIES := qti-telephony-utils qti-telephony-hidl-wrapper ims-ext-common

LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.radio.ims-V1.0-java \
    vendor.qti.hardware.radio.ims-V1.1-java vendor.qti.hardware.radio.ims-V1.2-java \
    vendor.qti.hardware.radio.ims-V1.3-java vendor.qti.hardware.radio.ims-V1.4-java \
    vendor.qti.hardware.radio.ims-V1.5-java vendor.qti.hardware.radio.ims-V1.6-java \

LOCAL_JNI_SHARED_LIBRARIES := libimsmedia_jni libimscamera_jni
LOCAL_REQUIRED_MODULES := libimsmedia_jni libimscamera_jni

LOCAL_PROTOC_OPTIMIZE_TYPE := micro
LOCAL_PRIVILEGED_MODULE := true

LOCAL_SRC_FILES := $(call all-java-files-under, $(src_java_ims))

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
# ==========================================================================
endif # TARGET_NO_TELEPHONY
