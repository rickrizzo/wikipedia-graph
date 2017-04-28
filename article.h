#include <string>
#include <vector>

#ifndef ARTICLE_H
#define ARTICLE_H

struct StringA {
  char t[100];
};


class Article {
private:
  StringA title;
  std::vector<StringA> links;
public:
  void setTitle(std::string title);
  std::string getTitle();
  void addLinks(std::string link);
  std::vector<StringA> getLinks();
};

#endif // ARTICLE_H
