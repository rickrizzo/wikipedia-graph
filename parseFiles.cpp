#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <cstring>

#define FILELIMIT 30
#define FILEPATH "enwiki-mini.xml"

std::string getArticleFilename(std::string title) {
  std::string firstLetters = title.substr(0, 2);
  // make first two letters of title lowercase
  std::transform(firstLetters.begin(), firstLetters.end(), firstLetters.begin(), ::tolower);
  std::string base = "article/";
  std::string ending = ".txt";
  base.append(firstLetters);
  base.append("/");
  base.append(title);
  base.append(ending);
  return base;
}

// FIXME
// Move to .h
std::string &ltrim(std::string &s);
std::string &rtrim(std::string &s);
std::string &trim(std::string &s);

int main() {
  // Variables
  int fileCount;
  std::string line;
  std::ifstream file(FILEPATH);

  // Make Directory
  system("mkdir -p article");
  for( char m = '0'; m<='z'; ++m) {
    for( char n = '0'; n<='z'; ++n) {
      std::stringstream stream;
      stream << "mkdir -p article/" << m << n;
      system(stream.str().c_str());
      if (n == '9') { n = '`'; }
    }
    if (m == '9') { m = '`'; }
  }

  // Read File
  if(file.is_open()) {
    while(getline(file, line)) {
      if(line.find("<title>") != std::string::npos) {
        line = trim(line);
        std::string title = line.substr(7, line.length() - 15);

        // create file object
        std::ofstream articleFile(getArticleFilename(title).c_str(), std::ofstream::out);

        while(getline(file, line)) {
		if(line.find("[[") != std::string::npos) {
			bool readingLink = false;
			int linkStart, linkEnd;
			for(int i = 0; i < line.length(); i++) {
				if(!readingLink && line[i] == '[' && line[i+1] == '[') {
					linkStart = i+2;
					readingLink = true;
					i++;
				}
				if(readingLink && line[i] == ']' && line[i+1] == ']') {
					readingLink = false;
					linkEnd = i;
					articleFile << line.substr(linkStart, linkEnd - linkStart) << std::endl;
				}
			}
		}  
		if(line.find("</page>") != std::string::npos) { break; }
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

std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}
