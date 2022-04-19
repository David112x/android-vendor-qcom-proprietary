
LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_DIR_PATH)

JPEG_PATH := $(LOCAL_DIR_PATH)/../../jpeg2

jpeg_test_defines:= -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h \

jpeg_test_defines += -D_ANDROID_
jpeg_test_defines += -DLOGE=ALOGE
jpeg_test_defines += -DCODEC_V1

ifeq ($(strip $(FACT_VER)),codecB)
jpeg_test_defines += -DCODEC_B
endif

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += system/core/libion/include
LOCAL_C_INCLUDES += system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dma
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../exif
# remove the following after reimplementing the exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/mm-image-codec/qexif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
endif
endif

LOCAL_CFLAGS := $(jpeg_test_defines)

ifeq ($(call is-board-platform-in-list, msm8916 msm8952 msm8937 msm8953),true)
LOCAL_CFLAGS+= -DCODEC_BR
endif

LOCAL_SRC_FILES := QJpegEncoderTest.cpp

LOCAL_SHARED_LIBRARIES:= \
   libmmqjpeg_codec libcutils libdl liblog
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_MODULE:= mm-qjpeg-enc-test

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)

################# decoder #####################################
ifeq ($(strip $(FACT_VER)),codecB)

include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_DIR_PATH)

JPEG_PATH := $(LOCAL_DIR_PATH)/../../jpeg2

jpeg_test_defines:= -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h

ifeq ($(PLATFORM_SDK_VERSION), 19)
jpeg_test_defines += -Werror
endif
jpeg_test_defines += -D_ANDROID_
jpeg_test_defines += -DLOGE=ALOGE
jpeg_test_defines += -DCODEC_V1

ifeq ($(strip $(FACT_VER)),codecB)
jpeg_test_defines += -DCODEC_B
endif

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += system/core/libion/include
LOCAL_C_INCLUDES += system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dma
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../exif
# remove the following after reimplementing the exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/mm-image-codec/qexif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
endif
endif

LOCAL_CFLAGS := $(jpeg_test_defines)

ifeq ($(call is-board-platform-in-list, msm8916 msm8952 msm8937 msm8953),true)
LOCAL_CFLAGS+= -DCODEC_BR
endif

LOCAL_SRC_FILES := QJpegDecoderTest.cpp

LOCAL_SHARED_LIBRARIES:= \
    libmmqjpeg_codec libcutils libdl liblog
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_MODULE:= mm-qjpeg-dec-test

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)
endif

################# makernote data extractor #####################################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_DIR_PATH)

JPEG_PATH := $(LOCAL_DIR_PATH)/../../jpeg2

jpeg_test_defines:= -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h

ifeq ($(PLATFORM_SDK_VERSION), 19)
jpeg_test_defines += -Werror
endif

jpeg_test_defines += -D_ANDROID_
jpeg_test_defines += -DLOGE=ALOGE
jpeg_test_defines += -DCODEC_V1

OPENSSL_HEADER_DIR := external/openssl/include

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += system/core/libion/include
LOCAL_C_INCLUDES += system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dma
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../exif
# remove the following after reimplementing the exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(OPENSSL_HEADER_DIR)
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/mm-image-codec/qexif
LOCAL_C_INCLUDES  += $(LOCAL_PATH)/../qcrypt
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
endif
endif

LOCAL_CFLAGS := $(jpeg_test_defines)

LOCAL_SRC_FILES := QJpegMakernoteExtract.cpp

LOCAL_SHARED_LIBRARIES:= \
   libmmqjpeg_codec libcutils libdl liblog
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_MODULE:= qmakernote-xtract

LOCAL_MODULE_TAGS := optional

LOCAL_PROPRIETARY_MODULE := true

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)

################# jpeg dma test #####################################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_DIR_PATH)

JPEG_PATH := $(LOCAL_DIR_PATH)/../../jpeg2

jpeg_test_defines:=  -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h \

jpeg_test_defines += -D_ANDROID_
jpeg_test_defines += -DLOGE=ALOGE
jpeg_test_defines += -DCODEC_V1

OPENSSL_HEADER_DIR := external/openssl/include

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += system/core/libion/include
LOCAL_C_INCLUDES += system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dma
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg_hw_10/jpegdma_hw/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../exif
# remove the following after reimplementing the exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/mm-image-codec/qexif
LOCAL_C_INCLUDES  += $(LOCAL_PATH)/../qcrypt

ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
endif
endif

LOCAL_CFLAGS := $(jpeg_test_defines)

LOCAL_SRC_FILES := QJpegDmaTest.cpp

LOCAL_SHARED_LIBRARIES:= \
   libmmqjpegdma libcutils libdl liblog
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_MODULE:= qjpeg-dma-test

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)
