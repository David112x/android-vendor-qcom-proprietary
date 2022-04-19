SRS_PROCESSING_ROOT := $(call my-dir)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_MODULE := lib_audio_srs_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := TruMedia
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_COPY_HEADERS)
else
LOCAL_USE_VNDK := true
LOCAL_COPY_HEADERS_TO := mm-audio/audio-effects
LOCAL_COPY_HEADERS := TruMedia/srs_processing.h
LOCAL_COPY_HEADERS += TruMedia/postpro_patch.h
include $(BUILD_COPY_HEADERS)
endif

