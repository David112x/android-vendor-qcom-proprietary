LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_HEADER_LIBRARIES := libcneoplookup_headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS := libcneoplookup_headers

LOCAL_SRC_FILES:=  OperatorLookup.cpp

LOCAL_MODULE:= libcneoplookup
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES :=  libcutils \
                           liblog
LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/../inc

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CLANG := true
LOCAL_CFLAGS += -Wall -Werror
include $(BUILD_SHARED_LIBRARY)
