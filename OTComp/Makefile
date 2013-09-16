# Makefile for OTComp

# .d files with dependencies should be generated automatically.

.PHONY : clean

otcompobjects = GlyphList.o Identifier.o Lookup.o OTComp.o \
	OTCompPreprocessor.o ReferenceTable.o Script.o

otfontdir = ../OTFont


utildir = ../Util
otfontdir = ../OTFont

include $(utildir)/OBJECTS
include $(otfontdir)/OBJECTS

../bin/otcomp: $(otcompobjects) util otfont
		$(CXX) -g -o ../bin/otcomp $(otcompobjects) $(otfontobjects) $(utilobjects)

otfont:
		$(MAKE) -C $(otfontdir)
util:
		$(MAKE) -C $(utildir)

%.d: %.cpp
		set -e; $(CXX) -MM $(CPPFLAGS) $< \
				  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
				[ -s $@ ] || rm -f $@

include $(otcompobjects:.o=.d)

clean:
		rm $(otcompobjects) $(otcompobjects:.o=.d)

