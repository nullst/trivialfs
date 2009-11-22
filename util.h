#ifndef __UTIL_H
#define __UTIL_H

#include <string>
#include <vector>

#include "dispatch.h"
std::vector<std::string> splitPath(const std::string&);
static inline std::vector<std::string> splitPath(const char *p){
  return splitPath(std::string(p));
}
std::pair< std::set<std::string>, std::set<std::string> > directoryStructure(const dispatcher& , std::set<std::string>);
bool doesFileExist(const dispatcher&, std::set<std::string> tags, std::string filename);

std::string extractFilename(std::vector<std::string>&);

void loadTags(dispatcher& disp, const std::string& path);

#endif /* __UTIL_H */
