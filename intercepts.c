/** \brief This file contains the intercepts for basic setup/tear down
 *  functions.
 *  Author: Nikhil Jain
 *  Contact: nikhil@illinois.edu
 */

#include "intercepts.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Init */
int MPI_Init(int *argc, char ***argv)
{
  int ret;
  ret =  PMPI_Init(argc,argv);
  PROFILER_INIT();
  /* call the init routines */
  return ret;
}

/* Finalize */
int MPI_Finalize(void)
{
  MPI_Pcontrol(0);
  PROFILER_FINALIZE();
  return PMPI_Finalize();
}


/* Pcontrol */
int MPI_Pcontrol(const int ctrl, ...) 
{
  PROFILER_PCONTROL(ctrl);
  PMPI_Pcontrol(ctrl);
}

#ifdef __cplusplus
}
#endif

