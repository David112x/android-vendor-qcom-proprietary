ifeq ($(TARGET_USES_QCOM_BSP),true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    secure_camera_sample_client.cpp \

LOCAL_SHARED_LIBRARIES:= \
    libui \
    libutils \
    libcutils \
    liblog \
    libbinder \
    libgui \
    libcamera_client \
    libQSEEComAPI

LOCAL_C_INCLUDES += \
    $(QC_PROP_ROOT)/securemsm/QSEEComAPI \
    $(QC_PROP_ROOT)/common/inc \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include


LOCAL_MODULE:= secure_camera_sample_client
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_TAGS:= optional

LOCAL_CFLAGS += -Wall $(QSEECOM_CFLAGS)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
#include $(BUILD_EXECUTABLE)
endif
