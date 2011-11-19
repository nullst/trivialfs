#ifndef __DISPATCH_H
#define __DISPATCH_H

#include <string>
#include <set>
#include <map>
#include <functional>
#include <vector>
#include <stdio.h>

class dispatcher
{
private:

  // we need many-to-many relationship.
  // we implement it using correspondence with strings and numbers (ids) to use them as indices in vectors
  typedef size_t fileid;
  typedef size_t tagid;

  size_t files_count;
  size_t tags_count;
  std::map<std::string, fileid> files_ids;
  std::map<std::string, tagid> tags_ids;
  std::vector<std::string> files_names;
  std::vector<std::string> tags_names;

  std::vector< std::vector<bool> > tags_of_file;
  std::vector< std::vector<bool> > files_with_tag;

  // inplace bitwise and for vector<bool>
  void mask_and(std::vector<bool>& a, const std::vector<bool>& b) const {
    for(size_t i = 0; i < a.size(); ++i){
      a[i] = a[i] & b[i];
    }
  }
  void mask_or(std::vector<bool>& a, const std::vector<bool>& b) const {
    for(size_t i = 0; i < a.size(); ++i){
      a[i] = a[i] | b[i];
    }
  }
  
public:

  dispatcher(void) :
    files_count(0), tags_count(0), files_ids(), tags_ids(),
    tags_of_file(), files_with_tag()
  { }

  std::string filename(fileid f) const {
    return files_names[f];
  }
  std::string tagname(tagid t) const {
    return tags_names[t];
  }
  
  bool isTagDefined(const std::string& t) const {
    std::map<std::string, tagid>::const_iterator it = tags_ids.find(t);
    return it != tags_ids.end();
  }
  bool isFileDefined(const std::string& f) const {
    std::map<std::string, fileid>::const_iterator it = files_ids.find(f);
    return it != files_ids.end();
  }
  bool validTags(const std::vector<std::string>& tags) const {
    for(std::vector<std::string>::const_iterator it = tags.begin(); it != tags.end(); ++it){
      if(!isTagDefined(*it)) return false;
    }
    return true;

  }

  // unfortunately we have to break down consistency by using both strings and ids for files and tags
  // this function (unlike standard ones) takes a vector of strings and checks whether the selected file
  // has all of them on it
  bool hasTags(const std::string& filename, const std::vector<std::string>& tags) const {
    if(!isFileDefined(filename)) return false;
    
    fileid id = files_ids.find(filename)->second;
    const std::vector<bool>& filetags = tags_of_file[id];
    for(size_t i = 0; i < tags.size(); ++i){
      if(!isTagDefined(tags[i])){
	// something bad happened here. There is no exception handling mechanism, so we
	// just pretend the only problem is that the file doesn't have such a tag
	// (which is, by the way, true, since the tag is not registered, but situation is still very strange)
	return false;
      }
      tagid tid = tags_ids.find(tags[i])->second;
      if(filetags[tid] == false) return false;
    }
    return true;
  }

  // the hopelessly specialized function. Of course, to create even an illusion of generality,
  // it would be neccessary to do something with this; for example, to create an analogous function
  // for converting a list of filenames to ids.
  std::vector<bool> convertTagsToIds(const std::vector<std::string>& tags) const {
    std::vector<bool> result(tags_count, false);
    for(size_t i = 0; i < tags.size(); ++i){
      result[ tags_ids.find(tags[i])->second ] = true;
    }
    return result;
  }
  // returns the list of files (as vector<bool> by ids) such that any file from
  // the output has all passed tags
  std::vector<bool> tagsIntersectionIds(const std::vector<tagid>& tags) const {
    std::vector<bool> result(files_count, true);
    for(size_t i = 0; i < tags.size(); ++i){
      tagid current_id = tags[i];
      mask_and(result, files_with_tag[current_id]);
    }
    return result;
  }
  std::vector<bool> tagsIntersection(const std::vector<bool>& tags) const {
    std::vector<tagid> result;
    for(size_t i = 0; i < tags.size(); ++i){
      if(tags[i]) result.push_back(i);
    }
    return tagsIntersectionIds(result);
  }

  // from the list of files produces the vector of tags (represented by ids)
  // such that for every tag in the output there is at least one file which is tagged by it
  std::vector<bool> filesUnionIds(const std::vector<fileid>& files) const {
    std::vector<bool> result(tags_count, false);
    for(size_t i = 0; i < files.size(); ++i){
      fileid current_id = files[i];
      mask_or(result, tags_of_file[current_id]);
    }
    return result;
  }
  std::vector<bool> filesUnion(const std::vector<bool>& files) const {
    std::vector<fileid> result;
    for(size_t i = 0; i < files.size(); ++i){
      if(files[i]) result.push_back(i);
    }
    return filesUnionIds(result);
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
