
#include "testInterface.h"

#include <iostream>
#include <vector>

#include "ARTypes.h"
#include "guidoparser.h"
#include "guidoelement.h"
#include "elementoperationvisitor.h"
#include "ARNote.h"
#include "ARFactory.h"
#include "libguidoar.h"
#include "tailOperation.h"
#include "headOperation.h"
#include "topOperation.h"
#include "bottomOperation.h"
#include "countvoicesvisitor.h"
#include "durationvisitor.h"
#include "seqOperation.h"
#include "getvoicesvisitor.h"
#include "extendVisitor.h"

using std::cout;
using std::vector;
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
using guido::tailOperation;
using guido::headOperation;
using guido::topOperation;
using guido::countvoicesvisitor;
using guido::bottomOperation;
using guido::durationvisitor;
using guido::seqOperation;
using guido::getvoicesvisitor;
using guido::SARVoice;
using guido::extendVisitor;

// ------------------------------------[ Helper Methods (public methods below) ]-----------------------------------------
rational doubleToRational(double input) {
	int denominator = 1;
	double numerDouble = input;
	while ((int)(numerDouble) != numerDouble) {
		denominator *= 10;
		numerDouble *= 10;
	}
	rational output = rational((int)(numerDouble), denominator).rationalise();
	return output;
}

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
	
	// Print the score to a string buffer
	ostringstream oss;
	score->print(oss);
	
	// Return a pointer to the text data
	return getPersistentPointer(oss.str());
}

/**
 *  Reads in the file referenced as a Guido score, deletes the elements in the range
 *  given.  Returns the text data for the resulting score.
 * 
 *  Voice is given in 1-based counting, and this method transfers it to 0-based counting.
 */
char* deleteRange(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice) {
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	// Initialize the variables to pass in
	elementoperationvisitor visitor;
	rational startTime = rational(startNum, startDen);
	rational endTime = rational(endNum, endDen);
	std::cout << "Deleting from " << startTime << "-" << endTime << "\n";
	std::cout.flush();
	
	// Run the deleteRange method
	OpResult result = visitor.deleteRange(score, startTime, endTime, startVoice-1, endVoice-1);
	
	if (result != OpResult::success) {
		return "ERROR: Problem in delete range operation! (Result not SUCCESS)";
	}
	
	// Print the score to a string buffer
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

char* shiftRangeNotePitch(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice, int pitchShiftDirection) {
	if (startDen == 0 || endDen == 0) return "ERROR Duration denominators cannot be zero!";
	if (startVoice < 0 || endVoice < 0) return "ERROR Voices cannot be < 0!";
	
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Couldn't read score!  (No score operation performed)";
	
	// Normalize the pitch shift direction to be +/- 1, or zero
	if (pitchShiftDirection != 0) {
		pitchShiftDirection /= abs(pitchShiftDirection);
	}
	
	// Create the rationals needed
	rational startTime = rational(startNum, startDen);
	rational endTime = rational(endNum, endDen);
	
	elementoperationvisitor visitor;
	
	visitor.shiftRangeNotePitch(
		score,
		startTime,
		endTime,
		startVoice-1,
		endVoice-1,
		pitchShiftDirection
	);
	
	// Print the score
	ostringstream oss;
	score->print(oss);
	
	// Return a pointer to the text data
	return getPersistentPointer(oss.str());
}

// Pass in voices in 1-based counting. This is adjust them.
char* getSelection(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice) {
	// Read and verify times
	rational startTime = rational(startNum, startDen);
	rational endTime = rational(endNum, endDen);
	if (startTime == endTime) return "ERROR selection was length of zero";
	if (startTime > endTime) {
		// Swap times so (start < end)
		rational temp = endTime;
		endTime = startTime;
		startTime = temp;
	}
	
	// Read the score.  If that fails, return error code as a string.
	Sguidoelement score = read(scoreData);
	if (!score) return "ERROR Couldn't read score!  (No score operation performed)";
	
	countvoicesvisitor voiceCounter;
	int voices = voiceCounter.count(score);
	startVoice--; endVoice--;  // Convert from 1-based counting to 0-based counting
	if (endVoice < startVoice) {
		int temp = startVoice;
		startVoice = endVoice;
		endVoice = temp;
	}
	if (startVoice < 0) startVoice = 0;
	if (endVoice > voices) endVoice = voices;
	
	// Do cut operations
	topOperation toperation;
	score = toperation(score, endVoice+1);  // +1 because it keeps the top X voices, and we changed X to be 0-based counting
	bottomOperation bottomOperation;
	score = bottomOperation(score, startVoice);
	
	if (! score) return "ERROR Score didn't make it through the voice cutting operations";
	
	tailOperation tailOperator;
	score = tailOperator(score, startTime, false);
	headOperation headOperator;
	score = headOperator(score, endTime - startTime);
	
	if (! score) return "ERROR Score didn't make it through the duration cutting operations";
	
	// Return string
	ostringstream oss;
	score->print(oss);
	return getPersistentPointer(oss.str());
}

char* pasteToDuration(const char* scoreData, const char* selectionData, int startNum, int startDen, int startVoice) {
	rational startDur = rational(startNum, startDen);
	
	// Read the score and selection.  If that fails, return error code as a string.
	Sguidoelement score = read(scoreData);
	if (!score) return "ERROR Couldn't read score!  (No score operation performed)";
	Sguidoelement selection = read(selectionData);
	if (!selection) return "ERROR Couldn't read selection!  (No score operation performed)";
	
	// Count how many voices are in the score and selection
	countvoicesvisitor voiceCounter;
	int scoreVoices = voiceCounter.count(score);
	int selectionVoices = voiceCounter.count(selection);
	
	// If the voice counts don't line up, deal with it
	if (scoreVoices == 0 || selectionVoices == 0) return "ERROR Either score or selection have zero voices";
	if (selectionVoices > scoreVoices) {
		// Cut the bottom voices off of the selection so it fits into score
		topOperation toperation;
		selection = toperation(selection, scoreVoices);
		selectionVoices = scoreVoices;
	}
	
	// If trying to paste multi-voice selection too far down the voices, shift it up to fit in the score
	startVoice--;
	int largestPossibleStartVoice = scoreVoices - selectionVoices;
	if (startVoice > largestPossibleStartVoice) startVoice = largestPossibleStartVoice;
	
	// Find how long the score and selection are
	durationvisitor dvis;
	rational scoreDur = dvis.duration(score);
	rational selectionDur = dvis.duration(selection);
	
	// If we need to, extend the base score
	if (scoreDur < selectionDur + startDur) {
		guido::extendVisitor extender;
		score = extender.extend(score, selectionDur + startDur);
	}
	
	getvoicesvisitor gvv;
	vector<SARVoice> selectionVoiceList = gvv(selection);
	elementoperationvisitor visitor;
	for (int i = 0; i < selectionVoiceList.size(); i++) {
		printf("About to insert on voice %d\n", startVoice+i);
		std::cout.flush();
		OpResult result = visitor.insertRange(score, selectionVoiceList.at(i), startDur, startVoice + i, selectionDur);
		if (result != OpResult::success) return "ERROR Insert range wasn't a success";
	}
	printf("Done doing the actual operation.  About to print..\n");
	std::cout.flush();
	
	// Return string
	ostringstream oss;
	score->print(oss);
	printf("Done printing score\n");
	std::cout.flush();
	return getPersistentPointer(oss.str());
}

char* testMethod(const char* scoreData) {
	ostringstream oss;
	guido::guidoVMultDuration(scoreData, 2.0f, oss);
	return getPersistentPointer(oss.str());
}
