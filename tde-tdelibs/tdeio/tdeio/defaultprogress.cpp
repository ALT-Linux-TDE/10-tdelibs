/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

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

#include <tqtimer.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqdatetime.h>
#include <tqcheckbox.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kstringhandler.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <klineedit.h>

#ifdef TQ_WS_X11
#include <twin.h>
#endif

#include "jobclasses.h"
#include "defaultprogress.h"

namespace TDEIO {

class DefaultProgress::DefaultProgressPrivate
{
public:
  bool keepOpenChecked;
  bool noCaptionYet;
  KPushButton *cancelClose;
  KPushButton *openFile;
  KPushButton *openLocation;
  TQCheckBox   *keepOpen;
  KURL        location;
  TQTime       startTime;
};

DefaultProgress::DefaultProgress( bool showNow )
  : ProgressBase( 0 ),
  m_iTotalSize(0), m_iTotalFiles(0), m_iTotalDirs(0),
  m_iProcessedSize(0), m_iProcessedDirs(0), m_iProcessedFiles(0)
{
  init();

  if ( showNow ) {
    show();
  }
}

DefaultProgress::DefaultProgress( TQWidget* parent, const char* /*name*/ )
  : ProgressBase( parent ),
  m_iTotalSize(0), m_iTotalFiles(0), m_iTotalDirs(0),
  m_iProcessedSize(0), m_iProcessedDirs(0), m_iProcessedFiles(0)
{
    init();
}

bool DefaultProgress::keepOpen() const
{
    return d->keepOpenChecked;
}

void DefaultProgress::init()
{
  d = new DefaultProgressPrivate;

#ifdef TQ_WS_X11 //FIXME(E): Remove once all the KWin::foo calls have been ported to QWS
  // Set a useful icon for this window!
  KWin::setIcons( winId(),
          TDEGlobal::iconLoader()->loadIcon( "document-save", TDEIcon::NoGroup, 32 ),
          TDEGlobal::iconLoader()->loadIcon( "document-save", TDEIcon::NoGroup, 16 ) );
#endif

  TQVBoxLayout *topLayout = new TQVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );
  topLayout->addStrut( 360 );   // makes dlg at least that wide

  TQGridLayout *grid = new TQGridLayout( 2, 3 );
  topLayout->addLayout(grid);
  grid->addColSpacing(1, KDialog::spacingHint());
  // filenames or action name
  grid->addWidget(new TQLabel(i18n("Source:"), this), 0, 0);

  sourceEdit = new KLineEdit(this);
  sourceEdit->setReadOnly(true);
  sourceEdit->setEnableSqueezedText(true);
  grid->addWidget(sourceEdit, 0, 2);

  destInvite = new TQLabel(i18n("Destination:"), this);
  grid->addWidget(destInvite, 1, 0);

  destEdit = new KLineEdit(this);
  destEdit->setReadOnly (true);
  destEdit->setEnableSqueezedText(true);
  grid->addWidget(destEdit, 1, 2);

  m_pProgressBar = new KProgress(this);
  topLayout->addWidget( m_pProgressBar );

  // processed info
  TQHBoxLayout *hBox = new TQHBoxLayout();
  topLayout->addLayout(hBox);

  sizeLabel = new TQLabel(this);
  hBox->addWidget(sizeLabel);

  resumeLabel = new TQLabel(this);
  hBox->addWidget(resumeLabel);

  progressLabel = new TQLabel( this );
/*  progressLabel->setSizePolicy( TQSizePolicy( TQSizePolicy::MinimumExpanding,
                                             TQSizePolicy::Preferred ) );*/
  progressLabel->setAlignment( TQLabel::AlignRight );
  hBox->addWidget( progressLabel );

  hBox = new TQHBoxLayout();
  topLayout->addLayout(hBox);

  speedLabel = new TQLabel(this);
  hBox->addWidget(speedLabel, 1);

