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
```

```
srun -N 2 -n 2 -c 2 --cpu_bind=cores ./papi_ib_example 10000 2 1 2&>output 

Arguments:

vector size: 10000

number of nodes: 2

number of MPI ranks per node: 1
```

```
srun -N 2 -n 80 -c 2 --cpu_bind=cores ./papi_ib_example 100000 2 40 2&>output 

Arguments:

vector size: 100000

number of nodes: 2

number of MPI ranks per node: 40
```
