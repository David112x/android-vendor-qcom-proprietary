# QMI RIL unit test lite
CC = clang
CXX = clang++
FEATURES := -D_BSD_SOURCE -D_GNU_SOURCE -DQMI_RIL_HAL_UTF -DQMI_RIL_UTF -DRIL_SHLIB -DFEATURE_QCRIL_8960 -DLIBXML_UNSUPPORTED -DQCRIL_PROTOBUF_BUILD_ENABLED -DRIL_UTF_L_MASTER -DFEATURE_QCRIL_UIM_QMI -DFEATURE_QCRIL_QMI_CAT -DFEATURE_DATA_EMBMS -DFEATURE_QCRIL_LTE_DIRECT -DFEATURE_QCRIL_OEMHOOK_IMS_PRESENCE -DUTF_DEBUG_MODE
FEATURES += -Dqmi_client_send_msg_async=ril_utf_qmi_client_send_msg_async -Dqmi_client_send_msg_sync=ril_utf_qmi_client_send_msg_sync -Dqmi_client_init=ril_utf_qmi_client_init -Dqmi_client_init_instance=ril_utf_qmi_client_init_instance -Dqmi_client_release=ril_utf_qmi_client_release -Dqmi_client_message_decode=ril_utf_qmi_client_message_decode -Dqmi_client_message_encode=ril_utf_qmi_client_message_encode -Dqmi_client_register_error_cb=ril_utf_qmi_client_register_error_cb -Dqmi_client_register_notify_cb=ril_utf_qmi_client_register_notify_cb -Dqmi_client_notifier_init=ril_utf_qmi_client_notifier_init -Dqmi_client_get_service_instance=ril_utf_qmi_client_get_service_instance -Dqmi_client_get_any_service=ril_utf_qmi_client_get_any_service
FEATURES += -DSQLITE_THREADSAFE=1
M32 = #-m32
CFLAGS = -Wall -g $(M32) $(FEATURES) -c -g -fsanitize=address -fno-omit-frame-pointer
CXXFLAGS = -Wall -g $(M32) $(FEATURES) -fpermissive -c -g -fsanitize=address -fno-omit-frame-pointer -std=c++17
CXXFLAGS += -fno-exceptions -frtti
LDFLAGS = $(M32) -g -fsanitize=address
OUTPUTDIR = ./bin/
OBJECTC = core.o
MKDIR = mkdir $(OUTPUTDIR)
LDFLAGS +=\
					-lpthread\
					-lm\
					-lrt\
					-lc++

