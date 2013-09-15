# Makefile for TrueTypeViewer

# .d files with dependencies should be generated automatically.

.PHONY : clean

# Make the compiler use QT
#QTDIR=/usr/share/qt3
CPPFLAGS=-I${QTDIR}/include -D_STANDALONE_
LDFLAGS=-L$(QTDIR)/lib -lqt

truetypeviewerobjects = cvtviewerdialog.o moc_glyphviewerdialog.o \
		cvtviewerdialogbase.o moc_glyphviewerdialogbase.o \
		featuredialog.o moc_graphicsstateviewerdialog.o \
		featuredialogbase.o moc_messagedialog.o \
		fontcache.o moc_messagedialogbase.o \
		glyphprocessor.o moc_preview.o \
		glyphview.o moc_stackviewerdialog.o \
		glyphviewerdialog.o moc_stackviewerdialogbase.o \
		glyphviewerdialogbase.o moc_storageviewerdialog.o \
		graphicsstateviewerdialog.o moc_truetypeviewerdialog.o \
		main.o moc_truetypeviewerdialogbase.o \
		messagedialog.o preview.o \
		messagedialogbase.o stackviewerdialog.o \
		moc_cvtviewerdialog.o stackviewerdialogbase.o \
		moc_cvtviewerdialogbase.o storageviewerdialog.o \
		moc_featuredialog.o truetypeviewerdialog.o \
		moc_featuredialogbase.o truetypeviewerdialogbase.o \
		moc_glyphview.o ftgrays.o \
		glyphviewerdialogextensionbase.o moc_glyphviewerdialogextensionbase.o \
		glyphviewerdialogextension.o moc_glyphviewerdialogextension.o

utildir = ../Util
otfontdir = ../OTFont
instructionprocessordir = ../InstructionProcessor

include $(utildir)/OBJECTS
include $(otfontdir)/OBJECTS
include $(instructionprocessordir)/OBJECTS

../truetypeviewer: $(truetypeviewerobjects) util otfont instructionprocessor
		$(CXX) $(LDFLAGS) -o ../ttviewer $(truetypeviewerobjects) $(otfontobjects) $(utilobjects) $(instructionprocessorobjects)

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

%.d: %.c
		set -e; $(CXX) -MM $(CPPFLAGS) $< \
				  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
				[ -s $@ ] || rm -f $@

moc_%.cpp: %.h
		moc $< -o $@

%.h: %.ui
		uic $< -o $@

%.cpp: %.ui
		uic -impl $*.h $< -o $@


include $(truetypeviewerobjects:.o=.d)

clean:
		rm $(truetypeviewerobjects) $(truetypeviewerobjects:.o=.d)
# DO NOT DELETE
