
#include <iostream>

#include "AROthers.h"
#include "elementoperationvisitor.h"
#include "guidoelement.h"
#include "ARTag.h"
#include "ARChord.h"
#include "ARNote.h"
#include "guidotags.h"

namespace guido
{

// Determines whether we replace notes with equal rests or not
// To set this to false would be to make it so that deleting a chord
// or note just collapses the voice around that point, making that
// particular voice shorter than the rest.
static bool DO_REPLACE = true;
// Helps deal with implicit durations
static rational lastDurFound = rational(0, 1);
// Forward declare methods so the public methods don't complain
static bool deleteNoteFromChord(SARVoice voice, SARChord parent, SARNote child);
static bool deleteChordFromVoice(SARVoice parent, SARChord child, bool doReplace);
static bool replaceElementWith(SARVoice parent, Sguidoelement existing, Sguidoelement newEl); 
static bool deleteNoteFromVoice(SARVoice parent, SARNote child, bool doReplace);
static bool insertNoteOnRest(SARVoice parent, SARNote rest, SARNote newNote);
static bool insertNoteIntoChord(SARChord parent, SARNote newNote);
static bool insertNoteToCreateChord(SARVoice parent, SARNote existingNote, SARNote newNote);
static bool deleteElementFrom(Sguidoelement parent, Sguidoelement child);
static int breakNoteTo(SARVoice voice, SARNote el, rational newDur);
static int breakChordTo(SARVoice voice, SARChord el, rational newDur);
static SARNote getCopyOfNote(SARNote el);
static SARChord getCopyOfChord(SARChord el);
static bool checkSongDuration(SARVoice voice, rational desiredLength);
static void shiftNoteMidiPitchBy(SARNote note, int currentPitch, int pitchShiftDirection, int keySig, int octaveShift);
static OpResult shiftRangeMidiPitchBy(SARVoice voice, Sguidoelement startEl, rational rangeLength, int pitchShiftDirection, int keySig, int octaveShift);
static rational getRealDuration(Sguidoelement el);

static void print(char* input) {
	std::cout << input;
	std::cout.flush();
}
static void print(int input) {
	std::cout << input;
	std::cout.flush();
}
static void print(double input) {
	std::cout << input;
	std::cout.flush();
}
static void print(Sguidoattribute input) {
	std::cout << input->getName() << ": " << input->getValue() << " (in " << input->getUnit() << ")";
	std::cout.flush();
}
static void print(bool input) {
	std::cout << (input ? "true" : "false");
	std::cout.flush();
}
static void print(unsigned int input) {
	std::cout << input;
	std::cout.flush();
}
static void print(Sguidoattributes input) {
	std::cout << "Attribute list:\n";
	for (Sguidoattributes::iterator s = input.begin(); s < input.end(); s++) {
		std::cout << "  - " << (*s)->getName() << ": " << (*s)->getValue() << "  (" << (*s)->getUnit() << ")\n";
	}
	std::cout.flush();
}
static void print(rational input) {
	std::cout << input;
	std::cout.flush();
}
static void printP(void* p) {
	std::cout << p;
	std::cout.flush();
}
static void print(ARVoice* voice) {
	auto it2 = voice->begin();
	std::cout << "\n\nVoice Contents:\n";
	while (it2 != voice->end()) {
		std::cout << "   " << (*it2)->getName() << " \t" << getRealDuration(*it2) << std::endl;
		it2.rightShift();
	}
	std::cout << "\n\n";
	std::cout.flush();
}

// These are for looking up data later
static int isAccidentalLookup[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
static std::string noteNamesSharp[12] = {"c", "c", "d", "d", "e", "f", "f", "g", "g", "a", "a", "b"};
static std::string noteNamesFlat[12] = {"c", "d", "d", "e", "e", "f", "g", "g", "a", "a", "b", "b"};
static std::string noteNames[12] = {"c", "n", "d", "n", "e", "f", "n", "g", "n", "a", "n", "b"};
// For these lists, 9 means never affected because keys go to +- 7.
static int isAffectedBySigSharp[12] = {2, 9, 4, 9, 6, 1, 9, 3, 9, 5, 9, 7};
static int isAffectedBySigFlat[12] = {6, 9, 4, 9, 2, 7, 9, 5, 9, 3, 9, 1};
/// Pitch jump for white notes from C to C inclusive
/// So [0] is C->D, [1] is D->E, etc. Last is B->C
/// Designed so you can add (pitchesToAdd[index for note] when moving
/// upwards, and minus pitchesToAdd[index -1] when moving down)
std::vector<int> pitchesToAdd = {2, 2, 1, 2, 2, 2, 1};
// Start index given pitch from C
std::vector<int> pitchStartIndexFromPitchFromC = {0, 999, 1, 999, 2, 3, 999, 4, 999, 5, 999, 6};

static int getKeySigAccidental(int pitchFromC, int keySignature) {
	if (keySignature == 0) {
		return 0;
	} else if (keySignature > 0 && keySignature >= isAffectedBySigSharp[pitchFromC]) {
		return 1;
	} else if (keySignature < 0 && (-keySignature) >= isAffectedBySigFlat[pitchFromC]) {
		return -1;
	}
	return 0;
}

// DONE - UNTESTED
static SARNote createNote(NewNoteInfo noteInfo, int keySignature) {
	// Get the note name wanted
	int pitchFromC = noteInfo.midiPitch % 12;
	std::string noteName = noteNames[pitchFromC];
	// Get the octave wanted
	int octave = (noteInfo.midiPitch / 12) - 1;
	// Get the accidental wanted
	int accidental = noteInfo.insistedAccidental;
	if (accidental == -7) { // -7 is code for: I don't care - just follow key signature
		accidental = getKeySigAccidental(pitchFromC, keySignature);
	}
	
	// Create the note itself
	SARNote note = ARFactory().createNote(noteName);
	// Set a few attributes of the note
	note->SetOctave(octave);
	note->SetAccidental(accidental);
	note->SetDots(noteInfo.dots);
	
	// Set the duration
	*note = rational(noteInfo.durLengthNum, noteInfo.durLengthDen);
	
	// Return the result
	return note;
}

// Helper to pull FULL duration
static rational getRealDuration(Sguidoelement el) {
	ARChord* isChord = dynamic_cast<ARChord*>((&*el));
	ARNote* isNote = dynamic_cast<ARNote*>((&*el));
	if (isChord != nullptr) {
		rational durPlain = isChord->duration();
		int dots = isChord->GetDots();
		if (dots > 2 || dots < 0) dots = 0;
		return isChord->totalduration(durPlain, dots);
	} else if (isNote != nullptr) {
		rational durPlain = isNote->duration();
		int dots = isNote->GetDots();
		if (dots > 2 || dots < 0) dots = 0;
		return isNote->totalduration(durPlain, dots);
	}
	rational durOut = ARNote::getImplicitDuration();
	int dots = 0;
	return el->totalduration(durOut, dots);
}

static void getInfoFromMidiPitch(int midiPitch, int keySig, std::string* name, int* octave, int* accidental) {
	// Initialize variables to fill
	int pitchFromC = midiPitch % 12;
	
	// If this note isn't a natural
	if (isAccidentalLookup[pitchFromC]) {
		// Use what the key signature uses, or default to sharps
		bool useSharps = true;
		if (keySig < 0) useSharps = false;
		*name = (useSharps ? noteNamesSharp[pitchFromC] : noteNamesFlat[pitchFromC]);
		*accidental = (useSharps ? 1 : -1);
	} else {
		*name = noteNames[pitchFromC];
		*accidental = 0;
	}
	
	// Get the octave wanted
	*octave = (midiPitch / 12) - 4;
}


// ---------------------------[ Public Methods ]---------------------------------
// ---------------------------[ Public Methods ]---------------------------------
// ---------------------------[ Public Methods ]---------------------------------
// ---------------------------[ Public Methods ]---------------------------------

OpResult elementoperationvisitor::deleteEvent(const Sguidoelement& score, const rational& time, unsigned int voiceIndex, int midiPitch) {
	// Start off by finding where we are working in the score
	findResultVoiceChordNote(score, time, voiceIndex, midiPitch);
	
	// Decide what action to take
	if (fFoundNote && !fFoundChord) {
		// We want to delete a note that's NOT in a chord
		deleteNoteFromVoice(fResultVoice, fResultNote, DO_REPLACE);
	} else if (fFoundChord && !fFoundNote) {
		// We want to delete an entire chord
		deleteChordFromVoice(fResultVoice, fResultChord, DO_REPLACE);
	} else if (fFoundNote && fFoundChord) {
		// We want to delete a note that's IN a chord
		deleteNoteFromChord(fResultVoice, fResultChord, fResultNote);
	} else {
		// Both are null, nothing was found.
		print("Nothing found to delete at that point!  No action taken..\n");
		return OpResult::noActionTaken;
	}
	
	return OpResult::success;
}

OpResult elementoperationvisitor::deleteRange (const Sguidoelement& score, const rational& startTime, const rational& endTime, int startVoice, int endVoice) {
	for (int currVoice = startVoice; currVoice <= endVoice; currVoice++) {

		// Start off by finding where we are working in the score
		findResultVoiceChordNote(score, startTime, currVoice, -1);
		
		if (!fFoundNote && !fFoundChord) return OpResult::failure;
		
		// Assemble list of rests to fill gap with
		rationals restDursToAdd = rational::getBaseRationals(endTime - startTime);
		std::vector<Sguidoelement> restsToAdd;
		for (int i = 0; i < restDursToAdd.size(); i++) {
			SARNote rest = ARFactory().createNote("_");
			*rest = restDursToAdd.at(i);
			rest->SetDots(0);
			restsToAdd.push_back(rest);
		}
		
		// Run the method to replace that selection area
		OpResult result = fFoundChord
					? cutScoreAndInsert(fResultVoice, fResultChord, restsToAdd)
					: cutScoreAndInsert(fResultVoice, fResultNote, restsToAdd);
		
		// Stop early if something went wrong
		if (result != OpResult::success) return result;
	}
	
	return OpResult::success;
}

OpResult elementoperationvisitor::insertNote(const Sguidoelement& score, NewNoteInfo noteInfo) {
	// Start off by finding where we are working in the score
	findResultVoiceChordNote(score, rational(noteInfo.durStartNum, noteInfo.durStartDen), noteInfo.voice, -1);
	
	// Create the note that we'll add
	SARNote noteToAdd = createNote(noteInfo, fCurrentKeySignature);
	
	// Make sure the score is long enough for the new note
	rational startTime = rational(noteInfo.durStartNum, noteInfo.durStartDen);
	rational noteDuration = getRealDuration(noteToAdd);
	if (! checkSongDuration(fResultVoice, startTime + noteDuration)) {
		return OpResult::needsMeasureAdded;
	}
	
	// Stop here if we didn't find some note to attach this one to
	if (!fFoundNote) {
		print("No note found; Stopping early!\n");
		std::cout.flush();
		return OpResult::noActionTaken;
	}
	
	// Find out whether we can create/add to a chord
	bool durationsMatch = getRealDuration(noteToAdd) == getRealDuration(fResultNote);
	// Insert the note however is best given the durations of the new and existing note
	if (durationsMatch) {
		// Insert note without any need for moving other things around
		bool existingNoteIsRest = fResultNote->isRest();
		if (!fFoundChord && existingNoteIsRest) {
			// Just replace the rest with the note
			insertNoteOnRest(fResultVoice, fResultNote, noteToAdd);
		} else if (!fFoundChord && ! existingNoteIsRest) {
			// Create a chord around the existing note, and add the new one, too
			insertNoteToCreateChord(fResultVoice, fResultNote, noteToAdd);
		} else if (fFoundChord) {
			// We are in a chord (Just add note to the chord)
			insertNoteIntoChord(fResultChord, noteToAdd);
		}
	} else {
		// Get the new element, and then make space for it in the score
		std::vector<Sguidoelement> wrapper;
		wrapper.push_back(noteToAdd);
		return fFoundChord
				? cutScoreAndInsert(fResultVoice, fResultChord, wrapper)
				: cutScoreAndInsert(fResultVoice, fResultNote, wrapper);
	}
	
	return OpResult::success;
}

OpResult elementoperationvisitor::setDurationAndDots(const Sguidoelement& score, const rational& time, int voice, rational newDur, int newDots) {
	// Start off by finding where we are working in the score
	findResultVoiceChordNote(score, time, voice, -1);
	
	rational foundDurPlain = fFoundChord
				? fResultChord->duration()
				: fResultNote->duration();
	int foundDotsPlain = fFoundChord
				? fResultChord->GetDots()
				: fResultNote->GetDots();
	rational foundDur = fFoundChord
				? getRealDuration(fResultChord)
				: getRealDuration(fResultNote);
	
	// If the caller omitted the duration or dots, just use the values we found in our search
	if (newDur == rational(0, 1)) newDur = foundDurPlain;
	if (newDots == -1) newDots = foundDotsPlain;
	rational desiredDur = ARFactory().createNote("e")
				->totalduration(newDur, newDots);
	
	// We don't need to do anything if the desired duration matches current
	if (foundDur == desiredDur) return OpResult::success;
	
	// Decide how to go about this duration change
	if (desiredDur > foundDur) {
		// We need to cut out some of the score in front of us if the desired duration is longer than the existing one
		// Strategy: Remove the original element as well as the notes infront of it that
		//			 block the way, and then insert a new copy of the original element that
		//			 has the new duration and dots.
		std::vector<Sguidoelement> wrapper;
		// This is broken out because of inheritance problems with using Sguidoelement for chords and notes
		if (fResultChord != nullptr) {
			SARChord replacement = getCopyOfChord(fResultChord);
			*replacement = newDur;
			replacement->SetDots(newDots);
			wrapper.push_back(replacement);
		} else {
			SARNote replacement = getCopyOfNote(fResultNote);
			*replacement = newDur;
			replacement->SetDots(newDots);
			wrapper.push_back(replacement);
		}
		return fFoundChord
			? cutScoreAndInsert(fResultVoice, fResultChord, wrapper)
			: cutScoreAndInsert(fResultVoice, fResultNote, wrapper);
		
	} else /* desiredDur < foundDur */ {
		// Here, we need to shorted the current element, and then insert the rests needed to
		// fill the gap we created.
		if (fResultChord != nullptr) {
			*fResultChord = newDur;
			fResultChord->SetDots(newDots);
		} else {
			*fResultNote = newDur;
			fResultNote->SetDots(newDots);
		}
		// Find gap length
		rational gapLength = foundDur - desiredDur;
		rationals restsToCreate = rational::getBaseRationals(gapLength, false);
		auto it = fResultVoice->begin();
		// Seek to find our spot
		while ((*it) != fResultChord && (*it) != fResultNote && it != fResultVoice->end()) { it++; }
		// Stop if we didn't find it (sanity check)
		if (it == fResultVoice->end()) return OpResult::failure;
		
		// Insert the rests
		it.rightShift();
		bool atEndOfVoice = it == fResultVoice->end();
		for (int i = 0; i < restsToCreate.size(); i++) {
			SARNote rest = ARFactory().createNote("_");
			*rest = restsToCreate.at(i);
			rest->SetDots(0);
			if (atEndOfVoice) {
				fResultVoice->push(rest);
			} else {
				it = it->insert(it, rest);
			}
		}
	}

	return OpResult::success;
}

OpResult elementoperationvisitor::setAccidental(const Sguidoelement& score, const rational& time, int voice,
			int midiPitch, int newAccidental, int* resultPitch) {
	// Start off by finding where we are working in the score
	findResultVoiceChordNote(score, time, voice, midiPitch);
	
	// Stop if we couldn't find a note
	if (!fFoundNote) {
		print("Could not set accidental; Couldn't find note to do it to!\n");
		*resultPitch = midiPitch;
		return OpResult::failure;
	}
	
	// Set the accidental, and return the pitch of the new note
	fResultNote->SetAccidental(newAccidental);
	int octave = fResultNote->GetOctave();
	*resultPitch = fResultNote->midiPitch(octave);
	
	return OpResult::success;
}

OpResult elementoperationvisitor::setNotePitch(const Sguidoelement& score, const rational& time, int voice, int oldPitch, int newPitch) {
	// Start off by finding where we are working in the score
	findResultVoiceChordNote(score, time, voice, oldPitch);
	
	if (!fFoundNote) {
		print("Could not set note pitch; Couldn't find note to do it to!\n");
		return OpResult::failure;
	}
	
	// Pull the name, octave, and accidental from the desired pitch, in the context of the key signature at that location
	int octave, accidental;
	std::string name;
	getInfoFromMidiPitch(newPitch, fCurrentKeySignature, &name, &octave, &accidental);
	
	// Go and do the setting
	fResultNote->setName(name);
	fResultNote->SetOctave(octave);
	fResultNote->SetAccidental(accidental);
	
	return OpResult::success;
}

// PitchShiftDirection should be +1 or -1, but any positive or negative will shift in that direction by 1.
OpResult elementoperationvisitor::shiftNotePitch(const Sguidoelement& score, const rational& time, int voice, int midiPitch,
			int pitchShiftDirection, int octaveShift, int* resultPitch) {
	print("Doing shift note pitch!\n");
	// Start off by finding where we are working in the score
	findResultVoiceChordNote(score, time, voice, midiPitch);
	
	// Stop if we couldn't find a note to shift
	if (!fFoundNote) {
		print("Could not set note pitch; Couldn't find note to do it to!\n");
		*resultPitch = midiPitch;
		return OpResult::failure;
	}
	
	// Do the actual midi pitch shifting
	shiftNoteMidiPitchBy(fResultNote, midiPitch, pitchShiftDirection, fCurrentKeySignature, octaveShift);
	int octave = fResultNote->GetOctave();
	*resultPitch = fResultNote->midiPitch(octave);
	
	return OpResult::success;
}

OpResult elementoperationvisitor::shiftRangeNotePitch(const Sguidoelement& score, const rational& startTime, const rational& endTime, int startVoice, int endVoice, int pitchShiftDirection, int octaveShift) {
	rational rangeLength = endTime - startTime;
	for (int currVoice = startVoice; currVoice <= endVoice; currVoice++) {
		// Start off by finding where we are working in the score
		findResultVoiceChordNote(score, startTime, currVoice, -1);
		
		// Formalize what element we will work with
		Sguidoelement target = fResultNote;
		if (fFoundChord) target = fResultChord;
		
		OpResult result = shiftRangeMidiPitchBy(
			fResultVoice,
			target,
			rangeLength,
			pitchShiftDirection,
			fCurrentKeySignature,
			octaveShift
		);
		
		if (result != OpResult::success) return result;
	}
	return OpResult::success;
}

OpResult elementoperationvisitor::insertRange(const Sguidoelement& score, SARVoice elsToAdd, rational startTime, int voice, rational insertListDur) {
	// Start off by finding where we are working in the score
	findResultVoiceChordNote(score, startTime, voice, -1);
	
	// Stop if we didn't find anything
	if (!fFoundChord && !fFoundNote) return OpResult::failure;
	
	std::vector<Sguidoelement> childrenToAdd = elsToAdd->elements();
	Sguidoelement firstChild = childrenToAdd.at(0);
	if (firstChild && firstChild->getName() == "tie") {
		std::vector<Sguidoelement> grandChildren = firstChild->elements();
		childrenToAdd.erase(childrenToAdd.begin());
		for (int i = 0; i < grandChildren.size(); i++) {
			childrenToAdd.insert(childrenToAdd.begin(), grandChildren.at(i));
		}
	}
	
	// Do the actual insert
	Sguidoelement target = fResultNote;
	if (fFoundChord) target = fResultChord;
	return cutScoreAndInsert(fResultVoice, target, childrenToAdd, insertListDur);
}

OpResult elementoperationvisitor::setVoiceInstrument(const Sguidoelement& score, int voice, const char* instrName, int instrCode) {
	// Start off by finding the first note in the score, and seeing if that voice had an isntrument already
	findResultVoiceChordNote(score, rational(0, 1), voice, -1);
	
	if (fCurrentInstrument) {
		// Change the existing tag's attributes to match desired instrument
		auto attrs = fCurrentInstrument->attributes();
		bool setName = false;
		bool setCode = false;
		for (int a = 0; a < attrs.size(); a++) {
			auto attr = attrs.at(a);
			if (attr->getName() == "" || attr->getName() == "name") {
				attr->setValue(std::string(instrName));
				attr->setQuoteVal(true);
				setName = true;
			} else if (attr->getName() == "MIDI") {
				attr->setValue(0L + instrCode);
				setCode = true;
			}
		}
		// If existing attributes didn't exist to modify, add new ones
		if (!setName) {
			auto nameAttr = guidoattribute::create();
			nameAttr->setName("name");
			nameAttr->setValue(std::string(instrName));
			nameAttr->setQuoteVal(true);
			fCurrentInstrument->add(nameAttr);
		}
		if (!setCode) {
			auto codeAttr = guidoattribute::create();
			codeAttr->setName("MIDI");
			codeAttr->setValue(0L + instrCode);
			fCurrentInstrument->add(codeAttr);
		}
	} else {
		// Create a new tag to describe instrument desired
		Sguidotag instrTag = ARFactory().createTag("instr");
		auto nameAttr = guidoattribute::create();
		nameAttr->setName("name");
		nameAttr->setValue(std::string(instrName));
		nameAttr->setQuoteVal(true);
		instrTag->add(nameAttr);
		auto codeAttr = guidoattribute::create();
		codeAttr->setName("MIDI");
		codeAttr->setValue(0L + instrCode);
		instrTag->add(codeAttr);
		auto doAutoPos = guidoattribute::create();
		doAutoPos->setName("autopos");
		doAutoPos->setValue("true");
		doAutoPos->setQuoteVal(true);
		// Place tag inside of voice
		fResultVoice->insert(fResultVoice->begin(), instrTag);
	}
	
	return OpResult::success;
}

// ---------------------------[ End Public Methods ]------------------------------------
// ---------------------------[ End Public Methods ]------------------------------------
// ---------------------------[ End Public Methods ]------------------------------------
// ---------------------------[ End Public Methods ]------------------------------------


// -----------------------------[ Action Methods ]--------------------------------------

// This method is used by all the public methods.  It is used to turn a location (defined by a time, voice, and pitch) into
// the result member variables for voice, chord, note, and things like key signature, and meter.  Also provides two helper
// flags fFoundNote and fFoundChord.
//
// Set midiPitch to -1 to look for chords or groups, and specify any other midiPitch to only find elements with that exact pitch
void elementoperationvisitor::findResultVoiceChordNote(const Sguidoelement& score, rational time, int voice, int midiPitch) {
	fTargetVoice = voice;
	fTargetDate = time;
	fTargetMidiPitch = midiPitch;
	init();
	
	fBrowser.browse(*score);
	
	fFoundNote = fResultNote != nullptr;
	fFoundChord = fResultChord != nullptr;
}

static void shiftNoteMidiPitchBy(SARNote note, int currentPitch, int pitchShiftDirection, int keySig, int octaveShift) {
	if (pitchShiftDirection == 0 && octaveShift == 0) return;
	
	// Only shift octave if that's what we want.
	if (octaveShift != 0) {
		int octave = note->GetOctave();
		note->SetOctave(octave + octaveShift);
		return;
	}
	// Otherwise, prep to shift pitch
	
	// Steps:
	//	- Take the note (by midi pitch), and make it natural (remove accidentals)
	//	- Increment or decrement the natural note (c->d, a->g, etc)
	//	- Conform the new note to the given key signature (if signature is 1 flat, take B to Bb)
	//	- Set the note's properties to reflect our final note decision
	
	// Take the note (by midi pitch), and make it natural
	int pitchFromC = (currentPitch - note->GetAccidental()) % 12;
	int adjustmentIndex = pitchStartIndexFromPitchFromC[pitchFromC];
	if (pitchShiftDirection < 0) { adjustmentIndex -= 1;  if (adjustmentIndex < 0) adjustmentIndex += 7; }  // 7 is length of pitchesToAdd
	
	// Increment or decrement the natural note
	int newPitch = currentPitch - note->GetAccidental();
	if (pitchShiftDirection > 0) {
		newPitch += pitchesToAdd[adjustmentIndex];
	} else if (pitchShiftDirection < 0) {
		newPitch -= pitchesToAdd[adjustmentIndex];
	}
	
	// Conform the new note to the given key signature
	newPitch += getKeySigAccidental(newPitch % 12, keySig);
	
	// Set the note's properties to reflect our final note decision
	int octave, accidental;
	std::string name;
	getInfoFromMidiPitch(newPitch, keySig, &name, &octave, &accidental);
	
	// Go and do the setting
	note->setName(name);
	note->SetOctave(octave);
	note->SetAccidental(accidental);
}

static void shiftChordMidiPitchBy(ARChord* chord, int pitchShiftDirection, int keySig, int octaveShift) {
	for (int i = 0; i < chord->notes().size(); i++) {
		SARNote note = chord->notes().at(i);
		int octave = note->GetOctave();
		shiftNoteMidiPitchBy(note, note->midiPitch(octave), pitchShiftDirection, keySig, octaveShift);
	}
}

static OpResult shiftRangeMidiPitchBy(SARVoice voice, Sguidoelement startEl, rational rangeLength, int pitchShiftDirection, int keySig, int octaveShift) {
	// Seek to the start element in the voice
	auto it = voice->begin();
	while (startEl != (*it) && it != voice->end()) { it++; }
	
	if (it == voice->end()) return OpResult::failure;
	
	// Set up variables we'll use
	rational zeroR = rational(0, 1);
	rational implicitDur = ARNote::getImplicitDuration();
	lastDurFound = zeroR;
	rational currentDur = getRealDuration(*it);
	rational rangeLengthLeft = rangeLength;
	
	// Start running through voice and shifting things
	while (rangeLengthLeft > zeroR && it != voice->end()) {
		// Replace bad durations with the last duration found (that's how Guido does it)
		if (((*it)->getName() == "chord" && currentDur == zeroR) || currentDur == implicitDur) {
			currentDur = lastDurFound;
		} else {
			lastDurFound = currentDur;
		}
		// Parse the element to see if it's a chord or note
		ARChord* isChord = dynamic_cast<ARChord*>((&**it));
		ARNote* isNote = dynamic_cast<ARNote*>((&**it));
		if (isChord) {
			shiftChordMidiPitchBy(isChord, pitchShiftDirection, keySig, octaveShift);
			rangeLengthLeft -= currentDur;
		} else if (isNote && !isNote->isRest()) { // Exclude rests; moving them causes a crash
			int octave = isNote->GetOctave();
			shiftNoteMidiPitchBy(isNote, isNote->midiPitch(octave), pitchShiftDirection, keySig, octaveShift);
			rangeLengthLeft -= currentDur;
		}
		
		it.rightShift();
		if (rangeLengthLeft <= zeroR) break;
		if (it == voice->end()) break;
		currentDur = getRealDuration(*it);
	}
	
	return OpResult::success;
}

// This method starts deleting things at the element specified from the voice given, until it has enough room to insert all of
// the elements in the newEls list.  You can specify insertListDur to be rational(-1,1) to indicate that this method should try
// to find the length of the elements to add itself (or just call the method overload that does not require an insertListDur).
// This internal-list-length-finding is good many times, but fails in some scenarios.
OpResult elementoperationvisitor::cutScoreAndInsert(SARVoice& voice, Sguidoelement existing, std::vector<Sguidoelement> newEls, rational insertListDur) {
	// print("Starting cutScoreAndInsert Method!\n");
	// The amount of time we want to eat out of the score
	rational timeToEat = insertListDur;
	if (timeToEat == rational(-1, 1)) {
		timeToEat = rational(0, 1);
		for (int i = 0; i < newEls.size(); i++) {
			timeToEat += getRealDuration(newEls.at(i));
		}
	}
	// print("Cutting Score and Inserting!\nTime to Eat: ");
	// print(timeToEat);
	// print("\n");
	
	// Seek to the existing element
	auto it = voice->begin();
	for ( ; it != voice->end() && (*it) != existing; it.rightShift());
	if (it == voice->end()) return OpResult::failure;
	
	// print("Got to desired start element\n");
	
	// Basic setup for later
	rational zeroR = rational(0, 1);
	lastDurFound = zeroR;
	rational implicitDur = ARNote::getImplicitDuration();
	rational currentDur = getRealDuration(*it);
	
	// Eat full elements until we don't need to any more
	while (timeToEat >= currentDur) {
		// print("Eating Element\n - Time To Eat: ");
		// print(timeToEat);
		// print("\n - Element Dur: ");
		// print(currentDur);
		// print("\n");
		// If this note doesn't have a duration, default to the last one we saw.
		// If it does, save that duration for when we see one that doesn't.
		if (((*it)->getName() == "chord" && currentDur == zeroR) || currentDur == implicitDur) {
			currentDur = lastDurFound;
		} else if (currentDur == zeroR) {
			// Found a tag. Delete it and move on
			it = voice->erase(it);
			continue;
		} else {
			lastDurFound = currentDur;
		}
		// print(" - Adjusted dur: ");
		// print(currentDur);
		// print("\n");
		
		// Eat the element and subtract its duration from timeToEat
		it = voice->erase(it);
		timeToEat = (timeToEat - currentDur).rationalise();
		
		// Do some flow control and setup for next loop iteration
		if (timeToEat == zeroR) break;
		if (it == voice->end()) return OpResult::needsMeasureAdded;
		currentDur = getRealDuration(*it);
	}
	// print("Done eating elements..\n");
	
	// If we want to eat just part of this last element
	if (timeToEat > zeroR) {
		// print("Wait, just part of this last one\n");
		// Cut this element's duration by the amount that we still want to eat
		ARChord* isChord = dynamic_cast<ARChord*>(&**it);
		ARNote* isNote = dynamic_cast<ARNote*>((&**it));
		if (isChord != nullptr) {
			breakChordTo(fResultVoice, isChord, timeToEat);
		} else if (isNote != nullptr) {
			breakNoteTo(fResultVoice, isNote, timeToEat);
		}
		// print("Done eating part of that last one\n");
	}
	
	// print("Inserting elements now\n");
	// Now actually insert the new elements that we wanted to insert
	if (it == voice->end()) {
		// print("It IS voice end\n");
		// If this wasn't reversed, the notes would appear reversed
		for (int i = int(newEls.size())-1; i > -1; i--) {
			voice->push(newEls.at(i));
		}
	} else {
		// print("It ISN'T voice end\n");
		for (int i = 0; i < newEls.size(); i++) {
			it = voice->insert(it, newEls.at(i));
			it.rightShift();
		}
	}
	// print("Done inserting elements");
	
	// Finish the method off with a successful return
	return OpResult::success;
}

/*! \brief See \c cutScoreAndInsertNote(voice, existing, newEls, insertListDur). Calls that method
		   with a blank list duration, allowing that method to find it for itself.
	\param voice The voice to add the elements to (only works with one voice at a time)
	\param existing The first element in the existing voice to replace
	\param newEls The list of elements to add to the voice
*/
OpResult elementoperationvisitor::cutScoreAndInsert(SARVoice& voice, Sguidoelement existing, std::vector<Sguidoelement> newEls) {
	return cutScoreAndInsert(voice, existing, newEls, rational(-1, 1));
}

// Small casting helper
static ARNote* castToNote(guido::treeIterator<Sguidoelement> input) {
	return (ARNote*)((guidoelement*)*input);
}


static bool deleteNoteFromChord(SARVoice voice, SARChord parent, SARNote child) {
	rational childDur = getRealDuration(child);
	rational implicit = ARNote::getImplicitDuration();
	// If the chord only has two notes right now, we want to get rid of the chord and leave just the
	// note that we AREN'T deleting left in it's place.
	if (parent->size() == 2) {
		// Find other note
		ARNote* noteFound;
		for (auto it = parent->begin(); it != parent->end(); it++) {
			if (child != (*it)) {
				noteFound = castToNote(it);
				break;
			}
		}
		// Quick check: If the remaining note is implicitly relying on the one we are
		// 				deleting, transfer that duration over
		if (getRealDuration(noteFound) == implicit) {
			*noteFound = childDur;
		}
		// Do the actual chord deletion, leaving just the note that wasn't the "child" parameter
		replaceElementWith(voice, parent, noteFound);
		return true;
	} else {
		// Otherwise, just delete the note from the chord directly
		for (auto it = parent->begin(); it != parent->end(); ) {
			if (child == (*it)) {
				it = parent->erase(it);
			} else {
				// And set all other notes' durations to match this one, if they are implicitly relying on it
				ARNote* casted = castToNote(it);
				if (getRealDuration(casted) == implicit) {
					*casted = childDur;
				}
				it++;
			}
		}
		return true;
	}
}
// TESTED
static bool deleteNoteFromVoice(SARVoice parent, SARNote child, bool doReplace) {
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (child == (*it)) {
			it = parent->erase(it);
			// If the next note was relying on this one, set its duration explicitly
			// UNTESTED
			rational dur; int dots;
			if ((*it)->totalduration(dur, dots) == ARNote::getImplicitDuration()) {
				rational newDur = child->duration();
				(*it)->totalduration(newDur, dots);
			}
			// END UNTESTED
			if (doReplace) {
				SARNote rest = ARFactory().createNote("_");
				*rest = child->duration();
				rest->SetDots(child->GetDots());
				it = parent->insert(it, rest);
			}
			
			break;
		}
	}
	return true;
}

static bool deleteElementFrom(Sguidoelement parent, Sguidoelement child) {
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (child == (*it)) {
			it = parent->erase(it);
			return true;
		}
	}
	return false;
}

