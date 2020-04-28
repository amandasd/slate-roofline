FC	= ftn
FFLAGS	= -h omp -O3 -g
#CC      = cc 
#CFLAGS	= -O3 -Wall -g

all: pzheevd_aries

pzheevd_aries: pzheevd_aries.f90
	#${CC} -c extern_c_functions.c -I../src -g
	#${FC} ${FFLAGS} pzheevd_aries.f90 extern_c_functions.o ../src/AriesCounters.o -o pzheevd_aries -I../src -L../src -I/opt/cray/pe/papi/5.7.0.2/include -L/opt/cray/pe/papi/5.7.0.2/lib -lariesncl -lpapi /opt/cray/pe/lib64/libmpich.so.3 /opt/cray/pe/lib64/libmpichf90_cray.so.3
	${FC} ${FFLAGS} pzheevd_aries.f90 ../lib/AriesNCL/src/AriesCounters.o -o pzheevd_aries -I../lib/AriesNCL/src -L../lib/AriesNCL/src -I/opt/cray/pe/papi/5.7.0.2/include -L/opt/cray/pe/papi/5.7.0.2/lib -lariesncl -lpapi /opt/cray/pe/lib64/libmpich.so.3 /opt/cray/pe/lib64/libmpichf90_cray.so.3

clean: 
	rm -f pzheevd_aries 

reallyclean:
	rm -f pzheevd_aries *.yaml *.bin
