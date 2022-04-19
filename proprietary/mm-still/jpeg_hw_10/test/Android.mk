LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../system/core/libion/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_SRC_DIR := $(LOCAL_PATH)
LOCAL_SRC_FILES := jpegdma_drv_test.c

LOCAL_MODULE := mm-still-jpegdma-drv-test
LOCAL_SHARED_LIBRARIES := libdl liblog libcutils
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif

LOCAL_MODULE_TAGS := optional
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