static bool deleteChordFromVoice(SARVoice parent, SARChord child, bool doReplace) {
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (child == (*it)) {
			it = parent->erase(it);
			if (doReplace) {
				SARNote rest = ARFactory().createNote("_");
				*rest = getRealDuration(child);
				parent->insert(it, rest);
			}
			break;
		}
	}
	return true;
}

static bool insertNoteOnRest(SARVoice parent, SARNote rest, SARNote newNote) {
	// This method assumes that the rest is a direct child of the voice, and not within a chord (since it's a rest)
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (rest == (*it)) {
			it = parent->erase(it);
			parent->insert(it, newNote);
			return true;
		}
	}
	return false;
}

static bool insertNoteIntoChord(SARChord parent, SARNote newNote) {
	// Could do some kind of verification that the note conforms to the rest of the chord here.
	// Things could be like:
	//	- Matches Dots status
	//	- Note Length matches rest of chord
	
	// Make sure that the chord doesn't already have a note with the same pitch in it
	for (auto it = parent->begin(); it != parent->end(); it++) {
		ARNote* child = castToNote(it);
		if (child->getName() == newNote->getName() && child->GetOctave() == newNote->GetOctave()) {
			return false;
		}
	}
	
	// Add the note in.
	parent->insert(parent->begin(), newNote);
	return true;
}

