#ifndef HELPERS_H
#define HELPERS_H

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#include <cmath>

using namespace std;

const string alphanumeric = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const string alphanumericlower = "0123456789abcdefghijklmnopqrstuvwxyz";

string getFolder(string title) {
  size_t first, second;
  first = title.find_first_of(alphanumeric);
  second = title.find_first_of(alphanumeric,first+1);

  std::stringstream folder;

  if (first == string::npos){
    folder << "00";
  }
  else if (second == string::npos){
    folder << (char)tolower(title[first]) << "0";
  }
  else{
    folder << (char)tolower(title[first]) << (char)tolower(title[second]);
  }
  return folder.str();
}

string makeArticleFilename(string title) {
  return "article/" + getFolder(title) + "/" + title + ".txt";
}

int getArticleDir(string folder){
  int firstLetter = alphanumericlower.find_first_of(folder[0]);
  int secondLetter = alphanumericlower.find_first_of(folder[1]);

  int dir = firstLetter * 36 + secondLetter;
  return dir;
}

int getArticlePid(string name, int num_directories, int num_procs){
  string folder = getFolder(name);

  int dir = getArticleDir(folder);
  int directories_per_rank = (num_directories / num_procs);

  int pid = dir / directories_per_rank;

  return pid;
}

// takes in an integer that represents the number directory we want
// returns a string, the two letter name of that directory
string getDirectoryName(int input) {
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

string &ltrim(string &s) {
  s.erase(s.begin(), find_if(s.begin(), s.end(),
  not1(ptr_fun<int, int>(isspace))));
  return s;
}

string &rtrim(string &s) {
  s.erase(find_if(s.rbegin(), s.rend(),
  not1(ptr_fun<int, int>(isspace))).base(), s.end());
  return s;
}

string &trim(string &s) {
  return ltrim(rtrim(s));
}

size_t gettid() {
    pthread_t ptid = pthread_self();
    size_t threadId = 0;
    memcpy(&threadId, &ptid, std::min(sizeof(threadId), sizeof(ptid)));
    return threadId;
}

typedef struct {
  pthread_mutex_t count_lock;
  pthread_cond_t ok_to_proceed;
  int count;
  int num_threads;
} mylib_barrier_t;

void mylib_init_barrier(mylib_barrier_t *b, int num_threads) {
  b->count = 0;
  b->num_threads = num_threads;
  pthread_mutex_init(&(b->count_lock), NULL);
  pthread_cond_init(&(b->ok_to_proceed), NULL);
}

void mylib_barrier (mylib_barrier_t *b) {
  pthread_mutex_lock(&(b->count_lock));
  b->count ++;
  if (b->count == b->num_threads) {
    b->count = 0;
    pthread_cond_broadcast(&(b->ok_to_proceed));
  }
  else{
    while (0 != pthread_cond_wait(&(b->ok_to_proceed), &(b->count_lock)));
  }
  pthread_mutex_unlock(&(b->count_lock));
}


#endif //HELPERS_H
