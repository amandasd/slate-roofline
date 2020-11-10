#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <papi.h>

__global__ void MatAdd(float *A, float *B, float *C, int n)
{
  // Get our global thread ID
  int i = blockIdx.x*blockDim.x+threadIdx.x;
  // Make sure we do not go out of bounds
  if(i < n)
     C[i] = A[i] + B[i];
}

int matrix_vt_create(int nlin, int ncol, float *m)
{
  for(int i=0; i < nlin; i++)
    for(int j=0; j < ncol; j++)
      m[j+i*ncol] = i + j*2;
  return 0;
}

#ifdef PROFILING
void matrix_vt_print(int nlin, int ncol, float *m)
{  
  for(int i=0; i < nlin; i++) {
    for(int j=0; j < ncol; j++)
      fprintf(stderr,"%.3f ",m[j+i*ncol]);
    fprintf(stderr,"\n");
  }
}
#endif

int main(int argc, char **argv) {

int event_set = PAPI_NULL;
int event_count = 8;
long long values[event_count]; 
const char *events[] = {
                 "infiniband:::mlx5_0_1:port_xmit_data",
		 "infiniband:::mlx5_0_1:port_rcv_data",
                 "infiniband:::mlx5_2_1:port_xmit_data",
		 "infiniband:::mlx5_2_1:port_rcv_data",
                 "infiniband:::mlx5_4_1:port_xmit_data",
		 "infiniband:::mlx5_4_1:port_rcv_data",
                 "infiniband:::mlx5_6_1:port_xmit_data",
		 "infiniband:::mlx5_6_1:port_rcv_data"
		 };

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
//fprintf(stderr,"nlin(%d) ncol(%d)\n",nlin,ncol);

int nodes, cpn;
if (argc < 5)
{       
   // Run the test on this many nodes.
   nodes = 2;
   // Run this many ranks per node in the test.
   cpn = 40;
}
else
{
   nodes = atoi(argv[3]);
   cpn = atoi(argv[4]);
}
 
float *matrix_A, 
      *matrix_B;

int  size, rank;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &size);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//fprintf(stderr,"Rank(%d) Size(%d)\n",rank,size);

if(rank == 0) {
   matrix_A = (float*)malloc(nlin*ncol*sizeof(float));
   if(matrix_A == NULL) {
      fprintf(stderr,"Error in Matrix A allocation.\n");
      return 1; 
   }
   if(matrix_vt_create(nlin,ncol,matrix_A)) {
      fprintf(stderr,"Error in Matrix A creation.\n");
      return 1; 
   }

   matrix_B = (float*)malloc(nlin*ncol*sizeof(float));
   if(matrix_B == NULL) {
      fprintf(stderr,"Error in Matrix B allocation.\n");
      return 1;
   }
   if(matrix_vt_create(nlin,ncol,matrix_B)) {
      fprintf(stderr,"Error in Matrix B creation.\n");
      return 1; 
   }
}
else {
   matrix_A = NULL;
   matrix_B = NULL;
}

//Host input and output vectors
//Allocate memory for each vector on host
float *vec_A = (float*)malloc((nlin/size)*ncol*sizeof(float));
if(vec_A == NULL) {
   fprintf(stderr,"Error in vector A allocation.\n");
   return 1;
}
float *vec_B = (float*)malloc((nlin/size)*ncol*sizeof(float));
if(vec_B == NULL) {
   fprintf(stderr,"Error in vector B allocation.\n");
   return 1;
}
float *vec_C = (float*)malloc((nlin/size)*ncol*sizeof(float));
if(vec_C == NULL) {
   fprintf(stderr,"Error in vector C allocation.\n");
   return 1;
}

// Initialize PAPI
PAPI_library_init(PAPI_VER_CURRENT);
PAPI_create_eventset(&event_set);
int code = 0;
for (int i = 0; i < event_count; i++)
{
   PAPI_event_name_to_code(events[i], &code);
   PAPI_add_event(event_set, code);
}

//Device input and output vectors
float *pA, *pB, *pC;
//Allocate memory for each vector on device
cudaMalloc((void**)&pA, ((nlin/size)*ncol)*sizeof(float));
cudaMalloc((void**)&pB, ((nlin/size)*ncol)*sizeof(float));
cudaMalloc((void**)&pC, ((nlin/size)*ncol)*sizeof(float));

