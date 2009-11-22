#!/usr/bin/python
import re
import sys
import os

# tags["name of file"] = array of tags for specialized file
tags = {}

def read_tags(tags_string):
    global tags
    tags_string = tags_string.replace("\n", " ")
    section_re = r"\s*([^{]+){\s*([^}]*)}"
    matches = re.findall(section_re, tags_string)
    for i in matches:
        name, tgs = i
        name = name.strip()
        tags[name] = []
        for tag in tgs.split(","):
            tag = tag.strip()
            tags[name].append(tag)
            pass
        pass
    pass

def load_tags(path):
    f = open(path, 'r')
    s = f.read()
    read_tags(s)
    f.close()

def serialize_tags():
    s = ''
    for file in tags.keys():
        s += file
        s += ' { '
        s += ", ".join(tags[file])
        s += ' }\n'
        pass
    return s

def store_tags(path):
    f = open(path, 'w')
    f.write(serialize_tags())
    f.close()

argc = len(sys.argv)

if argc is 1 or argc > 3:
    print """Usage:
    trivialtags <name of file> [tag1, tag2, ...]

When no tag is passed, current tags for file will be printed.
If tag name starts with '-' this tag will be removed (if present) from file,
   otherwise it'll be added to file
    """
    sys.exit(1)
else:
    load_tags("./.tags")
    pass

if argc is 2:
    if sys.argv[1] not in tags.keys():
        print "File " + sys.argv[1] + " is not tagged"
    else:
        print ", ".join(tags[sys.argv[1]])
elif argc is 3:
    name = sys.argv[1]
    if name not in tags.keys():
        tags[name] = []
    for tag in sys.argv[2].split(","):
        tag = tag.strip();
        if tag[0] is '-':
            tag = tag[1:]
            tags[name].remove(tag)
        else:
            tags[name].append(tag)
        pass
    store_tags("./.tags")
    pass
else:
    pass
