ifeq ($(strip $(ENABLE_SECCAM)),true)
#No seccam lib/bin in /vendor yet (except for hal service)
ifeq ($(strip $(ENABLE_SECCAM_QTI_SERVICE)),true)
SECUREMSM_SECCAM := vendor.qti.hardware.seccam@1.0.vendor
SECUREMSM_SECCAM += vendor.qti.hardware.seccam@1.0_vendor
SECUREMSM_SECCAM += vendor.qti.hardware.seccam@1.0-service-qti
SECUREMSM_SECCAM += vendor.qti.hardware.seccam@1.0-service-qti.rc
endif
endif

ifeq ($(strip $(ENABLE_SECURE_UI)),true)
SECUREMSM_SECDISP := libSecureUILib
SECUREMSM_SECDISP += libsi
SECUREMSM_SECDISP += libStDrvInt
SECUREMSM_SECDISP += secure_ui_sample_client
SECUREMSM_SECDISP += libsecureui_svcsock
SECUREMSM_SECDISP += libsecureui
SECUREMSM_SECDISP += vendor.qti.hardware.tui_comm@1.0.vendor
SECUREMSM_SECDISP += vendor.qti.hardware.tui_comm@1.0_vendor
SECUREMSM_SECDISP += vendor.qti.hardware.tui_comm@1.0-impl-qti
SECUREMSM_SECDISP += vendor.qti.hardware.tui_comm@1.0-service-qti
SECUREMSM_SECDISP += vendor.qti.hardware.tui_comm@1.0-service-qti.rc
SECUREMSM_SECDISP += vendor.qti.hardware.qdutils_disp@1.0.vendor
SECUREMSM_SECDISP += vendor.qti.hardware.qdutils_disp@1.0_vendor
SECUREMSM_SECDISP += vendor.qti.hardware.qdutils_disp@1.0-impl-qti
SECUREMSM_SECDISP += vendor.qti.hardware.qdutils_disp@1.0-service-qti
SECUREMSM_SECDISP += vendor.qti.hardware.qdutils_disp@1.0-service-qti.rc
endif

ifeq ($(strip $(ENABLE_TRUSTED_UI_2_0)),true)
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0.vendor
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0_vendor
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0_32
SECUREMSM_SECDISP += libTrustedInput
SECUREMSM_SECDISP += TrustedUISampleTest
SECUREMSM_SECDISP += libTrustedUI
SECUREMSM_SECDISP += vendor.qti.hardware.trustedui@1.0-service-qti
SECUREMSM_SECDISP += vendor.qti.hardware.trustedui@1.0-service-qti.rc
SECUREMSM_SECDISP += vendor.qti.hardware.trustedui@1.0
endif

PRODUCT_PACKAGES += $(SECUREMSM_SECCAM)
PRODUCT_PACKAGES += $(SECUREMSM_SECDISP)
