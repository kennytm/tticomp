/*
	(c) Copyright 2002, 2003 Rogier van Dalen
	(R.C.van.Dalen@umail.leidenuniv.nl for any comments, questions or bugs)

	This file is part of my OpenType/TrueType Font Tools.

	The OpenType/TrueType Font Tools is free software; you can redistribute
	it and/or modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	The OpenType/TrueType Font Tools is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
	Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the OpenType/TrueType Font Tools; if not, write to the Free
	Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	In addition, as a special exception, Rogier van Dalen gives permission
	to link the code of this program with Qt non-commercial edition (or with
	modified versions of Qt non-commercial edition that use the	same license
	as Qt non-commercial edition), and distribute linked combinations
	including the two.  You must obey the GNU General Public License in all
	respects for all of the code used other than Qt non-commercial edition.
	If you modify this file, you may extend this exception to your version of
	the file, but you are not obligated to do so.  If you do not wish to do
	so, delete this exception statement from your version.
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include <qcombobox.h>
#include <qlistview.h>

#include "../OTFont/OTTags.h"

#include "featuredialog.h"
#include "glyphviewerdialog.h"
#include "truetypeviewerdialog.h"

/*** CheckListItemWithBuddy ***/

class CheckListItemWithBuddy : public QCheckListItem {
	CheckListItemWithBuddy * buddy;
public:
	CheckListItemWithBuddy (QListViewItem *parent, CheckListItemWithBuddy * _buddy, const QString &text)
		: buddy (_buddy), QCheckListItem (parent, text, CheckBox) {}

	void setBuddy (CheckListItemWithBuddy *_buddy) {
		buddy = _buddy;
	}

	virtual void setOn (bool newSetting) {
		bool oldSetting = isOn();
		QCheckListItem::setOn (newSetting);
		if (newSetting != oldSetting)
			buddy->setOn (newSetting);
	}
};



/*** FeatureDialog ***/

