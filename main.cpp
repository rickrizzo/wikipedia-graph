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

// Read Files (ROB)
// -- Read into program
// -- Article
// -- Strip Article
// -- Parse XMLs

std::string getArticleNumber(int input);

int main(int argc, char *argv[]) {

  // MPI Variables
  int rank, num_procs;
  // MPI_File file;
  // MPI_Status status;

  // Instance Variables
  // int buffsize, num_chars;
  // char buffer[FILESIZE];

  // Initialize Environment
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Single Read
  // Likely more inline with expectations, but makes less sense
  //
  // buffsize = FILESIZE / num_procs;
  // num_chars = buffsize / sizeof(char);
  //
  // MPI_File_open(MPI_COMM_WORLD, FILEPATH, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
  // MPI_File_seek(file, rank * buffsize, MPI_SEEK_SET);
  // MPI_File_read(file, buffer, num_chars, MPI_CHAR, &status);
  // std::cout << "Rank: " << rank << " " << std::string(buffer) << std::endl;
  // MPI_File_close(&file);

  // Multifile Read
  // Unsure if this is "truely parallel" I/O
  // Seems the only logical way to read however
  //
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
