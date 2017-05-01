#ifndef _ARTICLE_H_
#define _ARTICLE_H_

#include <string>
#include <vector>

struct StringA {
  char t[100];
};

struct ArticleMatch {
  StringA source;
  StringA link;
};

class Article {
private:
  StringA title;
  std::vector<StringA> links;
public:
  void setTitle(std::string title);
  std::string getTitle();
  StringA getTitleA();
  void addLinks(std::string link);

  std::vector<StringA> getLinks();
};

#endif // ARTICLE_H
