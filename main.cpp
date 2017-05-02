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
#include <tr1/unordered_map>

#include "mpi.h"

#include "article.h"
#include "helpers.h"

using namespace std;
using namespace tr1;
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

  vector<string> *filePaths;
  vector<Article> *articles;
};

// MPI Variables
int mpi_rank, num_procs;
MPI_Request	send_request,recv_request;
MPI_Status status;

// int articles_per_rank;
int directories_per_rank;
int directories_per_thread;

void *readFiles(void *thread_arg);

bool sortOutNodes(Article a1, Article a2);

int main(int argc, char *argv[]) {
  // Initialize Environment
  MPI_Datatype MPI_ArticleMatch; // datatype for sending

  // Timing Variables
  double start = 0, file_ops = 0, end = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  MPI_Type_contiguous(200, MPI_CHAR, &MPI_ArticleMatch);
  MPI_Type_commit(&MPI_ArticleMatch);

  if(mpi_rank == 0) { start = MPI_Wtime(); }


  // indeces of first directory for this rank and first directory for next rank
  // int rankUpperbound = ((rank + 1) * NUM_DIRECTORIES / num_procs);
  directories_per_rank = (NUM_DIRECTORIES / num_procs);
  int rankLowerbound = (mpi_rank * directories_per_rank);
  //cout << mpi_rank << " " << rankLowerbound <<endl;

  // should divide evenly

  directories_per_thread = directories_per_rank / THREADS_PER_RANK;

  // store all the files for this rank here
  vector<string> filePaths;
  vector<Article> articles;
  tr1::unordered_map<string, Article*> articleMap;

  // Initilize pthread variables
  mylib_init_barrier(&thread_barrier, THREADS_PER_RANK);
  pthread_attr_t attr;
  pthread_attr_init (&attr);

  vector<thread_arg_t> thread_args;
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
      cerr << "MAIN: Could not create thread" << endl;
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

  if(mpi_rank == 0) { file_ops = MPI_Wtime(); }

  //cout << mpi_rank << ": articles: "<< articles.size() << endl;

  for (int i = 0; i < articles.size(); i++){
    cout << mpi_rank << ": article: "<< articles[i].getTitle() << endl;
    articleMap.insert(make_pair(articles[i].getTitle(),&articles[i]));
  }

  MPI_Barrier(MPI_COMM_WORLD);
  //cout << mpi_rank << " barrier done" << endl;

  // COMMUNICATION
  // Each rank iterates through article list
  // For each article, iterate through the links
  // For each link, identify the rank with that link and send your title
  // Upon receive, add that title to your links (potentially a second links field?)
  // Repeat until no more messages to send for each rank
  // Barrier and close
  vector<ArticleMatch> *articlesByRank = new vector<ArticleMatch>[num_procs];

  for(int i = 0; i < articles.size(); i++) {
    for(int j = 0; j < articles[i].getLinks().size(); j++) {
      int sendRank = getArticleRank(articles[i].getLinks()[j].t, NUM_DIRECTORIES, num_procs);
      ArticleMatch match;
      match.source = articles[i].getTitleA();
      match.link = articles[i].getLinks()[j];
      articlesByRank[sendRank].push_back(match);
    }
  }

  // Send data
  // int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              // MPI_Comm comm, MPI_Request *request)

  int *numSend = new int[num_procs];
  MPI_Request	*send_request_num = new MPI_Request[num_procs];

  for(int i = 0; i < num_procs; i++) {
    numSend[i] = articlesByRank[i].size();
    MPI_Isend(&numSend[i], 1, MPI_INT, i, 10*(num_procs*i + mpi_rank)+1, MPI_COMM_WORLD, &send_request_num[i]);
  }

  MPI_Request	*send_request_links = new MPI_Request[num_procs];

  for(int i = 0; i < num_procs; i++) {
    printf("From %d send %d links to %d\n", mpi_rank, numSend[i],  i);

    MPI_Isend(&articlesByRank[i][0], numSend[i], MPI_ArticleMatch, i, 10*(num_procs*i + mpi_rank)+2, MPI_COMM_WORLD, &send_request_links[i]);
  }

  int *numRecv = new int[num_procs];
  MPI_Request	*recv_request_num = new MPI_Request[num_procs];

  // GET NUM REQUESTS, THEN RECV
  //MPI_Barrier(MPI_COMM_WORLD);
  int num_links = 0;
  for(int i = 0; i < num_procs; i++) {
    MPI_Irecv(&numRecv[i], 1, MPI_INT, i, 10*(num_procs*mpi_rank + i)+1, MPI_COMM_WORLD, &recv_request_num[i]);
  }
  MPI_Waitall(num_procs, recv_request_num, MPI_STATUSES_IGNORE);

  for(int i = 0; i < num_procs; i++) {
    num_links += numRecv[i];
  }

  cout << "RANK " << mpi_rank << " WILL RECEIVE " << num_links << " LINKS" << endl;

  MPI_Request	*recv_request_links = new MPI_Request[num_procs];

  ArticleMatch *recivedArticles = new ArticleMatch[num_links];

  int currentLink = 0;
  for(int i = 0; i < num_procs; i++) {
    MPI_Irecv(&recivedArticles[currentLink], numRecv[i], MPI_ArticleMatch, i, 10*(num_procs*mpi_rank + i)+2, MPI_COMM_WORLD, &recv_request_links[i]);
    currentLink += numRecv[i];
  }

  MPI_Waitall(num_procs, recv_request_links, MPI_STATUSES_IGNORE);

  for(int i = 0; i < num_links; i++) {
    printf("%d recived source %s, link %s\n", mpi_rank, recivedArticles[i].source.t, recivedArticles[i].link.t);
    string title = StringAtoString(recivedArticles[i].link);
    string from = StringAtoString(recivedArticles[i].source);
    tr1::unordered_map<string,Article*>::iterator find = articleMap.find(title);
    if (find == articleMap.end()){
      cout <<"Article "<<title<<" not found"<<endl;
    }
    else{
      find->second->addLinkedTo(from);
    }

  }
  MPI_Waitall(num_procs, send_request_num, MPI_STATUSES_IGNORE);
  MPI_Waitall(num_procs, send_request_links, MPI_STATUSES_IGNORE);


  MPI_Barrier(MPI_COMM_WORLD);
  if(mpi_rank == 0) {
    end = MPI_Wtime();
    std::cout << "FILE I/O TIME: " << file_ops - start << std::endl;
    std::cout << "RUN TIME: " << end - start << std::endl;
  }

  // MOST NODES
  std::sort(articles.begin(), articles.end(), sortOutNodes);
  for(int i = 0; i < 3 && i < articles.size(); i++) {
    std::cout << "\tRANK " << mpi_rank << "-> " << i << ": " << articles[i].getLinks().size() << std::endl;
  }

  delete [] numSend ;
  delete [] articlesByRank ;
  delete [] send_request_num ;
  delete [] send_request_links ;
  delete [] numRecv ;
  delete [] recv_request_num ;
  delete [] recv_request_links ;
  // Exit Program
  MPI_Finalize();
  return EXIT_SUCCESS;
}

