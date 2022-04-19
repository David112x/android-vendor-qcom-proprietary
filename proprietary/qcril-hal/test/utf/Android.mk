#ifeq ($(QCRIL_HAL_ENABLE_UTF), true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

QCRIL_HAL_DIR := ../../../qcril-hal
QCRIL_DIR := ../../../qcril
QCRIL_DATA_DIR := ../../../qcril-data-hal
DATA_DIR := ../../../data
QMI_DIR := ../../../qmi
QMI_FWK_DIR := ../../../qmi-framework
QCRIL_QMI_DIR := ../../../qcril-qmi-services
IMS_SHIP_DIR := ../../../ims-ship
COMMON_DIR := ../../../common
QCRIL_QMI_SVC_HEADERS_DIR := ../../../qcril-qmi-services-headers


build_for_android := true

include $(LOCAL_PATH)/hal_master_make
LOCAL_SRC_FILES = $(c_sources) $(cc_sources)
$(info LOCAL_SRC_FILES: $(LOCAL_SRC_FILES))
LOCAL_CPP_EXTENSION := .cc

LOCAL_CFLAGS                := $(FEATURES) -ferror-limit=0 -g  -fno-omit-frame-pointer -O0
LOCAL_CXXFLAGS              := $(FEATURES) -g  -fno-omit-frame-pointer -std=c++17 -O0
LOCAL_CPPFLAGS              := $(FEATURES) -g  -fno-omit-frame-pointer -std=c++17 -O0
LOCAL_LDFLAGS               := -lrt -g  -ldl

LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/time-services
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi-framework/inc
#LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/diag/include
#LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/data/inc
LOCAL_C_INCLUDES            += system/libhidl/base/include
LOCAL_HEADER_LIBRARIES      += qtibus-headers
LOCAL_HEADER_LIBRARIES      += libqcril-qc-hal-cmas-headers

LOCAL_HEADER_LIBRARIES += libcutils_headers

utf_inc := $(patsubst -I%, $(LOCAL_PATH)/%, $(INC))
$(info utf_inc: $(utf_inc))
LOCAL_C_INCLUDES            += $(utf_inc)


LOCAL_SANITIZE_DIAG := address cfi

LOCAL_SANITIZE			   := address cfi
LOCAL_MODULE                := libqcril-hal
LOCAL_MODULE_OWNER          := qti
LOCAL_PROPRIETARY_MODULE    := true
LOCAL_MODULE_TAGS           := optional
LOCAL_SANITIZE              := address

LOCAL_SHARED_LIBRARIES      += libsqlite

include $(BUILD_HOST_STATIC_LIBRARY)

include $(CLEAR_VARS)
build_for_android := true
include $(LOCAL_PATH)/hal_master_make
LOCAL_SRC_FILES := $(cpp_sources)
LOCAL_CPP_EXTENSION := .cpp

LOCAL_CFLAGS				:= $(FEATURES) -ferror-limit=0 -g -fno-omit-frame-pointer
LOCAL_CXXFLAGS				:= $(FEATURES) -g -fno-omit-frame-pointer -std=c++17
LOCAL_CPPFLAGS				:= $(FEATURES) -g -fno-omit-frame-pointer -std=c++17

LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/time-services
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi-framework/inc
#LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/diag/include
#LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/data/inc
LOCAL_C_INCLUDES            += system/libhidl/base/include

LOCAL_HEADER_LIBRARIES      += libcutils_headers
LOCAL_HEADER_LIBRARIES      += qtibus-headers

utf_inc := $(patsubst -I%, $(LOCAL_PATH)/%, $(INC))
$(info utf_inc: $(utf_inc))
LOCAL_C_INCLUDES            += $(utf_inc)

LOCAL_SHARED_LIBRARIES      := libqmiservices
LOCAL_SHARED_LIBRARIES      += libqmi_encdec
LOCAL_SHARED_LIBRARIES      += libsqlite
LOCAL_SHARED_LIBRARIES      += libqmi_common_so
LOCAL_SHARED_LIBRARIES      += libcutils
LOCAL_SHARED_LIBRARIES      += libbase
LOCAL_SHARED_LIBRARIES      += liblog

LOCAL_MODULE				:= libqcril-utf-fwk
LOCAL_MODULE_OWNER			:= qti
LOCAL_PROPRIETARY_MODULE	:= true
LOCAL_MODULE_TAGS			:= optional
LOCAL_SANITIZE				:= address
include $(BUILD_HOST_STATIC_LIBRARY)


include $(CLEAR_VARS)

build_for_android := true

include $(LOCAL_PATH)/hal_master_make
LOCAL_SRC_FILES = $(qcril_hal_cpp_source) $(cpp_test_source)
$(info LOCAL_SRC_FILES: $(LOCAL_SRC_FILES))
LOCAL_CPP_EXTENSION := .cpp

LOCAL_CFLAGS                := $(FEATURES) -ferror-limit=0 -g  -fno-omit-frame-pointer -O0
LOCAL_CXXFLAGS              := $(FEATURES) -g  -fno-omit-frame-pointer -std=c++17 -O0
LOCAL_CPPFLAGS              := $(FEATURES) -g  -fno-omit-frame-pointer -std=c++17 -O0
LOCAL_LDFLAGS               := -lrt -g  -ldl

LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/time-services
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi-framework/inc
#LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/diag/include
#LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)
LOCAL_C_INCLUDES            += $(TARGET_OUT_HEADERS)/data/inc
LOCAL_C_INCLUDES            += system/libhidl/base/include

LOCAL_HEADER_LIBRARIES      += libcutils_headers
LOCAL_HEADER_LIBRARIES      += qtibus-headers
LOCAL_HEADER_LIBRARIES      += libqcril-qc-hal-cmas-headers

utf_inc := $(patsubst -I%, $(LOCAL_PATH)/%, $(INC))
$(info utf_inc: $(utf_inc))
LOCAL_C_INCLUDES            += $(utf_inc)


LOCAL_SANITIZE_DIAG := address cfi

LOCAL_SANITIZE			   := address cfi

LOCAL_MODULE                := qcril_hal_utf_test
LOCAL_MODULE_OWNER          := qti
LOCAL_PROPRIETARY_MODULE    := true
LOCAL_MODULE_TAGS           := optional

LOCAL_WHOLE_STATIC_LIBRARIES += libqcril-utf-fwk
LOCAL_STATIC_LIBRARIES      := libqcril-hal
LOCAL_STATIC_LIBRARIES      += libimsqmiservices
LOCAL_STATIC_LIBRARIES      += libdataqmiservices
LOCAL_STATIC_LIBRARIES      += librilqmiservices
LOCAL_STATIC_LIBRARIES      += qtibus

LOCAL_SHARED_LIBRARIES      := libqmiservices
LOCAL_SHARED_LIBRARIES      += libqmi_encdec
LOCAL_SHARED_LIBRARIES      += libsqlite
LOCAL_SHARED_LIBRARIES      += libqmi_common_so
LOCAL_SHARED_LIBRARIES      += libcutils
LOCAL_SHARED_LIBRARIES      += libbase
LOCAL_SHARED_LIBRARIES      += liblog

LOCAL_SANITIZE              := address

include $(BUILD_HOST_EXECUTABLE)

#endif
