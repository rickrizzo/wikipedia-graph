#ifndef _ARTICLE_H_
#define _ARTICLE_H_

#include <string>
#include <vector>

class Article {
private:
  std::string title;
  std::vector<char*> links;
public:
  void setTitle(std::string title);
  std::string getTitle();
  void addLinks(std::string link);
  std::vector<char*> getLinks();
};

#endif
