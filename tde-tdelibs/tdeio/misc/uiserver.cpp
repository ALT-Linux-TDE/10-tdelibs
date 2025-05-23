/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>
                 2001 George Staikos <staikos@kde.org>

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

#include <tqregexp.h>
#include <tqheader.h>
#include <tqevent.h>

#include <ksqueezedtextlabel.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <dcopclient.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <tdemessagebox.h>
#include <tdesu/client.h>
#include <twin.h>
#include <kdialog.h>
#include <ksystemtray.h>
#include <tdepopupmenu.h>
#include <tdeaction.h>

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpopupmenu.h>
#include <tqheader.h>

#include "observer_stub.h"
#include "observer.h" // for static methods only
#include "tdeio/defaultprogress.h"
#include "tdeio/jobclasses.h"
#include "uiserver.h"
#include "passdlg.h"
#include "tdeio/renamedlg.h"
#include "tdeio/skipdlg.h"
#include "slavebase.h" // for QuestionYesNo etc.
#include <ksslinfodlg.h>
#include <ksslcertdlg.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>


// pointer for main instance of UIServer
UIServer* uiserver;

// ToolBar field IDs
enum { TOOL_CANCEL, TOOL_CONFIGURE };

// StatusBar field IDs
enum { ID_TOTAL_FILES = 1, ID_TOTAL_SIZE, ID_TOTAL_TIME, ID_TOTAL_SPEED };

//static
int UIServer::s_jobId = 0;

static const int defaultColumnWidth[] = { 70,  // SIZE_OPERATION
                                    160, // LOCAL_FILENAME
                                    40,  // RESUME
                                    60,  // COUNT
                                    30,  // PROGRESS
                                    65,  // TOTAL
                                    70,  // SPEED
                                    70,  // REMAINING_TIME
                                    450  // URL
};

class UIServerSystemTray:public KSystemTray
{
   public:
      UIServerSystemTray(UIServer* uis)
         :KSystemTray(uis)
      {
         TDEPopupMenu* pop= contextMenu();
         pop->insertItem(i18n("Settings..."), uis, TQ_SLOT(slotConfigure()));
         pop->insertItem(i18n("Remove"), uis, TQ_SLOT(slotRemoveSystemTrayIcon()));
         setPixmap(loadIcon("document-save"));
         //actionCollection()->action("file_quit")->setEnabled(true);
         KStdAction::quit(uis, TQ_SLOT(slotQuit()), actionCollection());
      }
};

class ProgressConfigDialog:public KDialogBase
{
   public:
      ProgressConfigDialog(TQWidget* parent);
      ~ProgressConfigDialog()  {}
      void setChecked(int i, bool on);
      bool isChecked(int i) const;
      friend class UIServer;
   private:
      TQCheckBox *m_showSystemTrayCb;
      TQCheckBox *m_keepOpenCb;
      TQCheckBox *m_toolBarCb;
      TQCheckBox *m_statusBarCb;
      TQCheckBox *m_headerCb;
      TQCheckBox *m_fixedWidthCb;
      TDEListView *m_columns;
      TQCheckListItem *(m_items[ListProgress::TB_MAX]);
};

ProgressConfigDialog::ProgressConfigDialog(TQWidget *parent)
:KDialogBase(KDialogBase::Plain,i18n("Configure Network Operation Window"),KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Cancel,
             KDialogBase::Ok, parent, "configprog", false)
{
   TQVBoxLayout *layout=new TQVBoxLayout(plainPage(),spacingHint());
   m_showSystemTrayCb=new TQCheckBox(i18n("Show system tray icon"), plainPage());
   m_keepOpenCb=new TQCheckBox(i18n("Keep network operation window always open"), plainPage());
   m_headerCb=new TQCheckBox(i18n("Show column headers"), plainPage());
   m_toolBarCb=new TQCheckBox(i18n("Show toolbar"), plainPage());
   m_statusBarCb=new TQCheckBox(i18n("Show statusbar"), plainPage());
   m_fixedWidthCb=new TQCheckBox(i18n("Column widths are user adjustable"), plainPage());
   TQLabel *label=new TQLabel(i18n("Show information:"), plainPage());
   m_columns=new TDEListView(plainPage());

   m_columns->addColumn("info");
   m_columns->setSorting(-1);
   m_columns->header()->hide();

   m_items[ListProgress::TB_ADDRESS]        =new TQCheckListItem(m_columns, i18n("URL"), TQCheckListItem::CheckBox);
   m_items[ListProgress::TB_REMAINING_TIME] =new TQCheckListItem(m_columns, i18n("Remaining Time", "Rem. Time"), TQCheckListItem::CheckBox);
   m_items[ListProgress::TB_SPEED]          =new TQCheckListItem(m_columns, i18n("Speed"), TQCheckListItem::CheckBox);
   m_items[ListProgress::TB_TOTAL]          =new TQCheckListItem(m_columns, i18n("Size"), TQCheckListItem::CheckBox);
   m_items[ListProgress::TB_PROGRESS]       =new TQCheckListItem(m_columns, i18n("%"), TQCheckListItem::CheckBox);
   m_items[ListProgress::TB_COUNT]          =new TQCheckListItem(m_columns, i18n("Count"), TQCheckListItem::CheckBox);
   m_items[ListProgress::TB_RESUME]         =new TQCheckListItem(m_columns, i18n("Resume", "Res."), TQCheckListItem::CheckBox);
   m_items[ListProgress::TB_LOCAL_FILENAME] =new TQCheckListItem(m_columns, i18n("Local Filename"), TQCheckListItem::CheckBox);
   m_items[ListProgress::TB_OPERATION]      =new TQCheckListItem(m_columns, i18n("Operation"), TQCheckListItem::CheckBox);

   layout->addWidget(m_showSystemTrayCb);
   layout->addWidget(m_keepOpenCb);
   layout->addWidget(m_headerCb);
   layout->addWidget(m_toolBarCb);
   layout->addWidget(m_statusBarCb);
   layout->addWidget(m_fixedWidthCb);
   layout->addWidget(label);
   layout->addWidget(m_columns);
}

