LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=     \
    src/btsnoop_dump.c    \
    src/bt_logger.c   \
    src/bt_log_buffer.c

ifeq ($(TARGET_BUILD_VARIANT),userdebug)
LOCAL_CFLAGS += -DLOGGER_USERDEBUG
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/incl

LOCAL_MODULE:= bt_logger

LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog

include $(BUILD_EXECUTABLE)
