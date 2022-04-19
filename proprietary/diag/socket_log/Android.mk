################################################################################
# @file pkgs/stringl/Android.mk
# @brief Makefile for building the string library on Android.
################################################################################

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SRC_FILES:= \
        diag_socket_log.c

commonSharedLibraries :=libdiag

LOCAL_MODULE := diag_socket_log
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)

