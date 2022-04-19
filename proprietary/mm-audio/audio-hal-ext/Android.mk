ifneq ($(AUDIO_USE_STUB_HAL), true)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libaudioconfigstore
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

LOCAL_C_INCLUDES += $(LOCAL_PATH) \
                    $(LOCAL_PATH)/local

LOCAL_SRC_FILES := ConfigParser.cpp \
                   local/AudioHalExtLocal.cpp

LOCAL_SHARED_LIBRARIES := libexpat liblog

LOCAL_COPY_HEADERS_TO := mm-audio/audio-hal-ext
LOCAL_COPY_HEADERS := local/AudioHalExtLocal.h

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
