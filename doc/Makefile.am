all: Doxyfile
	$(DOXYGEN) Doxyfile

dist-hook: all
	-cp -r html $(distdir)