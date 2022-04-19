/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 */

#define LOG_TAG "fm_hci_helium"

#include <assert.h>
#include <utils/Log.h>

#include "bt_hci_bdroid.h"
#include "bt_vendor_lib.h"
#include "userial.h"
#include "fm_hci.h"
#include "wcnss_hci.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <cutils/properties.h>
#include <signal.h>

static int fm_hal_fd =0;

#define FM_VND_SERVICE_START "wc_transport.start_fmhci"
#define WAIT_TIMEOUT 200000 /* 200*1000us */

static void fm_hci_exit(void *arg);
static int power(struct fm_hci_t *hci, fm_power_state_t state);

static void event_notification(struct fm_hci_t *hci, uint16_t event)
{
    pthread_mutex_lock(&hci->event_lock);
    ALOGI("%s: Notifying worker thread with event: %d", __func__, event);
    ready_events |= event;
    pthread_cond_broadcast(&hci->event_cond);
    pthread_mutex_unlock(&hci->event_lock);
}

static void rx_thread_exit_handler(int sig)
{
    ALOGD("%s: sig = 0x%x", __func__, sig);
    if (sig == SIGUSR1) {
    ALOGE("Got the signal.. exiting");
    pthread_exit(NULL);
    }
}

static int vendor_init(struct fm_hci_t *hci)
{
    void *dlhandle = hci->dlhandle = NULL;
    unsigned char bdaddr[] = {0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33};

    dlhandle = dlopen("libbt-vendor.so", RTLD_NOW);
    if (!dlhandle) {
        ALOGE("!!! Failed to load libbt-vendor.so !!!");
        goto err;
    }

    hci->vendor = (bt_vendor_interface_t *) dlsym(dlhandle, "BLUETOOTH_VENDOR_LIB_INTERFACE");
    if (!hci->vendor) {
        ALOGE("!!! Failed to get bt vendor interface !!!");
        goto err;
    }

    ALOGI("FM-HCI: Registering the WCNSS HAL library by passing CBs and BD addr.");
    if (hci->vendor->init(&fm_vendor_callbacks, bdaddr) !=
                FM_HC_STATUS_SUCCESS) {
        ALOGE("FM vendor interface init failed");
        goto err;
    }

    return FM_HC_STATUS_SUCCESS;

err:
    return FM_HC_STATUS_FAIL;
}

static void vendor_close(struct fm_hci_t *hci)
{
    void *dlhandle = hci->dlhandle;

    if (hci->vendor)
        hci->vendor->cleanup();
    if (dlhandle) {
        dlclose(dlhandle);
        dlhandle = NULL;
    }
    hci->vendor = NULL;
}

/* De-queues the FM CMD from the struct transmit_queue_t */
static void dequeue_fm_tx_cmd(struct fm_hci_t *hci)
{
    struct transmit_queue_t *temp;
    uint16_t count = 0, len = 0;

    ALOGD("%s", __func__);
    while (1) {
        pthread_mutex_lock(&hci->tx_q_lock);
        temp = hci->first;
        if (!temp) {
            ALOGI("No FM CMD available in the Queue\n");
            pthread_mutex_unlock(&hci->tx_q_lock);
            return;
        } else {
            hci->first = temp->next;
        }
        pthread_mutex_unlock(&hci->tx_q_lock);

        pthread_mutex_lock(&hci->credit_lock);
wait_for_cmd_credits:
        while (hci->command_credits == 0) {
              pthread_cond_wait(&hci->cmd_credits_cond, &hci->credit_lock);
        }

        /* Check if we really got the command credits */
        if (hci->command_credits) {

            len = sizeof(struct fm_command_header_t) + temp->hdr->len;
again:
            /* Use socket 'fd' to send the command down to WCNSS Filter */
            count = write(hci->fd, (uint8_t *)temp->hdr + count, len);

            if (count < len) {
                len -= count;
                goto again;
            }
            count = 0;

            /* Decrement cmd credits by '1' after sending the cmd*/
            hci->command_credits--;
            if (temp->hdr)
                free(temp->hdr);
            free(temp);
        } else {
            if (!lib_running) {
                pthread_mutex_unlock(&hci->credit_lock);
                break;
            }
            goto wait_for_cmd_credits;
        }
        pthread_mutex_unlock(&hci->credit_lock);
    }
}

