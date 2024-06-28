/*

Author: Matthew Williams
Date: June 2024


This visitor's job is to find a specific element (Note, Chord, System, etc),
and do one thing to it.  Ideally this will include adding notes, replacing
notes with rests and vice versa, etc.  This is built to enable applications
that edit the sheet music real-time.

Matthew Williams
mbwilliams226@gmail.com

*/

#ifndef __ElementOperationVisitor__
#define __ElementOperationVisitor__

#include "arexport.h"
#include "durationvisitor.h"
#include "visitor.h"
#include "ARNote.h"
#include "ARChord.h"
#include "ARFactory.h"

namespace guido
{


/*!
\addtogroup visitors
@{
*/

enum OpIntent {
	InsertNote, DeleteEvent
};


class gar_export elementoperationvisitor :
	public durationvisitor
{
	
	public:
		         elementoperationvisitor() {  }
		virtual ~elementoperationvisitor() {  }
		
		bool 	deleteEvent (const Sguidoelement& score, const rational& time, unsigned int voiceIndex=0, int midiPitch=-1);
		bool	insertNote	(const Sguidoelement& score, const rational& time, SARNote& elt, unsigned int voiceIndex=0);
		
		
		bool done();
		bool done(SARNote& elt);
		void init();
		
		virtual void visitStart ( SARVoice& elt );
		virtual void visitStart ( SARNote& elt );
		virtual void visitStart ( SARChord& elt );

		virtual void visitEnd   ( SARVoice& elt );
		virtual void visitEnd   ( SARChord& elt );
		
	protected:
		rational		fTargetDate;
		unsigned int	fTargetVoice;
		unsigned int	fCurrentVoice;
		SARVoice		fCurrentVoiceRef;
		SARChord		fCurrentChordRef;
		bool			fDone;
		OpIntent		fOpIntent;
		int				fMidiPitch;
		// Used to return results from a browse to an edit method
		SARVoice		fResultVoice;
		SARChord		fResultChord;
		SARNote			fResultNote;
		
};

/*! @} */

} // End namespace

#endif
