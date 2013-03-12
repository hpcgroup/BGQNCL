/** \file profiler.h
 *  Author: Nikhil Jain
 *  Contact: nikhil@illinois.edu
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
