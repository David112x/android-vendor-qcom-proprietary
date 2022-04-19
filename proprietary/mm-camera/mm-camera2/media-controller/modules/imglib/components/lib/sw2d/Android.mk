LOCAL_PATH := $(call my-dir)
LOCAL_IMGLIB_PATH := $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/modules/imglib/

include $(CLEAR_VARS)

COMMON_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
COMMON_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
COMMON_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/utils/
COMMON_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/include/
COMMON_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/lib/
COMMON_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/sw2d/

COMMON_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/../../../includes
COMMON_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
COMMON_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_HEADER_LIBRARIES := libutils_headers

COMMON_CFLAGS := -Wno-non-virtual-dtor -fno-exceptions
COMMON_CFLAGS += -D_ANDROID_ -Werror

#module SW2D
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/include/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/includes/
LOCAL_CFLAGS := $(COMMON_CFLAGS)

USE_FASTCV_OPT ?= true
ifeq ($(USE_FASTCV_OPT),true)
LOCAL_CFLAGS += -DUSE_FASTCV_OPT
endif

LOCAL_SRC_FILES := QCameraPostProcSW2D.cpp

LOCAL_MODULE           := libmmcamera_sw2d_lib
include $(SDCLANG_COMMON_DEFS)
#LOCAL_CLANG            := true
LOCAL_SHARED_LIBRARIES := libmmcamera_imglib libmmcamera_dbg
LOCAL_SHARED_LIBRARIES += libdl libcutils liblog

ifeq ($(USE_FASTCV_OPT),true)
LOCAL_SHARED_LIBRARIES += libfastcvopt
endif

LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
include $(BUILD_SHARED_LIBRARY)
