ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DYNAMIC_LOG)), true)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE    := libaudio_log_utils

LOCAL_SRC_FILES := src/log_xml_parser.c
LOCAL_SRC_FILES += src/log_utils.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc \
                    external/expat/lib

LOCAL_SHARED_LIBRARIES := \
       libutils \
       libcutils \
       libexpat \
       liblog

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := both
LOCAL_COPY_HEADERS_TO := mm-audio/audio-log-utils
LOCAL_COPY_HEADERS := inc/log_xml_parser.h
LOCAL_COPY_HEADERS += inc/log_utils.h

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_dynamic_log.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := DATA
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/audio
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
