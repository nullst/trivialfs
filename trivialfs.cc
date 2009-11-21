#include <cstdio>
#include <set>
#include <vector>
#include <functional>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <utility>

#include "dispatch.h"
#include "parser.h"
#include "util.h"

struct printer
  : public std::binary_function<std::string, bool, void>
{
  void operator() (const std::string& s, bool p) const{
    std::printf("%s", s.c_str());
    if(p) std::putchar('/');
    std::putchar('\n');
  }
};

int main(int argc, char** argv){
  dispatcher disp;

  std::printf("Trying to parse .tags\n");
  std::printf(".tags loaded\n");

  if(argc < 2){
    std::printf("Run ./a.out /directory/like/described to test it contents\n");
    std::exit(1);
  }
  std::vector<std::string> path = splitPath(std::string(argv[1]));
  std::set<std::string> tags;
  std::copy(path.begin(), path.end(), std::inserter(tags, tags.begin()));
  {
    std::set<std::string>::iterator it = tags.find("");
    if(it != tags.end()){
      tags.erase(it);
    }
  }
  std::pair< std::set<std::string>, std::set<std::string> > contents = directoryStructure(disp, tags);
  if(contents.second.empty()){
    std::printf("No such path\n");
  }else{
    std::for_each(contents.first.begin(), contents.first.end(),
		  std::bind2nd(printer(), true));
    std::for_each(contents.second.begin(), contents.second.end(),
		  std::bind2nd(printer(), false));
  }
  
  return 0;
}
