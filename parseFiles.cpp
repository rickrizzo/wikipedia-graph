#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define FILELIMIT 30
#define FILEPATH "/Volumes/Triforce/Parallel/enwiki-20170101-pages-articles-multistream.xml"

std::string getArticleNumber(int input) {
  std::stringstream stream;
  stream << "article/article_" << input << ".txt";
  return stream.str();
}

int main() {
  // Variables
  int fileCount = 0;
  std::string line;
  std::ifstream file(FILEPATH);

  // Read File
  if(file.is_open()) {
    while(getline(file, line)) {
      if(line.find("<page>") != std::string::npos) {
        std::ofstream articleFile(getArticleNumber(fileCount));
        fileCount++;
        while(getline(file, line)) {
          if(line.find("</page>") != std::string::npos) { break; }
          articleFile << line << std::endl;
        }
        articleFile.close();
        if(fileCount == FILELIMIT) { break; }
      }
    }
    file.close();
  }

  // Finish
  return EXIT_SUCCESS;
}
