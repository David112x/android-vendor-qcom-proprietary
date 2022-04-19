LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=     \
    btsnoop_dump.c

LOCAL_C_INCLUDES :=

LOCAL_MODULE:= btsnoop

LOCAL_SHARED_LIBRARIES += libcutils

include $(BUILD_EXECUTABLE)
