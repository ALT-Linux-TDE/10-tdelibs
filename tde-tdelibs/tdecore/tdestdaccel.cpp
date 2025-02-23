/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@alpin.or.at)
    Copyright (C) 2000 Nicolas Hadacek (haadcek@kde.org)
    Copyright (C) 2001,2002 Ellis Whitehead (ellis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#define __KSTDACCEL_CPP_ 1

#include "tdestdaccel.h"

#include "tdeaccelaction.h"
#include "tdeaccelbase.h"
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdeshortcut.h>
#include <tdeshortcutlist.h>

namespace TDEStdAccel
{

struct TDEStdAccelInfo
{
	StdAccel id;
	const char* psName;
	const char* psDesc;
	int cutDefault, cutDefault4, cutDefault3B, cutDefault4B;
	TDEShortcut cut;
	bool bInitialized;
};

/** Array of predefined TDEStdAccelInfo objects, which cover all
    the "standard" accelerators. Each enum value from StdAccel
    should appear in this table.
*/
static TDEStdAccelInfo g_infoStdAccel[] =
{
	{AccelNone,            "Group:File", I18N_NOOP("File"), 0, 0, 0, 0, TDEShortcut(), false },
	{ Open,                I18N_NOOP("Open"), 0,     TQt::CTRL+TQt::Key_O, 0, 0, 0, TDEShortcut(), false },
	{ New,                 I18N_NOOP("New"), 0,      TQt::CTRL+TQt::Key_N, 0, 0, 0, TDEShortcut(), false },
	{ Close,               I18N_NOOP("Close"), 0,    TQt::CTRL+TQt::Key_W, TQt::CTRL+TQt::Key_Escape, 0, TQt::CTRL+TQt::Key_W, TDEShortcut(), false },
	{ Save,                I18N_NOOP("Save"), 0,     TQt::CTRL+TQt::Key_S, 0, 0, 0, TDEShortcut(), false },
	{ Print,               I18N_NOOP("Print"), 0,    TQt::CTRL+TQt::Key_P, 0, 0, 0, TDEShortcut(), false },
	{ Quit,                I18N_NOOP("Quit"), 0,     TQt::CTRL+TQt::Key_Q, 0, 0, 0, TDEShortcut(), false },
	{AccelNone,            "Group:Edit", I18N_NOOP("Edit"), 0, 0, 0, 0, TDEShortcut(), false },
	{ Undo,                I18N_NOOP("Undo"), 0,     TQt::CTRL+TQt::Key_Z, 0, 0, 0, TDEShortcut(), false },
	{ Redo,                I18N_NOOP("Redo"), 0,     TQt::CTRL+TQt::SHIFT+TQt::Key_Z, 0, 0, 0, TDEShortcut(), false },
	{ Cut,                 I18N_NOOP("Cut"), 0,      TQt::CTRL+TQt::Key_X, 0, TQt::SHIFT+TQt::Key_Delete, 0, TDEShortcut(), false },
	{ Copy,                I18N_NOOP("Copy"), 0,     TQt::CTRL+TQt::Key_C, 0, TQt::CTRL+TQt::Key_Insert, 0, TDEShortcut(), false },
	{ Paste,               I18N_NOOP("Paste"), 0,    TQt::CTRL+TQt::Key_V, 0, TQt::SHIFT+TQt::Key_Insert, 0, TDEShortcut(), false },
	{ PasteSelection,      I18N_NOOP("Paste Selection"), 0, 0, 0, TQt::CTRL+TQt::SHIFT+TQt::Key_Insert, 0, TDEShortcut(), false },
	{ SelectAll,           "SelectAll", I18N_NOOP("Select All"), TQt::CTRL+TQt::Key_A, 0, 0, 0, TDEShortcut(), false },
	{ Deselect,            I18N_NOOP("Deselect"), 0, TQt::CTRL+TQt::SHIFT+TQt::Key_A, 0, 0, 0, TDEShortcut(), false },
	{ DeleteWordBack,      "DeleteWordBack", I18N_NOOP("Delete Word Backwards"), TQt::CTRL+TQt::Key_Backspace, 0, 0, 0, TDEShortcut(), false },
	{ DeleteWordForward,   "DeleteWordForward", I18N_NOOP("Delete Word Forward"), TQt::CTRL+TQt::Key_Delete, 0,  0, 0, TDEShortcut(), false },
	{ Find,                I18N_NOOP("Find"), 0,     TQt::CTRL+TQt::Key_F, 0, 0, 0, TDEShortcut(), false },
	{ FindNext,            "FindNext", I18N_NOOP("Find Next"), TQt::Key_F3, 0, 0, 0, TDEShortcut(), false },
	{ FindPrev,            "FindPrev", I18N_NOOP("Find Prev"), TQt::SHIFT+TQt::Key_F3, 0, 0, 0, TDEShortcut(), false },
	{ Replace,             I18N_NOOP("Replace"), 0,  TQt::CTRL+TQt::Key_R, 0, 0, 0, TDEShortcut(), false },
	{AccelNone,            "Group:Navigation", I18N_NOOP("Navigation"), 0, 0, 0, 0, TDEShortcut(), false },
	{ Home,                I18N_NOOP2("Opposite to End","Home"), 0,     TQt::CTRL+TQt::Key_Home, 0, TQt::Key_HomePage, 0, TDEShortcut(), false },
	{ End,                 I18N_NOOP("End"), 0,      TQt::CTRL+TQt::Key_End, 0, 0, 0, TDEShortcut(), false },
	{ BeginningOfLine,     "BeginningOfLine", I18N_NOOP("Beginning of Line"), TQt::Key_Home, 0, 0, 0, TDEShortcut(), false},
	{ EndOfLine,           "EndOfLine", I18N_NOOP("End of Line"), TQt::Key_End, 0, 0, 0, TDEShortcut(), false},
	{ Prior,               I18N_NOOP("Prior"), 0,    TQt::Key_Prior, 0, 0, 0, TDEShortcut(), false },
	{ Next,                I18N_NOOP2("Opposite to Prior","Next"), 0,     TQt::Key_Next, 0, 0, 0, TDEShortcut(), false },
	{ GotoLine,            "GotoLine", I18N_NOOP("Go to Line"), TQt::CTRL+TQt::Key_G, 0, 0, 0, TDEShortcut(), false },
	{ AddBookmark,         "AddBookmark", I18N_NOOP("Add Bookmark"), TQt::CTRL+TQt::Key_B, 0, 0, 0, TDEShortcut(), false },
	{ ZoomIn,              "ZoomIn", I18N_NOOP("Zoom In"), TQt::CTRL+TQt::Key_Plus, 0, 0, 0, TDEShortcut(), false },
	{ ZoomOut,             "ZoomOut", I18N_NOOP("Zoom Out"), TQt::CTRL+TQt::Key_Minus, 0, 0, 0, TDEShortcut(), false },
	{ Up,                  I18N_NOOP("Up"), 0,       TQt::ALT+TQt::Key_Up, 0, 0, 0, TDEShortcut(), false },
	{ Back,                I18N_NOOP("Back"), 0,     TQt::ALT+TQt::Key_Left, 0, TQt::Key_Back, 0, TDEShortcut(), false },
	{ Forward,             I18N_NOOP("Forward"), 0,  TQt::ALT+TQt::Key_Right, 0, TQt::Key_Forward, 0, TDEShortcut(), false },
	{ Reload,              I18N_NOOP("Reload"), 0,   TQt::Key_F5, 0, TQt::Key_Refresh, 0, TDEShortcut(), false },
	{ PopupMenuContext,    "PopupMenuContext", I18N_NOOP("Popup Menu Context"), TQt::Key_Menu, 0, 0, 0, TDEShortcut(), false },
	{ ShowMenubar,         "ShowMenubar", I18N_NOOP("Show Menu Bar"), TQt::CTRL+TQt::Key_M, 0, 0, 0, TDEShortcut(), false },
	{ BackwardWord,        "BackwardWord", I18N_NOOP("Backward Word"), TQt::CTRL+TQt::Key_Left, 0, 0, 0, TDEShortcut(), false },
	{ ForwardWord,         "ForwardWord", I18N_NOOP("Forward Word"), TQt::CTRL+TQt::Key_Right, 0, 0, 0, TDEShortcut(), false },
	{ TabNext,             I18N_NOOP("Activate Next Tab"), 0,  TQt::CTRL+TQt::Key_Period, 0, TQt::CTRL+TQt::Key_BracketRight, 0, TDEShortcut(), false },
	{ TabPrev,             I18N_NOOP("Activate Previous Tab"), 0,   TQt::CTRL+TQt::Key_Comma, 0, TQt::CTRL+TQt::Key_BracketLeft, 0, TDEShortcut(), false },
	{ FullScreen,          "FullScreen", I18N_NOOP("Full Screen Mode"), TQt::CTRL+TQt::SHIFT+TQt::Key_F, 0, 0, 0, TDEShortcut(), false },
	{AccelNone,            "Group:Help", I18N_NOOP("Help"), 0, 0, 0, 0, TDEShortcut(), false },
	{ Help,                I18N_NOOP("Help"), 0,     TQt::Key_F1, 0, 0, 0, TDEShortcut(), false },
	{ WhatsThis,           "WhatsThis", I18N_NOOP("What's This"), TQt::SHIFT+TQt::Key_F1, 0, 0, 0, TDEShortcut(), false },
	{AccelNone,            "Group:TextCompletion", I18N_NOOP("Text Completion"), 0, 0, 0, 0, TDEShortcut(), false },
	{ TextCompletion,      "TextCompletion", I18N_NOOP("Text Completion"), TQt::CTRL+TQt::Key_E, 0, 0, 0, TDEShortcut(), false },
	{ PrevCompletion,      "PrevCompletion", I18N_NOOP("Previous Completion Match"), TQt::CTRL+TQt::Key_Up, 0, 0, 0, TDEShortcut(), false },
	{ NextCompletion,      "NextCompletion", I18N_NOOP("Next Completion Match"), TQt::CTRL+TQt::Key_Down, 0, 0, 0, TDEShortcut(), false },
	{ SubstringCompletion, "SubstringCompletion", I18N_NOOP("Substring Completion"), TQt::CTRL+TQt::Key_T, 0, 0, 0, TDEShortcut(), false },
	{ RotateUp,            "RotateUp", I18N_NOOP("Previous Item in List"), TQt::Key_Up, 0, 0, 0, TDEShortcut(), false },
	{ RotateDown,          "RotateDown", I18N_NOOP("Next Item in List"), TQt::Key_Down, 0, 0, 0, TDEShortcut(), false },
	{ AccelNone,           0, 0, 0, 0, 0, 0, TDEShortcut(), false }
};

/** Search for the TDEStdAccelInfo object associated with the given @p id. */
static TDEStdAccelInfo* infoPtr( StdAccel id )
{
	if( id != AccelNone ) {
		// Linear search. Changing the data structure doesn't seem possible
		// (since we need groups for the config stuff), but maybe a little
		// additional hashtable wouldn't hurt.
		for( uint i = 0; g_infoStdAccel[i].psName != 0; i++ ) {
			if( g_infoStdAccel[i].id == id )
				return &g_infoStdAccel[i];
		}
	}
	return 0;
}

/** Initialize the accelerator @p id by checking if it is overridden
    in the configuration file (and if it isn't, use the default).
*/
static void initialize( StdAccel id )
{
	TDEConfigGroupSaver saver( TDEGlobal::config(), "Shortcuts" );
	TDEStdAccelInfo* pInfo = infoPtr( id );

	if( !pInfo ) {
		kdWarning(125) << "TDEStdAccel: id not found!" << endl; // -- ellis
		return;
	}

	if( saver.config()->hasKey( pInfo->psName ) ) {
		TQString s = saver.config()->readEntry( pInfo->psName );
		if( s != "none" )
			pInfo->cut.init( s );
		else
			pInfo->cut.clear();
	} else
		pInfo->cut = shortcutDefault( id );
	pInfo->bInitialized = true;
}

TQString name( StdAccel id )
{
	TDEStdAccelInfo* pInfo = infoPtr( id );
	if( !pInfo )
		return TQString::null;
	return pInfo->psName;
}

TQString label( StdAccel id )
{
	TDEStdAccelInfo* pInfo = infoPtr( id );
	if( !pInfo )
		return TQString::null;
	return i18n((pInfo->psDesc) ? pInfo->psDesc : pInfo->psName);
}

// TODO: Add psWhatsThis entry to TDEStdAccelInfo
TQString whatsThis( StdAccel /*id*/ )
{
//	TDEStdAccelInfo* pInfo = infoPtr( id );
//	if( pInfo && pInfo->psWhatsThis )
//		return i18n(pInfo->psWhatsThis);
//	else
		return TQString::null;
}

const TDEShortcut& shortcut( StdAccel id )
{
	TDEStdAccelInfo* pInfo = infoPtr( id );
	if( !pInfo )
		return TDEShortcut::null();

	if( !pInfo->bInitialized )
		initialize( id );

	return pInfo->cut;
}

StdAccel findStdAccel( const KKeySequence& seq )
{
	if( !seq.isNull() ) {
		for( uint i = 0; g_infoStdAccel[i].psName != 0; i++ ) {
			StdAccel id = g_infoStdAccel[i].id;
			if( id != AccelNone ) {
				if( !g_infoStdAccel[i].bInitialized )
					initialize( id );
				if( g_infoStdAccel[i].cut.contains( seq ) )
					return id;
			}
		}
	}
	return AccelNone;
}

TDEShortcut shortcutDefault( StdAccel id )
{
	return (TDEAccelAction::useFourModifierKeys())
		? shortcutDefault4(id) : shortcutDefault3(id);
}

TDEShortcut shortcutDefault3( StdAccel id )
{
	TDEShortcut cut;

	TDEStdAccelInfo* pInfo = infoPtr( id );
	if( pInfo ) {
		if( pInfo->cutDefault )
			cut.init( pInfo->cutDefault );
		// FIXME: if there is no cutDefault, then this we be made the primary
		//  instead of alternate shortcut.
		if( pInfo->cutDefault3B )
			cut.append( KKey(pInfo->cutDefault3B) );
	}

	return cut;
}

TDEShortcut shortcutDefault4( StdAccel id )
{
	TDEShortcut cut;

	TDEStdAccelInfo* pInfo = infoPtr( id );
	if( pInfo ) {
		TDEStdAccelInfo& info = *pInfo;
		KKeySequence key2;

		cut.init( (info.cutDefault4) ?
			TQKeySequence(info.cutDefault) : TQKeySequence(info.cutDefault4) );

		if( info.cutDefault4B )
			key2.init( TQKeySequence(info.cutDefault4B) );
		else if( info.cutDefault3B )
			key2.init( TQKeySequence(info.cutDefault3B) );

		if( key2.count() )
			cut.append( key2 );
	}

	return cut;
}

#if 0 // unused
void createAccelActions( TDEAccelActions& actions )
{
	actions.clear();

	for( uint i = 0; g_infoStdAccel[i].psName != 0; i++ ) {
		StdAccel id = g_infoStdAccel[i].id;
		TDEStdAccelInfo* pInfo = &g_infoStdAccel[i];

		if( id != AccelNone ) {
			actions.insert( pInfo->psName,
				i18n((pInfo->psDesc) ? pInfo->psDesc : pInfo->psName),
				TQString::null, // pInfo->psWhatsThis,
				shortcutDefault3(id),
				shortcutDefault4(id) );
		} else
			actions.insert( pInfo->psName, i18n(pInfo->psDesc) );
	}
}
#endif

const TDEShortcut& open()                  { return shortcut( Open ); }
const TDEShortcut& openNew()               { return shortcut( New ); }
const TDEShortcut& close()                 { return shortcut( Close ); }
const TDEShortcut& save()                  { return shortcut( Save ); }
const TDEShortcut& print()                 { return shortcut( Print ); }
const TDEShortcut& quit()                  { return shortcut( Quit ); }
const TDEShortcut& cut()                   { return shortcut( Cut ); }
const TDEShortcut& copy()                  { return shortcut( Copy ); }
const TDEShortcut& paste()                 { return shortcut( Paste ); }
const TDEShortcut& pasteSelection()        { return shortcut( PasteSelection ); }
const TDEShortcut& deleteWordBack()        { return shortcut( DeleteWordBack ); }
const TDEShortcut& deleteWordForward()     { return shortcut( DeleteWordForward ); }
const TDEShortcut& undo()                  { return shortcut( Undo ); }
const TDEShortcut& redo()                  { return shortcut( Redo ); }
const TDEShortcut& find()                  { return shortcut( Find ); }
const TDEShortcut& findNext()              { return shortcut( FindNext ); }
const TDEShortcut& findPrev()              { return shortcut( FindPrev ); }
const TDEShortcut& replace()               { return shortcut( Replace ); }
const TDEShortcut& home()                  { return shortcut( Home ); }
const TDEShortcut& end()                   { return shortcut( End ); }
const TDEShortcut& beginningOfLine()       { return shortcut( BeginningOfLine ); }
const TDEShortcut& endOfLine()             { return shortcut( EndOfLine ); }
const TDEShortcut& prior()                 { return shortcut( Prior ); }
const TDEShortcut& next()                  { return shortcut( Next ); }
const TDEShortcut& backwardWord()          { return shortcut( BackwardWord ); }
const TDEShortcut& forwardWord()           { return shortcut( ForwardWord ); }
const TDEShortcut& gotoLine()              { return shortcut( GotoLine ); }
const TDEShortcut& addBookmark()           { return shortcut( AddBookmark ); }
const TDEShortcut& tabNext()               { return shortcut( TabNext ); }
const TDEShortcut& tabPrev()               { return shortcut( TabPrev ); }
const TDEShortcut& fullScreen()            { return shortcut( FullScreen ); }
const TDEShortcut& zoomIn()                { return shortcut( ZoomIn ); }
const TDEShortcut& zoomOut()               { return shortcut( ZoomOut ); }
const TDEShortcut& help()                  { return shortcut( Help ); }
const TDEShortcut& completion()            { return shortcut( TextCompletion ); }
const TDEShortcut& prevCompletion()        { return shortcut( PrevCompletion ); }
const TDEShortcut& nextCompletion()        { return shortcut( NextCompletion ); }
const TDEShortcut& rotateUp()              { return shortcut( RotateUp ); }
const TDEShortcut& rotateDown()            { return shortcut( RotateDown ); }
const TDEShortcut& substringCompletion()   { return shortcut( SubstringCompletion ); }
const TDEShortcut& popupMenuContext()      { return shortcut( PopupMenuContext ); }
const TDEShortcut& whatsThis()             { return shortcut( WhatsThis ); }
const TDEShortcut& reload()                { return shortcut( Reload ); }
const TDEShortcut& selectAll()             { return shortcut( SelectAll ); }
const TDEShortcut& up()                    { return shortcut( Up ); }
const TDEShortcut& back()                  { return shortcut( Back ); }
const TDEShortcut& forward()               { return shortcut( Forward ); }
const TDEShortcut& showMenubar()           { return shortcut( ShowMenubar ); }

//---------------------------------------------------------------------
// ShortcutList
//---------------------------------------------------------------------

ShortcutList::ShortcutList()
	{ }

ShortcutList::~ShortcutList()
	{ }

uint ShortcutList::count() const
{
	static uint g_nAccels = 0;
	if( g_nAccels == 0 ) {
		for( ; g_infoStdAccel[g_nAccels].psName != 0; g_nAccels++ )
			;
	}
	return g_nAccels;
}

TQString ShortcutList::name( uint i ) const
	{ return g_infoStdAccel[i].psName; }

TQString ShortcutList::label( uint i ) const
	{ return i18n((g_infoStdAccel[i].psDesc) ? g_infoStdAccel[i].psDesc : g_infoStdAccel[i].psName); }

TQString ShortcutList::whatsThis( uint ) const
	{ return TQString::null; }

const TDEShortcut& ShortcutList::shortcut( uint i ) const
{
	if( !g_infoStdAccel[i].bInitialized )
		initialize( g_infoStdAccel[i].id );
	return g_infoStdAccel[i].cut;
}

const TDEShortcut& ShortcutList::shortcutDefault( uint i ) const
{
	static TDEShortcut cut;
	cut = TDEStdAccel::shortcutDefault( g_infoStdAccel[i].id );
	return cut;
}

bool ShortcutList::isConfigurable( uint i ) const
	{ return (g_infoStdAccel[i].id != AccelNone); }

bool ShortcutList::setShortcut( uint i, const TDEShortcut& cut )
	{ g_infoStdAccel[i].cut = cut; return true; }

TQVariant ShortcutList::getOther( Other, uint ) const
	{ return TQVariant(); }

bool ShortcutList::setOther( Other, uint, TQVariant )
	{ return false; }

bool ShortcutList::save() const
{
	return writeSettings( TQString::null, 0, false, true );
}

TDECORE_EXPORT TQString action(StdAccel id)
	{ return name(id); }
TDECORE_EXPORT TQString description(StdAccel id)
	{ return label(id); }
TDECORE_EXPORT int key(StdAccel id)
	{ return shortcut(id).keyCodeQt(); }
TDECORE_EXPORT int defaultKey(StdAccel id)
	{ return shortcutDefault(id).keyCodeQt(); }

TDECORE_EXPORT bool isEqual(const TQKeyEvent* ev, int keyQt)
{
	KKey key1( ev ), key2( keyQt );
	return key1 == key2;
}

}

#undef __KSTDACCEL_CPP_
