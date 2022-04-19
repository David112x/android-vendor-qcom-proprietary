IMX230_CHROMATIX_3A_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PATH := $(IMX230_CHROMATIX_3A_PATH)
CHROMATIX_VERSION := 0309

LOCAL_C_INCLUDES := ../includes/$(CHROMATIX_VERSION)
LOCAL_SRC_FILES:= chromatix_imx230_preview_cpp.c
LOCAL_MODULE           := libchromatix_imx230_cpp_preview_0309

include $(BUILD_SHARED_LIBRARY)
