#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include "rdma_message.h"
#include <fcntl.h>

#define RDMA_CQ_SIZE 128
#define RDMA_POST_RECV_NUM 64

#define TIMEOUT_IN_MS 500
int on_addr_resolved(struct rdma_cm_id *id) {
  struct ibv_qp_init_attr qp_attr;

  struct ibv_pd *pd;
  struct ibv_cq *recv_cq, *send_cq;
  struct ibv_comp_channel *recv_comp_channel, *send_comp_channel;

  struct connect_context *context;
  //struct ibv_mr *recv_buffer_mr;
  struct ibv_mr *send_buffer_mr;
  //char *recv_buffer_region;
  char *send_buffer_region;

  struct ibv_recv_wr wr[RDMA_POST_RECV_NUM], *bad_wr = NULL;
  struct ibv_sge sge[RDMA_POST_RECV_NUM];

  uintptr_t uintptr_addr;
  int i;

  // 创建一个保护域 
  if ((pd = ibv_alloc_pd(id->verbs)) == NULL) {
    // gpr_log(GPR_ERROR, "Client: ibv_alloc_pd() failed: %s",strerror(errno));
    return -1;
  } 

  if ((send_comp_channel = ibv_create_comp_channel(id->verbs)) == NULL) { //FIXME
    // gpr_log(GPR_ERROR, "Client: ibv_create_comp_channel() failed: %s",strerror(errno));
    return -1;
  }
  if ((recv_comp_channel = ibv_create_comp_channel(id->verbs)) == NULL) { //FIXME
    // gpr_log(GPR_ERROR, "Client: ibv_create_comp_channel() failed: %s",strerror(errno));
    return -1;
  }

  if ((send_cq = ibv_create_cq(id->verbs, RDMA_CQ_SIZE, NULL, send_comp_channel, 0)) ==NULL) { //FIXME 
    // gpr_log(GPR_ERROR, "Client: ibv_create_cq() failed: %s",strerror(errno));
    return -1;
  }
  if ((recv_cq = ibv_create_cq(id->verbs, RDMA_CQ_SIZE, NULL, recv_comp_channel, 0)) ==NULL) { //FIXME 
    // gpr_log(GPR_ERROR, "Client: ibv_create_cq() failed: %s",strerror(errno));
    return -1;
  }

  if ((ibv_req_notify_cq(send_cq, 0)) != 0) {
    // gpr_log(GPR_ERROR, "Client: ibv_req_notify_cq() failed: %s",strerror(errno));
    return -1;
  }
  if ((ibv_req_notify_cq(recv_cq, 0)) != 0) {
    // gpr_log(GPR_ERROR, "Client: ibv_req_notify_cq() failed: %s",strerror(errno));
    return -1;
  }

  int flags;
  flags = fcntl(send_comp_channel->fd, F_GETFL);
  fcntl(send_comp_channel->fd, F_SETFL, flags | O_NONBLOCK); //set non-blocking
  flags = fcntl(recv_comp_channel->fd, F_GETFL);
  fcntl(recv_comp_channel->fd, F_SETFL, flags | O_NONBLOCK); //set non-blocking


  //build_qp_attr(&qp_attr);
  //创建队列对
  memset(&qp_attr, 0, sizeof(qp_attr)); 
  qp_attr.send_cq = send_cq;
  qp_attr.recv_cq = recv_cq;
  qp_attr.qp_type = IBV_QPT_RC;
  qp_attr.cap.max_send_wr = RDMA_CQ_SIZE; //TODO
  qp_attr.cap.max_recv_wr = RDMA_CQ_SIZE;
  qp_attr.cap.max_send_sge = 1;
  qp_attr.cap.max_recv_sge = 1;

  if ((rdma_create_qp(id, pd, &qp_attr)) != 0) {
    // gpr_log(GPR_ERROR, "Client: rdma_create_qp() failed: %s",strerror(errno));
    return -1;
  }
//   gpr_log(GPR_INFO,"The actual size is(%d,%d)",qp_attr.cap.max_send_wr,qp_attr.cap.max_recv_wr);
  // 创建一个发送缓冲区
  send_buffer_region = (char*)malloc(INIT_RECV_BUFFER_SIZE);
  // for(i=0;i<RDMA_POST_RECV_NUM;++i){
  //   // 注册内存区域
  //   if((mr = ibv_reg_mr(
	// 	             pd,
	// 		     buf,//缓冲区指针
  //                            sizeof(rdma_memory_region),//缓冲区大小
  //                            IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE)
  //       ==NULL)) {
	// //   gpr_log(GPR_ERROR, "Client(%d): ibv_reg_mr() failed: %s",i,strerror(errno));
	//   return(-1);
	// }
  // }

  if((send_buffer_mr = ibv_reg_mr(
          pd,
          send_buffer_region,
          INIT_RECV_BUFFER_SIZE,
          IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE))
      == NULL) {
    // gpr_log(GPR_ERROR, "Client: ibv_reg_mr() failed: %s",strerror(errno));
    return -1;
  }

  if ((ibv_post_recv(id->qp, &(wr[0]), &bad_wr)) != 0){
    // gpr_log(GPR_ERROR, "Client: ibv_post_recv() failed: %s",strerror(errno));
    return -1;
  }
  if ((rdma_resolve_route(id, TIMEOUT_IN_MS)) != 0){
    // gpr_log(GPR_ERROR, "Client: rdma_resolve_route() failed: %s",strerror(errno));
    return -1;
  }
  std::cout << "on_address_resloved" << std::endl;
  return 0; 
}
