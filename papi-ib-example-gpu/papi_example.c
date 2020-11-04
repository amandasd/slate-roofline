#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <papi.h>
#include "papi_test.h"
#include <mpi.h>

int main(int argc, char **argv)
{
   int eventSet = PAPI_NULL;
   int eventCount = 0;
   int eventNum = 0;
   int IB_ID = -1;
   char eventNames[500][500];
   char description[500][500];
   long long values[500]; 
   int eventX;

   int numprocs, myid;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &myid);

   /* Set TESTS_QUIET variable */
   tests_quiet( argc, argv );

   /* PAPI Initialization */
   int retval = PAPI_library_init( PAPI_VER_CURRENT );
   if ( retval != PAPI_VER_CURRENT ) {
       test_fail(__FILE__, __LINE__,"PAPI_library_init failed\n",retval);
   }

   /* Get total number of components detected by PAPI */ 
   int NumComponents = PAPI_num_components(); 

   /* Check if infiniband component exists */
   const PAPI_component_info_t *cmpInfo = NULL;
   for(int ComponentID = 0; ComponentID < NumComponents; ComponentID++) {

       if((cmpInfo = PAPI_get_component_info(ComponentID)) == NULL) {
           test_fail(__FILE__, __LINE__,"PAPI_get_component_info failed\n",-1);
           continue;
       }

       if(strcmp(cmpInfo->name, "infiniband") != 0) { //It does not match
           continue;
       }

       if (!TESTS_QUIET) {
           printf("Component %d (%d) - %d events - %s\n", ComponentID, cmpInfo->CmpIdx, cmpInfo->num_native_events, cmpInfo->name);
       }

       if(cmpInfo->disabled) {
           test_skip(__FILE__,__LINE__,"Component infiniband is disabled", 0);
           break;
       }

       eventCount = cmpInfo->num_native_events;
       IB_ID = ComponentID;
       break;
   }

   /* if we did not find any valid events, just skip the test. */
   if(eventCount == 0) {
       test_skip(__FILE__,__LINE__,"No infiniband events found", 0);
   }

   int size = 10000;

   float outmsg[size], inmsg[size];
   for(int i = 0; i < size; i++) { outmsg[i] = i; }

   PAPI_event_info_t eventInfo; 

   PAPI_create_eventset(&eventSet);

   /* find the code for first event in component */
   int code = PAPI_NATIVE_MASK;
   int r = PAPI_enum_cmp_event(&code, PAPI_ENUM_FIRST, IB_ID);

   /* add each event individually in the eventSet and measure event values. */
   /* for each event, repeat work with different data sizes. */
   while (r == PAPI_OK) {

      // attempt to add event to event set 
      PAPI_add_event(eventSet, code);
      /* get event name of added event */
      PAPI_event_code_to_name(code, eventNames[eventNum]);
      /* get long description of added event */
      PAPI_get_event_info(code, &eventInfo);
      strncpy(description[eventNum], eventInfo.long_descr, sizeof(description[0])-1);
      description[eventNum][sizeof(description[0])-1] = '\0';

      eventNum++;
      
      r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, IB_ID);
   }

   MPI_Status recv_status;

   int niter = 10;

   PAPI_start(eventSet);

   PAPI_read(eventSet, values);

   for (eventX = 0; eventX < eventNum; eventX++) {
       printf("Before data transfer --> Rank: %d; %s: %lld\n", myid, eventNames[eventX], values[eventX]);
   }

   MPI_Barrier(MPI_COMM_WORLD);
   if(myid == 0) {
      for( int i = 0; i < niter; i++) {
          MPI_Send(&outmsg, size, MPI_FLOAT, 1, i, MPI_COMM_WORLD);
      }
      MPI_Recv(&inmsg, size, MPI_FLOAT, 1, 100, MPI_COMM_WORLD, &recv_status);
   }
   else if(myid == 1) {
      MPI_Send(&outmsg, size, MPI_FLOAT, 0, 100, MPI_COMM_WORLD);
      for( int i = 0; i < niter; i++) {
         MPI_Recv(&inmsg, size, MPI_FLOAT, 0, i, MPI_COMM_WORLD, &recv_status);
      }
   }
   MPI_Barrier(MPI_COMM_WORLD);

   PAPI_stop(eventSet, values);
   //PAPI_reset(eventSet);
   //PAPI_destroy_eventset(&eventSet);
   //PAPI_shutdown();

   for (eventX = 0; eventX < eventNum; eventX++) {
       printf("After data transfer -->  Rank: %d; %s: %lld\n", myid, eventNames[eventX], values[eventX]);
   }

   MPI_Finalize();

   return 0;
}
