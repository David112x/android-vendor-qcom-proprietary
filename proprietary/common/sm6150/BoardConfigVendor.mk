## BoardConfigVendor.mk
## Qualcomm Technologies proprietary product specific compile-time definitions.
#

USE_CAMERA_STUB := false
TARGET_SCVE_DISABLED := true
TARGET_FASTCV_DISABLED := true
TARGET_USES_SSC := true
TARGET_DISABLE_MSMBUSPM := true
TARGET_INIT_COLDBOOT_TIMEOUT := 10000
TARGET_INIT_CONSOLE_TIMEOUT := 10000
##SECIMAGE tool feature flags
USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN := 1
USES_SEC_POLICY_INTEGRITY_CHECK := 1
USE_SOC_HW_VERSION := true
SOC_HW_VERSION := 0x60070000
SOC_VERS := 0x6007
BOARD_USES_LIBC_WRAPPER := false
TARGET_USES_IMS := true
BOARD_USES_QCNE := true
BOARD_USES_DPM := true
#Flags for generating signed images
USESECIMAGETOOL := true
QTI_GENSECIMAGE_MSM_IDS := $(MSMSTEPPE)
QTI_GENSECIMAGE_SIGNED_DEFAULT := $(MSMSTEPPE)
#USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN := 1
#USES_SEC_POLICY_INTEGRITY_CHECK := 1
OVERRIDE_RS_DRIVER := libRSDriver_adreno.so
BOARD_USES_ADRENO := true
HAVE_ADRENO_SOURCE := true
HAVE_ADRENO_SC_SOURCE := true
HAVE_ADRENO_FIRMWARE := true
TARGET_USES_ION := true
TARGET_COMPILE_WITH_MSM_KERNEL := false

## wlan flags
BOARD_HAS_QCOM_WLAN := true
BOARD_HAS_QCOM_WIGIG := true
TARGET_USES_ICNSS_QMI := true
BOARD_HAS_ATH_WLAN_AR6320 := true
BOARD_WLAN_DEVICE := qcwcn
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER := NL80211
WIFI_DRIVER_BUILT := qca_cld3
WIFI_DRIVER_DEFAULT := qca_cld3
WIFI_DRIVER_INSTALL_TO_KERNEL_OUT := true
WPA_SUPPLICANT_VERSION := VER_0_8_X
HOSTAPD_VERSION := VER_0_8_X
CONFIG_ACS := true
CONFIG_IEEE80211AC := true
BOARD_HAS_CFG80211_KERNEL3_4 := true
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)

## wigig flags
WIGIG_DRIVER_MODULE_PATH := "/system/vendor/lib/modules/wil6210.ko"
WIGIG_DRIVER_MODULE_NAME := "wil6210"
WIGIG_DRIVER_MODULE_ARG := "alt_ifname=1"

## Bluetooth/ANT/FM/Wipower
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_QCOM := true
BOARD_HAS_QCA_FM_SOC := "cherokee"
BOARD_ANT_WIRELESS_DEVICE := "qualcomm-hidl"
BOARD_USES_WIPOWER := false
TARGET_HAS_SPLIT_A2DP_FEATURE := true
#Enable uibeautification
#TARGET_TS_MAKEUP := true

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

# Pre-sil Flags to fix the compliation issues with minimal configuration.
# ------Start----------------
TARGET_DISABLE_CAMX-LIB :=true
TARGET_DISABLE_GNSS := true
TARGET_DISABLE_QTEE := true
# ----- end -----------------
