LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    IPerfManager.cpp \
    PerfService.cpp \
    main_PerfService.cpp

LOCAL_MODULE:= perfservice

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libbinder

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_INIT_RC := perfservice.rc
include $(BUILD_EXECUTABLE)