void ProgressConfigDialog::setChecked(int i, bool on)
{
   if (i>=ListProgress::TB_MAX)
      return;
   m_items[i]->setOn(on);
}

bool ProgressConfigDialog::isChecked(int i) const
{
   if (i>=ListProgress::TB_MAX)
      return false;
   return m_items[i]->isOn();
}

ProgressItem::ProgressItem( ListProgress* view, TQListViewItem *after, TQCString app_id, int job_id,
                            bool showDefault )
  : TQListViewItem( view, after ) {

  listProgress = view;

  m_iTotalSize = 0;
  m_iTotalFiles = 0;
  m_iProcessedSize = 0;
  m_iProcessedFiles = 0;
  m_iSpeed = 0;

  m_sAppId = app_id;
  m_iJobId = job_id;
  m_visible = true;
  m_defaultProgressVisible = showDefault;

  // create dialog, but don't show it
  defaultProgress = new TDEIO::DefaultProgress( false );
  defaultProgress->setOnlyClean( true );
  connect ( defaultProgress, TQ_SIGNAL( stopped() ), this, TQ_SLOT( slotCanceled() ) );
  connect ( &m_showTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT(slotShowDefaultProgress()) );

  if ( showDefault ) {
    m_showTimer.start( 500, true );
  }
}

bool ProgressItem::keepOpen() const
{
    return defaultProgress->keepOpen();
}

void ProgressItem::finished()
{
    defaultProgress->finished();
}

ProgressItem::~ProgressItem() {
    delete defaultProgress;
}


void ProgressItem::setTotalSize( TDEIO::filesize_t size ) {
  m_iTotalSize = size;

  // It's already in the % column...
  //setText( listProgress->lv_total, TDEIO::convertSize( m_iTotalSize ) );

  defaultProgress->slotTotalSize( 0, m_iTotalSize );
}


void ProgressItem::setTotalFiles( unsigned long files ) {
  m_iTotalFiles = files;

  defaultProgress->slotTotalFiles( 0, m_iTotalFiles );
}


void ProgressItem::setTotalDirs( unsigned long dirs ) {
  defaultProgress->slotTotalDirs( 0, dirs );
}


void ProgressItem::setProcessedSize( TDEIO::filesize_t size ) {
  m_iProcessedSize = size;

  setText( ListProgress::TB_TOTAL, TDEIO::convertSize( size ) );

  defaultProgress->slotProcessedSize( 0, size );
}


void ProgressItem::setProcessedFiles( unsigned long files ) {
  m_iProcessedFiles = files;

  TQString tmps = i18n("%1 / %2").arg( m_iProcessedFiles ).arg( m_iTotalFiles );
  setText( ListProgress::TB_COUNT, tmps );

  defaultProgress->slotProcessedFiles( 0, m_iProcessedFiles );
}


void ProgressItem::setProcessedDirs( unsigned long dirs ) {
  defaultProgress->slotProcessedDirs( 0, dirs );
}


void ProgressItem::setPercent( unsigned long percent ) {
  const TQString tmps = TDEIO::DefaultProgress::makePercentString( percent, m_iTotalSize, m_iTotalFiles );
  setText( ListProgress::TB_PROGRESS, tmps );

  defaultProgress->slotPercent( 0, percent );
}

void ProgressItem::setInfoMessage( const TQString & msg ) {
  TQString plainTextMsg(msg);
  plainTextMsg.replace( TQRegExp( "</?b>" ), TQString() );
  plainTextMsg.replace( TQRegExp( "<img.*>" ), TQString() );
  setText( ListProgress::TB_PROGRESS, plainTextMsg );

  defaultProgress->slotInfoMessage( 0, msg );
}

void ProgressItem::setSpeed( unsigned long bytes_per_second ) {
  m_iSpeed = bytes_per_second;
  m_remainingSeconds = TDEIO::calculateRemainingSeconds( m_iTotalSize, m_iProcessedSize, m_iSpeed );

  TQString tmps, tmps2;
  if ( m_iSpeed == 0 ) {
    tmps = i18n( "Stalled");
    tmps2 = tmps;
  } else {
    tmps = i18n( "%1/s").arg( TDEIO::convertSize( m_iSpeed ));
    tmps2 = TDEIO::convertSeconds( m_remainingSeconds );
  }
  setText( ListProgress::TB_SPEED, tmps );
  setText( ListProgress::TB_REMAINING_TIME, tmps2 );

  defaultProgress->slotSpeed( 0, m_iSpeed );
}


void ProgressItem::setCopying( const KURL& from, const KURL& to ) {
   setText( ListProgress::TB_OPERATION, i18n("Copying") );
   setText( ListProgress::TB_ADDRESS, from.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, to.fileName() );

  defaultProgress->slotCopying( 0, from, to );
}


void ProgressItem::setMoving( const KURL& from, const KURL& to ) {
   setText( ListProgress::TB_OPERATION, i18n("Moving") );
   setText( ListProgress::TB_ADDRESS, from.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, to.fileName() );

  defaultProgress->slotMoving( 0, from, to );
}


void ProgressItem::setCreatingDir( const KURL& dir ) {
   setText( ListProgress::TB_OPERATION, i18n("Creating") );
   setText( ListProgress::TB_ADDRESS, dir.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, dir.fileName() );

  defaultProgress->slotCreatingDir( 0, dir );
}


void ProgressItem::setDeleting( const KURL& url ) {
   setText( ListProgress::TB_OPERATION, i18n("Deleting") );
   setText( ListProgress::TB_ADDRESS, url.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, url.fileName() );

  defaultProgress->slotDeleting( 0, url );
}

