
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

bool elementoperationvisitor::deleteEvent(const Sguidoelement& score, const rational& time, unsigned int voiceIndex, int pitch, int octave) {
	print("Starting to run deleteEvent..\n");
	fTargetVoice = voiceIndex;
	fTargetDate = time;
	fPitchRef = pitch;
	fOctaveRef = octave;
	fOpIntent = DeleteEvent;
	print("All set up for deleteEvent!\n");
	init();
	print("Done with init for deleteEvent!  About to start browsing..\n");
	fBrowser.browse(*score);
	print("Done browsing!\n");
	
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
	print("Starting DeleteNoteFromChord..\n");
	for (auto it = parent->begin(); it != parent->end(); it++) {
		if (child == (*it)) {
			print("Found Element to delete.\n");
			it = parent->erase(it);
			print("Deleted element\n");
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
// TODO: THIS METHOD IS UNTESTED
static bool deleteChordFromVoice(SARVoice parent, SARChord child, bool doReplace) {
	print("Starting to delete chord from voice.\n");
	for (auto it = parent->begin(); it != parent->end(); it++) {
		print("Not this child..\n");
		if (child == (*it)) {
			print("Here we are!  This child is the chord.\n");
			it = parent->erase(it);
			print("Did erase.\n");
			if (doReplace) {
				print("We want to replace, I guess.\n");
				SARNote rest = ARFactory().createNote("_");
				rest->setImplicitDuration();
				*rest = child->duration();
				parent->insert(it, rest);
				print("Inserted note.\n");
			}
			break;
		}
	}
	print("Success for Chord From Voice!\n");
	return true;
}

// ---------------------------[ End Action Methods ]------------------------------------


// -----------------------------[ Browse Methods ]--------------------------------------

bool elementoperationvisitor::done() {
	// print("Are we done?\n");
	fDone = currentVoiceDate() > fTargetDate;
	//print("Checking if done: ");
	//print(fDone);
	//print("\n");
	// print("Done checking\n");
	return fDone;
}

bool elementoperationvisitor::wouldBeDone(SARNote& elt) {
	// print("Checking if w-done: ");
	// print((currentVoiceDate() + elt->duration()) > fTargetDate);
	// print("\n");
	bool temp = (currentVoiceDate() + elt->duration()) > fTargetDate;
	// print(temp);
	// print(" is the answer\n");
	return temp;
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
	print("Visit Start: Voice ");
	print(fCurrentVoice);
	print("\n");
	fCurrentVoiceRef = elt;
	if (fCurrentVoice == fTargetVoice) {
		durationvisitor::visitStart(elt);
	} else {
		fBrowser.stop();
	}
	print("Visit Start Done Voice\n");
}

void elementoperationvisitor::visitStart(SARNote& elt) {
	print("Visit Start: Note\n");
	if (done()) {
		// This means that a previous note was being looked for.  Stop.
		print("Already done!  Stopping this note.\n");
		fBrowser.stop();
		return;
	}
	durationvisitor::visitStart(elt);
	if (! done()) {
		// This means that we are not being looked for (that will be later).
		print("Not this note. Continuing.\n");
		return;
	}
	
	// If we're here, that means that we are the note being looked for!
	
	// Set the things needed for the outer method to do it's thing
	if (fOpIntent == DeleteEvent) {
		if (fInChord) {
			if (fPitchRef < 0 && fOctaveRef < 0) {
				// Delete chord from voice
				print("Signaling that we should delete chord from voice\n");
				fResultVoice = fCurrentVoiceRef;
				fResultChord = fCurrentChordRef;
				fResultNote  = nullptr;
			} else {
				// Delete note from chord
				print("Signaling that we should delete note from chord\n");
				fResultVoice = nullptr;
				fResultChord = fCurrentChordRef;
				fResultNote  = elt;
			}
		} else {
			// Delete note from voice
			print("Signaling that we should delete note from voice\n");
			fResultVoice = fCurrentVoiceRef;
			fResultChord = nullptr;
			fResultNote  = elt;
		}
	}
	
	/*
	
	// If we want to delete something
	if (fOpIntent == DeleteEvent) {
		print("So we want to delete..\n");
		// We are in a chord
		if (fInChord) {
			print("But we're in a chord!\n");
			int octave = elt->GetOctave();
			int zero = 0;
			int pitch = elt->GetPitch(zero);
			print("Octave is "); print(octave);
			print(" and pitch is "); print(pitch); print("\n");
			// If we want to delete the whole chord, not just a note from it
			if (fPitchRef < 0 && fOctaveRef < 0) {
				print("We want to delete the whole chord, I guess!\n");
				deleteChordFromVoice(fCurrentVoiceRef, fCurrentChordRef, DO_REPLACE);
				print("Done deleting chord from voice\n");
				fBrowser.stop();
			}
			// If we want to delete a note, not the whole chord (and this is that note specifically)
			if (fPitchRef >= 0 && fOctaveRef >= 0 
					&& octave == fOctaveRef && pitch == fPitchRef) {
				print("We want to delete a note from the chord!\n");
				deleteNoteFromChord(fCurrentChordRef, elt);
				print("Done deleting note from chord\n");
				fBrowser.stop();
			}
		// If we are still wanting to delete something, but we're not in a chord
		} else {
			print("Going to delete note from voice\n");
			// This note should be removed from voice
			deleteNoteFromVoice(fCurrentVoiceRef, elt, DO_REPLACE);
			print("Done deleting note from voice\n");
			fBrowser.stop();
		}
	}
	*/
	print("Visit Start Done Note\n");
}

void elementoperationvisitor::visitStart(SARChord& elt) {
	print("Visit Start: Chord\n");
	fInChord = true;
	fCurrentChordRef = elt;
	if (done()) fBrowser.stop();
	else {
		durationvisitor::visitStart(elt);
	}
	print("Visit Start Done Chord\n");
}

void elementoperationvisitor::visitEnd(SARVoice& elt) {
	print("Visit End: Voice\n");
	if (fCurrentVoice == fTargetVoice) {
		durationvisitor::visitEnd(elt);
	}
	fBrowser.stop(false);
	fCurrentVoice++;
}

void elementoperationvisitor::visitEnd(SARChord& elt) {
	print("Visit End: Chord\n");
	fInChord = false;
	if (done()) { fBrowser.stop(); return; }
	
	durationvisitor::visitEnd(elt);
	if (! done()) { return; }
	
	// If we got here, we want to act on this chord
	print("Some action is warranted on this chord..\n");
	
	// Set results so the outer method can do it's thing
	if (fOpIntent == DeleteEvent && fPitchRef < 0 && fOctaveRef < 0) {
		// If we want to delete this chord:
		print("Visit end chord: We want to delete this chord!");
		fResultVoice = fCurrentVoiceRef;
		fResultChord = fCurrentChordRef;
		fResultNote  = nullptr;
	} else {
		print("No action needed in visitEnd: Chord\n");
	}
	
	/*
	// If we want to delete it, and the other parameters are set for deleting chords and not notes
	if (fOpIntent == DeleteEvent) {
		if (fPitchRef < 0 && fOctaveRef < 0) {
			print("Going to try to delete chord from voice\n");
			deleteChordFromVoice(fCurrentVoiceRef, elt, DO_REPLACE);
			print("Deleted chord from voice!\n");
			print("Stage 1\n");
		}
		print("Stage 2\n");
	}
	print("Stage 3\n");*/
	print("Visit End done: Chord\n");
}

// ---------------------------[ End Browse Methods ]------------------------------------


} // namespace
