CC      = cc 
CFLAGS	= -O2 -Wall -g

INCLUDE_DIR = -I../lib/AriesNCL/src
LIBRARY_DIR = -L../lib/AriesNCL/src
LIBRARIES = -lmpich -lpapi -lariesncl

all: pt2pt

pt2pt: pt2pt.c
	${CC} ${CFLAGS} pt2pt.c -o pt2pt ${INCLUDE_DIR} ${LIBRARY_DIR} ${LIBRARIES}

clean: 
	rm -f pt2pt 

reallyclean:
	rm -f pt2pt *.json *.bin
