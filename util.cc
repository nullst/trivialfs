#include <string>
#include <set>
#include <vector>
#include <utility>
#include <algorithm>

#include "dispatch.h"
#include "parser.h"
#include "util.h"

// path ("/a/f/sad/sdf")
// is file "sdf" that is tagged as "a", "f", and "sad"
std::vector<std::string> splitPath(const std::string& path){
  // TODO: read about boost's string utilies
  // (specifically about boost::split)

  std::vector<std::string> result;
  // note: first character in string must be '/'
  const char* first = path.c_str();
  const char* last = first + path.size();
  const char* current = last;
  while(current >= first){
    if(*current == '/'){
      result.push_back(std::string(current + 1, last));
      last = current;
    }
    --current;
  }
  return result;
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

bool doesFileExist(const dispatcher& disp, std::set<std::string> tags, std::string filename){
  if(!disp.isFileDefined(filename)) return false;
  std::set<std::string> filetags = disp.tagsOfFile(filename);
  return std::includes(filetags.begin(), filetags.end(),
		       tags.begin(), tags.end());
}

void loadTags(dispatcher& disp, const std::string& path){
  parser par(path);
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
}

std::string extractFilename(std::vector<std::string>& v){
  std::string filename = v.back();
  v.pop_back();
  return filename;
}