void ProgressItem::setTransferring( const KURL& url ) {
   setText( ListProgress::TB_OPERATION, i18n("Loading") );
   setText( ListProgress::TB_ADDRESS, url.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, url.fileName() );

  defaultProgress->slotTransferring( 0, url );
}

void ProgressItem::setText(ListProgress::ListProgressFields field, const TQString& text)
{
  if (listProgress->m_lpcc[field].enabled)
  {
     TQString t=text;
     if ((field==ListProgress::TB_ADDRESS) && (listProgress->m_fixedColumnWidths))
//     if (((field==ListProgress::TB_LOCAL_FILENAME) || (field==ListProgress::TB_ADDRESS)) && (listProgress->m_fixedColumnWidths))
     {
        m_fullLengthAddress=text;
        listProgress->m_squeezer->resize(listProgress->columnWidth(listProgress->m_lpcc[field].index),50);
        listProgress->m_squeezer->setText(t);
        t=listProgress->m_squeezer->text();
     }
     TQListViewItem::setText(listProgress->m_lpcc[field].index,t);
  }
}

void ProgressItem::setStating( const KURL& url ) {
   setText( ListProgress::TB_OPERATION, i18n("Examining") );
   setText( ListProgress::TB_ADDRESS, url.url() );
   setText( ListProgress::TB_LOCAL_FILENAME, url.fileName() );

  defaultProgress->slotStating( 0, url );
}

void ProgressItem::setMounting( const TQString& dev, const TQString & point ) {
   setText( ListProgress::TB_OPERATION, i18n("Mounting") );
   setText( ListProgress::TB_ADDRESS, point ); // ?
   setText( ListProgress::TB_LOCAL_FILENAME, dev ); // ?

  defaultProgress->slotMounting( 0, dev, point );
}

void ProgressItem::setUnmounting( const TQString & point ) {
   setText( ListProgress::TB_OPERATION, i18n("Unmounting") );
   setText( ListProgress::TB_ADDRESS, point ); // ?
   setText( ListProgress::TB_LOCAL_FILENAME, "" ); // ?

  defaultProgress->slotUnmounting( 0, point );
}

void ProgressItem::setCanResume( TDEIO::filesize_t offset ) {
  /*
  TQString tmps;
  // set canResume
  if ( _resume ) {
    tmps = i18n("Yes");
  } else {
    tmps = i18n("No");
  }
  setText( listProgress->lv_resume, tmps );
  */
  defaultProgress->slotCanResume( 0, offset );
}


void ProgressItem::slotCanceled() {
  emit jobCanceled( this );
}

// Called 0.5s after the job has been started
void ProgressItem::slotShowDefaultProgress() {
  if (defaultProgress)
  {
    if ( m_visible && m_defaultProgressVisible )
      defaultProgress->show();
    else
      defaultProgress->hide();
  }
}

void ProgressItem::slotToggleDefaultProgress() {
  setDefaultProgressVisible( !m_defaultProgressVisible );
}

// Called when a rename or skip dialog pops up
// We want to prevent someone from killing the job in the uiserver then
void ProgressItem::setVisible( bool visible ) {
  if ( m_visible != visible )
  {
    m_visible = visible;
    updateVisibility();
  }
}

// Can be toggled by the user
void ProgressItem::setDefaultProgressVisible( bool visible ) {
  if ( m_defaultProgressVisible != visible )
  {
    m_defaultProgressVisible = visible;
    updateVisibility();
  }
}

// Update according to state
void ProgressItem::updateVisibility()
{
  if (defaultProgress)
  {
    if ( m_visible && m_defaultProgressVisible )
    {
      m_showTimer.start(250, true); // Show delayed
    }
    else
    {
      m_showTimer.stop();
      defaultProgress->hide();
    }
  }
}


//-----------------------------------------------------------------------------
ListProgress::ListProgress (TQWidget *parent, const char *name)
: TDEListView (parent, name)
{

  // enable selection of more than one item
  setMultiSelection( true );

  setAllColumnsShowFocus( true );

  m_lpcc[TB_OPERATION].title=i18n("Operation");
  m_lpcc[TB_LOCAL_FILENAME].title=i18n("Local Filename");
  m_lpcc[TB_RESUME].title=i18n("Resume", "Res.");
  m_lpcc[TB_COUNT].title=i18n("Count");
  m_lpcc[TB_PROGRESS].title=i18n("%");
  m_lpcc[TB_TOTAL].title=i18n("Size");
  m_lpcc[TB_SPEED].title=i18n("Speed");
  m_lpcc[TB_REMAINING_TIME].title=i18n("Remaining Time", "Rem. Time");
  m_lpcc[TB_ADDRESS].title=i18n("URL");
  readSettings();

  applySettings();

  //used for squeezing the text in local file name and url
  m_squeezer=new KSqueezedTextLabel(this);
  m_squeezer->hide();
  connect(header(),TQ_SIGNAL(sizeChange(int,int,int)),this,TQ_SLOT(columnWidthChanged(int)));
}


ListProgress::~ListProgress() {
}

void ListProgress::applySettings()
{
  int iEnabledCols=0;

  // Update listcolumns to show
  for (int i=0; i<TB_MAX; i++)
  {
    if ( !m_lpcc[i].enabled )
      continue;

    iEnabledCols++;

    // Add new or reuse existing column
    if ( iEnabledCols > columns() )
      m_lpcc[i].index=addColumn(m_lpcc[i].title, m_fixedColumnWidths?m_lpcc[i].width:-1);
    else
    {
      m_lpcc[i].index = iEnabledCols - 1;
      setColumnText(m_lpcc[i].index, m_lpcc[i].title);
    }

    setColumnWidth(m_lpcc[i].index, m_lpcc[i].width); //yes, this is required here, alexxx
    if (m_fixedColumnWidths)
        setColumnWidthMode(m_lpcc[i].index, Manual);
  }

  // Remove unused columns. However we must keep one column left
  // Otherwise the listview will be emptied
  while( iEnabledCols < columns() && columns() > 1 )
     removeColumn( columns() - 1 );

  if ( columns() == 0 )
    addColumn( "" );

  if ( !m_showHeader || iEnabledCols == 0 )
    header()->hide();
  else
    header()->show();
}

