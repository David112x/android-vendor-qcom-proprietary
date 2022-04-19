ifeq ($(call is-board-platform-in-list,msm8909 msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
libacdb-mcs-inc     += $(LOCAL_PATH)/inc
libacdb-mcs-inc     += $(LOCAL_PATH)/src

LOCAL_C_INCLUDES        := $(libacdb-mcs-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audcal
LOCAL_SRC_FILES:= src/acdb-mcs-test.c
LOCAL_MODULE := acdb-mcs-test

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_SHARED_LIBRARIES:= libcutils libutils libacdbloader
LOCAL_MODULE_TAGS := optional
ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_PROPRIETARY_MODULE := true
endif
include $(BUILD_EXECUTABLE)

endif
endif # is-board-platform-in-list

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
