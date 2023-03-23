#!/bin/bash
allip=($(hostname -I))

ip=${allip[1]}
echo $ip
export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/grpc-install/lib:/public/home/lifei/xinzk/toolbox/grpc-install/lib64:$LD_LIBRARY_PATH
../grpc_fs_rdma_server $ip:50051 >server_rdma_log.log 2>server-rdma.log #GRPC_TRACE=all GRPC_VERBOSITY=DEBUG

# export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/protoc/lib:/public/home/lifei/xinzk/toolbox/grpc-old-install/lib:${LD_LIBRARY_PATH}
# ip=${allip[1]}
# echo $ip
# GRPC_TRACE=all GRPC_VERBOSITY=DEBUG ../grpc_fs_old_server $ip:50051 >server_old_log.log 2>server-old.log

# ip=${allip[0]}
# echo $ip
# export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/grpc-tcp-install/lib:/public/home/lifei/xinzk/toolbox/grpc-tcp-install/lib64:$LD_LIBRARY_PATH
# ../grpc_fs_server $ip:50051 >server_tcp_log.log 2>server-tcp.log #GRPC_TRACE=all GRPC_VERBOSITY=DEBUG