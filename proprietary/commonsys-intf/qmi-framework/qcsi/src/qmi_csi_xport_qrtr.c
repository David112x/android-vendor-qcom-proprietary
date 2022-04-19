/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <sys/time.h>
#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qmi_csi_common.h"
#include "qmi_csi_target.h"
#include "qmi_fw_debug.h"
#include <linux/qrtr.h>
#include "libqrtr.h"
#include "ns.h"

#define MAX(a,b) (a > b ? a : b)
#define ALIGN_SIZE(x) ((4 - ((x) & 3)) & 3)
/* Tx queues for handling flow control:
 * +--------+  +--------+  +--------+
 * | q head |->| dest 1 |->| dest 2 |->...
 * +--------+  +--------+  +--------+
 *                 |            |
 *                buf 1        buf 1
 *                 |            |
 *                buf 2        buf 2
 *                 |            |
 *                ...          ...
 */
struct buf_s
{
  LINK(struct buf_s, link);
  void *msg;
  uint32_t len;
};

struct dest_s
{
  struct xport_qrtr_addr *dest_addr;
  LINK(struct dest_s, link);
  LIST(struct buf_s, bufs);
  uint8_t dest_busy;
};

struct xport_qrtr_addr {
        uint32_t node_id;
        uint32_t port_id;
};

struct xport_qrtr_svc {
  uint32_t service;
  uint32_t instance;
};

struct conn_cli
{
  LINK(struct conn_cli, link);
  struct xport_qrtr_addr addr;
};

struct xport_handle
{
  qmi_csi_xport_type *xport;
  struct xport_qrtr_svc svc;
  int fd;
  uint32_t max_rx_len;
  qmi_csi_lock_type tx_q_lock;
  LIST(struct dest_s, tx_q);
  LIST(struct conn_cli, client_list);
  qmi_csi_xport_options_type *xport_options;
};