static int read_fm_event(struct fm_hci_t *hci, struct fm_event_header_t *pbuf, int len)
{
    fd_set readFds;
    sigset_t sigmask, emptymask;
    int n = 0, ret = -1, evt_len = -1,status=0;
    volatile int fd = hci->fd;
    struct sigaction action;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &sigmask, NULL) == -1) {
    ALOGE("failed to sigprocmask");
    }
    memset(&action, 0, sizeof(struct sigaction));
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = rx_thread_exit_handler;

    sigemptyset(&emptymask);

    if (sigaction(SIGUSR1, &action, NULL) < 0) {
    ALOGE("%s:sigaction failed", __func__);
    }

    while (lib_running)
    {
        FD_ZERO(&readFds);
        FD_SET(fd, &readFds);

        ALOGV("%s: Waiting for events from WCNSS FILTER...\n", __func__);

        /* Wait for event/data from WCNSS Filter */
        n = pselect(fd+1, &readFds, NULL, NULL, NULL, &emptymask);
        if (n > 0)
        {
            /* Check if event is available or not */
            if (FD_ISSET(fd, &readFds)) {
                ret = read(fd, (uint8_t *)pbuf, (size_t)(sizeof(struct fm_event_header_t) + MAX_FM_EVT_PARAMS));
                if (0 == ret) {
                    ALOGV("%s: read() returned '0' bytes\n", __func__);
                    break;
                }
                else {
                    ALOGV("%s: read() returned %d bytes of FM event/data\n", __func__, ret);
                    while (ret > 0) {
                        pthread_mutex_lock(&hci->credit_lock);
                        if (pbuf->evt_code == FM_CMD_COMPLETE) {
                            hci->command_credits = pbuf->params[0];
                            pthread_cond_signal(&hci->cmd_credits_cond);
                        } else if (pbuf->evt_code == FM_CMD_STATUS) {
                            hci->command_credits = pbuf->params[1];
                            pthread_cond_signal(&hci->cmd_credits_cond);
                        } else if (pbuf->evt_code == FM_HW_ERR_EVENT) {
                              ALOGI("%s: FM H/w Err Event Recvd. Event Code: 0x%2x", __func__, pbuf->evt_code);
                         /*  remove until support added */
                              //hci->vendor->ssr_cleanup(0x22);
                              status  = power(hci, FM_RADIO_DISABLE);
                              if (status < 0) {
                                 ALOGE("power off fm radio failed during SSR ");
                              }
                        } else {
                            ALOGE("%s: Not CS/CC Event: Recvd. Event Code: 0x%2x", __func__, pbuf->evt_code);
                        }
                        pthread_mutex_unlock(&hci->credit_lock);

                        evt_len = pbuf->evt_len;

                        /* Notify 'hci_tx_thread' about availability of event or data */
                        ALOGI("%s: \nNotifying 'hci_tx_thread' availability of FM event or data...\n", __func__);
                        event_notification(hci, HC_EVENT_RX);

                        if (hci->cb && hci->cb->process_event)
                            hci->cb->process_event(hci->private_data, (uint8_t *)pbuf);
                        else
                            ALOGE("%s: ASSERT $$$$$$ Callback function NULL $$$$$", __func__);

                        ret = ret - (evt_len + 3);
                        ALOGD("%s: Length of available bytes @ HCI Layer: %d", __func__, ret);
                        if (ret > 0) {
                            ALOGE("%s: Remaining bytes of event/data: %d", __func__, ret);
                            pbuf = (struct fm_event_header_t *)&pbuf->params[evt_len];
                        }
                    }
                } //end of processing the event

            } else
                ALOGV("%s: No data available, though select returned!!!\n", __func__);
        }
        else if (n < 0) {
           ALOGE("%s: select() failed with return value: %d", __func__, ret);
           lib_running =0;
        }
        else if (n == 0)
            ALOGE("%s: select() timeout!!!", __func__);
    }

    return ret;
}

static void *hci_read_thread(void *arg)
{
    int length = 0;
    struct fm_hci_t *hci = (struct fm_hci_t *)arg;

    struct fm_event_header_t *evt_buf = (struct fm_event_header_t *) malloc(sizeof(struct fm_event_header_t) + MAX_FM_EVT_PARAMS);

    if (!evt_buf) {
        ALOGE("%s: Memory allocation failed for evt_buf", __func__);
        goto cleanup;
    }

    length = read_fm_event(hci, evt_buf, sizeof(struct fm_event_header_t) + MAX_FM_EVT_PARAMS);
    ALOGD("length=%d\n",length);
    if(length <=0) {
       lib_running =0;
    }
    goto exit;

cleanup:
    lib_running = 0;
    hci = NULL;

exit:
    ALOGV("%s: Leaving hci_read_thread()", __func__);
    if (evt_buf)
        free(evt_buf);
    pthread_exit(NULL);
    return arg;
}