void ListProgress::readSettings() {
  TDEConfig config("uiserverrc");

  // read listview geometry properties
  config.setGroup( "ProgressList" );
  for ( int i = 0; i < TB_MAX; i++ ) {
     TQString tmps="Col"+TQString::number(i);
     m_lpcc[i].width=config.readNumEntry( tmps, 0);
     if (m_lpcc[i].width==0) m_lpcc[i].width=defaultColumnWidth[i];

     tmps="Enabled"+TQString::number(i);
     m_lpcc[i].enabled=config.readBoolEntry(tmps,true);
  }
  m_showHeader=config.readBoolEntry("ShowListHeader",true);
  m_fixedColumnWidths=config.readBoolEntry("FixedColumnWidths",false);

  m_lpcc[TB_RESUME].enabled=false;
}

void ListProgress::columnWidthChanged(int column)
{
   //resqueeze if necessary
   if ((m_lpcc[TB_ADDRESS].enabled) && (column==m_lpcc[TB_ADDRESS].index))
   {
      for (TQListViewItem* lvi=firstChild(); lvi!=0; lvi=lvi->nextSibling())
      {
         ProgressItem *pi=(ProgressItem*)lvi;
         pi->setText(TB_ADDRESS,pi->fullLengthAddress());
      }
   }
   writeSettings();
}

void ListProgress::writeSettings() {
   TDEConfig config("uiserverrc");

   // write listview geometry properties
   config.setGroup( "ProgressList" );
   for ( int i = 0; i < TB_MAX; i++ ) {
      if (!m_lpcc[i].enabled) {
         TQString tmps= "Enabled" + TQString::number(i);
         config.writeEntry( tmps, false );
         continue;
      }
      m_lpcc[i].width=columnWidth(m_lpcc[i].index);
      TQString tmps="Col"+TQString::number(i);
      config.writeEntry( tmps, m_lpcc[i].width);
   }
   config.writeEntry("ShowListHeader", m_showHeader);
   config.writeEntry("FixedColumnWidths", m_fixedColumnWidths);
   config.sync();
}


//------------------------------------------------------------


UIServer::UIServer()
:TDEMainWindow(0, "")
,DCOPObject("UIServer")
,m_shuttingDown(false)
,m_configDialog(0)
,m_contextMenu(0)
,m_systemTray(0)
{

  readSettings();

  // setup toolbar
  toolBar()->insertButton("edit-delete", TOOL_CANCEL,
                          TQ_SIGNAL(clicked()), this,
                          TQ_SLOT(slotCancelCurrent()), FALSE, i18n("Cancel"));
  toolBar()->insertButton("configure", TOOL_CONFIGURE,
                          TQ_SIGNAL(clicked()), this,
                          TQ_SLOT(slotConfigure()), true, i18n("Settings..."));

  toolBar()->setBarPos( TDEToolBar::Left );

  // setup statusbar
  statusBar()->insertItem( i18n(" Files: %1 ").arg( 0 ), ID_TOTAL_FILES);
  statusBar()->insertItem( i18n("Remaining Size", " Rem. Size: %1 kB ").arg( "0" ), ID_TOTAL_SIZE);
  statusBar()->insertItem( i18n("Remaining Time", " Rem. Time: 00:00:00 "), ID_TOTAL_TIME);
  statusBar()->insertItem( i18n(" %1 kB/s ").arg("0"), ID_TOTAL_SPEED);

  // setup listview
  listProgress = new ListProgress( this, "progresslist" );

  setCentralWidget( listProgress );

  connect( listProgress, TQ_SIGNAL( selectionChanged() ),
           TQ_SLOT( slotSelection() ) );
  connect( listProgress, TQ_SIGNAL( executed( TQListViewItem* ) ),
           TQ_SLOT( slotToggleDefaultProgress( TQListViewItem* ) ) );
  connect( listProgress, TQ_SIGNAL( contextMenu( TDEListView*, TQListViewItem *, const TQPoint &)),
           TQ_SLOT(slotShowContextMenu(TDEListView*, TQListViewItem *, const TQPoint&)));


  // setup animation timer
  updateTimer = new TQTimer( this );
  connect( updateTimer, TQ_SIGNAL( timeout() ),
           TQ_SLOT( slotUpdate() ) );
  m_bUpdateNewJob=false;

  setCaption(i18n("Progress Dialog"));
  setMinimumSize( 150, 50 );
  resize( m_initWidth, m_initHeight);

  applySettings();

/*  if ((m_bShowList) && (m_keepListOpen))
  {
     cerr<<"show() !"<<endl;
     show();
  }
  else*/
  hide();
}

UIServer::~UIServer() {
  updateTimer->stop();
}

void UIServer::applySettings()
{
  if ((m_showSystemTray) && (m_systemTray==0))
  {
     m_systemTray=new UIServerSystemTray(this);
     m_systemTray->show();
  }
  else if ((m_showSystemTray==false) && (m_systemTray!=0))
  {
     delete m_systemTray;
     m_systemTray=0;
  }

  if (m_showStatusBar==false)
     statusBar()->hide();
  else
     statusBar()->show();
  if (m_showToolBar==false)
     toolBar()->hide();
  else
     toolBar()->show();
}

