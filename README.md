# slate-roofline

## Build

### CMake Options

#### Options defaulting to ON

- `BUILD_SHARED_LIBS` : build shared libraries
- `USE_TIMEMORY` : enable timemory instrumentation
- `USE_PROFILING` : enable simple `MPI_Wtime` timer

#### Options defaulting to OFF

- `USE_CUDA` : build the matrix-add-gpu
- `USE_PAPI` : enable PAPI counters
- `USE_COMPILER_INSTRUMENTATION` : enable timemory's compiler instrumentation

```console
git clone <this-repo> slate-roofline-source
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${PWD/slate-roofline -DUSE_CUDA=ON -B build-slate-roofline slate-roofline-source
cmake --build build-slate-roofline --target install --parallel 4
```

## Run

The build installed `matrix-add-cpu`, `matrix-add-gpu`, and `matrix-add-gpu-pinned` in the
`slate-roofline/bin` folder.

```console
export PATH=${PWD}/slate-roofline/bin:${PATH}
srun -n 8 -c 1 matrix-add-cpu
srun -n 8 -c 1 matrix-add-gpu
srun -n 8 -c 1 matrix-add-gpu-pinned
```
