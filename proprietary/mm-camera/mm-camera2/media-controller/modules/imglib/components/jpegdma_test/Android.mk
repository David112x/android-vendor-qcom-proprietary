LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/include/mm-camera/imglib
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/include/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../utils/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../includes
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_CFLAGS := -D_ANDROID_ -Werror

LOCAL_SRC_DIR := $(LOCAL_PATH)
LOCAL_SRC_FILES := jpegdma_test.c

LOCAL_MODULE := mm-camera-jpegdma-v4l2-test
include $(SDCLANG_COMMON_DEFS)
#LOCAL_CLANG := true
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog
LOCAL_SHARED_LIBRARIES += libmmcamera_imglib libmmcamera_dbg

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_32_BIT_ONLY := true

include $(BUILD_EXECUTABLE)

