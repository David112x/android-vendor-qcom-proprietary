ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8998),true)
LOCAL_PATH := $(call my-dir)

# Include the prebuilt sns_low_lat_stream_skel shared library
include $(CLEAR_VARS)
LOCAL_SRC_FILES     := prebuilt/lib/q6/libsns_low_lat_stream_skel.so
LOCAL_MODULE        := libsns_low_lat_stream_skel
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS  := ETC

LOCAL_MODULE_OWNER  := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH   := $(TARGET_OUT_VENDOR)/lib/rfsa/adsp
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_SRC_FILES     := prebuilt/lib/q6/libsns_low_lat_stream_skel.so
LOCAL_MODULE        := libsns_low_lat_stream_skel_system
LOCAL_MODULE_CLASS  := ETC

LOCAL_MODULE_PATH   := $(TARGET_OUT)/lib/rfsa/adsp
LOCAL_INSTALLED_MODULE_STEM := libsns_low_lat_stream_skel.so
include $(BUILD_PREBUILT)


# Compile the sns_low_lat_stream_stub shared library
include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/sns_low_lat_stream_stub.c
LOCAL_MODULE    := libsns_low_lat_stream_stub
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_CFLAGS := -D_GNU_SOURCE -DSNS_LA
LOCAL_SHARED_LIBRARIES += libdl liblog libc libsdsprpc
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER  := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/sns_low_lat_stream_stub.c
LOCAL_MODULE    := libsns_low_lat_stream_stub_system
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_CFLAGS := -D_GNU_SOURCE -DSNS_LA
LOCAL_SHARED_LIBRARIES += libdl liblog libc libsdsprpc_system
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_INSTALLED_MODULE_STEM := libsns_low_lat_stream_stub.so
include $(BUILD_SHARED_LIBRARY)


# Compile the sns_low_lat_test_client executable
include $(CLEAR_VARS)
SRC_C_LIST := $(wildcard $(LOCAL_PATH)/test/src/*.c)
LOCAL_SRC_FILES +=$(SRC_C_LIST:$(LOCAL_PATH)/%=%)
LOCAL_MODULE    := sns_low_lat_test_client
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_SHARED_LIBRARIES += \
    libdl \
    liblog \
    libc \
    libsdsprpc \
    libsns_low_lat_stream_stub
LOCAL_CFLAGS := -D_GNU_SOURCE
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER  := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_32_BIT_ONLY   := true
include $(BUILD_EXECUTABLE)


endif
endif
