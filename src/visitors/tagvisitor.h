
#ifndef __tagVisitor__
#define __tagVisitor__

#include "arexport.h"
#include "ARTypes.h"
#include "tree_browser.h"
#include "visitor.h"
#include "ARTag.h"
#include "guidotags.h"

namespace guido {

struct VoiceInitInfo {
	Sguidotag clef;
	Sguidotag instr;
	Sguidotag keySignature;
	Sguidotag meter;
};

/*!
\addtogroup visitors
@{
*/

class gar_export tagvisitor :
	public visitor<Sguidotag>,
	public visitor<SARNote>
{
public:
			 tagvisitor() { fBrowser.set(this); }
	virtual ~tagvisitor() {}
	
	VoiceInitInfo getVoiceInfo(const Sguidoelement& voice) {
		// Reset to defaults that we will return if we can't find them in voice
		fClef = nullptr;
		fInstr = nullptr;
		fKeySig = nullptr;
		fMeter = nullptr;
		
		// Look for info
		fBrowser.browse(*voice);
		
		// Pack info that we found into the object and return
		VoiceInitInfo v;
		v.instr = fInstr;
		v.clef = fClef;
		v.meter = fMeter;
		v.keySignature = fKeySig;
		return v;
	}
	
	// Check the tag to see if it matches what we care about
	virtual void visitStart(Sguidotag& tag) {
		if (tag->getType() == kTClef) {
			fClef = tag;
		} else if (tag->getType() == kTInstr || tag->getType() == kTInstrument) {
			fInstr = tag;
		} else if (tag->getType() == kTMeter) {
			fMeter = tag;
		} else if (tag->getType() == kTKey) {
			fKeySig = tag;
		}
	}
	
	// If we hit a note, we know that we are done seeing any useful
	// initial values for things like clef and instrument.  This
	// class is not meant to grab the first defined tags, it only
	// is meant to grab the ones defined at the start of the voice.
	// So, we can stop once we hit a note.
	virtual void visitStart(SARNote& elt) {
		fBrowser.stop();
	}
	
	
protected:
	tree_browser<guidoelement> fBrowser;
	Sguidotag fClef;
	Sguidotag fInstr;
	Sguidotag fKeySig;
	Sguidotag fMeter;

private:
	int getIntValueOfAttr(Sguidotag tag, char* attrName) {
		auto attrs = tag->attributes();
		for (int i = 0; i < attrs.size(); i++) {
			if (attrs.at(i)->getName().data() == attrName) {
				return tag->getAttributeIntValue(i, -1);
			}
		}
		return -1;
	}
};

/*! @} */

} // End namespace

#endif
