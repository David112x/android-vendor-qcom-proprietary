LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------------
# Define
#-----------------------------------------------------------------
LOCAL_CFLAGS := -D_ANDROID_

#----------------------------------------------------------------
# SRC CODE
#----------------------------------------------------------------
LOCAL_SRC_FILES := src/WFDSurfaceMediaSource.cpp

#----------------------------------------------------------------
# INCLUDE PATH
#----------------------------------------------------------------
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
ifeq ($(TARGET_USES_QCOM_DISPLAY_BSP), true)
    LOCAL_HEADER_LIBRARIES := display_headers
    LOCAL_CFLAGS += -DQTI_BSP
endif
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libmediautils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libstagefright_foundation

LOCAL_32_BIT_ONLY := true
LOCAL_MODULE:= libwfdavenhancements

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
