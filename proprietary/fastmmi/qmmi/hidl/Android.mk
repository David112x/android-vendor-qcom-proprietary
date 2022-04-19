LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.factory@1.0-service
LOCAL_INIT_RC := vendor.qti.hardware.factory@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_SRC_FILES := \
    service.cpp \
    Factory.cpp \
    socket.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware \
    libhidlbase \
    libbinder \
    libhidltransport \
    vendor.qti.hardware.factory@1.0 \
    vendor.qti.hardware.factory@1.1 \

include $(BUILD_EXECUTABLE)
