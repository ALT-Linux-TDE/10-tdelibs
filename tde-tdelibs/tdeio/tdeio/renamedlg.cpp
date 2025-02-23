/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
                  2001 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "tdeio/renamedlg.h"
#include "tdeio/renamedlgplugin.h"
#include <stdio.h>
#include <assert.h>

#include <tqfileinfo.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqdir.h>

#include <tdemessagebox.h>
#include <kpushbutton.h>
#include <tdeapplication.h>
#include <tdeio/global.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kdialog.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kseparator.h>
#include <kstringhandler.h>
#include <kstdguiitem.h>
#include <kguiitem.h>
#include <ksqueezedtextlabel.h>

#ifdef TQ_WS_X11
#include <twin.h>
#endif

using namespace TDEIO;

class RenameDlg::RenameDlgPrivate
{
 public:
  RenameDlgPrivate(){
    bCancel = 0;
    bRename = bSkip = bAutoSkip = bOverwrite = bOverwriteAll = 0;
    bResume = bResumeAll = bSuggestNewName = 0;
    m_pLineEdit = 0;
  }
  KPushButton *bCancel;
  TQPushButton *bRename;
  TQPushButton *bSkip;
  TQPushButton *bAutoSkip;
  TQPushButton *bOverwrite;
  TQPushButton *bOverwriteAll;
  TQPushButton *bResume;
  TQPushButton *bResumeAll;
  TQPushButton *bSuggestNewName;
  TQLineEdit* m_pLineEdit;
  KURL src;
  KURL dest;
  TQString mimeSrc;
  TQString mimeDest;
  bool modal;
  bool plugin;
};

