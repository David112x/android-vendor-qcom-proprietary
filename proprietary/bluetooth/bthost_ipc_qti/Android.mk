LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -Wno-unused-variable
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_CFLAGS += -Wno-format
LOCAL_CFLAGS += -Wno-sign-compare
LOCAL_CFLAGS += -Wno-format-extra-args
LOCAL_CFLAGS += -Wno-sometimes-uninitialized
LOCAL_CFLAGS += -Wno-sign-compare
LOCAL_CFLAGS += -Wno-unused-function

LOCAL_SRC_FILES := bthost_ipc.c
LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/bthost_ipc.h

LOCAL_MODULE := libbthost_if_qti
LOCAL_MODULE_SUFFIX  := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_HEADER_LIBRARIES := libhardware_headers
LOCAL_MODULE_TAGS := optional
ifdef TARGET_2ND_ARCH
LOCAL_MODULE_PATH_32 := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_VENDOR)/lib64
else
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
endif

include $(BUILD_SHARED_LIBRARY)
