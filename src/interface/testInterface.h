

#ifndef TESTINTERFACE_H
#define TESTINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "arexport.h"

gar_export char* deleteEvent(const char* scoreData, int num, int den, unsigned int voice, int midiPitch);
gar_export char* insertNote(const char* scoreData, int startNum, int startDen, int durNum, int durDen, int midiPitch, int voice, int dots, int insistedAccidental);
gar_export char* setDurationAndDots(const char* scoreData, int elStartNum, int elStartDen, int voice, int newDurNum, int newDurDen, int newDots);
gar_export char* setAccidental(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int newAccidental, int* resultPitch);
gar_export char* setNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int oldPitch, int newPitch);
gar_export char* shiftNotePitch(const char* scoreData, int elStartNum, int elStartDen, int voice, int midiPitch, int pitchShiftDirection, int* resultPitch);

#ifdef __cplusplus
}
#endif

#endif