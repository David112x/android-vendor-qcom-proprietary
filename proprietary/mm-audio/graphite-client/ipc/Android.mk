ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(call is-board-platform-in-list, msm8998 apq8098_latv sdm660 sdm845 sdm710 msm8953 msm8937 qcs605 msm8909 msmnile $(MSMSTEPPE) $(TRINKET) kona lito atoll),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

ipc-def := -D_ANDROID_

ifeq ($(AUDIO_FEATURE_ENABLED_BG_KEYWORD_DETECTION),true)
ipc-def += -DBG_KWD
endif # is platform MSM8909

# ---------------------------------------------------------------------------------
#             Make the Shared library (libgcs-ipc)
# ---------------------------------------------------------------------------------

LOCAL_CFLAGS            := $(ipc-def)
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/graphite-client/osal/
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
LOCAL_C_INCLUDES        += external/expat/lib
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_HEADER_LIBRARIES += libutils_headers
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES += audio_kernel_headers
  LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
endif

LOCAL_SHARED_LIBRARIES  := liblog libexpat libgcs-osal
LOCAL_COPY_HEADERS_TO   := mm-audio/graphite-client/ipc
LOCAL_COPY_HEADERS      := inc/packetizer.h

LOCAL_SRC_FILES         := src/packetizer.c \
                           src/ipc.c \
                           src/platform_info.c

LOCAL_MODULE             := libgcs-ipc
LOCAL_MODULE_TAGS        := optional
LOCAL_MODULE_OWNER       := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

include $(BUILD_SHARED_LIBRARY)

endif # BUILD_TINY_ANDROID
endif # is-board-platform-in-list
endif
