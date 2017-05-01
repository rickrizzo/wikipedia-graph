#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include "helpers.h"

using namespace std;
#define FILELIMIT 30
#define FILEPATH "enwiki-mini.xml"

int main() {
  // Variables
  int fileCount = 0;
  string line = "";
  ifstream file(FILEPATH);

  string titleStart("<title>");
  string pageEnd("</page>");
  string tagFile(":File:");
  string tagMedia("media:");
  string tagSpecial("Special:");
  string tagCategory("Category:");

  // Read File
  if(file.is_open()) {
    while (getline(file, line)) {
      size_t startString = line.find_first_not_of(" \t");
      //
      if (startString != string::npos &&
        startString + titleStart.length() <  line.length() &&
        line.compare(startString, titleStart.length(), titleStart) == 0) {

        fileCount += 1;
        string title = line.substr(startString+7, line.length() - (15+startString));

        // create file object
        ofstream articleFile(makeArticleFilename(title).c_str(), ofstream::out);
        articleFile << "title: "<< title <<endl;
        cout << "title: "<< title <<endl;

        while (getline(file, line)) {
          size_t pos = 0;
          size_t linkStart = line.find("[[", pos); // Find the start of the link
          size_t linkEnd = line.length();
          size_t pipe = line.length();
          while (linkStart != string::npos) {
            size_t linkStart = line.find("[[", pos); // Get the next link
            if (linkStart == string::npos){
              break;
            }
            linkStart += 2; // skip the [[
            linkEnd = line.find("]]", linkStart); // Find the end of the link
            pos = linkEnd + 2; // Move the current position to the end of the link
            pipe = line.find("|", linkStart); // Check if there is a pipe in the link
            if (pipe < linkEnd){
              linkEnd = pipe; // We don't need the piped part
            }

            if (line[linkStart] == '#' || //anchor link
                (line[linkStart] == '{' && line[linkStart+1] == '{') ||  //discussion page
                (linkStart+tagFile.length()<line.length() && line.compare(linkStart, tagFile.length(), tagFile) == 0) || // File
                (linkStart+tagMedia.length()<line.length() && line.compare(linkStart, tagMedia.length(), tagMedia) == 0) || // Media
                (linkStart+tagSpecial.length()<line.length() && line.compare(linkStart, tagSpecial.length(), tagSpecial) == 0) || // Special
                (linkStart+tagCategory.length()<line.length() && line.compare(linkStart, tagCategory.length(), tagCategory) == 0) // Category
              )
            // ignore, not a valid page
            {
              continue;
            }
            if (line[linkStart+1] == '/') // subpage, prepend this page's title
            {
              articleFile << title << line.substr(linkStart, linkEnd - linkStart) << endl;
            }
            else{
              articleFile << line.substr(linkStart, linkEnd - linkStart) << endl;
            }
          }

          startString = line.find_first_not_of(" \t");

          if(startString != string::npos &&
            startString + pageEnd.length() <= line.length() &&
            line.compare(startString, pageEnd.length(), pageEnd) == 0) {
              break;
            }
        }

        articleFile.close();
        if(fileCount == FILELIMIT) { break; }
      }
    }
    file.close();
  }
  cout << "num files: " << fileCount << endl;
  // Finish
  return EXIT_SUCCESS;
}
