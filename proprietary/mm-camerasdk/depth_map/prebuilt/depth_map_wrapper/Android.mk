# ==============================================================================
# ---------------------------------------------------------------------------------
#           Make the depthmap wrapper libaray (libdepthmapwrapper.so)
# ---------------------------------------------------------------------------------

SM_USES_DSP := true

LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE            := libdepthmapwrapper
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -g -Os -Wall -Wextra  -D_ANDROID_ -Wno-format-security
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libcutils libdl

LOCAL_C_INCLUDES    := $(LOCAL_PATH) \
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_SRC_FILES  := DepthMapWrapper.cpp
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti
ifeq ($(32_BIT_FLAG), true)
  LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)


