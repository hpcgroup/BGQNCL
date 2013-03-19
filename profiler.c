/** \file profiler.c
 *  Author: Nikhil Jain
 *  Contact: nikhil@illinois.edu
 */
#include "profiler.h"
#include "spi/include/kernel/process.h"
#include "spi/include/kernel/location.h"
#include <firmware/include/personality.h>
#include "bgpm/include/bgpm.h"

#ifdef __cplusplus
extern "C" {
#endif

int myrank, numranks, isZero;
unsigned int curset, hNWSet, maxset, numevents; 
MPI_Comm profile_comm;
FILE *dataFile;

UPC_NW_LinkMasks linkmask[] = { UPC_NW_LINK_A_MINUS,
UPC_NW_LINK_A_PLUS,
UPC_NW_LINK_B_MINUS,   
UPC_NW_LINK_B_PLUS,
UPC_NW_LINK_C_MINUS,   
UPC_NW_LINK_C_PLUS,
UPC_NW_LINK_D_MINUS,   
UPC_NW_LINK_D_PLUS,
UPC_NW_LINK_E_MINUS,   
UPC_NW_LINK_E_PLUS };

typedef struct _counters {
  uint64_t counters[60];
} Counters;

Counters values[10];

INLINE void PROFILER_INIT() 
{
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&numranks);
  if(!myrank) {
    printf("Init intercepted by bgqcounter unit\n");
  }
  char *filename = getenv("BGQ_COUNTER_FILE");
  if(filename != NULL)
    dataFile = fopen(filename,"w");
  else
    dataFile = stdout;

  Bgpm_Init(BGPM_MODE_SWDISTRIB);
  isZero = (Kernel_ProcessorID() == 0) ? 1 : 0;
  MPI_Comm_split(MPI_COMM_WORLD, isZero, myrank, &profile_comm);
  if(isZero) {
    hNWSet = Bgpm_CreateEventSet();
    Bgpm_AddEvent(hNWSet, PEVT_NW_USER_PP_SENT);
    Bgpm_AddEvent(hNWSet, PEVT_NW_USER_DYN_PP_SENT);
    Bgpm_AddEvent(hNWSet, PEVT_NW_USER_ESC_PP_SENT);
    //Bgpm_AddEvent(hNWSet, PEVT_NW_USER_WORLD_COL_SENT);
    //Bgpm_AddEvent(hNWSet, PEVT_NW_USER_SUBC_COL_SENT);
    //Bgpm_AddEvent(hNWSet, PEVT_NW_COMMWORLD_COL_SENT);
    numevents = 3;
    if (Bgpm_Attach(hNWSet, UPC_NW_ALL_TORUS_LINKS, 0) == BGPM_WALREADY_ATTACHED) {
      printf("Error: Another sw thread on node owns network link counters\n");
    }
    curset = maxset = 0;
    for(unsigned int i = 0; i < 10; i++) {
      for(unsigned int j = 0; j < 10*numevents; j++) {
        values[i].counters[j] = 0;
      }
    }
  }
}

INLINE void PROFILER_PCONTROL(int ctrl) {
  if(!myrank) {
    printf("Pcontrol change from %d to %d\n",curset,ctrl);
  }
  if(isZero) {
    if(ctrl == 0 && curset == 0) return;
    if(curset != 0) {
      unsigned int cnt = 0;
      uint64_t val;
      unsigned int numEvts = Bgpm_NumEvents(hNWSet);
      assert(numEvts == numevents);
      for(unsigned int i = 0; i < 10; i++) {
        for(unsigned int j = 0; j < numevents; j++) {
          Bgpm_NW_ReadLinkEvent(hNWSet, j, linkmask[i], &val);
          values[curset].counters[cnt++] += val;
        }
      }
    }
    if(ctrl != 0) Bgpm_ResetStart(hNWSet);
    curset = ctrl;
    if(curset > maxset) maxset = ctrl;
    
  }
}

INLINE void PROFILER_FINALIZE() {
  uint64_t *allCounters;
  int nranks;
  if(!myrank) {
    printf("Finalize intercepted: numevents: %u, max set: %u\n",numevents, maxset);
    MPI_Comm_size(profile_comm,&nranks);
    allCounters = (uint64_t*) malloc(10*numevents*nranks*sizeof(uint64_t));
  }
  if(isZero) {
    for(unsigned int i = 1; i <= maxset; i++) {
      MPI_Gather(values[i].counters, 10*numevents, MPI_UNSIGNED_LONG_LONG, allCounters, 
          10*numevents, MPI_UNSIGNED_LONG_LONG, 0, profile_comm);
      if(!myrank) {
        unsigned int cnt = 0;
        int coords[6];
        for(unsigned int j = 0; j < nranks; j++) {
          MPIX_Rank2torus(j*Kernel_ProcessCount(), coords); 
          fprintf(dataFile,"%d %d ",i,j);
          fprintf(dataFile,"%d %d %d %d %d %d ** ",coords[0],coords[1],coords[2],coords[3],coords[4],coords[5]);
          for(unsigned int k = 0; k < 10*numevents; k++) {
            fprintf(dataFile,"%lu ", allCounters[cnt++]);
          }
          fprintf(dataFile,"\n");
        }
      }
    }
  }
  if(!myrank) free(allCounters);
}
#ifdef __cplusplus
}
#endif

