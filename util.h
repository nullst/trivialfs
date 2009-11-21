#ifndef __UTIL_H
#define __UTIL_H

#include <string>
#include <vector>

#include "dispatch.h"

std::vector<std::string> splitPath(const std::string&);
std::pair< std::set<std::string>, std::set<std::string> > directoryStructure(const dispatcher& , std::set<std::string>);

#endif /* __UTIL_H */