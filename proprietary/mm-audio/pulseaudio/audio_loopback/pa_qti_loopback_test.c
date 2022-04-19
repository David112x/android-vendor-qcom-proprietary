/* pa_qti_loopback_test.c
*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*/

#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "pa_qti_loopback.h"
#include "pa_qti_effect_test.h"

#define MAX_SUPPORTED_SINK 1

#define PA_QTI_LOOPBACK_SOURCE_PORT_ID   0x4C00
#define PA_QTI_LOOPBACK_SINK_PORT_ID     0x4D00

volatile bool exit_loopback = false;

static struct user_config {
    char *sink_device;
    unsigned int num_channels;
    pa_encoding_t format;
    int sample_rate;
    pa_sample_format_t bitwidth;
    double loopback_gain;
} params[MAX_SUPPORTED_SINK];

static void pa_qti_loopback_test_usage(void);
static void pa_qti_loopback_test_signal_handler(int signal __attribute__((unused)));
static void pa_qti_loopback_test_init_port_config(int *num_srcs,
                                                  struct pa_qti_loopback_port_config *src_cfg,
                                                  int *num_sinks,
                                                  struct pa_qti_loopback_port_config *sink_cfg);

static void pa_qti_loopback_test_signal_handler(int signal __attribute__((unused))) {
    printf("%s, Prepare for loopback exit\n", __func__);
    exit_loopback = true;
}

static void pa_qti_loopback_test_usage() {
    printf("\nMax number of supported sinks is %d\n", MAX_SUPPORTED_SINK);
    printf("\nUsage : pa_qti_loopback_test -d <duration_in_seconds> -a -o <sink_device_name> -i "
           "<source_device_name> -w <src_channels> -x <src_format> -y <src_samplerate> -z "
           "<src_bitwidth> -c <sink_channels> -f <sink_format> -r <sink_sample_rate> -b <sink_bitwidth> "
           "-v <loopback_volume(range 0 to 4.0)>\n");
    printf("\nUse -a option if auto source port config detection is to be disabled\n");
    printf("\nBitwidth mapping (applicable for PCM only) - 16bit: %d, 24bit: %d, 32bit: %d\n",
                                            PA_SAMPLE_S16LE, PA_SAMPLE_S24LE, PA_SAMPLE_S32LE);
    printf("\nFormat mapping - PCM: %d, AC3: %d, EAC3: %d, TRUEHD: %d\n", PA_ENCODING_PCM,
            PA_ENCODING_AC3_IEC61937, PA_ENCODING_EAC3_IEC61937, PA_ENCODING_TRUEHD_IEC61937);
    printf("\nExample to play HDMI source for 1 minute on speaker with volume unity and "
           "format PCM: pa_qti_loopback_test -d 60 -o speaker -i hdmi-in -f 1 -r 48000-v 1.0\n");
    printf("\nExample to play for 5 minutes on headphone: pa_qti_loopback_test  -d 300 -o "
           "headphone -i hdmi-in -f 1 -r 48000 -v 2.0 -b 3 -c 2\n");
    printf("\nFor multisession usecase, use the option -P followed by the params of next sink\n");
    printf("\nMultisession usecase example, pa_qti_loopback_test -d 10 -o headphone -v 3.0 -P "
           "-o speaker -v 2.0 -f 48000\n");
    printf("\nHelp : pa_qti_loopback_test -h\n");
}

static void pa_qti_loopback_event_detection_callback(pa_qti_loopback_event_t event,
                                                 struct pa_qti_loopback_event_data event_data) {
    int i = 0;

    printf("Event %d received\n", event);

    if (event != PA_QTI_LOOPBACK_ERROR_EVENT) {
        printf("%s: portname - %s, sample rate - %u, channels - %u, format - %s\n", __func__, event_data.port_name,
                                           event_data.sample_rate, event_data.channels, event_data.encoding_string);

        for (i = 0; i < NUM_CHANNEL_POSITIONS; i++)
            printf("%s: channel maps position %d - %s\n", __func__, i, event_data.channel_maps[i]);
    }
}

