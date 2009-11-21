#include <cstdio>
#include <set>
#include <vector>
#include <functional>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <utility>

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

class relation
{
private:
  std::string file_;
  std::string tag_;

public:
  relation(void) : file_(), tag_() { }
  relation(const std::string& f, const std::string& t) : file_(f), tag_(t) { }
  const std::string& file(void) const { return file_; }
  const std::string& tag(void) const { return tag_; }
  
  friend class file_ordering;
  friend class tag_ordering;
};

struct file_ordering :
  public std::binary_function<relation, relation, bool>
{
  bool operator() (const relation& r1, const relation& r2) const {
    return r1.file_ < r2.file_;
  }
};

struct tag_ordering :
  public std::binary_function<relation, relation, bool>
{
  bool operator() (const relation& r1, const relation& r2) const {
    return r1.tag_ < r2.tag_;
  }
};

class dispatcher
{
private:
  std::set<std::string> tags_;
  std::set<std::string> files_;
  typedef std::multiset<relation, file_ordering> relations_by_file;
  typedef std::multiset<relation, tag_ordering> relations_by_tag;
  relations_by_file file_relations_;
  relations_by_tag tag_relations_;

  // used as functor for std::accumulate
  std::set<std::string> tagsIntersectionIter(std::set<std::string> sum, std::string current) const {
    std::set<std::string> current_set;
    std::set<std::string> result;
    current_set = filesWithTag(current);
    std::set_intersection(sum.begin(), sum.end(),
			  current_set.begin(), current_set.end(),
			  std::inserter(result, result.begin()));
    return result;
  }
  std::set<std::string> filesUnionIter(std::set<std::string> sum, std::string current) const {
    std::set<std::string> current_set;
    std::set<std::string> result;
    current_set = tagsOfFile(current);
    std::set_union(sum.begin(), sum.end(),
		   current_set.begin(), current_set.end(),
		   std::inserter(result, result.begin()));
    return result;
  }
  
public:

  dispatcher(void) : tags_(), files_(), file_relations_(), tag_relations_() { }

  bool isTagDefined(const std::string& t) const {
    std::set<std::string>::iterator it = tags_.find(t);
    return it != tags_.end();
  }
  bool isFileDefined(const std::string& f) const {
    std::set<std::string>::iterator it = files_.find(f);
    return it != files_.end();
  }

  std::set<std::string> filesWithTag(const std::string& t) const {
    relation rel("", t);
    std::set<std::string> result;
    std::pair<relations_by_tag::iterator, relations_by_tag::iterator> range;
    range = tag_relations_.equal_range(rel);
    std::transform(range.first, range.second,
		   std::inserter(result, result.begin()), 
		   std::mem_fun_ref(&relation::file));
    return result;
  }
  std::set<std::string> tagsOfFile(const std::string& f) const {
    relation rel(f, "");
    std::set<std::string> result;
    std::pair<relations_by_file::iterator, relations_by_file::iterator> range;
    range = file_relations_.equal_range(rel);
    std::transform(range.first, range.second,
		   std::inserter(result, result.begin()),
		   std::mem_fun_ref(&relation::tag));
    return result;
  }

  // requests for multiple specializers

  // returns such set of files, that
  // every file has all of given tags
  // T should be any iterable container of std::string
  template <typename T>
  std::set<std::string> tagsIntersection(T list) const {
    std::set<std::string> result;
    // what is wrong with that line?
    //    result = std::accumulate(++list.begin(), list.end(), filesWithTag(*(list.begin())), tagsIntersectionIter);
    // g++ error messages are so fucking helpful
    typename T::iterator it = list.begin();
    result = filesWithTag(*it++);
    for(; it != list.end(); ++it){
      result = tagsIntersectionIter(result, *it);
    }
    return result;
  }

  // for every returned tag, there is at least one file in input
  // which has this tag
  // again, T is any iterable container of std::string
  template <typename T>
  std::set<std::string> filesUnion(T list) const {
    std::set<std::string> result;
    // see comment in tagsIntersection
    for(typename T::iterator it = list.begin(); it != list.end(); ++it){
      result = filesUnionIter(result, *it);
    }
    return result;
  }

  // note: due to representation of directories
  // there must be no file named equally like tag
  // and no tag named equally like file
  void defineFile(const std::string& f){
    if(isTagDefined(f)) return;
    files_.insert(f);
  }
  void defineTag(const std::string& t){
    if(isFileDefined(t)) return;
    tags_.insert(t);
  }
  void link(const std::string& f, const std::string& t){
    if(!isFileDefined(f) || !isTagDefined(t)) return;
    relation rel(f, t);
    file_relations_.insert(rel);
    tag_relations_.insert(rel);
  }

};

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
  std::set<std::string> files = disp.tagsIntersection(tags);
  std::set<std::string> tags_union = disp.filesUnion(files);
  std::set<std::string> subdirs;
  std::set_difference(tags_union.begin(), tags_union.end(),
		      tags.begin(), tags.end(),
		      std::inserter(subdirs, subdirs.begin()));
  return std::make_pair(subdirs, files);
}


int main(int, char**){
  dispatcher disp;
  /*
  disp.defineFile("Test.pdf");
  disp.defineFile("Test2.pdf");
  disp.defineFile("Additional library.djvu");
  disp.defineFile("index.html");

  disp.defineTag("c++");
  disp.defineTag("book");
  disp.defineTag("lisp");
  disp.defineTag("classic");

  disp.link("Test.pdf", "c++");
  disp.link("Test.pdf", "book");
  disp.link("Test2.pdf", "lisp");
  disp.link("Test2.pdf", "book");
  disp.link("Test2.pdf", "classic");
  disp.link("Additional library.djvu", "classic");
  disp.link("Additional library.djvu", "book");

  std::set<std::string> files;
  std::set<std::string> files_list;
  files_list.insert("Test.pdf");
  files_list.insert("Additional library.djvu");
  files = disp.filesUnion(files_list);
  std::copy(files.begin(), files.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
  */
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
      //      std::printf("File '%s' linked with tag '%s'\n", name.c_str(), tag.c_str());
    }
  }
  std::printf(".tags loaded\n");
  
  return 0;
  /*
  std::pair<std::set<std::string>, std::string> p;
  p = splitPath("/");
  std::cout << "Tags are: " << std::endl;
  std::copy(p.first.begin(), p.first.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
  std::cout << "Name is " << p.second << std::endl;
  */
}
