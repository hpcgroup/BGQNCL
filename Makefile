##############################################################################
# Copyright (c) 2013, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# Written by:
#     Nikhil Jain <nikhil.jain@acm.org>
#     Abhinav Bhatele <bhatele@llnl.gov>
#
# LLNL-CODE-678958. All rights reserved.
#
# This file is part of BGQNCL. For details, see:
# https://github.com/LLNL/bgqncl
# Please also read the LICENSE file for our notice and the LGPL.
##############################################################################


# Common Variables
CC      ?= cc
CXX     ?= CC
COPTS   = -c -O3 -g
CXXOPTS = -c -O3 -g
LOPTS   = 
INC	= -I./
LIBS	= libprofiler.a

all: libprofiler.a
test-all: simple 

libprofiler.a: intercepts.o profiler.o
	ar cr $@ $^

profiler.o: profiler.c profiler.h
	$(CC) $(COPTS) -o $@ $< ${INC}

intercepts.o: intercepts.c intercepts.h profiler.h
	$(CC) $(COPTS) -o $@ $< ${INC}

simple:	simple.c libprofiler.a
	$(CC) $(COPTS) $< -o simple.o $(INC)
	$(CXX) -o $@ simple.o $(LIBS)

clean:
	rm -f *.o libprofiler.a simple

