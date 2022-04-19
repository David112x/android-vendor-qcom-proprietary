LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        diag_lsm_jni.cpp \
	diag_dci_jni.cpp

LOCAL_C_INCLUDES += \
        $(JNI_H_INCLUDE) \
        $(TOP)/libnativehelper/include/nativehelper \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/diag/include \
	$(LOCAL_PATH)/include \
        $(TOP)/libnativehelper/include/nativehelper

LOCAL_STATIC_LIBRARIES := \
	libdiag \
	libtime_genoff

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE:= libdiagjni

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
        diag_lsm_jni.cpp \
	diag_dci_jni.cpp

LOCAL_C_INCLUDES += \
        $(JNI_H_INCLUDE) \
        $(TOP)/libnativehelper/include/nativehelper \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/diag/include \
	$(LOCAL_PATH)/include \
LOCAL_CFLAGS += -DANDROID
LOCAL_STATIC_LIBRARIES := \
	libdiag \
	libtime_genoff \

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        liblog

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE:= libdiagjni

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_STATIC_LIBRARY)
