LOCAL_PATH            := $(call my-dir)
SECUREMSM_PATH        := vendor/qcom/proprietary/securemsm-noship
SECUREMSM_SHIP_PATH   := vendor/qcom/proprietary/securemsm
SSE_PATH              := $(SECUREMSM_PATH)/sse

COMMON_INCLUDES     := $(SSE_PATH)/log/inc \
                       $(LOCAL_PATH)/include \
                       $(SSE_PATH)/common/include \
                       $(SECUREMSM_PATH)/tzservices/secure_ui \
                       frameworks/native/include/utils/ \
                       $(TARGET_OUT_HEADERS)/common/inc \
                       $(TOP)/frameworks/native/include/gui\
                       $(TOP)/frameworks/native/include/ui \
                       frameworks/native/include/binder/

MY_CFLAGS  := -g -fdiagnostics-show-option -Wno-format -Wno-missing-braces \
              -Wno-missing-field-initializers -Wno-unused-parameter

SSE_LOG_ENABLE_CFLAGS := -DSSE_LOG_GLOBAL -DSSE_LOG_FILE

include $(CLEAR_VARS)
SSE_LOG_CFLAGS := -DLOG_TAG='"SUI_service_jni"'


LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE           := libsecureuisvc_jni
LOCAL_PRODUCT_MODULE   := true
LOCAL_PRELINK_MODULE    := false
LOCAL_CPP_EXTENSION     := .cpp

ifeq ($(TARGET_SUPPORTS_WEARABLES),true)
LOCAL_C_INCLUDES += $(TOP)/$(BOARD_DISPLAY_HAL)/libgralloc
else
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/display/libgralloc1
endif

# includes
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include \
                    $(COMMON_INCLUDES)

LOCAL_HEADER_LIBRARIES  := display_headers

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES         := svc.cpp wfd.cpp \
                           SecureDisplayAPI.cpp

LOCAL_SHARED_LIBRARIES := liblog \
                          libsecureui_svcsock_system \
                          libcutils \
                          libbinder \
                          libutils \
                          libhidlbase \
                          libhidltransport \
                          libhwbinder \
                          libgui \
                          libui \
                          vendor.display.config@1.0

LOCAL_CFLAGS += $(MY_CFLAGS)
LOCAL_CFLAGS += $(SSE_LOG_CFLAGS)
LOCAL_CFLAGS += $(SSE_LOG_ENABLE_CFLAGS)
LOCAL_MODULE_OWNER := qti

include $(BUILD_SHARED_LIBRARY)