INC=\
		-I./include/\
		-I./include/qcril/\
		-I./framework/inc\
		-I./include/qcril/framework/\
		-I./include/qcril/framework/oem_socket\
		-I./include/qcril/framework/radio_config\
		-I./include/qcril/framework/nanopb_utils\
		-I./include/qcril/modules/android\
		-I./include/qcril/modules/cellinfo\
		-I./include/qcril/modules/ims\
		-I./include/qcril/modules/lpa\
		-I./include/qcril/modules/mbn\
		-I./include/qcril/modules/pbm\
		-I./include/qcril/modules/qmi\
		-I./include/qcril/modules/sms\
		-I./include/qcril/modules/uim\
		-I./include/qcril/modules/voice\
		-I./include/qcril/modules/qti_radio\
		-I./framework/hidl_gen/hardware/interfaces/radio/\
		-I./framework/hidl_gen/base\
		-I./framework/hidl_gen/ims\
		-I./framework/hidl_gen/am\
		-I./framework/hidl_gen/manager\
		-I./framework/hidl_gen/qcrilhook\
		-I./framework/hidl_gen/uim_remote_client\
		-I./framework/hidl_gen/uim_remote_server\
		-I./framework/hidl_gen/uim\
		-I./framework/hidl_gen/lpa\
		-I./framework/hidl_gen/qtiradio\
		-I./framework/hidl_gen/secure_element\
		-I./platform\
		-I./include/platform/android_ims/\
		-I$(QCRIL_HAL_DIR)/include/modules/nas\
		-I$(QCRIL_HAL_DIR)/include/modules/lpa\
		-I$(QCRIL_HAL_DIR)/include/modules/sms\
		-I$(QCRIL_HAL_DIR)/include/modules/mbn\
		-I$(QCRIL_HAL_DIR)/include/modules/pbm\
		-I$(QCRIL_HAL_DIR)/include/modules/qmi\
		-I$(QCRIL_HAL_DIR)/include/modules/uim\
		-I$(QCRIL_HAL_DIR)/include/modules/uim_remote\
                -I$(QCRIL_HAL_DIR)/include/modules/gstk\
                -I$(QCRIL_HAL_DIR)/include/modules/uim_service\
                -I$(QCRIL_HAL_DIR)/include/modules/uim_remote_server\
		-I$(QCRIL_HAL_DIR)/include/modules/voice\
		-I$(QCRIL_HAL_DIR)/include/modules/oemhook\
		-I$(QCRIL_HAL_DIR)/include/translators/android\
		-I$(QCRIL_HAL_DIR)/include/modules/android\
		-I$(QCRIL_HAL_DIR)/include/framework\
		-I$(QCRIL_HAL_DIR)/qcrild/include/telephony\
		-I$(QCRIL_HAL_DIR)/qcrild/include\
		-I$(QCRIL_HAL_DIR)/qcrild/libril\
		-I$(QCRIL_HAL_DIR)/modules/qmi/src/\
		-I$(QCRIL_DATA_DIR)/datamodule/include\
		-I$(QCRIL_DATA_DIR)/datamodule/include/sync\
		-I$(QCRIL_DATA_DIR)/datamodule/include/request\
		-I$(QCRIL_DATA_DIR)/datamodule/include/UnSolMessages\
		-I$(QMI_FWK_DIR)/inc\
		-I$(QMI_FWK_DIR)/qcci/inc\
		-I$(QMI_FWK_DIR)/common/inc\
		-I$(QMI_FWK_DIR)/smem_log\
		-I$(DATA_DIR)/qdp/inc\
		-I$(DATA_DIR)/dsutils/inc\
		-I$(DATA_DIR)/dsi_netctrl/inc\
		-I$(QMI_DIR)/data\
		-I$(QMI_DIR)/services\
		-I$(IMS_SHIP_DIR)/qmi\
		-I$(QMI_DIR)/inc\
		-I$(QMI_DIR)/src\
		-I$(QMI_DIR)/services\
		-I$(QMI_DIR)/platform\
		-I./api/inc\
		-I./tests/ril/inc\
		-I$(COMMON_DIR)/inc\
		-I$(QCRIL_HAL_DIR)/include/\
		-I$(QCRIL_HAL_DIR)/qcril_qmi\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/uim_remote_service/server_socket\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/uim_remote_service/client_socket\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/lte_direct\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/radio_config\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/services\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/oem_socket\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/nanopb_utils\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/utilities\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/utilities/bit_field\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/utilities/control\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/utilities/list\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/utilities/log\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/utilities/memory\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/utilities/synchronization\
		-I$(QCRIL_HAL_DIR)/qcril_qmi/utilities/timer\
		-I$(QCRIL_HAL_DIR)/modules/sms/src\
		-I$(QCRIL_HAL_DIR)/modules/ims/src\
		-I$(QCRIL_HAL_DIR)/modules/android_ims_radio/src\
		-I$(QCRIL_HAL_DIR)/qtimutex/include\
                -I$(QCRIL_QMI_SVC_HEADERS_DIR)/\
		-I$(QCRIL_HAL_DIR)/nanopb/\
		-I$(QCRIL_HAL_DIR)/modules/ims/src/ims_socket\
        -I$(QCRIL_HAL_DIR)/modules/qti_radio/src\


