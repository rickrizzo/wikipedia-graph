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

#include "article.h"

#define FILENUM 30
#define FILESIZE 1000
#define FILEPATH "/Volumes/Triforce/Parallel/enwiki-20170101-pages-articles-multistream.xml"
#define THREADS_PER_RANK 2

// MPI Variables
int rank, num_procs;
int articles_per_rank;

// Function Templates
std::string getArticleNumber(int input);
std::string &ltrim(std::string &s);
std::string &rtrim(std::string &s);
std::string &trim(std::string &s);

void *read_files(void *thread_arg);

int main(int argc, char *argv[]) {


  // Initialize Environment
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // if last rank, how many files?
  // if last rank && numbers of files not evenly divisible by number of ranks
  if (rank == num_procs - 1 && FILENUM % num_procs > 0) {
    articles_per_rank = FILENUM % num_procs;

  // if not last rank or (last rank && num files evenly divisible by num ranks)
  } else {
    articles_per_rank = FILENUM / num_procs;
  }

  // hold each of the thread ids
  pthread_t threads[THREADS_PER_RANK];

  // dummy thread arg
  void *thread_arg;

  // create threads
  for (int i = 0; i < THREADS_PER_RANK; i++) {
    int rc = pthread_create(&threads[i], NULL, read_files, &thread_arg);

    // if we need to pass arguments into the threads, use this line
    // int rc = pthread_create(&threads[i], NULL, read_files, &thread_args[i]);

    if (rc != 0) {
      std::cerr << "MAIN: Could not create thread" << std::endl;
      return 1;
    }

  }

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

std::string getArticleNumber(int input) {
  std::stringstream stream;
  stream << "article/article_" << input << ".txt";
  return stream.str();
}

std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

void *read_files(void *thread_arg) {
  // Multifile Read
  int lowerbound = (rank * (FILENUM / num_procs));
  int upperbound = (rank + 1) * FILENUM / num_procs;
  for(int i = lowerbound; i < upperbound; i++) {
    std::ifstream file(getArticleNumber(i));
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
    }
    file.close();
  }

  unsigned int *return_val = new unsigned int;

  *return_val = pthread_self();
  pthread_exit(return_val);

}