FeatureDialog::FeatureDialog(QWidget* parent)
: FeatureDialogBase( parent, NULL, false, 0) {
	font = NULL;
	changingLanguage = false;

	// Disable sorting (otherwise GPOS will end up above GSUB)
	listFeatures->setSorting (-1);

	// Produce all tag descriptions
	tagDescriptions [OT_MAKE_TAG ('a','a','l','t')] = "Access All Alternates";
	tagDescriptions [OT_MAKE_TAG ('a','b','v','f')] = "Above-base Forms";
	tagDescriptions [OT_MAKE_TAG ('a','b','v','m')] = "Above-base Mark Positioning";
	tagDescriptions [OT_MAKE_TAG ('a','b','v','s')] = "Above-base Substitutions";
	tagDescriptions [OT_MAKE_TAG ('a','f','r','c')] = "Alternative Fractions";
	tagDescriptions [OT_MAKE_TAG ('a','k','h','n')] = "Akhands";
	tagDescriptions [OT_MAKE_TAG ('b','l','w','f')] = "Below-base Forms";
	tagDescriptions [OT_MAKE_TAG ('b','l','w','m')] = "Below-base Mark Positioning";
	tagDescriptions [OT_MAKE_TAG ('b','l','w','s')] = "Below-base Substitutions";
	tagDescriptions [OT_MAKE_TAG ('c','a','l','t')] = "Contextual Alternates";
	tagDescriptions [OT_MAKE_TAG ('c','a','s','e')] = "Case-Sensitive Forms";
	tagDescriptions [OT_MAKE_TAG ('c','c','m','p')] = "Glyph Composition / Decomposition";
	tagDescriptions [OT_MAKE_TAG ('c','l','i','g')] = "Contextual Ligatures";
	tagDescriptions [OT_MAKE_TAG ('c','p','s','p')] = "Capital Spacing";
	tagDescriptions [OT_MAKE_TAG ('c','s','w','h')] = "Contextual Swash";
	tagDescriptions [OT_MAKE_TAG ('c','u','r','s')] = "Cursive Positioning";
	tagDescriptions [OT_MAKE_TAG ('c','2','s','c')] = "Small Capitals From Capitals";
	tagDescriptions [OT_MAKE_TAG ('c','2','p','c')] = "Petite Capitals From Capitals";
	tagDescriptions [OT_MAKE_TAG ('d','i','s','t')] = "Distances ";
	tagDescriptions [OT_MAKE_TAG ('d','l','i','g')] = "Discretionary Ligatures";
	tagDescriptions [OT_MAKE_TAG ('d','n','o','m')] = "Denominators";
	tagDescriptions [OT_MAKE_TAG ('e','x','p','t')] = "Expert Forms";
	tagDescriptions [OT_MAKE_TAG ('f','a','l','t')] = "Final Glyph on Line Alternates";
	tagDescriptions [OT_MAKE_TAG ('f','i','n','2')] = "Terminal Forms #2";
	tagDescriptions [OT_MAKE_TAG ('f','i','n','3')] = "Terminal Forms #3";
	tagDescriptions [OT_MAKE_TAG ('f','i','n','a')] = "Terminal Forms";
	tagDescriptions [OT_MAKE_TAG ('f','r','a','c')] = "Fractions";
	tagDescriptions [OT_MAKE_TAG ('f','w','i','d')] = "Full Widths";
	tagDescriptions [OT_MAKE_TAG ('h','a','l','f')] = "Half Forms";
	tagDescriptions [OT_MAKE_TAG ('h','a','l','n')] = "Halant Forms";
	tagDescriptions [OT_MAKE_TAG ('h','a','l','t')] = "Alternate Half Widths";
	tagDescriptions [OT_MAKE_TAG ('h','i','s','t')] = "Historical Forms";
	tagDescriptions [OT_MAKE_TAG ('h','k','n','a')] = "Horizontal Kana Alternates";
	tagDescriptions [OT_MAKE_TAG ('h','l','i','g')] = "Historical Ligatures";
	tagDescriptions [OT_MAKE_TAG ('h','n','g','l')] = "Hangul";
	tagDescriptions [OT_MAKE_TAG ('h','w','i','d')] = "Half Widths";
	tagDescriptions [OT_MAKE_TAG ('i','n','i','t')] = "Initial Forms";
	tagDescriptions [OT_MAKE_TAG ('i','s','o','l')] = "Isolated Forms";
	tagDescriptions [OT_MAKE_TAG ('i','t','a','l')] = "Italics";
	tagDescriptions [OT_MAKE_TAG ('j','a','l','t')] = "Justification Alternates";
	tagDescriptions [OT_MAKE_TAG ('j','p','7','8')] = "JIS78 Forms";
	tagDescriptions [OT_MAKE_TAG ('j','p','8','3')] = "JIS83 Forms";
	tagDescriptions [OT_MAKE_TAG ('j','p','9','0')] = "JIS90 Forms";
	tagDescriptions [OT_MAKE_TAG ('k','e','r','n')] = "Kerning";
	tagDescriptions [OT_MAKE_TAG ('l','f','b','d')] = "Left Bounds";
	tagDescriptions [OT_MAKE_TAG ('l','i','g','a')] = "Standard Ligatures";
	tagDescriptions [OT_MAKE_TAG ('l','j','m','o')] = "Leading Jamo Forms";
	tagDescriptions [OT_MAKE_TAG ('l','n','u','m')] = "Lining Figures";
	tagDescriptions [OT_MAKE_TAG ('l','o','c','l')] = "Localized Forms";
	tagDescriptions [OT_MAKE_TAG ('m','a','r','k')] = "Mark Positioning";
	tagDescriptions [OT_MAKE_TAG ('m','e','d','2')] = "Medial Forms #2";
	tagDescriptions [OT_MAKE_TAG ('m','e','d','i')] = "Medial Forms";
	tagDescriptions [OT_MAKE_TAG ('m','g','r','k')] = "Mathematical Greek";
	tagDescriptions [OT_MAKE_TAG ('m','k','m','k')] = "Mark to Mark Positioning";
	tagDescriptions [OT_MAKE_TAG ('m','s','e','t')] = "Mark Positioning via Substitution";
	tagDescriptions [OT_MAKE_TAG ('n','a','l','t')] = "Alternate Annotation Forms";
	tagDescriptions [OT_MAKE_TAG ('n','l','c','k')] = "NLC Kanji Forms";
	tagDescriptions [OT_MAKE_TAG ('n','u','k','t')] = "Nukta Forms";
	tagDescriptions [OT_MAKE_TAG ('n','u','m','r')] = "Numerators";
	tagDescriptions [OT_MAKE_TAG ('o','n','u','m')] = "Oldstyle Figures";
	tagDescriptions [OT_MAKE_TAG ('o','p','b','d')] = "Optical Bounds";
	tagDescriptions [OT_MAKE_TAG ('o','r','d','n')] = "Ordinals";
	tagDescriptions [OT_MAKE_TAG ('o','r','n','m')] = "Ornaments";
	tagDescriptions [OT_MAKE_TAG ('p','a','l','t')] = "Proportional Alternate Widths";
	tagDescriptions [OT_MAKE_TAG ('p','c','a','p')] = "Petite Capitals";
	tagDescriptions [OT_MAKE_TAG ('p','n','u','m')] = "Proportional Figures";
	tagDescriptions [OT_MAKE_TAG ('p','r','e','f')] = "Pre-Base Forms";
	tagDescriptions [OT_MAKE_TAG ('p','r','e','s')] = "Pre-base Substitutions";
	tagDescriptions [OT_MAKE_TAG ('p','s','t','f')] = "Post-base Forms";
	tagDescriptions [OT_MAKE_TAG ('p','s','t','s')] = "Post-base Substitutions";
	tagDescriptions [OT_MAKE_TAG ('p','w','i','d')] = "Proportional Widths";
	tagDescriptions [OT_MAKE_TAG ('q','w','i','d')] = "Quarter Widths";
	tagDescriptions [OT_MAKE_TAG ('r','a','n','d')] = "Randomize";
	tagDescriptions [OT_MAKE_TAG ('r','l','i','g')] = "Required Ligatures";
	tagDescriptions [OT_MAKE_TAG ('r','p','h','f')] = "Reph Forms";
	tagDescriptions [OT_MAKE_TAG ('r','t','b','d')] = "Right Bounds";
	tagDescriptions [OT_MAKE_TAG ('r','t','l','a')] = "Right-to-left alternates";
	tagDescriptions [OT_MAKE_TAG ('r','u','b','y')] = "Ruby Notation Forms";
	tagDescriptions [OT_MAKE_TAG ('s','a','l','t')] = "Stylistic Alternates";
	tagDescriptions [OT_MAKE_TAG ('s','i','n','f')] = "Scientific Inferiors";
	tagDescriptions [OT_MAKE_TAG ('s','i','z','e')] = "Optical size";
	tagDescriptions [OT_MAKE_TAG ('s','m','c','p')] = "Small Capitals";
	tagDescriptions [OT_MAKE_TAG ('s','m','p','l')] = "Simplified Forms";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','1')] = "Stylistic Set 1";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','2')] = "Stylistic Set 2";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','3')] = "Stylistic Set 3";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','4')] = "Stylistic Set 4";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','5')] = "Stylistic Set 5";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','6')] = "Stylistic Set 6";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','7')] = "Stylistic Set 7";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','8')] = "Stylistic Set 8";
	tagDescriptions [OT_MAKE_TAG ('s','s','0','9')] = "Stylistic Set 9";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','0')] = "Stylistic Set 10";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','1')] = "Stylistic Set 11";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','2')] = "Stylistic Set 12";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','3')] = "Stylistic Set 13";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','4')] = "Stylistic Set 14";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','5')] = "Stylistic Set 15";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','6')] = "Stylistic Set 16";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','7')] = "Stylistic Set 17";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','8')] = "Stylistic Set 18";
	tagDescriptions [OT_MAKE_TAG ('s','s','1','9')] = "Stylistic Set 19";
	tagDescriptions [OT_MAKE_TAG ('s','s','2','0')] = "Stylistic Set 20";
	tagDescriptions [OT_MAKE_TAG ('s','u','b','s')] = "Subscript";
	tagDescriptions [OT_MAKE_TAG ('s','u','p','s')] = "Superscript";
	tagDescriptions [OT_MAKE_TAG ('s','w','s','h')] = "Swash";
	tagDescriptions [OT_MAKE_TAG ('t','i','t','l')] = "Titling";
	tagDescriptions [OT_MAKE_TAG ('t','j','m','o')] = "Trailing Jamo Forms";
	tagDescriptions [OT_MAKE_TAG ('t','n','a','m')] = "Traditional Name Forms";
	tagDescriptions [OT_MAKE_TAG ('t','n','u','m')] = "Tabular Figures";
	tagDescriptions [OT_MAKE_TAG ('t','r','a','d')] = "Traditional Forms";
	tagDescriptions [OT_MAKE_TAG ('t','w','i','d')] = "Third Widths";
	tagDescriptions [OT_MAKE_TAG ('u','n','i','c')] = "Unicase";
	tagDescriptions [OT_MAKE_TAG ('v','a','l','t')] = "Alternate Vertical Metrics";
	tagDescriptions [OT_MAKE_TAG ('v','a','t','u')] = "Vattu Variants";
	tagDescriptions [OT_MAKE_TAG ('v','e','r','t')] = "Vertical Writing";
	tagDescriptions [OT_MAKE_TAG ('v','h','a','l')] = "Alternate Vertical Half Metrics";
	tagDescriptions [OT_MAKE_TAG ('v','j','m','o')] = "Vowel Jamo Forms";
	tagDescriptions [OT_MAKE_TAG ('v','k','n','a')] = "Vertical Kana Alternates";
	tagDescriptions [OT_MAKE_TAG ('v','k','r','n')] = "Vertical Kerning";
	tagDescriptions [OT_MAKE_TAG ('v','p','a','l')] = "Proportional Alternate Vertical Metrics";
	tagDescriptions [OT_MAKE_TAG ('v','r','t','2')] = "Vertical Alternates and Rotation";
	tagDescriptions [OT_MAKE_TAG ('z','e','r','o')] = "Slashed Zero";
}

