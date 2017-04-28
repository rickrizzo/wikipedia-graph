#include <string>
#include <vector>
#include "article.h"

void Article::setTitle(std::string title) {
  this->title = title;
}

std::string Article::getTitle() {
  return this->title;
}

void Article::addLinks(std::string link) {
  links.push_back(link.c_str());
}

std::vector<char*> Article::getLinks() {
  return links;
}
