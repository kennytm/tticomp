# Makefile for TTIComp

# .d files with dependencies should be generated automatically.

# g++ -o tticomp -g *.cpp ../OTFont/*.cpp ../Util/*.cpp works as well

.PHONY : clean

CPPFLAGS += -g

tticompobjects = Expression.o TTIComp.o \
		FunctionScope.o Scope.o TTICompPreprocessor.o \
		InstructionSequence.o Statement.o

utildir = ../Util
otfontdir = ../OTFont
instructionprocessordir = ../InstructionProcessor

include $(utildir)/OBJECTS
include $(otfontdir)/OBJECTS
include $(instructionprocessordir)/OBJECTS

../tticomp: $(tticompobjects) util otfont instructionprocessor
		$(CXX) -g -o ../tticomp $(tticompobjects) $(otfontobjects) $(utilobjects) $(instructionprocessorobjects)

otfont:
		$(MAKE) -C $(otfontdir)
util:
		$(MAKE) -C $(utildir)
instructionprocessor:
		$(MAKE) -C $(instructionprocessordir)

%.d: %.cpp
		set -e; $(CXX) -MM $(CPPFLAGS) $< \
				  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
				[ -s $@ ] || rm -f $@

include $(tticompobjects:.o=.d)

clean:
		rm $(tticompobjects) $(tticompobjects:.o=.d)