void *readFiles(void *arg) {

  thread_arg_t thread_args = *(thread_arg_t*)arg;

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
    string dirPath = "article/";

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

    string tmpPath = (*thread_args.filePaths).back();
    (*thread_args.filePaths).pop_back();

    pthread_mutex_unlock(&mutexFilePath);

    ifstream file(tmpPath.c_str());
    if(file.is_open()) {

      string line;
      // create Article object
      Article current;
      getline(file, line); // discard title
      current.setTitle(tmpPath.substr(11, tmpPath.length() - 15));
      // current.setTitle(line.substr(7));

      while(!file.eof() && getline(file, line)) {
        if (!line.length()) {continue;}
        size_t linkEnd = line.find("#"); // Find a anchor
        if (linkEnd != string::npos){
          line = line.substr(0, linkEnd);
        }
        current.addLinks(line);
      }
      pthread_mutex_lock(&mutexArticle);
      (*thread_args.articles).push_back(current);
      pthread_mutex_unlock(&mutexArticle);

      file.close();
    } else {
      cout << "Cannot open file" << endl;
    }
  }
  // cout << mpi_rank<< "."<< thread_args.id<<" done" <<endl;

  unsigned int *return_val = new unsigned int;

  *return_val = gettid();
  pthread_exit(return_val);

  return return_val;
}

bool sortOutNodes(Article a1, Article a2) { return a1.getLinks().size() > a2.getLinks().size(); }
