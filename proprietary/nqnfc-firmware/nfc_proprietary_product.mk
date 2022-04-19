ifeq ($(call is-board-platform-in-list, msm8998 sdm660 msm8937 msm8953),true)
TARGET_USES_NQ_NFC := true
ifeq ($(TARGET_PRODUCT),msm8937_32go)
TARGET_USES_NQ_NFC := false
endif
endif

#NQ NFC Config files + firmware images
NQ_VENDOR_NFC_PROP := nqnfcinfo
NQ_VENDOR_NFC_PROP += libnfc-mtp_rf1.conf
NQ_VENDOR_NFC_PROP += libnfc-mtp_rf2.conf
NQ_VENDOR_NFC_PROP += libnfc-mtp_default.conf
NQ_VENDOR_NFC_PROP += libnfc-mtp-NQ3XX.conf
NQ_VENDOR_NFC_PROP += libnfc-mtp-NQ4XX.conf
NQ_VENDOR_NFC_PROP += libnfc-qrd_rf1.conf
NQ_VENDOR_NFC_PROP += libnfc-qrd_rf2.conf
NQ_VENDOR_NFC_PROP += libnfc-qrd_default.conf
NQ_VENDOR_NFC_PROP += libnfc-qrd-NQ3XX.conf
NQ_VENDOR_NFC_PROP += libnfc-qrd-NQ4XX.conf
NQ_VENDOR_NFC_PROP += libpn551_fw.so
NQ_VENDOR_NFC_PROP += libpn553_fw.so
NQ_VENDOR_NFC_PROP += libpn557_fw.so

ifeq ($(strip $(TARGET_USES_NQ_NFC)),true)
PRODUCT_PACKAGES += $(NQ_VENDOR_NFC_PROP)
endif
