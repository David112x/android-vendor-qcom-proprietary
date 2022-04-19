SECUREMSM_SECCAM_DBG := seccamsample

SECUREMSM_SECCAM := libseccamservice
SECUREMSM_SECCAM += libseccam-ipc
SECUREMSM_SECCAM += seccamservice
SECUREMSM_SECCAM += libseccam
SECUREMSM_SECCAM += vendor.qti.hardware.seccam@1.0

SECUREMSM_SECDISP := com.qualcomm.qti.services.secureui
SECUREMSM_SECDISP += libsecureuisvc_jni
SECUREMSM_SECDISP += libsecureui_svcsock_system
SECUREMSM_SECDISP += vendor.qti.hardware.tui_comm@1.0
SECUREMSM_SECDISP += vendor.qti.hardware.qdutils_disp@1.0

SECUREMSM_SECDISP += com.qualcomm.qti.services.systemhelper
SECUREMSM_SECDISP += libsystemhelper_jni
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0

PRODUCT_PACKAGES += $(SECUREMSM_SECCAM)
PRODUCT_PACKAGES += $(SECUREMSM_SECDISP)
PRODUCT_PACKAGES_DEBUG += $(SECUREMSM_SECCAM_DBG)
