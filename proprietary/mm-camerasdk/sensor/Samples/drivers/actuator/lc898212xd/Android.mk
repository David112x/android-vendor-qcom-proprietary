LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes/

LOCAL_SRC_FILES:= lc898212xd_actuator.c
LOCAL_MODULE           := libactuator_lc898212xd_s
LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_SHARED_LIBRARY)
