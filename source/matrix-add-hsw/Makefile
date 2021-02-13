CC      = cc 
CFLAGS	= -O3 -Wall -g

#INCLUDE_DIR = -I../AriesNCL/src
#LIBRARY_DIR = -L../AriesNCL/src
#LIBRARIES = -lmpich -lpapi -lariesncl 

#PAPI_INCLUDE_OPTS= -I/global/common/software/m1759/papi/papi-20201116hacked/include
#PAPI_POST_LINK_OPTS= -L/global/common/software/m1759/papi/papi-20201116hacked/lib -lpapi
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/global/common/software/m1759/papi/papi-20201116hacked/lib

all: matrix_add_inv
#all: matrix_add matrix_add_inv

#matrix_add: matrix_add.c
	#${CC} ${CFLAGS} matrix_add.c -o matrix_add -DPROFILING
	#${CC} ${CFLAGS} matrix_add.c -o matrix_add_aries ${INCLUDE_DIR} ${LIBRARY_DIR} ${LIBRARIES} -DARIES -DPROFILING
	#${CC} ${CFLAGS} matrix_add.c -o matrix_add_advisor -DPROFILING

matrix_add_inv: matrix_add_inv.c
	#${CC} ${CFLAGS} matrix_add_inv.c -o matrix_add_inv_papi ${PAPI_INCLUDE_OPTS} ${PAPI_POST_LINK_OPTS} -DPROFILING -DPAPI
	#module load papi/6.0.0.4
	${CC} ${CFLAGS} matrix_add_inv.c -o matrix_add_inv_papi -DPROFILING -DPAPI
	#${CC} ${CFLAGS} matrix_add_inv.c -o matrix_add_inv_advisor
	${CC} ${CFLAGS} matrix_add_inv.c -o matrix_add_inv -DPROFILING 
	${CC} ${CFLAGS} matrix_add_inv.c -o matrix_add_inv_mpiP /global/common/software/m1759/mpip/install/mpip20191031/hsw/lib/libmpiP.a /global/common/software/m1759/mpip/install/binutils/binutils-2.33.1/lib/libbfd.a /global/common/software/m1759/mpip/install/binutils/binutils-2.33.1/lib/libiberty.a 

clean: 
	rm -f matrix_add_inv matrix_add_inv_papi matrix_add_inv_advisor

reallyclean:
	rm -f matrix_add_inv matrix_add_inv_papi matrix_add_inv_advisor *.json *.bin
