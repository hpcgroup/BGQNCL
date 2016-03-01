/** \file simple.c
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

#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc, char** argv) {
  int rank, size;
  MPI_Status status;
  int send[100000],recv[100000];
  //MPI_Init(&argc,&argv);
  int asked=MPI_THREAD_SINGLE, provided;
  MPI_Init_thread(&argc,&argv, asked, &provided);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  MPI_Comm comm;

  if(size < 2) {
    if(!rank) printf("This test requires atleast 2 MPI processes\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  if(!rank)
    printf("[%d] Test run: size %d\n",rank,size);

  MPI_Comm_split(MPI_COMM_WORLD,1,rank,&comm);
  MPI_Pcontrol(1);
  sleep(1 * rank);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Pcontrol(2);
  sleep(10 * rank);
  MPI_Pcontrol(3);
  sleep(2 * rank);
  MPI_Pcontrol(0);
  MPI_Pcontrol(4);
  sleep(3 * rank);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}
