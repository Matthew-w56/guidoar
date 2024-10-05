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

#ifndef __additionalVoiceOperation__
#define __additionalVoiceOperation__

#include "arexport.h"
#include "AROthers.h"
#include "clonevisitor.h"
#include "durationvisitor.h"
#include "operation.h"
#include "tree_browser.h"


namespace guido 
{

enum SeekMode {
	copyOverVoice, referencingVoice, delayReferencingVoice
};

/*!
\addtogroup operations
@{
*/

/*!
\brief A visitor that cuts the head of a score voices.
*/
class gar_export additionalVoiceOperation :
	public operation,
	public clonevisitor,
	public durationvisitor
{
    public:
 				 additionalVoiceOperation()	{ fBrowser.set(this); }
		virtual ~additionalVoiceOperation()	{}

		/*! Inserts another voice into the song, with matching characteristics
			to the closest existing voice
			\param score the score to add a voice to
			\param insertIndex the desired index of the new voice
			\return a new score
		*/
		Sguidoelement operator() ( const Sguidoelement& score, int insertIndex ) {
			fSeekMode = SeekMode::copyOverVoice;
			fTargetVoice = insertIndex;
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
		int	fTargetVoice, fCurrentVoice;
		tree_browser<guidoelement> fBrowser;
		SeekMode fSeekMode;

		virtual void visitStart ( SARVoice& elt ) {
			durationvisitor::visitStart(elt);
			if (fCurrentVoice == 0 && fTargetVoice == 0) {
				// TODO: Special case here where we wait to clone both until visitEnd(Voice)
			}
			fCurrentVoice++;
			if (fCurrentVoice == fTargetVoice) {
				fSeekMode = SeekMode::referencingVoice;
				
			}
			
			if (copy())
				clonevisitor::visitStart (elt);
			else fBrowser.stop();
		}
		
		virtual void visitEnd   ( SARVoice& elt ) {
			durationvisitor::visitEnd(elt);
			fBrowser.stop(false);
			if (copy())
				clonevisitor::visitEnd (elt);
		}
		
		virtual void visitStart ( Sguidotag& tag ) {
			
		}
		
		virtual void visitEnd   ( Sguidotag& tag ) {
			
		}
};

/*! @} */

} // namespace MusicXML


#endif
