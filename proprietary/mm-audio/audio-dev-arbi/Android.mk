ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DEV_ARBI)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    aud_dev_arbi.cpp \
    aud_dev_arbi_server_if.cpp \
    aud_dev_arbi_client_if.cpp

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog

LOCAL_C_INCLUDES += \
    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_HEADER_LIBRARIES := libutils_headers libhardware_headers

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_COPY_HEADERS_TO   := mm-audio/audio-dev-arbi
LOCAL_COPY_HEADERS      := aud_dev_arbi_client_if.h

LOCAL_MODULE := libaudiodevarb
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif
endif
