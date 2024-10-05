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
		OpResult  	insertRange	  (const Sguidoelement& score, SARVoice elsToAdd, rational startTime, int voice, rational insertListDur);
		
		// Methods that adjust existing elements
		OpResult	setDurationAndDots(const Sguidoelement& score, const rational& time, int voice, rational newDur, int newDots);
		OpResult	setAccidental(const Sguidoelement& score, const rational& time, int voice, int midiPitch, int newAccidental, int* resultPitch);
		OpResult 	setNotePitch(const Sguidoelement& score, const rational& time, int voice, int oldPitch, int newPitch);
		OpResult 	shiftNotePitch(const Sguidoelement& score, const rational& time, int voice, int midiPitch, int pitchShiftDirection, int octaveShift, int* resultPitch);
		OpResult	shiftRangeNotePitch(const Sguidoelement& score, const rational& startTime, const rational& endTime, int startVoice, int endVoice, int pitchShiftDirection, int octaveShift);
		OpResult	setVoiceInstrument(const Sguidoelement& score, int voice, const char* instrName, int instrCode);
		
		
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
	
		void 		findResultVoiceChordNote(const Sguidoelement& score, rational time, int voice, int midiPitch);
		OpResult 	cutScoreAndInsert(SARVoice& voice, Sguidoelement existing, std::vector<Sguidoelement> newEls);
		/*! \brief Takes in a list of new elements to insert into the score (and the time to start adding them
				at), and removes existing elements that take up that space so that the score remains the
				same duration.
			\param voice The voice to add the elements to (only works with one voice at a time)
			\param existing The first element in the existing voice to replace
			\param newEls The list of elements to add to the voice
			\param insertListDur The duration of the new elements combined (pass in rational(-1, 1) to have the method attempt to find this itself)
		*/
		OpResult 	cutScoreAndInsert(SARVoice& voice, Sguidoelement existing, std::vector<Sguidoelement> newEls, rational insertListDur);
	
		// These represent what we are looking for
		rational		fTargetDate;
		unsigned int	fTargetVoice;
		int				fTargetMidiPitch;
		// These represent the current state of the browsing
		unsigned int	fCurrentVoiceNum;
		SARVoice		fCurrentVoiceRef;
		SARChord		fCurrentChordRef;
		int				fCurrentKeySignature = 0;
		std::string		fCurrentMeter = "";
		Sguidotag		fCurrentInstrument;
		bool			fDone;
		// Used to return results from a browse to an edit method
		SARVoice		fResultVoice;
		SARChord		fResultChord;
		SARNote			fResultNote;
		bool			fFoundNote;
		bool			fFoundChord;
		
};

/*! @} */

} // End namespace

#endif
