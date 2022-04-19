ifneq ($(TARGET_NO_TELEPHONY), true)

ifneq ($(TARGET_HAS_LOW_RAM),true)

LOCAL_PATH:= $(call my-dir)

# Build the auto generated files into a library to be used by both the
# app and the service
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := uimgbalibrary
LOCAL_PRODUCT_MODULE := true
LOCAL_SRC_FILES := src/com/qualcomm/qti/uimGba/IUimGbaServiceCallback.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qti/uimGba/IUimGbaService.aidl
LOCAL_SDK_VERSION := system_current
include $(BUILD_JAVA_LIBRARY)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := UimGba.xml
LOCAL_PRODUCT_MODULE := true

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

# This will install the file in /product/etc/permissions
#
LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)
# ==========================================================================
# Build the uimGba manager
# which can be used by client

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := uimgbamanagerlibrary
LOCAL_PRODUCT_MODULE := true
LOCAL_JAVA_LIBRARIES := uimgbalibrary

LOCAL_SRC_FILES := src/com/qualcomm/qti/uimGbaManager/UimGbaManager.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/uimGbaManager/UimGbaManagerCallback.java

include $(BUILD_JAVA_LIBRARY)

# ==========================================================================

include $(CLEAR_VARS)

LOCAL_MODULE := UimGbaManager.xml
LOCAL_PRODUCT_MODULE := true
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

#this will install the file in /product/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

endif

endif
