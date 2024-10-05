
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
#include "tagvisitor.h"
#include "parOperation.h"
#include "removevoiceOperation.h"

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
using guido::tagvisitor;
using guido::parOperation;
using guido::ARMusic;
using guido::guidoattribute;

#define PRINT(s) std::cout << s << "\n" << std::flush;

static double lastAccoladeId = 91;

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
	OpResult result = visitor.deleteEvent(score, time, voice-1, midiPitch);
	ostringstream oss;
	
	// If an error happened, return info about what it was
	if (result != OpResult::success) {
		oss << "ERROR Could not DELETE EVENT!  Error code: " << result;
		return getPersistentPointer(oss.str());
	}
	
	// Otherwise, return the score
	score->print(oss);
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
	
	// Run the deleteRange method
	OpResult result = visitor.deleteRange(score, startTime, endTime, startVoice-1, endVoice-1);
	ostringstream oss;
	
	// If an error happened, return info about what it was
	if (result != OpResult::success) {
		oss << "ERROR Could not DELETE RANGE!  Error code: " << result;
		return getPersistentPointer(oss.str());
	}
	
	// Otherwise, return the score
	score->print(oss);
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
	Sguidoelement score = read(scoreData);
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
	
	// If we need to, extend the base score
	durationvisitor dvis;
	rational scoreDur = dvis.duration(score);
	rational noteStartDur = rational(startNum, startDen);
	rational insertNoteDur = rational(durNum, durDen);
	if (scoreDur < noteStartDur + insertNoteDur) {
		guido::extendVisitor extender;
		score = extender.extend(score, noteStartDur + insertNoteDur);
	}
	
	// Run the delete routine
	OpResult result = visitor.insertNote(score, noteInfo);
	ostringstream oss;
	
	// If an error happened, return info about what it was
	if (result != OpResult::success) {
		oss << "ERROR Could not INSERT NOTE!  Error code: " << result;
		return getPersistentPointer(oss.str());
	}
	
	// Otherwise, return the score
	score->print(oss);
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
	ostringstream oss;
	
	// If an error happened, return info about what it was
	if (result != OpResult::success) {
		oss << "ERROR Could not SET DURATION AND DOTS!  Error code: " << result;
		return getPersistentPointer(oss.str());
	}
	
	// Otherwise, return the score
	score->print(oss);
	return getPersistentPointer(oss.str());
}

char* setAccidental(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int newAccidental, int* resultPitch) {
	if (elStartDen == 0) return "ERROR Start Duration denominator cannot be zero!";
	if (voice < 0) return "ERROR Voice cannot be < 0!";
	
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	elementoperationvisitor visitor;
	
	OpResult result = visitor.setAccidental(score, rational(elStartNum, elStartDen), voice-1, midiPitch, newAccidental, resultPitch);
	ostringstream oss;
	
	// If an error happened, return info about what it was
	if (result != OpResult::success) {
		oss << "ERROR Could not SET ACCIDENTAL!  Error code: " << result;
		return getPersistentPointer(oss.str());
	}
	
	// Otherwise, return the score
	score->print(oss);
	return getPersistentPointer(oss.str());
}

char* setNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int oldPitch, int newPitch) {
	if (elStartDen == 0) return "ERROR Start Duration denominator cannot be zero!";
	if (voice < 0) return "ERROR Voice cannot be < 0!";
	
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "ERROR Error reading score!  (No score operation performed)";
	
	elementoperationvisitor visitor;
	OpResult result = visitor.setNotePitch(score, rational(elStartNum, elStartDen), voice-1, oldPitch, newPitch);
	ostringstream oss;
	
	// If an error happened, return info about what it was
	if (result != OpResult::success) {
		oss << "ERROR Could not SET NOTE PITCH!  Error code: " << result;
		return getPersistentPointer(oss.str());
	}
	
	// Otherwise, return the score
	score->print(oss);
	return getPersistentPointer(oss.str());
}

char* shiftNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int pitchShiftDirection, int octaveShift, int* resultPitch) {
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
	
	OpResult result = visitor.shiftNotePitch(score, rational(elStartNum, elStartDen), voice-1, midiPitch, pitchShiftDirection, octaveShift, resultPitch);
	ostringstream oss;
	
	// If an error happened, return info about what it was
	if (result != OpResult::success) {
		oss << "ERROR Could not SHIFT NOTE PITCH!  Error code: " << result;
		return getPersistentPointer(oss.str());
	}
	
	// Otherwise, return the score
	score->print(oss);
	return getPersistentPointer(oss.str());
}

