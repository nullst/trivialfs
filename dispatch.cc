#include <string>
#include <set>
#include <algorithm>
#include <functional>

#include "dispatch.h"

// std::set<std::string> dispatcher::filesWithTag(const std::string& t) const {
//   relation rel("", t);
//   std::set<std::string> result;
//   std::pair<relations_by_tag::iterator, relations_by_tag::iterator> range;
//   range = tag_relations_.equal_range(rel);
//   std::transform(range.first, range.second,
// 		 std::inserter(result, result.begin()), 
// 		 std::mem_fun_ref(&relation::file));
//   return result;
// }
// std::set<std::string> dispatcher::tagsOfFile(const std::string& f) const {
//   relation rel(f, "");
//   std::set<std::string> result;
//   std::pair<relations_by_file::iterator, relations_by_file::iterator> range;
//   range = file_relations_.equal_range(rel);
//   std::transform(range.first, range.second,
// 		 std::inserter(result, result.begin()),
// 		 std::mem_fun_ref(&relation::tag));
//   return result;
// }

void dispatcher::defineFile(const std::string& f){
  if(isTagDefined(f)) return;
  if(isFileDefined(f)) return;
  
  fileid id = (fileid) files_count;
  files_ids[f] = id;
  files_names.push_back(f);
  files_count++;

  // empty list of tags
  tags_of_file.push_back(std::vector<bool>(tags_count, false));
  // modify every tag to have the desired number of possible files
  for(size_t i = 0; i < tags_count; ++i){
    files_with_tag[i].push_back(false);
  }
}

void dispatcher::defineTag(const std::string& t){
  if(isFileDefined(t)) return;
  if(isTagDefined(t)) return;

  tagid id = (tagid) tags_count;
  tags_ids[t] = id;
  tags_names.push_back(t);
  tags_count++;

  // no files are tagged with this tag yet
  files_with_tag.push_back(std::vector<bool>(files_count, false));
  // any file can now be tagged with newly created tag, so we need a slot to do it
  for(size_t i = 0; i < files_count; ++i){
    tags_of_file[i].push_back(false);
  }
}
void dispatcher::link(const std::string& f, const std::string& t){
  if(!isFileDefined(f) || !isTagDefined(t)) return;
  fileid f_id = files_ids[f];
  tagid t_id = tags_ids[t];
  tags_of_file[f_id][t_id] = true;
  files_with_tag[t_id][f_id] = true;
}

void dispatcher::reset(void){
  files_count = 0;
  tags_count = 0;
  
  tags_ids.clear();
  files_ids.clear();
  files_with_tag.clear();
  tags_of_file.clear();
}
