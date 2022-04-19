LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    src/com_qualcomm_listen_ListenSoundModel.cpp \
    src/SoundModel.cpp

LOCAL_SHARED_LIBRARIES := libutils liblog liblistensoundmodel2.qti
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(TOP)/libnativehelper/include/nativehelper \
    $(TARGET_OUT_HEADERS)/mm-audio/audio-listen

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_HEADER_LIBRARIES := lib_soundmodel_headers jni_headers
endif

LOCAL_MODULE:= liblistenjni.qti
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_SVA_MULTI_STAGE)),true)
LOCAL_CFLAGS += -DSVA_MULTI_STAGE_ENABLED
endif

include $(BUILD_SHARED_LIBRARY)
