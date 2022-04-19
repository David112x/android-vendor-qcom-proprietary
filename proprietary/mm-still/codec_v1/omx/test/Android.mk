OMX_TEST_PATH := $(call my-dir)


############ encoder ################################################
include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_TEST_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID

ifeq ($(PLATFORM_SDK_VERSION), 19)
LOCAL_CFLAGS += -Werror
endif

LOCAL_CFLAGS += -D_ANDROID_
LOCAL_CFLAGS += -include QIDbg.h

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

OMX_HEADER_DIR ?= hardware/qcom/media/mm-core/inc
OMX_CORE_DIR := $(SRC_CAMERA_HAL_DIR)/mm-image-codec

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
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

JPEG_PIPELINE_TARGET_LIST := msm8994
JPEG_PIPELINE_TARGET_LIST += msm8992
JPEG_PIPELINE_TARGET_LIST += msm8996
JPEG_PIPELINE_TARGET_LIST += msm8998
JPEG_PIPELINE_TARGET_LIST += apq8098_latv
JPEG_PIPELINE_TARGET_LIST += sdm660
JPEG_PIPELINE_TARGET_LIST += $(TRINKET)
JPEG_PIPELINE_TARGET_LIST += msmfalcon

ifneq (,$(filter  $(JPEG_PIPELINE_TARGET_LIST),$(TARGET_BOARD_PLATFORM)))
    LOCAL_CFLAGS+= -DOMX_JPEG_PIPELINE
endif

LOCAL_SRC_FILES := buffer_test.c
LOCAL_SRC_FILES += qomx_jpeg_enc_test.c

LOCAL_MODULE           := mm-qomx-ienc-test
LOCAL_SHARED_LIBRARIES := libcutils libdl liblog libqomx_core
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)


############ decoder ################################################
include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_TEST_PATH)
LOCAL_MODULE_TAGS := optional

ifeq ($(PLATFORM_SDK_VERSION), 19)
LOCAL_CFLAGS += -Werror
endif

LOCAL_CFLAGS += -D_ANDROID_
LOCAL_CFLAGS += -include QIDbg.h

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

OMX_CORE_DIR := $(SRC_CAMERA_HAL_DIR)/mm-image-codec

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
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES+= $(LOCAL_PATH)../../../../../../hardware/qcom/camera
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif


LOCAL_SRC_FILES := buffer_test.c
LOCAL_SRC_FILES += qomx_jpeg_dec_test.c

LOCAL_MODULE           := mm-qomx-idec-test
LOCAL_SHARED_LIBRARIES := libcutils libdl liblog libqomx_core
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)
