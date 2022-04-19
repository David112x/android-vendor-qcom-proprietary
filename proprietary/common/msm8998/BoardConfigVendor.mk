## BoardConfigVendor.mk
## Qualcomm Technologies proprietary product specific compile-time definitions.
#
#Enable/Disable Camera Daemon
CAMERA_DAEMON_NOT_PRESENT := true
USE_CAMERA_STUB := false
DEVICE_PACKAGE_OVERLAYS += vendor/qcom/proprietary/common/msm8998/overlay

BOARD_USES_LIBC_WRAPPER := true
BOARD_USES_DPM := true
BOARD_USES_QCNE := true
TARGET_USES_IMS := true
TARGET_FASTCV_DISABLED := true
TARGET_SCVE_DISABLED := true
#Enable/Disable uibeautification
TARGET_TS_MAKEUP := false

USESECIMAGETOOL := true
## Gensecimage generation of signed apps bootloader
QTI_GENSECIMAGE_MSM_IDS := msm8998
## Use signed image as default
QTI_GENSECIMAGE_SIGNED_DEFAULT := msm8998
#
##SECIMAGE tool feature flags
USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN := 1
USES_SEC_POLICY_INTEGRITY_CHECK := 1

USE_SOC_HW_VERSION := true
SOC_HW_VERSION := 0x30020000
SOC_VERS := 0x3005

OVERRIDE_RS_DRIVER := libRSDriver_adreno.so
BOARD_USES_ADRENO := true
HAVE_ADRENO_SOURCE := true
HAVE_ADRENO_SC_SOURCE := true
HAVE_ADRENO_FIRMWARE := true

TARGET_USES_ION := true

## wlan related flags
BOARD_HAS_QCOM_WLAN := true
BOARD_HAS_QCOM_WIGIG := false
TARGET_USES_ICNSS_QMI := true
TARGET_USES_SUBNET_DETECTION := true
BOARD_HAS_ATH_WLAN_AR6320 := true
BOARD_WLAN_DEVICE := qcwcn
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER := NL80211
WIFI_DRIVER_BUILT := qca_cld3
WIFI_DRIVER_DEFAULT := qca_cld3
WIFI_DRIVER_INSTALL_TO_KERNEL_OUT := true
WIFI_FST_DRIVER_MODULE_PATH := "/system/lib/modules/wil6210.ko"
WIFI_FST_DRIVER_MODULE_NAME := "wil6210"
WIFI_FST_DRIVER_MODULE_ARG := ""
WIFI1_DRIVER_MODULE_ARG := ""
WPA_SUPPLICANT_VERSION := VER_0_8_X
HOSTAPD_VERSION := VER_0_8_X
CONFIG_ACS := true
CONFIG_IEEE80211AC := true
BOARD_HAS_CFG80211_KERNEL3_4 := true
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := default
BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET := false

BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_QCOM_FM := true
BOARD_ANT_WIRELESS_DEVICE := "qualcomm-hidl"
BOARD_HAS_QCA_BT_SOC := "cherokee"
## Add FM chip type
BOARD_HAS_QCA_FM_SOC := "cherokee"
QCOM_BT_USE_SIBS := false
WCNSS_FILTER_USES_SIBS := true

ifeq ($(BOARD_HAVE_BLUETOOTH), true)
  BOARD_HAVE_BLUETOOTH_BLUEZ := false
  ifneq ($(BOARD_HAVE_BLUETOOTH_BLUEZ), true)
    BOARD_HAVE_BLUETOOTH_QCOM := true
    QCOM_BT_USE_SMD_TTY := true
    BOARD_USES_WIPOWER := false
    TARGET_HAS_SPLIT_A2DP_FEATURE := true
  endif # BOARD_HAVE_BLUETOOTH_BLUEZ
endif # BOARD_HAVE_BLUETOOTH

# TODO: Bring-up exception - all modules must be fixed by corresponding teams
INTERNAL_LOCAL_CLANG_EXCEPTION_PROJECTS += \
  $(abspath $(TOPDIR)vendor/qcom/proprietary/gles/adreno200) \
  vendor/qcom/proprietary/gles/adreno200 \
  vendor/qcom/proprietary/fastcv-noship \
  vendor/qcom/proprietary/gps-noship \
  vendor/qcom/proprietary/kernel-tests \
  vendor/qcom/proprietary/mare-noship \
  vendor/qcom/proprietary/mm-camera-lib \
  vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/modules/imglib \
  vendor/qcom/proprietary/mm-camera/mm-camera2/tests \
  vendor/qcom/proprietary/mm-mux

# TODO: Bring-up exception - all modules must be fixed by corresponding teams
TARGET_CINCLUDES_EXCEPTION_PROJECTS := \
  $(abspath $(TOPDIR)vendor/qcom/proprietary/gles/adreno200) \
  vendor/qcom/proprietary/gles/adreno200 \
  vendor/qcom/proprietary/gps-noship-external \
  vendor/qcom/proprietary/gps-noship \
  disregard/filesystems/mtd-utils

WIFI_DRIVER_STATE_CTRL_PARAM := "/dev/wlan"
WIFI_DRIVER_STATE_ON := "ON"
WIFI_DRIVER_STATE_OFF := "OFF"