static void pa_qti_loopback_test_init_port_config(int *num_srcs,
                                                  struct pa_qti_loopback_port_config *src_cfg,
                                                  int *num_sinks,
                                                  struct pa_qti_loopback_port_config *sink_cfg) {
    int i;

    printf("Initializing default configs\n");

    *num_srcs = 1;
    src_cfg->id = PA_QTI_LOOPBACK_SOURCE_PORT_ID;
    src_cfg->role =  PA_QTI_LOOPBACK_PORT_ROLE_SOURCE;
    src_cfg->config_mask = PA_QTI_LOOPBACK_PORT_CONFIG_AUTO;
    src_cfg->sample_rate = 48000;
    src_cfg->format = PA_ENCODING_PCM;
    src_cfg->num_channels = 2;
    src_cfg->bitwidth = PA_SAMPLE_S16LE;
    src_cfg->gain = 1.0;
    src_cfg->port_name = "hdmi-in";

    *num_sinks = MAX_SUPPORTED_SINK;
    for (i = 0; i < *num_sinks; i++) {
        sink_cfg[i].id = PA_QTI_LOOPBACK_SINK_PORT_ID;
        sink_cfg[i].role =  PA_QTI_LOOPBACK_PORT_ROLE_SINK;
        sink_cfg[i].config_mask = (PA_QTI_LOOPBACK_PORT_CONFIG_ALL ^
                                   PA_QTI_LOOPBACK_PORT_CONFIG_GAIN);
        sink_cfg[i].sample_rate = 48000;
        sink_cfg[i].format = PA_ENCODING_PCM;
        sink_cfg[i].num_channels = 2;
        sink_cfg[i].bitwidth = PA_SAMPLE_S16LE;
        sink_cfg[i].gain = 1.0;
        sink_cfg[i].port_name = "speaker";
    }

    printf("Done initializing default configs\n");
}

