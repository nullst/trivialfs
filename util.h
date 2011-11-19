#ifndef __UTIL_H
#define __UTIL_H

#include <string>
#include <vector>

#include "dispatch.h"
std::vector<std::string> splitPath(const std::string&);
static inline std::vector<std::string> splitPath(const char *p){
  return splitPath(std::string(p));
}
std::pair< std::vector<bool>, std::vector<bool> > directoryStructure(const dispatcher& disp, const std::vector<std::string>& tags);
bool doesFileExist(const dispatcher& disp, const std::vector<std::string>& tags, const std::string& filename);

std::string extractFilename(std::vector<std::string>&);

void loadTags(dispatcher& disp, const std::string& path);

#endif /* __UTIL_H */
