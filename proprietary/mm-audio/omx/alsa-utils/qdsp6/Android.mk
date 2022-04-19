ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list, msm8909 msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 msmnile $(MSMSTEPPE) $(TRINKET) kona lito atoll),true)
ifneq ($(BUILD_TINY_ANDROID),true)


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


# ---------------------------------------------------------------------------------
#               Common definitons
# ---------------------------------------------------------------------------------

mm-alsa-utils-def := -g -O3
mm-alsa-utils-def += -D_ANDROID_
mm-alsa-utils-def += -DQDSP6V2


# ---------------------------------------------------------------------------------
#             Make the alsa-utils (libalsautils)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-alsa-utils-inc     := $(LOCAL_PATH)/inc
mm-alsa-utils-inc     += $(TARGET_OUT_HEADERS)/mm-audio/audio-alsa
mm-alsa-utils-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
mm-alsa-utils-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
mm-alsa-utils-inc     += $(TARGET_OUT_HEADERS)/mm-audio/libalsa-intf
mm-alsa-utils-inc     += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
  mm-alsa-utils-inc += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif


LOCAL_MODULE            := libalsautils
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(mm-alsa-utils-def)
LOCAL_C_INCLUDES        := $(mm-alsa-utils-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_SHARED_LIBRARIES  := libaudioalsa
LOCAL_SHARED_LIBRARIES  += libalsa-intf
LOCAL_SHARED_LIBRARIES  += libmm-omxcore
LOCAL_LDLIBS            := -lalsa-intf

LOCAL_SRC_FILES         := src/omx_alsa_utils.c
LOCAL_COPY_HEADERS_TO   := mm-audio/omx/alsa-utils/qdsp6/inc/
LOCAL_COPY_HEADERS      := inc/omx_alsa_utils.h
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
endif #TARGET_BOARD_PLATFORM
endif
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
