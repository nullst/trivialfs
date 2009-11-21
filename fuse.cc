#define FUSE_USE_VERSION 28

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <fuse.h>
#include <fuse_opt.h>

#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>

#include "dispatch.h"
#include "util.h"
#include "parser.h"

time_t mount_time;
std::string storage_path;
static dispatcher disp;

bool is_root(const char *path){
  return (*path == '/') && (*(path + 1) == '\0');
}

static int tri_getattr(const char *path, struct stat *st){
  memset(st, 0, sizeof(struct stat));

  std::string filename;
  bool is_directory = false;
  bool is_file = false;
  if(is_root(path)){
    is_directory = true;
  }else{
    std::vector<std::string> tags = splitPath(std::string(path));
    filename = tags.back();
    tags.pop_back();

    // now tags should be really vector of valid tags
    for(std::vector<std::string>::iterator it = tags.begin(); it != tags.end(); ++it){
      if(!disp.isTagDefined(*it)) return -ENOENT;
    }
    
    /* there is no files and tags with the same names
       so when last element in path is tag, it's directory
    */
    if(disp.isTagDefined(filename)) is_directory = true;
    if(disp.isFileDefined(filename)) is_file = true;
  }
  
  if(is_directory){
    st->st_mode = S_IFDIR | 0775;
    st->st_nlink = 2;
  }else if(is_file){
    st->st_mode = S_IFLNK | 0400;
    st->st_nlink = 1;
    // +1 for '/' between storage path and filename
    st->st_size = storage_path.size() + filename.size() + 1;
  }else{
    /* there is no such file at all */
    return -ENOENT;
  }
  /* writing defaults */
  struct fuse_context *cx = fuse_get_context();
  st->st_uid = cx->uid;
  st->st_gid = cx->gid;
  
  st->st_atime = mount_time;
  st->st_mtime = mount_time;
  st->st_ctime = mount_time;
  return 0;
}

static int tri_opendir(const char *path, struct fuse_file_info *fi){
  if(is_root(path)) return 0;
  std::vector<std::string> tags = splitPath(std::string(path));
  for(std::vector<std::string>::iterator it = tags.begin(); it != tags.end(); ++it){
    if(!disp.isTagDefined(*it)) return -ENOENT;
  }
  return 0;
}

static int tri_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi){
  std::pair< std::set<std::string>, std::set<std::string> > dirs;
  if(is_root(path)){
    dirs.first = disp.tags();
    dirs.second = disp.files();
  }else{ 
    std::vector<std::string> tags_vec = splitPath(std::string(path));
    std::set<std::string> tags;
    std::copy(tags_vec.begin(), tags_vec.end(),
	      std::inserter(tags, tags.begin()));
    for(std::set<std::string>::iterator it = tags.begin(); it != tags.end(); ++it){
      if(!disp.isTagDefined(*it)) return -ENOENT;
    }
    dirs = directoryStructure(disp, tags);
  }

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  
  std::set<std::string>::iterator it;
  // directories (tags)
  it = dirs.first.begin();
  for(; it != dirs.first.end(); ++it){
    filler(buf, it->c_str(), NULL, 0);
  }
  // files (symlinks)
  it = dirs.second.begin();
  for(; it != dirs.second.begin(); ++it){
    filler(buf, it->c_str(), NULL, 0);
  }
  return 0;
}

static int tri_open(const char *path, struct fuse_file_info *fi){
  std::vector<std::string> tags_vec = splitPath(std::string(path));
  std::set<std::string> tags;
  std::string filename = tags_vec.back();
  tags_vec.pop_back();
  std::copy(tags_vec.begin(), tags_vec.end(),
	    std::inserter(tags, tags.begin()));
  bool exist = doesFileExist(disp, tags, filename);
  if(!exist){
    return -ENOENT;
  }
  /* if file is opened not only for reading */
  if((fi->flags & 3) != O_RDONLY){
    return -EACCES;
  }
  /* all is ok */
  return 0;
}

static int tri_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
  /* here I assume that if someone is reading file, he's opened it earlier
     and there is no need to check existence of file
  */
  std::string path_s(path);
  std::string name = path_s.substr(path_s.find_last_of('/') + 1);
  std::string contents = storage_path + "/" + name;
  size_t sz = contents.size();
  if(offset > sz) return 0;
  if(offset + size > sz) size = sz - offset - 1;
  memcpy(buf, contents.substr(offset, size).c_str(), size);
  return size;
}

static struct fuse_operations tri_operations;

int main(int argc, char **argv){

  tri_operations.getattr = tri_getattr;
  tri_operations.opendir = tri_opendir;
  tri_operations.readdir = tri_readdir;
  tri_operations.open = tri_open;
  tri_operations.read = tri_read;

  mount_time = time(NULL);
  
  if(argc < 3){
    printf("Usage:\n"
	   "trivialfs /path/to/storage /mount/point\n");
    exit(1);
  }
  storage_path = std::string(argv[1]);
  loadTags(disp, storage_path + "/.tags");
  struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
  fuse_opt_add_arg(&args, argv[0]);
  fuse_opt_add_arg(&args, argv[2]);
  return fuse_main(args.argc, args.argv, &tri_operations, NULL);
}
