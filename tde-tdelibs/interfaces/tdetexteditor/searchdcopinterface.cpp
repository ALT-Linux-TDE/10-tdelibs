#include "searchdcopinterface.h"
#include "searchinterface.h"

#include <dcopclient.h>
#include <tqregexp.h>

using namespace KTextEditor;

SearchDCOPInterface::SearchDCOPInterface( SearchInterface *Parent, const char *name)
	: DCOPObject(name)
{
	m_parent = Parent;
	m_currentcol = 0;
	m_currentrow = 0;
	m_currentmatchlen = 0;
}

SearchDCOPInterface::~SearchDCOPInterface()
{

}

bool SearchDCOPInterface::findFirstString( TQString text, bool caseSensitive)
{
	return m_parent->searchText(0, 0, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive);
}
bool SearchDCOPInterface::findNextString( TQString text, bool caseSensitive)
{
	return m_parent->searchText(m_currentrow, m_currentcol+1, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive);
}

bool SearchDCOPInterface::findPreviousString( TQString text, bool caseSensitive)
{
	if( m_currentcol == 0)
		m_currentrow--;
	else
		m_currentcol--;
	return m_parent->searchText(m_currentrow, m_currentcol, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive, true);
}

bool SearchDCOPInterface::findLastString( TQString text, bool caseSensitive)
{
	return m_parent->searchText(0,0, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive, true);
}

bool SearchDCOPInterface::findStringAt( uint  row, uint  col, TQString text, bool caseSensitive)
{
	return m_parent->searchText(row,col, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive);

}

bool SearchDCOPInterface::findFirstRegExp( TQString regexp)
{
	return m_parent->searchText( 0,0, TQRegExp(regexp), &m_currentrow, &m_currentcol,  &m_currentmatchlen);
}

bool SearchDCOPInterface::findNextRegExp( TQString regexp)
{
	return m_parent->searchText( m_currentrow, m_currentcol+1, TQRegExp(regexp), &m_currentrow, &m_currentcol,  &m_currentmatchlen);
}

bool SearchDCOPInterface::findPreviousRegExp( TQString regexp)
{
	if( m_currentcol == 0)
		m_currentrow--;
	else
		m_currentcol--;
	return m_parent->searchText( m_currentrow, m_currentcol, TQRegExp(regexp), &m_currentrow, &m_currentcol,  &m_currentmatchlen, true);

}

bool SearchDCOPInterface::findLastRegExp(TQString regexp)
{
	return m_parent->searchText( 0,0, TQRegExp(regexp), &m_currentrow, &m_currentcol,  &m_currentmatchlen, true);
}

bool SearchDCOPInterface::findRegExpAt( uint  row, uint  col, TQString regexp)
{
	return m_parent->searchText( row, col, TQRegExp(regexp), &m_currentrow, &m_currentcol, &m_currentmatchlen, false);
}

uint SearchDCOPInterface::currentMatchLine()
{
	return m_currentrow;
}
uint SearchDCOPInterface::currentMatchCol()
{
	return m_currentcol;
}
uint SearchDCOPInterface::currentMatchLength()
{
	return m_currentmatchlen;	
}


