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
	InsertNote, DeleteEvent, AddMeasure, SetElementProperties, SetGroupProperties, DeleteRange
};

struct NewNoteInfo {
	int voice,
		midiPitch,
		durStartNum,
		durStartDen,
		durLengthNum,
		durLengthDen,
		dots = 0,
		insistedAccidental = 0;
};

enum OpResult { none, success, needsMeasureAdded, failure, noActionTaken };


class gar_export elementoperationvisitor :
	public durationvisitor,
	public visitor<Sguidotag>
{
	
	public:
		         elementoperationvisitor() {  }
		virtual ~elementoperationvisitor() {  }
		
		// Methods to add/remove elements
		OpResult 	deleteEvent   (const Sguidoelement& score, const rational& time, unsigned int voiceIndex, int midiPitch=-1);
		OpResult	deleteRange	  (const Sguidoelement& score, const rational& startTime, const rational& endTime, int startVoice, int endVoice);
		OpResult	insertNote	  (const Sguidoelement& score, NewNoteInfo noteInfo);
		
		// Methods to adjust score as a whole
		void		appendMeasure (const Sguidoelement& score);
		
		// Methods that adjust existing elements
		OpResult	setDurationAndDots(const Sguidoelement& score, const rational& time, int voice, rational newDur, int newDots);
		OpResult	setAccidental(const Sguidoelement& score, const rational& time, int voice, int midiPitch, int newAccidental, int* resultPitch);
		OpResult 	setNotePitch(const Sguidoelement& score, const rational& time, int voice, int oldPitch, int newPitch);
		OpResult 	shiftNotePitch(const Sguidoelement& score, const rational& time, int voice, int midiPitch, int pitchShiftDirection, int* resultPitch);
		
		
		bool done();
		bool done(SARNote& elt);
		void init();
		
		virtual void visitStart ( SARVoice& elt );
		virtual void visitStart ( SARNote& elt );
		virtual void visitStart ( SARChord& elt );
		virtual void visitStart ( Sguidotag& tag );

		virtual void visitEnd   ( SARVoice& elt );
		virtual void visitEnd   ( SARChord& elt );
		
	protected:
	
		void 		handleEqualDurationsNoteInsertion(SARNote& noteToAdd);
		OpResult 	cutScoreAndInsert(SARVoice& voice, Sguidoelement existing, std::vector<Sguidoelement> newEls);
	
			// These represent what we are looking for
		rational		fTargetDate;
		unsigned int	fTargetVoice;
		int				fMidiPitch;
			// These represent the current state of the browsing
		unsigned int	fCurrentVoiceNum;
		SARVoice		fCurrentVoiceRef;
		SARChord		fCurrentChordRef;
		int				fCurrentKeySignature = 0;
		std::string		fCurrentMeter = "";
		bool			fDone;
			// These represent data that comes from the public methods
		OpIntent		fOpIntent;
		// Used to return results from a browse to an edit method
		SARVoice		fResultVoice;
		SARChord		fResultChord;
		SARNote			fResultNote;
		
};

/*! @} */

} // End namespace

#endif
