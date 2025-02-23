#include "selectioninterface.h"
#include "selectiondcopinterface.h"
#include "document.h"
#include "view.h"

using namespace KTextEditor;

namespace KTextEditor
{
	class PrivateSelectionInterface
	{
	public:
		PrivateSelectionInterface()
		{
		interface = 0;
		}
		~PrivateSelectionInterface(){}
	// Data Members
	SelectionDCOPInterface *interface;
	};

}

unsigned int SelectionInterface::globalSelectionInterfaceNumber = 0;

SelectionInterface::SelectionInterface()
{
	d = new PrivateSelectionInterface();
	globalSelectionInterfaceNumber++;
        mySelectionInterfaceNumber = globalSelectionInterfaceNumber;
	TQString name = "SelectionInterface#" + TQString::number(mySelectionInterfaceNumber);
	 d->interface = new SelectionDCOPInterface(this, name.latin1());
}
SelectionInterface::~SelectionInterface()
{
  delete d->interface;
  delete d;
}

unsigned int SelectionInterface::selectionInterfaceNumber () const
{
  return mySelectionInterfaceNumber;
}

void SelectionInterface::setSelectionInterfaceDCOPSuffix (const TQCString &suffix)
{
  d->interface->setObjId ("SelectionInterface#"+suffix);
}

SelectionInterface *KTextEditor::selectionInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::SelectionInterface*>(doc);
}

SelectionInterface *KTextEditor::selectionInterface (View *view)
{
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::SelectionInterface*>(view);
}

