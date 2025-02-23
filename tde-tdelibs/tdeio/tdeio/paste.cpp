/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "paste.h"
#include "pastedialog.h"

#include "tdeio/job.h"
#include "tdeio/global.h"
#include "tdeio/netaccess.h"
#include "tdeio/observer.h"
#include "tdeio/renamedlg.h"
#include "tdeio/tdeprotocolmanager.h"

#include <kurl.h>
#include <kurldrag.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <kinputdialog.h>
#include <tdemessagebox.h>
#include <kmimetype.h>
#include <tdetempfile.h>

#include <tqapplication.h>
#include <tqclipboard.h>
#include <tqdragobject.h>
#include <tqtextstream.h>
#include <tqvaluevector.h>

static KURL getNewFileName( const KURL &u, const TQString& text )
{
  bool ok;
  TQString dialogText( text );
  if ( dialogText.isEmpty() )
    dialogText = i18n( "Filename for clipboard content:" );
  TQString file = KInputDialog::getText( TQString::null, dialogText, TQString::null, &ok );
  if ( !ok )
     return KURL();

  KURL myurl(u);
  myurl.addPath( file );

  if (TDEIO::NetAccess::exists(myurl, false, 0))
  {
      kdDebug(7007) << "Paste will overwrite file.  Prompting..." << endl;
      TDEIO::RenameDlg_Result res = TDEIO::R_OVERWRITE;

      TQString newPath;
      // Ask confirmation about resuming previous transfer
      res = Observer::self()->open_RenameDlg(
                          0L, i18n("File Already Exists"),
                          u.pathOrURL(),
                          myurl.pathOrURL(),
                          (TDEIO::RenameDlg_Mode) (TDEIO::M_OVERWRITE | TDEIO::M_SINGLE), newPath);

      if ( res == TDEIO::R_RENAME )
      {
          myurl = newPath;
      }
      else if ( res == TDEIO::R_CANCEL )
      {
          return KURL();
      }
  }

  return myurl;
}

// The finaly step: write _data to tempfile and move it to neW_url
static TDEIO::CopyJob* pasteDataAsyncTo( const KURL& new_url, const TQByteArray& _data )
{
     KTempFile tempFile;
     tempFile.dataStream()->writeRawBytes( _data.data(), _data.size() );
     tempFile.close();

     KURL orig_url;
     orig_url.setPath(tempFile.name());

     return TDEIO::move( orig_url, new_url );
}

#ifndef TQT_NO_MIMECLIPBOARD
static TDEIO::CopyJob* chooseAndPaste( const KURL& u, TQMimeSource* data,
                                     const TQValueVector<TQCString>& formats,
                                     const TQString& text,
                                     TQWidget* widget,
                                     bool clipboard )
{
    TQStringList formatLabels;
    for ( uint i = 0; i < formats.size(); ++i ) {
        const TQCString& fmt = formats[i];
        KMimeType::Ptr mime = KMimeType::mimeType( fmt );
        if ( mime != KMimeType::defaultMimeTypePtr() )
            formatLabels.append( i18n( "%1 (%2)" ).arg( mime->comment() ).arg( fmt.data() ) );
        else
            formatLabels.append( fmt );
    }

    TQString dialogText( text );
    if ( dialogText.isEmpty() )
        dialogText = i18n( "Filename for clipboard content:" );
    TDEIO::PasteDialog dlg( TQString::null, dialogText, TQString::null, formatLabels, widget, clipboard );

    if ( dlg.exec() != KDialogBase::Accepted )
        return 0;

    if ( clipboard && dlg.clipboardChanged() ) {
        KMessageBox::sorry( widget,
                            i18n( "The clipboard has changed since you used 'paste': "
                                  "the chosen data format is no longer applicable. "
                                  "Please copy again what you wanted to paste." ) );
        return 0;
    }

    const TQString result = dlg.lineEditText();
    const TQCString chosenFormat = formats[ dlg.comboItem() ];

    kdDebug() << " result=" << result << " chosenFormat=" << chosenFormat << endl;
    KURL new_url( u );
    new_url.addPath( result );
    // if "data" came from TQClipboard, then it was deleted already - by a nice 0-seconds timer
    // In that case, get it again. Let's hope the user didn't copy something else meanwhile :/
    if ( clipboard ) {
        data = TQApplication::clipboard()->data();
    }
    const TQByteArray ba = data->encodedData( chosenFormat );
    return pasteDataAsyncTo( new_url, ba );
}
#endif

// KDE4: remove
TDEIO_EXPORT bool TDEIO::isClipboardEmpty()
{
#ifndef TQT_NO_MIMECLIPBOARD
  TQMimeSource *data = TQApplication::clipboard()->data();
  if ( data->provides( "text/uri-list" ) && data->encodedData( "text/uri-list" ).size() > 0 )
    return false;
#else
  // Happens with some versions of Qt Embedded... :/
  // Guess.
  TQString data = TQApplication::clipboard()->text();
  if(data.contains("://"))
	  return false;
#endif
  return true;
}

