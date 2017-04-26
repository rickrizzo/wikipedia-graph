#include <string>
#include <vector>

class Article {
private:
  std::string title;
  std::vector<std::string> links;
public:
  void setTitle(std::string title);
  void addLinks(std::string link);
};
