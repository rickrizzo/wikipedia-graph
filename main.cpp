#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <mpi.h>

#include "article.h"

#define FILENUM 30
#define FILESIZE 1000
#define FILEPATH "/Volumes/Triforce/Parallel/enwiki-20170101-pages-articles-multistream.xml"

// Function Templates
std::string getArticleNumber(int input);

int main(int argc, char *argv[]) {

  // MPI Variables
  int rank, num_procs;

  // Initialize Environment
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Multifile Read
  int lowerbound = (rank * (FILENUM / num_procs));
  int upperbound = (rank + 1) * FILENUM / num_procs;
  for(int i = lowerbound; i < upperbound; i++) {
    std::ifstream file(getArticleNumber(i));
    if(file.is_open()) {
      std::string line;
      while(getline(file, line)) {
        std::cout << line << std::endl;
      }
    }
    file.close();
  }

  // Exit Program
  MPI_Finalize();
  return EXIT_SUCCESS;
}

std::string getArticleNumber(int input) {
  std::stringstream stream;
  stream << "article/article_" << input << ".txt";
  return stream.str();
}
