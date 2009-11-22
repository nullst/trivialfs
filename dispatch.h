#ifndef __DISPATCH_H
#define __DISPATCH_H

#include <string>
#include <set>
#include <functional>

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
  // used like functors for std::accumulate
  std::set<std::string> tagsIntersectionIter(std::set<std::string> sum, std::string current) const;
  std::set<std::string> filesUnionIter(std::set<std::string> sum, std::string current) const;
  
public:

  dispatcher(void) : tags_(), files_(), file_relations_(), tag_relations_() { }

  std::set<std::string> tags(void) const {
    return tags_;
  }
  std::set<std::string> files(void) const {
    return files_;
  }
  
  bool isTagDefined(const std::string& t) const {
    std::set<std::string>::iterator it = tags_.find(t);
    return it != tags_.end();
  }
  bool isFileDefined(const std::string& f) const {
    std::set<std::string>::iterator it = files_.find(f);
    return it != files_.end();
  }

  std::set<std::string> filesWithTag(const std::string& t) const;
  std::set<std::string> tagsOfFile(const std::string& f) const;

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
  void defineFile(const std::string& f);
  void defineTag(const std::string& t);
  void link(const std::string& f, const std::string& t);

  void reset(void);
  
};

#endif /* __DISPATCH_H */
