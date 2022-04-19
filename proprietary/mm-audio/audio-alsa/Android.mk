ifeq ($(call is-vendor-board-platform,QCOM),true)

ifneq ($(BUILD_TINY_ANDROID),true)


ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

mm-audio-alsa-def := -g -O3
mm-audio-alsa-def += -D_ANDROID_

ifeq ($(call is-board-platform-in-list,msm8660 msm8960 msm8974 msm8226 msm8610 copper apq8084 msm8994 msm8992 msm8909 msm8996 msm8952 msm8937 thorium msm8953 msmgold msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 msmnile $(MSMSTEPPE) $(TRINKET) kona lito),true)
mm-audio-alsa-def += -DQDSP6V2
endif

# ---------------------------------------------------------------------------------
# 			Make the Shared library (libaudioalsa)
# ---------------------------------------------------------------------------------

LOCAL_MODULE		:= libaudioalsa
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS	  	:= $(mm-audio-alsa-def)
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/inc
LOCAL_PRELINK_MODULE	:= false
LOCAL_SRC_FILES		:= src/hw.c
LOCAL_COPY_HEADERS_TO   := mm-audio/audio-alsa
LOCAL_COPY_HEADERS      := inc/control.h

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
# 			Make the apps-test (mm-audio-alsa-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE		:= mm-audio-alsa-test
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS	  	:= $(mm-audio-alsa-def)
LOCAL_C_INCLUDES	:= $(LOCAL_PATH)/inc
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libaudioalsa
LOCAL_SRC_FILES		:= test/client.c

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_PROPRIETARY_MODULE := true
endif

include $(BUILD_EXECUTABLE)

endif #BUILD_TINY_ANDROID
endif # is-vendor-board-platform

# ---------------------------------------------------------------------------------
# 					END
# ---------------------------------------------------------------------------------