void UIServer::slotShowContextMenu(TDEListView*, TQListViewItem* item, const TQPoint& pos)
{
   if (m_contextMenu==0)
   {
      m_contextMenu=new TQPopupMenu(this);
      m_idCancelItem = m_contextMenu->insertItem(i18n("Cancel Job"), this, TQ_SLOT(slotCancelCurrent()));
//      m_contextMenu->insertItem(i18n("Toggle Progress"), this, TQ_SLOT(slotToggleDefaultProgress()));
      m_contextMenu->insertSeparator();
      m_contextMenu->insertItem(i18n("Settings..."), this, TQ_SLOT(slotConfigure()));
   }
   if ( item )
       item->setSelected( true );
   bool enabled = false;
   TQListViewItemIterator it( listProgress );
   for ( ; it.current(); ++it ) {
     if ( it.current()->isSelected() ) {
       enabled = true;
       break;
     }
   }
   m_contextMenu->setItemEnabled( m_idCancelItem, enabled);

   m_contextMenu->popup(pos);
}

void UIServer::slotRemoveSystemTrayIcon()
{
   m_showSystemTray=false;
   applySettings();
   writeSettings();
}

void UIServer::slotConfigure()
{
   if (m_configDialog==0)
   {
      m_configDialog=new ProgressConfigDialog(0);
//      connect(m_configDialog,TQ_SIGNAL(cancelClicked()), this, TQ_SLOT(slotCancelConfig()));
      connect(m_configDialog,TQ_SIGNAL(okClicked()), this, TQ_SLOT(slotApplyConfig()));
      connect(m_configDialog,TQ_SIGNAL(applyClicked()), this, TQ_SLOT(slotApplyConfig()));
   }
   m_configDialog->m_showSystemTrayCb->setChecked(m_showSystemTray);
   m_configDialog->m_keepOpenCb->setChecked(m_keepListOpen);
   m_configDialog->m_toolBarCb->setChecked(m_showToolBar);
   m_configDialog->m_statusBarCb->setChecked(m_showStatusBar);
   m_configDialog->m_headerCb->setChecked(listProgress->m_showHeader);
   m_configDialog->m_fixedWidthCb->setChecked(listProgress->m_fixedColumnWidths);
   for (int i=0; i<ListProgress::TB_MAX; i++)
   {
      m_configDialog->setChecked(i, listProgress->m_lpcc[i].enabled);
   }
   m_configDialog->show();
}

void UIServer::slotApplyConfig()
{
   m_showSystemTray=m_configDialog->m_showSystemTrayCb->isChecked();
   m_keepListOpen=m_configDialog->m_keepOpenCb->isChecked();
   m_showToolBar=m_configDialog->m_toolBarCb->isChecked();
   m_showStatusBar=m_configDialog->m_statusBarCb->isChecked();
   listProgress->m_showHeader=m_configDialog->m_headerCb->isChecked();
   listProgress->m_fixedColumnWidths=m_configDialog->m_fixedWidthCb->isChecked();
   for (int i=0; i<ListProgress::TB_MAX; i++)
      listProgress->m_lpcc[i].enabled=m_configDialog->isChecked(i);


   applySettings();
   listProgress->applySettings();
   writeSettings();
   listProgress->writeSettings();
}

int UIServer::newJob( TQCString observerAppId, bool showProgress )
{
  kdDebug(7024) << "UIServer::newJob observerAppId=" << observerAppId << ". "
            << "Giving id=" << s_jobId+1 << endl;

  TQListViewItemIterator it( listProgress );
  for ( ; it.current(); ++it ) {
    if ( it.current()->itemBelow() == 0L ) { // this will find the end of list
      break;
    }
  }

  // increment counter
  s_jobId++;

  bool show = !m_bShowList && showProgress;

  ProgressItem *item = new ProgressItem( listProgress, it.current(), observerAppId, s_jobId, show );
  connect( item, TQ_SIGNAL( jobCanceled( ProgressItem* ) ),
           TQ_SLOT( slotJobCanceled( ProgressItem* ) ) );

  if ( m_bShowList && !updateTimer->isActive() )
    updateTimer->start( 1000 );

  m_bUpdateNewJob=true;

  return s_jobId;
}


ProgressItem* UIServer::findItem( int id )
{
  TQListViewItemIterator it( listProgress );

  ProgressItem *item;

  for ( ; it.current(); ++it ) {
    item = (ProgressItem*) it.current();
    if ( item->jobId() == id ) {
      return item;
    }
  }

  return 0L;
}


void UIServer::setItemVisible( ProgressItem * item, bool visible )
{
  item->setVisible( visible );
  // Check if we were the last one to be visible
  // or the first one -> hide/show the list in that case
  // (Note that the user could have hidden the listview by hand yet, no time)
  if ( m_bShowList ) {
      m_bUpdateNewJob = true;
      slotUpdate();
  }
}

// Called by Observer when opening a skip or rename dialog
void UIServer::setJobVisible( int id, bool visible )
{
  kdDebug(7024) << "UIServer::setJobVisible id=" << id << " visible=" << visible << endl;
  ProgressItem *item = findItem( id );
  Q_ASSERT( item );
  if ( item )
      setItemVisible( item, visible );
}

void UIServer::jobFinished( int id )
{
  kdDebug(7024) << "UIServer::jobFinished id=" << id << endl;
  ProgressItem *item = findItem( id );

  // remove item from the list and delete the corresponding defaultprogress
  if ( item ) {
    if ( item->keepOpen() )
      item->finished();
    else
    delete item;
  }
}


void UIServer::totalSize( int id, unsigned long size )
{ totalSize64(id, size); }

void UIServer::totalSize64( int id, TDEIO::filesize_t size )
{
//  kdDebug(7024) << "UIServer::totalSize " << id << " " << TDEIO::number(size) << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalSize( size );
  }
}

void UIServer::totalFiles( int id, unsigned long files )
{
  kdDebug(7024) << "UIServer::totalFiles " << id << " " << (unsigned int) files << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalFiles( files );
  }
}

void UIServer::totalDirs( int id, unsigned long dirs )
{
  kdDebug(7024) << "UIServer::totalDirs " << id << " " << (unsigned int) dirs << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTotalDirs( dirs );
  }
}

