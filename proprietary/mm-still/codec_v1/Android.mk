ifneq ($(USE_CAMERA_STUB),true)

LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

include $(LOCAL_PATH)/omx/Android.mk
LOCAL_PATH:= $(LOCAL_DIR_PATH)

include $(LOCAL_PATH)/jpeg_sw/Android.mk
ifeq ($(strip $(JPEGE_VER)),jpeg10)
include $(LOCAL_PATH)/../jpeg_hw_10/Android.mk
endif
ifeq ($(strip $(JPEGE_VER)),gemini)
include $(LOCAL_PATH)/../gemini/Android.mk
endif

include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_DIR_PATH)
JPEG_PATH := $(LOCAL_DIR_PATH)/../jpeg2

ifneq ($(call is-board-platform-in-list, msm8996 sdm660 msm8998 apq8098_latv $(TRINKET)),true)
USE_FASTCV_OPT := true
endif

mmstillv1_defines:= -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h

mmstillv1_defines += -Werror

mmstillv1_defines += -D_ANDROID_
mmstillv1_defines += -DCODEC_V1
mmstillv1_defines += -DLOGE=ALOGE

ifeq ($(strip $(FACT_VER)),codecB)
mmstillv1_defines += -DCODEC_B
endif

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += system/core/libion/include
LOCAL_C_INCLUDES += system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/dma
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils
#following files needs to be created in codec folder
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/mm-image-codec/qexif
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_C_INCLUDES += device/qcom/common/power
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/modules/imglib/components/include/

LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/includes
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/includes
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/stream
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/object
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/tools
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/event
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/module
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/bus
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/mct/port
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/mm-camera/mm-camera2/media-controller/modules/imglib/components/lib

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../jpeg_hw_10/jpegd_hw/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../jpeg_hw_10/jpege_hw/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../jpeg_hw_10/jpegdma_hw/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../gemini/inc
LOCAL_C_INCLUDES += external/openssl/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/adsprpc/inc

LOCAL_CFLAGS := $(mmstillv1_defines)

ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_CFLAGS+= -DJPEG_USE_QDSP6_ENCODER
endif

ifeq ($(USE_FASTCV_OPT),true)
LOCAL_CFLAGS+= -DJPEG_USE_FASTCV_OPT
endif

ifeq ($(call is-board-platform-in-list, sdm660 msm8998 apq8098_latv $(TRINKET)),true)
LOCAL_CFLAGS+= -DCODEC_HB_V2
LOCAL_CFLAGS+= -DJPEG_USE_DMA_V4L2=1
else
LOCAL_CFLAGS+= -DJPEG_USE_DMA_V4L2=0
endif

LOCAL_SRC_FILES := common/QIBase.cpp
LOCAL_SRC_FILES += common/QIPlane.cpp
LOCAL_SRC_FILES += common/QImage.cpp
LOCAL_SRC_FILES += common/QIParams.cpp
LOCAL_SRC_FILES += common/QIThread.cpp
LOCAL_SRC_FILES += common/QIBuffer.cpp
LOCAL_SRC_FILES += common/QIHeapBuffer.cpp
LOCAL_SRC_FILES += common/QIONBuffer.cpp
LOCAL_SRC_FILES += common/QITime.cpp
LOCAL_SRC_FILES += utils/QINode.cpp
LOCAL_SRC_FILES += utils/QIQueue.cpp
LOCAL_SRC_FILES += encoder/QEncodeParams.cpp
LOCAL_SRC_FILES += encoder/QImageSWEncoder.cpp
LOCAL_SRC_FILES += decoder/QIDecoderParams.cpp
LOCAL_SRC_FILES += exif/QExifComposerParams.cpp
LOCAL_SRC_FILES += exif/QExifComposer.cpp
LOCAL_SRC_FILES += exif/QExifParser.cpp
LOCAL_SRC_FILES += qcrypt/QCrypt.cpp
LOCAL_SRC_FILES += exif/QExifCameraTuningParams.cpp

ifeq ($(USE_FASTCV_OPT), true)
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libmmjpeg libmmqjpegdma libfastcvopt
else
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libmmjpeg libmmqjpegdma
endif

ifeq ($(strip $(FACT_VER)),codecB)
LOCAL_SRC_FILES += common/QImageCodecFactoryBR.cpp
LOCAL_SRC_FILES += common/QImageCodecFactoryB.cpp
LOCAL_SRC_FILES += common/QImageCodecFactoryHB_v2.cpp
LOCAL_SRC_FILES += encoder/QImageHWEncoder10.cpp
LOCAL_SRC_FILES += decoder/QImageHWDecoder10.cpp
ifeq ($(USE_PERF_LOCK),true)
LOCAL_CFLAGS+= -DUSE_PERFORMANCE_LOCK
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../jpeg2/test/
LOCAL_SRC_FILES += common/QIPerf.c
endif
LOCAL_SRC_FILES += encoder/QImageHybridEncoder.cpp
ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_SRC_FILES += encoder/QImageQ6Encoder.cpp
LOCAL_SHARED_LIBRARIES += libadsprpc
endif
else
LOCAL_SRC_FILES += common/QImageCodecFactoryA.cpp
LOCAL_SRC_FILES += encoder/QImageGeminiEncoder.cpp
LOCAL_SHARED_LIBRARIES += libgemini
endif

LOCAL_SRC_FILES += common/QImageDMAFactory.cpp

ifeq ($(strip $(TARGET_ENABLE_INSTRUMENTATION)),true)
  LOCAL_CFLAGS += -DTARGET_ENABLE_INSTRUMENTATION
  LOCAL_CFLAGS += --coverage
  LOCAL_LDLIBS := -lgcov
  LOCAL_LDFLAGS := --coverage
endif

LOCAL_SHARED_LIBRARIES += libcrypto
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_HEADER_LIBRARIES := libutils_headers

#LOCAL_CLANG := false

LOCAL_MODULE           := libmmqjpeg_codec
LOCAL_32_BIT_ONLY       := true
LOCAL_PRELINK_MODULE   := false

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH:= $(LOCAL_DIR_PATH)
DMA_PATH:= $(LOCAL_DIR_PATH)/dma

include $(LOCAL_PATH)/test/Android.mk

include $(DMA_PATH)/Android.mk

ifeq ($(call is-board-platform-in-list, msm8610),true)
################ Q6 JPEG prebuilt library #############
  include $(CLEAR_VARS)
  LOCAL_MODULE       := libadsp_jpege_skel
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
  LOCAL_SRC_FILES := ../jpeg_q6/libadsp_jpege_skel.so
  LOCAL_MODULE_TAGS := optional
  LOCAL_MODULE_OWNER := qcom
  LOCAL_32_BIT_ONLY := true
  LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/rfsa/adsp

  include $(BUILD_PREBUILT)

################ component library ######################
endif #8610
endif # ($(USE_CAMERA_STUB),true)
