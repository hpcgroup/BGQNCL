/** \file profiler.c
 * Copyright (c) 2013, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 *
 * Written by:
 *     Nikhil Jain <nikhil.jain@acm.org>
 *
 * LLNL-CODE-678958. All rights reserved.
 *
 * This file is part of BGQNCL. For details, see:
 * https://github.com/LLNL/bgqncl
 * Please also read the LICENSE file for our notice and the LGPL.
 */

#include "profiler.h"

#define BGQ_DEBUG	0
#define NUM_REGIONS	10000

#ifdef __cplusplus
extern "C" {
#endif

int myrank, numranks, isZero, isMaster, masterRank;
unsigned int curset, hNWSet, maxset, numevents; 
MPI_Comm profile_comm;
FILE *dataFile;

typedef struct _counters {
  double time;
} Counters;

Counters values[NUM_REGIONS];

INLINE void PROFILER_INIT() 
{
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);

  masterRank = 0;
  if(!myrank) {
    isMaster = 1;
  } else {
    isMaster = 0;
  }
  isZero = 1;

  char *filename = getenv("PROFILER_TIMER_FILE");
  if(isMaster) {
    if(filename != NULL)
      dataFile = fopen(filename,"w");
    else
      dataFile = stdout;
  }

#if BGQ_DEBUG
  if(isMaster) {
    printf("File opened\n");
  }
#endif

  if(isZero) {
    curset = maxset = 0;
    unsigned int i;
    for(i = 0; i < NUM_REGIONS; i++) {
      values[i].time = 0;
    }
  }
#if BGQ_DEBUG
  if(isMaster) {
    printf("Init intercept complete\n");
  }
#endif
}

INLINE void PROFILER_PCONTROL(int ctrl) {
  if(isMaster) {
#if BGQ_DEBUG
    printf("Pcontrol change from %d to %d\n",curset,ctrl);
#endif
  }
  if(isZero) {
    if(ctrl == 0 && curset == 0) return;

    /* Save the current counter values and change curset to the new value of ctrl */
    if(curset != 0) {
      values[curset].time += MPI_Wtime();
    }
    if(ctrl != 0) {
      values[ctrl].time -= MPI_Wtime();
    }
    curset = ctrl;
    if(curset > maxset) maxset = ctrl;
  }
}

INLINE void PROFILER_FINALIZE() {
  double *timesMax, *timesMin, *timesAvg;
  if(isMaster) {
    timesMax = (double*) malloc((maxset + 1) * sizeof(double));
    timesMin = (double*) malloc((maxset + 1)* sizeof(double));
    timesAvg = (double*) malloc((maxset + 1) * sizeof(double));
  }
  if(isZero) {
    MPI_Reduce(&values[0], timesMax, maxset + 1, MPI_DOUBLE, MPI_MAX, masterRank, MPI_COMM_WORLD);
    MPI_Reduce(&values[0], timesMin, maxset + 1, MPI_DOUBLE, MPI_MIN, masterRank, MPI_COMM_WORLD);
    MPI_Reduce(&values[0], timesAvg, maxset + 1, MPI_DOUBLE, MPI_SUM, masterRank, MPI_COMM_WORLD);
    if(isMaster) {
      unsigned int i;
      for(i = 1; i <= maxset; i++) {
        fprintf(dataFile, "Timing Summary for region %d: min - %.3f s, avg - %.3f s, max - %.3f s\n",
          i, timesMin[i], timesAvg[i]/numranks, timesMax[i]);
      }
      free(timesMax);
      free(timesMin);
      free(timesAvg);
      if(dataFile != stdout)
        fclose(dataFile);
#if BGQ_DEBUG
      printf("Done profiling, exiting\n");
#endif
    }
  }
}

#ifdef __cplusplus
}
#endif

