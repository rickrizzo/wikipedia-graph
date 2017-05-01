#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
 #include <cstdlib>

using namespace std;

int main() {
  // Make Directory
  system("mkdir -p article");
  for( char m = '0'; m<='z'; ++m) {
    for( char n = '0'; n<='z'; ++n) {
      stringstream stream;
      stream << "mkdir -p article/" << m << n;
      system(stream.str().c_str());
      if (n == '9') { n = '`'; }
    }
    if (m == '9') { m = '`'; }
  }
  return EXIT_SUCCESS;
}