FeatureDialog::~FeatureDialog() {
	((TrueTypeViewerDialog *) parent()) -> featureDialogClosed();
}

QString tagToQString (Tag tag) {
	char *c = (char*) &tag;
	int length;
	if (c[3] != ' ')
		length = 4;
	else {
		if (c[2] != ' ')
			length = 3;
		else {
			if (c[1] != ' ')
				length = 2;
			else
				length = 1;
		}
	}

	QString s;
	do {
		s += *c;
		c++;
		length --;
	} while (length);

	return s;
}

Tag QStringToTag (QString s) {
	Tag tag = 0x20202020;
	char *c = (char *) &tag;
	assert (s.length() <= 4);
	for (int i=0; (unsigned)i < s.length(); i ++) {
		*c = s[i].latin1();
		c ++;
	}
	return tag;
}

QString FeatureDialog::getTagDescription (Tag tag) const {
	std::map <Tag, QString>::const_iterator i = tagDescriptions.find (tag);
	if (i != tagDescriptions.end())
		return i->second;
	else
		return "<Unknown tag>";
}

void FeatureDialog::setFont (smart_ptr <OpenTypeFont> aFont) {
	Tag scriptID, languageID;
	if (comboScript->count()==0) {
		scriptID = 0;
		languageID = 0;
	} else {
		scriptID = QStringToTag (comboScript->currentText().latin1());
		languageID = QStringToTag (comboLanguage->currentText().latin1());
	}
	font = aFont;
	TagVector scripts = font->getScripts();

	comboScript->clear();
	int newScriptIndex = 0;
	
	for (TagVector::iterator script = scripts.begin(); script != scripts.end(); script ++) {
		comboScript->insertItem (tagToQString (*script));
		if (*script == scriptID)
			newScriptIndex = script - scripts.begin();
	}
	comboScript->setCurrentItem(newScriptIndex);

	if (scripts.empty())
		scriptChanged ("");
	else
		scriptChanged (comboScript->currentText());
}

