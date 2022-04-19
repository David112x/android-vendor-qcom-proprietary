ifeq ($(strip $(AUDIO_FEATURE_ENABLED_QAF)),true)
QAF_PROCESSING_ROOT := $(call my-dir)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_USE_VNDK := true
LOCAL_MODULE := audio_qaf_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
include $(BUILD_HEADER_LIBRARY)

endif