  TQFrame *line = new TQFrame( this );
  line->setFrameShape( TQFrame::HLine );
  line->setFrameShadow( TQFrame::Sunken );
  topLayout->addWidget( line );

  d->keepOpen = new TQCheckBox( i18n("&Keep this window open after transfer is complete"), this);
  connect( d->keepOpen, TQ_SIGNAL( toggled(bool) ), TQ_SLOT( slotKeepOpenToggled(bool) ) );
  topLayout->addWidget(d->keepOpen);
  d->keepOpen->hide();

  hBox = new TQHBoxLayout();
  topLayout->addLayout(hBox);

  d->openFile = new KPushButton( i18n("Open &File"), this );
  connect( d->openFile, TQ_SIGNAL( clicked() ), TQ_SLOT( slotOpenFile() ) );
  hBox->addWidget( d->openFile );
  d->openFile->setEnabled(false);
  d->openFile->hide();

  d->openLocation = new KPushButton( i18n("Open &Destination"), this );
  connect( d->openLocation, TQ_SIGNAL( clicked() ), TQ_SLOT( slotOpenLocation() ) );
  hBox->addWidget( d->openLocation );
  d->openLocation->hide();

  hBox->addStretch(1);

  d->cancelClose = new KPushButton( KStdGuiItem::cancel(), this );
  connect( d->cancelClose, TQ_SIGNAL( clicked() ), TQ_SLOT( slotStop() ) );
  hBox->addWidget( d->cancelClose );

  resize( sizeHint() );
  setMaximumHeight(sizeHint().height());

  d->keepOpenChecked = false;
  d->noCaptionYet = true;
  setCaption(i18n("Progress Dialog")); // show something better than tdeio_uiserver
}

DefaultProgress::~DefaultProgress()
{
  delete d;
}

void DefaultProgress::slotTotalSize( TDEIO::Job*, TDEIO::filesize_t size )
{
  // size is measured in bytes
  if ( m_iTotalSize == size )
      return;
  m_iTotalSize = size;
  if (d->startTime.isNull())
    d->startTime.start();
}


void DefaultProgress::slotTotalFiles( TDEIO::Job*, unsigned long files )
{
  if ( m_iTotalFiles == files )
      return;
  m_iTotalFiles = files;
  showTotals();
}


void DefaultProgress::slotTotalDirs( TDEIO::Job*, unsigned long dirs )
{
  if ( m_iTotalDirs == dirs )
      return;
  m_iTotalDirs = dirs;
  showTotals();
}

void DefaultProgress::showTotals()
{
  // Show the totals in the progress label, if we still haven't
  // processed anything. This is useful when the stat'ing phase
  // of CopyJob takes a long time (e.g. over networks).
  if ( m_iProcessedFiles == 0 && m_iProcessedDirs == 0 )
  {
    TQString tmps;
    if ( m_iTotalDirs > 1 )
      // that we have a singular to translate looks weired but is only logical
      // xgettext: no-c-format
      tmps = i18n("%n folder", "%n folders", m_iTotalDirs) + "   ";
    // xgettext: no-c-format
    tmps += i18n("%n file", "%n files", m_iTotalFiles);
    progressLabel->setText( tmps );
  }
}

//static
TQString DefaultProgress::makePercentString( unsigned long percent,
                                            TDEIO::filesize_t totalSize,
                                            unsigned long totalFiles )
{
  if ( totalSize )
      return i18n( "%1 % of %2 " ).arg( TQString::number(percent) , TDEIO::convertSize( totalSize ) );
  else if ( totalFiles )
      return i18n( "%1 % of 1 file", "%1 % of %n files", totalFiles ).arg( percent );
  else
      return i18n( "%1 %" ).arg( percent );
}

