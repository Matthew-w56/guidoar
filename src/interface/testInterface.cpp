
#include "testInterface.h"

#include <iostream>

#include "ARTypes.h"
#include "guidoparser.h"
#include "guidoelement.h"
#include "elementoperationvisitor.h"
#include "ARNote.h"
#include "ARFactory.h"

using std::cout;
using std::ostringstream;
using guido::SARMusic;
using guido::guidoparser;
using guido::Sguidoelement;
using guido::rational;
using guido::elementoperationvisitor;
using guido::SARNote;
using guido::ARFactory;
using guido::NewNoteInfo;
using guido::OpResult;

// ------------------------------------[ Helper Methods (public methods below) ]-----------------------------------------

static SARMusic read (const char* buff)
{
	if (!buff) return 0;
	guidoparser r;
	return r.parseString(buff);
}

static char* getPersistentPointer(std::string stringObj) {
	int len = int(stringObj.size());
	char* textOutput = (char*)malloc(len + 2); // +2 to allow me to pad end
	memcpy(textOutput, stringObj.c_str(), len);
	textOutput[len] = 0;
	textOutput[len+1] = 0;
	return textOutput;
}


// ---------------------------------------[ Public Method Definitions ]---------------------------------------------

/**
 *  Reads in the file referenced as a Guido score, deletes the note in question,
 *  and then returns the text data for the new score.
 * 
 *  Voice is given in 1-based counting, and this method transfers it to 0-based counting.
 */
char* deleteEvent(const char* scoreData, int num, int den, unsigned int voice, int midiPitch) {
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	// Initialize the objects needed with raw parameter data
	elementoperationvisitor visitor;
	rational time = rational(num, den);
	
	// Run the delete routine
	visitor.deleteEvent(score, time, voice-1, midiPitch);
	
	// Print the score
	ostringstream oss;
	score->print(oss);
	
	// Return a pointer to the text data
	return getPersistentPointer(oss.str());
}

/**
 *  Reads in the file referenced as a Guido score, inserts a note, and
 *  then returns the text data for the new score
 * 
 *  Voice is given in 1-based counting, and this method transfers it to 0-based counting.
 */
char* insertNote(const char* scoreData, int startNum, int startDen, int durNum, int durDen, int midiPitch, int voice, int dots, int insistedAccidental) {
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	// Initialize the objects needed with raw parameter data
	elementoperationvisitor visitor;
	// Create the note that we will insert
	NewNoteInfo noteInfo;
	noteInfo.durStartNum = startNum;
	noteInfo.durStartDen = startDen;
	noteInfo.durLengthNum = durNum;
	noteInfo.durLengthDen = durDen;
	noteInfo.voice = voice-1;
	noteInfo.midiPitch = midiPitch;
	noteInfo.dots = dots;
	noteInfo.insistedAccidental = insistedAccidental;
	
	// Run the delete routine
	OpResult result = visitor.insertNote(score, noteInfo);
	
	// If we need to try again, handle the old result and do so
	if (result == OpResult::needsMeasureAdded) {
		std::cout.flush();
		visitor.appendMeasure(score);
		OpResult secondResult = visitor.insertNote(score, noteInfo);
	}
	
	// Print the score
	ostringstream oss;
	score->print(oss);
	
	// Return a pointer to the text data
	return getPersistentPointer(oss.str());
}

char* setDurationAndDots(const char* scoreData, int elStartNum, int elStartDen, int voice, int newDurNum, int newDurDen, int newDots) {
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	rational desiredDur;
	if (newDurNum == 0 || newDurDen == 0) desiredDur = rational(0, 1);
	else desiredDur = rational(newDurNum, newDurDen);
	
	elementoperationvisitor visitor;
	OpResult result = visitor.setDurationAndDots(score, rational(elStartNum, elStartDen), voice-1, desiredDur, newDots);
	
	if (result != OpResult::success) {
		return "ERROR Problem during operation!";
	}
	
	// Print the score
	ostringstream oss;
	score->print(oss);
	
	// Return a pointer to the text data
	return getPersistentPointer(oss.str());
}

char* setAccidental(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int newAccidental, int* resultPitch) {
	if (elStartDen == 0) return "ERROR Start Duration denominator cannot be zero!";
	if (voice < 0) return "ERROR Voice cannot be < 0!";
	
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	elementoperationvisitor visitor;
	
	visitor.setAccidental(score, rational(elStartNum, elStartDen), voice-1, midiPitch, newAccidental, resultPitch);
	
	// Print the score
	ostringstream oss;
	score->print(oss);
	
	// Return a pointer to the text data
	return getPersistentPointer(oss.str());
}

char* setNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int oldPitch, int newPitch) {
	if (elStartDen == 0) return "ERROR Start Duration denominator cannot be zero!";
	if (voice < 0) return "ERROR Voice cannot be < 0!";
	
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	elementoperationvisitor visitor;
	
	visitor.setNotePitch(score, rational(elStartNum, elStartDen), voice-1, oldPitch, newPitch);
	
	// Print the score
	ostringstream oss;
	score->print(oss);
	
	// Return a pointer to the text data
	return getPersistentPointer(oss.str());
}

char* shiftNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int pitchShiftDirection, int* resultPitch) {
	if (elStartDen == 0) return "ERROR Start Duration denominator cannot be zero!";
	if (voice < 0) return "ERROR Voice cannot be < 0!";
	
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	// Normalize the pitch shift direction to be +/- 1, or zero
	if (pitchShiftDirection != 0) {
		pitchShiftDirection /= abs(pitchShiftDirection);
	}
	
	elementoperationvisitor visitor;
	
	visitor.shiftNotePitch(score, rational(elStartNum, elStartDen), voice-1, midiPitch, pitchShiftDirection, resultPitch);
	
	// Print the score
	ostringstream oss;
	score->print(oss);
	
	// Return a pointer to the text data
	return getPersistentPointer(oss.str());
}
