LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_HEADER_LIBRARIES := libcutils_headers

LOCAL_SRC_FILES := \
	diag_mdlog.c

commonSharedLibraries :=libdiag liblog

LOCAL_MODULE := diag_mdlog
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)

ifdef TARGET_FS_CONFIG_GEN
LOCAL_HEADER_LIBRARIES += oemaids_headers
LOCAL_CFLAGS += -DTARGET_FS_CONFIG_GEN
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
