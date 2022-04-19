

#IMS_NEWARCH
IMS_NEWARCH := imsdatadaemon
IMS_NEWARCH += imsdatadaemon.rc
IMS_NEWARCH += imsqmidaemon
IMS_NEWARCH += imsqmidaemon.rc
IMS_NEWARCH += ims_rtp_daemon
IMS_NEWARCH += ims_rtp_daemon.rc
IMS_NEWARCH += lib-imsdpl
IMS_NEWARCH += lib-imsqimf
IMS_NEWARCH += lib-imsSDP
IMS_NEWARCH += lib-siputility
IMS_NEWARCH += lib-rtpcommon
IMS_NEWARCH += lib-rtpcore
IMS_NEWARCH += lib-rtpsl
IMS_NEWARCH += lib-imsvtcore
IMS_NEWARCH += librcc
IMS_NEWARCH += vendor.qti.imsrtpservice@2.1-service-Impl



#IMS_RCS
IMS_RCS := lib-imsxml
IMS_RCS += lib-uceservice
IMS_RCS += lib-imscmservice
IMS_RCS += imsrcsd
IMS_RCS += imsrcsd.rc
IMS_RCS += lib-imsrcs-v2
IMS_RCS += lib-imsrcsbaseimpl


PRODUCT_PACKAGES += $(IMS_NEWARCH)
PRODUCT_PACKAGES += $(IMS_RCS)