CFLAGS += $(INC)
CXXFLAGS += $(INC)

c_test_source =\
                ./framework/src/smem_log.c\
                ./framework/src/qmi_errors.c\
		./control_service_v01.c\
		./api/src/ril_utf_ims_payload.c\
		./api/src/ril_utf_androidTel_payload.c\
		./api/src/radio_config_interface.pb.c\
		./api/src/sap-api.pb.c\
		./api/src/LteDirectDiscovery.pb.c\
		./framework/src/ril_utf_global_defs.c\
		./framework/src/ril_utf_qmi_sim.c\
		./framework/src/ril_utf_rild_sim.c\
		./framework/src/ril_utf_msim_rild.c\
		./framework/src/ril_utf_oem_sim.c\
		./framework/src/ril_utf_proxy_sim.c\
		./framework/src/ril_utf_stubs.c\
		./framework/src/ril_utf_timed_cb_sim.c\
		./framework/src/ril_utf_set_sim_info.c\
		./framework/src/qtuner_v01.c\

ifneq ($(LOCAL_PATH),)
  qcril_hal_c_source := $(shell cd $(LOCAL_PATH); find $(QCRIL_HAL_DIR) -name '*.c' ! -path "*/utf/*")
else
  qcril_hal_c_source := $(shell find $(QCRIL_HAL_DIR) -name '*.c' ! -path "*/utf/*")
endif

