/* This file is part of the KDE libraries
   Copyright (C) 2001 Holger Freyther <freyther@kde.org>

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

#include "kstdguiitem.h"

#include <kguiitem.h>
#include <tdelocale.h>
#include <tdeapplication.h>

KGuiItem KStdGuiItem::guiItem ( StdItem ui_enum )
{
  switch (ui_enum ) {
  case Ok       : return ok();
  case Cancel   : return cancel();
  case Yes      : return yes();
  case No       : return no();
  case Discard  : return discard();
  case Save     : return save();
  case DontSave : return dontSave();
  case SaveAs   : return saveAs();
  case Apply    : return apply();
  case Clear    : return clear();
  case Help     : return help();
  case Close    : return close();
  case Defaults : return defaults();
  case Back     : return back();
  case Forward  : return forward();
  case Print    : return print();
  case Continue : return cont();
  case Open     : return open();
  case Quit     : return quit();
  case AdminMode: return adminMode();
  case Reset    : return reset();
  case Delete   : return del();
  case Insert   : return insert();
  case Find     : return find();
  case Stop     : return stop();
  case Add      : return add();
  case Remove   : return remove();
  case Test     : return test();
  case Properties : return properties();
  case Overwrite : return overwrite();
  default       : return KGuiItem();
  };
}

TQString KStdGuiItem::stdItem( StdItem ui_enum )
{
  switch (ui_enum ) {
  case Ok       : return TQString::fromLatin1("ok");
  case Cancel   : return TQString::fromLatin1("cancel");
  case Yes      : return TQString::fromLatin1("yes");
  case No       : return TQString::fromLatin1("no");
  case Discard  : return TQString::fromLatin1("discard");
  case Save     : return TQString::fromLatin1("save");
  case DontSave : return TQString::fromLatin1("dontSave");
  case SaveAs   : return TQString::fromLatin1("saveAs");
  case Apply    : return TQString::fromLatin1("apply");
  case Help     : return TQString::fromLatin1("help");
  case Close    : return TQString::fromLatin1("close");
  case Defaults : return TQString::fromLatin1("defaults");
  case Back     : return TQString::fromLatin1("back");
  case Forward  : return TQString::fromLatin1("forward");
  case Print    : return TQString::fromLatin1("print");
  case Continue : return TQString::fromLatin1("continue");
  case Open     : return TQString::fromLatin1("open");
  case Quit     : return TQString::fromLatin1("quit");
  case AdminMode: return TQString::fromLatin1("adminMode");
  case Delete   : return TQString::fromLatin1("delete");
  case Insert   : return TQString::fromLatin1("insert");
  case Find     : return TQString::fromLatin1("find");
  case Stop     : return TQString::fromLatin1("stop");
  case Add      : return TQString::fromLatin1("add");
  case Remove   : return TQString::fromLatin1("remove");
  case Test     : return TQString::fromLatin1("test");
  case Properties : return TQString::fromLatin1("properties");
  case Overwrite : return TQString::fromLatin1("overwrite");
  default       : return TQString::null;
  };
}

KGuiItem KStdGuiItem::ok()
{
  return KGuiItem( i18n( "&OK" ), "button_ok" );
}


KGuiItem KStdGuiItem::cancel()
{
  return KGuiItem( i18n( "&Cancel" ), "button_cancel" );
}

KGuiItem KStdGuiItem::yes()
{
  return KGuiItem( i18n( "&Yes" ), "button_ok", i18n( "Yes" ) );
}

KGuiItem KStdGuiItem::no()
{
  return KGuiItem( i18n( "&No" ), "window-close", i18n( "No" ) );
}

KGuiItem KStdGuiItem::discard()
{
  return KGuiItem( i18n( "&Discard" ), "edittrash", i18n( "Discard changes" ),
                   i18n( "Pressing this button will discard all recent "
                         "changes made in this dialog" ) );
}

KGuiItem KStdGuiItem::save()
{
  return KGuiItem( i18n( "&Save" ), "document-save", i18n( "Save data" ) );
}

KGuiItem KStdGuiItem::dontSave()
{
  return KGuiItem( i18n( "&Do Not Save" ), "edittrash",
                   i18n( "Don't save data" ) );
}

KGuiItem KStdGuiItem::saveAs()
{
  return KGuiItem( i18n( "Save &As..." ), "document-save-as",
                   i18n( "Save file with another name" ) );
}

KGuiItem KStdGuiItem::apply()
{
  return KGuiItem( i18n( "&Apply" ), "apply", i18n( "Apply changes" ),
                   i18n( "When clicking <b>Apply</b>, the settings will be "
                         "handed over to the program, but the dialog "
                         "will not be closed.\n"
                         "Use this to try different settings." ) );
}

KGuiItem KStdGuiItem::adminMode()
{
  return KGuiItem( i18n( "Administrator &Mode..." ), "wizard", i18n( "Enter Administrator Mode" ),
                   i18n( "When clicking <b>Administrator Mode</b> you will be prompted "
                         "for the administrator (root) password in order to make changes "
                         "which require root privileges." ) );
}

KGuiItem KStdGuiItem::clear()
{
  return KGuiItem( i18n( "C&lear" ), "locationbar_erase",
                   i18n( "Clear input" ),
                   i18n( "Clear the input in the edit field" ) );
}

KGuiItem KStdGuiItem::help()
{
  return KGuiItem( i18n( "show help", "&Help" ), "help",
                   i18n( "Show help" ) );
}

KGuiItem KStdGuiItem::close()
{
  return KGuiItem( i18n( "&Close" ), "window-close",
                   i18n( "Close the current window or document" ) );
}

KGuiItem KStdGuiItem::defaults()
{
  return KGuiItem( i18n( "&Defaults" ), "reload",
                   i18n( "Reset all items to their default values" ) );
}

KGuiItem KStdGuiItem::back( BidiMode useBidi )
{
  TQString icon = ( useBidi == UseRTL && TQApplication::reverseLayout() )
                 ? "forward" : "back";
  return KGuiItem( i18n( "go back", "&Back" ), icon,
                   i18n( "Go back one step" ) );
}

KGuiItem KStdGuiItem::forward( BidiMode useBidi )
{
  TQString icon = ( useBidi == UseRTL && TQApplication::reverseLayout() )
                 ? "back" : "forward";
  return KGuiItem( i18n( "go forward", "&Forward" ), icon,
                   i18n( "Go forward one step" ) );
}

TQPair<KGuiItem, KGuiItem> KStdGuiItem::backAndForward()
{
    return qMakePair( back( UseRTL ), forward( UseRTL ) );
}

KGuiItem KStdGuiItem::print()
{
  return KGuiItem( i18n( "&Print..." ), "document-print",
                   i18n( "Opens the print dialog to print "
                         "the current document" ) );
}

KGuiItem KStdGuiItem::cont()
{
  return KGuiItem( i18n( "C&ontinue" ), TQString::null,
                   i18n( "Continue operation" ) );
}

KGuiItem KStdGuiItem::del()
{
  return KGuiItem( i18n( "&Delete" ), "edit-delete",
                   i18n( "Delete item(s)" ) );
}

KGuiItem KStdGuiItem::open()
{
  return KGuiItem( i18n( "&Open..." ), "document-open",
                   i18n( "Open file" ) );
}

KGuiItem KStdGuiItem::quit()
{
  return KGuiItem( i18n( "&Quit" ), "system-log-out",
                   i18n( "Quit application" ) );
}

KGuiItem KStdGuiItem::reset()
{
  return KGuiItem( i18n( "&Reset" ), "edit-undo",
                  i18n( "Reset configuration" ) );
}

KGuiItem KStdGuiItem::insert()
{
  return KGuiItem( i18n( "Verb", "&Insert" ), "edit-paste");
}

KGuiItem KStdGuiItem::configure()
{
  return KGuiItem( i18n( "Confi&gure..." ), "configure" );
}

KGuiItem KStdGuiItem::find()
{
  return KGuiItem( i18n("&Find"), "edit-find" );
}

KGuiItem KStdGuiItem::stop()
{
  return KGuiItem( i18n("Stop"), "process-stop" );
}

KGuiItem KStdGuiItem::add()
{
  return KGuiItem( i18n("Add"), "add" );
}

KGuiItem KStdGuiItem::remove()
{
  return KGuiItem( i18n("Remove"), "remove" );
}

KGuiItem KStdGuiItem::test()
{
  return KGuiItem( i18n("Test") );
}

KGuiItem KStdGuiItem::properties()
{
  return KGuiItem( i18n("Properties"), "document-properties" );
}

KGuiItem KStdGuiItem::overwrite()
{
  return KGuiItem( i18n("&Overwrite"), "document-save-as" );
}
