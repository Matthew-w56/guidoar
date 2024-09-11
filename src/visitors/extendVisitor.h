


#ifndef __extendVisitor__
#define __extendVisitor__

#include "arexport.h"
#include "ARTag.h"
#include "ARTypes.h"
#include "tree_browser.h"
#include "visitor.h"

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
	
	virtual void visitStart( SARVoice& elt  ) { fCurrentMeter = 1; }
	virtual void visitStart( Sguidotag& elt ) { if (elt->getName() == "meter") fCurrentMeter = elt->attributes().at(0)->getValue(); }
	virtual void visitEnd  ( SARVoice& elt  ) {
		printf("Starting a visitEnd for voice!\n");
		std::cout.flush();
		rational durAdded = rational(0, 1);
		rational currentMeterDur = rational(fCurrentMeter);
		while (durAdded < fDurToFill) {
			rational restDur = currentMeterDur;
			restDur = restDur.rationalise();
			std::vector<int> dotsOut;
			rationals brokenDownDurs = rational::getBaseRationals(restDur, false, nullptr);
			
			for (int i = 0; i < brokenDownDurs.size(); i++) {
				SARNote rest = ARFactory().createNote("_");
				*rest = brokenDownDurs.at(i);
				rest->SetDots(0);
				elt->push(rest);
			}
			
			durAdded += currentMeterDur;
		}
		printf("Done with visitEnd for voice\n");
		std::cout.flush();
	}
	virtual void visitEnd  ( Sguidotag& elt ) { }

	protected:
	rational  	fDurToFill;
	std::string	fCurrentMeter;
	int			fMeasuresToAdd;
	
	tree_browser<guidoelement> fBrowser;
		
};

/*! @} */

}


#endif
