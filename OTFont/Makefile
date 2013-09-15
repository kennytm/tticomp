# Makefile for OTFont

# .d files with dependencies should be generated automatically.

.PHONY : clean

CPPFLAGS += -g

otfontdir = .

include OBJECTS

otfonts : $(otfontobjects)

%.d : %.cpp
		set -e; $(CXX) -MM $(CPPFLAGS) $< \
				  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
				[ -s $@ ] || rm -f $@

include $(otfontobjects:.o=.d)

clean:
		rm $(otfontobjects) $(otfontobjects:.o=.d)
