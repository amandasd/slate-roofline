##############################################################################
# Copyright (c) 2014, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# Written by:
#     Dylan Wang <dylan.wang@gmail.com>
#     Staci Smith <smiths949@email.arizona.edu>
#     Abhinav Bhatele <bhatele@llnl.gov>
#
# LLNL-CODE-678960. All rights reserved.
#
# This file is part of AriesNCL. For details, see:
# https://github.com/LLNL/ariesncl
# Please also read the LICENSE file for our notice and the LGPL.
##############################################################################

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
