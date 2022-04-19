S5K3M2XX_CHROMATIX_3A_CAMCORDER_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#                      Make the shared library
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(S5K3M2XX_CHROMATIX_3A_CAMCORDER_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        $(CHROMATRIX_LOCAL_CFLAGS) \
        -include camera_defs_i.h

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../../../../../../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../../common/
LOCAL_C_INCLUDES += chromatix_s5k3m2xx_1080p_video_ad5816g.h

LOCAL_SRC_FILES:= chromatix_s5k3m2xx_1080p_video_ad5816g.c

LOCAL_MODULE           := libchromatix_s5k3m2xx_1080p_video_ad5816g
LOCAL_SHARED_LIBRARIES := libcutils
include $(LOCAL_PATH)/../../../../../../../../../local_additional_dependency.mk

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
