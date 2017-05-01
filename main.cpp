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
#include <dirent.h>
#include <sys/types.h>

#include <mpi.h>
#include <cstdlib>

using namespace std;

#include "article.h"
#include "helpers.h"

#define FILENUM 30
#define NUM_DIRECTORIES 1296

#define THREADS_PER_RANK 1

// Pthread Variables
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
struct thread_arg_t {
  int threadLowerbound;
  int threadUpperbound;

  std::vector<std::string> *filePaths;
  std::vector<Article> *articles;
};

// MPI Variables
int mpi_rank, num_procs;

// int articles_per_rank;
int directories_per_rank;
int directories_per_thread;

// Function Templates
std::string getArticleFilename(int input);
std::string getDirectoryName(int input);
std::string &ltrim(std::string &s);
std::string &rtrim(std::string &s);
std::string &trim(std::string &s);

void *readFiles(void *thread_arg);

int main(int argc, char *argv[]) {


  // Initialize Environment
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  // if last rank, how many files?
  // if last rank && numbers of files not evenly divisible by number of ranks
  // if (mpi_rank == num_procs - 1 && FILENUM % num_procs > 0) {
  //   articles_per_rank = FILENUM % num_procs;
  //
  // // if not last rank or (last rank && num files evenly divisible by num ranks)
  // } else {
  //   articles_per_rank = FILENUM / num_procs;
  // }


  // indeces of first directory for this rank and first directory for next rank
  int rankLowerbound = (mpi_rank * (NUM_DIRECTORIES / num_procs));
  // int rankUpperbound = ((rank + 1) * NUM_DIRECTORIES / num_procs);

  // should divide evenly
  directories_per_rank = NUM_DIRECTORIES / num_procs;

  directories_per_thread = directories_per_rank / THREADS_PER_RANK;

  // hold each of the thread ids
  pthread_t threads[THREADS_PER_RANK];

  // store all the files for this rank here
  std::vector<std::string> filePaths;
  std::vector<Article> articles;

  std::vector<thread_arg_t> thread_args;
  for (int i = 0; i < THREADS_PER_RANK; i++) {

    // divide up directories by thread
    thread_arg_t tmp;
    tmp.threadLowerbound = rankLowerbound + (directories_per_thread * i);
    tmp.threadUpperbound = rankLowerbound + (directories_per_thread * (i + 1));
    tmp.filePaths = &filePaths; // share this vector across threads
    tmp.articles = &articles; // share across all threads in this rank

    thread_args.push_back(tmp);
  }

  // create threads
  for (int i = 0; i < THREADS_PER_RANK; i++) {
    int rc = pthread_create(&threads[i], NULL, readFiles, &thread_args[i]);

    // if we need to pass arguments into the threads, use this line
    // int rc = pthread_create(&threads[i], NULL, readFiles, &thread_args[i]);

    if (rc != 0) {
      std::cerr << "MAIN: Could not create thread" << std::endl;
      return EXIT_FAILURE;
    }

  }
  // read_files((void *)(intptr_t)(0));

  // join threads
  for (int i = 0; i < THREADS_PER_RANK; i++)
  {
    unsigned int *x;
    pthread_join(threads[i], (void **)&x);

    delete(x);
  }

  for(int i = 0; i < articles.size(); i++) {
    cout << mpi_rank << " " << articles[i].getTitle() << endl;
  }

  MPI_Barrier(MPI_COMM_WORLD);

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
  std::string directoryName = "~~/";

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

void *readFiles(void *arg) {

  // int lowerbound = (rank * (FILENUM / num_procs));
  // int upperbound = (rank + 1) * FILENUM / num_procs;

  // // indeces of first directory for this rank and first directory for next rank
  // int lowerbound = (rank * (NUM_DIRECTORIES / num_procs));
  // int upperbound = ((rank + 1) * NUM_DIRECTORIES / num_procs);

  thread_arg_t thread_args = *(thread_arg_t*)arg;

  int threadLowerbound = thread_args.threadLowerbound;
  int threadUpperbound = thread_args.threadUpperbound;

  // for each directory in article/
  for (int i = threadLowerbound; i < threadUpperbound; i++) {
    std::string dirPath = "article/";

    dirPath.append(getDirectoryName(i));

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirPath.c_str())) != NULL) {
      while ((ent = readdir (dir)) != NULL) {

        // exclude hidden files
        if (ent->d_name[0] == '.') { continue; }

        pthread_mutex_lock(&mutex1);
        // std::cout << dirPath + ent->d_name << std::endl;

        // ent->d_name is the name of the file
        (*thread_args.filePaths).push_back(dirPath + ent->d_name);
        pthread_mutex_unlock(&mutex1);

      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("Couldn't open directory! ");
      exit (EXIT_FAILURE);
    }
  }

    // now that the files are stored in the vector, read them
    pthread_mutex_lock(&mutex2);
    int fileCount = (*thread_args.filePaths).size();
    pthread_mutex_unlock(&mutex2);

    // while files remain
    while (fileCount > 0) {
      pthread_mutex_lock(&mutex2);
      if (fileCount == 0) { break; }
      // std::cout << "rank " << rank << " file count " << fileCount << '\n';

      std::string tmpPath = (*thread_args.filePaths).back();
      (*thread_args.filePaths).pop_back();
      fileCount = (*thread_args.filePaths).size();

      pthread_mutex_unlock(&mutex2);

      std::ifstream file(tmpPath.c_str());

      // create Article object
      if(file.is_open()) {
        std::string line;
        Article article;

        article.setTitle(tmpPath.substr(11, tmpPath.length() - 15));
        while(getline(file, line)) {
          article.addLinks(line);
        }
        thread_args.articles->push_back(article);

      file.close();
      } else {
        std::cout << "Cannot open file" << std::endl;
      }
    }



  unsigned int *return_val = new unsigned int;

  *return_val = gettid();
  pthread_exit(return_val);

  return return_val;
}
