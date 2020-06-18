CC      = cc 
CFLAGS	= -O2 -Wall -g

all: matrix_add

matrix_add: matrix_add.c
	${CC} ${CFLAGS} matrix_add.c -o matrix_add_aries   -I../lib/AriesNCL/src -L../lib/AriesNCL/src -lmpich -lpapi -lariesncl -DARIES=1 -DPROFILING=1
	${CC} ${CFLAGS} matrix_add.c -o matrix_add_advisor -I../lib/AriesNCL/src -L../lib/AriesNCL/src -lmpich -lpapi -lariesncl -DARIES=0 -DPROFILING=0

clean: 
	rm -f matrix_add_aries matrix_add_advisor

reallyclean:
	rm -f matrix_add_aries matrix_add_advisor *.json *.bin
