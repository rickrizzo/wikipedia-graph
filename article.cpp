#include <string>
#include <vector>
#include "article.h"

void Article::setTitle(std::string title) {
  strcpy(this->title.t,title.c_str());
}

std::string Article::getTitle() {
  return std::string(this->title.t);
}

void Article::addLinks(std::string link) {
  StringA l;
  strcpy(l.t,link.c_str());
  links.push_back(l);
}

std::vector<StringA> Article::getLinks() {
  return links;
}
