#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc, char** argv) {
  int rank, size;
  MPI_Status status;
  int send[10000],recv[10000];
  MPI_Init(&argc,&argv);
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
  MPI_Sendrecv(send, 1000, MPI_INT, (rank+1)%size, 0,
               recv, 1000, MPI_INT, (rank-1+size)%size, 0,
                comm, &status);
  MPI_Pcontrol(2);
  MPI_Alltoall(send,10, MPI_INT, recv, 10, MPI_INT, MPI_COMM_WORLD);
  MPI_Pcontrol(3);
  MPI_Alltoall(send,10, MPI_INT, recv, 10, MPI_INT, comm);
  MPI_Pcontrol(0);
  MPI_Comm_split(MPI_COMM_WORLD,rank%2,rank,&comm);
  MPI_Pcontrol(4);
  if(rank%2)
  MPI_Alltoall(send,10, MPI_INT, recv, 10, MPI_INT, comm);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}
