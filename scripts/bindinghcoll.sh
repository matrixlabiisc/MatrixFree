#!/bin/bash

case $(( ${SLURM_LOCALID}/1 )) in
0) UCX_NET_DEVICES=mlx5_0:1 HCOLL_MAIN_IB=mlx5_0:1 CUDA_VISIBLE_DEVICES=0 $* ;;
1) UCX_NET_DEVICES=mlx5_1:1 HCOLL_MAIN_IB=mlx5_1:1 CUDA_VISIBLE_DEVICES=1 $* ;;
2) UCX_NET_DEVICES=mlx5_2:1 HCOLL_MAIN_IB=mlx5_2:1 CUDA_VISIBLE_DEVICES=2 $* ;;
3) UCX_NET_DEVICES=mlx5_3:1 HCOLL_MAIN_IB=mlx5_3:1 CUDA_VISIBLE_DEVICES=3 $* ;;
4) UCX_NET_DEVICES=mlx5_6:1 HCOLL_MAIN_IB=mlx5_6:1 CUDA_VISIBLE_DEVICES=4 $* ;;
5) UCX_NET_DEVICES=mlx5_7:1 HCOLL_MAIN_IB=mlx5_7:1 CUDA_VISIBLE_DEVICES=5 $* ;;
6) UCX_NET_DEVICES=mlx5_8:1 HCOLL_MAIN_IB=mlx5_8:1 CUDA_VISIBLE_DEVICES=6 $* ;;
7) UCX_NET_DEVICES=mlx5_9:1 HCOLL_MAIN_IB=mlx5_9:1 CUDA_VISIBLE_DEVICES=7 $* ;;
esac
