### Modules

```
module load papi
```

You may also need to unload the darshan module.

### Environment variables

 ```
export OMP_NUM_THREADS=1

PAT_RT_PERFCTR_FILE=configurable_counters
```

### AriesNCL

```
cd lib/AriesNCL/src
make
```

### Pt2pt

```
cd pt2pt
make
```

### Example on Cori

```
salloc --qos=interactive -C haswell --time=10 --nodes=2

cd pt2pt
srun -N 2 -n 4 ./pt2pt
```
number of nodes (N): 2 

number of MPI ranks (n): 4