static bool insertNoteToCreateChord(SARVoice parent, SARNote existingNote, SARNote newNote) {
	// Create a blank chord
	SARChord newChord = ARFactory().createChord();
	
	// Try to loop and replace the note with the chord
	bool didReplace = false;
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (existingNote == (*it)) {
			it = parent->erase(it);
			parent->insert(it, newChord);
			didReplace = true;
			break;
		}
	}
	if (! didReplace) return false;
	
	// Add both notes into the chord
	newChord->insert(newChord->begin(), existingNote);
	// Reuse this method to make use of checks and such.
	bool success = insertNoteIntoChord(newChord, newNote);
	
	if (! success) {
		// Deal with the fact that we now have a chord with one note in it
		replaceElementWith(parent, newChord, existingNote);
	}
	
	// Return that it was a success
	return true;
}

static bool replaceElementWith(SARVoice parent, Sguidoelement existing, Sguidoelement newEl) {
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (existing == (*it)) {
			it = parent->erase(it);
			parent->insert(it, newEl);
			return true;
		}
	}
	return false;
}

// Returns number of notes added
static int breakNoteTo(SARVoice voice, SARNote el, rational timeToRemove) {
	rational elDur = getRealDuration(el);
	rational newDur = elDur - timeToRemove;
	// If the desired length is the current length, we're done.
	if (timeToRemove <= rational(0, 1)) return 0;
	
	std::vector<int> noDots = std::vector<int>();
	rationals breakout = rational::getBaseRationals(newDur, false, &noDots);
	std::vector<int> dots = std::vector<int>();
	rationals breakoutDotted = rational::getBaseRationals(newDur, true, &dots);
	// If the new duration can be represented as one note, do that.
	if (breakout.size() == 1) { *el = newDur;  el->SetDots(0);  return 1; }
	if (breakoutDotted.size() == 1) { *el = breakoutDotted.at(0);  el->SetDots(1);  return 1; }
	
	// If we hit this point, we need to actually break up the note into separate notes to represent it
	
	// Point list at non-dotted breakout, unless dotted provides a solution with strictly less notes
	rationals* durList = &breakout;
	std::vector<int>* dotList = &noDots;
	if (breakoutDotted.size() < breakout.size()) { durList = &breakoutDotted;  dotList = &dots; }
	// Sanity check: dots and durations have same length
	if (durList->size() != dotList->size()) { printf("Dots and Durs have different lengths!\n");  return 0; }
	
	// Find the element in the voice first (so we can use the iterator as an index)
	guido::treeIterator<guido::Sguidoelement> it;
	for (it = voice->begin(); it != voice->end(); it++) {
		if ((*it) == el) break;
	}
	// If we didn't find it, we can't do anything here
	if (it == voice->end()) {
		printf("Cannot break out note because we cannot find that note in the voice given!\n");
		std::cout.flush();  return 0;
	}
	
	// Remove the original element
	voice->erase(it);
	
	// For each duration in the breakout chosen, create the note to match the original, and insert it
	for (int i = 0; i < durList->size(); i++) {
		// Copy old note, and adjust duration
		SARNote durNote = getCopyOfNote(el);
		*durNote = durList->at(i);
		durNote->SetDots(dotList->at(i));
		// Insert into parent
		voice->insert(it, durNote);
	}
	
	// Return the number of notes added
	return (int)(durList->size());
}

