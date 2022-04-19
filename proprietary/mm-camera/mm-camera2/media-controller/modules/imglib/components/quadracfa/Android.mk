LOCAL_PATH := $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

###d remosaic library to be used
include $(CLEAR_VARS)

COMMON_CFLAGS := -Wno-non-virtual-dtor -fno-exceptions
COMMON_CFLAGS += -D_ANDROID_

LOCAL_MODULE           := libremosaic_daemon
#LOCAL_CLANG := false
LOCAL_32_BIT_ONLY      := true
LOCAL_MODULE_TAGS      := optional

LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/prebuilt

LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_SRC_FILES := ./prebuilt/dummy/remosaic_dummy.cpp

LOCAL_SHARED_LIBRARIES += libdl libcutils liblog

LOCAL_CFLAGS += -DUSE_API1_REMOSAIC_LIB_DUMMY

LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE_OWNER := qti
LOCAL_32_BIT_ONLY  := true
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

###################################################################################

include $(CLEAR_VARS)

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../../../../mm-camera/mm-camera2
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/include/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/common/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/test/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/components/lib/
COMMON_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/utils
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES  += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/include/
LOCAL_C_INCLUDES  += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES  += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_HEADER_LIBRARIES := libutils_headers

COMMON_CFLAGS := -Wno-non-virtual-dtor -fno-exceptions
COMMON_CFLAGS += -D_ANDROID_

LOCAL_MODULE           := libmmcamera_quadracfa
#LOCAL_CLANG := false
LOCAL_32_BIT_ONLY      := true
LOCAL_MODULE_TAGS      := optional

LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/prebuilt

LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_SRC_FILES := QCameraQuadraCfa.cpp

LOCAL_SHARED_LIBRARIES += libdl libcutils liblog
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

#LOCAL_SHARED_LIBRARIES += libremosaiclib

LOCAL_SHARED_LIBRARIES += libremosaic_daemon

LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE_OWNER := qti
LOCAL_32_BIT_ONLY  := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

###################################################################################

#include $(CLEAR_VARS)
#
#LOCAL_MODULE            := quadtest
#LOCAL_MODULE_TAGS       := optional
#LOCAL_CFLAGS            :=  -g -Os -Wall -Wextra -Werror -D_ANDROID_
#
#LOCAL_MODULE           := quadtest
#LOCAL_CLANG := false
#LOCAL_32_BIT_ONLY      := true
#LOCAL_MODULE_TAGS      := optional
#
#LOCAL_C_INCLUDES += $(COMMON_C_INCLUDES)
#
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/prebuilt
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/prebuilt/test
#
#LOCAL_CFLAGS := $(COMMON_CFLAGS)
#LOCAL_SRC_FILES := ./prebuilt/test/remosaic_test.cpp
#
#LOCAL_SHARED_LIBRARIES += libdl libcutils liblog
#
#LOCAL_SHARED_LIBRARIES += libremosaiclib
#
#LOCAL_MODULE_TAGS      := optional
#LOCAL_MODULE_OWNER := qti
#LOCAL_32_BIT_ONLY  := true
#LOCAL_PROPRIETARY_MODULE := true
#
#include $(BUILD_EXECUTABLE)



