/** \file profiler.c
 *  Author: Nikhil Jain
 *  Contact: nikhil@illinois.edu
 */
#include "profiler.h"
#include "spi/include/kernel/process.h"
#include "spi/include/kernel/location.h"
#include <firmware/include/personality.h>
#include "bgpm/include/bgpm.h"

#define BGQ_DEBUG 1

#ifdef __cplusplus
extern "C" {
#endif

int myrank, numranks, isZero, isMaster, masterRank;
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
  int coords[6], tmasterRank;
  MPIX_Rank2torus(myrank, coords);
  if(coords[0]+coords[1]+coords[2]+coords[3]+coords[4]+coords[5] == 0) {
    isMaster = 1;
  } else {
    isMaster = 0;
  }
  if(isMaster) {
    printf("Init intercepted by bgqcounter unit\n");
  }

  char *filename = getenv("BGQ_COUNTER_FILE");
  if(filename != NULL)
    dataFile = fopen(filename,"w");
  else
    dataFile = stdout;
#if BGQ_DEBUG
  if(isMaster) {
    printf("File opened, Initializing BGPM\n");
  }
#endif

  Bgpm_Init(BGPM_MODE_SWDISTRIB);
#if BGQ_DEBUG
  if(isMaster) {
    printf("Initialized BGPM, Splitting communicator\n");
  }
#endif

  isZero = (coords[5] == 0) ? 1 : 0;
  MPI_Comm_split(MPI_COMM_WORLD, isZero, myrank, &profile_comm);
#if BGQ_DEBUG
  if(isMaster) {
    printf("Communicator split done, informing master\n");
  }
#endif

  coords[0] = coords[1] = coords[2] = coords[3] = coords[4] = coords[5] = 0;
  MPIX_Torus2rank(coords, &tmasterRank);
#if BGQ_DEBUG
  if(isMaster) {
    printf("Init intercepted by bgqcounter unit 5\n");
  }
#endif

  if(isMaster) {
    MPI_Comm_rank(profile_comm, &masterRank);
  }
  MPI_Bcast(&masterRank, 1, MPI_INT, tmasterRank, MPI_COMM_WORLD);
#if BGQ_DEBUG
  if(isMaster) {
    printf("Informed master, attaching counters\n");
  }
#endif

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
  if(isMaster) {
    printf("Init intercept complete\n");
  }
}

INLINE void PROFILER_PCONTROL(int ctrl) {
  if(isMaster) {
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
  if(isMaster) {
    printf("Finalize intercepted: numevents: %u, max set: %u\n",numevents, maxset);
    MPI_Comm_size(profile_comm,&nranks);
    allCounters = (uint64_t*) malloc(10*numevents*nranks*sizeof(uint64_t));
  }
  if(isZero) {
    for(unsigned int i = 1; i <= maxset; i++) {
      MPI_Gather(values[i].counters, 10*numevents, MPI_UNSIGNED_LONG_LONG, allCounters, 
          10*numevents, MPI_UNSIGNED_LONG_LONG, masterRank, profile_comm);
      if(isMaster) {
        MPI_Group world, profile_group;
        MPI_Comm_group(MPI_COMM_WORLD, &world);
        MPI_Comm_group(profile_comm, &profile_group);
        int *world_ranks, *profile_ranks;
        profile_ranks = (int*)malloc(nranks*sizeof(int));
        world_ranks = (int*)malloc(nranks*sizeof(int));
        for(unsigned int j = 0; j < nranks; j++) {
          profile_ranks[j] = j;
        }
        MPI_Group_translate_ranks(profile_group, nranks, profile_ranks, world, world_ranks);
        unsigned int cnt = 0;
        int coords[6];
        for(unsigned int j = 0; j < nranks; j++) {
          //MPIX_Rank2torus(j*Kernel_ProcessCount(), coords); 
          MPIX_Rank2torus(world_ranks[j], coords); 
          fprintf(dataFile,"%d %d ",i,j);
          fprintf(dataFile,"%d %d %d %d %d %d ** ",coords[0],coords[1],coords[2],coords[3],coords[4],coords[5]);
          for(unsigned int k = 0; k < 10*numevents; k++) {
            fprintf(dataFile,"%lu ", allCounters[cnt++]);
          }
          fprintf(dataFile,"\n");
        }
        fclose(dataFile);
        free(profile_ranks); free(world_ranks);
      }
    }
  }
  if(isMaster) {
    printf("Done profiling, exiting\n");
    free(allCounters);
  }
}
#ifdef __cplusplus
}
#endif

