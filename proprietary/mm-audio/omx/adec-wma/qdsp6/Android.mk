ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list,msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 msmnile $(MSMSTEPPE) $(TRINKET) kona lito atoll),true)
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxWmaDec-def := -g -O3
libOmxWmaDec-def += -DQC_MODIFIED
libOmxWmaDec-def += -D_ANDROID_
libOmxWmaDec-def += -D_ENABLE_QC_MSG_LOG_
libOmxWmaDec-def += -DVERBOSE
libOmxWmaDec-def += -D_DEBUG
libOmxWmaDec-def += -DAUDIOV2
libOmxWmaDec-def += -Wconversion
ifeq ($(call is-board-platform-in-list,msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 msmnile $(MSMSTEPPE) $(TRINKET) kona lito atoll),true)
libOmxWmaDec-def += -DQCOM_AUDIO_USE_SYSTEM_HEAP_ID
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_NT_PAUSE_TIMEOUT)),true)
libOmxWmaDec-def += -DNT_PAUSE_TIMEOUT_ENABLED
endif

# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxWmaDec)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxWmaDec-inc        := $(LOCAL_PATH)/inc
TARGET_INCLUDE_LIBION := false
ifeq ($(call is-board-platform-in-list,msmnile $(MSMSTEPPE) $(TRINKET) kona lito atoll),true)
    TARGET_INCLUDE_LIBION := true
else
    ifeq ($(call is-board-platform-in-list,sdm660),true)
        ifeq ($(TARGET_KERNEL_VERSION), 4.14)
           TARGET_INCLUDE_LIBION := true
        endif
    endif
endif
ifeq ($(TARGET_INCLUDE_LIBION),true)
libOmxWmaDec-inc        += system/core/libion/include
libOmxWmaDec-inc        += system/core/libion/kernel-headers
endif
libOmxWmaDec-inc        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
libOmxWmaDec-inc	+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
libOmxWmaDec-inc        += $(TARGET_OUT_HEADERS)/mm-core/omxcore
libOmxWmaDec-inc        += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
  libOmxWmaDec-inc += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

LOCAL_MODULE            := libOmxWmaDec
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libOmxWmaDec-def)
LOCAL_C_INCLUDES        := $(libOmxWmaDec-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog
ifeq ($(TARGET_INCLUDE_LIBION),true)
LOCAL_SHARED_LIBRARIES  += libion
endif

LOCAL_SRC_FILES         := src/adec_svr.c
LOCAL_SRC_FILES         += src/omx_wma_adec.cpp

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#             Make the apps-test (mm-adec-omxwma-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-wma-dec-test-inc     := $(LOCAL_PATH)/inc
mm-wma-dec-test-inc     += $(LOCAL_PATH)/test
mm-wma-dec-test-inc     += $(TARGET_OUT_HEADERS)/mm-audio/audio-alsa
mm-wma-dec-test-inc     += $(TARGET_OUT_HEADERS)/mm-core/omxcore
mm-wma-dec-test-inc     += $(TARGET_OUT_HEADERS)/common/inc
mm-wma-dec-test-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
mm-wma-dec-test-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
  mm-wma-dec-test-inc += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

LOCAL_MODULE            := mm-adec-omxwma-test
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libOmxWmaDec-def)
LOCAL_C_INCLUDES        := $(mm-wma-dec-test-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libmm-omxcore
LOCAL_SHARED_LIBRARIES  += libOmxWmaDec
LOCAL_SHARED_LIBRARIES  += libaudioalsa

LOCAL_SRC_FILES         := test/omx_wma_dec_test.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_PROPRIETARY_MODULE := true
endif

include $(BUILD_EXECUTABLE)

endif #BUILD_TINY_ANDROID
endif #TARGET_BOARD_PLATFORM
endif
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
