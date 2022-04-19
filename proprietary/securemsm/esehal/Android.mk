ifneq ($(ENABLE_HYP),true)
ifeq ($(strip $(TARGET_USES_GPQESE)),true)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SMSM_PATH           := vendor/qcom/proprietary/securemsm

COMMON_INCLUDES     :=  $(SMSM_PATH)/GPTEE/inc \
                        $(TARGET_OUT_HEADERS)/common/inc \
                        $(SMSM_PATH)/esepowermanager \
                        $(LOCAL_PATH)/ese-clients/inc \
                        external/libcxx/include

ifeq ($(strip $(TARGET_USES_NQ_NFC)),true)
LOCAL_HEADER_LIBRARIES += libese_client_headers
endif

LOCAL_MODULE := android.hardware.secure_element@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := \
    gpqese/gpqese-be.cpp \
    gpqese/gpqese-apdu.cpp \
    gpqese/gpqese-utils.cpp \

LOCAL_C_INCLUDES := $(COMMON_INCLUDES)

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libutils \
    liblog \
    libhardware \
    libbase \
    libcutils \
    android.hardware.secure_element@1.0 \
    libGPTEE_vendor \
    vendor.qti.esepowermanager@1.0 \

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := vendor.qti.secure_element@1.0-service
LOCAL_INIT_RC := vendor.qti.secure_element@1.0-service.rc
LOCAL_C_INCLUDES := $(SMSM_PATH)/GPTEE/inc \
                    $(SMSM_PATH)/esepowermanager \
                    $(LOCAL_PATH)/ese-clients/inc \

LOCAL_SRC_FILES := \
    service.cpp \
    interface.cpp \
    SecureElement.cpp \
    SecureElementHalCallback.cpp

ifeq ($(strip $(TARGET_USES_NQ_NFC)),true)
LOCAL_HEADER_LIBRARIES += libese_client_headers
LOCAL_CFLAGS += -DESE_CLIENT_LIB_PRESENT=true
LOCAL_SRC_FILES += \
    ese-clients/src/eSEClient.cpp
else
LOCAL_CFLAGS += -DESE_CLIENT_LIB_PRESENT=false
endif

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware \

ifeq ($(strip $(TARGET_USES_NQ_NFC)),true)
LOCAL_SHARED_LIBRARIES += \
    se_nq_extn_client \
    ls_nq_client \
    jcos_nq_client
endif
LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libhidltransport \
    android.hardware.secure_element@1.0 \
    vendor.qti.esepowermanager@1.0 \
    android.hardware.secure_element@1.0-impl

include $(BUILD_EXECUTABLE)
endif
endif
