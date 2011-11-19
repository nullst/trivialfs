DDIR=/usr/bin

compile:
	g++ $$(pkg-config --cflags --libs fuse) fuse.cc dispatch.cc parser.cc util.cc -o trivialfs
install:
	cp ./trivialfs ./trivialtags $(DESTDIR)$(DDIR)
uninstall:
	rm $(DDIR)/trivialfs $(DDIR)/trivialtags
clean:
	rm trivialfs
dist:
	mkdir -p /tmp/trivialfs
	cp Makefile *.cc *.h trivialtags /tmp/trivialfs
	PWD=`pwd`
	(cd /tmp && tar czf ${PWD}/trivialfs.tar.gz trivialfs)
	rm -rf /tmp/trivialfs
