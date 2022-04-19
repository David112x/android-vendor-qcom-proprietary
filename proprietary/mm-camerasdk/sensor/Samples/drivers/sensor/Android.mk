SENSOR_LIBS_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(SENSOR_LIBS_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes/

LOCAL_SRC_FILES:= imx230_lib.c
LOCAL_MODULE           := libmmcamera_imx230_s
LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_SHARED_LIBRARY)
