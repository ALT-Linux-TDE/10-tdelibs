#ifndef Encoding_DCOP_INTERFACE_H
#define Encoding_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <tqstringlist.h>
#include <tqcstring.h>
//#include "editdcopinterface.moc"
namespace KTextEditor
{
	class EncodingInterface;
	/**
	This is the main interface to the EncodingInterface of KTextEdit.
	This will provide a consistant dcop interface to all KDE applications that use it.
	@short DCOP interface to EncodingInterface.
	@author Ian Reinhart Geiser <geiseri@kde.org>
	*/
	class KTEXTEDITOR_EXPORT EncodingDCOPInterface : virtual public DCOPObject
	{
	K_DCOP

	public:
		/**
		Construct a new interface object for the text editor.
		@param Parent the parent EncodingInterface object
		that will provide us with the functions for the interface.
		@param name the TQObject's name
		*/
		EncodingDCOPInterface( EncodingInterface *Parent, const char *name );
		/**
		Destructor
		Cleans up the object.
		*/
		virtual ~EncodingDCOPInterface();
	k_dcop:
		uint encodingInterfaceNumber ();
		void setEncoding (TQString e) ;
		TQString encoding();
   
	private:
		EncodingInterface *m_parent;
	};
}
#endif


