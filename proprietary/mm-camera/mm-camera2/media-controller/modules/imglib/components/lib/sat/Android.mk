OLD_LOCAL_PATH := $(LOCAL_PATH)
LOCAL_PATH := $(call my-dir)
LOCAL_MMCAMERA_PATH  := $(QC_PROP_ROOT)/mm-camera/mm-camera2
LOCAL_IMGLIB_PATH  := $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/include

include $(CLEAR_VARS)
COMMON_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
COMMON_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/utils/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/lib/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/include/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/common/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/multiframeproc/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes
COMMON_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

COMMON_C_INCLUDES += $(LOCAL_PATH)/wrapper/dummy
COMMON_CFLAGS := -Wno-non-virtual-dtor -fno-exceptions
COMMON_CFLAGS += -D_ANDROID_ -Werror

ifneq ($(CAM_BRINGUP_NEW_SP), true)
LOCAL_CLANG := false
endif
LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_SRC_FILES := wrapper/dummy/QCameraSATDummy.cpp

LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libmmcamera_dbg libmmcamera_imglib
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE           := libmmcamera_sat_test_lib
LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE_OWNER     := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
LOCAL_PATH := $(OLD_LOCAL_PATH)
