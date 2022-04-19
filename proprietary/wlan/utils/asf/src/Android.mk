LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CLANG := true
LOCAL_MODULE := libAniAsf
LOCAL_MODULE_TAGS := optional
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED), true)
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_SRC_FILES +=  \
			aniAsfTimer.c \
			aniAsfIpc.c \
			aniAsfEvent.c \
			aniAsfLog.c \
			aniAsfProcessUtils.c \
			aniAsfUtils.c \
			aniAsfMisc.c  \
			aniAsfHashTable.c \
			aniAsfDict.c \
			aniAsfPacket.c \
		  aniAsfPortMap.c

LOCAL_CFLAGS += \
	-fno-short-enums \
	-Wall \
	-Werror

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../inc

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_SANITIZE := signed-integer-overflow unsigned-integer-overflow

include $(BUILD_STATIC_LIBRARY)
