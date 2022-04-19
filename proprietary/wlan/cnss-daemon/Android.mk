LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := cnss-daemon
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
LOCAL_PROPRIETARY_MODULE := true
endif
LOCAL_MODULE_TAGS := optional

LOCAL_CLANG := true
LOCAL_SANITIZE := integer_overflow
LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi-framework/inc \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(TARGET_OUT_HEADERS)/qmi-framework/qcci/inc \
	$(TARGET_OUT_HEADERS)/external/libnl/include \
	$(TARGET_OUT_HEADERS)/cld80211-lib

LOCAL_CFLAGS += -DICNSS_QMI

LOCAL_C_INCLUDES += \
	$(TARGET_OUT_HEADERS)/libmdmdetect/inc \
	$(TARGET_OUT_HEADERS)/libperipheralclient/inc

ifneq ($(strip $(TARGET_USES_NO_SUBNET_DETECTION)),true)
LOCAL_CFLAGS += -DENABLE_SUBNET_DETECTION
endif

LOCAL_LD_FLAGS := -Wl,--hash-style=sysv

LOCAL_SRC_FILES := \
	main.c \
	nl_loop.c \
	debug.c \
	cnss_qmi_client.c \
	wireless_lan_proxy_service_v01.c \
	cnss_dp.c

LOCAL_SRC_FILES += wlfw_qmi_client.c \
		wlan_firmware_service_v01.c

LOCAL_SRC_FILES += \
	cnss_user.c \
	cnss_genl.c

ifneq ($(strip $(TARGET_USES_NO_SUBNET_DETECTION)),true)
LOCAL_SRC_FILES += cnss_gw_update.c
endif

ifneq ($(strip $(TARGET_USES_NO_NETLINK_COMMON)),true)
LOCAL_CFLAGS += -DENABLE_NETLINK_COMMON
LOCAL_SRC_FILES += cnss_netlink_common.c

ifneq ($(strip $(TARGET_USES_NO_INTEROP_ISSUES_AP_DETECTION)),true)
LOCAL_CFLAGS += -DENABLE_INTEROP_ISSUES_AP_DETECTION
LOCAL_SRC_FILES += cnss_interop_issues_ap.c
endif
endif

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \
	libqmi_cci \
	libqmi_common_so \
	libnl \
	libcld80211

LOCAL_HEADER_LIBRARIES += libril-qc-qmi-services-headers

LOCAL_SHARED_LIBRARIES += \
	libperipheral_client \
	libmdmdetect \
	libqmiservices

LOCAL_CFLAGS += \
	-DCONFIG_DEBUG \
	-DCONFIG_DEBUG_LOGCAT \
	-DCONFIG_CNSS_DP \
	-DCONFIG_CNSS_GW_UPDATE \
	-DCONFIG_CNSS_LPASS_SCAN \
	-DCONFIG_WLAN_MSG_SVC \
	-DCONFIG_READ_NV_MAC_ADDR

LOCAL_CFLAGS += -DCONFIG_CNSS_USER

ifeq ($(strip $(TARGET_CAL_DATA_CLEAR)),true)
LOCAL_CFLAGS += -DCAL_DATA_REMOVE
endif

include $(BUILD_EXECUTABLE)

##################################################

include $(CLEAR_VARS)
LOCAL_MODULE := cnss_cli
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := cnss_cli.c
include $(BUILD_EXECUTABLE)
