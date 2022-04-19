OMX_JPEG_DEC_PATH := $(call my-dir)

# ------------------------------------------------------------------------------
#                Make the shared library (libqomx_decoder)
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_JPEG_DEC_PATH)
JPEG_PATH := $(LOCAL_PATH)/../../../jpeg2

LOCAL_MODULE_TAGS := optional

omx_jpeg_defines:= -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \
        -include QIDbg.h

omx_jpeg_defines += -Werror

omx_jpeg_defines += -DCODEC_V1

ifeq ($(strip $(FACT_VER)),codecB)
omx_jpeg_defines += -DCODEC_B
endif

LOCAL_CFLAGS := $(omx_jpeg_defines)

ifeq ($(call is-board-platform-in-list, msm8916 msm8952 msm8937 msm8953),true)
LOCAL_CFLAGS+= -DCODEC_BR
endif

OMX_HEADER_DIR ?= hardware/qcom/media/mm-core/inc
OMX_CORE_DIR := $(SRC_CAMERA_HAL_DIR)/mm-image-codec

LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dma
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common

LOCAL_SRC_FILES := OMXImageDecoder.cpp
LOCAL_SRC_FILES += OMXJpegDecoder.cpp

LOCAL_MODULE           := libqomx_jpegdec
LOCAL_SHARED_LIBRARIES := libcutils liblog libdl libmmqjpeg_codec libmmjpeg

LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_WHOLE_STATIC_LIBRARIES := qomx_core_helper

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

#LOCAL_CLANG := false
LOCAL_32_BIT_ONLY := true
include $(BUILD_SHARED_LIBRARY)
