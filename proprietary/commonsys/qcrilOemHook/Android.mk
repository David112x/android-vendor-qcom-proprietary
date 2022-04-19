################################################################################
# @file  Android.mk
# @brief Rules for compiling the source files
################################################################################

LOCAL_PATH:= $(call my-dir)
# ==========================================================================
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := qti-telephony-utils qti-telephony-hidl-wrapper

LOCAL_MODULE := qcrilhook
LOCAL_SRC_FILES := $(call all-java-files-under,src/com/qualcomm/qcrilhook)
LOCAL_SRC_FILES += src/com/qualcomm/qcrilhook/IOemHookCallback.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qcrilmsgtunnel/IQcrilMsgTunnel.aidl
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

include $(BUILD_JAVA_LIBRARY)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := qti-telephony-utils qti-telephony-hidl-wrapper

LOCAL_MODULE := qcrilhook-static
LOCAL_SRC_FILES := $(call all-java-files-under,src/com/qualcomm/qcrilhook)
LOCAL_SRC_FILES += src/com/qualcomm/qcrilhook/IOemHookCallback.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qcrilmsgtunnel/IQcrilMsgTunnel.aidl
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

include $(BUILD_STATIC_JAVA_LIBRARY)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := qcrilhook.xml


LOCAL_MODULE_CLASS := ETC

# This will install the file in /product/etc/permissions
#
LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := qti-telephony-utils qti-telephony-hidl-wrapper
LOCAL_STATIC_JAVA_LIBRARIES := vendor.qti.hardware.radio.qcrilhook-V1.0-java

LOCAL_PRIVILEGED_MODULE := true

LOCAL_SRC_FILES := $(call all-java-files-under,src/com/qualcomm/qcrilmsgtunnel)
LOCAL_SRC_FILES += src/com/qualcomm/qcrilhook/IOemHookCallback.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qcrilmsgtunnel/IQcrilMsgTunnel.aidl

LOCAL_PACKAGE_NAME := qcrilmsgtunnel
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRODUCT_MODULE := true

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
