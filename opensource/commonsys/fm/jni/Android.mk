ifeq ($(BOARD_HAVE_QCOM_FM),true)
ifneq (,$(filter $(QCOM_BOARD_PLATFORMS),$(TARGET_BOARD_PLATFORM)))
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
android_hardware_fm.cpp \
ConfFileParser.cpp \
ConfigFmThs.cpp \
FmIoctlsInterface.cpp \
FmPerformanceParams.cpp

LOCAL_LDLIBS += -ldl
LOCAL_SHARED_LIBRARIES := \
        libandroid_runtime \
        libnativehelper \
        liblog \
        libcutils \
        libbtconfigstore

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                   $(TOP)/libnativehelper/include/nativehelper \
                   $(TOP)/vendor/qcom/opensource/commonsys-intf/bluetooth/include \
                   $(TOP)/vendor/qcom/opensource/commonsys/bluetooth_ext/system_bt_ext/btconfigstore \
                   vendor/qcom/opensource/commonsys/fm/helium

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE := libqcomfm_jni
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


endif # is-vendor-board-platform
endif # BOARD_HAVE_QCOM_FM
