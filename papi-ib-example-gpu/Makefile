CC      = mpicc 
FLAGS	= -g 

PAPI_INCLUDE_OPTS= -I/global/common/software/m1759/papi/papi-6.0.0/include
PAPI_POST_LINK_OPTS= -L/global/common/software/m1759/papi/papi-6.0.0/lib -lpapi -lpfm

all: papi_example

papi_example: papi_example.c
	gcc -c test_utils.c ${PAPI_INCLUDE_OPTS}
	${CC} ${FLAGS} papi_example.c test_utils.o -o papi_example ${PAPI_INCLUDE_OPTS} ${PAPI_POST_LINK_OPTS}

clean: 
	rm -f papi_example