int connect_to_local_fmsocket(char* name) {
       socklen_t len; int sk = -1;

       ALOGE("%s: ACCEPT ", __func__);
       sk  = socket(AF_LOCAL, SOCK_STREAM, 0);
       if (sk < 0) {
           ALOGE("Socket creation failure");
           return -1;
       }

        if(socket_local_client_connect(sk, name,
            ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM) < 0)
        {
             ALOGE("failed to connect (%s)", strerror(errno));
             close(sk);
             sk = -1;
        } else {
             ALOGE("%s: Connection succeeded\n", __func__);
        }
        return sk;
}

/*
 * Reads the FM-CMDs from the struct transmit_queue_t and sends it down to WCNSS Filter
 * Reads events sent by the WCNSS Filter and copies onto RX_Q
 */
static void* hci_tx_thread(void *arg)
{
    uint16_t events;
    struct fm_hci_t *hci = (struct fm_hci_t *)arg;

    while (lib_running) {
        pthread_mutex_lock(&hci->event_lock);
        if (!(ready_events & HC_EVENT_TX))
            pthread_cond_wait(&hci->event_cond, &hci->event_lock);
        ALOGE("%s: ready_events= %d", __func__, ready_events);
        events = ready_events;
        if (ready_events & HC_EVENT_TX)
            ready_events &= (~HC_EVENT_TX);
        if (ready_events & HC_EVENT_RX)
            ready_events &= (~HC_EVENT_RX);
        pthread_mutex_unlock(&hci->event_lock);

        if (events & HC_EVENT_TX) {
            dequeue_fm_tx_cmd(hci);
        }
        if (events & HC_EVENT_RX) {
             ALOGI("\n##### FM-HCI Task : EVENT_RX available #####\n");
        }
        if (events & HC_EVENT_EXIT) {
            ALOGE("GOT HC_EVENT_EXIT.. exiting");
            break;
        }
    }

    ALOGV("%s: ##### Exiting hci_tx_thread Worker thread!!! #####", __func__);
    return NULL;
}

void stop_fmhal_service() {
       char value[PROPERTY_VALUE_MAX] = {'\0'};
       ALOGI("%s: Entry ", __func__);
       property_get(FM_VND_SERVICE_START, value, "false");
       if (strcmp(value, "false") == 0) {
           ALOGI("%s: fmhal service  has been stopped already", __func__);
//         return;
       }
       close(fm_hal_fd);
       fm_hal_fd = -1;
       property_set(FM_VND_SERVICE_START, "false");
       property_set("wc_transport.fm_service_status", "0");
       ALOGI("%s: Exit ", __func__);
}

void start_fmhal_service() {
       ALOGI("%s: Entry ", __func__);
       int i, init_success = 0;
       char value[PROPERTY_VALUE_MAX] = {'\0'};

       property_get(FM_VND_SERVICE_START, value, false);

       if (strcmp(value, "true") == 0) {
           ALOGI("%s: hci_filter has been started already", __func__);
           return;
       }
  //     property_set("wc_transport.fm_service_status", "0");
       usleep(100 * 1000);	// 100 msecs
       property_set(FM_VND_SERVICE_START, "true");
       ALOGI("%s: %s set to true ", __func__, FM_VND_SERVICE_START );
       for(i=0; i<45; i++) {
          property_get("wc_transport.fm_service_status", value, "0");
          if (strcmp(value, "1") == 0) {
               ALOGI("%s: wc_transport.fm_service_status set to %s", __func__,value);
               init_success = 1;
               break;
           } else {
               usleep(WAIT_TIMEOUT);
           }
        }
        ALOGI("start_fmhal_service status:%d after %f seconds \n", init_success, 0.2*i);

        ALOGI("%s: Exit ", __func__);
}

