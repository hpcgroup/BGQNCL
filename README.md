BGQNCL v1.1
===========

**B**lue **G**ene/**Q** **N**etwork Performance **C**ounters Monitoring **L**ibrary

BGQNCL is a library to monitor and record network performance counters on the
5D torus interconnection network of IBM’s Blue Gene/Q platform.  It accesses
the Universal Performance Counter (UPC) hardware counters through the Blue
Gene/Q Hardware Performance Monitoring (HPM) API.


### Method of Solution

Transparent interception of MPI\_Init, MPI\_Pcontrol, and MPI\_Finalize.  Use
MPI\_Pcontrol around regions of interest. The argument passed to MPI\_Pcontrol
identifies the region. Multiple calls with same argument leads to summation of
counter values during such executions. Argument value 0 is assumed at startup
and is used by the library as a stopper.  Use positive values for specify start
of your region of interest and 0 to mark the end. You can implicitly mark end
of a region by starting a new region. 

Limitation: Argument has to be less than equal to 9.

### Output

Set environment variable BGQ\_COUNTER\_FILE to the output file
to which counters should be written. If the such a file can't be created,
the output is dumped to stdout.

##### Output format:
Each line of the output file contains data for a physical node. The meta 
data for a line is:

Pcontrol\_region world\_rank coords[0] coords[1] coords[2] coords[3] coords[4] coords[5] \*\* linkdata

link data = (d\_A-) (d\_A+) (d\_B-) (d\_B+) (d\_C-) (d\_C+) (d\_D-) (d\_D+) (d\_E-) (d\_E+)

d\_\* = sent\_chunks (32 bytes)  dynamic\_chunks deterministic\_chunks col\_packets (ignore) recv\_packets (512 bytes) fifo\_length

Hence, there are 60 entries that are part of link data.

### Build

Edit Makefile to point BGPM to the installation location of bgpm and type:
```bash
make
```

### Run

Link the profiler before your MPI library. Here is a sample link line
```
$(CC) -o mypgm mybin.o libprofiler.a -L $(BGPM)/lib -lbgpm -lrt -lstdc++
```

##### Test
```
make test-all
```
Run the binary simple without any arguments on >= 2 processes.

### Reference

Any published work that utilizes this library should include the following
reference:

```
Abhinav Bhatele, Nikhil Jain, Katherine E. Isaacs, Ronak Buch, Todd Gamblin,
Steven H. Langer, and Laxmikant V. Kale. Optimizing the performance of parallel
applications on a 5D torus via task mapping. In Proceedings of IEEE
International Conference on High Performance Computing, HiPC '14. IEEE Computer
Society, December 2014. LLNL-CONF-655465.
```

### Release

Copyright (c) 2013, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory.

Written by:
```
    Nikhil Jain <nikhil.jain@acm.org>
    Abhinav Bhatele <bhatele@llnl.gov>
```

LLNL-CODE-678958. All rights reserved.

This file is part of BGQNCL. For details, see:
https://github.com/LLNL/bgqncl
Please also read the LICENSE file for our notice and the LGPL.
