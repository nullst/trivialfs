DESTDIR=/usr/bin

compile:
	g++ $$(pkg-config --cflags --libs fuse) fuse.cc dispatch.cc parser.cc util.cc -o trivialfs
install:
	cp ./trivialfs ./trivialtags ${DESTDIR}
uninstall:
	rm ${DESTDIR}/trivialfs ${DESTDIR}/trivialtags
clean:
	rm trivialfs
