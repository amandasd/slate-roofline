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
srun -N 2 -n 4 --cpu_bind=cores ./pzheevd_aries 1024 2 2 256
```
