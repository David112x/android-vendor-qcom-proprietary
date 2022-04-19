CURRENT_LOCAL_PATH := $(LOCAL_PATH)

LOCAL_PATH := $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MMCAMERA_PATH  := $(QC_PROP_ROOT)/mm-camera/mm-camera2


COMMON_C_INCLUDES := $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/include/
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/include/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/kernel-headers/
endif
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/utils/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/lib/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/lib/utils/
COMMON_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-camera-interface/

COMMON_CFLAGS := -Wno-non-virtual-dtor -fno-exceptions
COMMON_CFLAGS += -D_ANDROID_
ifeq ($(TARGET_USES_GRALLOC1), true)
  COMMON_CFLAGS += -DTARGET_USES_GRALLOC1
endif

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
  COMMON_CFLAGS += -DTARGET_USES_GRALLOC
endif

LOCAL_MODULE           := libmmcamera_ppeiscore
#LOCAL_CLANG := false

ifeq ($(CAM_BRINGUP_NEW_SP), true)
  LOCAL_USE_CLANG_LLD := false
endif

LOCAL_32_BIT_ONLY      := true
LOCAL_MODULE_TAGS      := optional

LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcom/display
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common
LOCAL_C_INCLUDES += $(LOCAL_C_HEADER)
LOCAL_C_INCLUDES        += frameworks/native/libs/arect/include
LOCAL_PROPRIETARY_MODULE := true

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_HEADER_LIBRARIES += libcutils_headers
LOCAL_HEADER_LIBRARIES += liblog_headers
LOCAL_HEADER_LIBRARIES += libhardware_headers
LOCAL_HEADER_LIBRARIES += libbinder_headers
LOCAL_HEADER_LIBRARIES += libui_headers
LOCAL_HEADER_LIBRARIES += libnativebase_headers
LOCAL_HEADER_LIBRARIES += libbase_headers

LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_CFLAGS  += -include $(LOCAL_IMGLIB_PATH)/components/include/img_dbg.h

ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) > 25 ))" )))
    LOCAL_CFLAGS  += -DANDROID_O
endif

LOCAL_SRC_FILES := QCameraPPEisCore.cpp ../utils/QCameraBufferMap.cpp

LOCAL_SHARED_LIBRARIES += libdl libcutils liblog libGLESv2_adreno libEGL libGLESv2 libhardware libqdMetaData libutils libmmcamera_dbg libmmcamera_imglib

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE_OWNER := qti
LOCAL_32_BIT_ONLY  := true
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(CURRENT_LOCAL_PATH)
