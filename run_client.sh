#!/bin/bash

export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/grpc-install/lib:/public/home/lifei/xinzk/toolbox/grpc-install/lib64:$LD_LIBRARY_PATH
gdb --args ./rdma-client $1 # >xclient_rdma_log.log 2>xclient-rdma.log && cat client_rdma_log.log

# export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/grpc-old-install/lib:/lib64:/public/home/lifei/xinzk/toolbox/protoc/lib:${LD_LIBRARY_PATH}
# GRPC_TRACE=all GRPC_VERBOSITY=DEBUG ./rdma-client-old $1 >client_old_log.log 2>client-old.log && cat client_old_log.log

# export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/grpc-tcp-install/lib:/public/home/lifei/xinzk/toolbox/grpc-tcp-install/lib64:$LD_LIBRARY_PATH
# GRPC_TRACE=all GRPC_VERBOSITY=DEBUG ./client $1 >xclient_tcp_log.log 2>xclient-tcp.log && cat client_tcp_log.log 