void UIServer::processedSize( int id, unsigned long size )
{ processedSize64(id, size); }

void UIServer::processedSize64( int id, TDEIO::filesize_t size )
{
  //kdDebug(7024) << "UIServer::processedSize " << id << " " << TDEIO::number(size) << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedSize( size );
  }
}

void UIServer::processedFiles( int id, unsigned long files )
{
  //kdDebug(7024) << "UIServer::processedFiles " << id << " " << (unsigned int) files << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedFiles( files );
  }
}

void UIServer::processedDirs( int id, unsigned long dirs )
{
  kdDebug(7024) << "UIServer::processedDirs " << id << " " << (unsigned int) dirs << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setProcessedDirs( dirs );
  }
}

void UIServer::percent( int id, unsigned long ipercent )
{
  //kdDebug(7024) << "UIServer::percent " << id << " " << (unsigned int) ipercent << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setPercent( ipercent );
  }
}

void UIServer::speed( int id, unsigned long bytes_per_second )
{
  //kdDebug(7024) << "UIServer::speed " << id << " " << (unsigned int) bytes_per_second << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setSpeed( bytes_per_second );
  }
}

void UIServer::infoMessage( int id, const TQString & msg )
{
  //kdDebug(7024) << "UIServer::infoMessage " << id << " " << msg << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setInfoMessage( msg );
  }
}

void UIServer::canResume( int id, unsigned long offset )
{ canResume64(id, offset); }

void UIServer::canResume64( int id, TDEIO::filesize_t offset )
{
  //kdDebug(7024) << "UIServer::canResume " << id << " " << offset << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCanResume( offset );
  }
}

void UIServer::copying( int id, KURL from, KURL to )
{
  //kdDebug(7024) << "UIServer::copying " << id << " " << from.url() << "  " << to.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCopying( from, to );
  }
}

void UIServer::moving( int id, KURL from, KURL to )
{
  //kdDebug(7024) << "UIServer::moving " << id << " " << from.url() << "  " << to.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setMoving( from, to );
  }
}

void UIServer::deleting( int id, KURL url )
{
  //kdDebug(7024) << "UIServer::deleting " << id << " " << url.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setDeleting( url );
  }
}

void UIServer::transferring( int id, KURL url )
{
  //kdDebug(7024) << "UIServer::transferring " << id << " " << url.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setTransferring( url );
  }
}

void UIServer::creatingDir( int id, KURL dir )
{
  kdDebug(7024) << "UIServer::creatingDir " << id << " " << dir.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setCreatingDir( dir );
  }
}

void UIServer::stating( int id, KURL url )
{
  kdDebug(7024) << "UIServer::stating " << id << " " << url.url() << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setStating( url );
  }
}

void UIServer::mounting( int id, TQString dev, TQString point )
{
  kdDebug(7024) << "UIServer::mounting " << id << " " << dev << " " << point << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setMounting( dev, point );
  }
}

void UIServer::unmounting( int id, TQString point )
{
  kdDebug(7024) << "UIServer::unmounting " << id << " " << point << endl;

  ProgressItem *item = findItem( id );
  if ( item ) {
    item->setUnmounting( point );
  }
}

void UIServer::killJob( TQCString observerAppId, int progressId )
{
    // Contact the object "TDEIO::Observer" in the application <appId>
    Observer_stub observer( observerAppId, "TDEIO::Observer" );
    // Tell it to kill the job
    observer.killJob( progressId );
}

void UIServer::slotJobCanceled( ProgressItem *item ) {
  kdDebug(7024) << "UIServer::slotJobCanceled appid=" << item->appId() << " jobid=" << item->jobId() << endl;
  // kill the corresponding job
  killJob( item->appId(), item->jobId() );

  // TDEIO::Job, when killed, should call back jobFinished(), but we can't
  // really rely on that - the app may have crashed
  delete item;
}


void UIServer::slotQuit()
{
  m_shuttingDown = true;
  kapp->quit();
}

void UIServer::slotUpdate() {
  // don't do anything if we don't have any inserted progress item
  // or if they're all hidden
   TQListViewItemIterator lvit( listProgress );
   bool visible = false;
   for ( ; lvit.current(); ++lvit )
      if ( ((ProgressItem*)lvit.current())->isVisible() ) {
         visible = true;
         break;
      }

   if ( !visible || !m_bShowList ) {
      if (!m_keepListOpen) hide();
      updateTimer->stop();
      return;
   }

  // Calling show() is conditional, so that users can close the window
  // and it only pops up back when a new job is started
  if (m_bUpdateNewJob)
  {
    m_bUpdateNewJob=false;
    show();

    // Make sure we'll be called back
    if ( m_bShowList && !updateTimer->isActive() )
      updateTimer->start( 1000 );
  }

  int iTotalFiles = 0;
  TDEIO::filesize_t iTotalSize = 0;
  int iTotalSpeed = 0;
  unsigned int totalRemTime = 0; // in seconds

  ProgressItem *item;

  // count totals for statusbar
  TQListViewItemIterator it( listProgress );

  for ( ; it.current(); ++it ) {
    item = (ProgressItem*) it.current();
    if ( item->totalSize() != 0 ) {
      iTotalSize += ( item->totalSize() - item->processedSize() );
    }
    iTotalFiles += ( item->totalFiles() - item->processedFiles() );
    iTotalSpeed += item->speed();

    if ( item->remainingSeconds() > totalRemTime ) {
      totalRemTime = item->remainingSeconds();
    }
  }

  // update statusbar
  statusBar()->changeItem( i18n( " Files: %1 ").arg( iTotalFiles ), ID_TOTAL_FILES);
  statusBar()->changeItem( i18n( "Remaining Size", " Rem. Size: %1 ").arg( TDEIO::convertSize( iTotalSize ) ),
                           ID_TOTAL_SIZE);
  statusBar()->changeItem( i18n( "Remaining Time", " Rem. Time: %1 ").arg( TDEIO::convertSeconds( totalRemTime ) ),
                           ID_TOTAL_TIME);
  statusBar()->changeItem( i18n( " %1/s ").arg( TDEIO::convertSize( iTotalSpeed ) ),
                           ID_TOTAL_SPEED);

}

