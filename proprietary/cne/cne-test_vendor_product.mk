

#CNE_DBG

CNE_DBG := AndsfParser
CNE_DBG += cnelogger
CNE_DBG += swimcudpclient
CNE_DBG += swimstcpclient
CNE_DBG += swimtcpclient
CNE_DBG += swimudpclient
#CNE_DBG += test2client
#CNESETTINGS_DBG := CNESettings


PRODUCT_PACKAGES += $(CNE_DBG)

PRODUCT_PACKAGES_DEBUG += init.qcom.data.test.rc
