CC      = mpicc 
FLAGS	= -O0 -g 

PAPI_INCLUDE_OPTS= -I/global/common/software/m1759/papi/papi-6.0.0/include
PAPI_POST_LINK_OPTS= -L/global/common/software/m1759/papi/papi-6.0.0/lib -lpapi -lpfm

all: papi_ib_example

papi_ib_example: papi_ib_example.c
	${CC} ${FLAGS} papi_ib_example.c -o papi_ib_example ${PAPI_INCLUDE_OPTS} ${PAPI_POST_LINK_OPTS}

clean: 
	rm -f papi_ib_example

