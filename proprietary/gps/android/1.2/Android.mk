ifneq ($(BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE),)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.gnss@1.2-impl
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := \
    LocHidlAGnss.cpp \
    LocHidlDebugReportService.cpp \
    LocHidlFlpService.cpp \
    LocHidlGeofenceService.cpp \
    LocHidlIzatProvider.cpp \
    LocHidlGnssNi.cpp \
    LocHidlIzatOsNpGlue.cpp \
    LocHidlRilInfoMonitor.cpp \
    LocHidlIzatSubscription.cpp \
    LocHidlGnss.cpp \
    LocHidlXT.cpp \
    LocHidlWiFiDBReceiver.cpp \
    LocHidlWWANDBReceiver.cpp \

LOCAL_SRC_FILES += \
    location_api/LocHidlNiClient.cpp \
    location_api/LocHidlFlpClient.cpp \
    location_api/LocHidlGeofenceClient.cpp \
    location_api/LocHidlUtils.cpp \

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/location_api \
    $(TARGET_OUT_HEADERS)/common/inc \
    $(TARGET_OUT_HEADERS)/libxtadapter \
    $(TARGET_OUT_HEADERS)/libizat_core \
    $(TARGET_OUT_HEADERS)/liblbs_core \

LOCAL_HEADER_LIBRARIES := \
    izat_remote_api_headers \
    libgps.utils_headers \
    libloc_core_headers \
    libloc_pla_headers \
    liblocation_api_headers \
    liblocationservice_glue_headers \
    liblocationservice_headers \
    libdataitems_headers \
    izat_remote_api_prop_headers \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libutils \
    android.hardware.gnss@1.0 \
    vendor.qti.gnss@1.0 \
    vendor.qti.gnss@1.1 \
    vendor.qti.gnss@1.2 \

LOCAL_SHARED_LIBRARIES += \
    libloc_core \
    libgps.utils \
    libdl \
    liblocation_api \
    liblocationservice_glue \
    liblbs_core \
    libdataitems \
    liblocationservice \
    libxtadapter \
    libizat_core \
    libizat_client_api \

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.gnss@1.2-service
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_VINTF_FRAGMENTS := vendor.qti.gnss@1.2-service.xml
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := \
    service/service.cpp \

LOCAL_HEADER_LIBRARIES := \
    libgps.utils_headers

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libgps.utils \
    libqti_vndfwk_detect \

LOCAL_SHARED_LIBRARIES += \
    libhwbinder \
    libhidlbase \
    libhidltransport \
    android.hardware.gnss@1.0 \
    android.hardware.gnss@1.1 \
    vendor.qti.gnss@1.0 \
    vendor.qti.gnss@1.1 \
    vendor.qti.gnss@1.2 \

LOCAL_CFLAGS += $(GNSS_CFLAGS)

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -DARCH_ARM_32
endif

include $(BUILD_SHARED_LIBRARY)

endif # BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE
