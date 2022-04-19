ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list,msm8937 msm8953 sdm660 sdm845 msmnile $(MSMSTEPPE) $(TRINKET) kona sdm710 lito msm8998 atoll qcs605),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
ROOT_DIR := $(LOCAL_PATH)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libaudioparsers-def := -g -O3
libaudioparsers-def += -D_ANDROID_


# ---------------------------------------------------------------------------------
#             Make the component library (libaudioparsers)
# ---------------------------------------------------------------------------------

libaudioparsers-inc       := $(LOCAL_PATH)/inc
libaudioparsers-inc       += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_MODULE            := libaudioparsers
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libaudioparsers-def)
LOCAL_C_INCLUDES        := $(libaudioparsers-inc)

LOCAL_ADDITIONAL_DEPENDENCIES  += $(KERNEL_HEADERS_INSTALL)


LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog

LOCAL_COPY_HEADERS_TO   := mm-audio/audio-parsers
LOCAL_COPY_HEADERS      := inc/audio_parsers.h

LOCAL_SRC_FILES += src/audio_parsers_interface.c
LOCAL_SRC_FILES += src/audio_parsers_ac3.c
LOCAL_SRC_FILES += src/audio_parsers_dts.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
endif # is-board-platform
endif
