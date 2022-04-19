OMX_JPEG_PATH := $(call my-dir)

# ------------------------------------------------------------------------------
#                Make the shared library (libqomx_encoder)
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)

ifneq ($(PLATFORM_SDK_VERSION), 19)
LOCAL_32_BIT_ONLY := true
else
LOCAL_32_BIT_ONLY := false
endif

ifeq ($(OEM_CHROMATIX_0308), 1)
  CHROMATIX_VERSION := 0308E
else
  CHROMATIX_VERSION := 0309
endif

LOCAL_PATH := $(OMX_JPEG_PATH)
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

ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_CFLAGS+= -DJPEG_USE_QDSP6_ENCODER
endif

LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../system/core/libion/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../exif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../mobicat
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dma
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../qcrypt
LOCAL_C_INCLUDES += external/openssl/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../adsprpc/inc


LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/stream
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/object
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/includes
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/tools
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/includes
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/event
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/module
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/bus
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/port
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/server-tuning/tuning
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)

LOCAL_SRC_FILES := OMXImageEncoder.cpp
LOCAL_SRC_FILES += OMXJpegEncoder.cpp
LOCAL_SRC_FILES += ../mobicat/QMobicatComposer.cpp

LOCAL_MODULE           := libqomx_jpegenc
LOCAL_SHARED_LIBRARIES := libcutils liblog libdl libmmjpeg libmmqjpeg_codec
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_WHOLE_STATIC_LIBRARIES := qomx_core_helper
ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_SHARED_LIBRARIES += libadsprpc
endif

ifeq ($(strip $(TARGET_ENABLE_INSTRUMENTATION)),true)
  LOCAL_CFLAGS += -DTARGET_ENABLE_INSTRUMENTATION
  LOCAL_CFLAGS += --coverage
  LOCAL_LDLIBS := -lgcov
  LOCAL_LDFLAGS := --coverage
endif

#LOCAL_CLANG := false
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

# ------------------------------------------------------------------------------
#                Make the shared library (libqomx_jpegenc_pipe)
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_JPEG_PATH)
JPEG_PATH := $(LOCAL_PATH)/../../../jpeg2

ifneq ($(PLATFORM_SDK_VERSION), 19)
LOCAL_32_BIT_ONLY := true
else
LOCAL_32_BIT_ONLY := false
endif

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

ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_CFLAGS+= -DJPEG_USE_QDSP6_ENCODER
endif

ifeq ($(call is-board-platform-in-list, sdm660 msm8998 apq8098_latv $(TRINKET)),true)
LOCAL_CFLAGS+= -DJPEG_USE_DMA_V4L2=1
else
LOCAL_CFLAGS+= -DJPEG_USE_DMA_V4L2=0
endif

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_CFLAGS+= -DJPEG_USE_NO_DMACPY=1
else
LOCAL_CFLAGS+= -DJPEG_USE_NO_DMACPY=0
endif

LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../system/core/libion/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../exif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../mobicat
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dma
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../qcrypt
LOCAL_C_INCLUDES += external/openssl/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../adsprpc/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../jpeg_hw_10/jpegdma_hw/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera/imglib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../jpeg_encoder


LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/stream
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/object
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/includes
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/tools
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/includes
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/event
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/module
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/bus
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/port
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/server-tuning/tuning
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)

LOCAL_SRC_FILES := ../common/QOMX_Buffer.cpp
LOCAL_SRC_FILES += ../common/QIMessage.cpp
LOCAL_SRC_FILES += OMXImageEncoder.cpp
LOCAL_SRC_FILES += OMXJpegEncoderPipeline.cpp
LOCAL_SRC_FILES += ../common/qomx_core_component.cpp
LOCAL_SRC_FILES += ../common/QOMXImageCodec.cpp
LOCAL_SRC_FILES += ../mobicat/QMobicatComposer.cpp

LOCAL_MODULE           := libqomx_jpegenc_pipe
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libdl liblog libmmqjpeg_codec libmmjpeg libmmqjpegdma
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif

LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_SHARED_LIBRARIES += libadsprpc
endif

#LOCAL_CLANG := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

