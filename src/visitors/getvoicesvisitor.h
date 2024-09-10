


#ifndef __getVoicesVisitor__
#define __getVoicesVisitor__

#include "arexport.h"
#include "ARTypes.h"
#include "tree_browser.h"
#include "visitor.h"

namespace guido 
{

/*!
\addtogroup visitors
@{
*/

//_______________________________________________________________________________
/*!
\brief  a visitor to count the number of voices in a score
*/
class gar_export getvoicesvisitor : public visitor<SARVoice>
{
    public: 
				 getvoicesvisitor() { fBrowser.set(this); }
		virtual ~getvoicesvisitor() {}
    
    std::vector<SARVoice> operator() (const Sguidoelement& elt) {
		fVoices.clear();
		fBrowser.browse(*elt);
		return fVoices;
    }
    
    virtual void visitStart ( SARVoice& elt )   { fVoices.push_back(elt); }

	protected:
		tree_browser<guidoelement> fBrowser;
		std::vector<SARVoice> fVoices;
};

/*! @} */

} // namespace

#endif
