LOCAL_PATH:= $(call my-dir)
#disable for now
include $(CLEAR_VARS)

LOCAL_HEADER_LIBRARIES := libcneqmiutils_headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS := libcneqmiutils_headers

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= CneQmiUtils.cpp

LOCAL_MODULE:= libcneqmiutils
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Wall -Werror

LOCAL_SHARED_LIBRARIES := libc libutils

LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/../inc \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/qmi/inc

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CLANG := true
include $(BUILD_SHARED_LIBRARY)
