LOCAL_PATH := $(call my-dir)
common_cflags := -Wall
common_cflags += -g
common_cflags += -O2

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := qrtr/inc
LOCAL_COPY_HEADERS      := lib/libqrtr.h
LOCAL_COPY_HEADERS      += src/ns.h

LOCAL_CFLAGS := $(common_cflags)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src
LOCAL_SRC_FILES  := lib/qrtr.c
LOCAL_SRC_FILES  += lib/qmi.c

LOCAL_MODULE := libqrtr
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/lib
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CFLAGS := $(common_cflags)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src
LOCAL_SRC_FILES  := src/cfg.c
LOCAL_SHARED_LIBRARIES := libqrtr
LOCAL_MODULE := qrtr-cfg
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CFLAGS := $(common_cflags)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src
LOCAL_SRC_FILES  := src/ns.c
LOCAL_SRC_FILES  += src/map.c
LOCAL_SRC_FILES  += src/hash.c
LOCAL_SRC_FILES  += src/waiter.c
LOCAL_SRC_FILES  += src/util.c
LOCAL_SHARED_LIBRARIES := libqrtr
LOCAL_MODULE := qrtr-ns
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CFLAGS := $(common_cflags)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src
LOCAL_SRC_FILES  := src/lookup.c
LOCAL_SRC_FILES  += src/util.c
LOCAL_SHARED_LIBRARIES := libqrtr
LOCAL_MODULE := qrtr-lookup
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
