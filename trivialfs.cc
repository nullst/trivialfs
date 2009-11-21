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

// path ("/a/f/sad/sdf")
// is file "sdf" that is tagged as "a", "f", and "sad"
std::pair< std::set<std::string>, std::string> splitPath(const std::string& path){
  // TODO: read about boost's string utilies
  // (specifically about boost::split)
  std::set<std::string> tags;

  const char* first = path.c_str();
  const char* last = first + path.size();
  const char* current = last;
  while(*current != '/') --current;
  std::string name(current + 1, last);
  last = current--;
  while(current >= first){
    if(*current == '/'){
      tags.insert(std::string(current + 1, last));
      last = current;
    }
    --current;
  }
  return std::make_pair(tags, name);
}

// first part of result are subdirectories:
//   for every file that has all specialized tags
//   if it has another tag, there should be subdirectory called like this tag
//   for example: if file1 has tags a, b, and c; file2 has b and c
//   that in directory "/b" should be subdirs a and c
//   and in "/b/c" should exist subdirectory "a" (in which only file1 will be)
// second part of result is just intersection of given tags
// using example upper, for directory "/b" it should be file1 and file2
// while for directory "/a/b" and "/a" it will be file1
std::pair< std::set<std::string>, std::set<std::string> > directoryStructure(const dispatcher& disp, std::set<std::string> tags){
  if(tags.empty()){
    // root directory: all files and all tags
    std::set<std::string> files = disp.files();
    std::set<std::string> subdirs = disp.tags();
    return std::make_pair(subdirs, files);
  }
  std::set<std::string> files = disp.tagsIntersection(tags);
  std::set<std::string> tags_union = disp.filesUnion(files);
  std::set<std::string> subdirs;
  std::set_difference(tags_union.begin(), tags_union.end(),
		      tags.begin(), tags.end(),
		      std::inserter(subdirs, subdirs.begin()));
  return std::make_pair(subdirs, files);
}

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
  parser par("./.tags");
  parser::config conf = par.parse();
  parser::config::iterator it = conf.begin();
  for(; it != conf.end(); ++it){
    parser::name name = it->first;
    parser::tags tags = it->second;
    disp.defineFile(name);
    parser::tags::iterator tag_iter = tags.begin();
    for(; tag_iter != tags.end(); ++tag_iter){
      std::string tag(*tag_iter);
      disp.defineTag(tag);
      disp.link(name, tag);
    }
  }
  std::printf(".tags loaded\n");

  if(argc < 2){
    std::printf("Run ./a.out /directory/like/described to test it contents\n");
    std::exit(1);
  }
  std::pair< std::set<std::string>, std::string> path = splitPath(std::string(argv[1]));
  std::set<std::string> tags(path.first);
  tags.insert(path.second);
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
