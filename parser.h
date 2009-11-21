#ifndef __PARSER_H
#define __PARSER_H

#include <set>
#include <string>
#include <utility>

class parser
{
public:
  typedef const char* position;
  typedef std::string name;
  typedef std::set<std::string> tags;
  typedef std::pair<name, tags> section;
  typedef std::set<section> config;
  
private:
  char *contents;
  position p;

  tags read_tags(void);
  name read_name(void);
  section read_section(void);
  config read_all(void);

  void skip_spaces(void);
  name cut_string(position start, position finish);
  bool is_space(char);
  bool is_eof(void);

public:
  
  parser(const std::string& path);
  ~parser(void);

  config parse(void){
    if(contents == NULL) return config();
    p = contents;
    return read_all();
  }

};

#endif /* __PARSER_H */
