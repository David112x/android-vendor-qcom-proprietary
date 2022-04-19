LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := data/inc
LOCAL_COPY_HEADERS      := inc/portbridge_sockif.h

LOCAL_SRC_FILES := port_bridge.c

LOCAL_MODULE:= port-bridge
LOCAL_SANITIZE:=integer_overflow
# Local rc file for port-bridge
LOCAL_INIT_RC := port-bridge.rc
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libdsutils

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/data/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_CFLAGS += -DFEATURE_PB_LINUX_ANDROID
#LOCAL_CFLAGS += -DPORT_BRIDGE_TEST

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
