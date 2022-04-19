/*
 * Copyright (c) 2012-2013, 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _SEC_DISP_INTERNALS_H
#define _SEC_DISP_INTERNALS_H

/*
 * This class wraps the android NativeWindow API and 
 * provides simplified interface to allocate and display buffers.
 * The code here was referenced from --
 *   vendor/qcom/proprietary/mm-video-noship/vidc/vdec-omx/
 */

#define SUCCESS 0
#define FAILURE (-1)

class SecDisp {
public:
    static const uint32_t DEFAULT_DISPLAY_WIDTH = 1440;
    static const uint32_t DEFAULT_DISPLAY_HEIGHT = 2560;

    class Config {
        public:
            uint32_t width;
            uint32_t height;
            uint32_t x;
            uint32_t y;
            int colorFormat;
            uint32_t usageFlags;

        Config () {
            reset();
        }

        void reset () {
            width = 0; height = 0;
            x = 0; y = 0;
            colorFormat = 0;
            usageFlags = 0x0;
        }
    };

    SecDisp(uint32_t width, uint32_t height);
    ~SecDisp();

    int32_t init();
    int32_t setConfig(const Config& config);
    int32_t allocateNativeBuffers(uint32_t buffersForHeap);
    int32_t dequeue(ANativeWindowBuffer*& dispBuf);
    int32_t enqueue(ANativeWindowBuffer* dispBuf, bool render);

    Config mConfig;
    uint32_t mDisplayWidth;
    uint32_t mDisplayHeight;
    uint16_t numAllocatedBufs;

private:
    sp<SurfaceComposerClient> mSurfaceComposer;
    sp<Surface> mSurface;
    sp<SurfaceControl> mControl;
    sp<ANativeWindow> mNativeWindow;
};


#define BYTES_IN_MEX       (4)
#define BYTES_IN_INPUT       (32)

#define SUI_MSG_CMD_MSK 0x08 /* Command, requires response */
#define SUI_MSG_RSP_MSK 0x04 /* Response to a command */
#define SUI_MSG_NOT_MSK 0x02 /* Notification */

/* ID for exchanged messages */
/* IDs for communications initiated by the Android Svc */
/* Call notifications*/
#define  SUI_MSG_ID_CALL_STATUS    0x01
/* Abort, for unreported reason (e.g. power) */
#define  SUI_MSG_ID_ABORT          0x02
/* Screen status notifications */
#define  SUI_MSG_ID_SCREEN_STATUS  0x03
/* IDs for communications initiated by the Listener */
/* Secure Display notifications */
#define  SUI_MSG_ID_SD_STARTING      0x10
#define  SUI_MSG_ID_SD_STOPPED       0x11
#define  SUI_MSG_SEC_DISP_PROPERTIES 0x12
#define  SUI_MSG_SEC_DISP_ENQUEUE    0x13
#define  SUI_MSG_SEC_DISP_DEQUEUE    0x14
#define  SUI_MSG_SEC_DISP_START      0x15
#define  SUI_MSG_SEC_DISP_STOP       0x16


/* General purpose IDs */
/* Invalid */
#define  SUI_MSG_ID_INVALID           0x00

/* Response codes, in the payload */
#define  SUI_MSG_RET_OK              0x00
#define  SUI_MSG_RET_ABORT           0x01
#define  SUI_MSG_RET_NOP             0x0F

typedef struct {
  uint8_t  bDisableOverlay;
  uint8_t bToRender;
  uint32_t colorFormat;
  uint32_t u32Height;
  uint32_t u32Width;
  uint32_t u32HPos;
  uint32_t u32VPos;
  uint64_t bufHandle;
}secdisp_ip;

typedef struct {
  int32_t status;
  uint16_t  u16NumBuffers;
  uint32_t u32Height;
  uint32_t u32Width;
  uint64_t bufHandle;
  uint32_t u32Stride;
  uint32_t u32PixelWidth;
  android::hardware::hidl_handle fd;
}secdisp_op;

/**
 * @brief Callback for received notifications.
 *
 * @param id[in]        1 byte identifier
 * @param payload[in]   1 byte payload
 * @param src_name[in]  Name (sun_path) of the originating socket
 * @param src_len[in]   Name length
 * */
typedef void (*svc_sock_notification_callback_t)(
  uint8_t const id,
  uint8_t const payload,
  char const * const src_name,
  size_t const src_len);

/**
 * @brief Callback for received commands.
 * If response is requested, the response parameter will be not-NULL and the
 * function is expected to assign a value to it before returning.
 *
 * @param id[in]        1 byte identifier
 * @param payload[in]   1 byte payload
 * @param input[in]     input data for the secure display operations
 * @param src_name[in]  Name (sun_path) of the originating socket
 * @param src_len[in]   Name lengt
 * @param response[out] 1 byte response
 *
 * @return 0 on success, negative errno otherwise
 * */
typedef int (*svc_sock_command_callback_t)(
  uint8_t const id,
  uint8_t const payload,
  secdisp_ip input,
  char const * const src_name,
  size_t const src_len);

/**
 * @brief Initialize the socket library.
 * Initialize the local socket, sets the default remote socket address,
 * starts the receiver thread.
 * Names to be used for local and remote sockets must be valid AF_UNIX
 * socket names.
 *
 * @param localname[in]       Name for local socket
 * @param localnamelen[in]    Its length
 * @param remotename[in]      Name for remote socket
 * @param remotenamelen[in]   Its length
 * @param filter[in]          1 to filter out all received messages not originating
 *                            from the specified remote socket
 *
 * @return 0 on success, negative errno otherwise
 * */
