LOCAL_PATH            := $(call my-dir)

COMMON_INCLUDES     := $(TARGET_OUT_HEADERS)/common/inc \
                       $(TOP)/frameworks/native/include/gui\
                       $(TOP)/frameworks/native/include/ui \
                       $(TOP)/frameworks/native/include/binder \
                       $(TOP)/frameworks/native/libs/nativewindow/include \
                       $(TOP)/frameworks/native/libs/arect/include \
                       $(TOP)/frameworks/native/libs/nativebase/include \
                       $(TOP)/frameworks/av/media/libstagefright/bqhelper/include

MY_CFLAGS  := -g -fdiagnostics-show-option -Wno-format -Wno-missing-braces \
              -Wno-missing-field-initializers -Wno-unused-parameter

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS      := optional
LOCAL_MODULE           := libsystemhelper_jni
LOCAL_PRODUCT_MODULE   := true
LOCAL_PRELINK_MODULE    := false
LOCAL_CPP_EXTENSION     := .cpp

# includes
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include \
                    $(COMMON_INCLUDES)

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES         := svc.cpp \
                           SystemEvent.cpp \
                           SystemResource.cpp

LOCAL_SHARED_LIBRARIES := liblog \
                          libcutils \
                          libbinder \
                          libutils \
                          libhidlbase \
                          libhidltransport \
                          libhwbinder \
                          libgui \
                          libui \
                          vendor.qti.hardware.systemhelper@1.0 \
                          android.hardware.graphics.bufferqueue@1.0 \
                          libstagefright_bufferqueue_helper

LOCAL_CFLAGS += $(MY_CFLAGS)
LOCAL_MODULE_OWNER := qti

include $(BUILD_SHARED_LIBRARY)

