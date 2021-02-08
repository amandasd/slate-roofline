#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <papi.h>

int main(int argc, char *argv[])
{
   int event_set = PAPI_NULL;
   int event_count = 8;
   long long values[event_count]; 
   char *events[] = {
                    "infiniband:::mlx5_0_1_ext:port_xmit_data",
		    "infiniband:::mlx5_0_1_ext:port_rcv_data",
                    "infiniband:::mlx5_2_1_ext:port_xmit_data",
		    "infiniband:::mlx5_2_1_ext:port_rcv_data",
                    "infiniband:::mlx5_4_1_ext:port_xmit_data",
		    "infiniband:::mlx5_4_1_ext:port_rcv_data",
                    "infiniband:::mlx5_6_1_ext:port_xmit_data",
		    "infiniband:::mlx5_6_1_ext:port_rcv_data",
		    };

   int numprocs, myid;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &myid);

   int size, nodes, cpn;
   if (argc < 4)
   {       
      size = 10000*numprocs;
      // Run the test on this many nodes.
      nodes = 2;
      // Run this many ranks per node in the test.
      cpn = 40;
   }
   else
   {
      size = atoi(argv[1]);
      nodes = atoi(argv[2]);
      cpn = atoi(argv[3]);
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

   int niter = 1;

   float inmsg[size/numprocs], outmsg[size];
   for(int i = 0; i < size; i++) { outmsg[i] = i; }

   //StartRecordAriesCounters
   PAPI_start(event_set);

   MPI_Barrier(MPI_COMM_WORLD);
   for(int i = 0; i < niter; i++) {
      MPI_Scatter(outmsg,size/numprocs,MPI_FLOAT,inmsg,size/numprocs,MPI_FLOAT,0,MPI_COMM_WORLD);
      MPI_Gather(inmsg,size/numprocs,MPI_FLOAT,outmsg,size/numprocs,MPI_FLOAT,0,MPI_COMM_WORLD);
   }
   MPI_Barrier(MPI_COMM_WORLD);
   
   for(int id=0; id<nodes; id++) {
      if(myid == id * cpn) {
         //EndRecordAriesCounters
         PAPI_stop(event_set, values);
         printf("\n");
         long long xmit = 0, rcv = 0;
         for(int i = 0; i < event_count; i++) {
            if(strstr(events[i],"xmit")) {
               xmit = xmit + values[i];
            }
            else if(strstr(events[i],"rcv")) {
               rcv = rcv + values[i];
            }
         }
         printf("node %d -> %lld sent bytes\n", id, xmit);
         printf("node %d -> %lld received bytes\n", id, rcv);
      }
   }
   MPI_Barrier(MPI_COMM_WORLD);

   // Cleanup papi
   //PAPI_cleanup_eventset(event_set);
   //PAPI_destroy_eventset(&event_set);
   //PAPI_shutdown();

   //MPI_Finalize();

   return 0;
}
