#define FUSE_USE_VERSION 28
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <fuse_opt.h>

#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>

std::string storage_path;
static dispatcher disp;

static int tri_getattr(const char *path, struct stat *st){
  memset(st, 0, sizeof(struct stat));

  std::vector<std::string> tags = splitPath(std::string(path));
  const std::string& name = *tags.rbegin();
  bool is_directory = false;
  if(tags.empty()) is_directory = true;
  /* there is no files and tags with the same names
     so when last element in path is tag, it's directory
     Well, in fact I have to check whether all subdirs in path are tags
  */
  if(disp.isTagDefined(name)) is_directory = true;
  
  if(is_directory){
    st->st_mode = S_IFDIR | 0700;
    st->st_nlink = 2;
  }else if(disp.isFileDefined(name)){
    st->st_mode = S_IFLNK | 0400;
    st->st_nlink = 1;
    // +1 for '/' between storage path and filename
    st->st_size = storage_path.size() + name.size() + 1;
  }else{
    /* there is no such file at all */
    return -ENOENT;
  }
  return 0;
}

static int tri_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi){
  std::vector<std::string> tags_vec = splitPath(std::string(path));
  std::set<std::string> tags;
  std::copy(tags_vec.begin(), tags_vec.end(),
	    std::inserter(tags, tags.begin()));
  std::pair< std::set<std::string>, std::set<std::string> > dirs = directoryStructure(disp, tags);
  
  /* if there is no such tag for path */
  if(dirs.second.empty()){
    return -ENOINT;
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
  std::copy(++tags_vec.rbegin(), tags_vec.rend(),
	    std::inserter(tags, tags.begin()));
  bool exist = doesFileExist(disp, tags, tags_vec[tags_vec.size() - 1]);
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

static struct fuse_operations tri_operations = {
  .getattr = tri_getattr,
  .readdir = tri_readdir,
  .open = tri_open,
  .read = tri_read
};

int main(int argc, char **argv){
  if(argc < 3){
    printf("Usage:\n"
	   "trivialfs /path/to/storage /mount/point\n");
    exit(1);
  }
  storage_path = std::string(argv[1]);
  loadTags(disp, storage_path + "/.tags");
  struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
  fuse_opt_add_arg(&args, argv[2]);
  return fuse_main(args.argc, args.argv, &tri_operations, NULL);
}
