#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timemory-interface/interface.h"

int 
matrix_vt_add(int num, float *m, float *n, float *r)
{
    for(int i = 0; i < num; i++) {
      r[i] = m[i] + n[i];
    }
    return 0;
}

int 
matrix_vt_create(int nlin, int ncol, float *m, int rank)
{
    for(int i = 0; i < nlin; i++)
        for(int j = 0; j < ncol; j++)
            m[j+i*ncol] = i + (nlin*rank) + j*2;
    return 0;
}

#ifdef PROFILING
void 
matrix_vt_print(int nlin, int ncol, float *m)
{  
    for(int i = 0; i < nlin; i++) {
        for(int j = 0; j < ncol; j++)
            fprintf(stderr, "%.3f ", m[j+i*ncol]);
        fprintf(stderr, "\n");
    }
}
#endif

int 
main(int argc, char **argv) 
{
    set_papi_events(event_count, events);

    int  size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    push_region("main");
    int  nlin, ncol;
    if (argc < 3)
    {       
         nlin = 8;
         ncol = 8;
    }
    else
    {
         nlin = atoi(argv[1]);
         ncol = atoi(argv[2]);
    }
    
    float *matrix_C = (float*)malloc(nlin*ncol*sizeof(float));
    if(matrix_C == NULL) {
         fprintf(stderr, "Error in Matrix C allocation.\n");
         return 1; 
    }
    
    float *vec_A, *vec_B, *vec_C;
    vec_A = (float*)malloc((nlin / size)*ncol*sizeof(float));
    if(vec_A == NULL) {
         fprintf(stderr, "Error in vector A allocation.\n");
         return 1;
    }
    vec_B = (float*)malloc((nlin / size)*ncol*sizeof(float));
    if(vec_B == NULL) {
         fprintf(stderr, "Error in vector B allocation.\n");
         return 1;
    }
    vec_C = (float*)malloc((nlin / size)*ncol*sizeof(float));
    if(vec_C == NULL) {
         fprintf(stderr, "Error in vector C allocation.\n");
         return 1;
    }
    
    if(matrix_vt_create(nlin / size, ncol, vec_A, rank)) {
         fprintf(stderr, "Error in vector A creation.\n");
         return 1; 
    }
    if(matrix_vt_create(nlin / size, ncol, vec_B, rank)) {
         fprintf(stderr, "Error in vector B creation.\n");
         return 1; 
    }
    
    MPI_Datatype rowtype;
    MPI_Type_contiguous(ncol, MPI_FLOAT, &rowtype);
    MPI_Type_commit(&rowtype);
    
    int niter = 1;
    
#ifdef PROFILING
    double t_start = 0.0, t_end = 0.0, t = 0.0;
    t_start = MPI_Wtime();
#endif
    push_region("profiling"); 

    for(int i = 0; i < niter; i++) {
        if(matrix_vt_add((nlin / size)*ncol, vec_A, vec_B, vec_C)) {
            fprintf(stderr, "Rank %d: Error in addition operation.\n", rank);
            return 1;
        }
    }

    // Non-collective MPI routines
    // MPI_Status recv_status;
    // MPI_Request send_status;
    // for(int r = 0; r < size; r++) {
    //    if(r != rank) {
    //       MPI_Isend(vec_C, (nlin/size), rowtype, r, rank*100+r, MPI_COMM_WORLD, &send_status);
    //       MPI_Recv(matrix_C+r*(nlin/size)*ncol, (nlin/size), rowtype, r, r*100+rank, MPI_COMM_WORLD, &recv_status);
    //       MPI_Wait(&send_status, &recv_status);
    //    }
    // }
    // for(int j = rank*(nlin/size)*ncol; j < (nlin/size)*ncol; j++) {
    //    matrix_C[j] = vec_C[j];
    // }
    
    // Collective MPI routines
    MPI_Allgather(vec_C, (nlin / size), rowtype, matrix_C, (nlin / size), rowtype,
                  MPI_COMM_WORLD); 

    MPI_Barrier(MPI_COMM_WORLD);
    pop_region("profiling"); 

#ifdef PROFILING
    MPI_Barrier(MPI_COMM_WORLD);
    t_end = MPI_Wtime();
    t     = t_end - t_start;
#endif
    
#ifdef PROFILING
    if(rank == 0) {
        // long double sum = 0.;
        // for(int i=0; i < nlin; i++)
        //   for(int j=0; j < ncol; j++)
        //     sum = sum + matrix_C[j+i*ncol];
        // fprintf(stderr, "Sum of all elements of the matrix: %Lf\n", sum);
        fprintf(stderr, "Time: %lf\n", t);
        // matrix_vt_print(nlin, ncol, matrix_C);
        // matrix_vt_print(nlin/size, ncol, vec_A);
    }
#endif
    
    // Release host memory
    free(matrix_C);
    MPI_Type_free(&rowtype);
    free(vec_A);
    free(vec_B);
    free(vec_C);

    pop_region("main");   
    finalize();
    MPI_Finalize();
     
    return 0;
}
