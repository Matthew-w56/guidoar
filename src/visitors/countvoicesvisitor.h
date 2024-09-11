
#ifndef __countVoicesVisitor__
#define __countVoicesVisitor__

#include "arexport.h"
#include "guidoelement.h"
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
class gar_export countvoicesvisitor : public visitor<SARVoice>
{
public: 
				 countvoicesvisitor() { fBrowser.set(this); }
		virtual ~countvoicesvisitor() {}
    
    int count (const Sguidoelement& elt) {
		fCountEnter = 0;
		fCountLeave = 0;
        if (elt) fBrowser.browse (*elt);
        if (fCountEnter != fCountLeave) {
			printf("Didn't count same voice count going in and out! (%d, %d)\n", fCountEnter, fCountLeave);
			std::cout.flush();
		}
		return fCountEnter;
    }
    
    virtual void visitStart ( SARVoice& elt )   { fCountEnter++; }
	virtual void visitEnd   ( SARVoice& elt )   { fCountLeave++; }

protected:
	tree_browser<guidoelement> fBrowser;
	int fCountEnter;
	int	fCountLeave;
};

/*! @} */

} // namespace

#endif
