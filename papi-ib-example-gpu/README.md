## Modules

```
module purge
module load cgpu gcc cuda openmpi/4.0.3
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

srun -N 2 -n 80 -c 1 --cpu_bind=cores ./papi_example 2&> output 
```
