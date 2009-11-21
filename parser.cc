#include <stdio.h>
#include <stdlib.h>

#include <set>
#include <utility>

#include "parser.h"

parser::parser(const std::string& path) : p(NULL), contents(NULL){
  FILE *f;
  f = fopen(path.c_str(), "r");
  if(f != NULL){
    contents = (char *) malloc(200 * 1024);
    int sz = fread(contents, 1, 200 * 1024, f);
    contents[sz] = '\0';
    fclose(f);
  }
}

parser::~parser(void){
  if(contents != NULL) free(contents);
}

parser::tags parser::read_tags(void){
  tags t;
  printf("DEBUG:: reading tags\n");
  skip_spaces();
  position token_start = p;
  while(true){
    while(*p != '}' && *p != ',') ++p;

    // *p is separator, so it should be skipped
    t.insert(cut_string(token_start, p - 1));
    // when separator is '}', tags finished
    if(*p == '}') break;
    ++p;
    
    skip_spaces();
    if(is_eof()) break;
    token_start = p;
  }
  // skipping '}'
  ++p;
  return t;
}
parser::name parser::read_name(void){
  position token_start;
  name n;
  skip_spaces();
  if(is_eof()) return name("");
  token_start = p;
  while(*p != '{'){
    ++p;
  }
  printf("DEBUG:: name skipped, %d characters\n", p - token_start);
  // this string is executed when *p == '{', which is not part of name
  n = cut_string(token_start, p - 1);
  ++p;
  return n;
}
parser::section parser::read_section(void){
  name section_name;
  section_name = read_name();
  if(is_eof()) return section();

  tags section_tags;
  section_tags = read_tags();
  return std::make_pair(section_name, section_tags);
}
parser::config parser::read_all(void){
  config result;
  section sect;
  
  sect = read_section();
  while(!is_eof()){
    printf("DEBUG:: section readed\n");
    result.insert(sect);
    sect = read_section();
  }
  return result;
}

void parser::skip_spaces(void){
  position t = p;
  while(is_space(*p)) ++p;
  printf("DEBUG:: skipped %d spaces\n", p - t);
}
parser::name parser::cut_string(parser::position st, parser::position fi){
  while(is_space(*st)) ++st;
  while(is_space(*fi)) --fi;
  return name(st, fi+1);
}
bool parser::is_space(const char c){
  return (c == ' ') || (c == '\n') || (c == '\t');
}
bool parser::is_eof(void){
  return *p == '\0';
}


