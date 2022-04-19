

#CNE
CNE := cnd

ifeq ($(GENERIC_ODM_IMAGE),true)
CNE += cnd-generic.rc
else
CNE += cnd.rc
endif

CNE += CneApp
CNE += libcne
CNE += libcneapiclient
CNE += libwms
CNE += libxml

#this lib is no more compiled
#CNE += libvendorconn
#CNE += libmasc

#CNE_DBG
#CNE_DBG += test2client

#CACERT
CACERT := CACertService
CACERT += libcacertclient
CACERT += libjnihelper

#MUTUALEX
MUTUALEX := mutualex
MUTUALEX += mutualex.rc


PRODUCT_PACKAGES_ENG += cneapitest
PRODUCT_PACKAGES += $(CNE)
PRODUCT_PACKAGES += $(CACERT)
PRODUCT_PACKAGES += $(MUTUALEX)
#PRODUCT_PACKAGES_DEBUG += $(CNE_DBG)