static int start_tx_thread(struct fm_hci_t *hci)
{
    struct sched_param param;
    int policy, result;

    ALOGI("FM-HCI: Creating the FM-HCI TASK...");
    if (pthread_create(&hci->tx_thread, NULL, hci_tx_thread, hci) != 0)
    {
        ALOGE("pthread_create failed!");
        lib_running = 0;
        return FM_HC_STATUS_FAIL;
    }

    if(pthread_getschedparam(hci->tx_thread, &policy, &param)==0)
    {
        policy = SCHED_NORMAL;
#if (BTHC_LINUX_BASE_POLICY!=SCHED_NORMAL)
        param.sched_priority = BTHC_MAIN_THREAD_PRIORITY;
#endif
        result = pthread_setschedparam(hci->tx_thread, policy, &param);
        if (result != 0)
        {
            ALOGW("libbt-hci init: pthread_setschedparam failed (%d)", \
                  result);
        }
    } else
        ALOGI("FM-HCI: Failed to get the Scheduling parameters!!!");

    return FM_HC_STATUS_SUCCESS;
}

static void stop_tx_thread(struct fm_hci_t *hci)
{
    int ret;

    ALOGV("%s++", __func__);
    if ((ret = pthread_kill(hci->tx_thread, SIGUSR1))
            == FM_HC_STATUS_SUCCESS) {
        ALOGE("%s:pthread_join", __func__);
         if ((ret = pthread_join(hci->tx_thread, NULL)) != FM_HC_STATUS_SUCCESS)
             ALOGE("Error joining tx thread, error = %d (%s)",
                     ret, strerror(ret));
    } else {
        ALOGE("Error killing tx thread, error = %d (%s)",
                ret, strerror(ret));
    }
}

static void *hci_mon_thread(void *arg)
{
    struct fm_hci_t *hci = (struct fm_hci_t *)arg;
    uint16_t events;
    ALOGV("%s", __func__);

    while (lib_running) {
        pthread_mutex_lock(&hci->event_lock);
        if (!(ready_events & HC_EVENT_EXIT))
            pthread_cond_wait(&hci->event_cond, &hci->event_lock);
        events = ready_events;
        if (ready_events & HC_EVENT_EXIT)
            ready_events &= (~HC_EVENT_EXIT);
        pthread_mutex_unlock(&hci->event_lock);

        ALOGD("events = 0x%x", events);
        if (events & HC_EVENT_EXIT) {
            ALOGD("Got Exit event.. Exiting HCI");
            fm_hci_exit(hci);
            break;
        }
    }
    ALOGV("%s--", __func__);
    return NULL;
}

static int start_mon_thread(struct fm_hci_t *hci)
{
    int ret = FM_HC_STATUS_SUCCESS;
    ALOGD("%s", __func__);
    if ((ret = pthread_create(&hci->mon_thread, NULL,
                    hci_mon_thread, hci)) !=0) {
        ALOGE("pthread_create failed! status = %d (%s)",
                ret, strerror(ret));
        lib_running = 0;
    }
    return ret;
}

static void stop_mon_thread(struct fm_hci_t *hci)
{
    int ret;
    ALOGV("%s++", __func__);
    if ((ret = pthread_kill(hci->mon_thread, SIGUSR1))
            == FM_HC_STATUS_SUCCESS) {
        ALOGE("%s:pthread_join", __func__);
        if ((ret = pthread_join(hci->mon_thread, NULL)) != FM_HC_STATUS_SUCCESS)
            ALOGE("Error joining mon thread, error = %d (%s)",
                    ret, strerror(ret));
    } else {
        ALOGE("Error killing mon thread, error = %d (%s)",
                ret, strerror(ret));
    }
}

static int start_rx_thread(struct fm_hci_t *hci)
{
    int ret = FM_HC_STATUS_SUCCESS;
    ALOGV("%s++", __func__);

    ALOGD("%s: Starting the userial read thread....", __func__);
    if ((ret = pthread_create(&hci->rx_thread, NULL, \
                       hci_read_thread, hci)) != 0) {
        ALOGE("pthread_create failed! status = %d (%s)",
                ret, strerror(ret));
        lib_running = 0;
    }
    return ret;
}

static void stop_rx_thread(struct fm_hci_t *hci)
{
    int ret;
    ALOGV("%s++", __func__);
    if ((ret = pthread_kill(hci->rx_thread, SIGUSR1))
            == FM_HC_STATUS_SUCCESS) {
        ALOGE("%s:pthread_join", __func__);
        if ((ret = pthread_join(hci->rx_thread, NULL)) != FM_HC_STATUS_SUCCESS)
            ALOGE("Error joining rx thread, error = %d (%s)",
                    ret, strerror(ret));
    } else {
        ALOGE("Error killing rx thread, error = %d (%s)",
                ret, strerror(ret));
    }
}

