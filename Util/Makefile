# Makefile for String.cpp

# .d files with dependencies should be generated automatically.

.PHONY : clean

CPPFLAGS += -g

utildir = .

include OBJECTS

utils : $(utilobjects)

%.d : %.cpp
		set -e; $(CC) -MM $(CPPFLAGS) $< \
				  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
				[ -s $@ ] || rm -f $@

include $(utilobjects:.o=.d)

clean:
		rm $(utilobjects) $(utilobjects:.o=.d)
