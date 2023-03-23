#!/bin/bash
allip=($(hostname -I))

# ip=${allip[1]}
# echo $ip
# export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/grpc-install/lib:/public/home/lifei/xinzk/toolbox/grpc-install/lib64:$LD_LIBRARY_PATH
# GRPC_TRACE=all GRPC_VERBOSITY=DEBUG ./rdma-server $ip >server_rdma_log.log 2>server-rdma.log


# export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/protoc/lib:/public/home/lifei/xinzk/toolbox/grpc-old-install/lib:${LD_LIBRARY_PATH}
# ip=${allip[1]}
# echo $ip
# GRPC_TRACE=all GRPC_VERBOSITY=DEBUG ./rdma-server-old $ip >server_old_log.log 2>server-old.log

ip=${allip[0]}
echo $ip
export LD_LIBRARY_PATH=/public/home/lifei/xinzk/toolbox/grpc-tcp-install/lib:/public/home/lifei/xinzk/toolbox/grpc-tcp-install/lib64:$LD_LIBRARY_PATH
./route_guide_server --db_path=route_guide_db.json #>server_tcp_log.log 2>server-tcp.log GRPC_TRACE=all GRPC_VERBOSITY=DEBUG 