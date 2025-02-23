/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>

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

#include "kstdaction.h"

#include <tqtoolbutton.h>
#include <tqwhatsthis.h>

#include <tdeaboutdata.h>
#include <tdeaction.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <tdestdaccel.h>
#include <tdemainwindow.h>
#include "kstdaction_p.h"

namespace KStdAction
{

TQStringList stdNames()
{
    return internal_stdNames();
}

TDEAction* create( StdAction id, const char *name, const TQObject *recvr, const char *slot, TDEActionCollection* parent )
{
	TDEAction* pAction = 0;
	const KStdActionInfo* pInfo = infoPtr( id );
	kdDebug(125) << "KStdAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << ", " << name << " )" << endl; // ellis
	if( pInfo ) {
		TQString sLabel, iconName = pInfo->psIconName;
		switch( id ) {
		 case Back: sLabel = i18n("go back", "&Back");
			if (TQApplication::reverseLayout() )
			    iconName = "forward";
			break;

		 case Forward: sLabel = i18n("go forward", "&Forward");
			if (TQApplication::reverseLayout() )
			    iconName = "back";
			break;

		 case Home: sLabel = i18n("beginning (of line)", "&Home"); break;
		 case Help: sLabel = i18n("show help", "&Help"); break;
		 case AboutApp: iconName = kapp->miniIconName();
		 case Preferences:
		 case HelpContents:
			{
			const TDEAboutData *aboutData = TDEGlobal::instance()->aboutData();
			/* TODO KDE4
			const TDEAboutData *aboutData;
			if ( parent )
			    aboutData = parent->instance()->aboutData();
			else
			    aboutData = TDEGlobal::instance()->aboutData();
			*/
			TQString appName = (aboutData) ? aboutData->programName() : TQString::fromLatin1(tqApp->name());
			sLabel = i18n(pInfo->psLabel).arg(appName);
			}
			break;
		 default: sLabel = i18n(pInfo->psLabel);
		}

		if (TQApplication::reverseLayout()){
			if (id == Prior) iconName = "forward";
			if (id == Next ) iconName = "back";
			if (id == FirstPage) iconName = "go-last";
			if (id == LastPage) iconName = "go-first";
		}

		TDEShortcut cut = TDEStdAccel::shortcut(pInfo->idAccel);
		switch( id ) {
		 case OpenRecent:
			pAction = new TDERecentFilesAction( sLabel, pInfo->psIconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			break;
		 case ShowMenubar:
		 case ShowToolbar:
		 case ShowStatusbar:
		 {
			TDEToggleAction *ret;
			ret = new TDEToggleAction( sLabel, pInfo->psIconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			ret->setChecked( true );
			pAction = ret;
			break;
		 }
		 case FullScreen:
		 {
			TDEToggleFullScreenAction *ret;
			ret = new TDEToggleFullScreenAction( cut, recvr, slot,
					parent, NULL, (name) ? name : pInfo->psName );
			ret->setChecked( false );
			pAction = ret;
			break;
		 }
         case PasteText:
         {
            TDEPasteTextAction *ret;
            ret = new TDEPasteTextAction(sLabel, iconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
            pAction = ret;
            break;
         }
		 default:
			pAction = new TDEAction( sLabel, iconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			break;
		}
	}
	return pAction;
}

const char* name( StdAction id )
{
	const KStdActionInfo* pInfo = infoPtr( id );
	return (pInfo) ? pInfo->psName : 0;
}

TDEAction *openNew( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( New, name, recvr, slot, parent ); }
TDEAction *open( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Open, name, recvr, slot, parent ); }
TDERecentFilesAction *openRecent( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return (TDERecentFilesAction*) KStdAction::create( OpenRecent, name, recvr, slot, parent ); }
TDEAction *save( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Save, name, recvr, slot, parent ); }
TDEAction *saveAs( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( SaveAs, name, recvr, slot, parent ); }
TDEAction *revert( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Revert, name, recvr, slot, parent ); }
TDEAction *print( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Print, name, recvr, slot, parent ); }
TDEAction *printPreview( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( PrintPreview, name, recvr, slot, parent ); }
TDEAction *close( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Close, name, recvr, slot, parent ); }
TDEAction *mail( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Mail, name, recvr, slot, parent ); }
TDEAction *quit( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Quit, name, recvr, slot, parent ); }
TDEAction *undo( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Undo, name, recvr, slot, parent ); }
TDEAction *redo( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Redo, name, recvr, slot, parent ); }
TDEAction *cut( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Cut, name, recvr, slot, parent ); }
TDEAction *copy( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Copy, name, recvr, slot, parent ); }
TDEAction *paste( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Paste, name, recvr, slot, parent ); }
TDEAction *pasteText( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( PasteText, name, recvr, slot, parent ); }
TDEAction *clear( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Clear, name, recvr, slot, parent ); }
TDEAction *selectAll( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( SelectAll, name, recvr, slot, parent ); }
TDEAction *deselect( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Deselect, name, recvr, slot, parent ); }
TDEAction *find( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Find, name, recvr, slot, parent ); }
TDEAction *findNext( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( FindNext, name, recvr, slot, parent ); }
TDEAction *findPrev( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( FindPrev, name, recvr, slot, parent ); }
TDEAction *replace( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Replace, name, recvr, slot, parent ); }
TDEAction *actualSize( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( ActualSize, name, recvr, slot, parent ); }
TDEAction *fitToPage( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToPage, name, recvr, slot, parent ); }
TDEAction *fitToWidth( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToWidth, name, recvr, slot, parent ); }
TDEAction *fitToHeight( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToHeight, name, recvr, slot, parent ); }
TDEAction *zoomIn( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( ZoomIn, name, recvr, slot, parent ); }
TDEAction *zoomOut( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( ZoomOut, name, recvr, slot, parent ); }
TDEAction *zoom( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Zoom, name, recvr, slot, parent ); }
TDEAction *redisplay( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Redisplay, name, recvr, slot, parent ); }
TDEAction *up( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Up, name, recvr, slot, parent ); }
TDEAction *back( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Back, name, recvr, slot, parent ); }
TDEAction *forward( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Forward, name, recvr, slot, parent ); }
TDEAction *home( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Home, name, recvr, slot, parent ); }
TDEAction *prior( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Prior, name, recvr, slot, parent ); }
TDEAction *next( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Next, name, recvr, slot, parent ); }
TDEAction *goTo( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Goto, name, recvr, slot, parent ); }
TDEAction *gotoPage( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( GotoPage, name, recvr, slot, parent ); }
TDEAction *gotoLine( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( GotoLine, name, recvr, slot, parent ); }
TDEAction *firstPage( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( FirstPage, name, recvr, slot, parent ); }
TDEAction *lastPage( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( LastPage, name, recvr, slot, parent ); }
TDEAction *addBookmark( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( AddBookmark, name, recvr, slot, parent ); }
TDEAction *editBookmarks( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( EditBookmarks, name, recvr, slot, parent ); }
TDEAction *spelling( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Spelling, name, recvr, slot, parent ); }

