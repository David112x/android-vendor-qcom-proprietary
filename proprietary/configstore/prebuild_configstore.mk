# This makefile helps in calling the script which calls the xmlparser with the xml
# and return 0 if parsing is sucessful

configstore_xml_parser: $(HOST_OUT_EXECUTABLES)/configstore_xml$(HOST_EXECUTABLE_SUFFIX)

 XMLPARSER_SCRIPT_NAME := configparser.sh
 XMLPARSER_SCRIPT := $(HOST_OUT_EXECUTABLES)/$(XMLPARSER_SCRIPT_NAME)

 #filtering out PASSED from the return as return value is stdio and consist of all the stdio done through xmlparser and script.

 ifeq (,$(filter $(shell $(XMLPARSER_SCRIPT) $(LOCAL_PATH)/$(LOCAL_MODULE)),"PASSED"))
  include $(BUILD_PREBUILT)
 endif

files: configstore_xml_parser