RenameDlg::RenameDlg(TQWidget *parent, const TQString & _caption,
                     const TQString &_src, const TQString &_dest,
                     RenameDlg_Mode _mode,
                     TDEIO::filesize_t sizeSrc,
                     TDEIO::filesize_t sizeDest,
                     time_t ctimeSrc,
                     time_t ctimeDest,
                     time_t mtimeSrc,
                     time_t mtimeDest,
                     bool _modal)
  : TQDialog ( parent, "TDEIO::RenameDialog" , _modal )
{
    d = new RenameDlgPrivate( );
    d->modal = _modal;
#if 0
    // Set "StaysOnTop", because this dialog is typically used in tdeio_uiserver,
    // i.e. in a separate process.
    // ####### This isn't the case anymore - remove?
#if !defined(TQ_WS_QWS) && !defined(TQ_WS_WIN) //FIXME(E): Implement for QT Embedded & win32
    if (d->modal)
        KWin::setState( winId(), NET::StaysOnTop );
#endif
#endif

    d->src = _src;
    d->dest = _dest;
    d->plugin = false;


    setCaption( _caption );

    d->bCancel = new KPushButton( KStdGuiItem::cancel(), this );
    connect(d->bCancel, TQ_SIGNAL(clicked()), this, TQ_SLOT(b0Pressed()));

    if ( ! (_mode & M_NORENAME ) ) {
        d->bRename = new TQPushButton( i18n( "&Rename" ), this );
        d->bRename->setEnabled(false);
        d->bSuggestNewName = new TQPushButton( i18n( "Suggest New &Name" ), this );
        connect(d->bSuggestNewName, TQ_SIGNAL(clicked()), this, TQ_SLOT(b8Pressed()));
        connect(d->bRename, TQ_SIGNAL(clicked()), this, TQ_SLOT(b1Pressed()));
    }

    if ( ( _mode & M_MULTI ) && ( _mode & M_SKIP ) ) {
        d->bSkip = new TQPushButton( i18n( "&Skip" ), this );
        connect(d->bSkip, TQ_SIGNAL(clicked()), this, TQ_SLOT(b2Pressed()));

        d->bAutoSkip = new TQPushButton( i18n( "&Auto Skip" ), this );
        connect(d->bAutoSkip, TQ_SIGNAL(clicked()), this, TQ_SLOT(b3Pressed()));
    }

    if ( _mode & M_OVERWRITE ) {
        d->bOverwrite = new TQPushButton( i18n( "&Overwrite" ), this );
        connect(d->bOverwrite, TQ_SIGNAL(clicked()), this, TQ_SLOT(b4Pressed()));

        if ( _mode & M_MULTI ) {
            d->bOverwriteAll = new TQPushButton( i18n( "O&verwrite All" ), this );
            connect(d->bOverwriteAll, TQ_SIGNAL(clicked()), this, TQ_SLOT(b5Pressed()));
        }
    }

    if ( _mode & M_RESUME ) {
        d->bResume = new TQPushButton( i18n( "&Resume" ), this );
        connect(d->bResume, TQ_SIGNAL(clicked()), this, TQ_SLOT(b6Pressed()));

        if ( _mode & M_MULTI )
        {
            d->bResumeAll = new TQPushButton( i18n( "R&esume All" ), this );
            connect(d->bResumeAll, TQ_SIGNAL(clicked()), this, TQ_SLOT(b7Pressed()));
        }
    }

    TQVBoxLayout* pLayout = new TQVBoxLayout( this, KDialog::marginHint(),
                                    KDialog::spacingHint() );
    pLayout->addStrut( 360 );	// makes dlg at least that wide

    // User tries to overwrite a file with itself ?
    if ( _mode & M_OVERWRITE_ITSELF ) {
        TQLabel *lb = new TQLabel( i18n( "This action would overwrite '%1' with itself.\n"
                                       "Please enter a new file name:" ).arg( KStringHandler::csqueeze( d->src.pathOrURL(),100 ) ), this );
        d->bRename->setText(i18n("C&ontinue"));
        pLayout->addWidget( lb );
    }
    else if ( _mode & M_OVERWRITE ) {

        // Figure out the mimetype and load one plugin
        // (This is the only mode that is handled by plugins)
        pluginHandling();
        TDETrader::OfferList plugin_offers;
        if( d->mimeSrc != KMimeType::defaultMimeType()   ){
            plugin_offers = TDETrader::self()->query(d->mimeSrc, "'RenameDlg/Plugin' in ServiceTypes");

        }else if(d->mimeDest != KMimeType::defaultMimeType() ) {
            plugin_offers = TDETrader::self()->query(d->mimeDest, "'RenameDlg/Plugin' in ServiceTypes");
        }
        if(!plugin_offers.isEmpty() ){
            kdDebug(7024) << "Offers" << endl;
            TDETrader::OfferList::ConstIterator it = plugin_offers.begin();
            TDETrader::OfferList::ConstIterator end = plugin_offers.end();
            for( ; it != end; ++it ){
                TQString libName = (*it)->library();
                if( libName.isEmpty() ){
                    kdDebug(7024) << "lib is empty" << endl;
                    continue;
                }
                KLibrary *lib = KLibLoader::self()->library(libName.local8Bit() );
                if(!lib) {
                    continue;
                }
                KLibFactory *factory = lib->factory();
                if(!factory){
                    lib->unload();
                    continue;
                }
                TQObject *obj = factory->create( this, (*it)->name().latin1() );
                if(!obj) {
                    lib->unload();
                    continue;
                }
                RenameDlgPlugin *plugin = static_cast<RenameDlgPlugin *>(static_cast<TQWidget*>(obj));
                if(!plugin ){
                    delete obj;
                    continue;
                }
                if( plugin->initialize( _mode, _src, _dest, d->mimeSrc,
                                        d->mimeDest, sizeSrc, sizeDest,
                                        ctimeSrc, ctimeDest,
                                        mtimeSrc, mtimeDest ) ) {
                    d->plugin = true;
                    pLayout->addWidget(plugin );
                    kdDebug(7024) << "RenameDlgPlugin" << endl;
                    break;
                } else {
                    delete obj;
                }
            }

        }

        if( !d->plugin ){
            // No plugin found, build default dialog
            TQGridLayout * gridLayout = new TQGridLayout( 0L, 9, 2, KDialog::marginHint(),
                                                        KDialog::spacingHint() );
            pLayout->addLayout(gridLayout);
            gridLayout->setColStretch(0,0);
            gridLayout->setColStretch(1,10);

            TQString sentence1;
            if (mtimeDest < mtimeSrc)
                sentence1 = i18n("An older item named '%1' already exists.");
            else if (mtimeDest == mtimeSrc)
                sentence1 = i18n("A similar file named '%1' already exists.");
            else
                sentence1 = i18n("A newer item named '%1' already exists.");

            TQLabel * lb1 = new KSqueezedTextLabel( sentence1.arg(d->dest.pathOrURL() ), this );
            gridLayout->addMultiCellWidget( lb1, 0, 0, 0, 1 ); // takes the complete first line

            lb1 = new TQLabel( this );
            lb1->setPixmap( KMimeType::pixmapForURL( d->dest ) );
            gridLayout->addMultiCellWidget( lb1, 1, 3, 0, 0 ); // takes the first column on rows 1-3

            int row = 1;
            if ( sizeDest != (TDEIO::filesize_t)-1 )
            {
                TQLabel * lb = new TQLabel( i18n("size %1").arg( TDEIO::convertSize(sizeDest) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;

            }
            if ( ctimeDest != (time_t)-1 )
            {
                TQDateTime dctime; dctime.setTime_t( ctimeDest );
                TQLabel * lb = new TQLabel( i18n("created on %1").arg( TDEGlobal::locale()->formatDateTime(dctime) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;
            }
            if ( mtimeDest != (time_t)-1 )
            {
                TQDateTime dmtime; dmtime.setTime_t( mtimeDest );
                TQLabel * lb = new TQLabel( i18n("modified on %1").arg( TDEGlobal::locale()->formatDateTime(dmtime) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;
            }

            if ( !d->src.isEmpty() )
            {
                // rows 1 to 3 are the details (size/ctime/mtime), row 4 is empty
                gridLayout->addRowSpacing( 4, 20 );

                TQLabel * lb2 = new KSqueezedTextLabel( i18n("The source file is '%1'").arg(d->src.pathOrURL()), this );
                gridLayout->addMultiCellWidget( lb2, 5, 5, 0, 1 ); // takes the complete first line

                lb2 = new TQLabel( this );
                lb2->setPixmap( KMimeType::pixmapForURL( d->src ) );
                gridLayout->addMultiCellWidget( lb2, 6, 8, 0, 0 ); // takes the first column on rows 6-8

                row = 6;

                if ( sizeSrc != (TDEIO::filesize_t)-1 )
                {
                    TQLabel * lb = new TQLabel( i18n("size %1").arg( TDEIO::convertSize(sizeSrc) ), this );
                    gridLayout->addWidget( lb, row, 1 );
                    row++;
                }
                if ( ctimeSrc != (time_t)-1 )
                {
                    TQDateTime dctime; dctime.setTime_t( ctimeSrc );
                    TQLabel * lb = new TQLabel( i18n("created on %1").arg( TDEGlobal::locale()->formatDateTime(dctime) ), this );
                    gridLayout->addWidget( lb, row, 1 );
                    row++;
                }
                if ( mtimeSrc != (time_t)-1 )
                {
                    TQDateTime dmtime; dmtime.setTime_t( mtimeSrc );
                    TQLabel * lb = new TQLabel( i18n("modified on %1").arg( TDEGlobal::locale()->formatDateTime(dmtime) ), this );
                    gridLayout->addWidget( lb, row, 1 );
                    row++;
                }
            }
        }
    }
    else
    {
        // This is the case where we don't want to allow overwriting, the existing
        // file must be preserved (e.g. when renaming).
        TQString sentence1;
        if (mtimeDest < mtimeSrc)
            sentence1 = i18n("An older item named '%1' already exists.");
        else if (mtimeDest == mtimeSrc)
            sentence1 = i18n("A similar file named '%1' already exists.");
        else
            sentence1 = i18n("A newer item named '%1' already exists.");

        TQLabel *lb = new KSqueezedTextLabel( sentence1.arg(d->dest.pathOrURL()), this );
        pLayout->addWidget(lb);
    }
    TQHBoxLayout* layout2 = new TQHBoxLayout();
    pLayout->addLayout( layout2 );

    d->m_pLineEdit = new TQLineEdit( this );
    layout2->addWidget( d->m_pLineEdit );
    TQString fileName = d->dest.fileName();
    d->m_pLineEdit->setText( TDEIO::decodeFileName( fileName ) );
    if ( d->bRename || d->bOverwrite )
        connect(d->m_pLineEdit, TQ_SIGNAL(textChanged(const TQString &)),
                TQ_SLOT(enableRenameButton(const TQString &)));
    if ( d->bSuggestNewName )
    {
        layout2->addWidget( d->bSuggestNewName );
        setTabOrder( d->m_pLineEdit, d->bSuggestNewName );
    }

    KSeparator* separator = new KSeparator( this );
    pLayout->addWidget( separator );

    TQHBoxLayout* layout = new TQHBoxLayout();
    pLayout->addLayout( layout );

    layout->addStretch(1);

    if ( d->bRename )
    {
        layout->addWidget( d->bRename );
        setTabOrder( d->bRename, d->bCancel );
    }
    if ( d->bSkip )
    {
        layout->addWidget( d->bSkip );
        setTabOrder( d->bSkip, d->bCancel );
    }
    if ( d->bAutoSkip )
    {
        layout->addWidget( d->bAutoSkip );
        setTabOrder( d->bAutoSkip, d->bCancel );
    }
    if ( d->bOverwrite )
    {
        layout->addWidget( d->bOverwrite );
        setTabOrder( d->bOverwrite, d->bCancel );
    }
    if ( d->bOverwriteAll )
    {
        layout->addWidget( d->bOverwriteAll );
        setTabOrder( d->bOverwriteAll, d->bCancel );
    }
    if ( d->bResume )
    {
        layout->addWidget( d->bResume );
        setTabOrder( d->bResume, d->bCancel );
    }
    if ( d->bResumeAll )
    {
        layout->addWidget( d->bResumeAll );
        setTabOrder( d->bResumeAll, d->bCancel );
    }

    d->bCancel->setDefault( true );
    layout->addWidget( d->bCancel );

    resize( sizeHint() );

    d->m_pLineEdit->setFocus();
}

RenameDlg::~RenameDlg()
{
  delete d;
  // no need to delete Pushbuttons,... qt will do this
}

void RenameDlg::enableRenameButton(const TQString &newDest)
{
  if ( newDest != TDEIO::decodeFileName( d->dest.fileName() ) && !newDest.isEmpty() )
  {
    d->bRename->setEnabled( true );
    d->bRename->setDefault( true );
    if ( d->bOverwrite )
        d->bOverwrite->setEnabled( false ); // prevent confusion (#83114)
  }
  else
  {
    d->bRename->setEnabled( false );
    if ( d->bOverwrite )
        d->bOverwrite->setEnabled( true );
  }
}

KURL RenameDlg::newDestURL()
{
  KURL newDest( d->dest );
  TQString fileName = d->m_pLineEdit->text();
  newDest.setFileName( TDEIO::encodeFileName( fileName ) );
  return newDest;
}

void RenameDlg::b0Pressed()
{
  done( 0 );
}

// Rename
void RenameDlg::b1Pressed()
{
  if ( d->m_pLineEdit->text().isEmpty() )
    return;

  KURL u = newDestURL();
  if ( !u.isValid() )
  {
    KMessageBox::error( this, i18n( "Malformed URL\n%1" ).arg( u.url() ) );
    return;
  }

  done( 1 );
}

TQString RenameDlg::suggestName(const KURL& baseURL, const TQString& oldName)
{
  TQString dotSuffix, suggestedName;
  TQString basename = oldName;

  int index = basename.find( '.' );
  if ( index != -1 ) {
    dotSuffix = basename.mid( index );
    basename.truncate( index );
  }

  int pos = basename.findRev( '_' );
  if(pos != -1 ){
    TQString tmp = basename.mid( pos+1 );
    bool ok;
    int number = tmp.toInt( &ok );
    if ( !ok ) {// ok there is no number
      suggestedName = basename + "1" + dotSuffix;
    }
    else {
     // yes there's already a number behind the _ so increment it by one
      basename.replace( pos+1, tmp.length(), TQString::number(number+1) );
      suggestedName = basename + dotSuffix;
    }
  }
  else // no underscore yet
    suggestedName = basename + "_1" + dotSuffix ;

  // Check if suggested name already exists
  bool exists = false;
  // TODO: network transparency. However, using NetAccess from a modal dialog
  // could be a problem, no? (given that it uses a modal widget itself....)
  if ( baseURL.isLocalFile() )
     exists = TQFileInfo( baseURL.path(+1) + suggestedName ).exists();

  if ( !exists )
    return suggestedName;
  else // already exists -> recurse
    return suggestName( baseURL, suggestedName );
}

// Propose button clicked
void RenameDlg::b8Pressed()
{
  /* no name to play with */
  if ( d->m_pLineEdit->text().isEmpty() )
    return;

  KURL destDirectory( d->dest );
  destDirectory.setPath( destDirectory.directory() );
  d->m_pLineEdit->setText( suggestName( destDirectory, d->m_pLineEdit->text() ) );
  return;
}

void RenameDlg::b2Pressed()
{
  done( 2 );
}

void RenameDlg::b3Pressed()
{
  done( 3 );
}

void RenameDlg::b4Pressed()
{
  done( 4 );
}

void RenameDlg::b5Pressed()
{
  done( 5 );
}

void RenameDlg::b6Pressed()
{
  done( 6 );
}

void RenameDlg::b7Pressed()
{
  done( 7 );
}

static TQString mime( const KURL& src )
{
  KMimeType::Ptr type = KMimeType::findByURL( src );
  //if( type->name() == KMimeType::defaultMimeType() ){ // ok no mimetype
    //    TQString ty = TDEIO::NetAccess::mimetype(d->src );
    // return ty;
    return type->name();
}

/** This will figure out the mimetypes and query for a plugin
 *  Loads it then and asks the plugin if it wants to do the job
 *  We'll take the first available mimetype
 *  The scanning for a mimetype will be done in 2 ways
 *
 */
void RenameDlg::pluginHandling()
{
  d->mimeSrc = mime( d->src );
  d->mimeDest = mime(d->dest );

  kdDebug(7024) << "Source Mimetype: "<< d->mimeSrc << endl;
  kdDebug(7024) << "Dest Mimetype: "<< d->mimeDest << endl;
}


RenameDlg_Result TDEIO::open_RenameDlg( const TQString & _caption,
                                      const TQString & _src, const TQString & _dest,
                                      RenameDlg_Mode _mode,
                                      TQString& _new,
                                      TDEIO::filesize_t sizeSrc,
                                      TDEIO::filesize_t sizeDest,
                                      time_t ctimeSrc,
                                      time_t ctimeDest,
                                      time_t mtimeSrc,
                                      time_t mtimeDest)
{
  Q_ASSERT(kapp);

  RenameDlg dlg( 0L, _caption, _src, _dest, _mode,
                 sizeSrc, sizeDest, ctimeSrc, ctimeDest, mtimeSrc, mtimeDest,
                 true /*modal*/ );
  int i = dlg.exec();
  _new = dlg.newDestURL().path();

  return (RenameDlg_Result)i;
}

#include "renamedlg.moc"