MPI_Datatype rowtype;
MPI_Type_contiguous(ncol, MPI_FLOAT, &rowtype);
MPI_Type_commit(&rowtype);

//MPI_Status recv_status;
int niter = 1;

int nblocks, blockSize;
// Number of threads in each block
if(argc < 6)
{       
   blockSize = 1024;
}
else
{
   blockSize = min(1024,(int)(pow(2.0f,(int)ceil(log2((float)atoi(argv[5]))))));
}
// Number of blocks; number max is 65535
nblocks = (int)ceil((float)((nlin/size)*ncol)/blockSize);
//fprintf(stderr,"blockSize(%d) nblocks(%d)\n",blockSize,nblocks);
if(nblocks > 65535)
{
   fprintf(stderr,"Number of blocks is higher than 65535!\n");
   //return 0;
}

#ifdef PROFILING
double t_start = 0.0, t_end = 0.0, t = 0.0;
t_start = MPI_Wtime();
#endif

//StartRecordAriesCounters
PAPI_start(event_set);

MPI_Barrier(MPI_COMM_WORLD);
for(int i = 0; i < niter; i++) {
   //Initialize vectors on host
   MPI_Scatter(matrix_A,(nlin/size),rowtype,vec_A,(nlin/size),rowtype,0,MPI_COMM_WORLD);
   MPI_Scatter(matrix_B,(nlin/size),rowtype,vec_B,(nlin/size),rowtype,0,MPI_COMM_WORLD);
  
   //Copy host vectors to device
   cudaMemcpy(pA, vec_A, ((nlin/size)*ncol)*sizeof(float), cudaMemcpyHostToDevice);
   cudaMemcpy(pB, vec_B, ((nlin/size)*ncol)*sizeof(float), cudaMemcpyHostToDevice);
 
   //Execute the kernel
   MatAdd<<<nblocks, blockSize>>>(pA, pB, pC, (nlin/size)*ncol);
   
   //Copy array back to host
   cudaMemcpy(vec_C, pC, ((nlin/size)*ncol)*sizeof(float), cudaMemcpyDeviceToHost);
   
   MPI_Gather(vec_C,(nlin/size),rowtype,matrix_A,(nlin/size),rowtype,0,MPI_COMM_WORLD);
}
MPI_Barrier(MPI_COMM_WORLD);

PAPI_stop(event_set, values);
PAPI_reset(event_set);

#ifdef PROFILING
t_end = MPI_Wtime();
t = t_end - t_start;
#endif

for(int id=0; id<nodes; id++) {
   if(rank == id * cpn) {
      printf("\n");
      long long xmit = 0, rcv = 0;
      for(int i = 0; i < event_count; i++) {
	 if(strstr(events[i],"xmit")) {
            //printf("xmit... %s: %lld\n", events[i], values[i]);
	    xmit = xmit + values[i];
	 }
	 else if(strstr(events[i],"rcv")) {
            //printf("rcv... %s: %lld\n", events[i], values[i]);
	    rcv = rcv + values[i];
	 }
      }
      printf("node %d -> %lld sent bytes\n", id, xmit);
      printf("node %d -> %lld received bytes\n", id, rcv);
   }
}

#ifdef PROFILING
if(rank == 0) {
   long double sum = 0.;
   for(int i=0; i < nlin; i++)
     for(int j=0; j < ncol; j++)
       sum = sum + matrix_A[j+i*ncol];
   fprintf(stderr,"Sum of all elements of the matrix: %Lf\n",sum);
   fprintf(stderr,"Time: %lf\n",t);
   //matrix_vt_print(nlin,ncol,matrix_A);
}
#endif

if(rank == 0) {
   fprintf(stderr,"Matrix[0][1]: %.3f\n",matrix_A[1]);
   fprintf(stderr,"Matrix[nlin-1][ncol-2]: %.3f\n",matrix_A[(ncol-2)+(nlin-1)*ncol]);
}

//Release host memory
if(rank == 0) {
   free(matrix_A);
   free(matrix_B);
}

//Release host memory
MPI_Type_free(&rowtype);
free(vec_A);
free(vec_B);
free(vec_C);

//Release device memory
cudaFree(pA); 
cudaFree(pB); 
cudaFree(pC);

// Cleanup papi
PAPI_cleanup_eventset(event_set);
PAPI_destroy_eventset(&event_set);
PAPI_shutdown();

MPI_Finalize();
 
return 0;
}
