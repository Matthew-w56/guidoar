/*
  Copyright � Grame 2008

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Grame Research Laboratory, 9, rue du Garet 69001 Lyon - France
    research@grame.fr

*/

#ifndef __removeVoiceOperation__
#define __removeVoiceOperation__

#include "arexport.h"
#include "AROthers.h"
#include "clonevisitor.h"
#include "operation.h"
#include "tree_browser.h"


namespace guido 
{

/*!
\addtogroup operations
@{
*/

/*!
\brief A visitor that cuts the head of a score voices.
*/
class gar_export removeVoiceOperation :
	public operation,
	public clonevisitor
{		
    public:
 				 removeVoiceOperation()	{ fBrowser.set(this); }
		virtual ~removeVoiceOperation()	{}

		/*! cuts the head of the score voices before a given voice
			\param score the score to be cut
			\param voicenum the score voice to drop
			\return a new score
		*/
		Sguidoelement operator() ( const Sguidoelement& score, int voicenum ) {
			fVoiceNum = voicenum;
			fCurrentVoice = 0;
			Sguidoelement outscore;
			if (score) {
				fBrowser.stop(false);
				fBrowser.browse (*score);
				outscore = fStack.top();
				fStack.pop();
			}
			return outscore;
		}

		/*! Does Nothing!  Implemented to fit definition of operation.
		*/
		SARMusic operator() ( const SARMusic& score1, const SARMusic& score2 ) {
			return score1; // No-op.
		}
 
     protected:
		int	fVoiceNum, fCurrentVoice;
		tree_browser<guidoelement> fBrowser;

		virtual bool copy  () {
			return fCurrentVoice != fVoiceNum;
		}
		virtual void visitStart ( SARVoice& elt ) {
			fCurrentVoice++;
			if (copy())
				clonevisitor::visitStart (elt);
			else fBrowser.stop();
		}
		
		virtual void visitEnd   ( SARVoice& elt ) {
			fBrowser.stop(false);
			if (copy())
				clonevisitor::visitEnd (elt);
		}
};

/*! @} */

} // namespace MusicXML


#endif
