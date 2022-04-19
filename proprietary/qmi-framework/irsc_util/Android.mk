LOCAL_PATH := $(call my-dir)

common_irsc_util_cflags := -g
common_irsc_util_cflags += -O2
common_irsc_util_cflags += -fno-inline
common_irsc_util_cflags += -fno-short-enums
common_irsc_util_cflags += -fpic
common_irsc_util_cflags += -Werror

include $(CLEAR_VARS)
LOCAL_MODULE := irsc_util
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../qsocket/inc
LOCAL_CFLAGS := $(common_irsc_util_cflags) -DDEBUG_ANDROID
LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SRC_FILES  := irsc_util.c
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

