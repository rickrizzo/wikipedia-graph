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
#include <utility>
#include <mpi.h>
#include <cstdlib>

using namespace std;

#include "article.h"
#include "helpers.h"

#define FILENUM 30
#define NUM_DIRECTORIES 1296

// hold each of the thread ids
#define THREADS_PER_RANK 8
#define MAXTHRDS 8

pthread_mutex_t mutexFilePath = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexArticle = PTHREAD_MUTEX_INITIALIZER;

pthread_t threads[MAXTHRDS];
mylib_barrier_t thread_barrier;

struct thread_arg_t {
  int threadLowerbound;
  int threadUpperbound;
  int id;

  std::vector<std::string> *filePaths;
  std::vector<Article> *articles;
};

// MPI Variables
int mpi_rank, num_procs;
MPI_Request	send_request,recv_request;

// int articles_per_rank;
int directories_per_rank;
int directories_per_thread;

// Function Templates
std::string getArticleFilename(int input);
std::string getDirectoryName(int input);
int getArticlePid(string name);
int getArticleDir(string folder);

void *readFiles(void *thread_arg);

int main(int argc, char *argv[]) {
  // Initialize Environment
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  // indeces of first directory for this rank and first directory for next rank
  // int rankUpperbound = ((rank + 1) * NUM_DIRECTORIES / num_procs);
  directories_per_rank = (NUM_DIRECTORIES / num_procs);
  int rankLowerbound = (mpi_rank * directories_per_rank);
  // cout << mpi_rank << " " << rankLowerbound<< " " <<directories_per_rank <<endl;

  // should divide evenly

  directories_per_thread = directories_per_rank / THREADS_PER_RANK;

  // store all the files for this rank here
  std::vector<std::string> filePaths;
  std::vector<Article> articles;

  // Initilize pthread variables
  mylib_init_barrier(&thread_barrier, THREADS_PER_RANK);
  pthread_attr_t attr;
  pthread_attr_init (&attr);

  std::vector<thread_arg_t> thread_args;

  for (int i = 0; i < THREADS_PER_RANK; i++) {

    // divide up directories by thread
    thread_arg_t tmp;
    tmp.threadLowerbound = rankLowerbound + (directories_per_thread * i);
    tmp.threadUpperbound = rankLowerbound + (directories_per_thread * (i + 1));
    tmp.id = i; // share this vector across threads
    tmp.filePaths = &filePaths; // share this vector across threads
    tmp.articles = &articles; // share across all threads in this rank
    thread_args.push_back(tmp);
  }

  // create threads
  for (int i = 0; i < THREADS_PER_RANK; i++) {
    int rc = pthread_create(&threads[i], &attr, readFiles, &thread_args[i]);

    if (rc != 0) {
      std::cerr << "MAIN: Could not create thread" << std::endl;
      return EXIT_FAILURE;
    }

  }

  // join threads
  for (int i = 0; i < THREADS_PER_RANK; i++)
  {
    unsigned int *x;
    pthread_join(threads[i], (void **)&x);
    delete(x);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // COMMUNICATION
  // Each rank iterates through article list
  // For each article, iterate through the links
  // For each link, identify the rank with that link and send your title
  // Upon receive, add that title to your links (potentially a second links field?)
  // Repeat until no more messages to send for each rank
  // Barrier and close

  vector<ArticleMatch> articlesPerPid[num_procs];
  for(int i = 0; i < articles.size(); i++) {
    for(int j = 0; j < articles[i].getLinks().size(); j++) {

      int sendPid = getArticlePid(articles[i].getLinks()[j].t, NUM_DIRECTORIES, num_procs);
      printf("send %s from %s to %d\n", articles[i].getLinks()[j].t, articles[i].getTitle().c_str(), sendPid);
      ArticleMatch match;
      match.source = articles[i].getTitleA();
      match.link = articles[i].getLinks()[j];
      articlesPerPid[sendPid].push_back(match);
    }
  }
  for(int i = 0; i < num_procs; i++) {
    for(int j = 0; j < articlesPerPid[i].size(); j++) {
      printf("%d send %s link %s to %d\n", mpi_rank, articlesPerPid[i][j].source.t, articlesPerPid[i][j].link.t, i);
      MPI_Isend(articlesPerPid[i][j].link.t, 100, MPI_CHAR, i, 0, MPI_COMM_WORLD, &send_request);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // Exit Program
  MPI_Finalize();
  return EXIT_SUCCESS;
}

void *readFiles(void *arg) {

  thread_arg_t thread_args = *(thread_arg_t*)arg;

  int threadLowerbound = thread_args.threadLowerbound;
  int threadUpperbound = thread_args.threadUpperbound;
  // for each directory in article/
  for (int i = threadLowerbound; i < threadUpperbound; i++) {
    std::string dirPath = "article/";

    dirPath.append(getDirectoryName(i));
    // cout << mpi_rank << " " << i << " " << dirPath << " " << getArticleDir(getDirectoryName(i))<<" "<< getArticlePid(getDirectoryName(i), NUM_DIRECTORIES, num_procs)<< " " <<directories_per_rank <<endl;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirPath.c_str())) != NULL) {
      while ((ent = readdir (dir)) != NULL) {

        // exclude hidden files
        if (ent->d_name[0] == '.') { continue; }

        pthread_mutex_lock(&mutexFilePath);
        // std::cout << dirPath + ent->d_name << std::endl;

        // ent->d_name is the name of the file
        (*thread_args.filePaths).push_back(dirPath + ent->d_name);
        pthread_mutex_unlock(&mutexFilePath);

      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("Couldn't open directory! ");
      exit (EXIT_FAILURE);
    }
  }
  mylib_barrier(&thread_barrier);

  // now that the files are stored in the vector, read them
  pthread_mutex_lock(&mutexFilePath);
  int fileCount = (*thread_args.filePaths).size();
  pthread_mutex_unlock(&mutexFilePath);

  // while files remain
  while (true) {
    pthread_mutex_lock(&mutexFilePath);

    fileCount = (*thread_args.filePaths).size();
    if (fileCount <= 0) {
      pthread_mutex_unlock(&mutexFilePath);
      break;
    }

    std::string tmpPath = (*thread_args.filePaths).back();
    (*thread_args.filePaths).pop_back();

    pthread_mutex_unlock(&mutexFilePath);

    std::ifstream file(tmpPath.c_str());
    if(file.is_open()) {

      std::string line;
      // create Article object
      Article current;
      current.setTitle(tmpPath.substr(11, tmpPath.length() - 15));
      // cout << "Current: "<< current.getTitle() <<endl;
      while(!file.eof() && getline(file, line)) {
        // cout <<current.getTitle();
        if (!line.length()) {continue;}
        current.addLinks(line);
      }
      pthread_mutex_lock(&mutexArticle);
      (*thread_args.articles).push_back(current);
      pthread_mutex_unlock(&mutexArticle);

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
