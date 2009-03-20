
#ifdef WIN32
# pragma warning (disable : 4786)
# define basename(name)	(name)
# define _CRT_SECURE_NO_DEPRECATE
#else 
# include <libgen.h>
#endif

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libguidoar.h"

using namespace std;
using namespace guido;

//#define guidodebug

//_______________________________________________________________________________
static void usage(char * name)
{
	cerr << "usage: " << basename(name) << " <score> <evIndex> [voiceIndex]"  << endl;
	cerr << "       converts an event index to a time position"  << endl;
	cerr << "       score: the input file or '-' to read stdin"  << endl;
	cerr << "       evIndex: the index of the target event (1 based)"  << endl;
	cerr << "       voiceIndex: optional voice index (defaults to the first voice)"  << endl;
	exit (1);
}

//_______________________________________________________________________________
static void readErr (const char * file) 
{
	cerr << "failed to read '" << file << "'" << endl;
	exit(1);
}

//_______________________________________________________________________________
static int numArg (const char* vi) 
{
	int num=0;
	int n = sscanf(vi, "%d", &num);
	if (n != 1) num = -1;
	return num;
}

//_______________________________________________________________________________
int main(int argc, char *argv[]) 
{
	char *name = argv[0];
#ifdef guidodebug
	argc = 4;
	char * args[] = {"a.gmn", "2", "1", 0};
	char ** argsPtr = args;
#else
	if ((argc != 3) && (argc != 4)) usage(name);
	char ** argsPtr = &argv[1];
#endif

	const char *file = argsPtr[0];
	char *buff = strcmp(file,"-") ? guidoread(file) : guidoread(stdin);
	if (!buff) readErr(file);

	int eventIndex = numArg (argsPtr[1]);		// get the event index argument
	if (eventIndex <= 0) usage(name);
	
	int voiceIndex = 1;									// default voice is 1
	if (argc == 4) voiceIndex = numArg (argsPtr[2]);	// get the voice index argument
	if (voiceIndex <= 0) usage(name);
		
	cout << file << ": voice " << voiceIndex << " event " << eventIndex
		 << ": time " << string(guidoEv2Time(buff, eventIndex-1, voiceIndex-1)) << endl;
	delete[] buff;
	return 0;
}
