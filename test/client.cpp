#include <iostream>
// #include "rdma_helper.h"
#include "rdma_message.h"
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define TIMEOUT_IN_MS 2000
void on_addr_resolved(struct rdma_cm_id *id, struct resources *res) {
  struct ibv_qp_init_attr qp_init_attr;
  struct ibv_comp_channel *recv_comp_channel, *send_comp_channel;
  size_t size;
  int i;
  int mr_flags = 0;
  int cq_size = 0;
  int num_devices;
  res->ib_ctx=id->verbs;
  //创建保护域
  res->pd = ibv_alloc_pd(res->ib_ctx);
  cq_size = 1;
  send_comp_channel = ibv_create_comp_channel(id->verbs);
  recv_comp_channel = ibv_create_comp_channel(id->verbs);
  // 创建完成队列
  res->send_cq = ibv_create_cq(res->ib_ctx, cq_size, send_comp_channel, nullptr, 0);
  res->recv_cq = ibv_create_cq(res->ib_ctx, cq_size, recv_comp_channel, nullptr, 0);
  res->cq = ibv_create_cq(res->ib_ctx, cq_size, nullptr, nullptr, 0);
  ibv_req_notify_cq(res->send_cq, 0);
  ibv_req_notify_cq(res->recv_cq, 0);
  int flags;
  flags = fcntl(send_comp_channel->fd, F_GETFL);
  fcntl(send_comp_channel->fd, F_SETFL, flags | O_NONBLOCK); //set non-blocking
  flags = fcntl(recv_comp_channel->fd, F_GETFL);
  fcntl(recv_comp_channel->fd, F_SETFL, flags | O_NONBLOCK); //set non-blocking
  res->send_comp_channel=send_comp_channel;
  res->recv_comp_channel=recv_comp_channel;
  size = kMemSize;
  res->buf = (char *) malloc(size);
  memset(res->buf, 0, size);
  mr_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE;
  res->mr = ibv_reg_mr(res->pd, res->buf, size, mr_flags);
  memset(&qp_init_attr, 0, sizeof(qp_init_attr));
  qp_init_attr.qp_type = IBV_QPT_RC;
  // qp_init_attr.send_cq = res->send_cq;
  // qp_init_attr.recv_cq = res->recv_cq;
  qp_init_attr.send_cq = res->cq;
  qp_init_attr.recv_cq = res->cq;
  qp_init_attr.cap.max_send_wr = 1;
  qp_init_attr.cap.max_recv_wr = 1;
  qp_init_attr.cap.max_send_sge = 1;
  qp_init_attr.cap.max_recv_sge = 1;
  //分配队列对给id
  rdma_create_qp(id,res->pd,&qp_init_attr);
  res->qp = id->qp;

  struct ibv_recv_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;
  memset(&sge, 0, sizeof(sge));
  memset(&wr, 0, sizeof(wr));
  wr.next = nullptr;
  wr.wr_id = (uintptr_t)res;
  wr.sg_list = &sge;
  wr.num_sge = 1;

  sge.addr = (uintptr_t) res->buf;
  sge.length = kMemSize;
  sge.lkey = res->mr->lkey;
  id->context=res;
  ibv_post_recv(res->qp, &wr, &bad_wr);
  rdma_resolve_route(id,TIMEOUT_IN_MS);
}

void on_connected(struct rdma_cm_id *id, struct resources *res){
  std::cout << "connected " << std::endl;
  struct ibv_cq *cq=res->cq;
  struct ibv_send_wr sr;
  struct ibv_sge sge;
  struct ibv_send_wr *bad_wr = nullptr;
  struct ibv_wc wc;
  std::string data = "client client!";
  memcpy(res->buf, data.c_str(), data.size());
  memset(&sge, 0, sizeof(sge));
  sge.addr = (uintptr_t) res->buf;
  sge.length = kMemSize;
  sge.lkey = res->mr->lkey;
  memset(&sr, 0, sizeof(sr));
  sr.next = nullptr;
  sr.wr_id = (uintptr_t)id->context;
  sr.sg_list = &sge;
  sr.num_sge = 1;
  sr.opcode = IBV_WR_SEND;
  sr.send_flags = IBV_SEND_SIGNALED;
  ibv_post_send(res->qp, &sr, &bad_wr);
  std::cout << "ibv_post_send" << std::endl;
  void *ctx;
  ibv_get_cq_event(res->send_comp_channel, &cq, &ctx);
  ibv_ack_cq_events(cq, 1);
  ibv_req_notify_cq(cq, 0);
  while (ibv_poll_cq(cq, 1, &wc))
      std::cout << "wc.opcode " << wc.opcode << std::endl;
}

void on_disconnected(struct rdma_cm_id *id, struct resources *res){
  rdma_destroy_qp(id);
  ibv_dereg_mr(res->mr); 
  free(res->buf);
  ibv_dealloc_pd(res->pd);
  ibv_destroy_cq(res->send_cq);
  ibv_destroy_cq(res->recv_cq);
  rdma_destroy_id(id);
}

using namespace std;

int main(){
    struct rdma_event_channel *event_channel;
    struct rdma_cm_id *id;
    struct rdma_conn_param cm_params;
    int fd;
    struct resources resources_;
    memset(&resources_, 0, sizeof(resources_));
    // Create an event channel
    event_channel = rdma_create_event_channel();
    rdma_create_id(event_channel,&id,NULL,RDMA_PS_TCP);
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    inet_pton(AF_INET, "12.8.10.72", &addr.sin_addr);
    rdma_resolve_addr(id, NULL, (struct sockaddr *)&addr, 2000);

    if (!event_channel) {
    perror("rdma_create_event_channel");
    exit(1);
    }

    // Get the file descriptor associated with the event channel
    // fd = event_channel->fd;
    // Wait for events on the event channel
    struct rdma_event_channel *ec = event_channel;
    struct rdma_cm_event *event;
  while (true) {

    // Get the next event on the event channel
    rdma_get_cm_event(ec, &event);
    id = event->id;
    // Process the event
    switch (event->event) {
      case RDMA_CM_EVENT_ADDR_RESOLVED:
        // Address resolution completed successfully
        std::cout << "Address resolution completed successfully" << std::endl;
        on_addr_resolved(id,&resources_);
        rdma_ack_cm_event(event);
        break;
      case RDMA_CM_EVENT_ROUTE_RESOLVED:
        // Route resolution completed successfully
        std::cout << "Route resolution completed successfully" << std::endl;
        memset(&cm_params, 0, sizeof(cm_params));
        rdma_connect(id, &cm_params);
        rdma_ack_cm_event(event);
        break;
      case RDMA_CM_EVENT_CONNECT_REQUEST:
        // Received a connection request from a remote peer
        std::cout << "Received a connection request from a remote peer" << std::endl;
        rdma_ack_cm_event(event);
        break;
      case RDMA_CM_EVENT_ESTABLISHED:
        // Connection has been established
        std::cout << "Connection has been established" << std::endl;
        on_connected(id,&resources_);
        rdma_ack_cm_event(event);
        break;
      case RDMA_CM_EVENT_DISCONNECTED:
        // Connection has been disconnected
        std::cout << "Connection has been disconnected" << std::endl;
        on_disconnected(id,&resources_);
        rdma_ack_cm_event(event);
        return 0;
      default:
        // Other events ...
        break;
    }

    // Acknowledge the event
    // if (rdma_ack_cm_event(event) < 0) {
    //   perror("rdma_ack_cm_event");
    //   exit(1);
    // }
    
  }
    return 0;
}
