bin_PROGRAMS = ahaltest

AHALTEST_INCLUDES = -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-audio/qahw_api/inc

AHALTEST_CPPFLAGS = -std=c++14
AHALTEST_CPPFLAGS += -Wall -Wextra
AHALTEST_CPPFLAGS += -D__unused=__attribute__\(\(__unused__\)\)

ahaltest_CPPFLAGS = $(AHALTEST_CPPFLAGS) $(AHALTEST_INCLUDES)

ahaltest_LDADD = -lutils -lqahw

ahaltest_SOURCES = audio_input_flag.cpp
ahaltest_SOURCES += audio_input_flags.cpp
ahaltest_SOURCES += audio_io_handle_allocator.cpp
ahaltest_SOURCES += audio_output_flag.cpp
ahaltest_SOURCES += audio_output_flags.cpp
ahaltest_SOURCES += capture_bufferer.cpp
ahaltest_SOURCES += capture_test.cpp
ahaltest_SOURCES += chunk.cpp
ahaltest_SOURCES += command_line_parser.cpp
ahaltest_SOURCES += controller.cpp
ahaltest_SOURCES += data_semi_chunk.cpp
ahaltest_SOURCES += deserializable.cpp
ahaltest_SOURCES += dts_reader.cpp
ahaltest_SOURCES += file_capture_stream.cpp
ahaltest_SOURCES += file_capture_stream_config.cpp
ahaltest_SOURCES += file_capture_stream_factory.cpp
ahaltest_SOURCES += file_playback_stream.cpp
ahaltest_SOURCES += file_playback_stream_config.cpp
ahaltest_SOURCES += file_playback_stream_factory.cpp
ahaltest_SOURCES += file_playback_stream_mock.cpp
ahaltest_SOURCES += file_playback_stream_mock_2.cpp
ahaltest_SOURCES += file_reader.cpp
ahaltest_SOURCES += format_chunk.cpp
ahaltest_SOURCES += hal_stream.cpp
ahaltest_SOURCES += hal_test.cpp
ahaltest_SOURCES += hal_test_factory.cpp
ahaltest_SOURCES += hal_test_result_serializer.cpp
ahaltest_SOURCES += logger.cpp
ahaltest_SOURCES += log_message.cpp
ahaltest_SOURCES += log_writer.cpp
ahaltest_SOURCES += main.cpp
ahaltest_SOURCES += metadata.cpp
ahaltest_SOURCES += null_controller.cpp
ahaltest_SOURCES += playback_bufferer.cpp
ahaltest_SOURCES += playback_test.cpp
ahaltest_SOURCES += proxy_reader.cpp
ahaltest_SOURCES += qahw_module.cpp
ahaltest_SOURCES += read_stream.cpp
ahaltest_SOURCES += riff_wave_semi_chunk.cpp
ahaltest_SOURCES += session.cpp
ahaltest_SOURCES += session_factory.cpp
ahaltest_SOURCES += session_test.cpp
ahaltest_SOURCES += shared_buffer_control.cpp
ahaltest_SOURCES += sink.cpp
ahaltest_SOURCES += source.cpp
ahaltest_SOURCES += stream.cpp
ahaltest_SOURCES += stream_logger.cpp
ahaltest_SOURCES += stream_config.cpp
ahaltest_SOURCES += unused_chunk.cpp
ahaltest_SOURCES += wav_format.cpp
ahaltest_SOURCES += wav_reader.cpp
ahaltest_SOURCES += wav_writer.cpp
ahaltest_SOURCES += write_stream.cpp
ahaltest_SOURCES += xml_config.cpp