####################################################
# For addons to enable set TARGET_USES_QTIC as     #
# # true and to disable, set it to false.            #
# ####################################################
#
#
 ifeq ($(strip $(TARGET_USES_QTIC)),)
 TARGET_USES_QTIC := true
 endif
#
#
# ####################################################
# # addon specifics                                  #
# ####################################################
#
TARGET_USES_QTIC_CMCC            := $(TARGET_USES_QTIC)
TARGET_USES_QTIC_EXTENSION       := $(TARGET_USES_QTIC)
TARGET_USES_QTIC_CT              := $(TARGET_USES_QTIC)
TARGET_USES_QTIC_CU              := $(TARGET_USES_QTIC)
TARGET_USES_QTIC_REGIONAL        := $(TARGET_USES_QTIC)
TARGET_USES_QTIC_CTA             := $(TARGET_USES_QTIC)
TARGET_USES_QTIC_SVA             := $(TARGET_USES_QTIC)
TARGET_USES_QTIC_VOICEUI         := $(TARGET_USES_QTIC)
