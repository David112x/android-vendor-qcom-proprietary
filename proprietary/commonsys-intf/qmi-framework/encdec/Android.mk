LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

common_libencdec_cflags := -g
common_libencdec_cflags += -O2
common_libencdec_cflags += -fno-inline
common_libencdec_cflags += -fno-short-enums
common_libencdec_cflags += -fpic
common_libencdec_cflags += -Wall
common_libencdec_cflags += -Werror
common_libencdec_cflags += -Wno-unused-parameter

LOCAL_CFLAGS := $(common_libencdec_cflags)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc

LOCAL_SRC_FILES  := qmi_idl_accessor.c
LOCAL_SRC_FILES  += qmi_idl_lib.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils

LOCAL_MODULE := libqmi_encdec

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

common_libencdec_cflags := -g
common_libencdec_cflags += -O2
common_libencdec_cflags += -fno-inline
common_libencdec_cflags += -fno-short-enums
common_libencdec_cflags += -fpic
common_libencdec_cflags += -Wall
common_libencdec_cflags += -Werror
common_libencdec_cflags += -Wno-unused-parameter

LOCAL_CFLAGS := $(common_libencdec_cflags)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc

LOCAL_SRC_FILES  := qmi_idl_accessor.c
LOCAL_SRC_FILES  += qmi_idl_lib.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils

LOCAL_MODULE := libqmi_encdec

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_STATIC_LIBRARY)

#System Library
include $(CLEAR_VARS)

common_libencdec_cflags := -g
common_libencdec_cflags += -O2
common_libencdec_cflags += -fno-inline
common_libencdec_cflags += -fno-short-enums
common_libencdec_cflags += -fpic
common_libencdec_cflags += -Wall
common_libencdec_cflags += -Werror
common_libencdec_cflags += -Wno-unused-parameter

LOCAL_CFLAGS := $(common_libencdec_cflags)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc

LOCAL_SRC_FILES  := qmi_idl_accessor.c
LOCAL_SRC_FILES  += qmi_idl_lib.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils

LOCAL_MODULE := libqmi_encdec_system
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

# Header library
include $(CLEAR_VARS)

LOCAL_MODULE := libqmi_encdec_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/../inc

include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

common_libencdec_cflags := -g
common_libencdec_cflags += -O2
common_libencdec_cflags += -fno-inline
common_libencdec_cflags += -fno-short-enums
common_libencdec_cflags += -fpic
common_libencdec_cflags += -Wall
common_libencdec_cflags += -Werror
common_libencdec_cflags += -Wno-unused-parameter

LOCAL_CFLAGS := $(common_libencdec_cflags)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc

LOCAL_SRC_FILES  := qmi_idl_accessor.c
LOCAL_SRC_FILES  += qmi_idl_lib.c

LOCAL_MODULE := libqmi_encdec

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_HOST_SHARED_LIBRARY)
