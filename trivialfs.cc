#include <cstdio>
#include <set>
#include <vector>
#include <functional>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <utility>

#include "dispatch.h"
#include "parser.h"
#include "util.h"

struct printer
  : public std::binary_function<std::string, bool, void>
{
  void operator() (const std::string& s, bool p) const{
    std::printf("%s", s.c_str());
    if(p) std::putchar('/');
    std::putchar('\n');
  }
};

