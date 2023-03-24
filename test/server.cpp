#include <iostream>
// #include "rdma_helper.h"
#include "rdma_message.h"
#include <cstdio>

using namespace std;

void on_connect_request(struct rdma_cm_id *id, struct resources *res){
  struct ibv_qp_init_attr qp_init_attr;
    struct rdma_conn_param cm_params;
  struct ibv_comp_channel *recv_comp_channel, *send_comp_channel;
  struct ibv_recv_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;
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
  memset(&sge, 0, sizeof(sge));
  memset(&wr, 0, sizeof(wr));
  sge.addr = (uintptr_t) res->buf;
  sge.length = kMemSize;
  sge.lkey = res->mr->lkey;
  wr.next = nullptr;
  wr.wr_id = 0;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  ibv_post_recv(res->qp, &wr, &bad_wr);
  rdma_accept(id, &cm_params);
}

void on_connected(struct rdma_cm_id *id, struct resources *res){
  struct ibv_cq *cq=res->cq;
  struct ibv_wc wc;
  // void *ctx;
  // unsigned events_completed=0;
  
  // struct ibv_send_wr sr, *bad_wr = nullptr;
  struct ibv_recv_wr rr, *bad_rr = NULL;
  struct ibv_sge sge;
  memset(&sge, 0, sizeof(sge));
  sge.addr = (uintptr_t) res->buf;
  sge.length = kMemSize;
  sge.lkey = res->mr->lkey;
  void *ctx;
  ibv_get_cq_event(res->recv_comp_channel, &cq, &ctx);
  ibv_ack_cq_events(cq, 1);
  ibv_req_notify_cq(cq, 0);
  ibv_poll_cq(cq,1,&wc);
    if(wc.status==IBV_WC_SUCCESS){
      // rdma_post_recv(id,NULL,res->buf,kMemSize,res->mr);
      std::cout << "mem in server: " << res->buf << std::endl;
      std::string data = "server server!";
      memcpy(res->buf, data.c_str(), data.size());
      std::cout << "mem in server: " << res->buf << std::endl;
  //     rdma_post_send(id,NULL,res->buf,kMemSize,res->mr,0);
    }


}

void on_disconnected(struct rdma_cm_id *id, struct resources *res){
  ibv_dereg_mr(res->mr); 
  ibv_dealloc_pd(res->pd);
  ibv_destroy_cq(res->send_cq);
  ibv_destroy_cq(res->recv_cq);
  free(res->buf);
  rdma_destroy_id(id);
}

int main(){
  struct rdma_event_channel *event_channel;
  struct rdma_cm_id *id;
  struct resources resources_;
  memset(&resources_, 0, sizeof(resources_));
  // Create an event channel
  event_channel = rdma_create_event_channel();
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);
  rdma_create_id(event_channel,&id,NULL,RDMA_PS_TCP);
  rdma_bind_addr(id,(struct sockaddr *)&addr);
  rdma_listen(id,10);
  // int flags = fcntl(event_channel->fd, F_GETFL);
  // fcntl(event_channel->fd, F_SETFL, flags | O_NONBLOCK); //set non-blocking
  // Wait for events on the event channel
  
    struct rdma_event_channel *ec = event_channel;
    struct rdma_cm_event *event;
  while(true){
    // Get the next event on the event channel
    int x=rdma_get_cm_event(ec, &event);
    id=event->id;
    // Process the event
    switch (event->event) {
      case RDMA_CM_EVENT_CONNECT_REQUEST:
        // Received a connection request from a remote peer
        std::cout << "Received a connection request from a remote peer" << std::endl;
        on_connect_request(id,&resources_);
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
        on_disconnected(id, &resources_);
        rdma_ack_cm_event(event);
        break;
      default:
        // Other events ...
        break;
    }

    // Acknowledge the event
  }
  return 0;
}
