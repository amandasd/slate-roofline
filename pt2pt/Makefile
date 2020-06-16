CC      = cc 
CFLAGS	= -O2 -Wall -g

all: pt2pt

pt2pt: pt2pt.c
	${CC} ${CFLAGS} pt2pt.c -o pt2pt -I../lib/AriesNCL/src -L../lib/AriesNCL/src -lmpich -lpapi -lariesncl
clean: 
	rm -f pt2pt 

reallyclean:
	rm -f pt2pt *.yaml *.bin
