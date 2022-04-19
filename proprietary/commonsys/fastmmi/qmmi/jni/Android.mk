LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    MmiDiagJNIInterface.cpp

LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
    $(TARGET_OUT_HEADERS)/common/inc \
    $(TARGET_OUT_HEADERS)/diag/include \
    $(TOP)/libnativehelper/include/nativehelper

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libdiag_system \
    liblog

LOCAL_MODULE:= libmmi_jni
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
