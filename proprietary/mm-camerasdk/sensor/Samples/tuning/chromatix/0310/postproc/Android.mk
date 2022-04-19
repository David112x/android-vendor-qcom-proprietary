IMX230_CHROMATIX_3A_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PATH := $(IMX230_CHROMATIX_3A_PATH)
CHROMATIX_VERSION := 0310

LOCAL_C_INCLUDES := ../includes/$(CHROMATIX_VERSION)
LOCAL_SRC_FILES:= chromatix_imx230_sw_post_proc.c
LOCAL_MODULE           := libchromatix_imx230_postproc_0310

include $(BUILD_SHARED_LIBRARY)
