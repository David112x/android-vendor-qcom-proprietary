ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER)),true)

ifeq ($(strip $(BOARD_SUPPORTS_QSTHW_API)),true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
        qsthw_api.cpp

LOCAL_C_INCLUDES := \
        external/tinyalsa/include \
        $(TARGET_OUT_HEADERS)/mm-audio/sound_trigger \
        $(TARGET_OUT_HEADERS)/mm-audio/qsthw \
        $(TARGET_OUT_HEADERS)/mm-audio/qti-audio-server

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
        libdl \
        libtinyalsa \
        libhardware \
        libqtisoundtriggerwrapper

LOCAL_MODULE := libqtisoundtriggerhw
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
LOCAL_PROPRIETARY_MODULE := true

LOCAL_COPY_HEADERS_TO   := mm-audio/qsthw_api
LOCAL_COPY_HEADERS      := qsthw_api.h
LOCAL_COPY_HEADERS      += qsthw_defs.h

include $(BUILD_SHARED_LIBRARY)

#compile qti sound trigger test app
include $(CLEAR_VARS)
LOCAL_SRC_FILES := qsthw_test.c
LOCAL_C_INCLUDES += \
        $(TARGET_OUT_HEADERS)/mm-audio/qsthw_api

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libqtisoundtriggerhw

LOCAL_MODULE := qti_sound_trigger_test
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

include $(BUILD_EXECUTABLE)

endif #BOARD_SUPPORTS_QSTHW_API
endif #BOARD_SUPPORTS_SOUND_TRIGGER
