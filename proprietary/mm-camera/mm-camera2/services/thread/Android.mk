ifeq ($(call is-vendor-board-platform,QCOM),true)
CAM_TOOLS_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PATH:= $(CAM_TOOLS_PATH)

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_SRC_FILES:= services.c session.c memory.c profile.c decision.c core.c dispatcher.c

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS:= -Werror -O3
LOCAL_CFLAGS += \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \

LOCAL_CFLAGS  += -D_ANDROID
LOCAL_CPPFLAGS  += -D_ANDROID
ifeq ($(BOARD_QTI_CAMERA_V2),true)
LOCAL_CPPFLAGS  += -Wno-error=literal-suffix
endif

LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libdl

LOCAL_PRELINK_MODULE:= true
LOCAL_MODULE := libmmcamera_thread_services
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
endif
