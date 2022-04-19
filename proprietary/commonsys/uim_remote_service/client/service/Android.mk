#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/

ifneq ($(TARGET_HAS_LOW_RAM),true)

LOCAL_PATH:= $(call my-dir)

# Build the auto generated files into a library to be used by both the
# app and the service
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE := uimremoteclientlibrary
LOCAL_PRODUCT_MODULE := true
LOCAL_SRC_FILES := src/com/qualcomm/uimremoteclient/IUimRemoteClientService.aidl
LOCAL_SRC_FILES += src/com/qualcomm/uimremoteclient/IUimRemoteClientServiceCallback.aidl

include $(BUILD_JAVA_LIBRARY)

# ==========================================================================
# Build the service
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
src_proto := src
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SRC_FILES += $(call all-proto-files-under, $(src_proto))

LOCAL_PROTOC_OPTIMIZE_TYPE := micro

LOCAL_PACKAGE_NAME := uimremoteclient
LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_JAVA_LIBRARIES := uimremoteclientlibrary qti-telephony-hidl-wrapper
LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.radio.uim_remote_client-V1.0-java

include $(BUILD_PACKAGE)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := uimremoteclient.xml
LOCAL_PRODUCT_MODULE := true

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

# This will install the file in /product/etc/permissions
#
LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

endif
