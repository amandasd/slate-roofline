### Modules

```
module purge
module load PrgEnv-cray 
module load craype-haswell
module load papi
```

You may also need to unload the darshan module.

### AriesNCL

```
cd lib/AriesNCL/src
make
```

### Pzheevd

```
cd eigensolvers
make
```

### Example on Cori

```
salloc --qos=interactive -C haswell --time=120 --nodes=2

cd eigensolvers
srun -N 2 -n 4 --cpu_bind=cores ./pzheevd_aries 1024 2 2 256 2
```
number of nodes (N): 2 

number of MPI ranks (n): 4

matrix size: 1024

number of row ranks (nx): 2 

number of column ranks (ny): 2 

size of the blocks (nb): 256 

number of nodes: 2 



