

#ifndef TESTINTERFACE_H
#define TESTINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "arexport.h"

gar_export char* deleteNoteFrom(const char* scoreData, int num, int den, unsigned int voice);

#ifdef __cplusplus
}
#endif

#endif