void UIServer::setListMode( bool list )
{
  m_bShowList = list;
  TQListViewItemIterator it( listProgress );
  for ( ; it.current(); ++it ) {
    // When going to list mode -> hide all progress dialogs
    // When going back to separate dialogs -> show them all
    ((ProgressItem*) it.current())->setDefaultProgressVisible( !list );
  }

  if (m_bShowList)
  {
    show();
    updateTimer->start( 1000 );
  }
  else
  {
    hide();
    updateTimer->stop();
  }
}

void UIServer::slotToggleDefaultProgress( TQListViewItem *item ) {
  ((ProgressItem*) item )->slotToggleDefaultProgress();
}


void UIServer::slotSelection() {
  TQListViewItemIterator it( listProgress );

  for ( ; it.current(); ++it ) {
    if ( it.current()->isSelected() ) {
      toolBar()->setItemEnabled( TOOL_CANCEL, TRUE);
      return;
    }
  }
  toolBar()->setItemEnabled( TOOL_CANCEL, FALSE);
}

// This code is deprecated, slaves go to Observer::openPassDlg now,
// but this is kept for compat (DCOP calls to tdeio_uiserver).
TQByteArray UIServer::openPassDlg( const TDEIO::AuthInfo &info )
{
    kdDebug(7024) << "UIServer::openPassDlg: User= " << info.username
                  << ", Msg= " << info.prompt << endl;
    TDEIO::AuthInfo inf(info);
    int result = TDEIO::PasswordDialog::getNameAndPassword( inf.username, inf.password,
                                                          &inf.keepPassword, inf.prompt,
                                                          inf.readOnly, inf.caption,
                                                          inf.comment, inf.commentLabel );
    TQByteArray data;
    TQDataStream stream( data, IO_WriteOnly );
    if ( result == TQDialog::Accepted )
        inf.setModified( true );
    else
        inf.setModified( false );
    stream << inf;
    return data;
}

int UIServer::messageBox( int progressId, int type, const TQString &text, const TQString &caption, const TQString &buttonYes, const TQString &buttonNo )
{
    return Observer::messageBox( progressId, type, text, caption, buttonYes, buttonNo );
}

void UIServer::showSSLInfoDialog(const TQString &url, const TDEIO::MetaData &meta)
{
    return showSSLInfoDialog(url,meta,0);
}

void UIServer::showSSLInfoDialog(const TQString &url, const TDEIO::MetaData &meta, int mainwindow)
{
   KSSLInfoDlg *kid = new KSSLInfoDlg(meta["ssl_in_use"].upper()=="TRUE", 0L /*parent?*/, 0L, true);
   KSSLCertificate *x = KSSLCertificate::fromString(meta["ssl_peer_certificate"].local8Bit());
   if (x) {
      // Set the chain back onto the certificate
      TQStringList cl =
                      TQStringList::split(TQString("\n"), meta["ssl_peer_chain"]);
      TQPtrList<KSSLCertificate> ncl;

      ncl.setAutoDelete(true);
      for (TQStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
         KSSLCertificate *y = KSSLCertificate::fromString((*it).local8Bit());
         if (y) ncl.append(y);
      }

      if (ncl.count() > 0)
         x->chain().setChain(ncl);

      kdDebug(7024) << "ssl_cert_errors=" << meta["ssl_cert_errors"] << endl;
      kid->setCertState(meta["ssl_cert_errors"]);
      TQString ip = meta.contains("ssl_proxied") ? "" : meta["ssl_peer_ip"];
      kid->setup( x,
                  ip,
                  url, // the URL
                  meta["ssl_cipher"],
                  meta["ssl_cipher_desc"],
                  meta["ssl_cipher_version"],
                  meta["ssl_cipher_used_bits"].toInt(),
                  meta["ssl_cipher_bits"].toInt(),
                  KSSLCertificate::KSSLValidation(meta["ssl_cert_state"].toInt()));
      kdDebug(7024) << "Showing SSL Info dialog" << endl;
#ifndef TQ_WS_WIN
      if( mainwindow != 0 )
          KWin::setMainWindow( kid, mainwindow );
#endif
      kid->exec();
      delete x;
      kdDebug(7024) << "SSL Info dialog closed" << endl;
   } else {
      KMessageBox::information( 0L, // parent ?
                              i18n("The peer SSL certificate appears to be corrupt."), i18n("SSL") );
   }
   // Don't delete kid!!
}

KSSLCertDlgRet UIServer::showSSLCertDialog(const TQString& host, const TQStringList& certList)
{
    return showSSLCertDialog( host, certList, 0 );
}

KSSLCertDlgRet UIServer::showSSLCertDialog(const TQString& host, const TQStringList& certList, int mainwindow)
{
   KSSLCertDlgRet rc;
   rc.ok = false;
   if (!certList.isEmpty()) {
      KSSLCertDlg *kcd = new KSSLCertDlg(0L, 0L, true);
      kcd->setupDialog(certList);
      kcd->setHost(host);
      kdDebug(7024) << "Showing SSL certificate dialog" << endl;
#ifndef TQ_WS_WIN
      if( mainwindow != 0 )
          KWin::setMainWindow( kcd, mainwindow );
#endif
      kcd->exec();
      rc.ok = true;
      rc.choice = kcd->getChoice();
      rc.save = kcd->saveChoice();
      rc.send = kcd->wantsToSend();
      kdDebug(7024) << "SSL certificate dialog closed" << endl;
      delete kcd;
   }
   return rc;
}


