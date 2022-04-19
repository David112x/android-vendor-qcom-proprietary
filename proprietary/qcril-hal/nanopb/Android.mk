LOCAL_PATH := $(call my-dir)

# nanopb_c library
# =======================================================
nanopb_c_src_files := \
	pb_decode.c \
	pb_encode.c

include $(CLEAR_VARS)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE			   := $(qcril_sanitize)
endif

LOCAL_MODULE := libril-protobuf-c-nano-enable_malloc
LOCAL_MODULE_TAGS := optional
LOCAL_C_EXTENSION := .c
LOCAL_SRC_FILES := $(nanopb_c_src_files)
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_VENDOR_MODULE := true

LOCAL_CFLAGS := -DPB_ENABLE_MALLOC $(qcril_cflags)

include $(BUILD_STATIC_LIBRARY)

