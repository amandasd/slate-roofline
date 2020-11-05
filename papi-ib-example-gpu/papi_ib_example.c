#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <papi.h>

int main(int argc, char *argv[])
{
   int event_set = PAPI_NULL;
   int event_count = 8;
   long long values[event_count]; 
   char *events[] = {
                    "infiniband:::mlx5_0_1:port_xmit_data",
		    "infiniband:::mlx5_0_1:port_rcv_data",
                    "infiniband:::mlx5_2_1:port_xmit_data",
		    "infiniband:::mlx5_2_1:port_rcv_data",
                    "infiniband:::mlx5_4_1:port_xmit_data",
		    "infiniband:::mlx5_4_1:port_rcv_data",
                    "infiniband:::mlx5_6_1:port_xmit_data",
		    "infiniband:::mlx5_6_1:port_rcv_data",
		    };

   int nodes, cpn;
   if (argc < 3)
   {       
      // Run the test on this many nodes.
      nodes = 2;
      // Run this many ranks per node in the test.
      cpn = 40;
   }
   else
   {
      nodes = atoi(argv[1]);
      cpn = atoi(argv[2]);
   }
 
   int numprocs, myid;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &myid);

   // Initialize PAPI
   PAPI_library_init(PAPI_VER_CURRENT);
   PAPI_create_eventset(&event_set);
   int code = 0;
   for (int i = 0; i < event_count; i++)
   {
      PAPI_event_name_to_code(events[i], &code);
      PAPI_add_event(event_set, code);
   }

   //StartRecordAriesCounters
   PAPI_start(event_set);

   PAPI_read(event_set, values);
   for(int id=0; id<nodes; id++) {
      if(myid == id * cpn) {
         printf("\n");
         for(int i = 0; i < event_count; i++) {
            printf("Before data transfer --> Rank: %d; %s: %lld\n", myid, events[i], values[i]);
         }
         printf("\n");
      }
   }

   int niter = 1;

   int size = 1000*numprocs;
   float inmsg[size/numprocs], outmsg[size];
   for(int i = 0; i < size; i++) { outmsg[i] = i; }

   MPI_Barrier(MPI_COMM_WORLD);
   for(int i = 0; i < niter; i++) {
      MPI_Scatter(outmsg,size/numprocs,MPI_FLOAT,inmsg,size/numprocs,MPI_FLOAT,0,MPI_COMM_WORLD);
      MPI_Gather(inmsg,size/numprocs,MPI_FLOAT,outmsg,size/numprocs,MPI_FLOAT,0,MPI_COMM_WORLD);
   }
   MPI_Barrier(MPI_COMM_WORLD);
   
   //int size = 10000;
   //float inmsg[size], outmsg[size];
   //for(int i = 0; i < size; i++) { outmsg[i] = i; }

   //MPI_Status recv_status;

   //MPI_Barrier(MPI_COMM_WORLD);
   //if(myid == 0) {
   //   for( int i = 0; i < niter; i++) {
   //       MPI_Send(&outmsg, size, MPI_FLOAT, 1, i, MPI_COMM_WORLD);
   //   }
   //   MPI_Recv(&inmsg, size, MPI_FLOAT, 1, 100, MPI_COMM_WORLD, &recv_status);
   //}
   //else if(myid == 1) {
   //   for( int i = 0; i < niter; i++) {
   //      MPI_Recv(&inmsg, size, MPI_FLOAT, 0, i, MPI_COMM_WORLD, &recv_status);
   //   }
   //   MPI_Send(&outmsg, size, MPI_FLOAT, 0, 100, MPI_COMM_WORLD);
   //}
   //MPI_Barrier(MPI_COMM_WORLD);

   //EndRecordAriesCounters
   PAPI_stop(event_set, values);
   PAPI_reset(event_set);

   for(int id=0; id<nodes; id++) {
      if(myid == id * cpn) {
	 printf("\n");
         for(int i = 0; i < event_count; i++) {
            printf("After data transfer --> Rank: %d; %s: %lld\n", myid, events[i], values[i]);
         }
	 printf("\n");
      }
   }

   // Cleanup papi
   PAPI_cleanup_eventset(event_set);
   PAPI_destroy_eventset(&event_set);
   PAPI_shutdown();

   MPI_Finalize();

   return 0;
}
