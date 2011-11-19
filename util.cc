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
// note: there is inline wrapper for const char * defined in util.h
std::vector<std::string> splitPath(const std::string& path){
  // TODO: use boost::string

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
  std::reverse(result.begin(), result.end());
  return result;
}

// AN OVERVIEW OF FILESYSTEM STRUCTURE:
// a directory is always a tag. Nested directories imply the intersection of tags.
// for example, "/math/book/algebra" is a directory for files having all three tags
// the order doesn't matter: "/algebra/book/math" will make no difference.
// Hence if we look at "/algebra/book", there should be a subdirectory corresponding to tag
// "math", if there files with all three tags (command-line completion is crucial).
// Of course, if no file has tags "algebra"
// and "economics" simultaneously, it makes no sense to create a subdirectory for "economics"
// in "/algebra/book".
// Summarizing, when the user browses the directory "/math/book", we need to provide:
//   (a) files, ie. links to real files in the storage which are tagged with "math" and "book"
//   (b) directories, which correspond to different tags with non-empty intersection with "math" and "book"
// we return the structure of the current directory (passed as the list of tags) as bitmasks corresponding
// to internal masks in dispatcher (the nth bit in the .first is set if and only if the nth tag should be presented,
// similarly for the .second, which is the bitmask of files lying in the path.
std::pair< std::vector<bool>, std::vector<bool> > directoryStructure(const dispatcher& disp, const std::vector<std::string>& tags){
  std::vector<bool> tags_by_id = disp.convertTagsToIds(tags);
  
  std::vector<bool> filelist = disp.tagsIntersection(tags_by_id);
  std::vector<bool> total_taglist = disp.filesUnion(filelist);

  for(size_t i = 0; i < total_taglist.size(); ++i){
    if(tags_by_id[i]) total_taglist[i] = false;
  }
  return std::make_pair(total_taglist, filelist);
}

// std::pair< std::set<std::string>, std::set<std::string> > directoryStructure(const dispatcher& disp, std::vector<std::string> tags){
//   if(tags.empty()){
//     // root directory: all files and all tags
//     std::set<std::string> files = disp.files();
//     std::set<std::string> subdirs = disp.tags();
//     return std::make_pair(subdirs, files);
//   }
//   std::set<std::string> files = disp.tagsIntersection(tags);
//   std::set<std::string> tags_union = disp.filesUnion(files);
//   std::set<std::string> subdirs;
//   std::set_difference(tags_union.begin(), tags_union.end(),
// 		      tags.begin(), tags.end(),
// 		      std::inserter(subdirs, subdirs.begin()));
//   return std::make_pair(subdirs, files);
// }

bool doesFileExist(const dispatcher& disp, const std::vector<std::string>& tags, const std::string& filename){
  if(!disp.isFileDefined(filename)) return false;
  return disp.hasTags(filename, tags);
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