#ifndef TQT_NO_MIMECLIPBOARD
// The main method for dropping
TDEIO::CopyJob* TDEIO::pasteMimeSource( TQMimeSource* data, const KURL& dest_url,
                                    const TQString& dialogText, TQWidget* widget, bool clipboard )
{
  TQByteArray ba;

  // Now check for plain text
  // We don't want to display a mimetype choice for a TQTextDrag, those mimetypes look ugly.
  TQString text;
  if ( TQTextDrag::canDecode( data ) && TQTextDrag::decode( data, text ) )
  {
      TQTextStream txtStream( ba, IO_WriteOnly );
      txtStream << text;
  }
  else
  {
      TQValueVector<TQCString> formats;
      const char* fmt;
      for ( int i = 0; ( fmt = data->format( i ) ); ++i ) {
          if ( qstrcmp( fmt, "application/x-qiconlist" ) == 0 ) // see QIconDrag
              continue;
          if ( qstrcmp( fmt, "application/x-tde-cutselection" ) == 0 ) // see KonqDrag
              continue;
          if ( strchr( fmt, '/' ) == 0 ) // e.g. TARGETS, MULTIPLE, TIMESTAMP
              continue;
          formats.append( fmt );
      }

      if ( formats.size() == 0 )
          return 0;

      if ( formats.size() > 1 ) {
          return chooseAndPaste( dest_url, data, formats, dialogText, widget, clipboard );
      }
      ba = data->encodedData( formats.first() );
  }
  if ( ba.size() == 0 )
  {
    KMessageBox::sorry(0, i18n("The clipboard is empty"));
    return 0;
  }

  return pasteDataAsync( dest_url, ba, dialogText );
}
#endif

// The main method for pasting
TDEIO_EXPORT TDEIO::Job *TDEIO::pasteClipboard( const KURL& dest_url, bool move )
{
  if ( !dest_url.isValid() ) {
    KMessageBox::error( 0L, i18n( "Malformed URL\n%1" ).arg( dest_url.url() ) );
    return 0;
  }

#ifndef TQT_NO_MIMECLIPBOARD
  TQMimeSource *data = TQApplication::clipboard()->data();

  // First check for URLs.
  KURL::List urls;
  if ( KURLDrag::canDecode( data ) && KURLDrag::decode( data, urls ) ) {
    if ( urls.count() == 0 ) {
      KMessageBox::error( 0L, i18n("The clipboard is empty"));
      return 0;
    }

    TDEIO::Job *res = 0;
    if ( move )
      res = TDEIO::move( urls, dest_url );
    else
      res = TDEIO::copy( urls, dest_url );

    // If moving, erase the clipboard contents, the original files don't exist anymore
    if ( move )
      TQApplication::clipboard()->clear();
    return res;
  }
  return pasteMimeSource( data, dest_url, TQString::null, 0 /*TODO parent widget*/, true /*clipboard*/ );
#else
  TQByteArray ba;
  TQTextStream txtStream( ba, IO_WriteOnly );
  TQStringList data = TQStringList::split("\n", TQApplication::clipboard()->text());
  KURL::List urls;
  KURLDrag::decode(data, urls);
  TQStringList::Iterator end(data.end());
  for(TQStringList::Iterator it=data.begin(); it!=end; ++it)
      txtStream << *it;
  if ( ba.size() == 0 )
  {
    KMessageBox::sorry(0, i18n("The clipboard is empty"));
    return 0;
  }
  return pasteDataAsync( dest_url, ba );
#endif
}


TDEIO_EXPORT void TDEIO::pasteData( const KURL& u, const TQByteArray& _data )
{
    KURL new_url = getNewFileName( u, TQString::null );
    // We could use TDEIO::put here, but that would require a class
    // for the slotData call. With NetAccess, we can do a synchronous call.

    if (new_url.isEmpty())
       return;

    KTempFile tempFile;
    tempFile.setAutoDelete( true );
    tempFile.dataStream()->writeRawBytes( _data.data(), _data.size() );
    tempFile.close();

    (void) TDEIO::NetAccess::upload( tempFile.name(), new_url, 0 );
}

TDEIO_EXPORT TDEIO::CopyJob* TDEIO::pasteDataAsync( const KURL& u, const TQByteArray& _data )
{
    return pasteDataAsync( u, _data, TQString::null );
}

TDEIO_EXPORT TDEIO::CopyJob* TDEIO::pasteDataAsync( const KURL& u, const TQByteArray& _data, const TQString& text )
{
    KURL new_url = getNewFileName( u, text );

    if (new_url.isEmpty())
       return 0;

    return pasteDataAsyncTo( new_url, _data );
}

TDEIO_EXPORT TQString TDEIO::pasteActionText()
{
    TQMimeSource *data = TQApplication::clipboard()->data();
    KURL::List urls;
    if ( KURLDrag::canDecode( data ) && KURLDrag::decode( data, urls ) ) {
        if ( urls.isEmpty() )
            return TQString::null; // nothing to paste
        else if ( urls.first().isLocalFile() )
            return i18n( "&Paste File", "&Paste %n Files", urls.count() );
        else
            return i18n( "&Paste URL", "&Paste %n URLs", urls.count() );
    } else if ( data->format(0) != 0 ) {
        return i18n( "&Paste Clipboard Contents" );
    } else {
        return TQString::null;
    }
}

