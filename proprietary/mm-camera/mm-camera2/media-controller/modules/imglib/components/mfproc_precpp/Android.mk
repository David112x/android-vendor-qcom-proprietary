LOCAL_PATH := $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

### MultiFrame Process precpp library to be used
include $(CLEAR_VARS)

COMMON_CFLAGS := -Wno-non-virtual-dtor -fno-exceptions
COMMON_CFLAGS += -D_ANDROID_

LOCAL_MODULE           := libmfproc_dummy_daemon
LOCAL_32_BIT_ONLY      := true
LOCAL_MODULE_TAGS      := optional

LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/prebuilt

LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_SRC_FILES := ./prebuilt/mfproc_dummy_lib/mfproc_dummy.cpp

LOCAL_SHARED_LIBRARIES += libdl libcutils liblog

LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE_OWNER := qti
LOCAL_32_BIT_ONLY  := true
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

###################################################################################

include $(CLEAR_VARS)

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../../../../mm-camera/mm-camera2
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/include/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/common/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/test/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/lib/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/utils
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES  += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/include/
LOCAL_C_INCLUDES  += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES  += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_HEADER_LIBRARIES := libutils_headers

COMMON_CFLAGS := -Wno-non-virtual-dtor -fno-exceptions
COMMON_CFLAGS += -D_ANDROID_

LOCAL_MODULE           := libmmcamera_mfproc_precpp
LOCAL_32_BIT_ONLY      := true
LOCAL_MODULE_TAGS      := optional

LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/prebuilt

LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_SRC_FILES := QCameraMultiFrameProc_Precpp.cpp

LOCAL_SHARED_LIBRARIES += libdl libcutils liblog
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES += libmfproc_dummy_daemon

LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE_OWNER := qti
LOCAL_32_BIT_ONLY  := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)




