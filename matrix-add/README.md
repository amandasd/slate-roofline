### Modules

```
module load papi
```

You may also need to unload the darshan module.

### Environment variables

 ```
PAT_RT_PERFCTR_FILE=configurable_counters
```

### AriesNCL

```
cd lib/AriesNCL/src
make
```

### Matrix Addition

```
cd matrix-add
make
```

### Example on Cori

```
salloc --qos=interactive -C haswell --time=120 --nodes=8

cd matrix-add
srun -N 8 -n 256 --cpu_bind=cores ./matrix 8192 8 32
```
number of nodes (N): 8 

number of MPI ranks (n): 256

matrix size: 8192x8192

number of nodes: 8 

number of MPI ranks per node: 32





