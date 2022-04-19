# ---------------------------------------------------------------------------------
#             Make the libseccamenc library .so
# ---------------------------------------------------------------------------------
ifeq ($(call is-board-platform-in-list, sdm660),true)
ifneq ($(TARGET_USES_QMAA), true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

commonIncludes := $(TARGET_OUT_HEADERS)/common/inc \
                  $(TOP)/vendor/qcom/proprietary/securemsm/QSEEComAPI \
                  $(TOP)/vendor/qcom/proprietary/securemsm/tzcommon/inc

commonSharedLibraries := liblog \
                         libutils \
                         libQSEEComAPI

sourceFiles := \
               SecCamEnc.cpp

LOCAL_SRC_FILES := $(sourceFiles)
LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE:= libseccamenc
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)
endif
endif
