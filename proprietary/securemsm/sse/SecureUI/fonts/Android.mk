LOCAL_PATH:= $(call my-dir)

#Copy fonts requried for running the secure UI sample TA on a specific platform
#To save space: for each target only fonts which are used by target's layout in Secure UI TA should be packed into data/misc/qsee folder

#Disabled the data/misc/qsee creation at compilation for FBE.
#TODO create dir at runtime from init.rc.
#ifeq ($(call is-board-platform-in-list, msm8953),true)
ifeq (false, true)

include $(CLEAR_VARS)
LOCAL_MODULE := default_qti_regular_37.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/misc/qsee
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := default_qti_regular_43.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/misc/qsee
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := default_qti_regular_90.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/misc/qsee
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif


ifeq ($(call is-board-platform-in-list, msm8996 apq8098_latv),true)

include $(CLEAR_VARS)
LOCAL_MODULE := default_qti_regular_42.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/misc/qsee
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := default_qti_regular_48.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/misc/qsee
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := default_qti_regular_100.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/misc/qsee
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif
