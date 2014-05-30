trivialfs
==

Introduction
--
This is a fuse-based virtual file system which allows use to attach tags to files and then navigate the tag cloud just like usual directories.

Two principal limitations: you cannot tag directories, and all the files you wish to access through one mount point must reside in one "source" directory somewhere else. Or at least symlinks to them, which is still cumbersome.

How it looks like
--
Assume you mounted trivialfs to `~/tags`.

    ls ~/tags/algebra/books

will show you the list of all files which are tagged `algebra` and `books`. Similarly, `~/tags/algebra/books/finished` gives you all files with three tags, `algebra`, `books`, `finished`. The directory `~/tags/books/finished/algebra` is absolutely the same. **Do not use `find` on trivialfs directories!** It will do an exponential amount of work, treating all combinations of tags (which actually correspond to at least one file) as separate directories.

Subdirectories in trivialfs mounts are, as you may guess, more tags which could be added to the list of tags (like `algebra` and `books` in the example) with at least one file having all the tags.

It is impossible in current version to filter out files with tag `a` and not tag `b` beyond some `awk`/`sed` manipulations with lists of files: only intersections are supported.

How to tag files
--
First of all, you need a separate directory with all the files you are going to access through one mountpoint. Let's assume it's `~/info-storage`. The information about tags is contained will be contained in the file `.tags` there. It has a human-readable format, so you may just modify it as you with. It's usually more convenient to use the script `trivialtags` bundled together with trivialfs.

To see which tags does the file "Vakil AG.pdf" have currently, run
    trivialtags "Vakil AG.pdf"
To remove all tags from this file, execute
    trivialtags "Vakil AG.pdf" -
To remove just one tag, say, "ag", execute
    trivialtags "Vakil AG.pdf" -ag
There is no way to remove several but not all tags from a file beyond repeating this command with different tags. It seems reasonable to assume that if you have 10+ tags on every file, you already have some way to automate tagging.
To add a tag "finished" to this file, run
    trivialtags "Vakil AG.pdf" finished
To add several tags give a list separated by commas (as one of arguments):
    trivialtags "Vakil AG.pdf" "book to be used again and again, nice, fundamental"
(spaces around commas will be ignored, but spaces inside the tag are nice)

If you run `trivialtags` without arguments, it will give you similar help text.

How to mount
--

If the directory with all the files and `.tags` is `~/source` and you wish to access them through `~/tags`, execute
    trivialfs ~/source ~/tags &
(if you need to mount only if it's not mounted yet, use
    (mount | grep -q $HOME/tags || trivialfs /mnt/files/info-storage $HOME/tags) &
but you probably know this better than me)

Remembed to use `&`: trivialfs doesn't switch to a daemon state.

How to compile and install
--

Run `make` in the directory with source files. If you are using Arch Linux, you may also want to run `make dist` after that to create trivialfs.tar.gz with all the binaries, modify md5 in `PKGBUILD` and then run `makepkg` to obtain `pacman`-installable package.