ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER)),true)

#compile qti wrapper library
ifeq ($(strip $(BOARD_SUPPORTS_QSTHW_API)),true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
        qsthw.c

LOCAL_C_INCLUDES := \
        external/tinyalsa/include \
        $(TARGET_OUT_HEADERS)/mm-audio/sound_trigger

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
        libdl \
        libtinyalsa \
        libhardware

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_KEEP_ALIVE)),true)
LOCAL_CFLAGS += -DUSE_KEEP_ALIVE
endif

LOCAL_MODULE := libqtisoundtriggerwrapper
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
LOCAL_PROPRIETARY_MODULE := true

LOCAL_COPY_HEADERS_TO   := mm-audio/qsthw
LOCAL_COPY_HEADERS      := qsthw.h
LOCAL_COPY_HEADERS      += qsthw_defs.h

include $(BUILD_SHARED_LIBRARY)

endif #BOARD_SUPPORTS_QSTHW_API
endif #BOARD_SUPPORTS_SOUND_TRIGGER
