/*
	this file is automatically generated from template.cxx
	don't modify unles you're ready to loose your changes
*/

#include <iostream>

#include "common.cxx"

#include "desc/guidoevtailn.h"

//_______________________________________________________________________________
static void usage(char * name)
{
	cerr << "usage: " << basename(name) << " score  " << gArg  << endl;
	cerr << "       " << gDesc << endl;
	cerr << "       " << scoredesc << endl;
	cerr << "       " << gArg << ": " << gArgDesc  << endl;
	exit (-1);
}

//_______________________________________________________________________________
template <typename T> class operation
{
	typedef garErr (*TOperator)(const char*, T, ostream&);
	TOperator	fOperator;
	string		fScore;			// the score argument
	string		fScoreArg;		// to store the second score argument
	T			fArg;
	string		fSdtIn;			// intended to share stdin
	

	bool readArg(const char* arg, int& value) const			{ return intVal (arg, value); }
	bool readArg(const char* arg, float& value) const		{ return floatVal (arg, value); }
	bool readArg(const char* arg, rational& value) const	{ return rationalVal (arg, value); }
	bool readArg(const char* arg, string& value)			{ return gmnVal (arg, value, fSdtIn); }
	bool readArg(const char* arg, const char*& value)		{
			if (gmnVal (arg, fScoreArg, fSdtIn)) {
				value = fScoreArg.c_str();
				return true;
			} 
			return false; 
		}

	public :
				 operation(TOperator op) : fOperator (op) {}
		virtual ~operation() {}
		
		bool init (int argc, char* argv[]) {
			if (argc != 3) return false;
			if (!readArg (argv[1], fScore)) return false;
			if (!readArg (argv[2], fArg)) return false;
			return true;
		}
		garErr  run (ostream& out)		{ return fOperator (fScore.c_str(), fArg, out); }
};


//_______________________________________________________________________________
int main (int argc, char* argv[])
{
	operation<int> op (guidoVETail);
	if (op.init(argc, argv)) {
		garErr err = op.run (cout);
		if (err != kNoErr) {
			error (err);
			return err;
		}
	}
	else usage (argv[0]);
	return 0;
}

