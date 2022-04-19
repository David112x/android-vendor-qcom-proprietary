ifeq ($(call is-board-platform-in-list,kona),true)

####### Build Sensing Shared Lib #######

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

COMMON_INCLUDES :=
LOCAL_C_INCLUDES := $(COMMON_INCLUDES)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/misc

# need kernel to build first to have UAPI headers
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_PREBUILT_INT_KERNEL)
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE := libwigigsensing
LOCAL_CLANG := true
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

WPA_SUPPL_DIR := $(TOP)/external/wpa_supplicant_8

LOCAL_C_INCLUDES += $(WPA_SUPPL_DIR)/src/utils \
                    $(WPA_SUPPL_DIR)/src/common

LOCAL_SRC_FILES += \
  src/wigig_sensing_lib.c \
  src/wigig_sensing_apps.c

LOCAL_COPY_HEADERS_TO := libwigigsensing
LOCAL_COPY_HEADERS := src/wigig_sensing_lib.h \
                      src/wigig_sensing_internal.h \
                      src/wigig_sensing_apps.h

LOCAL_CFLAGS += -D_GNU_SOURCE -Wno-unused-parameter -DPB_FIELD_16BIT
LOCAL_CFLAGS += -DAPPS_LIB=1

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

####### Build Sensing Shared Lib SLPI #######

include $(CLEAR_VARS)

COMMON_INCLUDES :=
COMMON_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES := $(COMMON_INCLUDES)

LOCAL_HEADER_LIBRARIES :=
LOCAL_HEADER_LIBRARIES += libnanopb_headers
LOCAL_HEADER_LIBRARIES += libsns_api_headers
LOCAL_HEADER_LIBRARIES += libqmi_encdec_headers
LOCAL_HEADER_LIBRARIES += libqmi_common_headers

LOCAL_MODULE := libwigigsensing_slpi
LOCAL_CLANG := true
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES += libqmi_common_so
LOCAL_SHARED_LIBRARIES += libqmi_cci
LOCAL_SHARED_LIBRARIES += libqmi_encdec
LOCAL_SHARED_LIBRARIES += libnanopb
LOCAL_SHARED_LIBRARIES += libsns_api

WPA_SUPPL_DIR := $(TOP)/external/wpa_supplicant_8

LOCAL_C_INCLUDES += $(WPA_SUPPL_DIR)/src/utils \
                    $(WPA_SUPPL_DIR)/src/common

LOCAL_SRC_FILES += \
  src/wigig_sensing_lib.c \
  src/wigig_sensing_data.c \
  src/wigig_sensing_encode_decode.c \
  src/wigig_sensing_slpi.c \
  src/sns_wigig.pb.c

LOCAL_COPY_HEADERS_TO := libwigigsensing_slpi
LOCAL_COPY_HEADERS := src/wigig_sensing_lib.h \
                      src/wigig_sensing_data.h \
                      src/wigig_sensing_encode_decode.h \
                      src/wigig_sensing_process.h \
                      src/wigig_sensing_internal.h \
                      src/wigig_sensing_slpi.h \
                      src/sns_wigig.pb.h

LOCAL_CFLAGS += -D_GNU_SOURCE -Wno-unused-parameter -DPB_FIELD_16BIT

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

####### Build Sensing Sample App #######

include $(CLEAR_VARS)

COMMON_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc

LOCAL_C_INCLUDES := $(COMMON_INCLUDES)

LOCAL_CFLAGS := -Wno-unused-parameter

LOCAL_SRC_FILES := src/main.c
LOCAL_SHARED_LIBRARIES := libwigigsensing

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libwigigsensing

LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := sensing_sample_app

include $(BUILD_EXECUTABLE)

endif
