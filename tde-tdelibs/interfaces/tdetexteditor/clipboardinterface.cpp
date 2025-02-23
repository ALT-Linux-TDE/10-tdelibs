#include "clipboardinterface.h"
#include "clipboarddcopinterface.h"

#include "view.h"

using namespace KTextEditor;

namespace KTextEditor
{
	class PrivateClipboardInterface
	{
	public:
		PrivateClipboardInterface()
		{
		interface = 0;
		}
		~PrivateClipboardInterface(){}
	// Data Members
	ClipboardDCOPInterface *interface;
	};

}

unsigned int ClipboardInterface::globalClipboardInterfaceNumber = 0;

ClipboardInterface::ClipboardInterface()
{
	d = new PrivateClipboardInterface();
	globalClipboardInterfaceNumber++;
        myClipboardInterfaceNumber = globalClipboardInterfaceNumber++;
	TQString name = "ClipboardInterface#" + TQString::number(myClipboardInterfaceNumber);
	d->interface = new ClipboardDCOPInterface(this, name.latin1());
}
ClipboardInterface::~ClipboardInterface()
{
  delete d->interface;
  delete d;
}

unsigned int ClipboardInterface::clipboardInterfaceNumber () const
{
  return myClipboardInterfaceNumber;
}

void ClipboardInterface::setClipboardInterfaceDCOPSuffix (const TQCString &suffix)
{
  d->interface->setObjId ("ClipboardInterface#"+suffix);
}

ClipboardInterface *KTextEditor::clipboardInterface (View *view)
{                                    
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::ClipboardInterface*>(view);
}
