ifneq ($(TARGET_NO_TELEPHONY), true)

QCRIL_DIR := $(call my-dir)

include $(QCRIL_DIR)/qcrilhook_oem/Android.mk
include $(QCRIL_DIR)/qcril_database/Android.mk
include $(QCRIL_DIR)/qcril_qmi/qcril_qmi.mk
include $(QCRIL_DIR)/settingsd/Android.mk

endif
