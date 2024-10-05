


#ifndef __extendVisitor__
#define __extendVisitor__

#include "arexport.h"
#include "ARTag.h"
#include "ARTypes.h"
#include "tree_browser.h"
#include "visitor.h"

#define PRINT(s) std::cout << s << "\n" << std::flush
#define PRINTW(s, n) std::cout << s << n << std::endl << std::flush

namespace guido 
{

/*!
\addtogroup operations
@{
*/

/*!
\brief A visitor that extends a score by adding rests
*/
class gar_export extendVisitor :
	public visitor<SARVoice>,
	public visitor<Sguidotag>
{		
public:
				extendVisitor() { fBrowser.set(this); }
	virtual ~extendVisitor() { }

	/*! Adds rests to extend a score to the given duration.  No change if duration <= score length.
		Also rounds up to the nearest full measure, given the most recent meter found.
		\param score the score to be extended
		\param duration the score duration to extend to
		\return a new score
	*/
	Sguidoelement extend (const Sguidoelement& score, const rational& duration) {
		durationvisitor dvis;
		rational scoreDur = dvis.duration(score);
		if (duration <= scoreDur) return score;

		fDurToFill = duration - scoreDur;
		fBrowser.browse(*score);
		return score;
	}
	
	virtual void visitStart( SARVoice& elt  ) { fCurrentMeter = "1/1"; }
	virtual void visitStart( Sguidotag& elt ) {
		// printAttributes(elt);
		if (elt->getType() == kTMeter) {
			fCurrentMeter = elt->attributes().at(0)->getValue();
			if (fCurrentMeter == "c" || fCurrentMeter == "c/") {
				fCurrentMeter = "1/1";
			}
		}
	}
	virtual void visitEnd  ( SARVoice& elt  ) {
		rational durAdded = rational(0, 1);
		rational currentMeterDur = rational(fCurrentMeter);
		while (durAdded < fDurToFill) {
			rational restDur = currentMeterDur;
			restDur = restDur.rationalise();
			rationals brokenDownDurs = rational::getBaseRationals(restDur, false, nullptr);
			for (int i = 0; i < brokenDownDurs.size(); i++) {
				SARNote rest = ARFactory().createNote("_");
				*rest = brokenDownDurs.at(i);
				rest->SetDots(0);
				elt->push(rest);
			}
			
			durAdded += currentMeterDur;
		}
	}
	virtual void visitEnd  ( Sguidotag& elt ) { }

	protected:
	rational  	fDurToFill;
	std::string	fCurrentMeter;
	int			fMeasuresToAdd;
	
	tree_browser<guidoelement> fBrowser;
	
	void printAttributes(Sguidotag& elt) {
		PRINT("Printing Element Attributes:------------------");
		auto attrs = elt->attributes();
		for (int i = 0; i < attrs.size(); i++) {
			PRINT("Element Attribute:");
			PRINTW("  - Name:  ", attrs.at(i)->getName());
			PRINTW("  - Value: ", attrs.at(i)->getValue());
		}
		PRINT("----------------------------------------------");
	}
		
};

/*! @} */

}


#endif
