#ifndef ViewStatusMsg_DCOP_INTERFACE_H
#define ViewStatusMsg_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <tqstringlist.h>
#include <tqcstring.h>
//#include "editdcopinterface.moc"
namespace KTextEditor
{
	class ViewStatusMsgInterface;
	/**
	This is the main interface to the ViewStatusMsgInterface of KTextEdit.
	This will provide a consistant dcop interface to all KDE applications that use it.
	@short DCOP interface to ViewStatusMsgInterface.
	@author Ian Reinhart Geiser <geiseri@kde.org>
	*/
	class KTEXTEDITOR_EXPORT ViewStatusMsgDCOPInterface : virtual public DCOPObject
	{
	K_DCOP

	public:
		/**
		Construct a new interface object for the text editor.
		@param Parent the parent ViewStatusMsgInterface object
		that will provide us with the functions for the interface.
		@param name the TQObject's name
		*/
		ViewStatusMsgDCOPInterface( ViewStatusMsgInterface *Parent, const char *name );
		/**
		Destructor
		Cleans up the object.
		*/
		virtual ~ViewStatusMsgDCOPInterface();
	k_dcop:
		uint viewStatusMsgInterfaceNumber ();
		void viewStatusMsg (class TQString msg) ;
	
	private:
		ViewStatusMsgInterface *m_parent;
	};
}
#endif


