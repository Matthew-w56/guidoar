
#include "testInterface.h"

#include <iostream>

#include "ARTypes.h"
#include "guidoparser.h"
#include "guidoelement.h"
#include "elementoperationvisitor.h"

using std::cout;
using std::ostringstream;
using guido::SARMusic;
using guido::guidoparser;
using guido::Sguidoelement;
using guido::rational;
using guido::elementoperationvisitor;

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

/**
 *  Reads in the file referenced as a Guido score, deletes the note in question,
 *  and then returns the text data for the new score.
 */
char* deleteEvent(const char* scoreData, int num, int den, unsigned int voice, int midiPitch) {
	// Read the score.  If that fails, return error code as a string.
	SARMusic score = read(scoreData);
	if (!score) return "Error reading score!  (No score operation performed)";
	
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