static int breakChordTo(SARVoice voice, SARChord el, rational timeToRemove) {
	rational elDur = getRealDuration(el);
	rational newDur = elDur - timeToRemove;
	// If the desired length is the current length, we're done.
	if (timeToRemove <= rational(0, 1)) return 0;
	
	std::vector<int> noDots = std::vector<int>();
	rationals breakout = rational::getBaseRationals(newDur, false, &noDots);
	std::vector<int> dots = std::vector<int>();
	rationals breakoutDotted = rational::getBaseRationals(newDur, true, &dots);
	// If the new duration can be represented as one chord, do that.
	if (breakout.size() == 1) { *el = newDur;  el->SetDots(0);  return 1; }
	if (breakoutDotted.size() == 1) { *el = breakoutDotted.at(0);  el->SetDots(1);  return 1; }
	
	// If we hit this point, we need to actually break up the chord into separate chords to represent it
	
	// Point list at non-dotted breakout, unless dotted provides a solution with strictly less pieces
	rationals* durList = &breakout;
	std::vector<int>* dotList = &noDots;
	if (breakoutDotted.size() < breakout.size()) { durList = &breakoutDotted;  dotList = &dots; }
	// Sanity check: dots and durations have same length
	if (durList->size() != dotList->size()) { printf("Dots and Durs have different lengths!\n");  return 0; }
	
	// Find the element in the voice first (so we can use the iterator as an index)
	guido::treeIterator<guido::Sguidoelement> it;
	for (it = voice->begin(); it != voice->end(); it++) {
		if ((*it) == el) break;
	}
	// If we didn't find it, we can't do anything here
	if (it == voice->end()) {
		printf("Cannot break out chord because we cannot find that chord in the voice given!\n");
		std::cout.flush();  return 0;
	}
	
	// Remove the original element
	voice->erase(it);
	
	// For each duration in the breakout chosen, create the chord to match the original, and insert it
	for (int i = 0; i < durList->size(); i++) {
		// Copy old chord, and adjust duration
		SARChord durChord = getCopyOfChord(el);
		*durChord = durList->at(i);
		durChord->SetDots(dotList->at(i));
		// Insert into parent
		voice->insert(it, durChord);
	}
	
	// Return the number of chords added
	return (int)(durList->size());
}

