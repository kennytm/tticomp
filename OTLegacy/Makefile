# Makefile for OTLegacy

# .d files with dependencies should be generated automatically.

# g++ -o otlegacy -g *.cpp ../OTFont/*.cpp ../Util/*.cpp works as well

.PHONY : clean

CPPFLAGS += -g

otlegacyobjects = CompositeOTText.o Decompositions.o OTLegacy.o \
		PostscriptNameList.o UnicodeRanges.o

utildir = ../Util
otfontdir = ../OTFont

include $(utildir)/OBJECTS
include $(otfontdir)/OBJECTS

../bin/otlegacy: $(otlegacyobjects) util otfont
		$(CXX) -g -o ../bin/otlegacy $(otlegacyobjects) $(otfontobjects) $(utilobjects)

otfont:
		$(MAKE) -C $(otfontdir)
util:
		$(MAKE) -C $(utildir)

%.d: %.cpp
		set -e; $(CXX) -MM $(CPPFLAGS) $< \
				  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
				[ -s $@ ] || rm -f $@

include $(otlegacyobjects:.o=.d)

clean:
		rm $(otlegacyobjects) $(otlegacyobjects:.o=.d)