int main(int argc, char *argv[]) {
    unsigned int play_duration_in_seconds = 600;
    unsigned int play_duration_in_msec = 0;
    unsigned int play_duration_elapsed_msec = 0;
    double loopback_gain = 1.0;
    char *sink_device = NULL;
    char *source_device = NULL;
    unsigned int src_num_channels = 0;
    pa_encoding_t src_format = PA_ENCODING_INVALID;
    int src_sample_rate = 0;
    pa_sample_format_t src_bitwidth = PA_SAMPLE_INVALID;
    bool src_cfg_auto_detection_disable = 0;
    int num_srcs = 0, num_sinks = 0;
    struct pa_qti_loopback_port_config src_cfg;
    struct pa_qti_loopback_port_config sink_cfg[MAX_SUPPORTED_SINK];
    pa_qti_loopback_handle_t loopback_handle = PA_QTI_LOOPBACK_PATCH_HANDLE_NONE;
    struct pa_qti_loopback_port_config cfg;
    int status = 0;
    int i, index = 0;
    thread_data_t *ethread_data = NULL;
    thread_func_t func_ptr = NULL;
    cmd_data_t cmd_data;
    int effect_index = -1;
    int rc = 0;

    struct option long_options[] = {
        {"sink-device", required_argument, 0, 'o'},
        {"source-device", required_argument, 0, 'i'},
        {"play-duration", required_argument, 0, 'd'},
        {"src-channels", required_argument, 0, 'w'},
        {"src-format", required_argument, 0, 'x'},
        {"src-samplerate", required_argument, 0, 'y'},
        {"src-bitwidth", required_argument, 0, 'z'},
        {"sink-channels", required_argument, 0, 'c'},
        {"sink-format", required_argument, 0, 'f'},
        {"sink-samplerate", required_argument, 0, 'r'},
        {"sink-bitwidth", required_argument, 0, 'b'},
        {"play-volume", required_argument, 0, 'v'},
        {"effect-index", required_argument, 0, 'e'},
        {"src-config", no_argument, 0, 'a'},
        {"next-sink", no_argument, 0, 'P'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt = 0;
    int option_index = 0;

    /* Setting default sink gain as 1.0 */
    for (i = 0; i < MAX_SUPPORTED_SINK; i++)
        params[i].loopback_gain = 1.0;

    while ((opt = getopt_long(argc,
                              argv,
                              "-o:i:d:w:x:y:z:c:f:r:b:v:e:hPa",
                              long_options,
                              &option_index)) != -1) {
        printf("for argument %c, value is %s\n", opt, optarg);

        switch (opt) {
            case 'o':
                params[index].sink_device = (char *)malloc(strlen(optarg));
                strncpy(params[index].sink_device, optarg, strlen(optarg));
                break;
            case 'i':
                source_device = (char *)malloc(strlen(optarg));
                strncpy(source_device, optarg, strlen(optarg));
                break;
            case 'a':
                src_cfg_auto_detection_disable = true;
                break;
            case 'w':
                src_num_channels = atoi(optarg);
                break;
            case 'x':
                src_format = atoi(optarg);
                break;
            case 'y':
                src_sample_rate = atoi(optarg);
                break;
            case 'z':
                src_bitwidth = atoi(optarg);
                break;
            case 'd':
                play_duration_in_seconds = atoi(optarg);
                break;
            case 'c':
                params[index].num_channels = atoi(optarg);
                break;
            case 'f':
                params[index].format = atoi(optarg);
                break;
            case 'r':
                params[index].sample_rate = atoi(optarg);
                break;
            case 'b':
                params[index].bitwidth = atoi(optarg);
                break;
            case 'v':
                params[index].loopback_gain = atof(optarg);
                break;
            case 'e':
                effect_index = atoi(optarg);
                if (effect_index < 0 || effect_index >= MAX_SUPPORTED_EFFECTS) {
                    printf("Invalid effect index. Please enter a value b/n 0 & 4.\n");
                    return -EINVAL;
                }
                func_ptr = effect_thread_funcs[effect_index];
                ethread_data = pa_qti_create_effect_thread(func_ptr, effect_index);
                break;
            case 'P':
                index++;
                if (index >= MAX_SUPPORTED_SINK) {
                    printf("Max supported sinks is %d\n", MAX_SUPPORTED_SINK);
                    return 0;
                }
                printf("Index incremented to %d\n", index);
                break;
            case 'h':
            default:
                pa_qti_loopback_test_usage();
                return 0;
                break;
        }
    }

    printf("\nTranscode loopback test begin\n");

    if (play_duration_in_seconds < 0 | play_duration_in_seconds > 360000) {
        printf("\nPlayback duration %d invalid or unsupported(range : 1 to 360000, defaulting to 600 seconds )\n",
               play_duration_in_seconds);
        play_duration_in_seconds = 600;
    }

    play_duration_in_msec = play_duration_in_seconds * 1000;

    /* Register the SIGINT to close the App properly */
    if (signal(SIGINT, pa_qti_loopback_test_signal_handler) == SIG_ERR)
        printf("Failed to register SIGINT: %d\n", errno);
    else
        printf("SIGINT registered successfully\n");

    printf("Initializing GDBus for loopback usecase\n");
    status = pa_qti_loopback_init(PA_QTI_LOOPBACK_MODULE_ID_PRIMARY);
    if (status < 0) {
        printf("GDBus setup failed\n");
        goto exit;
    }

    /* Initialize with default port configs */
    pa_qti_loopback_test_init_port_config(&num_srcs, &src_cfg, &num_sinks, sink_cfg);

    if (source_device)
        src_cfg.port_name = source_device;

    if (src_num_channels)
        src_cfg.num_channels = src_num_channels;

    if (src_format != PA_ENCODING_INVALID)
        src_cfg.format = src_format;

    if (src_bitwidth != PA_SAMPLE_INVALID)
        src_cfg.bitwidth = src_bitwidth;

    if (src_sample_rate)
        src_cfg.sample_rate = src_sample_rate;

    if (src_cfg_auto_detection_disable)
        src_cfg.config_mask = (PA_QTI_LOOPBACK_PORT_CONFIG_ALL ^ PA_QTI_LOOPBACK_PORT_CONFIG_GAIN);

    for (i = 0; i <= index; i++) {
        if (params[i].sink_device)
            sink_cfg[i].port_name = params[i].sink_device;

        if (params[i].num_channels != 0)
            sink_cfg[i].num_channels = params[i].num_channels;

        if (params[i].sample_rate != 0)
            sink_cfg[i].sample_rate = params[i].sample_rate;

        if (params[i].format != 0)
            sink_cfg[i].format = params[i].format;

        if (params[i].bitwidth != 0)
            sink_cfg[i].bitwidth = params[i].bitwidth;

        if (fabs(params[i].loopback_gain - 1.0) > 0.0001)
            sink_cfg[i].gain = params[i].loopback_gain;
    }

    loopback_handle = pa_qti_loopback_create(num_srcs, &src_cfg, num_sinks, sink_cfg, pa_qti_loopback_event_detection_callback);
    if (loopback_handle == PA_QTI_LOOPBACK_PATCH_HANDLE_NONE) {
        printf("pa_qti_loopback_create failed\n");
        exit_loopback = true;
    } else {
        printf("pa_qti_loopback_create success\n");

        /* Run loopback for 5s with user specified volume in -v option */
        sleep(5);

        /* Set the sink volume during run time */
        for (i = 0; i < num_sinks; i++) {
            sink_cfg[i].gain = 2.0;
            sink_cfg[i].config_mask = PA_QTI_LOOPBACK_PORT_CONFIG_GAIN;
            status = pa_qti_loopback_set_port_config(&sink_cfg[i]);

            if (!status)
                printf("pa_qti_loopback_set_port_config success\n");
            else
                printf("pa_qti_loopback_set_port_config failed\n");
        }

        status = pa_qti_loopback_get_port_config(&cfg, PA_QTI_LOOPBACK_SINK_PORT_ID,
                                                     PA_QTI_LOOPBACK_PORT_ROLE_SINK);
        if (!status) {
            printf("pa_qti_loopback_get_port_config success\n");
            printf("id %d, role %d, configmask %u, samplerate %u, channels %u, format %d, "
                   "bitwidth %d, gain %lf, port_name %s\n", cfg.id, cfg.role, cfg.config_mask,
                   cfg.sample_rate, cfg.num_channels, cfg.format, cfg.bitwidth, cfg.gain,
                   cfg.port_name);
        } else {
            printf("pa_qti_loopback_get_port_config failed\n");
        }

        if (ethread_data != NULL) {
            ethread_data->patch_id = loopback_handle;
            ethread_data->effect_type = EFFECT_TYPE_LOOPBACK;
            pa_qti_effect_init();
            pa_qti_notify_effect_command(ethread_data, EFFECT_CREATE, -1, 0, NULL);
            cmd_data.exit = false;
            cmd_data.fx_data_ptr = &ethread_data;
            pthread_attr_init(&(cmd_data.attr));
            pthread_attr_setdetachstate(&(cmd_data.attr), PTHREAD_CREATE_JOINABLE);
            rc = pthread_create(&(cmd_data.cmd_thread), &(cmd_data.attr),
                                &command_thread_func, &(cmd_data));
            if (rc < 0) {
                printf("could not create effect command thread!\n");
                return -1;
            }
        }
    }

    printf("Main thread loop\n");

    while (!exit_loopback) {
        usleep(100 * 1000);
        play_duration_elapsed_msec += 100;
        if (play_duration_in_msec <= play_duration_elapsed_msec) {
            exit_loopback = true;
            printf("Elapsed set playback duration %d seconds, exiting test\n",
                                                      play_duration_in_msec/1000);
            break;
        }
    }

    printf("Main thread loop exit\n");

    if (loopback_handle != PA_QTI_LOOPBACK_PATCH_HANDLE_NONE) {
        if (ethread_data != NULL) {
            if (ethread_data->exit != true)
                pa_qti_notify_effect_command(ethread_data, EFFECT_RELEASE, -1, 0, NULL);
            ethread_data = NULL;
            cmd_data.exit = true;
            usleep(100000);
            rc = pthread_cancel(cmd_data.cmd_thread);
            if (rc != 0)
                printf("Fail to cancel thread!\n");
            rc = pthread_join(cmd_data.cmd_thread, NULL);
            if (rc < 0 )
                printf("Failed to join effect command thread\n");
        }
        if (pa_qti_loopback_stop(loopback_handle) < 0)
            printf("pa_qti_loopback_stop failed\n");
        else
            loopback_handle = PA_QTI_LOOPBACK_PATCH_HANDLE_NONE;
    }

exit:
    pa_qti_loopback_deinit();

    if (source_device)
        free(source_device);

    for (i = 0; i <= index; i++) {
        if (params[i].sink_device)
            free(params[i].sink_device);
    }

    return 0;
}
