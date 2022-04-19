ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH := $(call my-dir)
LOCAL_DEPTHMAP_PATH := $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../mm-camera/mm-camera2
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/include/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/lib/
COMMON_C_INCLUDES += $(LOCAL_PATH)/prebuilt/
COMMON_CFLAGS += -D_ANDROID_

LOCAL_MODULE           := libmmcamera_depth_map
LOCAL_32_BIT_ONLY      := true
LOCAL_MODULE_TAGS      := optional
LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/prebuilt/depth_map_wrapper
LOCAL_C_INCLUDES += $(LOCAL_PATH)/prebuilt/depth_map_wrapper/inc
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/camera/QCamera2/stack/common
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_HEADER_LIBRARIES := libsystem_headers
LOCAL_HEADER_LIBRARIES += libutils_headers

LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_SRC_FILES := QCameraDepthMap.cpp

LOCAL_SHARED_LIBRARIES := libdepthmapwrapper
LOCAL_SHARED_LIBRARIES += libdl liblog

LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_OWNER := qti
LOCAL_32_BIT_ONLY  := true
LOCAL_PROPRIETARY_MODULE := true
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
include $(BUILD_SHARED_LIBRARY)


#************* depth_map_wrapper libs start ************#
include $(LOCAL_DEPTHMAP_PATH)/prebuilt/depth_map_wrapper/Android.mk
#************* LLVD libs end ************#
endif
