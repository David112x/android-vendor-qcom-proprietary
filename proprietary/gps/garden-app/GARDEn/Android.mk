LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libgarden_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/garden_framework \
    $(LOCAL_PATH)/locationapi_api \
    $(LOCAL_PATH)/android

include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libgarden_case_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/gnss_case \
    $(LOCAL_PATH)/gnss_multiclient_case \
    $(LOCAL_PATH)/flp_case \
    $(LOCAL_PATH)/geofence_case

include $(BUILD_HEADER_LIBRARY)


# Build libgarden
include $(CLEAR_VARS)

LOCAL_MODULE := libgarden
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES += \
    garden_framework/GardenFramework.cpp \
    garden_framework/GardenUtil.cpp \
    garden_framework/GnssCbBase.cpp \
    locationapi_api/GnssLocationAPI.cpp \
    locationapi_api/GnssLocationAPIClient.cpp \
    locationapi_api/FlpLocationAPIClient.cpp \
    locationapi_api/GeofenceLocationAPIClient.cpp \
    locationapi_api/GnssMultiClientLocationAPIClient.cpp \
    android/HidlAPI.cpp

LOCAL_CFLAGS += \
    -DDEBUG \
    -D_ANDROID_ \
    -fno-short-enums \
    -fexceptions
SUBDIRS += \
    sample_case

LOCAL_C_INCLUDES:= $(TARGET_OUT_HEADERS)/common/inc

LOCAL_HEADER_LIBRARIES := \
    libgarden_headers \
    libgps.utils_headers \
    libloc_core_headers \
    libloc_pla_headers \
    liblbs_core_headers \
    libgarden_case_headers \
    liblocation_api_headers\
    libdataitems_headers \
    izat_remote_api_prop_headers\
    izat_remote_api_headers

LOCAL_SHARED_LIBRARIES := \
    libdl \
    libgps.utils \
    libloc_core \
    liblbs_core \
    liblocation_api \
    liblog \
    libdataitems \
    libutils \
    android.hardware.gnss@1.0 \
    android.hardware.gnss@1.1 \
    android.hardware.gnss@2.0 \
    android.hardware.gnss.visibility_control@1.0 \
    vendor.qti.gnss@3.0 \
    libhidlbase \
    libhidltransport

ifneq ($(BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE),)
LOCAL_CFLAGS += -DLOCHIDL
LOCAL_SHARED_LIBRARIES += \
    vendor.qti.gnss@1.0 \
    vendor.qti.gnss@1.1 \
    vendor.qti.gnss@1.2 \
    vendor.qti.gnss@2.1 \
    vendor.qti.gnss@3.0 \

endif # BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

# Build libgarden_haltests_e2e
include $(CLEAR_VARS)

LOCAL_MODULE := libgarden_haltests_e2e
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES += \
    gnss_case/GnssCase.cpp \
    gnss_case/GnssCb.cpp \
    gnss_multiclient_case/GnssMultiClientCase.cpp \
    gnss_multiclient_case/GnssMultiClientCb.cpp \
    gnss_multiclient_case/GnssMultiClientHidlIGnss.cpp \
    gnss_multiclient_case/GnssMultiClientHidlIGnssCb.cpp \
    gnss_multiclient_case/GnssMultiClientHidlILocHidlGnss.cpp \
    gnss_multiclient_case/GnssMultiClientHidlILocHidlGnssCb.cpp \
    gnss_multiclient_case/GnssMultiClientFileTest.cpp \
    gnss_multiclient_case/GnssMultiClientCaseUtils.cpp \
    geofence_case/GeofenceCb.cpp \
    geofence_case/GeofenceCase.cpp \
    flp_case/FlpCase.cpp \
    flp_case/FlpCb.cpp

LOCAL_CFLAGS += \
    -DDEBUG \
    -D_ANDROID_ \
    -fno-short-enums \
    -fexceptions

LOCAL_C_INCLUDES:= $(TARGET_OUT_HEADERS)/common/inc

LOCAL_HEADER_LIBRARIES := \
    libgarden_headers \
    libgarden_case_headers \
    libgps.utils_headers \
    libloc_core_headers \
    libloc_pla_headers \
    liblbs_core_headers \
    liblocation_api_headers\
    libdataitems_headers \
    izat_remote_api_prop_headers\
    izat_remote_api_headers

LOCAL_SHARED_LIBRARIES := \
    libgarden \
    libdl \
    libgps.utils \
    libloc_core \
    liblocation_api \
    liblog \
    libutils \
    android.hardware.gnss@1.0 \
    android.hardware.gnss@1.1 \
    android.hardware.gnss@2.0 \
    android.hardware.gnss.visibility_control@1.0 \
    vendor.qti.gnss@3.0 \
    libhidlbase \
    libhidltransport

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

# Build garden_app
include $(CLEAR_VARS)
LOCAL_MODULE := garden_app
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += \
    $(GNSS_CFLAGS)
LOCAL_CFLAGS += \
    -DDEBUG \
    -D_ANDROID_ \
    -fno-short-enums \
    -fexceptions

LOCAL_SRC_FILES := \
    main.cpp \

LOCAL_C_INCLUDES:= $(TARGET_OUT_HEADERS)/common/inc

LOCAL_HEADER_LIBRARIES := \
    libutils_headers \
    libgarden_headers \
    libgarden_case_headers \
    libloc_core_headers \
    libgps.utils_headers \
    libloc_pla_headers \
    liblbs_core_headers \
    liblocation_api_headers\
    libdataitems_headers \
    izat_remote_api_prop_headers\
    izat_remote_api_headers

LOCAL_SHARED_LIBRARIES := \
    libgarden \
    libgarden_haltests_e2e \
    libdl \
    libgps.utils \
    liblocation_api \
    libloc_core \
    liblog \
    libutils \

include $(BUILD_EXECUTABLE)

include $(addsuffix /Android.mk, $(addprefix $(LOCAL_PATH)/, $(SUBDIRS)))