// Simple helper to copy notes
static SARNote getCopyOfNote(SARNote el) {
	SARNote newNote = ARFactory().createNote(el->getName());
	*newNote = el->duration();
	newNote->SetDots(el->GetDots());
	newNote->SetAccidental(el->GetAccidental());
	newNote->SetOctave(el->GetOctave());
	return newNote;
}
// Simple helper to copy chords
static SARChord getCopyOfChord(SARChord el) {
	SARChord newChord = ARFactory().createChord();
	// Make a copy of every note, and add to this chord
	for (int i = 0; i < el->notes().size(); i++) {
		newChord->insert(newChord->begin(), getCopyOfNote(el->notes().at(i)));
	}
	return newChord;
}

static bool checkSongDuration(SARVoice voice, rational desiredLength) {
	rational total = rational(0, 1);
	for (auto it = voice->begin(); it != voice->end(); it++) {
		total += getRealDuration(*it);
		if (total >= desiredLength) {
			return true;
		}
	}
	return false;
}

// ---------------------------[ End Action Methods ]------------------------------------


// -----------------------------[ Browse Methods ]--------------------------------------

// Returns whether we were already done by now
bool elementoperationvisitor::done() {
	return currentVoiceDate() > fTargetDate;
}

// Returns whether the given element is the one we are done now because of
bool elementoperationvisitor::done(SARNote& elt) {
	bool midiSatisfied = fTargetMidiPitch == -1;
	if (! midiSatisfied) {
		int octave = elt->GetOctave();
		int midiPitch = elt->midiPitch(octave);
		midiSatisfied = midiPitch == fTargetMidiPitch;
	}
	return midiSatisfied && currentVoiceDate() == fTargetDate;
}

