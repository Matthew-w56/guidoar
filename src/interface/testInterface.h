

#ifndef TESTINTERFACE_H
#define TESTINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "arexport.h"

gar_export char* deleteEvent(const char* scoreData, int num, int den, unsigned int voice, int midiPitch);
gar_export char* deleteRange(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice);
gar_export char* insertNote(const char* scoreData, int startNum, int startDen, int durNum, int durDen, int midiPitch, int voice, int dots, int insistedAccidental);
gar_export char* setDurationAndDots(const char* scoreData, int elStartNum, int elStartDen, int voice, int newDurNum, int newDurDen, int newDots);
gar_export char* setAccidental(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int newAccidental, int* resultPitch);
gar_export char* setNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int oldPitch, int newPitch);
gar_export char* shiftNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int pitchShiftDirection, int* resultPitch);
gar_export char* shiftRangeNotePitch(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice, int pitchShiftDirection);

gar_export char* getSelection(const char* scoreData, int startNum, int startDen, int endNum, int endDen, int startVoice, int endVoice);
gar_export char* pasteToDuration(const char* scoreData, const char* selectionData, int startNum, int startDen, int startVoice);
gar_export char* testMethod(const char* scoreData);

#ifdef __cplusplus
}
#endif

#endif