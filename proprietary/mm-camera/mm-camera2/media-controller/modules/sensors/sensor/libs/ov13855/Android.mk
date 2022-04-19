LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../../../../../mm-camerasdk/sensor/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../sensors/module/
LOCAL_C_INCLUDES += ov13855_lib.h

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SRC_FILES        := ov13855_lib.c
LOCAL_MODULE           := libmmcamera_ov13855
LOCAL_SHARED_LIBRARIES := libcutils

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
