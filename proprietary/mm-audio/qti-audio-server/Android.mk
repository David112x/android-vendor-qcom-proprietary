ifeq ($(strip $(BOARD_SUPPORTS_QTI_AUDIO_SERVER)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -DQTI_AUDIO_SERVER_ENABLED
LOCAL_CFLAGS += -Wall -Werror

LOCAL_SRC_FILES := \
    qti_audio_server.cpp \
    qti_audio_proxy.cpp \
    qti_audio_server_callback.cpp \
    qti_audio_client.cpp \
    sva/qti_sva_server.cpp \
    sva/qti_sva_proxy.cpp \
    sva/qti_sva_server_callback.cpp \
    qas_client_death_notifier_common.cpp \
    qti_audio_client_death_notifier.cpp \
    sva/qti_sva_client_death_notifier.cpp

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH) \
    external/tinyalsa/include \
    $(TARGET_OUT_HEADERS)/mm-audio/qahw_api/inc \
    $(TARGET_OUT_HEADERS)/mm-audio/qahw/inc \
    $(TARGET_OUT_HEADERS)/mm-audio/qsthw

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libtinyalsa \
    libhardware \
    libdl \
    libbinder \
    libpthread \
    libutils\
    libqahwwrapper

LOCAL_MODULE := libqtiaudioserver
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_COPY_HEADERS_TO   := mm-audio/qti-audio-server
LOCAL_COPY_HEADERS      := \
            qti_audio_server.h \
            qti_audio_server_interface.h \
            qti_audio_server_client.h \
            qti_audio_server_callback.h \
            qti_audio_client.h \
            sva/qti_sva_server_callback.h  \
            sva/qti_sva_server_interface.h \
            sva/qti_sva_server.h \
            qas_client_death_notifier_common.h

include $(BUILD_SHARED_LIBRARY)

endif