void DefaultProgress::slotPercent( TDEIO::Job*, unsigned long percent )
{
  TQString caption = makePercentString( percent, m_iTotalSize, m_iTotalFiles );
  m_pProgressBar->setValue( percent );
  switch(mode) {
  case Copy:
    caption.append(i18n(" (Copying)"));
    break;
  case Move:
    caption.append(i18n(" (Moving)"));
    break;
  case Delete:
    caption.append(i18n(" (Deleting)"));
    break;
  case Create:
    caption.append(i18n(" (Creating)"));
    break;
  case Done:
    caption.append(i18n(" (Done)"));
    break;
  }

  setCaption( caption );
  d->noCaptionYet = false;
}


void DefaultProgress::slotInfoMessage( TDEIO::Job*, const TQString & msg )
{
  speedLabel->setText( msg );
  speedLabel->setAlignment( speedLabel->alignment() & ~TQt::WordBreak );
}


void DefaultProgress::slotProcessedSize( TDEIO::Job*, TDEIO::filesize_t bytes ) {
  if ( m_iProcessedSize == bytes )
    return;
  m_iProcessedSize = bytes;

  TQString tmp = i18n( "%1 of %2 complete")
                .arg( TDEIO::convertSize(bytes) )
                .arg( TDEIO::convertSize(m_iTotalSize));
  sizeLabel->setText( tmp );
}


void DefaultProgress::slotProcessedDirs( TDEIO::Job*, unsigned long dirs )
{
  if ( m_iProcessedDirs == dirs )
    return;
  m_iProcessedDirs = dirs;

  TQString tmps;
  tmps = i18n("%1 / %n folder", "%1 / %n folders", m_iTotalDirs).arg( m_iProcessedDirs );
  tmps += "   ";
  tmps += i18n("%1 / %n file", "%1 / %n files", m_iTotalFiles).arg( m_iProcessedFiles );
  progressLabel->setText( tmps );
}


void DefaultProgress::slotProcessedFiles( TDEIO::Job*, unsigned long files )
{
  if ( m_iProcessedFiles == files )
    return;
  m_iProcessedFiles = files;

  TQString tmps;
  if ( m_iTotalDirs > 1 ) {
    tmps = i18n("%1 / %n folder", "%1 / %n folders", m_iTotalDirs).arg( m_iProcessedDirs );
    tmps += "   ";
  }
  tmps += i18n("%1 / %n file", "%1 / %n files", m_iTotalFiles).arg( m_iProcessedFiles );
  progressLabel->setText( tmps );
}


void DefaultProgress::slotSpeed( TDEIO::Job*, unsigned long speed )
{
  if ( speed == 0 ) {
    speedLabel->setText( i18n( "Stalled") );
  } else {
    speedLabel->setText( i18n( "%1/s ( %2 remaining )").arg( TDEIO::convertSize( speed ))
        .arg( TDEIO::convertSeconds( TDEIO::calculateRemainingSeconds( m_iTotalSize, m_iProcessedSize, speed ))) );
  }
}


void DefaultProgress::slotCopying( TDEIO::Job*, const KURL& from, const KURL& to )
{
  if ( d->noCaptionYet ) {
    setCaption(i18n("Copy File(s) Progress"));
    d->noCaptionYet = false;
  }
  mode = Copy;
  sourceEdit->setText(from.prettyURL());
  setDestVisible( true );
  checkDestination( to );
  destEdit->setText(to.prettyURL());
}


void DefaultProgress::slotMoving( TDEIO::Job*, const KURL& from, const KURL& to )
{
  if ( d->noCaptionYet ) {
    setCaption(i18n("Move File(s) Progress"));
    d->noCaptionYet = false;
  }
  mode = Move;
  sourceEdit->setText(from.prettyURL());
  setDestVisible( true );
  checkDestination( to );
  destEdit->setText(to.prettyURL());
}


void DefaultProgress::slotCreatingDir( TDEIO::Job*, const KURL& dir )
{
  if ( d->noCaptionYet ) {
    setCaption(i18n("Creating Folder"));
    d->noCaptionYet = false;
  }
  mode = Create;
  sourceEdit->setText(dir.prettyURL());
  setDestVisible( false );
}


