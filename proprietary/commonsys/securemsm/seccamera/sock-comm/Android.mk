ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libseccam-ipc
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := $(COMMON_CFLAGS) \
                -fno-short-enums \
                -g -fdiagnostics-show-option -Wno-format \
                -Wno-missing-braces -Wno-missing-field-initializers \
                -fpermissive -Wno-unused-parameter

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PRODUCT_MODULE := true
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES += \
    libc \
    liblog \
    libandroid

LOCAL_SRC_FILES := \
    src/sock_comm.cpp

include $(BUILD_SHARED_LIBRARY)

endif # not BUILD_TINY_ANDROID
