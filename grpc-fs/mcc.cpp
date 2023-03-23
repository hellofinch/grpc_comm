#include <iostream>

int main(){
    enum ibv_wc_opcode {
	IBV_WC_SEND,
	IBV_WC_RDMA_WRITE,
	IBV_WC_RDMA_READ,
	IBV_WC_COMP_SWAP,
	IBV_WC_FETCH_ADD,
	IBV_WC_BIND_MW,
	IBV_WC_LOCAL_INV,
	IBV_WC_TSO,
	IBV_WC_FLUSH,
	IBV_WC_ATOMIC_WRITE = 9,
/*
 * Set value of IBV_WC_RECV so consumers can test if a completion is a
 * receive by testing (opcode & IBV_WC_RECV).
 */
	IBV_WC_RECV			= 1 << 7,
	IBV_WC_RECV_RDMA_WITH_IMM,

	IBV_WC_TM_ADD,
	IBV_WC_TM_DEL,
	IBV_WC_TM_SYNC,
	IBV_WC_TM_RECV,
	IBV_WC_TM_NO_TAG,
	IBV_WC_DRIVER1,
	IBV_WC_DRIVER2,
	IBV_WC_DRIVER3,
};
ibv_wc_opcode x=IBV_WC_RECV;
ibv_wc_opcode x1=IBV_WC_SEND;
ibv_wc_opcode x2=IBV_WC_RDMA_READ;
ibv_wc_opcode x3=IBV_WC_TM_SYNC;
    std::cout << x << " " << x1 << " " << x2 << " " << x3 << std::endl;
    return 0;
}