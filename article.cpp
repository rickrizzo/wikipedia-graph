#include <string>
#include <vector>
#include <cstring>
#include "article.h"

void Article::setTitle(std::string title) {

  strncpy(this->title.t,title.c_str(), 100);
}

std::string Article::getTitle() {
  return std::string(this->title.t);
}

StringA Article::getTitleA() {
  return this->title;
}

void Article::addLinks(std::string link) {
  StringA l;
  strncpy(l.t,link.c_str(), 100);
  links.push_back(l);
}
void Article::addLinkedTo(std::string link) {
  StringA l;
  strncpy(l.t,link.c_str(), 100);
  linkedTo.push_back(l);
}

std::vector<StringA> Article::getLinks() {
  return links;
}

std::vector<StringA> Article::getLinkedTo() {
  return linkedTo;
}

std::string StringAtoString(StringA s){
  return std::string(s.t);
}
