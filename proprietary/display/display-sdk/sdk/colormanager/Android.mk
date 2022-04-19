# @file Android.mk
#
LOCAL_PATH := $(call my-dir)

# Build the library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := com.qti.snapdragon.sdk.display
LOCAL_SRC_FILES := $(call all-java-files-under,.)
LOCAL_SRC_FILES += com/qti/snapdragon/sdk/display/IColorService.aidl
include $(BUILD_JAVA_LIBRARY)

# Copy com.qti.service.xml to /system/etc/permissions/
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := com.qti.snapdragon.sdk.display.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
