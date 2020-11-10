## Modules

```
module purge
module load cgpu gcc/8.3.0 cuda/11.0.3 openmpi/4.0.3
```

## Environment variables

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/global/common/software/m1759/papi/papi-6.0.0/lib
```

## Matrix Addition

```
cd matrix-add-gpu
make
```

## Example on Cori GPU to obtain the PAPI infiniband counters

```
salloc -C gpu -N 2 -t 15 -A m1759 --exclusive -q special

srun -N 2 -n 16 -G 16 -c 2 --cpu_bind=cores ./matrix_add 8192 8192 2 8

number of nodes (N): 2 

number of MPI ranks (n): 16

number of GPUS (G): 16

number of rows in the matrix: 8192

number of columns in the matrix: 8192

number of nodes: 2 

number of MPI ranks per node: 8
```


