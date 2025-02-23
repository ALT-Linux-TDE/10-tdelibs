#ifndef SEARCH_DCOP_INTERFACE_H
#define SEARCH_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <tqstringlist.h>
#include <tqcstring.h>

namespace KTextEditor
{
	class SearchInterface;
	/**
	This is the main interface to the SearchInterface of KTextEdit.
	This will provide a consistant dcop interface to all KDE applications that use it.
	@short DCOP interface to SearchInterface.
	@author Ian Reinhart Geiser <geiseri@kde.org>
	*/
	class KTEXTEDITOR_EXPORT SearchDCOPInterface : virtual public DCOPObject
	{
	K_DCOP

	public:
		/**
		Construct a new interface object for the text editor.
		@param Parent the parent SearchInterface object
		that will provide us with the functions for the interface.
		@param name the TQObject's name
		*/
		SearchDCOPInterface( SearchInterface *Parent, const char *name );
		/**
		Destructor
		Cleans up the object.
		**/
		virtual ~SearchDCOPInterface();
	k_dcop:
		bool findFirstString(TQString text, bool caseSensitive);
		bool findNextString(TQString text, bool caseSensitive);
		bool findPreviousString( TQString text, bool caseSensitive);
		bool findLastString(TQString text, bool caseSensitive);
		bool findStringAt( uint  row, uint  col, TQString text, bool caseSensitive);

		bool findFirstRegExp( TQString regexp);
		bool findNextRegExp( TQString regexp);
		bool findPreviousRegExp( TQString regexp);
		bool findLastRegExp( TQString regexp);
		bool findRegExpAt( uint  row, uint  col, TQString regexp);

		uint currentMatchLine();
		uint currentMatchCol();
		uint currentMatchLength();

	private:
		SearchInterface *m_parent;
		uint  m_currentcol;
		uint  m_currentrow;
		uint  m_currentmatchlen;
	};
}
#endif
