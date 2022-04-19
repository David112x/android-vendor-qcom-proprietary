/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <qtibus/QtiBusTransportServer.h>
#include "QtiBusSocketTransport.h"
#include "QtiBusSocketTransportServer.h"

QtiBusTransportServer &QtiBusTransportServer::get() {
  static QtiBusSocketTransportServer server(IPC_SOCKET_NAME);
  return server;
}
