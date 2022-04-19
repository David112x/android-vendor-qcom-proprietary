#ifeq ($(BOARD_HAVE_BLUETOOTH),true)


LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -Wno-unused-variable
LOCAL_CFLAGS += -Wno-sometimes-uninitialized
LOCAL_CFLAGS += -Wno-format

LOCAL_C_INCLUDES := system/bt/hci/include

LOCAL_SRC_FILES:= \
              btconfig.c

LOCAL_MODULE := btconfig

LOCAL_SHARED_LIBRARIES += libcutils   \
                          libutils    \
                          libdl

LOCAL_SANITIZE := integer_overflow

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
include $(BUILD_EXECUTABLE)
#endif
