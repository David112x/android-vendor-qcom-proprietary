LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# vendor executablesY
include $(CLEAR_VARS)
LOCAL_MODULE := init.qti.fm.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)

LOCAL_INIT_RC := init.qti.fm.rc

include $(BUILD_PREBUILT)

LOCAL_SHARED_LIBRARIES := liblog

LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/fm_hci/Android.mk
