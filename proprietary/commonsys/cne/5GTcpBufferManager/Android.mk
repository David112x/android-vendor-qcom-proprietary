ifneq ($(TARGET_BOARD_TYPE),auto)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= ConnectivityExt
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES := telephony-ext
LOCAL_DEX_PREOPT := nostripping

LOCAL_JAVACFLAGS := -Xlint -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
include $(BUILD_JAVA_LIBRARY)
endif
