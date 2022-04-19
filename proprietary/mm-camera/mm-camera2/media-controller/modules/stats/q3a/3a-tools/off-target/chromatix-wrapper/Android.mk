LOCAL_PATH := $(call my-dir)
VENDOR_QPROP_PATH=$(QC_PROP_ROOT)

include $(CLEAR_VARS)

ifeq ($(call is-board-platform-in-list,msm8998 apq8098_latv),true)
  CHROMATIX_VERSION := 0310
else
  CHROMATIX_VERSION := 0309
endif

LOCAL_CFLAGS += -DMMCAMERA2
LOCAL_CFLAGS  += -D_ANDROID_

LOCAL_MMCAMERA_PATH  := $(VENDOR_QPROP_PATH)/mm-camera/mm-camera2
LOCAL_MMCAMERACORE_PATH  := $(VENDOR_QPROP_PATH)/mm-camera-core
LOCAL_MM3ACORE_PATH  := $(VENDOR_QPROP_PATH)/mm-3a-core
LOCAL_MMCAMERA_NOSHIP_PATH  := $(VENDOR_QPROP_PATH)/mm-camera-noship
LOCAL_MMCAMERA_SDK_PATH := $(VENDOR_QPROP_PATH)/mm-camerasdk


LOCAL_C_INCLUDES += $(LOCAL_MM3ACORE_PATH)/stats
LOCAL_C_INCLUDES += $(LOCAL_MM3ACORE_PATH)/stats/debug-data
LOCAL_C_INCLUDES += $(LOCAL_MM3ACORE_PATH)/stats/mobicat-data
LOCAL_C_INCLUDES += $(LOCAL_MM3ACORE_PATH)/stats/q3a/aec/algorithm
LOCAL_C_INCLUDES += $(LOCAL_MM3ACORE_PATH)/stats/q3a/awb/algorithm
LOCAL_C_INCLUDES += $(LOCAL_MM3ACORE_PATH)/stats/q3a/af/algorithm
LOCAL_C_INCLUDES += $(LOCAL_MM3ACORE_PATH)/stats/q3a/3a-tools/off-target/platform/inc

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/debug-data
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/include
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/chromatix-wrapper

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_SDK_PATH)/sensor/includes/$(CHROMATIX_VERSION)

LOCAL_SRC_DIR := $(LOCAL_PATH)/../../../../chromatix-wrapper
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name 'stats_chromatix_wrapper.c' | sed s:^$(LOCAL_PATH)::g )

ifeq ($(call is-board-platform-in-list,msm8998 apq8098_latv),true)
  LOCAL_SRC_DIR := $(LOCAL_PATH)/../../../af_v2
else
  LOCAL_SRC_DIR := $(LOCAL_PATH)/../../../af
endif
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name 'af_algo_params.c' | sed s:^$(LOCAL_PATH)::g )


LOCAL_MODULE           := test_chromatix_wrapper
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_TAGS      := optional eng

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_HOST_STATIC_LIBRARY)
