LOCAL_PATH := $(call my-dir)
common_cflags := -Wall
common_cflags += -g
common_cflags += -O2

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := qsocket/inc
LOCAL_COPY_HEADERS      := inc/msm_ipc.h
LOCAL_COPY_HEADERS      += inc/qsocket.h
LOCAL_COPY_HEADERS      += inc/qsocket_ipcr.h

LOCAL_CFLAGS := $(common_cflags)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_SRC_FILES  := src/qsocket_ipcr.c

LOCAL_MODULE := libqsocket
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc
include $(BUILD_SHARED_LIBRARY)