TQByteArray UIServer::open_RenameDlg( int id,
                                     const TQString & caption,
                                     const TQString& src, const TQString & dest,
                                     int mode,
                                     unsigned long sizeSrc,
                                     unsigned long sizeDest,
                                     unsigned long ctimeSrc,
                                     unsigned long ctimeDest,
                                     unsigned long mtimeSrc,
                                     unsigned long mtimeDest
                                     )
{ return open_RenameDlg64(id, caption, src, dest, mode, sizeSrc, sizeDest,
                          ctimeSrc, ctimeDest, mtimeSrc, mtimeDest); }


TQByteArray UIServer::open_RenameDlg64( int id,
                                     const TQString & caption,
                                     const TQString& src, const TQString & dest,
                                     int mode,
                                     TDEIO::filesize_t sizeSrc,
                                     TDEIO::filesize_t sizeDest,
                                     unsigned long ctimeSrc,
                                     unsigned long ctimeDest,
                                     unsigned long mtimeSrc,
                                     unsigned long mtimeDest
                                     )
{
  // Hide existing dialog box if any
  ProgressItem *item = findItem( id );
  if ( item )
    setItemVisible( item, false );
  TQString newDest;
  kdDebug(7024) << "Calling TDEIO::open_RenameDlg" << endl;
  TDEIO::RenameDlg_Result result = TDEIO::open_RenameDlg( caption, src, dest,
                                                      (TDEIO::RenameDlg_Mode) mode, newDest,
                                                      sizeSrc, sizeDest,
                                                      (time_t)ctimeSrc, (time_t)ctimeDest,
                                                      (time_t)mtimeSrc, (time_t)mtimeDest );
  kdDebug(7024) << "TDEIO::open_RenameDlg done" << endl;
  TQByteArray data;
  TQDataStream stream( data, IO_WriteOnly );
  stream << TQ_UINT8(result) << newDest;
  if ( item && result != TDEIO::R_CANCEL )
    setItemVisible( item, true );
  return data;
}

int UIServer::open_SkipDlg( int id,
                            int /*bool*/ multi,
                            const TQString & error_text )
{
  // Hide existing dialog box if any
  ProgressItem *item = findItem( id );
  if ( item )
    setItemVisible( item, false );
  kdDebug(7024) << "Calling TDEIO::open_SkipDlg" << endl;
  TDEIO::SkipDlg_Result result = TDEIO::open_SkipDlg( (bool)multi, error_text );
  if ( item && result != TDEIO::S_CANCEL )
    setItemVisible( item, true );
  return (TDEIO::SkipDlg_Result) result;
}


void UIServer::readSettings() {
  TDEConfig config("uiserverrc");
  config.setGroup( "UIServer" );
  m_showStatusBar=config.readBoolEntry("ShowStatusBar",false);
  m_showToolBar=config.readBoolEntry("ShowToolBar",true);
  m_keepListOpen=config.readBoolEntry("KeepListOpen",false);
  m_initWidth=config.readNumEntry("InitialWidth",460);
  m_initHeight=config.readNumEntry("InitialHeight",150);
  m_bShowList = config.readBoolEntry( "ShowList", false );
  m_showSystemTray=config.readBoolEntry("ShowSystemTray", false);
}

void UIServer::writeSettings() {
  TDEConfig config("uiserverrc");
  config.setGroup( "UIServer" );
  config.writeEntry("InitialWidth",width());
  config.writeEntry("InitialHeight",height());
  config.writeEntry("ShowStatusBar", m_showStatusBar);
  config.writeEntry("ShowToolBar", m_showToolBar);
  config.writeEntry("KeepListOpen", m_keepListOpen);
  config.writeEntry("ShowList", m_bShowList);
  config.writeEntry("ShowSystemTray", m_showSystemTray);
}


void UIServer::slotCancelCurrent() {
  TQListViewItemIterator it( listProgress );
  ProgressItem *item;

  // kill selected jobs
  for ( ; it.current() ; ++it )
  {
    if ( it.current()->isSelected() ) {
      item = (ProgressItem*) it.current();
      killJob( item->appId(), item->jobId() );
      return;
    }
  }
}

void UIServer::resizeEvent(TQResizeEvent* e)
{
   TDEMainWindow::resizeEvent(e);
   writeSettings();
}

bool UIServer::queryClose()
{
  if (( !m_shuttingDown ) && !kapp->sessionSaving()) {
    hide();
    return false;
  }
  return true;
}

UIServer* UIServer::createInstance()
{
    return new UIServer;
}

//------------------------------------------------------------

extern "C" TDE_EXPORT int kdemain(int argc, char **argv)
{
    TDELocale::setMainCatalogue("tdelibs");
    //  GS 5/2001 - I changed the name to "TDE" to make it look better
    //              in the titles of dialogs which are displayed.
    TDEAboutData aboutdata("tdeio_uiserver", I18N_NOOP("TDE"),
                         "0.8", I18N_NOOP("TDE Progress Information UI Server"),
                         TDEAboutData::License_GPL, "(C) 2000, David Faure & Matt Koss");
    // Who's the maintainer ? :)
    aboutdata.addAuthor("David Faure",I18N_NOOP("Developer"),"faure@kde.org");
    aboutdata.addAuthor("Matej Koss",I18N_NOOP("Developer"),"koss@miesto.sk");

    TDECmdLineArgs::init( argc, argv, &aboutdata );
    // TDECmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start())
    {
      kdDebug(7024) << "tdeio_uiserver is already running!" << endl;
      return (0);
    }

    KUniqueApplication app;

    // This app is started automatically, no need for session management
    app.disableSessionManagement();
    app.dcopClient()->setDaemonMode( true );

    uiserver = UIServer::createInstance();

//    app.setMainWidget( uiserver );

    return app.exec();
}

#include "uiserver.moc"