TDEToggleAction *showMenubar( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *_name )
{
    TDEToggleAction *ret;
    ret = new TDEToggleAction(i18n("Show &Menubar"), "showmenu", TDEStdAccel::shortcut(TDEStdAccel::ShowMenubar), recvr, slot,
                            parent, _name ? _name : name(ShowMenubar));
    ret->setWhatsThis( i18n( "Show Menubar<p>"
                             "Shows the menubar again after it has been hidden" ) );
    KGuiItem guiItem( i18n("Hide &Menubar"), 0 /*same icon*/, TQString::null,
                      i18n( "Hide Menubar<p>"
                            "Hide the menubar. You can usually get it back using the right mouse button inside the window itself." ) );
    ret->setCheckedState( guiItem );
    ret->setChecked(true);
    return ret;
}

// obsolete
TDEToggleAction *showToolbar( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *_name )
{
    TDEToggleAction *ret;
    ret = new TDEToggleAction(i18n("Show &Toolbar"), 0, recvr, slot, parent,
                            _name ? _name : name(ShowToolbar));
    ret->setChecked(true);
    return ret;

}

// obsolete
TDEToggleToolBarAction *showToolbar( const char* toolBarName, TDEActionCollection* parent, const char *_name )
{
    TDEToggleToolBarAction *ret;
    ret = new TDEToggleToolBarAction(toolBarName, i18n("Show &Toolbar"), parent,
                            _name ? _name : name(ShowToolbar));
    return ret;
}

TDEToggleAction *showStatusbar( const TQObject *recvr, const char *slot,
                                         TDEActionCollection* parent, const char *_name )
{
    TDEToggleAction *ret;
    ret = new TDEToggleAction(i18n("Show St&atusbar"), 0, recvr, slot, parent,
                            _name ? _name : name(ShowStatusbar));
    ret->setWhatsThis( i18n( "Show Statusbar<p>"
                             "Shows the statusbar, which is the bar at the bottom of the window used for status information." ) );
    KGuiItem guiItem( i18n("Hide St&atusbar"), TQString::null, TQString::null,
                      i18n( "Hide Statusbar<p>"
                            "Hides the statusbar, which is the bar at the bottom of the window used for status information." ) );
    ret->setCheckedState( guiItem );

    ret->setChecked(true);
    return ret;
}

TDEToggleFullScreenAction *fullScreen( const TQObject *recvr, const char *slot, TDEActionCollection* parent,
    TQWidget* window, const char *name )
{
    TDEToggleFullScreenAction *ret;
    ret = static_cast< TDEToggleFullScreenAction* >( KStdAction::create( FullScreen, name, recvr, slot, parent ));
    ret->setWindow( window );
    return ret;
}

TDEAction *saveOptions( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( SaveOptions, name, recvr, slot, parent ); }
TDEAction *keyBindings( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( KeyBindings, name, recvr, slot, parent ); }
TDEAction *preferences( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Preferences, name, recvr, slot, parent ); }
TDEAction *configureToolbars( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( ConfigureToolbars, name, recvr, slot, parent ); }
TDEAction *configureNotifications( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( ConfigureNotifications, name, recvr, slot, parent ); }
TDEAction *help( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( Help, name, recvr, slot, parent ); }
TDEAction *helpContents( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( HelpContents, name, recvr, slot, parent ); }
TDEAction *whatsThis( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( WhatsThis, name, recvr, slot, parent ); }
TDEAction *tipOfDay( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( TipofDay, name, recvr, slot, parent ); }
TDEAction *reportBug( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( ReportBug, name, recvr, slot, parent ); }
TDEAction *switchApplicationLanguage( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( SwitchApplicationLanguage, name, recvr, slot, parent ); }
TDEAction *aboutApp( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( AboutApp, name, recvr, slot, parent ); }
TDEAction *aboutKDE( const TQObject *recvr, const char *slot, TDEActionCollection* parent, const char *name )
	{ return KStdAction::create( AboutKDE, name, recvr, slot, parent ); }

}