int qmi_csi_xport_qrtr_supported(void)
{
  int fd = 0;

  fd = socket(AF_QIPCRTR, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if(fd < 0)
  {
    if(errno == EAFNOSUPPORT)
    {
      QMI_CSI_LOGD("%s: QRTR IS NOT SUPPORTED: errno[%d] \n", __func__, errno);
      return 0;
    }
    else
    {
      QMI_FW_LOGE("%s: failed errno[%d]\n", __func__, errno);
      return 1;
    }
  }
  QMI_CSI_LOGD("%s: QRTR IS SUPPORTED\n", __func__);
  close(fd);
  return 1;
}

/* List functions */
static void add_conn_cli
(
struct xport_handle *xp,
struct xport_qrtr_addr *addr
)
{
  struct conn_cli *client = LIST_HEAD(xp->client_list);
  while(client)
  {
    if(addr->node_id == client->addr.node_id
      && addr->port_id == client->addr.port_id)
      return;
    client = client->link.next;
  }
  client = calloc(1, sizeof(struct conn_cli));
  if(client)
  {
    LINK_INIT(client->link);
    client->addr.node_id = addr->node_id;
    client->addr.port_id = addr->port_id;
    LIST_ADD(xp->client_list, client, link);
  }
}

static void purge_conn_cli
(
struct xport_handle *xp
)
{
  struct conn_cli *client, *tmp;

  client = LIST_HEAD(xp->client_list);
  while(client)
  {
    tmp = client;
    client = client->link.next;
    LIST_REMOVE(xp->client_list, tmp, link);
    FREE(tmp);
  }
}

static struct dest_s *find_tx_q
(
struct xport_handle *xp,
struct xport_qrtr_addr *addr
)
{
  struct dest_s *dest = LIST_HEAD(xp->tx_q);
  while(dest)
  {
    if(!memcmp(addr, dest->dest_addr, sizeof(struct xport_qrtr_addr)))
      break;
    dest = dest->link.next;
  }
  return dest;
}

static struct dest_s *get_tx_q
(
struct xport_handle *xp,
struct xport_qrtr_addr *addr
)
{
  struct dest_s *dest = find_tx_q(xp, addr);
  if(!dest)
  {
    dest = calloc(1, sizeof(struct dest_s));
    if(dest)
    {
      LINK_INIT(dest->link);
      LIST_INIT(dest->bufs);
      dest->dest_addr = addr;
      LIST_ADD(xp->tx_q, dest, link);
    }
  }
  return dest;
}

static void purge_tx_q
(
struct xport_handle *xp,
struct dest_s *dest
)
{
  struct buf_s *buf = LIST_HEAD(dest->bufs);

  LIST_REMOVE(xp->tx_q, dest, link);
  while(buf)
  {
    struct buf_s *to_free = buf;
    FREE(buf->msg);
    buf = buf->link.next;
    FREE(to_free);
  }
  FREE(dest);
}

static qmi_csi_error  put_tx_q
(
struct xport_handle *xp,
struct xport_qrtr_addr *addr,
uint8_t *msg,
uint32_t msg_len,
uint32_t  max_q_len
)
{
  struct dest_s *dest;
  struct buf_s *buf;
  qmi_csi_error rc = QMI_CSI_NO_ERR;

  dest = get_tx_q(xp, addr);
  if(!dest)
  {
    rc = QMI_CSI_INTERNAL_ERR;
    goto bail_fail;
  }
  if(max_q_len > 0 && LIST_CNT(dest->bufs) >=  max_q_len)
  {
    dest->dest_busy =  1;
    rc = QMI_CSI_CONN_BUSY;
    goto bail_fail;
  }
  buf = calloc(1, sizeof(struct buf_s));

  if(!buf)
  {
    rc = QMI_CSI_INTERNAL_ERR;
    goto bail_fail;
  }
  LINK_INIT(buf->link);
  buf->len = msg_len;
  buf->msg = MALLOC(msg_len);
  if(!buf->msg)
  {
    FREE(buf);
    rc = QMI_CSI_INTERNAL_ERR;
    goto bail_fail;
  }
  memcpy(buf->msg, msg, msg_len);
  LIST_ADD(dest->bufs, buf, link);
  return QMI_CSI_NO_ERR;

bail_fail:
  return rc;
}

static void purge_dest_s
(
struct xport_handle *xp
)
{
  struct dest_s *dest, *tmp;

  LOCK(&xp->tx_q_lock);
  dest = LIST_HEAD(xp->tx_q);
  while (dest)
  {
    tmp = dest;
    dest = dest->link.next;
    purge_tx_q(xp, tmp);
  }
  UNLOCK(&xp->tx_q_lock);
}

static void handle_resume_tx
(
void *handle,
struct xport_qrtr_addr *addr
)
{
  struct sockaddr_qrtr sq;
  struct dest_s *dest;
  struct buf_s *q_buf, *to_free;
  ssize_t sendto_rc;
  struct xport_handle *xp = (struct xport_handle *)handle;
  uint32_t  max_q_len = 0;
  int notify_resume_client = 0;

  if (!xp || !addr)
      return;

  if (QMI_CSI_SEND_FLAG_RATE_LIMITED && NULL != xp->xport_options)
  {
    max_q_len = xp->xport_options->rate_limited_queue_size;
  }

  LOCK(&xp->tx_q_lock);
  dest = find_tx_q(xp, addr);
  if (dest) {
    q_buf = LIST_HEAD(dest->bufs);
    sq.sq_family = AF_QIPCRTR;
    sq.sq_node = dest->dest_addr->node_id;
    sq.sq_port = dest->dest_addr->port_id;

    while (q_buf)
    {
      sendto_rc = sendto(xp->fd, q_buf->msg, q_buf->len, MSG_DONTWAIT, (void *)&sq, sizeof(sq));
      if((sendto_rc < 0) && (errno == EAGAIN))
      {
        QMI_CSI_LOGD("%s Send Failed! for port %08x:%08x, Retry Later\n", __func__,
                    dest->dest_addr->node_id, dest->dest_addr->port_id);
        break;
      }
      else if(sendto_rc >= 0)
      {
        QMI_CSI_LOGD("%s Sent [%d]: %d queued bytes for port %08x:%08x\n", __func__,
                                       xp->fd, q_buf->len, dest->dest_addr->node_id,
                                       dest->dest_addr->port_id);
      }
      else
      {
        QMI_CSI_LOGD("%s Send Failed! for port %08x:%08x\n", __func__,
                    dest->dest_addr->node_id, dest->dest_addr->port_id);
      }
      to_free = q_buf;
      LIST_REMOVE(dest->bufs, q_buf, link);
      q_buf = q_buf->link.next;
      FREE(to_free->msg);
      FREE(to_free);
    }

    if (LIST_CNT(dest->bufs) < max_q_len && dest->dest_busy)
    {
      notify_resume_client = 1;
      dest->dest_busy = 0;
    }

    if(!(LIST_CNT(dest->bufs)))
    {
      LIST_REMOVE(xp->tx_q, dest, link);
      FREE(dest);
    }
  }
  UNLOCK(&xp->tx_q_lock);

  if (notify_resume_client)
    qmi_csi_xport_resume_client(xp->xport, addr);
}

static qmi_csi_error init_socket
(
struct xport_handle *xp,
qmi_csi_os_params *os_params
)
{
  int val;

  xp->fd = socket(AF_QIPCRTR, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if(xp->fd <= 0)
  {
    QMI_FW_LOGE("%s: socket creation failed - %d\n", __func__, errno);
    return QMI_CSI_NO_MEM;
  }

  /* set fd nonblocking */
  val = fcntl(xp->fd, F_GETFL, 0);
  fcntl(xp->fd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(xp->fd, &os_params->fds);
  os_params->max_fd = MAX(os_params->max_fd, xp->fd);

  return QMI_CSI_NO_ERR;
}

static void *xport_open
(
 void *xport_data,
 qmi_csi_xport_type *xport,
 uint32_t max_rx_len,
 qmi_csi_os_params *os_params,
 qmi_csi_xport_options_type *options
)
{
  struct xport_handle *xp = calloc(1, sizeof(struct xport_handle));
  int align_size = 0;

  if (!xp)
  {
    QMI_FW_LOGE("%s: xp calloc failed\n", __func__);
    return NULL;
  }

  xp->svc.service = (uint32_t)-1;
  xp->svc.instance = (uint32_t)-1;
  xp->xport = xport;

  if (max_rx_len < sizeof(struct qrtr_ctrl_pkt))
    xp->max_rx_len = sizeof(struct qrtr_ctrl_pkt);
  else
    xp->max_rx_len = max_rx_len;
  xp->max_rx_len += QMI_HEADER_SIZE;
  align_size = ALIGN_SIZE(xp->max_rx_len);
  xp->max_rx_len += align_size;

  xp->xport_options = options;
  LOCK_INIT(&xp->tx_q_lock);
  LIST_INIT(xp->tx_q);
  LIST_INIT(xp->client_list);

  if (init_socket(xp, os_params) != QMI_CSI_NO_ERR)
    goto xport_open_free_xp;

  QMI_CSI_LOGD("xport_open[%d]: max_rx_len=%d\n", xp->fd, max_rx_len);
  return xp;

xport_open_free_xp:
  free(xp);
  return NULL;
}

static qmi_csi_error xport_reg
(
 void *handle,
 uint32_t service_id,
 uint32_t version
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  struct sockaddr_qrtr sq;
  socklen_t sl = sizeof(sq);
  struct qrtr_ctrl_pkt pkt;
  int rc;

  if (service_id == (uint32_t)-1 || version == (uint32_t)-1)
  {
    QMI_FW_LOGE("%s Invalid svc:%d ins:%d\n", __func__, service_id, version);
    return QMI_CSI_INTERNAL_ERR;
  }

  if(getsockname(xp->fd, (void *)&sq, &sl))
  {
    QMI_FW_LOGE("%s Failed to getsockname %d\n", __func__, errno);
    return QMI_CSI_INTERNAL_ERR;
  }

  if(sq.sq_family != AF_QIPCRTR || sl != sizeof(sq))
  {
    QMI_FW_LOGE("%s Invalid socket family\n", __func__);
    return QMI_CSI_INTERNAL_ERR;
  }

  memset(&pkt, 0, sizeof(pkt));
  pkt.cmd = cpu_to_le32(QRTR_TYPE_NEW_SERVER);
  pkt.server.service = cpu_to_le32(service_id);
  pkt.server.instance = cpu_to_le32(version);
  pkt.server.node = cpu_to_le32(sq.sq_node);
  pkt.server.port = cpu_to_le32(sq.sq_port);

  //send NEW_SERVER control message to Name server of same node
  sq.sq_port = QRTR_PORT_CTRL;

  rc = sendto(xp->fd, &pkt, sizeof(pkt), 0, (void *)&sq, sizeof(sq));
  if(rc < 0)
  {
    QMI_FW_LOGE("%s Failed for service_id=0x%x version=0x%x on %d error %d\n", __func__,
                                                   service_id, version, xp->fd, errno);
    return QMI_CSI_INTERNAL_ERR;
  }

  xp->svc.service = service_id;
  xp->svc.instance = version;

  QMI_CSI_LOGD("xport_reg[%d]: service_id=0x%x version=0x%x\n", xp->fd, service_id, version);
  return QMI_CSI_NO_ERR;
}

static qmi_csi_error xport_unreg
(
 void *handle,
 uint32_t service_id,
 uint32_t version
 )
{
  struct xport_handle *xp;
  xp = (struct xport_handle *)handle;
  QMI_CSI_LOGD("xport_unreg[%d]: type=0x%x version=0x%x\n", xp->fd, service_id, version);
  return QMI_CSI_NO_ERR;
}

static qmi_csi_error xport_send
(
 void *handle,
 void *addr,
 uint8_t *msg,
 uint32_t msg_len,
 uint32_t flags,
 void **client_data
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  struct sockaddr_qrtr sq;
  struct xport_qrtr_addr *s_addr = (struct xport_qrtr_addr *)addr;
  struct dest_s *dest;
  qmi_csi_error rc;
  ssize_t sendto_rc;
  uint32_t max_q_len = 0;

  if (!s_addr)
  {
    QMI_FW_LOGE("%s: Invalid address parameter\n", __func__);
    return QMI_CSI_INTERNAL_ERR;
  }

  if (0 != (flags & QMI_CSI_SEND_FLAG_RATE_LIMITED) && NULL != xp->xport_options)
  {
    max_q_len = xp->xport_options->rate_limited_queue_size;
  }

  sq.sq_family = AF_QIPCRTR;
  sq.sq_node = s_addr->node_id;
  sq.sq_port = s_addr->port_id;

  LOCK(&xp->tx_q_lock);
  dest = find_tx_q(xp, s_addr);

  if( dest && LIST_CNT(dest->bufs))
  {
    /* Queue the message so that it doesn't go out of order */
    rc = put_tx_q(xp, s_addr, msg, msg_len, max_q_len);
    if(rc == QMI_CSI_CONN_BUSY)
    {
      QMI_FW_LOGE("%s Queue exceeded, Retry sending for port %08x:%08x\n",
                               __func__, s_addr->node_id, s_addr->port_id);
    }
    else if(rc == QMI_CSI_NO_ERR)
    {
      QMI_CSI_LOGD("%s Packet queued for port %08x:%08x\n", __func__,
                                    s_addr->node_id, s_addr->port_id);
    }
    else
    {
      QMI_FW_LOGE("%s Error queuing packet for port %08x:%08x\n", __func__,
                                         s_addr->node_id, s_addr->port_id);
    }
    UNLOCK(&xp->tx_q_lock);
    return rc;
  }

  sendto_rc = sendto(xp->fd, msg, msg_len, MSG_DONTWAIT, (void *)&sq, sizeof(sq));
  if ((sendto_rc < 0) && (errno == EAGAIN))
  {
    /* queue to tx queue */
     rc = put_tx_q(xp, addr, msg, msg_len, max_q_len);
     if(rc == QMI_CSI_CONN_BUSY)
     {
       QMI_FW_LOGE("%s Queue exceeded, Retry sending for port %08x:%08x\n",
                                __func__, s_addr->node_id, s_addr->port_id);
     }
     else if(rc == QMI_CSI_NO_ERR)
     {
       QMI_CSI_LOGD("%s Packet queued for port %08x:%08x\n", __func__,
                                     s_addr->node_id, s_addr->port_id);
     }
     else
     {
       QMI_FW_LOGE("%s Error queuing packet for port %08x:%08x\n", __func__,
                                           s_addr->node_id, s_addr->port_id);
     }
  }
  else if (sendto_rc >= 0)
  {
    QMI_CSI_LOGD("Sent[%d]: %d bytes to port %08x:%08x\n", xp->fd, msg_len,
                 s_addr->node_id, s_addr->port_id);
    UNLOCK(&xp->tx_q_lock);
    return QMI_CSI_NO_ERR;
  }
  else /* Err on all other cases */
  {
    rc = QMI_CSI_INTERNAL_ERR;
    QMI_FW_LOGE("%s  QCSI Sendto failed for port %08x:%08x err[%d]\n", __func__, s_addr->node_id, s_addr->port_id, errno);
  }
  UNLOCK(&xp->tx_q_lock);

  return rc;
}

static void xport_handle_net_reset
(
struct xport_handle *xp,
qmi_csi_os_params *os_params
)
{
  purge_dest_s(xp);
  purge_conn_cli(xp);
  xport_reg((void*)xp, xp->svc.service, xp->svc.instance);
}

static void xport_handle_event
(
 void *handle,
 qmi_csi_os_params *os_params
 )
{
  int rx_len;
  unsigned char *buf;
  struct sockaddr_qrtr sq;
  socklen_t src_addr_size = sizeof(struct sockaddr_qrtr);
  struct xport_qrtr_addr addr;
  struct qrtr_ctrl_pkt rx_ctl_msg;
  struct xport_handle *xp = (struct xport_handle *)handle;
  struct dest_s *dest;

  if(FD_ISSET(xp->fd, &os_params->fds))
  {
    buf = malloc(xp->max_rx_len);
    if(!buf)
    {
      QMI_FW_LOGE("%s: Unable to allocate memory for buf\n", __func__);
      return;
    }
    do {
      src_addr_size = sizeof(struct sockaddr_qrtr);
      rx_len = recvfrom(xp->fd, buf, xp->max_rx_len, MSG_DONTWAIT, (void *)&sq, &src_addr_size);

      if (rx_len < 0)
      {
        if (errno == EAGAIN)
          break;
        if (errno == ENETRESET)
        {
          xport_handle_net_reset(xp, os_params);
          break;
        }
        QMI_FW_LOGE("%s: recvfrom err, len:%d errno:%d\n", __func__, rx_len, errno);
      }

      addr.node_id = sq.sq_node;
      addr.port_id = sq.sq_port;
      if (sq.sq_port == QRTR_PORT_CTRL)
      {
        memcpy(&rx_ctl_msg, buf, sizeof(rx_ctl_msg));
        addr.node_id = rx_ctl_msg.client.node;
        addr.port_id = rx_ctl_msg.client.port;
        QMI_CSI_LOGD("%s: CONTROL PKT cmd %d node %d port %d\n", __func__, rx_ctl_msg.cmd,
                     rx_ctl_msg.client.node, rx_ctl_msg.client.port);

        if (rx_ctl_msg.cmd == QRTR_TYPE_DEL_CLIENT) {
	  struct conn_cli *client;
          QMI_CSI_LOGD("Received REMOVE_CLIENT cmd for %08x:%08x\n",
                      rx_ctl_msg.client.node, rx_ctl_msg.client.port);
          /* Purge the Tx queue */
          LOCK(&xp->tx_q_lock);
          dest = find_tx_q(xp, &addr);
          if (dest)
             purge_tx_q(xp, dest);
          UNLOCK(&xp->tx_q_lock);
          qmi_csi_xport_disconnect(xp->xport, &addr);
	  LIST_FIND(xp->client_list, client, link,
	    (client->addr.node_id == addr.node_id && client->addr.port_id == addr.port_id));
	  if(client)
	  {
	      LIST_REMOVE(xp->client_list, client, link);
	      FREE(client);
	  }
        }
        if (rx_ctl_msg.cmd == QRTR_TYPE_BYE) {
	  struct conn_cli *client, *tmp;
	  client = LIST_HEAD(xp->client_list);
	  while (client)
	  {
	    tmp = client;
	    client = client->link.next;
	    if(addr.node_id == tmp->addr.node_id)
	    {
	      /* Purge the Tx queue */
	      LOCK(&xp->tx_q_lock);
	      dest = find_tx_q(xp, &tmp->addr);
	      if (dest)
	        purge_tx_q(xp, dest);
	      UNLOCK(&xp->tx_q_lock);
	      qmi_csi_xport_disconnect(xp->xport, &tmp->addr);
	      LIST_REMOVE(xp->client_list, tmp, link);
	      FREE(tmp);
	    }
	  }
	}
      }
      else if ((src_addr_size == sizeof(struct sockaddr_qrtr)) && (rx_len == 0x0))
      {
        //TODO Arun: need to check how Qrouter send resume_Tx
        QMI_FW_LOGE("%s: QCSI Received Resume_Tx from %08x:%08x on FD- %d\n",
                      __func__, addr.node_id, addr.port_id, xp->fd);
        handle_resume_tx(xp, &addr);
      }
      else if(rx_len > 0)
      {
        qmi_csi_xport_recv(xp->xport, &addr, buf, rx_len);
	add_conn_cli(xp, &addr);
      }
      else
      {
        break;
      }
    } while(rx_len >= 0);
    QMI_CSI_LOGD("xport_handle_event[%d]\n", xp->fd);
    free(buf);
  }
}

static void xport_close
(
 void *handle
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;

  /* Purge the TX queue */
  purge_dest_s(xp);
  purge_conn_cli(xp);

  QMI_CSI_LOGD("xport_close[%d]\n", xp->fd);
  close(xp->fd);
  qmi_csi_xport_closed(xp->xport);
  free(xp);
}

static uint32_t xport_addr_len
(
 void
 )
{
  return sizeof(struct xport_qrtr_addr);
}

qmi_csi_xport_ops_type qcsi_qrtr_ops = {
  NULL, /* Legacy Open */
  xport_reg,
  xport_unreg,
  NULL, /* Legacy Send */
  xport_handle_event,
  xport_close,
  xport_addr_len,
  xport_open,
  xport_send
};