int svc_sock_init(
  char const * const localname,
  size_t const localnamelen,
  char const * const remotename,
  size_t const remotenamelen,
  uint32_t filter);

/**
 * @brief Closes the socket library.
 * Closes the local socket and stops the receiver thread.
 * */
int svc_sock_terminate(char *lname);

/**
 * @brief Send a notification message.
 * Send a notification message to a remote socket.
 * If no destination name is specified (i.e. NULL), the notification is sent to
 * the default remote address specified during @see svc_sock_init
 *
 * @param id[in]        1 byte identifier
 * @param payload[in]   1 byte payload
 * @param dest_name[in] Name (sun_path) of the destination socket
 * @param dest_len[in]  Name length
 *
 * @return 0 on success, negative errno otherwise
 * */
int svc_sock_send_notification(
  uint8_t const id,
  uint8_t const payload,
  char const * const dest_name,
  size_t const dest_len);

/**
 * @brief Send a notification message from an anonymous temporary socket.
 * Send a notification message to a remote socket from a temporarily created
 * anonymous socket, which is closed before the function terminated.
 * This function is meant to be used without the need to a previous call
 * to @see svc_sock_init
 * The destination name in this case has to be specified.
 *
 * @param id[in]        1 byte identifier
 * @param payload[in]   1 byte payload
 * @param dest_name[in] Name (sun_path) of the destination socket
 * @param dest_len[in]  Name length
 *
 * @return 0 on success, negative errno otherwise
 * */
int svc_sock_send_notification_noreg(
  uint8_t const id,
  uint8_t const payload,
  char const * const dest_name,
  size_t const dest_len);

/**
 * @brief Send a command message.
 * Send a command message to a remote socket.
 * If no destination name is specified (i.e. NULL), the notification is sent to
 * the default remote address specified during @see svc_sock_init.
 * The response to the command (1 byte) is returned via the response parameter.
 *
 * @param id[in]        1 byte identifier
 * @param payload[in]   1 byte payload
 * @param dest_name[in] Name (sun_path) of the destination socket
 * @param dest_len[in]  Name length
 * @param input[in]     input data for the secure display operations
 * @param output[out]   output data returned by the  secure display operations
 * @param response[out] Response
 *
 * @return 0 on success, negative errno otherwise
 * */
int svc_sock_send_command(
  uint8_t const id,
  uint8_t const payload,
  char const * const dest_name,
  size_t const dest_len,
  secdisp_ip * input,
  secdisp_op * output ,
  uint8_t *response);

/**
 * @brief Register the notification callback.
 * Register the user-provided function as notification callback.
 * A NULL pointer can be passed to unregister a previously registered callback.
 *
 * @param cback[in]   Callback
 * */
void svc_sock_register_notification_callback(svc_sock_notification_callback_t cback);

/**
 * @brief Register the command callback.
 * Register the user-provided function as command callback.
 * A NULL pointer can be passed to unregister a previously registered callback.
 *
 * @param cback[in]   Callback
 * */
void svc_sock_register_command_callback(svc_sock_command_callback_t cback);

/**
 * @brief Wait for a message.
 * Wait for a message to be received.
 * The message (and optionally its originating socket name) are passed out in the
 * output parameters.
 * If the name of the originating socket isn't required, a NULL can be passed.
 *
 * @param mex[out]          4 byte message
 * @param mex_len[in|out]   length of the message buffer
 * @param input[in]     input data for the secure display operations
 * @param src_name[out]     Name (sun_path) of the originating socket
 * @param src_len[out]      Name length
 *
 * @return 0 on success, negative errno otherwise
 * */
int svc_sock_wait_for_message(
  uint8_t *mex,
  size_t const mex_len,
  secdisp_ip *input,
  char * src_name,
  size_t *src_len);

/**
 * @brief Send the response to a command message.
 * Send the response to a command message to a remote socket.
 * If no destination name is specified (i.e. NULL), the message is sent to
 * the default remote address specified during @see svc_sock_init.
 *
 * @param id[in]        1 byte identifier
 * @param payload[in]   1 byte payload
 * @param dest_name[in] Name (sun_path) of the destination socket
 * @param dest_len[in]  Name length
 *
 * @return 0 on success, negative errno otherwise
 * */
int svc_sock_send_response(
  uint8_t const id,
  uint8_t const payload,
  char const * const dest_name,
  size_t const dest_len);

/**
 * @brief Send the response to a secure display command message.
 * Send the response to a command message to a remote socket.
 * If no destination name is specified (i.e. NULL), the message is sent to
 * the default remote address specified during @see svc_sock_init.
 *
 * @param id[in]        1 byte identifier
 * @param payload[in]   1 byte payload
 * @param output[out]   output data returned by the  secure display operations
 * @param dest_name[in] Name (sun_path) of the destination socket
 * @param dest_len[in]  Name length
 *
 * @return 0 on success, negative errno otherwise
 * */
int svc_sock_send_response_getdisp(
  uint8_t const id,
  uint8_t const payload,
  secdisp_op *output,
  char const * const dest_name,
  size_t const dest_len);

#endif //_SEC_DISP_INTERNALS_H