// In addition to this, always specify: fMidiPitch, fTargetDate, fTargetVoice
void elementoperationvisitor::init() {
	fCurrentVoiceNum = 0;
	fInChord = false;
	fCurrentKeySignature = 0;
	fCurrentMeter = "";
	fCurrentInstrument = nullptr;
	fDone = false;
	fResultVoice = nullptr;
	fResultChord = nullptr;
	fResultNote = nullptr;
	fFoundNote = false;
	fFoundChord = false;
	durationvisitor::reset();
}

void elementoperationvisitor::visitStart(SARVoice& elt) {
	// print("Visit Start: Voice\n");
	fCurrentVoiceRef = elt;
	if (fCurrentVoiceNum == fTargetVoice) {
		durationvisitor::visitStart(elt);
	} else {
		fBrowser.stop();
	}
}

void elementoperationvisitor::visitStart(SARChord& elt) {
	durationvisitor::visitStart(elt);
}

void elementoperationvisitor::visitStart(SARNote& elt) {
	// print("Visit Start: Note\n");
	
	if (done()) {
		// This means that a previous note was being looked for.  Stop.
		fBrowser.stop();
		return;
	}
	
	if (! done(elt)) {
		durationvisitor::visitStart(elt);
		return;
	}
	
	// If we're here, that means that we are at the time slot being looked for
	
	// Set the data we need to go and do what we want after the browsing is done.
	fResultVoice = fCurrentVoiceRef;
	fResultChord = fInChord ? fCurrentChordRef : nullptr;
	fResultNote  = elt;
	
	// Call for browsing to stop
	fBrowser.stop();
}

