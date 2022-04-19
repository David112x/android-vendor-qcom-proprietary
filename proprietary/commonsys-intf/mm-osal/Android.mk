LOCAL_PATH:= $(call my-dir)
############################################################
########## Build the library for system partition ##########
############################################################
include $(CLEAR_VARS)

LOCAL_CFLAGS := \
    -D_ANDROID_

LOCAL_SRC_FILES:=       \
     ./src/MMMalloc.c \
     ./src/MMCriticalSection.c \
     ./src/MMDebug.c \
     ./src/MMTimer.c \
     ./src/MMThread.c \
     ./src/MMSignal.c \
     ./src/MMFile.c  \
     ./src/AEEstd.c

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/./inc \
    $(LOCAL_PATH)/../common/inc \
    $(LOCAL_PATH)/../../common/inc

LOCAL_SHARED_LIBRARIES := \
    libutils  \
    libcutils \
    liblog

LOCAL_PRELINK_MODULE:= false

LOCAL_MODULE:= libmmosal
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
############################################################
########## Build the library for vendor partition ##########
############################################################
include $(CLEAR_VARS)

LOCAL_CFLAGS := \
    -D_ANDROID_ \
    -D_VENDOR_OSAL_

LOCAL_SRC_FILES:=       \
     ./src/MMMalloc.c \
     ./src/MMCriticalSection.c \
     ./src/MMDebug.c \
     ./src/MMTimer.c \
     ./src/MMThread.c \
     ./src/MMSignal.c \
     ./src/MMFile.c  \
     ./src/AEEstd.c

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/./inc \
    $(LOCAL_PATH)/../common/inc \
    $(LOCAL_PATH)/../../common/inc

LOCAL_SHARED_LIBRARIES := \
    libutils  \
    libcutils \
    liblog

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE:= libmmosal_proprietary
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)


############################################################
########## Build the library for header files ##########
############################################################
include $(CLEAR_VARS)
LOCAL_MODULE := libmmosal_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/./inc
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libmmosal_proprietary_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/./inc
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)
