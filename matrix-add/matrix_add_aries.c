#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include "AriesCounters.h"

int matrix_vt_create(int nlin, int ncol, float *m)
{
  for(int i=0; i < nlin; i++)
    for(int j=0; j < ncol; j++)
      m[j+i*ncol] = i + j*2;
  return(0);
}

int matrix_vt_add(int ncol, float *m, float *n, float *r)
{
  for(int i=0; i < ncol; i++) {
    r[i] = m[i] + n[i];
  }
  return(0);
}

void matrix_vt_print(int nlin, int ncol, float *m)
{  
  for(int i=0; i < nlin; i++) {
    fprintf(stderr,"Linha %d: ",i+1);
    for(int j=0; j < ncol; j++)
      fprintf(stderr,"%.3f ",m[j+i*ncol]);
    fprintf(stderr,"\n");
  }
}

int main(int argc, char **argv)
{
  float *matrix_A, 
        *matrix_B;

  int  size, rank;
  int  nlin, ncol;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //BEGIN ARIES
  int AC_event_set;
  char** AC_events;
  long long * AC_values;
  int AC_event_count;

  int nodes,
      cpn;

  if (argc == 1)
  {       
     nlin = 8192;
     ncol = 8192;
     // Run the test on this many nodes.
     nodes = 8;
     // Run this many ranks per node in the test.
     cpn = 32;
  }
  else
  {
     nlin = atoi(argv[1]);
     ncol = atoi(argv[1]);
     nodes = atoi(argv[2]);
     cpn = atoi(argv[3]);
  }

  // Since we only want to do a gather on every n'th rank, we need to create a new MPI_Group
  MPI_Group mod_group, group_world;
  MPI_Comm mod_comm;
  int members[nodes];
  for (int id=0; id<nodes; id++)
  {
      members[id] = id * cpn;
  }
  MPI_Comm_group(MPI_COMM_WORLD, &group_world);
  MPI_Group_incl(group_world, nodes, members, &mod_group);
  MPI_Comm_create(MPI_COMM_WORLD, mod_group, &mod_comm);

  int len_nodes = snprintf(NULL, 0, "%d", nodes);
  int len_ncol  = snprintf(NULL, 0, "%d", ncol);
  int len_cpn   = snprintf(NULL, 0, "%d", cpn);
  size_t len = strlen(argv[0]) + len_nodes + len_ncol + len_cpn + 4; /* + 1 for terminating NULL */
  char *filename = (char*)malloc(len);
  snprintf(filename, len, "%s-%d-%d-%d", argv[0],nodes,ncol,size/nodes);

  InitAriesCounters(filename, rank, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
  //END ARIES

  if(rank == 0) {
     matrix_A = malloc(nlin*ncol*sizeof(float));
     if(matrix_A == NULL) {
        fprintf(stderr,"malloc does not work (Matrix A)\n");
        return 1; 
     }
     matrix_vt_create(nlin,ncol,matrix_A);

     matrix_B = malloc(nlin*ncol*sizeof(float));
     if(matrix_B == NULL) {
        fprintf(stderr,"malloc does not work (Matrix B)\n");
        return 1;
     }
     matrix_vt_create(nlin,ncol,matrix_B);
  }
  else {
     matrix_A = NULL;
     matrix_B = NULL;
  }

  float *line_A = malloc((nlin/size)*ncol*sizeof(float));
  if(line_A == NULL) {
     fprintf(stderr,"malloc does not work (line A)\n");
     return 1;
  }
  float *line_B = malloc((nlin/size)*ncol*sizeof(float));
  if(line_B == NULL) {
     fprintf(stderr,"malloc does not work (line B)\n");
     return 1;
  }

  //if(rank == 0) {
  //   matrix_vt_print(nlin,ncol,matrix_A);
  //   matrix_vt_print(nlin,ncol,matrix_B);
  //}

  MPI_Status recv_status;
  int niter = 100;

  StartRecordAriesCounters(rank, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
  MPI_Barrier(MPI_COMM_WORLD);

  for(int i = 0; i < niter; i++) {
     if(rank == 0) {
        for(int r = 1; r < size; r++) { 
           MPI_Send(matrix_A+r*(nlin/size)*ncol,(nlin/size)*ncol,MPI_FLOAT,r,r,MPI_COMM_WORLD);
           MPI_Send(matrix_B+r*(nlin/size)*ncol,(nlin/size)*ncol,MPI_FLOAT,r,size+r,MPI_COMM_WORLD);
        }
     }
     else {
        MPI_Recv(line_A,(nlin/size)*ncol,MPI_FLOAT,0,rank,MPI_COMM_WORLD,&recv_status);
        MPI_Recv(line_B,(nlin/size)*ncol,MPI_FLOAT,0,size+rank,MPI_COMM_WORLD,&recv_status);
     }
     if(rank == 0) {
        matrix_vt_add((nlin/size)*ncol,matrix_A,matrix_B,matrix_A);
     }
     else {
        matrix_vt_add((nlin/size)*ncol,line_A,line_B,line_A);
     }
     if(rank == 0) {
        for(int r = 1; r < size; r++) { 
           MPI_Recv(matrix_A+r*(nlin/size)*ncol,(nlin/size)*ncol,MPI_FLOAT,r,2*size+r,MPI_COMM_WORLD,&recv_status);
        }
     }
     else {
        MPI_Send(line_A,(nlin/size)*ncol,MPI_FLOAT,0,2*size+rank,MPI_COMM_WORLD);
     }
  }

  //MPI_Scatter(matrix_A,(nlin/size)*ncol,MPI_FLOAT,line_A,(nlin/size)*ncol,MPI_FLOAT,0,MPI_COMM_WORLD);
  //MPI_Scatter(matrix_B,(nlin/size)*ncol,MPI_FLOAT,line_B,(nlin/size)*ncol,MPI_FLOAT,0,MPI_COMM_WORLD);
  //for(int i = 0; i < niter; i++) {
  //   matrix_vt_add((nlin/size)*ncol,line_A,line_B,line_A);
  //   MPI_Gather(line_A,(nlin/size)*ncol,MPI_FLOAT,matrix_A,(nlin/size)*ncol,MPI_FLOAT,0,MPI_COMM_WORLD);
  //}

  MPI_Barrier(MPI_COMM_WORLD);
  EndRecordAriesCounters(rank, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
  FinalizeAriesCounters(&mod_comm, rank, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);

  if(rank == 0) {
     double sum = 0.;
     for(int i=0; i < nlin; i++)
       for(int j=0; j < ncol; j++)
         sum = sum + matrix_A[j+i*ncol];
     fprintf(stderr,"Sum of Matrix: %lf\n",sum);
     //matrix_vt_print(nlin,ncol,matrix_A);
  }

  if(rank == 0) {
     free(matrix_A);
     free(matrix_B);
  }

  free(line_A);
  free(line_B);

  MPI_Finalize();
  return 0;
}
