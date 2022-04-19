#
# Build ListenJNI library for either full SVA or ST variant
#

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_LISTEN)),true)
# Build Listen JNI for SVA
# all SVA including SVA 1.0 built
BUILD_SVA_JNI := true
BUILD_FULL_SVA := true
endif

ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER)),true)
# Build Listen JNI for SoundTrigger implementation
# subset of SVA 2.x built
BUILD_SVA_JNI := true
BUILD_FULL_SVA := false
endif

ifeq ($(BUILD_SVA_JNI),true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(BUILD_FULL_SVA),false)
LOCAL_SRC_FILES:= \
    src/com_qualcomm_listen_ListenSoundModel.cpp \
    src/SoundModel.cpp

LOCAL_SHARED_LIBRARIES := libutils liblog liblistensoundmodel2
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(TOP)/libnativehelper/include/nativehelper \
    $(TARGET_OUT_HEADERS)/mm-audio/audio-listen
endif

ifeq ($(BUILD_FULL_SVA),true)
LOCAL_CFLAGS += -DAUDIO_LISTEN_ENABLED -DLOG_NDEBUG \
    -DSVA1_SUPPORTED -DLISTEN_SERVICE_USED

LOCAL_SRC_FILES:= \
    src/com_qualcomm_listen_ListenSoundModel.cpp \
    src/SoundModel.cpp \
    src/com_qualcomm_listen_ListenReceiver.cpp \
    src/com_qualcomm_listen_ListenMasterControl.cpp \
    src/com_qualcomm_listen_ListenVoiceWakeupSession.cpp

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_SHARED_LIBRARIES := \
    libnativehelper libutils libbinder \
    libui libcutils libgui liblog \
    liblisten liblistensoundmodel2

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(TOP)/hardware/libhardware/include \
    $(TOP)/hardware/libhardware_legacy/include \
    $(TARGET_OUT_HEADERS)/mm-audio/audio-listen \
    $(TOP)/vendor/qcom/proprietary/mm-audio/audio-listen/framework/service/inc
endif

ifeq ($(call is-platform-sdk-version-at-least,28),true)   #Android P and above
LOCAL_HEADER_LIBRARIES := lib_soundmodel_headers jni_headers
endif

LOCAL_MODULE:= liblistenjni
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_SVA_MULTI_STAGE)),true)
LOCAL_CFLAGS += -DSVA_MULTI_STAGE_ENABLED
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_OWNER  := qti
LOCAL_PROPRIETARY_MODULE := true
endif

include $(BUILD_SHARED_LIBRARY)
endif
