imx230_qc2002_with_gyro_CHROMATIX_3A_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#                      Make the shared library
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(imx230_qc2002_with_gyro_CHROMATIX_3A_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS:= -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../../../../../../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../../common/
LOCAL_C_INCLUDES += chromatix_imx230_qc2002_with_gyro_4k_preview_lc898212xd.h

LOCAL_SRC_FILES:= chromatix_imx230_qc2002_with_gyro_4k_preview_lc898212xd.c

LOCAL_MODULE           := libchromatix_imx230_qc2002_with_gyro_4k_preview_lc898212xd
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