void FeatureDialog::scriptChanged (const QString &newScript) {
	comboLanguage->clear();
	if (newScript == ""){
		languageChanged("");
	} else {
		assert(font);

		TagVector languages = font->getLanguages (QStringToTag (newScript));
		for (TagVector::iterator l = languages.begin(); l < languages.end(); l ++)
			comboLanguage->insertItem (tagToQString (*l));
		if (languages.empty())
			languageChanged ("");
		else
			languageChanged (comboLanguage->currentText());
	}
}

void FeatureDialog::languageChanged(const QString &newLanguage) {
	changingLanguage = true;
	listFeatures->clear();
	if (newLanguage!="") {
		Tag scriptId = QStringToTag (comboScript->currentText());
		Tag languageId = QStringToTag (newLanguage);
		/*TagVector features = font->getFeatures (scriptId, languageId);

		for (TagVector::iterator f = features.begin(); f != features.end(); f ++) {
			QCheckListItem *item =
				new QCheckListItem (listFeatures, tagToQString (*f), QCheckListItem::CheckBox);
			item->setText (1, getTagDescription (*f));
			for (TagVector::iterator s = selectedFeatures.begin(); s != selectedFeatures.end(); s ++) {
				if (*f == *s)
					item->setOn (true);
			}
		}*/

		TagVector substitutionTags = font->getSubstitutionFeatures (scriptId, languageId);
		TagVector positioningTags = font->getPositioningFeatures (scriptId, languageId);

		// Because Qt inserts items before those that were previously added,
		// they need to be added in reverse order.

		QListViewItem *positioningItem = new QListViewItem (listFeatures, "GPOS", "Glyph positioning");
		positioningItem->setOpen (true);
		QListViewItem *substitutionItem = new QListViewItem (listFeatures, "GSUB", "Glyph substitution");
		substitutionItem->setOpen (true);

		TagVector::reverse_iterator s, p, sel;
		s = substitutionTags.rbegin();
		p = positioningTags.rbegin();
		sel = selectedFeatures.rbegin();
		while (s != substitutionTags.rend() || p != positioningTags.rend()) {
			if (p == positioningTags.rend() || (s != substitutionTags.rend() && compareTags (*p, *s)))
			{	// *s is the first one to consider
				QCheckListItem *item =
					new QCheckListItem (substitutionItem, tagToQString (*s), QCheckListItem::CheckBox);
				item->setText (1, getTagDescription (*s));
				while (sel != selectedFeatures.rend() && compareTags (*s, *sel))
					++ sel;
				if (sel != selectedFeatures.rend() && *sel == *s)
					item->setOn (true);
				++ s;
			} else
			{	// *p is considered first, but *s may be the same
				while (sel != selectedFeatures.rend() && compareTags (*p, *sel))
					++ sel;
				if (s == substitutionTags.rend() || *s != *p)
				{	// *p is the first one to consider
					QCheckListItem *item =
						new QCheckListItem (positioningItem, tagToQString (*p), QCheckListItem::CheckBox);
					item->setText (1, getTagDescription (*p));
					if (sel != selectedFeatures.rend() && *sel == *p)
						item->setOn (true);
					++ p;
				} else
				{	// *p and *s are equal, so they should be added in 2 categories
					QString tagString = tagToQString (*s);
					QString description = getTagDescription (*p);
					CheckListItemWithBuddy *sItem = 
						new CheckListItemWithBuddy (substitutionItem, NULL, tagString);
					CheckListItemWithBuddy *pItem =
						new CheckListItemWithBuddy (positioningItem, sItem, tagString);
					sItem->setBuddy (pItem);
					sItem->setText (1, description);
					pItem->setText (1, description);

					if (sel != selectedFeatures.rend() && *sel == *s)
						sItem->setOn (true);

					++ s;
					++ p;
				}
			}
		}
	}
	changingLanguage = false;
	featuresChanged();
}

