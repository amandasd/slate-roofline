CC      = mpicc 
FF      = mpifort
FLAGS	= -O0 -g 

PAPI_INCLUDE_OPTS= -I/global/common/software/m1759/papi/papi-20201116hacked/include
PAPI_POST_LINK_OPTS= -L/global/common/software/m1759/papi/papi-20201116hacked/lib -lpapi -lpfm
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/global/common/software/m1759/papi/papi-20201116hacked/lib

all: papi_ib_example

papi_ib_example: papi_ib_example.c
	#module purge; module load cgpu gcc/8.3.0 cuda/11.0.3 openmpi/4.0.3
	${CC} ${FLAGS} papi_ib_example.c -o papi_ib_example ${PAPI_INCLUDE_OPTS} ${PAPI_POST_LINK_OPTS}
	${FF} ${FLAGS} papi_ib_example.f90 -o papi_ib_example-f ${PAPI_INCLUDE_OPTS} ${PAPI_POST_LINK_OPTS}

clean: 
	rm -f papi_ib_example papi_ib_example-f



