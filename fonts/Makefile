# Makefile for Legendum and Garogier

.PHONY : clean

fontfiles = Garogier.otf Legendum.otf LegendumBold.otf TestInstructions.ttf
legacyfiles = Garogier_legacy.otf Legendum_legacy.otf LegendumBold_legacy.otf

all : $(fontfiles) $(legacyfiles)

Garogier_unhinted.otf: Garogier.ttf Garogier.ot
	../bin/otcomp Garogier.ot
Garogier.otf: Garogier_unhinted.otf Garogier.tti
	../bin/tticomp -o  Garogier.tti
Garogier_legacy.otf: Garogier_unhinted.otf
	../bin/otlegacy Garogier_unhinted.otf Garogier_legacy.otf

Legendum_unhinted.otf: Legendum.ttf Legendum.ot
	../bin/otcomp Legendum.ot
Legendum.otf: Legendum_unhinted.otf Legendum.tti
	../bin/tticomp -o Legendum.tti
Legendum_legacy.otf: Legendum.otf
	../bin/otlegacy Legendum.otf Legendum_legacy.otf

LegendumBold_unhinted.otf: LegendumBold.ttf LegendumBold.ot
	../bin/otcomp LegendumBold.ot
LegendumBold.otf: LegendumBold_unhinted.otf LegendumBold.tti
	../bin/tticomp -o LegendumBold.tti
LegendumBold_legacy.otf: LegendumBold.otf
	../bin/otlegacy LegendumBold.otf LegendumBold_legacy.otf

TestInstructions.ttf: TestInstructions1.ttf TestInstructions.tti
	../bin/tticomp -o TestInstructions.tti

clean:
	rm *.otf *_unhinted.ttf
