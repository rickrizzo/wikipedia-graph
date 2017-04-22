#include <iostream>
#include <mpi.h>
#include <curl/curl.h>

int main(int argc, char *argv[]) {

  // Instance Variables
  int mpi_rank, mpi_commsize;

  // Initialize Environment
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_commsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  // Print Hello
  std::cout << "Hello from rank " << mpi_rank << std::endl;

  // Exit Program
  MPI_Finalize();
  return EXIT_SUCCESS;
}
