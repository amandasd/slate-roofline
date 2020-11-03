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

   int numprocs, myid;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &myid);

   int size;
   if (argc < 2)
   {       
      size = 10000*numprocs;
   }
   else
   {
      size = atoi(argv[1]);
   }

   float inmsg[size/numprocs], outmsg[size];
   for(int i = 0; i < size; i++) { outmsg[i] = 1.; }

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

   int niter = 1;

   PAPI_start(eventSet);

   MPI_Barrier(MPI_COMM_WORLD);
   for(int i = 0; i < niter; i++) {
      MPI_Scatter(outmsg,size/numprocs,MPI_FLOAT,inmsg,size/numprocs,MPI_FLOAT,0,MPI_COMM_WORLD);
      MPI_Gather(inmsg,size/numprocs,MPI_FLOAT,outmsg,size/numprocs,MPI_FLOAT,0,MPI_COMM_WORLD);
   }
   MPI_Barrier(MPI_COMM_WORLD);

   PAPI_stop(eventSet, values);
   //PAPI_reset(eventSet);
   PAPI_destroy_eventset(&eventSet);
   //PAPI_shutdown();

   // print description of each event
   /* print results: event values and descriptions */
   if (myid == 0) {
       // print event values at each process/rank
       printf("\nPOST WORK EVENT VALUES (Rank, Event Name, List of Event Values, Description of Events)>>>\n");
       int eventX;
       for (eventX = 0; eventX < eventNum; eventX++) {
           //printf("\tEvent --> %d/%d> %s --> ", eventX, eventNum, eventNames[eventX]);
           //printf("%lld --> %s\n", values[eventX], description[eventX]);
           printf("Rank(%d): %lld --> %s --> %s\n", myid, values[eventX], eventNames[eventX], description[eventX]);
       }
   }

   MPI_Finalize();

   return 0;
}