void FeatureDialog::featuresChanged() {
	if (!changingLanguage) {
		ULong scriptID = 0;
		ULong languageID = 0;
		selectedFeatures.clear();

		/*QListViewItem * item = listFeatures->firstChild();
		for (; item != NULL; item = item->nextSibling()) {
			//if (listFeatures->isSelected(i))
			if (((QCheckListItem *) item)->isOn())
				selectedFeatures.push_back (QStringToTag (item->text (0)));
				//selectedFeatures.push_back (features[i]);
		}
		*/

		QListViewItem *substitutionItem = listFeatures->firstChild();
		if (substitutionItem) {
			QCheckListItem *item = (QCheckListItem *) substitutionItem->firstChild();
			for (; item != NULL; item = (QCheckListItem *) item->nextSibling())
				if (item->isOn())
					selectedFeatures.push_back (QStringToTag (item->text (0)));

			TagVector::iterator s = selectedFeatures.begin();
			item = (QCheckListItem *) substitutionItem->nextSibling()->firstChild();
			for (; item != NULL; item = (QCheckListItem *) item->nextSibling()) {
				Tag tag = QStringToTag (item->text (0));
				while (s != selectedFeatures.end() && compareTags (*s, tag))
					++ s;
				if (item->isOn()) {
					if (s == selectedFeatures.end() || *s != tag)
						s = selectedFeatures.insert (s, tag);
				}
			}
		}

		scriptID = QStringToTag (comboScript->currentText());
		languageID = QStringToTag (comboLanguage->currentText());
		
		((TrueTypeViewerDialog *) parent())->setFeatures(scriptID, languageID, selectedFeatures);
	}
}
