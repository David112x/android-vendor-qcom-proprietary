# Copyright 2013 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    librilutils.c \
    record_stream.c \

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../include

LOCAL_MODULE:= qcrild_librilutils
sapapi_generated_dir := $(call generated-sources-dir-for,SHARED_LIBRARIES,$(LOCAL_MODULE),,)

LOCAL_VENDOR_MODULE:= true
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := qcrild_librilutils-headers
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := qti
LOCAL_CFLAGS += $(qcril_cflags)
LOCAL_CPPLAGS += $(qcril_cppflags)
LOCAL_LDFLAGS += $(qcril_ldflags)
ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE := $(qcril_sanitize)
endif

include $(BUILD_HEADER_LIBRARY)


# Create static library for those that want it
# =========================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    librilutils.c \
    record_stream.c \

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../include

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE := $(qcril_sanitize)
endif

LOCAL_MODULE:= qcrild_librilutils_static

sapapi_generated_dir := $(call generated-sources-dir-for,SHARED_LIBRARIES,$(LOCAL_MODULE),,)

LOCAL_VENDOR_MODULE:= true
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_STATIC_LIBRARY)


