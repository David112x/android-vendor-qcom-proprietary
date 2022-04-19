ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER)),true)
ifneq ($(strip $(USE_SOUND_TRIGGER_LEGACY_HAL)),true)
ifneq ($(strip $(BOARD_SUPPORTS_OPENSOURCE_STHAL)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -DSOUND_TRIGGER_PLATFORM=$(TARGET_BOARD_PLATFORM)

# _ANDROID_ is needed by GCS, it uses this flag to determine the struct packing keyword per OS.
# Ideally we should not need this as GCS should be able to
# determine from copmiler flags
LOCAL_CFLAGS += -D_ANDROID_

LOCAL_CFLAGS += -Wall -Werror

LOCAL_SRC_FILES := \
    sound_trigger_hw.c \
    sound_trigger_platform.c \
    st_session.c \
    st_hw_session_lsm.c \
    st_hw_session_gcs.c \
    st_hw_common.c \
    st_buffering.c

ifeq ($(strip $(BOARD_SUPPORTS_QSTHW_API)),true)
    LOCAL_CFLAGS += -DST_EXTN_ENABLED
    LOCAL_SRC_FILES += st_extn/st_hw_extn.c
    LOCAL_SRC_FILES += st_extn/st_hw_extn_intf.c
endif

ifeq ($(strip $(SOUND_TRIGGER_FEATURE_LPMA_ENABLED)),true)
    LOCAL_CFLAGS += -DST_LMPA_EXTN_ENABLED
    LOCAL_SRC_FILES += st_extn/st_hw_lpma_extn.c
endif

ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER_ARM)),true)
    LOCAL_CFLAGS += -DST_ARM_ENABLED
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio-noship/include/ffv
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/../esp/lib/inc
    LOCAL_SRC_FILES += st_hw_session_pcm.c
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_KEEP_ALIVE)),true)
    LOCAL_CFLAGS += -DUSE_KEEP_ALIVE
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_SVA_MULTI_STAGE)),true)
    LOCAL_CFLAGS += -DST_MULTI_STAGE_ENABLED
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/../capiv2_api
    LOCAL_SRC_FILES += st_second_stage.c
endif

ifeq ($(call is-platform-sdk-version-at-least,29),true)
    LOCAL_CFLAGS += -DST_SUPPORT_GET_MODEL_STATE
endif

ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER_CPU_AFFINITY_SET)),true)
    LOCAL_CFLAGS += -DSOUND_TRIGGER_CPU_AFFINITY_SET -D_GNU_SOURCE
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_C_INCLUDES += \
    external/tinyalsa/include \
    external/expat/lib \
    $(call include-path-for, audio-route) \
    $(LOCAL_PATH)/st_extn
ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_HEADER_LIBRARIES := libhardware_headers \
                          libsystem_headers
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/graphite-client/gcs
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/graphite-client/osal
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES += audio_kernel_headers
  LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libtinyalsa \
    libaudioroute \
    libdl \
    libexpat \
    libhardware_legacy

LOCAL_MODULE := sound_trigger.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := $(AUDIOSERVER_MULTILIB)

LOCAL_COPY_HEADERS_TO   := mm-audio/sound_trigger
LOCAL_COPY_HEADERS      := sound_trigger_prop_intf.h

include $(BUILD_SHARED_LIBRARY)

#
# to build sound_trigger_test executable
# uncomment the lines below
#
#include $(CLEAR_VARS)
#
#LOCAL_SRC_FILES := sound_trigger_test.c
#LOCAL_C_INCLUDES += \
#   system/core/include \
#   hardware/libhardware/include
#
#LOCAL_SHARED_LIBRARIES := \
#   libcutils \
#   libhardware
#
#LOCAL_MODULE := sound_trigger_test
#LOCAL_MODULE_OWNER := qti
#LOCAL_MODULE_TAGS := optional
#LOCAL_32_BIT_ONLY := true
#LOCAL_PROPRIETARY_MODULE := true
#
#include $(BUILD_EXECUTABLE)

endif
endif
endif
