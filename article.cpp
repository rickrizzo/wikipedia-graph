#include <string>
#include <vector>
#include "article.h"

void Article::setTitle(std::string title) {
  this.title = title;
}

void Article::addLinks(std::string link) {
  links.push_back(link);
}
