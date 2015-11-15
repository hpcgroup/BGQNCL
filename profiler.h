/** \file profiler.h
 * Copyright (c) 2013, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 *
 * Written by:
 *     Nikhil Jain <nikhil.jain@acm.org>
 *     Abhinav Bhatele <bhatele@llnl.gov>
 *
 * LLNL-CODE-678958. All rights reserved.
 *
 * This file is part of BGQNCL. For details, see:
 * https://github.com/scalability-llnl/bgqncl
 * Please also read the LICENSE file for our notice and the LGPL.
 */

#ifndef PROFILER_H_
#define PROFILER_H_

#include "mpi.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define INLINE 

#ifdef __cplusplus
extern "C" {
#endif

typedef int myInt;
#define MY_MPI_INT MPI_INT
typedef double myReal;
#define MY_MPI_REAL MPI_DOUBLE

extern INLINE void PROFILER_INIT();
extern INLINE void PROFILER_FINALIZE();
extern INLINE void PROFILER_PCONTROL();

#ifdef __cplusplus
}
#endif

#endif //end of PROFILER_H_
