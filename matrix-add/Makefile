CC      = cc 
CFLAGS	= -O2 -Wall -g

all: matrix_add_aries

matrix_add_aries: matrix_add_aries.c
	${CC} ${CFLAGS} matrix_add_aries.c -o matrix_add_aries -I../lib/AriesNCL/src -L../lib/AriesNCL/src -lmpich -lpapi -lariesncl
clean: 
	rm -f matrix_add_aries 

reallyclean:
	rm -f matrix_add_aries *.yaml *.bin
