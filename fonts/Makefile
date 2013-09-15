# Makefile for Legendum and Garogier

.PHONY : clean

fontfiles = Garogier.otf Legendum.otf LegendumBold.otf TestInstructions.ttf
legacyfiles = Garogier_legacy.otf Legendum_legacy.otf LegendumBold_legacy.otf

all : $(fontfiles) $(legacyfiles)

Garogier_unhinted.otf: Garogier.ttf Garogier.ot
	../otcomp Garogier.ot
Garogier.otf: Garogier_unhinted.otf Garogier.tti
	../tticomp -o  Garogier.tti
Garogier_legacy.otf: Garogier_unhinted.otf
	../otlegacy Garogier_unhinted.otf Garogier_legacy.otf

Legendum_unhinted.otf: Legendum.ttf Legendum.ot
	../otcomp Legendum.ot
Legendum.otf: Legendum_unhinted.otf Legendum.tti
	../tticomp -o Legendum.tti
Legendum_legacy.otf: Legendum.otf
	../otlegacy Legendum.otf Legendum_legacy.otf

LegendumBold_unhinted.otf: LegendumBold.ttf LegendumBold.ot
	../otcomp LegendumBold.ot
LegendumBold.otf: LegendumBold_unhinted.otf LegendumBold.tti
	../tticomp -o LegendumBold.tti
LegendumBold_legacy.otf: LegendumBold.otf
	../otlegacy LegendumBold.otf LegendumBold_legacy.otf

TestInstructions.ttf: TestInstructions1.ttf TestInstructions.tti
	../tticomp -o TestInstructions.tti

clean:
	rm *.otf *_unhinted.ttf