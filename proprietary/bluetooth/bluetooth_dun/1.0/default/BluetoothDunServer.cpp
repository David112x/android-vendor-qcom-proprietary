/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "BluetoothDunServer.h"
#include "portbridge_sockif.h"
#include <utils/Log.h>
#include <cutils/sockets.h>
#include <sys/un.h>

#define LOG_TAG "vendor.qti.bluetooth_dun@1.0-impl"

namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth_dun {
namespace V1_0 {
namespace implementation {

class BluetoothDunServerDeathRecipient : public hidl_death_recipient {
    public:
        BluetoothDunServerDeathRecipient(const sp<IBluetoothDunServer> ServerIntf)
                                                : mServerIntf (ServerIntf) {}

        virtual void serviceDied(
            uint64_t /*cookie*/,
            const wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
                ALOGE("BluetoothDunServerDeathRecipient::serviceDied - Dun service died");
                has_died_ = true;
                mServerIntf->close_server();
            }
        sp<IBluetoothDunServer> mServerIntf;
        bool getHasDied() const { return has_died_; }
        void setHasDied(bool has_died) { has_died_ = has_died; }

    private:
        bool has_died_;
};

BluetoothDunServer::BluetoothDunServer()
    : death_recipient_(new BluetoothDunServerDeathRecipient(this)) {}


// Methods from ::vendor::qti::hardware::bluetooth_dun::V1_0::IBluetoothDunServer follow.
Return<Status> BluetoothDunServer::initialize(const sp<IBluetoothDunServerResponse>& resCallback) {
    ALOGI("%s: initializing the response cb", __func__);
    conn_sk = -1;
    res_cb = resCallback;

    if (res_cb != nullptr) {
      death_recipient_->setHasDied(false);
      res_cb->linkToDeath(death_recipient_, 0);
    }

    unlink_cb_ = [this](sp<BluetoothDunServerDeathRecipient>& death_recipient) {
        if (death_recipient->getHasDied()){
            ALOGI("Skipping unlink call, service died.");
        } else {
            res_cb->unlinkToDeath(death_recipient);
            ALOGI(" unlink  to death recipient ");
        }
    };

    return Status::SUCCESS;
}

Return<Status> BluetoothDunServer::sendCtrlMsg(CtrlMsg msgType) {
    portbridge_sock_intf_data_t ipc_msg;

    ALOGI("%s: msgType: %d", __func__, msgType);

    if (msgType == CtrlMsg::DUN_CONNECT_REQ) {
       int sk = -1;
       struct sockaddr_un addr;

       if (conn_sk == -1) {
          if ((sk = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0)) == -1) {
              ALOGE("%s: socket error : %s (%d)", __func__, strerror(errno), errno);
              return Status::FAILED;
          }

          memset(&addr, 0, sizeof(addr));
          addr.sun_family = AF_UNIX;
          strlcpy(addr.sun_path, PORTBRIDGE_SOCK_PATH, sizeof(addr.sun_path));

          if (connect(sk, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
              ALOGE("%s: connect error : %s (%d)", __func__, strerror(errno), errno);
              close(sk);
              return Status::FAILED;
          }

          conn_sk = sk;
          ALOGI("%s:port bridge socket interface successfully connected: fd: %d",
                  __func__, sk);
      }

      startReadThread();
      ipc_msg.msg_id = PORTBRIDGE_SOCK_SOCKET_MODE_ENABLE_REQ;
      int len = send (conn_sk, &ipc_msg, sizeof(ipc_msg.msg_id), 0);

      if (len < 0) {
          ALOGE("%s: Not able to send mesg to remote dev: %s, conn_sk: %d",
                  __func__, strerror(errno), conn_sk);
          stopReadThread();
      } else {
          ALOGI("%s: IPC msg type %d writen to socket: len:%d",
                  __func__, ipc_msg.msg_id, len);
      }
    } else if(msgType == CtrlMsg::DUN_DISCONNECT_REQ){
        ipc_msg.msg_id = PORTBRIDGE_SOCK_SOCKET_MODE_DISABLE_REQ;
        int len = send (conn_sk, &ipc_msg, sizeof(ipc_msg.msg_id), 0);

        if (len < 0) {
            ALOGE("%s: Not able to send mesg to remote dev: %s",
                    __func__, strerror(errno));
        }
        stopReadThread();
    }

    return Status::SUCCESS;
}

Return<Status> BluetoothDunServer::sendUplinkData(const hidl_vec<uint8_t>& packet) {
    portbridge_sock_intf_data_t ipc_msg;
    int data_len = packet.size();
    int num_of_bytes_to_send = sizeof(ipc_msg.msg_id)
            + sizeof(ipc_msg.data_msg.data_buffer.data_len) + data_len;

    memcpy(ipc_msg.data_msg.data_buffer.data, packet.data(), data_len);
    ipc_msg.msg_id = PORTBRIDGE_SOCK_DATA_IND;
    ipc_msg.data_msg.data_buffer.data_len = data_len;
    int len = send(conn_sk, &ipc_msg, num_of_bytes_to_send, 0);

    if (len < 0) {
        ALOGE("%s: Not able to send mesg to remote dev: %s",
                __func__, strerror(errno));
        return Status::FAILED;
    } else {
        ALOGV("%s: IPC msg type %d writen to socket: len:%d,",
                __func__, ipc_msg.msg_id, len);
    }

    return Status::SUCCESS;
}

Return<Status> BluetoothDunServer::sendModemStatus(uint8_t status) {
    portbridge_sock_intf_data_t ipc_msg;
    int num_of_bytes_to_send = sizeof(ipc_msg.msg_id) + sizeof(ipc_msg.data_msg.signals);

    ipc_msg.msg_id = PORTBRIDGE_SOCK_SIGNAL_IND;
    ipc_msg.data_msg.signals = status;
    int len = send (conn_sk, &ipc_msg, num_of_bytes_to_send, 0);

    if (len < 0) {
        ALOGE("%s: Not able to send mesg to remote dev: %s",
                __func__, strerror(errno));
        return Status::FAILED;
    } else {
        ALOGV("%s: IPC msg type %d writen to socket",
                __func__, status);
    }

    return Status::SUCCESS;
}

Return<void> BluetoothDunServer::close_server()
{
    ALOGI("%s: conn_sk: %d", __func__, conn_sk);

    unlink_cb_(death_recipient_);
    res_cb = nullptr;
    if (conn_sk != -1) {
        sendCtrlMsg(CtrlMsg::DUN_DISCONNECT_REQ);
    }
    return Void();
}

/*******************************************************************************
**
** Function         startReadThread
**
** Description      This function is called to start rx worker thread.
**
** Parameters:      void
**
**
** Returns          int
**
*******************************************************************************/
int BluetoothDunServer :: startReadThread()
{
    int ret = 0;
    ALOGI("Creating the read...");

    read_thread_ = std::thread([this]() { portBridgeReadThreadRoutine(); });

    if (!read_thread_.joinable()) {
        ALOGE("read thread is not joinable");
        ret = -1;
    }

    return ret;
}

/*******************************************************************************
**
** Function         stopReadThread
**
** Description      This function is called to stop read worker thread.
**
** Parameters:      void
**
**
** Returns          void
**
*******************************************************************************/
void BluetoothDunServer :: stopReadThread()
{
    ALOGI("%s:stopReadThread ++", __func__);
    read_thread_.join();
    ALOGI("%s:stopReadThread --", __func__);
}

/*******************************************************************************
**
** Function         portBridgeReadThreadRoutine
**
** Description      This function is main function of read worker thread.
**
** Parameters:      void
**
**
** Returns          void
**
*******************************************************************************/
void BluetoothDunServer :: portBridgeReadThreadRoutine()
{

    ALOGI("%s: starting portBridgeReadThreadRoutine conn_sk = %d", __func__, conn_sk);
    bool is_interuppted = false;
    portbridge_sock_intf_data_t ipc_msg;

    while (conn_sk > 0 && is_interuppted == false) {
        //wait for rx event
        int len = recv(conn_sk, &ipc_msg, sizeof(ipc_msg), 0);

        if (len <= 0) {
            ALOGE("%s: Not able to receive msg to remote dev: %s",
                    __func__, strerror(errno));
            is_interuppted = true;
            conn_sk = -1;
            break;
        } else {
            ALOGV("%s: IPC msg type : %d, len: %d", __func__, ipc_msg.msg_id, len);

            switch(ipc_msg.msg_id) {
                case PORTBRIDGE_SOCK_DATA_IND:
                    ALOGV("%s: PORTBRIDGE_SOCK_DATA_IND", __func__);

                    if (res_cb != nullptr) {
                        DunPacket packet;

                        packet.setToExternal((uint8_t *)&ipc_msg.data_msg.data_buffer.data[0],
                                ipc_msg.data_msg.data_buffer.data_len);
                        res_cb->downlinkDataEvent(packet);
                    }
                    break;

                case PORTBRIDGE_SOCK_SIGNAL_IND:
                    ALOGI("%s: PORTBRIDGE_SOCK_SIGNAL_IND:: signal: %d",
                            __func__, ipc_msg.data_msg.signals);

                    if (res_cb != nullptr) {
                        res_cb->modemStatusChangeEvent(ipc_msg.data_msg.signals);
                    }
                    break;

                case PORTBRIDGE_SOCK_SOCKET_MODE_ENABLE_RES:
                    ALOGI("%s: PORTBRIDGE_SOCK_SOCKET_MODE_ENABLE_RES:: result: %d",
                           __func__, ipc_msg.data_msg.result);
                    if (res_cb != nullptr) {
                    res_cb->ctrlMsgEvent(CtrlMsg::DUN_CONNECT_RESP,
                        ((ipc_msg.data_msg.result == PORT_BRIDGE_SUCCESS)
                          ? Status::SUCCESS : Status::FAILED));
                    }

                    if (ipc_msg.data_msg.result != PORT_BRIDGE_SUCCESS) {
                        is_interuppted = true;
                        close(conn_sk);
                        conn_sk = -1;
                    }
                    break;

                case PORTBRIDGE_SOCK_SOCKET_MODE_DISABLE_RES:
                    ALOGI("%s: PORTBRIDGE_SOCK_SOCKET_MODE_DISABLE_RES:: result: %d",
                         __func__, ipc_msg.data_msg.result);
                    if (res_cb != nullptr) {
                        res_cb->ctrlMsgEvent(CtrlMsg::DUN_DISCONNECT_RESP,
                            ((ipc_msg.data_msg.result == PORT_BRIDGE_SUCCESS)
                              ? Status::SUCCESS : Status::FAILED));
                    }

                    is_interuppted = true;
                    close(conn_sk);
                    conn_sk = -1;
                    break;

                default:
                    ALOGI("%s: unknown msg type: %d ", __func__, ipc_msg.msg_id);
                    break;
           }
        }
    }
    ALOGI("%s: Exiting portBridgeReadThreadRoutine!!!", __func__);
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IBluetoothDunServer* HIDL_FETCH_IBluetoothDunServer(const char* name) {
    return new BluetoothDunServer();
}
}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth_dun
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
