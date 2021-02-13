
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PAPI
#    include <papi.h>
#endif

#include "timemory-interface/interface.h"

int
matrix_vt_add(int num, float* m, float* n, float* r)
{
    for(int i = 0; i < num; i++)
    {
        r[i] = m[i] + n[i];
    }
    return 0;
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
    int         event_count = 5;
    const char* events[]    = { "AR_NIC_ORB_PRF_REQ_BYTES_SENT",
                             "AR_NIC_ORB_PRF_RSP_BYTES_RCVD",
                             "AR_NIC_RAT_PRF_REQ_BYTES_RCVD",
                             "AR_NIC_RSPMON_NPT_EVENT_CNTR_NL_FLITS",
                             "AR_NIC_RSPMON_NPT_EVENT_CNTR_NL_PKTS" };

    set_papi_events(event_count, events);

    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    initialize(&argc, &argv);

    push_region("main");
#ifdef PAPI
    int       event_set = PAPI_NULL;
    long long values[event_count];
#endif

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
    // fprintf(stderr,"nlin(%d) ncol(%d)\n",nlin,ncol);

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
    // fprintf(stderr,"Rank(%d) Size(%d)\n",rank,size);

    float* matrix_C = (float*) malloc(nlin * ncol * sizeof(float));
    if(matrix_C == NULL)
    {
        fprintf(stderr, "Error in Matrix C allocation.\n");
        return 1;
    }

    float *vec_A, *vec_B, *vec_C;
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

    MPI_Datatype rowtype;
    MPI_Type_contiguous(ncol, MPI_FLOAT, &rowtype);
    MPI_Type_commit(&rowtype);

    // MPI_Status recv_status;
    int niter = 1;

#ifdef PROFILING
    double t_start = 0.0, t_end = 0.0, t = 0.0;
    t_start = MPI_Wtime();
#endif

    push_region("profiling");

#ifdef PAPI
    PAPI_start(event_set);
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    for(int i = 0; i < niter; i++)
    {
        if(matrix_vt_add((nlin / size) * ncol, vec_A, vec_B, vec_C))
        {
            fprintf(stderr, "Rank %d: Error in addition operation.\n", rank);
            return 1;
        }
    }
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
    MPI_Allgather(vec_C, (nlin / size), rowtype, matrix_C, (nlin / size), rowtype,
                  MPI_COMM_WORLD);

#ifdef PAPI
    MPI_Barrier(MPI_COMM_WORLD);
    PAPI_stop(event_set, values);
    PAPI_reset(event_set);
#endif

    pop_region("profiling");

#ifdef PROFILING
    MPI_Barrier(MPI_COMM_WORLD);
    t_end = MPI_Wtime();
    t     = t_end - t_start;
#endif

#ifdef PAPI
    // printf("AR_NIC_ORB_PRF_RSP_BYTES_RCVD [rank %d]: %lld;
    // AR_NIC_RAT_PRF_REQ_BYTES_RCVD [rank %d]: %lld\n", rank, values[1], rank,
    // values[2]);
    for(int id = 0; id < nodes; id++)
    {
        if(rank == id * cpn)
        {
            printf("\n");
            printf("AR_NIC_ORB_PRF_REQ_BYTES_SENT [rank %d]: %lld; "
                   "AR_NIC_ORB_PRF_RSP_BYTES_RCVD [rank %d]: %lld; "
                   "AR_NIC_RAT_PRF_REQ_BYTES_RCVD [rank %d]: %lld; "
                   "AR_NIC_RSPMON_NPT_EVENT_CNTR_NL_FLITS [rank %d]: %lld; "
                   "AR_NIC_RSPMON_NPT_EVENT_CNTR_NL_PKTS [rank %d]: %lld; "
                   "AR_NIC_ORB_PRF_RSP_BYTES_SENT [rank %d]: %lld\n",
                   rank, values[0], rank, values[1], rank, values[2], rank, values[3],
                   rank, values[4], rank, (values[3] - values[4]) * 16);
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
        // matrix_vt_print(nlin/size,ncol,vec_A);
    }
#endif

    // if(rank == 0) {
    //   fprintf(stderr,"Matrix[0][1]: %.3f\n",matrix_C[1]);
    //   fprintf(stderr,"Matrix[nlin-1][ncol-2]:
    //   %.3f\n",matrix_C[(ncol-2)+(nlin-1)*ncol]);
    //}

    // Release host memory
    free(matrix_C);

    // Release host memory
    MPI_Type_free(&rowtype);
    free(vec_A);
    free(vec_B);
    free(vec_C);

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