static int power(struct fm_hci_t *hci, fm_power_state_t state)
{
        int i,opcode,ret;
        int init_success = 0;
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        if (fm_hal_fd)
        {
            if (state)
                opcode = 2;
            else {
                opcode = 1;
            }
            ALOGI("%s:opcode: %x", LOG_TAG, opcode);
            ret = write(fm_hal_fd,&opcode, 1);
            if (ret < 0) {
                ALOGE("failed to write fm hal socket");
            } else {
                ret = FM_HC_STATUS_SUCCESS;
            }
        } else {
            ALOGE("Connect to socket failed ..");
            ret = -1;
        }
        if (state == FM_RADIO_DISABLE) {
            for (i=0; i<10; i++) {
                property_get("wc_transport.fm_power_status", value, "0");
                if (strcmp(value, "0") == 0) {
                    init_success = 1;
                    break;
                } else {
                    usleep(WAIT_TIMEOUT);
                }
            }
            ALOGI("fm power OFF status:%d after %f seconds \n", init_success, 0.2*i);
            stop_fmhal_service();
        }
        if (state == FM_RADIO_ENABLE) {
            for (i=0; i<10; i++) {
                property_get("wc_transport.fm_power_status", value, "0");
                if (strcmp(value, "1") == 0) {
                    init_success = 1;
                    break;
                } else {
                    usleep(WAIT_TIMEOUT);
                }
            }
            ALOGI("fm power ON status:%d after %f seconds \n", init_success, 0.2*i);
        }
        return ret;
}

#define CH_MAX 3
static int serial_port_init(struct fm_hci_t *hci)
{
    int i, ret;
    int fd_array[CH_MAX];

    for (int i = 0; i < CH_MAX; i++)
        fd_array[i] = -1;

    ALOGI("%s: Opening the TTy Serial port...", __func__);
    ret = hci->vendor->op(BT_VND_OP_FM_USERIAL_OPEN, &fd_array);

    if (fd_array[0] == -1) {
        ALOGE("%s unable to open TTY serial port", __func__);
        goto err;
    }
    hci->fd = fd_array[0];

    return FM_HC_STATUS_SUCCESS;

err:
    return FM_HC_STATUS_FAIL;
}

static void serial_port_close(struct fm_hci_t *hci)
{
    //TODO: what if hci/fm_vnd_if is null.. need to take lock and check
    ALOGI("%s: Closing the TTy Serial port!!!", __func__);
    hci->vendor->op(BT_VND_OP_FM_USERIAL_CLOSE, NULL);
    hci->fd = -1;
}

static int enqueue_fm_tx_cmd(struct fm_hci_t *hci, struct fm_command_header_t *pbuf)
{
    struct transmit_queue_t *element =  (struct transmit_queue_t *) malloc(sizeof(struct transmit_queue_t));

    if (!element) {
        ALOGI("Failed to allocate memory for element!!\n");
        return FM_HC_STATUS_NOMEM;
    }
    element->hdr = pbuf;
    element->next = NULL;

    pthread_mutex_lock(&hci->tx_q_lock);

    if (!hci->first) {
        hci->last = hci->first = element;
    } else {
        hci->last->next = element;
        hci->last = element;
    }
    ALOGI("%s: FM-CMD ENQUEUED SUCCESSFULLY", __func__);

    pthread_mutex_unlock(&hci->tx_q_lock);

    return FM_HC_STATUS_SUCCESS;
}

int fm_hci_transmit(void *hci, struct fm_command_header_t *buf)
{
    int status = FM_HC_STATUS_FAIL;

    if (!hci || !buf) {
        ALOGE("NULL input arguments");
        return FM_HC_STATUS_NULL_POINTER;
    }

    if ((status = enqueue_fm_tx_cmd((struct fm_hci_t *)hci, buf))
            == FM_HC_STATUS_SUCCESS)
        event_notification(hci, HC_EVENT_TX);

    return status;
}

