// #define RDMA_MSG_CONTENT_SIZE 32767
// #define INIT_RECV_BUFFER_SIZE (sizeof(rdma_memory_region))
// #include <stddef.h>
// typedef struct{
//   int msg_info;
//   size_t msg_len;
//   char msg_content[RDMA_MSG_CONTENT_SIZE];
// } rdma_message;
// typedef struct{
//   int msg_info;
//   size_t msg_len;
//   char msg_content[2];
// } rdma_smessage;//In order to keep the same structure of rdma_message
// typedef struct{
//   rdma_message msg;
//   rdma_smessage sms;
// } rdma_memory_region;
// typedef struct _rdma_mem_node rdma_mem_node;
// typedef struct _rdma_mem_manager rdma_mem_manager;
// struct _rdma_mem_node{
//   rdma_memory_region context;
//   struct ibv_mr *mr;
//   rdma_mem_node *next,*prev;
//   rdma_mem_manager *manager;
// };
// struct _rdma_mem_manager{
//   rdma_mem_node *nil;
//   int refs;
//   int node_count;//for debugging
// };

#include <rdma/rdma_cma.h>
#include <fcntl.h>
#include <rdma/rdma_verbs.h>

const uint64_t kMemSize = 1 * 1024 * 1024 * 1024;

struct resources {
  // struct ibv_device_attr device_attr;
  // /* Device attributes */
  // struct ibv_port_attr port_attr;       /* IB port attributes */
  // struct cm_con_data_t remote_props; /* values to connect to remote side */
  struct ibv_context *ib_ctx;           /* device handle */
  struct ibv_pd *pd;                   /* PD handle */
  struct ibv_cq *send_cq, *recv_cq, *cq;    /* CQ handle */
  struct ibv_qp *qp;                   /* QP handle */
  struct ibv_mr *mr;                   /* MR handle for buf */
  struct ibv_comp_channel *recv_comp_channel, *send_comp_channel, *comp_channel;
  /* memory buffer pointer, used for RDMA and send ops */
  char *buf;
  int sock;                           /* TCP socket file descriptor */
};