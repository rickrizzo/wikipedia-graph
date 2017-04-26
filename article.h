#include <string>
#include <vector>

class Article {
private:
  std::string title;
  std::vector<char*> links;
public:
  void setTitle(std::string title);
  void getTitle();
  void addLinks(std::string link);
  void getLinks();
};