cpp_test_framework :=\
                ./framework/src/HidlSupport.cpp\
                ./framework/src/HidlInternal.cpp\
                ./framework/src/Status.cpp\
                ./framework/src/host_init_stubs.cpp\
                ./framework/src/Threads.cpp\
                ./framework/src/RefBase.cpp\
                ./framework/src/StrongPointer.cpp\
                ./framework/src/String8.cpp\
                ./framework/src/String16.cpp\
                ./framework/src/Unicode.cpp\
                ./framework/src/Static.cpp\
                ./framework/src/SharedBuffer.cpp\
		./api/src/ril_utf_if.cpp\
		./api/src/ril_utf_qmiril_if.cpp\
		./api/src/ril_utf_ims_if.cpp\
		./api/src/ril_utf_androidTel_if.cpp\
		./api/src/ril_utf_oem_if.cpp\
		./api/src/ril_utf_msim_if.cpp\
		./api/src/ril_utf_func_exec_if.cpp\
		./api/src/ril_utf_ssr_if.cpp\
		./api/src/ril_utf_test_if.cpp\
		./api/src/ril_utf_qmi_msg_analyzer.cpp\
		./api/src/ril_utf_db_if.cpp\
		./api/src/ril_utf_property_if.cpp\
		./framework/src/ril_utf_core.cpp\
		./framework/src/ril_utf_ims_sim.cpp\
		./framework/src/ril_utf_message_printer.cpp\
		./framework/src/qcril_qmi_ims_packing.cpp\
		./framework/src/ril_utf_hidl_services.cpp\
		./framework/src/ril_utf_attributes.cpp\
		./framework/src/ril_utf_test_runner.cpp\
		./framework/src/ril_utf_i_ims_radio_sim.cpp\
		./framework/src/ril_utf_i_ims_radio_sim_utils.cpp\
		./framework/src/ril_utf_i_qti_oem_hook_sim.cpp\
		./framework/src/ril_utf_property.cpp\
		./platform/src/android/AndroidIRadio.cpp\
		./platform/src/android/AndroidQtiRadio.cpp\
		./platform/src/common/platform.cpp\
		./platform/src/android/SmsHidlUtil.cpp\
		./platform/src/android/NasHidlUtil.cpp\
		./platform/src/android/VoiceHidlUtil.cpp\
                ./platform/src/android/SimHidlUtil.cpp\
		./platform/src/android/IRadioResponseClientImpl.cpp\
		./platform/src/android/IRadioIndicationClientImpl.cpp\
		./platform/src/android/IQtiRadioResponseClientImpl.cpp\
		./platform/src/android_ims/imsIF.pb.cpp\
		./platform/src/android_ims/qcril_qmi_ims_msg_meta.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.0/RadioAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.0/RadioResponseAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.0/RadioIndicationAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.1/RadioAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.1/RadioResponseAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.1/RadioIndicationAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.2/RadioAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.2/RadioResponseAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.2/RadioIndicationAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.3/RadioAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.3/RadioResponseAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.3/RadioIndicationAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.4/types.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.4/RadioAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.4/RadioResponseAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/1.4/RadioIndicationAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/deprecated/1.0/OemHookAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/deprecated/1.0/OemHookResponseAll.cpp\
        ./framework/hidl_gen/hardware/interfaces/radio/android/hardware/radio/deprecated/1.0/OemHookIndicationAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.0/ImsRadioAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.0/ImsRadioResponseAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.0/ImsRadioIndicationAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.1/ImsRadioAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.1/ImsRadioIndicationAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.2/ImsRadioAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.2/ImsRadioResponseAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.2/ImsRadioIndicationAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.3/ImsRadioAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.3/ImsRadioResponseAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.3/ImsRadioIndicationAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.4/ImsRadioAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.4/ImsRadioResponseAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.4/ImsRadioIndicationAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.5/ImsRadioAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.5/ImsRadioIndicationAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.5/ImsRadioResponseAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.6/ImsRadioAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.6/ImsRadioIndicationAll.cpp\
		./framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.6/ImsRadioResponseAll.cpp\
		./framework/hidl_gen/qcrilhook/vendor/qti/hardware/radio/qcrilhook/1.0/QtiOemHookAll.cpp\
		./framework/hidl_gen/qcrilhook/vendor/qti/hardware/radio/qcrilhook/1.0/QtiOemHookIndicationAll.cpp\
		./framework/hidl_gen/qcrilhook/vendor/qti/hardware/radio/qcrilhook/1.0/QtiOemHookResponseAll.cpp\
		./framework/hidl_gen/am/vendor/qti/hardware/radio/am/1.0/QcRilAudioAll.cpp\
		./framework/hidl_gen/am/vendor/qti/hardware/radio/am/1.0/QcRilAudioCallbackAll.cpp\
		./framework/hidl_gen/uim/vendor/qti/hardware/radio/uim/1.0/UimAll.cpp\
		./framework/hidl_gen/uim/vendor/qti/hardware/radio/uim/1.0/UimIndicationAll.cpp\
		./framework/hidl_gen/uim/vendor/qti/hardware/radio/uim/1.0/UimResponseAll.cpp\
		./framework/hidl_gen/uim/vendor/qti/hardware/radio/uim/1.1/UimAll.cpp\
		./framework/hidl_gen/uim/vendor/qti/hardware/radio/uim/1.1/UimResponseAll.cpp\
		./framework/hidl_gen/uim_remote_client/vendor/qti/hardware/radio/uim_remote_client/1.0/UimRemoteServiceClientAll.cpp\
		./framework/hidl_gen/uim_remote_client/vendor/qti/hardware/radio/uim_remote_client/1.0/UimRemoteServiceClientIndicationAll.cpp\
		./framework/hidl_gen/uim_remote_client/vendor/qti/hardware/radio/uim_remote_client/1.0/UimRemoteServiceClientResponseAll.cpp\
		./framework/hidl_gen/uim_remote_server/vendor/qti/hardware/radio/uim_remote_server/1.0/UimRemoteServiceServerAll.cpp\
		./framework/hidl_gen/uim_remote_server/vendor/qti/hardware/radio/uim_remote_server/1.0/UimRemoteServiceServerIndicationAll.cpp\
		./framework/hidl_gen/uim_remote_server/vendor/qti/hardware/radio/uim_remote_server/1.0/UimRemoteServiceServerResponseAll.cpp\
		./framework/hidl_gen/lpa/vendor/qti/hardware/radio/lpa/1.0/UimLpaAll.cpp\
		./framework/hidl_gen/lpa/vendor/qti/hardware/radio/lpa/1.0/UimLpaResponseAll.cpp\
		./framework/hidl_gen/lpa/vendor/qti/hardware/radio/lpa/1.0/UimLpaIndicationAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/1.0/QtiRadioAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/1.0/QtiRadioResponseAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/1.0/QtiRadioIndicationAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.0/QtiRadioAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.0/QtiRadioResponseAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.0/QtiRadioIndicationAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.1/QtiRadioAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.1/QtiRadioResponseAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.1/QtiRadioIndicationAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.2/QtiRadioAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.2/QtiRadioResponseAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.2/QtiRadioIndicationAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.3/QtiRadioAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.3/QtiRadioResponseAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.4/QtiRadioAll.cpp\
		./framework/hidl_gen/qtiradio/vendor/qti/hardware/radio/qtiradio/2.4/QtiRadioResponseAll.cpp\
		./framework/hidl_gen/secure_element/android/hardware/secure_element/1.0/SecureElementAll.cpp\
		./framework/hidl_gen/secure_element/android/hardware/secure_element/1.0/SecureElementHalCallbackAll.cpp\

