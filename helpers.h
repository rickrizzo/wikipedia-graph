#ifndef HELPERS_H
#define HELPERS_H

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cstdlib>

using namespace std;

string alphanumeric = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

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


#endif //HELPERS_H
