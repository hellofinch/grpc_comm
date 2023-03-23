#!/bin/bash
#SBATCH -J monitorrocm
#SBATCH --cpus-per-task=1
#SBATCH --tasks-per-node=32
#SBATCH --mem=100G
#SBATCH -N 2
#SBATCH -p normal
#SBATCH -o slurm.log
#SBATCH -e slurm.err

rm -f ./flag
echo "flag" >flag

nohup python -u test.py output.log 2>output_err.log &

nodes_array=($(scontrol show hostnames $SLURM_JOB_NODELIST)) # Getting the node names
((worker_num=$SLURM_JOB_NUM_NODES)) # Must be one less that the total number of nodes
touch output.json
echo "" > output.json
x=0
while [ -f flag ]
do
    sleep 1s
    for ((i=0 ; i<=worker_num))
    do
        value=$(ssh ${nodes_array[i]} "rocm-smi --json -u --showmemuse")
        echo "{${node_array[i]}: $value,count: $x}," >> output.json
    done
    x=x+1
done
