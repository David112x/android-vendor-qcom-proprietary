/******************************************************************************
  @file framework/hidl_gen/utf_hidl_readme.txt
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

The files in framework/hidl_gen/ are copied from the intermediate files from the target build which are generated from the hidl interfaces.

IMS HIDL
========

- Hidl interface @ vendor/qcom/proprietary/interfaces/radio/ims/
- Intermediate header files from the target build @
    out/soong/.intermediates/vendor/qcom/proprietary/interfaces/radio/ims/1.0/vendor.qti.hardware.radio.ims@1.0_genc++_headers/gen/vendor/qti/hardware/radio/ims/1.0/
- UTF hidl files @ framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.0/

- *IMPORTANT NOTE*:
  Whenever you are making any changes in the ims hidl interface generate the header files using a normal target build and then copy the intermediate header files generated from the ims hidl to UTF hidl files location.
   Header files to copy
      types.h
      IImsRadio.h
      IImsRadioResponse.h
      IImsRadioIndication.h
- The ImsRadioAll.cpp, ImsRadioResponseAll.cpp and ImsRadioIndicationAll.cpp are copied from the intermediate files generated at out/soong/.intermediates/vendor/qcom/proprietary/interfaces/radio/ims/1.0/vendor.qti.hardware.radio.ims@1.0_genc++/gen/vendor/qti/hardware/radio/ims/1.0/ and keep only the relevent dummy function definitions. (These files are mostly not required to update in case of hidl interface change)

OEMHOOK HIDL
============
- Hidl interface @ vendor/qcom/proprietary/interfaces/radio/qcrilhook/
- Intermediate header files from the target build @
    out/soong/.intermediates/vendor/qcom/proprietary/interfaces/radio/qcrilhook/1.0/vendor.qti.hardware.radio.qcrilhook@1.0_genc++_headers/gen/vendor/qti/hardware/radio/qcrilhook/1.0/
- UTF hidl files @ framework/hidl_gen/qcrilhook/vendor/qti/hardware/radio/qcrilhook/1.0/

- *IMPORTANT NOTE*:
  Whenever you are making any changes in the ims hidl interface generate the header files using a normal target build and then copy the intermediate header files generated from the ims hidl to UTF hidl files location.
   Header files to copy
      types.h
      IQtiOemHook.h
      IQtiOemHookResponse.h
      IQtiOemHookIndication.h

BASE & MANAGER HIDL
===================
- These are required for compiling qcril related HIDL files.