void fm_hci_close(void *arg) {

    ALOGV("%s  close fm userial ", __func__);

    struct fm_hci_t *hci = (struct fm_hci_t *)arg;
    if (!hci) {
        ALOGE("NULL arguments");
        return;
    }
    event_notification(hci, HC_EVENT_EXIT);
    pthread_mutex_lock(&hci->event_lock);
again:
    pthread_cond_wait(&hci->event_cond, &hci->event_lock);
    if (!(ready_events & HC_EVENT_EXIT_DONE))
        goto again;
    pthread_mutex_unlock(&hci->event_lock);
}

int fm_hci_init(fm_hci_hal_t *hci_hal)
{
    int ret = FM_HC_STATUS_FAIL;
    struct fm_hci_t *hci = NULL;
    ALOGV("++%s", __func__);

    if (!hci_hal || !hci_hal->hal) {
        ALOGE("NULL input argument");
        return FM_HC_STATUS_NULL_POINTER;
    }

    hci = malloc(sizeof(struct fm_hci_t));
    if (!hci) {
        ALOGE("Failed to malloc hci context");
        return FM_HC_STATUS_NOMEM;
    }
    memset(hci, 0, sizeof(struct fm_hci_t));

    pthread_mutex_init(&hci->tx_q_lock, NULL);
    pthread_mutex_init(&hci->credit_lock, NULL);
    pthread_mutex_init(&hci->event_lock, NULL);

    pthread_cond_init(&hci->event_cond, NULL);
    pthread_cond_init(&hci->cmd_credits_cond, NULL);

    start_fmhal_service();
    fm_hal_fd = connect_to_local_fmsocket("fmhal_sock");
    if (fm_hal_fd == -1) {
        ALOGI("FM hal service socket connect failed..");
        goto err_socket;
    }
    ALOGI("fm_hal_fd = %d", fm_hal_fd);

    lib_running = 1;
    ready_events = 0;
    hci->command_credits = 1;
    hci->fd = -1;

    ret = vendor_init(hci);
    if (ret)
        goto err_vendor;
    ret = power(hci, FM_RADIO_ENABLE);
    if (ret)
        goto err_power;
    ret = serial_port_init(hci);
    if (ret)
        goto err_serial;
    ret = start_mon_thread(hci);
    if (ret)
        goto err_thread_mon;
    ret = start_tx_thread(hci);
    if (ret)
        goto err_thread_tx;
    ret = start_rx_thread(hci);
    if (ret)
        goto err_thread_rx;

    hci->cb = hci_hal->cb;
    hci->private_data = hci_hal->hal;
    hci_hal->hci = hci;
    ALOGD("--%s success", __func__);
    return FM_HC_STATUS_SUCCESS;

err_thread_rx:
    stop_rx_thread(hci);
err_thread_tx:
    stop_tx_thread(hci);
err_thread_mon:
    stop_mon_thread(hci);
err_serial:
    serial_port_close(hci);
err_power:
    power(hci, FM_RADIO_DISABLE);
err_vendor:
    vendor_close(hci);
err_socket:
    stop_fmhal_service();

    pthread_mutex_destroy(&hci->tx_q_lock);
    pthread_mutex_destroy(&hci->credit_lock);
    pthread_mutex_destroy(&hci->event_lock);
    pthread_cond_destroy(&hci->event_cond);
    pthread_cond_destroy(&hci->cmd_credits_cond);

    lib_running = 0;
    ready_events = 0;
    hci->command_credits = 0;
    free(hci);

    ALOGE("--%s fail", __func__);
    return ret;
}

static void fm_hci_exit(void *arg)
{
    struct fm_hci_t *hci = (struct fm_hci_t *)arg;
    ALOGE("%s", __func__);

    lib_running = 0;
    ready_events = HC_EVENT_EXIT;
    hci->command_credits = 0;
    serial_port_close(hci);
    power(hci, FM_RADIO_DISABLE);//need to address this
    vendor_close(hci);
    pthread_cond_broadcast(&hci->event_cond);
    pthread_cond_broadcast(&hci->cmd_credits_cond);
    event_notification(hci, HC_EVENT_EXIT_DONE);
    stop_rx_thread(hci);
    stop_tx_thread(hci);
    ALOGD("Tx, Rx Threads join done");
    pthread_mutex_destroy(&hci->tx_q_lock);
    pthread_mutex_destroy(&hci->credit_lock);
    pthread_mutex_destroy(&hci->event_lock);
    pthread_cond_destroy(&hci->event_cond);
    pthread_cond_destroy(&hci->cmd_credits_cond);

    free(hci);
    hci = NULL;
}

