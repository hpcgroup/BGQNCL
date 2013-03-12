#=============================================================================
# Common Variables
CC      = mpicc
CXX     = mpicxx
COPTS   = -c -O3
CXXOPTS = -c -O3
LOPTS   = 
INC	= -I./
LIBS	= libprofiler.a -L /soft/perftools/bgpm/lib/ -lbgpm -lrt -lstdc++

all: libprofiler.a
test-all: simple jacobi2dc jacobi2dcc

libprofiler.a: intercepts.o profiler.o
	ar -q libprofiler.a intercepts.o profiler.o

profiler.o: profiler.c profiler.h
	$(CC) $(COPTS) -o profiler.o profiler.c ${INC}

intercepts.o: intercepts.c intercepts.h profiler.h
	$(CC) $(COPTS) -o intercepts.o intercepts.c ${INC}

simple:	simple.c libprofiler.a
	$(CC) $(COPTS) simple.c -o simple.o $(INC)
	$(CXX) -o simple simple.o $(LIBS)

clean:
	rm -f *.o libprofiler.a simple

