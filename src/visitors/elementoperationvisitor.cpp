
#include <iostream>

#include "AROthers.h"
//#include "ARFactory.h"
#include "elementoperationvisitor.h"
#include "guidoelement.h"

namespace guido
{

// Determines whether we replace notes with equal rests or not
static bool DO_REPLACE = true;
// Forward declare methods so the public methods don't complain
static bool deleteNoteFromChord(SARChord parent, SARNote child);
static bool deleteChordFromVoice(SARVoice parent, SARChord child, bool doReplace);
static bool deleteNoteFromVoice(SARVoice parent, SARNote child, bool doReplace);

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


// ---------------------------[ Public Methods ]---------------------------------

bool elementoperationvisitor::deleteEvent(const Sguidoelement& score, const rational& time, unsigned int voiceIndex, int midiPitch) {
	fTargetVoice = voiceIndex;
	fTargetDate = time;
	fMidiPitch = midiPitch;
	fOpIntent = DeleteEvent;
	init();
	fBrowser.browse(*score);
	
	// Here we can assume we have our fResult objects
	bool noChord = fResultChord == nullptr;
	bool noNote = fResultNote == nullptr;
	// Decide what action to take
	if (noChord && ! noNote) {
		// We want to delete a note that's NOT in a chord
		deleteNoteFromVoice(fResultVoice, fResultNote, DO_REPLACE);
	} else if (! noChord && noNote) {
		// We want to delete an entire chord
		deleteChordFromVoice(fResultVoice, fResultChord, DO_REPLACE);
	} else if (! noChord && ! noNote) {
		// We want to delete a note that's IN a chord
		deleteNoteFromChord(fResultChord, fResultNote);
	} else {
		// Both are null, nothing was found.
		print("Nothing found to delete at that point!  No action taken..\n");
		return false;
	}
	
	return true;
}

bool elementoperationvisitor::insertNote(const Sguidoelement& score, const rational& time, SARNote& elt, unsigned int voiceIndex) {
	print("Insert?  Shouldn't happen..\n");
	return true;
}

// ---------------------------[ End Public Methods ]------------------------------------


// -----------------------------[ Action Methods ]--------------------------------------

// TODO: THIS METHOD IS UNTESTED
static bool deleteNoteFromChord(SARChord parent, SARNote child) {
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (child == (*it)) {
			it = parent->erase(it);
			break;
		}
	}
	return true;
}
// TESTED
static bool deleteNoteFromVoice(SARVoice parent, SARNote child, bool doReplace) {
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (child == (*it)) {
			it = parent->erase(it);
			if (doReplace) {
				SARNote rest = ARFactory().createNote("_");
				rest->setImplicitDuration();
				*rest = child->duration();
				rest->SetDots(child->GetDots());
				parent->insert(it, rest);
			}
			break;
		}
	}
	return true;
}
// TESTED
static bool deleteChordFromVoice(SARVoice parent, SARChord child, bool doReplace) {
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (child == (*it)) {
			it = parent->erase(it);
			if (doReplace) {
				SARNote rest = ARFactory().createNote("_");
				rest->setImplicitDuration();
				*rest = child->duration();
				parent->insert(it, rest);
			}
			break;
		}
	}
	return true;
}

// ---------------------------[ End Action Methods ]------------------------------------


// -----------------------------[ Browse Methods ]--------------------------------------

bool elementoperationvisitor::done() {
	return currentVoiceDate() > fTargetDate;
}

bool elementoperationvisitor::done(SARNote& elt) {
	int octave = elt->GetOctave();
	int midiPitch = elt->midiPitch(octave);
	return currentVoiceDate() == fTargetDate
			&& midiPitch == fMidiPitch;
}

void elementoperationvisitor::init() {
	fCurrentVoice = 0;
	fDone = false;
	fResultVoice = nullptr;
	fResultChord = nullptr;
	fResultNote = nullptr;
	durationvisitor::reset();
}

void elementoperationvisitor::visitStart(SARVoice& elt) {
	// print("Visit Start: Voice\n");
	fCurrentVoiceRef = elt;
	if (fCurrentVoice == fTargetVoice) {
		durationvisitor::visitStart(elt);
	} else {
		fBrowser.stop();
	}
}

void elementoperationvisitor::visitStart(SARChord& elt) {
	// print("Visit Start: Chord\n");
	fInChord = true;
	fCurrentChordRef = elt;
	if (done()) fBrowser.stop();
	else {
		durationvisitor::visitStart(elt);
	}
}

void elementoperationvisitor::visitStart(SARNote& elt) {
	// print("Visit Start: Note\n");
	
	if (done()) {
		// This means that a previous note was being looked for.  Stop.
		fBrowser.stop();
		return;
	}
	
	durationvisitor::visitStart(elt);
	
	if (! done(elt)) {
		durationvisitor::visitStart(elt);
		return;
	}
	
	// If we're here, that means that we are at the time slot being looked for
	
	// Set the things needed for the outer method to do it's thing
	if (fOpIntent == DeleteEvent) {
		// Case to delete chord from voice taken care of in VisitEnd(Chord)
		if (fInChord) {
			// Delete note from chord
			fResultVoice = nullptr;
			fResultChord = fCurrentChordRef;
			fResultNote  = elt;
		} else {
			// Delete note from voice
			fResultVoice = fCurrentVoiceRef;
			fResultChord = nullptr;
			fResultNote  = elt;
		}
	}
	fBrowser.stop();
}

void elementoperationvisitor::visitEnd(SARVoice& elt) {
	// print("Visit End: Voice\n");
	if (fCurrentVoice == fTargetVoice) {
		durationvisitor::visitEnd(elt);
	}
	fBrowser.stop(false);
	fCurrentVoice++;
}

void elementoperationvisitor::visitEnd(SARChord& elt) {
	// print("Visit End: Chord\n");
	fInChord = false;
	if (done()) { fBrowser.stop(); return; }
	
	durationvisitor::visitEnd(elt);
	
	if (! done()) { return; }
	
	// If we got here, we want to act on this chord
	
	// Set results so the outer method can do it's thing
	if (fOpIntent == DeleteEvent && fMidiPitch < 0) {
		// If we want to delete this chord:
		fResultVoice = fCurrentVoiceRef;
		fResultChord = fCurrentChordRef;
		fResultNote  = nullptr;
	}
}

// ---------------------------[ End Browse Methods ]------------------------------------


} // namespace
