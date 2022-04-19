/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 */

#include "pa_qti_effect_test.h"
#include <stdio.h>
#include <getopt.h>
#include <signal.h>

static void pa_qti_usage(void) {
    printf("\nUsage : pa_qti_effect_test -e <effect type> 0 for sink based effects. 1 for port based effects -i for interactive mode\n");
    printf("\nExample to enable effect on sink with sink id 1: pa_qti_effect_test -e 0. Give sink id as 1, effect index 0:bassboost 1:virualizer 2:equalizer 3:reverb 4:audiosphere\n");
    printf("\nExample to enable audiosphere effect on speaker: pa_qti_effect_test -e 4. Give speaker as port name, effect index 4:audiosphere\n");
    printf("\nHelp : pa_qti_effect_test -h\n");
}

int main(int argc, char *argv) {
    int sink_index = -1;
    char port_name[64]= {0};
    int effect_index = -1;
    thread_func_t func_ptr = NULL;
    thread_data_t *ethread_data = NULL;
    bool interactive = false;
    uint32_t effect_type = EFFECT_TYPE_INVALID;
    int rc = -1;
    cmd_data_t cmd_data = {0};

    struct option long_options[] = {
        {"effect-type", required_argument, 0, 'e'},
        {"interactive", no_argument, 0, 'i'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };

    int opt = 0;
    int opt_index = 0;

    while ((opt = getopt_long(argc,
                              (char * const *)argv,
                              "-ie:h",
                              long_options,
                              &opt_index)) != -1) {

        print_log("for argument %c, value is %s\n", opt, optarg);

        switch(opt) {
            case 'e':
                effect_type = atoi(optarg);
                if (effect_type == EFFECT_TYPE_SINK) {
                    printf("Enter sink id\n");
                    scanf("%d", &sink_index);
                } else if (effect_type == EFFECT_TYPE_PORT) {
                    printf("Enter port name\n");
                    scanf("%s", port_name);
                } else {
                    printf("Invalid effect type\n");
                    return -1;
                }
                printf("Enter effect index \n");
                scanf("%d", &effect_index);
                if (effect_index < 0 || effect_index >= MAX_SUPPORTED_EFFECTS) {
                    printf("Invalid effect index\n");
                    return -1;
                }
                func_ptr = effect_thread_funcs[effect_index];
                break;
            case 'i':
                printf("Interactive mode\n");
                interactive = true;
                break;
            case 'h':
            default:
                pa_qti_usage();
                return 0;
                break;
       }
    }

    if(effect_type == EFFECT_TYPE_INVALID) {
        printf("Please enter effect type: 0->sink based, 1->port based\n");
        scanf("%d", &effect_type);
        printf("Enter sink id\n");
        scanf("%d", &sink_index);
        if(effect_type == EFFECT_TYPE_SINK) {
            printf("Enter effect index \n");
            scanf("%d", &effect_index);
        } else if(effect_type == EFFECT_TYPE_PORT) {
            printf("Enter port name\n");
            scanf("%s", port_name);
            printf("Enter effect index \n");
            scanf("%d", &effect_index);
        } else {
            printf("Invalid effect type\n");
            return -1;
        }
        if (effect_index < 0 || effect_index >= MAX_SUPPORTED_EFFECTS) {
            printf("Invalid effect index\n");
            return -1;
        }

        func_ptr = effect_thread_funcs[effect_index];
    }

    ethread_data = pa_qti_create_effect_thread(func_ptr, effect_index);
    ethread_data->sink_index = sink_index;
    ethread_data->port_name = port_name;
    ethread_data->effect_type = effect_type;
    rc = pa_qti_effect_init();

    if (rc != 0) {
        print_log("pa_qti_effect_init failed\n");
        ethread_data->exit = true;
        free(ethread_data);
        return -1;
    }

    pa_qti_notify_effect_command(ethread_data, EFFECT_CREATE, -1, 0, NULL);
    if(effect_type == 1)
        pa_qti_notify_effect_command(ethread_data, EFFECT_CMD, PA_QTI_EFFECT_CMD_SET_DEVICE, sizeof(uint32_t), &ethread_data->device_index);

    if (!interactive) {
        pa_qti_notify_effect_command(ethread_data, EFFECT_GET_DESC, -1, 0, NULL);
        pa_qti_notify_effect_command(ethread_data, EFFECT_GET_VERSION, -1, 0, NULL);
        pa_qti_notify_effect_command(ethread_data, EFFECT_RELEASE, -1, 0, NULL);
        print_log("Waiting for effect thread to exit\n");
        pthread_join(ethread_data->effect_thread, NULL);
    } else {
        cmd_data.exit = false;
        cmd_data.fx_data_ptr = &(ethread_data);
        pthread_attr_init(&(cmd_data.attr));
        pthread_attr_setdetachstate(&(cmd_data.attr), PTHREAD_CREATE_JOINABLE);
        rc = pthread_create(&(cmd_data.cmd_thread), &(cmd_data.attr),
                &command_thread_func, &(cmd_data));
        if (rc < 0) {
            print_log("could not create effect command thread!\n");
            return -1;
        }
        print_log("Waiting for effect thread to exit\n");
        pthread_join(cmd_data.cmd_thread, NULL);
        print_log("Exiting main\n");
    }
}

