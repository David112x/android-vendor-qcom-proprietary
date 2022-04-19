#QMI_DATA
# Legacy packages
QMI_DATA := libidl
QMI_DATA += libqcci_legacy
QMI_DATA += libqmi_csvt_srvc
QMI_DATA += libqmi

# Active packages
QMI_DATA += libqmi_client_helper
QMI_DATA += libqmiservices
QMI_DATA += libqmi_client_qmux

#QMUXD
ifeq ($(call is-board-platform-in-list,apq8084 msm8916 msm8939 msm8909 msm8992 msm8994 msm8952 msm8976 msm8226),true)
QMI_DATA += qmi_config.xml
QMI_DATA += qmuxd
QMI_DATA += qmuxd.rc
endif

# QMI_DATA_HY11_HY22_diff
QMI_DATA_HY11_HY22_diff := libqmi_client_qmux

PRODUCT_PACKAGES += $(QMI_DATA)
PRODUCT_PACKAGES += $(QMI_DATA_HY11_HY22_diff)

