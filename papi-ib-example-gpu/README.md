## Modules

```
module purge
module load cgpu gcc/8.3.0 cuda/11.0.3 openmpi/4.0.3
```

## Environment variables

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/global/common/software/m1759/papi/papi-6.0.0/lib
```

## PAPI infiniband counters on Cori GPU

```
cd papi-ib-example-gpu
make

salloc -C gpu -N 2 -t 15 -A m1759 --exclusive -q special

srun -N 2 -n 2 -c 1 --cpu_bind=cores ./papi_example 2&> output 

cat output | grep 'mlx5_0_1:port_xmit_data\|mlx5_2_1:port_xmit_data\|mlx5_4_1:port_xmit_data\|mlx5_6_1:port_xmit_data' | grep "Rank: 0"
cat output | grep 'mlx5_0_1:port_xmit_data\|mlx5_2_1:port_xmit_data\|mlx5_4_1:port_xmit_data\|mlx5_6_1:port_xmit_data' | grep "Rank: 1"
cat output | grep 'mlx5_0_1:port_rcv_data\|mlx5_2_1:port_rcv_data\|mlx5_4_1:port_rcv_data\|mlx5_6_1:port_rcv_data' | grep "Rank: 0"
cat output | grep 'mlx5_0_1:port_rcv_data\|mlx5_2_1:port_rcv_data\|mlx5_4_1:port_rcv_data\|mlx5_6_1:port_rcv_data' | grep "Rank: 1"
```
