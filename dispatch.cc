#include <string>
#include <set>
#include <algorithm>
#include <functional>

#include "dispatch.h"

// this two functions are used like std::accumulate ones
std::set<std::string> dispatcher::tagsIntersectionIter(std::set<std::string> sum, std::string current) const {
  std::set<std::string> current_set;
  std::set<std::string> result;
  current_set = filesWithTag(current);
  std::set_intersection(sum.begin(), sum.end(),
			current_set.begin(), current_set.end(),
			std::inserter(result, result.begin()));
  return result;
}
std::set<std::string> dispatcher::filesUnionIter(std::set<std::string> sum, std::string current) const {
  std::set<std::string> current_set;
  std::set<std::string> result;
  current_set = tagsOfFile(current);
  std::set_union(sum.begin(), sum.end(),
		 current_set.begin(), current_set.end(),
		 std::inserter(result, result.begin()));
  return result;
}

std::set<std::string> dispatcher::filesWithTag(const std::string& t) const {
  relation rel("", t);
  std::set<std::string> result;
  std::pair<relations_by_tag::iterator, relations_by_tag::iterator> range;
  range = tag_relations_.equal_range(rel);
  std::transform(range.first, range.second,
		 std::inserter(result, result.begin()), 
		 std::mem_fun_ref(&relation::file));
  return result;
}
std::set<std::string> dispatcher::tagsOfFile(const std::string& f) const {
  relation rel(f, "");
  std::set<std::string> result;
  std::pair<relations_by_file::iterator, relations_by_file::iterator> range;
  range = file_relations_.equal_range(rel);
  std::transform(range.first, range.second,
		 std::inserter(result, result.begin()),
		 std::mem_fun_ref(&relation::tag));
  return result;
}

void dispatcher::defineFile(const std::string& f){
  if(isTagDefined(f)) return;
  files_.insert(f);
}
void dispatcher::defineTag(const std::string& t){
  if(isFileDefined(t)) return;
  tags_.insert(t);
}
void dispatcher::link(const std::string& f, const std::string& t){
  if(!isFileDefined(f) || !isTagDefined(t)) return;
  relation rel(f, t);
  file_relations_.insert(rel);
  tag_relations_.insert(rel);
}

void dispatcher::reset(void){
  tags_.clear();
  files_.clear();
  file_relations_.clear();
  tag_relations_.clear();
}