cpp_test_source :=\
		./tests/ril/src/ril_utf_test.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril_uim.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril_L.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril_boot_cases.cpp\
		./tests/ril/src/ril_utf_test_list_qmi_ril.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril_nas.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril_voice.cpp\
		./tests/ril/src/ril_utf_utility_if.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril_autogen.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril_lte_direct.cpp\
		./tests/ril/src/ril_utf_test_qmi_ril_ims.cpp\

ifneq ($(LOCAL_PATH),)
  qcril_hal_cpp_source := $(shell cd $(LOCAL_PATH);  find $(QCRIL_HAL_DIR) -name '*.cpp' ! -path "*/utf/*" ! -path "*/test/Qualcomm/*" | sort -d)
else
  qcril_hal_cpp_source := $(shell find $(QCRIL_HAL_DIR) -name '*.cpp'  ! -path "*/utf/*" ! -path "*/test/Qualcomm/*" | sort -d)
endif

cc_test_source =\

ifneq ($(LOCAL_PATH),)
  qcril_hal_cc_source += $(shell cd $(LOCAL_PATH);  find $(QCRIL_HAL_DIR) -name '*.cc' ! -path "*/utf/*")
else
  qcril_hal_cc_source += $(shell find $(QCRIL_HAL_DIR) -name '*.cc' ! -path "*/utf/*")
endif

c_sources := $(c_test_source)
c_sources += $(qcril_hal_c_source)
c_objects := $(c_sources:.c=.o)

cpp_sources := $(cpp_test_framework)
#cpp_sources += $(qcril_hal_cpp_source)
cpp_objects := $(cpp_sources:.cpp=.o)

cc_sources := $(cc_test_source)
cc_sources += $(qcril_hal_cc_source)
cc_objects := $(cc_sources:.cc=.o)


# QMI RIL and UTF core sources
objects = $(c_objects)
# Add UTF cpp sources
objects += $(cpp_objects)
# ADD UTF cc sources
objects += $(cc_objects)

ifneq ($(build_for_android), true)

all: test

test: $(objects)
	mkdir -p log
	@echo $(LOCAL_PATH)
	@echo $(cpp_test_source)
	@echo $(qcril_hal_cc_source)
	@echo $(qcril_hal_cpp_source)
	@echo =========================================
	@echo linking
	@echo =========================================
	$(CXX) $(LDFLAGS) -o test $(objects) -lrt -lpthread

clean:
	rm -f test $(objects) ./log/*
	rm -f ./doc/*.png *.db oem_connect_socket* rild_ims* rild_sync*

endif
