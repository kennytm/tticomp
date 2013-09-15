# Makefile for InstructionProcessor

# .d files with dependencies should be generated automatically.

.PHONY : clean

CPPFLAGS += -g

instructionprocessordir = .

include OBJECTS

instructionprocessor : $(instructionprocessorobjects)

%.d : %.cpp
		set -e; $(CXX) -MM $(CPPFLAGS) $< \
				  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
				[ -s $@ ] || rm -f $@

include $(instructionprocessorobjects:.o=.d)

clean:
		rm $(instructionprocessorobjects) $(instructionprocessorobjects:.o=.d)
