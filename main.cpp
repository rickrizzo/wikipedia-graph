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
#include <cstdlib>

#include "mpi.h"

#include "article.h"
#include "helpers.h"

using namespace std;

#define FILENUM 30
#define NUM_DIRECTORIES 1296

// hold each of the thread ids
#define THREADS_PER_RANK 8
#define MAXTHRDS 8

pthread_mutex_t mutexFilePath = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexArticle = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCurrentDir = PTHREAD_MUTEX_INITIALIZER;

pthread_t threads[MAXTHRDS];
mylib_barrier_t thread_barrier;

struct thread_arg_t {
  int rankLowerbound;
  int rankUpperbound;
  int id;
  int * currentDir;

  std::vector<std::string> *filePaths;
  std::vector<Article> *articles;
};

// MPI Variables
int mpi_rank, num_procs;
MPI_Request	send_request,recv_request;
MPI_Status status;

// int articles_per_rank;
int directories_per_rank;
int directories_per_thread;

void *readFiles(void *thread_arg);

int main(int argc, char *argv[]) {
  // Initialize Environment
  MPI_Datatype MPI_ArticleMatch; // datatype for sending

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  MPI_Type_contiguous(200, MPI_CHAR, &MPI_ArticleMatch);
  MPI_Type_commit(&MPI_ArticleMatch);


  // indeces of first directory for this rank and first directory for next rank
  // int rankUpperbound = ((rank + 1) * NUM_DIRECTORIES / num_procs);
  directories_per_rank = (NUM_DIRECTORIES / num_procs);
  int rankLowerbound = (mpi_rank * directories_per_rank);
  cout << mpi_rank << " " << rankLowerbound <<endl;

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
  int currentDir = rankLowerbound -1;
  for (int i = 0; i < THREADS_PER_RANK; i++) {
    // divide up directories by thread
    thread_arg_t tmp;
    tmp.rankLowerbound = rankLowerbound;
    tmp.rankUpperbound = rankLowerbound + directories_per_rank;
    tmp.id = i; // share this vector across threads
    tmp.currentDir = &currentDir; // share this vector across threads
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
    // delete(x);
  }

  cout << mpi_rank << ": articles: "<< articles.size() << endl;

  MPI_Barrier(MPI_COMM_WORLD);
  cout << mpi_rank << " barrier done" << endl;

  // COMMUNICATION
  // Each rank iterates through article list
  // For each article, iterate through the links
  // For each link, identify the rank with that link and send your title
  // Upon receive, add that title to your links (potentially a second links field?)
  // Repeat until no more messages to send for each rank
  // Barrier and close
  vector<ArticleMatch> *articlesByRank = new vector<ArticleMatch>[num_procs]; //right

  for(int i = 0; i < articles.size(); i++) {
    for(int j = 0; j < articles[i].getLinks().size(); j++) {
      int sendRank = getArticleRank(articles[i].getLinks()[j].t, NUM_DIRECTORIES, num_procs);
      ArticleMatch match;
      match.source = articles[i].getTitleA();
      match.link = articles[i].getLinks()[j];
      articlesByRank[sendRank].push_back(match);
    }
  }
  for(int i = 0; i < num_procs; i++) {
    int temp = articlesByRank[i].size();
    MPI_Isend(&temp, 1, MPI_INT, i, i, MPI_COMM_WORLD, &send_request);
  }

  for(int i = 0; i < num_procs; i++) {
    for(int j = 0; j < articlesByRank[i].size(); j++) {
      printf("From R%d article:%s send link:%s to R%d\n", mpi_rank, articlesByRank[i][j].source.t, articlesByRank[i][j].link.t, i);
      MPI_Isend(articlesByRank[i][j].link.t, 100, MPI_CHAR, i, 0, MPI_COMM_WORLD, &send_request);
    }
  }


  // GET NUM REQUESTS, THEN RECV
  //MPI_Barrier(MPI_COMM_WORLD);
  int num_msgs = 0;
  for(int i = 0; i < num_procs; i++) {
    int temp = 0;
    MPI_Irecv(&temp, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &send_request);
    MPI_Wait(&send_request, &status);
    num_msgs += temp;
  }

  std::cout << "RANK " << mpi_rank << " WILL RECEIVE " << num_msgs << " MESSAGES" << std::endl;

  for(int i = 0; i < num_msgs; i++) {
    // char buffer[1000];
    // MPI_Irecv(buffer, 100, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request);
    // printf("RECV: %s\n", buffer);
    // printf("%s", rbuf);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // Exit Program
  MPI_Finalize();
  return EXIT_SUCCESS;
}

void *readFiles(void *arg) {

  thread_arg_t thread_args = *(thread_arg_t*)arg;

  int rankLowerbound = thread_args.rankLowerbound;
  int rankUpperbound = thread_args.rankUpperbound;
  // for each directory in article/
  int dirIndex = 0;

  while (true){
    pthread_mutex_lock(&mutexCurrentDir);
    *(thread_args.currentDir) += 1;
    dirIndex = *(thread_args.currentDir);
    pthread_mutex_unlock(&mutexCurrentDir);
    if (dirIndex >= rankUpperbound){
      break;
    }
    std::string dirPath = "article/";

    dirPath.append(getDirectoryName(dirIndex));
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirPath.c_str())) != NULL) {
      while ((ent = readdir (dir)) != NULL) {

        // exclude hidden files
        if (ent->d_name[0] == '.') { continue; }

        pthread_mutex_lock(&mutexFilePath);

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
  if (thread_args.id == 0){
    cout << mpi_rank<< " fileCount: " << fileCount <<endl;
  }
  pthread_mutex_unlock(&mutexFilePath);
  // cout << mpi_rank<< "."<< thread_args.id<<" start" <<endl;
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
      // current.setTitle(line.substr(7));

      while(!file.eof() && getline(file, line)) {
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
  // cout << mpi_rank<< "."<< thread_args.id<<" done" <<endl;

  unsigned int *return_val = new unsigned int;

  *return_val = gettid();
  pthread_exit(return_val);

  return return_val;
}
