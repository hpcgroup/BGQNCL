/** \file intercepts.h
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
 * https://github.com/LLNL/bgqncl
 * Please also read the LICENSE file for our notice and the LGPL.
 */

/** \brief This file contains the intercepts for basic setup/tear down
 *  functions.
 */

#include "intercepts.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Init */
int MPI_Init(int *argc, char ***argv)
{
  int ret;
  ret =  PMPI_Init(argc, argv);
  PROFILER_INIT();
  /* call the init routines */
  return ret;
}

int MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
  int ret;
  ret =  PMPI_Init_thread(argc, argv, required, provided);
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

