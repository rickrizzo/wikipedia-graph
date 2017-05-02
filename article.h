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

std::string StringAtoString(StringA s);

class Article {
private:
  StringA title;
  std::vector<StringA> links;
  std::vector<StringA> linkedTo;
public:
  void setTitle(std::string title);
  std::string getTitle();
  StringA getTitleA();
  void addLinks(std::string link);
  void addLinkedTo(std::string link);

  std::vector<StringA> getLinks();
  std::vector<StringA> getLinkedTo();
};

#endif // ARTICLE_H
