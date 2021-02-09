#include <cuda.h>
#include <cuda_runtime.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PAPI
#    include <papi.h>
#endif

#include "interface.h"

__global__ void
MatAdd(float* A, float* B, float* C, int n, int taskperItem)
{
    // Get our global thread ID
    int global_id = blockIdx.x * blockDim.x * taskperItem + threadIdx.x;
    for(int t = 0; t < taskperItem; t++)
    {
        int i = t * blockDim.x + global_id;
        // Make sure we do not go out of bounds
        if(i < n)
            C[i] = A[i] + B[i];
    }
}

int
matrix_vt_create(int nlin, int ncol, float* m, int rank)
{
    for(int i = 0; i < nlin; i++)
        for(int j = 0; j < ncol; j++)
            m[j + i * ncol] = i + (nlin * rank) + j * 2;
    return 0;
}

#ifdef PROFILING
void
matrix_vt_print(int nlin, int ncol, float* m)
{
    for(int i = 0; i < nlin; i++)
    {
        for(int j = 0; j < ncol; j++)
            fprintf(stderr, "%.3f ", m[j + i * ncol]);
        fprintf(stderr, "\n");
    }
}
#endif

int
main(int argc, char** argv)
{
    int         event_count = 9;
    const char* events[]    = { "infiniband:::mlx5_0_1_ext:port_xmit_data",
                             "infiniband:::mlx5_0_1_ext:port_rcv_data",
                             "infiniband:::mlx5_2_1_ext:port_xmit_data",
                             "infiniband:::mlx5_2_1_ext:port_rcv_data",
                             "infiniband:::mlx5_4_1_ext:port_xmit_data",
                             "infiniband:::mlx5_4_1_ext:port_rcv_data",
                             "infiniband:::mlx5_6_1_ext:port_xmit_data",
                             "infiniband:::mlx5_6_1_ext:port_rcv_data" };

#ifdef PAPI
    int event_set = PAPI_NULL;
    long long values[event_count];
#else
    set_papi_events(event_count, events);
#endif

    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    initialize(&argc, &argv);

    push_region("main");
    int nlin, ncol;
    if(argc < 3)
    {
        nlin = 8;
        ncol = 8;
    }
    else
    {
        nlin = atoi(argv[1]);
        ncol = atoi(argv[2]);
    }

#ifdef PAPI
    int nodes, cpn;
    if(argc < 5)
    {
        // Run the test on this many nodes.
        nodes = 2;
        // Run this many ranks per node in the test.
        cpn = 40;
    }
    else
    {
        nodes = atoi(argv[3]);
        cpn   = atoi(argv[4]);
    }
#endif

    float* matrix_C = (float*) malloc(nlin * ncol * sizeof(float));
    if(matrix_C == NULL)
    {
        fprintf(stderr, "Error in Matrix A allocation.\n");
        return 1;
    }

    // Host input and output vectors
    // Allocate memory for each vector on host
    float *vec_A, *vec_B, *vec_C;
#ifdef PINNED
    cudaError_t status =
        cudaMallocHost((void**) &vec_A, (nlin / size) * ncol * sizeof(float));
    if(status != cudaSuccess)
    {
        fprintf(stderr, "Error in pinned vector A allocation.\n");
        return 1;
    }
    status = cudaMallocHost((void**) &vec_B, (nlin / size) * ncol * sizeof(float));
    if(status != cudaSuccess)
    {
        fprintf(stderr, "Error in pinned vector B allocation.\n");
        return 1;
    }
    status = cudaMallocHost((void**) &vec_C, (nlin / size) * ncol * sizeof(float));
    if(status != cudaSuccess)
    {
        fprintf(stderr, "Error in pinned vector C allocation.\n");
        return 1;
    }
#else
    vec_A = (float*) malloc((nlin / size) * ncol * sizeof(float));
    if(vec_A == NULL)
    {
        fprintf(stderr, "Error in vector A allocation.\n");
        return 1;
    }
    vec_B = (float*) malloc((nlin / size) * ncol * sizeof(float));
    if(vec_B == NULL)
    {
        fprintf(stderr, "Error in vector B allocation.\n");
        return 1;
    }
    vec_C = (float*) malloc((nlin / size) * ncol * sizeof(float));
    if(vec_C == NULL)
    {
        fprintf(stderr, "Error in vector C allocation.\n");
        return 1;
    }
#endif

    if(matrix_vt_create(nlin / size, ncol, vec_A, rank))
    {
        fprintf(stderr, "Error in vector A creation.\n");
        return 1;
    }
    if(matrix_vt_create(nlin / size, ncol, vec_B, rank))
    {
        fprintf(stderr, "Error in vector B creation.\n");
        return 1;
    }

#ifdef PAPI
    PAPI_library_init(PAPI_VER_CURRENT);
    PAPI_create_eventset(&event_set);
    int code = 0;
    for(int i = 0; i < event_count; i++)
    {
        PAPI_event_name_to_code(events[i], &code);
        PAPI_add_event(event_set, code);
    }
#endif

    // Device input and output vectors
    float *pA, *pB, *pC;
    // Allocate memory for each vector on device
    cudaMalloc((void**) &pA, ((nlin / size) * ncol) * sizeof(float));
    cudaMalloc((void**) &pB, ((nlin / size) * ncol) * sizeof(float));
    cudaMalloc((void**) &pC, ((nlin / size) * ncol) * sizeof(float));

    MPI_Datatype rowtype;
    MPI_Type_contiguous(ncol, MPI_FLOAT, &rowtype);
    MPI_Type_commit(&rowtype);

    int niter = 100;

    int nblocks, blockSize, taskperItem;
    taskperItem = 1;
    // Number of threads in each block
    if(argc < 6)
    {
        blockSize = 1024;
    }
    else
    {
        blockSize = min(1024, (int) (pow(2.0f, (int) ceil(log2((float) atoi(argv[5]))))));
    }
    // Number of blocks; number max is 65535
    nblocks = (int) ceil((float) ((nlin / size) * ncol) / blockSize);
    if(nblocks > 65535)
    {
        fprintf(stderr, "Number of blocks is higher than 65535!\n");
    }

#ifdef PROFILING
    double t_start = 0.0, t_end = 0.0, t = 0.0;
    t_start = MPI_Wtime();
#endif
    push_region("profiling");

#ifdef PAPI
    PAPI_start(event_set);
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    // Copy host vectors to device
    cudaMemcpy(pA, vec_A, ((nlin / size) * ncol) * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(pB, vec_B, ((nlin / size) * ncol) * sizeof(float), cudaMemcpyHostToDevice);

    for(int i = 0; i < niter; i++)
    {
        // Execute the kernel
        MatAdd<<<nblocks, blockSize / taskperItem>>>(pA, pB, pC, (nlin / size) * ncol,
                                                     taskperItem);
    }

    // Copy array back to host
    cudaMemcpy(vec_C, pC, ((nlin / size) * ncol) * sizeof(float), cudaMemcpyDeviceToHost);

    // Non-collective MPI routines
    // MPI_Status recv_status;
    // MPI_Request send_status;
    // for(int r = 0; r < size; r++) {
    //   if(r != rank) {
    //      MPI_Isend(vec_C,(nlin/size),rowtype,r,rank*100+r,MPI_COMM_WORLD,&send_status);
    //      MPI_Recv(matrix_C+r*(nlin/size)*ncol,(nlin/size),rowtype,r,r*100+rank,MPI_COMM_WORLD,&recv_status);
    //      MPI_Wait(&send_status, &recv_status);
    //   }
    //}
    // for(int j = rank*(nlin/size)*ncol; j < (nlin/size)*ncol; j++) {
    //   matrix_C[j] = vec_C[j];
    //}

    // Collective MPI routine
    MPI_Allgather(vec_C, (nlin / size), rowtype, matrix_C, (nlin / size), rowtype,
                  MPI_COMM_WORLD);

#ifdef PAPI
    MPI_Barrier(MPI_COMM_WORLD);
    PAPI_stop(event_set, values);
#endif

    pop_region("profiling");

#ifdef PROFILING
    MPI_Barrier(MPI_COMM_WORLD);
    t_end = MPI_Wtime();
    t     = t_end - t_start;
#endif

#ifdef PAPI
    for(int id = 0; id < nodes; id++)
    {
        if(rank == id * cpn)
        {
            printf("\n");
            long long xmit = 0, rcv = 0;
            for(int i = 0; i < event_count; i++)
            {
                if(strstr(events[i], "xmit"))
                {
                    xmit = xmit + values[i];
                }
                else if(strstr(events[i], "rcv"))
                {
                    rcv = rcv + values[i];
                }
            }
            printf("node %d -> %lld sent bytes\n", id, xmit);
            printf("node %d -> %lld received bytes\n", id, rcv);
        }
    }
#endif

#ifdef PROFILING
    if(rank == 0)
    {
        // long double sum = 0.;
        // for(int i=0; i < nlin; i++)
        //  for(int j=0; j < ncol; j++)
        //    sum = sum + matrix_C[j+i*ncol];
        // fprintf(stderr,"Sum of all elements of the matrix: %Lf\n",sum);
        fprintf(stderr, "Time: %lf\n", t);
        // matrix_vt_print(nlin,ncol,matrix_C);
    }
#endif

    // Release host memory
    free(matrix_C);

    // Release host memory
    MPI_Type_free(&rowtype);
#ifdef PINNED
    cudaFreeHost(vec_A);
    cudaFreeHost(vec_B);
    cudaFreeHost(vec_C);
#else
    free(vec_A);
    free(vec_B);
    free(vec_C);
#endif

    // Release device memory
    cudaFree(pA);
    cudaFree(pB);
    cudaFree(pC);

#ifdef PAPI
    PAPI_cleanup_eventset(event_set);
    PAPI_destroy_eventset(&event_set);
    PAPI_shutdown();
#endif

    pop_region("main");
    finalize();
    MPI_Finalize();

    return 0;
}
