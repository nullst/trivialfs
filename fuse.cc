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

std::string storage_path;

// neccessary attributes applied to all virtual files
time_t mount_time;
uid_t uid;
gid_t gid;

// dispatcher is an engine for all tag operations (intersections and so on)
static dispatcher disp;

// auxiliary function that's used only to check whether we can read .tags
// in particular it checks whether file exists
bool isFileReadable(std::string path){
  FILE *f;
  f = fopen(path.c_str(), "r");
  if(f == NULL) return false;
  fclose(f);
  return true;
}

static void initDefaults(void){
  disp.reset();
  loadTags(disp, storage_path + "/.tags");
  mount_time = time(NULL);
  uid = getuid();
  gid = getgid();
}

bool is_root(const char *path){
  return (*path == '/') && (*(path + 1) == '\0');
}

static int tri_getattr(const char *path, struct stat *st){
  memset(st, 0, sizeof(struct stat));

  // last element in path (it may be name of tag, actually)
  std::string filename;
  bool is_directory = false;
  bool is_file = false;
  
  if(is_root(path)){
    is_directory = true;
  }else{
    std::vector<std::string> tags = splitPath(path);
    filename = extractFilename(tags);

    // now tags should be really vector of valid tags
    
    /* we assume there are no files and tags named the same
       so when last element in path is tag, it's directory
    */
    if(disp.isTagDefined(filename)){
      if(!disp.validTags(tags)) return -ENOENT;
      is_directory = true;
    }else if(disp.isFileDefined(filename)){
      if(!doesFileExist(disp, tags, filename)){
	return -ENOENT;
      }      
      is_file = true;
    }
  }
  
  if(is_directory){
    st->st_mode = S_IFDIR | 0700;
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
  st->st_uid = uid;
  st->st_gid = gid;
  
  st->st_atime = mount_time;
  st->st_mtime = mount_time;
  st->st_ctime = mount_time;
  return 0;
}

static int tri_opendir(const char *path, struct fuse_file_info *fi){
  if(is_root(path)) return 0;
  // all elements in path must be valid tags
  std::vector<std::string> tags = splitPath(path);
  if(!disp.validTags(tags)) return -ENOENT;

  return 0;
}

static int tri_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi){
  // directories are always tags, files are always files
  // so we will use a pair consisting of tags we are to show as subdirectories
  // and files which lie in the current directory
  // and we are going to keep the list by (internal to dispatcher) ids
  // so dirs.first is a bool vector which has 'true' at nth place if the ith tag is
  // to be shown

  std::vector<std::string> tags = splitPath(path);
  if(!disp.validTags(tags)) return -ENOENT;
  
  std::pair< std::vector<bool>, std::vector<bool> > structure = directoryStructure(disp, tags);
  std::vector<bool> dirs = structure.first;
  std::vector<bool> files = structure.second;
  
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

  for(size_t i = 0; i < dirs.size(); ++i){
    if(dirs[i]){
      filler(buf, disp.tagname(i).c_str(), NULL, 0);
    }
  }
  for(size_t i = 0; i < files.size(); ++i){
    if(files[i]){
      filler(buf, disp.filename(i).c_str(), NULL, 0);
    }
  }

  return 0;
}

static int tri_open(const char *path, struct fuse_file_info *fi){
  std::vector<std::string> tags = splitPath(path);
  std::string filename = extractFilename(tags);
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

static int tri_readlink(const char *path, char *buf, size_t size){

  std::vector<std::string> path_v = splitPath(path);
  std::string filename = extractFilename(path_v);
  
  if(!doesFileExist(disp, path_v, filename)) return -ENOENT;
  
  std::string contents = storage_path + "/" + filename;
  // note: "+ 1" and "- 1" are here to include trailing '\0' byte
  memcpy(buf, contents.c_str(),
	 std::min(size, contents.size() + 1));
  return 0;
}

static int tri_create(const char *path, mode_t mode, struct fuse_file_info *fi){

  if(std::string(path) == "/reload") initDefaults();
  return -ENOENT;
}


static struct fuse_operations tri_operations;

int main(int argc, char **argv){

  tri_operations.getattr = tri_getattr;
  tri_operations.opendir = tri_opendir;
  tri_operations.readdir = tri_readdir;
  tri_operations.open = tri_open;
  tri_operations.readlink = tri_readlink;
  tri_operations.create = tri_create;

  if(argc < 3){
    printf("Usage:\n"
	   "trivialfs /path/to/storage /mount/point\n");
    exit(1);
  }
  
  storage_path = std::string(argv[1]);
  // TODO:: use boost::string (starts_with)
  if(storage_path.c_str()[0] != '/'){
    fprintf(stderr, "Storage path must be absolute\n");
    exit(1);
  }
  if(!isFileReadable(storage_path + "/.tags")){
    fprintf(stderr, "No .tags found in storage directory; use trivialtags to make initial taggings before mounting\n");
    exit(1);
  }
  initDefaults();
  struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
  fuse_opt_add_arg(&args, argv[0]);
  // enable this to debug
  //  fuse_opt_add_arg(&args, "-f");
  fuse_opt_add_arg(&args, argv[2]);
  return fuse_main(args.argc, args.argv, &tri_operations, NULL);
}
