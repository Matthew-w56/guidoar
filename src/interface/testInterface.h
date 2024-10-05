

#ifndef TESTINTERFACE_H
#define TESTINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

struct vinforaw{
	int voiceNum,
		initInstrCode;
	const char* initClef,
				*initInstrName;
};
typedef struct vinforaw VoiceInfo;

#include "arexport.h"

/*! \brief Deletes an event that starts at the given duration in the form num/den, on the given voice.

	If midiPitch is not -1, the pitch will also be used to match to find the right event to
	delete. This allows for deleting a note from a chord without deleting the whole chord.
	
	\param scoreData The GMN data for the score to work with
	\param num The numerator of the element's start duration
	\param den The denominator of the element's start duration
	\param voice The voice the event belongs to
	\param midiPitch The pitch of the event (note) to delete.  Pass in -1 to abstain from checking midi pitch.
*/
gar_export char* deleteEvent(const char* scoreData, int num, int den, unsigned int voice, int midiPitch);

/*! \brief Deletes all events that lie in the given range on the given voice(s).

	All events includes notes, tags, etc that lie in the range given by the start duration to the end duration.
	
	\param scoreData The GMN data for the score to work with
	\param startNum The numerator of the range's start duration
	\param startDen The denominator of the range's start duration
	\param endNum The numerator of the range's end duration
	\param endDen The denominator of the range's end duration
	\param startVoice The first voice to start deleting things from
	\param endVoice The last voice to delete things from  (if this == startVoice, only the startVoice will be affected)
*/
gar_export char* deleteRange(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice);

/*! \brief Inserts a note in the given location with the information passed in.

	\param scoreData The GMN data for the score to work with
	\param startNum The numerator of the duration where this note should start
	\param startDen The denominator of the duration where this note should start
	\param durNum The numerator of the length this note should be (for half note, = 1)
	\param durDen The denominator of the length this note should be (for half note, = 2)
	\param midiPitch The midi pitch this note should be (When \d insistedAccidental is == -7, accidental is also found using this pitch)
	\param voice The voice the note should be added to
	\param dots The number of dots to give this note
	\param insistedAccidental The accidental to force this note to ([-2, 2]).  Pass -7 to allow any natural accidental for the score's key signature.
*/
gar_export char* insertNote(const char* scoreData, int startNum, int startDen, int durNum, int durDen, int midiPitch, int voice, int dots, int insistedAccidental);

/*! \brief Sets the duration and dots of the event at the given location.

	\param scoreData The GMN data for the score to work with
	\param elStartNum The numerator of where the event starts
	\param elStartDen The denominator of where the event starts
	\param voice The voice the event lives in
	\param newDurNum The new duration's numerator
	\param newDurDen The new duration's denominator
	\param newDots The number of dots desired for the event
*/
gar_export char* setDurationAndDots(const char* scoreData, int elStartNum, int elStartDen, int voice, int newDurNum, int newDurDen, int newDots);

gar_export char* setAccidental(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int newAccidental, int* resultPitch);
gar_export char* setNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int oldPitch, int newPitch);
gar_export char* shiftNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int pitchShiftDirection, int octaveShift, int* resultPitch);
gar_export char* shiftRangeNotePitch(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice, int pitchShiftDirection, int octaveShift);

gar_export char* getSelection(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice);
gar_export char* pasteToDuration(const char* scoreData, const char* selectionData, int startNum, int startDen, int startVoice);

// Voice Operations and Queries

gar_export VoiceInfo* getVoicesInfo(const char* scoreData, int* voiceCountOut);
gar_export char* addBlankVoice(const char* scoreData);
gar_export char* deleteVoice(const char* scoreData, int voiceToDelete);
gar_export char* setVoiceInitClef(const char* scoreData, const char* initClef);
gar_export char* setVoiceInitInstrument(const char* scoreData, const char* instrumentName, int instrumentCode);

#ifdef __cplusplus
}
#endif

#endif