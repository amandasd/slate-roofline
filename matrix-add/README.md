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

### Matrix Addition

```
cd matrix-add
make
```

### Example on Cori to obtain the AriesNCL data

```
salloc --qos=interactive -C haswell --time=60 --nodes=8

cd matrix-add
srun -N 8 -n 256 -c 2 --cpu_bind=cores ./matrix_add_aries 8192 8192 8 32
```
number of nodes (N): 8 

number of MPI ranks (n): 256

number of rows in the matrix: 8192

number of columns in the matrix: 8192

number of nodes: 8 

number of MPI ranks per node: 32

### Printing AriesNCL data

```
cd matrix-add
python ../scripts/print_counters.py -N 8 -f matrix_add_aries-8-8192-32.counters.0.bin
```

### Example on Cori to obtain the Roofline data

```
module load advisor
```

### Advisor with a single MPI rank

```
salloc --qos=interactive -C haswell --time=60 --nodes=1

cd matrix-add
srun -N 1 -n 1 -c 1 advixe-cl --collect survey --project-dir matrix_advisor-1-1-32x8192  -- ./matrix_add_advisor 32 8192
srun -N 1 -n 1 -c 1 advixe-cl --collect tripcounts -enable-cache-simulation -flop --project-dir matrix_advisor-1-1-32x8192 -- ./matrix_add_advisor 32 8192
```

number of nodes (N): 1

number of MPI ranks (n): 1

number of rows in the matrix: 32

number of columns in the matrix: 8192

### Advisor with multiple MPI ranks

```
salloc --qos=interactive -C haswell --time=60 --nodes=2

cd matrix-add
srun -N 2 -n 8 -c 2 --cpu_bind=cores ./script.sh

where script.sh is:

#!/bin/bash
if [ $SLURM_PROCID -eq 0 ]
then
   advixe-cl --collect survey --project-dir matrix_advisor-2-4-8192x8192  -- ./matrix_add_advisor 8192 8192
else
   ./matrix_add_advisor 8192 8192
fi
```

### Printing Roofline data

```
cd scripts
python print_roofline.py --project-dir matrix_advisor-2-4-8192x8192
```

