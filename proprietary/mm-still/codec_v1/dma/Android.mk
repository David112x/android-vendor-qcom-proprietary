LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

include $(CLEAR_VARS)


mmstillv1_defines:= -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h

mmstillv1_defines += -Werror

mmstillv1_defines += -D_ANDROID_
mmstillv1_defines += -DCODEC_V1
mmstillv1_defines += -DLOGE=ALOGE

ifeq ($(strip $(FACT_VER)),codecB)
mmstillv1_defines += -DCODEC_B
endif

LOCAL_CFLAGS := $(mmstillv1_defines)

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES := system/core/libion/include
LOCAL_C_INCLUDES += system/core/libion/kernel-headers/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
else
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/src/os
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg_hw_10/jpegdma_hw/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg_hw_10/jpegd_hw/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg_hw_10/jpege_hw/inc
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/mm-image-codec/qexif
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera/imglib

LOCAL_SRC_FILES := ../common/QIBase.cpp
LOCAL_SRC_FILES += ../common/QIPlane.cpp
LOCAL_SRC_FILES += ../common/QImage.cpp
LOCAL_SRC_FILES += ../common/QIParams.cpp
LOCAL_SRC_FILES += ../common/QIThread.cpp
LOCAL_SRC_FILES += ../common/QIBuffer.cpp
LOCAL_SRC_FILES += ../common/QIHeapBuffer.cpp
LOCAL_SRC_FILES += ../common/QIONBuffer.cpp
LOCAL_SRC_FILES += ../common/QITime.cpp
LOCAL_SRC_FILES += ../utils/QINode.cpp
LOCAL_SRC_FILES += ../utils/QIQueue.cpp

LOCAL_SRC_FILES += QJPEGDMAEngine.cpp
LOCAL_SRC_FILES += QJPEGDMA_V4L2_Engine.cpp

LOCAL_SHARED_LIBRARIES := libdl libcutils liblog
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_HEADER_LIBRARIES := libutils_headers

#LOCAL_CLANG := false

LOCAL_MODULE           := libmmqjpegdma
LOCAL_PRELINK_MODULE   := false

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_32_BIT_ONLY := true
include $(BUILD_SHARED_LIBRARY)
