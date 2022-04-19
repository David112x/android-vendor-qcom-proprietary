ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list, msm8960 apq8084 msm8996 sdm845 msmnile sm6150),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

csd-client-def := -g -O3
csd-client-def += -D_ANDROID_
# csd-client-def += -D_ENABLE_QC_MSG_LOG_
# ---------------------------------------------------------------------------------
#             Make the Shared library (libcsd-client)
# ---------------------------------------------------------------------------------

csd-client-inc          := $(LOCAL_PATH)/inc

LOCAL_MODULE            := libcsd-client
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(csd-client-def)
LOCAL_C_INCLUDES        := $(csd-client-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audcal
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audio-acdb-util
LOCAL_C_INCLUDES        += $(LOCAL_PATH)/../../../mm-audio-cal/audio-acdb-util/acdb-loader/inc/8974
LOCAL_C_INCLUDES        += $(LOCAL_PATH)/../../../mm-audio-cal/audio-acdb-util/acdb-loader/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/libmdmdetect/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES	+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
  LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

LOCAL_SHARED_LIBRARIES  := libutils libqmi_cci libqmi_common_so libacdbloader libcutils libmdmdetect
LOCAL_COPY_HEADERS_TO   := mm-audio/audio-qmi/csd-client
LOCAL_COPY_HEADERS      := inc/csd_client.h

LOCAL_SRC_FILES         := src/csd_client.c \
                           src/core_sound_driver_v01.c \

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

ifeq ($(TARGET_BOARD_AUTO),true)
  LOCAL_CFLAGS += -DPLATFORM_AUTO
endif

ifeq ($(call is-board-platform-in-list, msmnile),true)
 LOCAL_CFLAGS += -DPLATFORM_AUTO
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # BUILD_TINY_ANDROID
endif # is-board-platform-in-list
endif

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
