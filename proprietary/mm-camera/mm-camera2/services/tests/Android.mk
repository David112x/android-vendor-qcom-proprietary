ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
#
# unit test executables
#

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

# Global flag and include definitions
TEST_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  -DMSM_CAMERA_BIONIC

ifeq ($(BOARD_QTI_CAMERA_V2),false)
  TEST_CFLAGS  += -Werror
endif

TEST_C_INCLUDES:= $(LOCAL_PATH)
TEST_C_INCLUDES+= $(LOCAL_PATH)/../thread/
TEST_C_INCLUDES+= $(LOCAL_PATH)/../thread/include

LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_CFLAGS  += -D_ANDROID_

LOCAL_C_INCLUDES := $(TEST_C_INCLUDES)

LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_SRC_FILES:= test_thread_library.c

LOCAL_SHARED_LIBRARIES:= libcutils liblog libmmcamera_thread_services

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:= test_thread_library
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_TAGS := optional

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

endif
