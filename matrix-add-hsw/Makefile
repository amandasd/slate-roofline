CC      = cc 
CFLAGS	= -O3 -Wall -g

#INCLUDE_DIR = -I../AriesNCL/src
#LIBRARY_DIR = -L../AriesNCL/src
#LIBRARIES = -lmpich -lpapi -lariesncl 

all: matrix_add_inv
#all: matrix_add matrix_add_inv

#matrix_add: matrix_add.c
	#${CC} ${CFLAGS} matrix_add.c -o matrix_add -DPROFILING
	#${CC} ${CFLAGS} matrix_add.c -o matrix_add_aries ${INCLUDE_DIR} ${LIBRARY_DIR} ${LIBRARIES} -DARIES -DPROFILING
	#${CC} ${CFLAGS} matrix_add.c -o matrix_add_advisor -DPROFILING

matrix_add_inv: matrix_add_inv.c
	#module load papi/6.0.0.4
	${CC} ${CFLAGS} matrix_add_inv.c -o matrix_add_inv_papi -DPROFILING -DPAPI
	${CC} ${CFLAGS} matrix_add_inv.c -o matrix_add_inv -DPROFILING 

clean: 
	rm -f matrix_add matrix_add_aries matrix_add_advisor matrix_add_inv matrix_add_inv_papi

reallyclean:
	rm -f matrix_add matrix_add_aries matrix_add_advisor matrix_add_inv matrix_add_inv_papi *.json *.bin