char* shiftRangeNotePitch(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice, int pitchShiftDirection, int octaveShift) {
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
	
	OpResult result = visitor.shiftRangeNotePitch(
		score,
		startTime,
		endTime,
		startVoice-1,
		endVoice-1,
		pitchShiftDirection,
		octaveShift
	);
	ostringstream oss;
	
	// If an error happened, return info about what it was
	if (result != OpResult::success) {
		oss << "ERROR Could not SHIFT RANGE PITCH!  Error code: " << result;
		return getPersistentPointer(oss.str());
	}
	
	// Otherwise, return the score
	score->print(oss);
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
	if (!score) return "ERROR Couldn't read SCORE!  (No score operation performed)";
	Sguidoelement selection = read(selectionData);
	if (!selection) return "ERROR Couldn't read SELECTION!  (No score operation performed)";
	
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
		OpResult result = visitor.insertRange(score, selectionVoiceList.at(i), startDur, startVoice + i, selectionDur);
		if (result != OpResult::success) {
			ostringstream oss;
			oss << "ERROR Could not INSERT RANGE!  Error code: " << result;
			return getPersistentPointer(oss.str());
		}
	}
	
	// Return string
	ostringstream oss;
	score->print(oss);
	return getPersistentPointer(oss.str());
}

VoiceInfo* getVoicesInfo(const char* scoreData, int* voiceCountOut) {
	Sguidoelement score = read(scoreData);
	if (!score) {
		*voiceCountOut = 0;
		return nullptr;
	}
	
	getvoicesvisitor gvv;
	tagvisitor tv;
	vector<SARVoice> voices = gvv(score);
	const int vals = voices.size();
	std::vector<VoiceInfo> outList;
	for (int i = 0; i < voices.size(); i++) {
		guido::VoiceInitInfo vInfo = tv.getVoiceInfo(voices.at(i));
		VoiceInfo info;
		// TODO: FINISH THIS AND RETURN HELPFUL INFORMATION
		// info.initClef = vInfo.clef;
		// info.initInstrName = vInfo.instrumentName;
		// info.initInstrCode = vInfo.instrumentCode;
		info.voiceNum = i+1;
		outList.push_back(info);
	}
	// Output the information
	*voiceCountOut = outList.size();
	if (*voiceCountOut == 0) return nullptr;
	return &outList.at(0);
}

char* addBlankVoice(const char* scoreData) {
	SARMusic score = read(scoreData);
	if (!score) {
		return "ERROR Could not parse score data! (No action performed)";
	}
	
	// Get score length
	durationvisitor dvis;
	rational scoreDur = dvis.duration(score);
	// Get the voices
	getvoicesvisitor gvv;
	vector<SARVoice> voices = gvv(score);
	if (voices.size() == 0) return "ERROR Score has no voices!";
	// Get the information about the reference voice
	SARVoice reference = voices.at(voices.size()-1);
	tagvisitor tv;
	guido::VoiceInitInfo vInfo = tv.getVoiceInfo(reference);
	// Create blank voice to use
	SARVoice newVoice = ARFactory().createVoice();
	// Make this new voice a system of itself (Accolade)
	guido::Sguidotag tag = ARFactory().createTag("accol");
	guido::Sguidoattribute id_attr = guidoattribute::create();
	id_attr->setName(std::string("id"));
	id_attr->setValue(lastAccoladeId);
	guido::Sguidoattribute range_attr = guidoattribute::create();
	range_attr->setName("range");
	range_attr->setValue(lastAccoladeId);
	range_attr->setQuoteVal(true);
	guido::Sguidotag barFormat_tag = ARFactory().createTag("barFormat");
	guido::Sguidoattribute bfAttr = guidoattribute::create();
	bfAttr->setValue("system");
	tag->add(id_attr);
	tag->add(range_attr);
	barFormat_tag->add(bfAttr);
	newVoice->push(tag);
	newVoice->push(barFormat_tag);
	lastAccoladeId += 1;
	// Insert the Target's clef, key signature, meter, and instrument
	if (vInfo.instr) newVoice->push(vInfo.instr);
	if (vInfo.clef) newVoice->push(vInfo.clef);
	else newVoice->push(ARFactory().createTag("clef"));
	if (vInfo.keySignature) newVoice->push(vInfo.keySignature);
	if (vInfo.meter) newVoice->push(vInfo.meter);
	// Pass Target to extend visitor
	extendVisitor extV;
	Sguidoelement extendedVoice = extV.extend(newVoice, scoreDur);
	// Push Target to score
	score->push(extendedVoice);
	
	ostringstream oss;
	score->print(oss);
	return getPersistentPointer(oss.str());
}

char* deleteVoice(const char* scoreData, int voiceToDelete) {
	Sguidoelement score = read(scoreData);
	if (!score) {
		return "ERROR Could not parse score data! (No action performed)";
	}
	
	countvoicesvisitor cvv;
	int count = cvv.count(score);
	if (count == 1) {
		return "ERROR Cannot delete the last voice!";
	}
	
	guido::removeVoiceOperation rvo;
	score = rvo(score, voiceToDelete);
	
	// Return string
	ostringstream oss;
	score->print(oss);
	return getPersistentPointer(oss.str());
}

char* setVoiceInitClef(const char* scoreData, const char* initClef) {
	return "ERROR unimplemented";
}

char* setVoiceInitInstrument(const char* scoreData, const char* instrumentName, int instrumentCode) {
	return "ERROR unimplemented";
}