void DefaultProgress::slotDeleting( TDEIO::Job*, const KURL& url )
{
  if ( d->noCaptionYet ) {
    setCaption(i18n("Delete File(s) Progress"));
    d->noCaptionYet = false;
  }
  mode = Delete;
  sourceEdit->setText(url.prettyURL());
  setDestVisible( false );
}

void DefaultProgress::slotTransferring( TDEIO::Job*, const KURL& url )
{
  if ( d->noCaptionYet ) {
    setCaption(i18n("Loading Progress"));
    d->noCaptionYet = false;
  }
  sourceEdit->setText(url.prettyURL());
  setDestVisible( false );
}

void DefaultProgress::slotStating( TDEIO::Job*, const KURL& url )
{
  setCaption(i18n("Examining File Progress"));
  sourceEdit->setText(url.prettyURL());
  setDestVisible( false );
}

void DefaultProgress::slotMounting( TDEIO::Job*, const TQString & dev, const TQString & point )
{
  setCaption(i18n("Mounting %1").arg(dev));
  sourceEdit->setText(point);
  setDestVisible( false );
}

void DefaultProgress::slotUnmounting( TDEIO::Job*, const TQString & point )
{
  setCaption(i18n("Unmounting"));
  sourceEdit->setText(point);
  setDestVisible( false );
}

void DefaultProgress::slotCanResume( TDEIO::Job*, TDEIO::filesize_t resume )
{
  if ( resume ) {
    resumeLabel->setText( i18n("Resuming from %1").arg(TDEIO::number(resume)) );
  } else {
    resumeLabel->setText( i18n("Not resumable") );
  }
}

void DefaultProgress::setDestVisible( bool visible )
{
  // We can't hide the destInvite/destEdit labels,
  // because it screws up the TQGridLayout.
  if (visible)
  {
      destInvite->show();
      destEdit->show();

    destInvite->setText( i18n("Destination:") );
  }
  else
  {
      destInvite->hide();
      destEdit->hide();
    destInvite->setText( TQString::null );
    destEdit->setText( TQString::null );
  }
}

void DefaultProgress::slotClean() {
  if (d->keepOpenChecked) {
    mode = Done;
    slotPercent(0, 100);
    d->cancelClose->setGuiItem( KStdGuiItem::close() );
    d->openFile->setEnabled(true);
    slotProcessedSize(0, m_iTotalSize);
    d->keepOpen->setEnabled(false);
    if (!d->startTime.isNull()) {
      int s = d->startTime.elapsed();
      if (!s)
        s = 1;
      speedLabel->setText(i18n("%1/s (done)").arg(TDEIO::convertSize(1000 * m_iTotalSize / s)));
    }
    setOnlyClean(false);
  }
  else
    hide();
}

void DefaultProgress::slotKeepOpenToggled(bool keepopen)
{
  d->keepOpenChecked=keepopen;
}

void DefaultProgress::checkDestination(const KURL& dest) {
  bool ok = true;
  if ( dest.isLocalFile() ) {
      TQString path = dest.path( -1 );
      TQStringList tmpDirs = TDEGlobal::dirs()->resourceDirs( "tmp" );
      for ( TQStringList::Iterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it )
          if ( path.contains( *it ) )
              ok = false; // it's in the tmp resource
  }

  if ( ok ) {
    d->openFile->show();
    d->openLocation->show();
    d->keepOpen->show();
    d->location=dest;
  }
}

void DefaultProgress::slotOpenFile()
{
  TDEProcess proc;
  proc << "konqueror" << d->location.prettyURL();
  proc.start(TDEProcess::DontCare);
}

void DefaultProgress::slotOpenLocation()
{
  TDEProcess proc;
  d->location.setFileName("");
  proc << "konqueror" << d->location.prettyURL();
  proc.start(TDEProcess::DontCare);
}

void DefaultProgress::virtual_hook( int id, void* data )
{ ProgressBase::virtual_hook( id, data ); }

} /* namespace */

#include "defaultprogress.moc"
