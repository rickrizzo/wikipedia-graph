#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <mpi.h>
using namespace std;

#include "article.h"
#include "helpers.h"

#define FILENUM 30
#define NUM_DIRECTORIES 1296

#define THREADS_PER_RANK 2

// MPI Variables
int rank, num_procs;

// int articles_per_rank;
int directories_per_rank;

// Function Templates
std::string getArticleFilename(int input);
std::string getDirectoryName(int input);
void *read_files(void *thread_arg);

int main(int argc, char *argv[]) {


  // Initialize Environment
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // if last rank, how many files?
  // if last rank && numbers of files not evenly divisible by number of ranks
  // if (rank == num_procs - 1 && FILENUM % num_procs > 0) {
  //   articles_per_rank = FILENUM % num_procs;
  //
  // // if not last rank or (last rank && num files evenly divisible by num ranks)
  // } else {
  //   articles_per_rank = FILENUM / num_procs;
  // }

  // should divide evenly
  directories_per_rank = NUM_DIRECTORIES / num_procs;

  // hold each of the thread ids
  pthread_t threads[THREADS_PER_RANK];

  // dummy thread arg
  void *thread_arg;

  // create threads
  for (int i = 0; i < THREADS_PER_RANK; i++) {
    int rc = pthread_create(&threads[i], NULL, read_files, (void *)(intptr_t)(i+1));

    // if we need to pass arguments into the threads, use this line
    // int rc = pthread_create(&threads[i], NULL, read_files, thread_args[i]);

    if (rc != 0) {
      std::cerr << "MAIN: Could not create thread" << std::endl;
      return 1;

    }

  }
  read_files((void *)(intptr_t)(0));

  // join threads
  for (int i = 0; i < THREADS_PER_RANK; i++)
  {
    unsigned int *x;
    pthread_join(threads[i], (void **)&x);

    delete(x);
  }

  // Exit Program
  MPI_Finalize();
  return EXIT_SUCCESS;
}

std::string getArticleFilename(int input) {
  std::stringstream stream;
  stream << "article/article_" << input << ".txt";
  return stream.str();
}

// takes in an integer that represents the number directory we want
// returns a string, the two letter name of that directory
std::string getDirectoryName(int input) {
  std::string directoryName = "~~";

  char firstLetter, secondLetter;

  // relative position of the first and second letters in the list
  // 0123456789abcdefghijklmnopqrstuvwxyz
  int firstLetterStart = input / 36;
  int secondLetterStart = input % 36;

  // offset those relative positions so they match up with the ascii table
  if (firstLetterStart < 10) {
    // character '0' is at decimel 48, so need to add 48
    firstLetter = 48 + firstLetterStart;
  } else {
    firstLetter = 87 + firstLetterStart;
  }

  if (secondLetterStart < 10) {
    secondLetter = 48 + secondLetterStart;
  } else {
    secondLetter = 87 + secondLetterStart;
  }

  directoryName[0] = firstLetter;
  directoryName[1] = secondLetter;

  return directoryName;
}

void *read_files(void *thread_arg) {
  // Multifile Read
  int thread_id = (intptr_t)thread_arg;

  int lowerbound = (rank * (FILENUM / num_procs));
  int upperbound = (rank + 1) * FILENUM / num_procs;
  for(int i = lowerbound; i < upperbound; i++) {
    std::ifstream file(getArticleFilename(i).c_str());
    if(file.is_open()) {
      std::string line;
      while(getline(file, line)) {
        if(line.find("<title>") != std::string::npos) {
          line = trim(line);
          std::cout << line.substr(7, line.length() - 15) << std::endl;
        }
        if(line.find("<text>") != std::string::npos) {
          // First line...
          while(getline(file, line)) {
            if(line.find("</text>") != std::string::npos) { break; }
          }
        }
      }
    file.close();
    }
  }

  unsigned int *return_val = new unsigned int;

  *return_val = pthread_self();
  pthread_exit(return_val);

  return return_val;
}
