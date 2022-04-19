"""
===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================
"""

blacklist = {
# <violation>  :  <expected usage>
# "msmexample" :  "$(MSMEXAMPLE)"
"msmsteppe"    :  "$(MSMSTEPPE)",
"talos"        :  "$(MSMSTEPPE)"
}

ignore_paths = [
# Relative paths from tree root, under which we accept/ignore violations.
"compatibility",
"developers",
"development",
"device",
"out",
"prebuilts",
"sdk",
"toolchain",
"tools",
"vendor/qcom/proprietary/common",
#
# TODO: Below are existing violations.
# Remove them once resolved.
#
"vendor/qcom/proprietary/prebuilt_HY11",
"vendor/qcom/proprietary/prebuilt_HY22",
"vendor/qcom/opensource/audio-kernel/asoc/Android.mk",
"vendor/qcom/proprietary/chi-cdk/vendor/chioverride/default/build/android/Android.mk",
"vendor/qcom/proprietary/fastmmi/Android.mk",
"vendor/qcom/proprietary/commonsys/fastmmi/Android.mk",
"vendor/qcom/proprietary/securemsm-noship/tlocd/Android.mk",
"vendor/qcom/proprietary/chi-cdk/vendor/Android.mk"
]