void elementoperationvisitor::visitStart(Sguidotag& tag) {
	// print("Visit Start: Tag\n");
	
	switch (tag->getType()) {
		case kTKey: {
			std::string val = tag->getAttribute(0)->getValue();
			fCurrentKeySignature = stoi(val);
			break;}
		case kTMeter:
			fCurrentMeter = tag->attributes().at(0)->getValue();
			break;
		case kTInstr:
		case kTInstrument:
			fCurrentInstrument = tag;
			break;
	}
}

void elementoperationvisitor::visitEnd(SARVoice& elt) {
	// print("Visit End: Voice\n");
	if (fCurrentVoiceNum == fTargetVoice) {
		durationvisitor::visitEnd(elt);
	}
	fBrowser.stop(false);
	fCurrentVoiceNum++;
}

void elementoperationvisitor::visitEnd(SARChord& elt) {
	// print("Visit End: Chord\n");
	fInChord = false;
	if (done()) { fBrowser.stop(); return; }
	
	durationvisitor::visitEnd(elt);
	
	if (! done()) { return; }
	
	// If we got here, we want to act on this chord
	
	// Set results so the outer method can do it's thing
	if (fTargetMidiPitch < 0) {
		fResultVoice = fCurrentVoiceRef;
		fResultChord = fCurrentChordRef;
		fResultNote  = nullptr;
	}
}

// ---------------------------[ End Browse Methods ]------------------------------------


} // namespace
