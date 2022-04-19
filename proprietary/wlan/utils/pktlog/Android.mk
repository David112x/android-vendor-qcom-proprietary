LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := pktlogconf
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED), true)
LOCAL_PROPRIETARY_MODULE := true
endif
LOCAL_CLANG := true
LOCAL_VENDOR_MODULE := true
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../uapi/linux
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../target/inc
LOCAL_SRC_FILES := pktlogconf.c
LOCAL_CFLAGS += -Wall -Werror
LOCAL_SANITIZE := integer_overflow
include $(BUILD_EXECUTABLE)
