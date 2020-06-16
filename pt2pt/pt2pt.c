#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "AriesCounters.h"

int main (int argc, char *argv[])
{
    int myid, numprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    //BEGIN ARIES
	int AC_event_set;
	char** AC_events;
	long long * AC_values;
	int AC_event_count;
  
	int nodes;
	// Run the test on this many nodes.
	nodes = 2;
	int cpn;
	// Run this many ranks per node in the test.
	cpn = 2;
	
	// Since we only want to do a gather on every n'th rank, we need to create a new MPI_Group
	MPI_Group mod_group, group_world;
	MPI_Comm mod_comm;
	int members[nodes];
	int rank;
	for (rank=0; rank<nodes; rank++)
	{
	    members[rank] = rank * cpn;
	}
	MPI_Comm_group(MPI_COMM_WORLD, &group_world);
	MPI_Group_incl(group_world, nodes, members, &mod_group);
	MPI_Comm_create(MPI_COMM_WORLD, mod_group, &mod_comm);

	//int myrank;
	//MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	InitAriesCounters(argv[0], myid, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
    //END ARIES

    int i, j, k, count;

    int size = 100000;
    char inmsg[size], outmsg[size];
    for(int i = 0; i < size; i++) {
        outmsg[i] = 'a';
        inmsg[i] = 'b';
    }

    MPI_Status recv_status;

    StartRecordAriesCounters(myid, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
    MPI_Barrier(MPI_COMM_WORLD);
    for(k = 0; k < 1; k++) {
    	if(myid == 0) {
            MPI_Recv(&inmsg, size, MPI_CHAR, 2, 100, MPI_COMM_WORLD, &recv_status);
            MPI_Get_count(&recv_status,MPI_CHAR,&count);
            printf("Rank %d: received %d int(s) from rank %d with tag %d.\n",myid,count,recv_status.MPI_SOURCE,recv_status.MPI_TAG);

            MPI_Recv(&inmsg, size, MPI_CHAR, 3, 101, MPI_COMM_WORLD, &recv_status);
            MPI_Get_count(&recv_status,MPI_CHAR,&count);
            printf("Rank %d: received %d int(s) from rank %d with tag %d.\n",myid,count,recv_status.MPI_SOURCE,recv_status.MPI_TAG);
    	}
	else if(myid == 1) {
            MPI_Recv(&inmsg, size, MPI_CHAR, 2, 102, MPI_COMM_WORLD, &recv_status);
            MPI_Get_count(&recv_status,MPI_CHAR,&count);
            printf("Rank %d: received %d int(s) from rank %d with tag %d.\n",myid,count,recv_status.MPI_SOURCE,recv_status.MPI_TAG);

            MPI_Recv(&inmsg, size, MPI_CHAR, 3, 103, MPI_COMM_WORLD, &recv_status);
            MPI_Get_count(&recv_status,MPI_CHAR,&count);
            printf("Rank %d: received %d int(s) from rank %d with tag %d.\n",myid,count,recv_status.MPI_SOURCE,recv_status.MPI_TAG);
    	}
	else if(myid == 2) {
            MPI_Send(&outmsg, size, MPI_CHAR, 1, 102, MPI_COMM_WORLD);
            MPI_Send(&outmsg, size, MPI_CHAR, 0, 100, MPI_COMM_WORLD);
	}
	else if(myid == 3) {
            MPI_Send(&outmsg, size, MPI_CHAR, 1, 103, MPI_COMM_WORLD);
            MPI_Send(&outmsg, size, MPI_CHAR, 0, 101, MPI_COMM_WORLD);
	    //sleep(20);
	}
    }
    MPI_Barrier(MPI_COMM_WORLD);
    EndRecordAriesCounters(myid, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);

    FinalizeAriesCounters(&mod_comm, myid, cpn, &AC_event_set, &AC_events, &AC_values, &AC_event_count);
    MPI_Finalize();

    return EXIT_SUCCESS;
}
