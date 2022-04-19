LOCAL_PATH := $(call my-dir)

C_INCLUDES := $(LOCAL_PATH)/include \
              $(LOCAL_PATH)/libtlv2 \
              $(LOCAL_PATH)/libtlv2/include \
              $(LOCAL_PATH)/libtlvutil \
              $(LOCAL_PATH)/libtlvutil/common \

include $(CLEAR_VARS)

LOCAL_CLANG := true
LOCAL_MODULE = libtestcmd6174
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS = $(L_CFLAGS) -D_HOST_SIM_TESTING -DLinux -DUSE_TLV2
LOCAL_SRC_FILES = athtestcmdlib.c

ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED), true)
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_C_INCLUDES += $(C_INCLUDES)
LOCAL_HEADER_LIBRARIES := libtcmd_headers

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_WHOLE_STATIC_LIBRARIES := libtcmd libtlvutil libtlv2

ifneq ($(wildcard system/core/libnl_2),)
# ICS ships with libnl 2.0
LOCAL_SHARED_LIBRARIES += libnl_2
else
LOCAL_SHARED_LIBRARIES += libnl
endif

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/testcmd6174.h \
                               $(LOCAL_PATH)/include/testcmd.h \
                               $(LOCAL_PATH)/include/art_utf_common.h
LOCAL_SANITIZE := signed-integer-overflow unsigned-integer-overflow
include $(BUILD_STATIC_LIBRARY)

