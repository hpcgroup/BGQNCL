#=============================================================================
# Common Variables
CC      = mpixlc
CXX     = mpixlcxx
COPTS   = -c -O3
CXXOPTS = -c -O3
LOPTS   = 
INC	= -I./
BGPM = /bgsys/drivers/ppcfloor/bgpm
#BGPM = /soft/perftools/bgpm
LIBS	= libprofiler.a -L $(BGPM)/lib -lbgpm -lrt -lstdc++

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

