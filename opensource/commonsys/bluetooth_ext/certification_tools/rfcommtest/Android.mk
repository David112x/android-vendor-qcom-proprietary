LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= rfcommtest.cpp

LOCAL_C_INCLUDES += . \
    vendor/qcom/opensource/commonsys/system/bt/stack/include \
    system/bt/include \
    system/bt/types \
    vendor/qcom/opensource/commonsys/system/bt/internal_include \
    vendor/qcom/opensource/commonsys/system/bt/stack/l2cap \
    vendor/qcom/opensource/commonsys/system/bt/utils/include \
    vendor/qcom/opensource/commonsys/system/bt/ \
    vendor/qcom/opensource/commonsys/system/bt/btif/include \
    vendor/qcom/opensource/commonsys/bluetooth_ext/system_bt_ext/include \
    vendor/qcom/opensource/commonsys/bluetooth_ext/vhal/include \
    external/libchrome \

LOCAL_CFLAGS += -DHAS_NO_BDROID_BUILDCFG
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE:= rfc

LOCAL_SHARED_LIBRARIES += libcutils   \
                          libchrome   \
                          libutils    \

LOCAL_STATIC_LIBRARIES += libbluetooth-types

include $(BUILD_EXECUTABLE)

