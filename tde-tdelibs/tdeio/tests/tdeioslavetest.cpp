 /*
  This file is or will be part of KDE desktop environment

  Copyright 1999 Matt Koss <koss@miesto.sk>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <tqlayout.h>
#include <tqmessagebox.h>
#include <tqdir.h>

#include <kacl.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <kurl.h>
#include <kstatusbar.h>
#include <tdeio/job.h>
#include <tdeio/scheduler.h>
#include <kprotocolinfo.h>

#include "tdeioslavetest.h"

using namespace TDEIO;

KioslaveTest::KioslaveTest( TQString src, TQString dest, uint op, uint pr )
  : TDEMainWindow(0, "")
{

  job = 0L;

  main_widget = new TQWidget( this, "");
  TQBoxLayout *topLayout = new TQVBoxLayout( main_widget, 10, 5 );

  TQGridLayout *grid = new TQGridLayout( 2, 2, 10 );
  topLayout->addLayout( grid );

  grid->setRowStretch(0,1);
  grid->setRowStretch(1,1);

  grid->setColStretch(0,1);
  grid->setColStretch(1,100);

  lb_from = new TQLabel( "From:", main_widget );
  grid->addWidget( lb_from, 0, 0 );

  le_source = new TQLineEdit( main_widget );
  grid->addWidget( le_source, 0, 1 );
  le_source->setText( src );

  lb_to = new TQLabel( "To:", main_widget );
  grid->addWidget( lb_to, 1, 0 );

  le_dest = new TQLineEdit( main_widget );
  grid->addWidget( le_dest, 1, 1 );
  le_dest->setText( dest );

  // Operation groupbox & buttons
  opButtons = new TQButtonGroup( "Operation", main_widget );
  topLayout->addWidget( opButtons, 10 );
  connect( opButtons, TQ_SIGNAL(clicked(int)), TQ_SLOT(changeOperation(int)) );

  TQBoxLayout *hbLayout = new TQHBoxLayout( opButtons, 15 );

  rbList = new TQRadioButton( "List", opButtons );
  opButtons->insert( rbList, List );
  hbLayout->addWidget( rbList, 5 );

  rbListRecursive = new TQRadioButton( "ListRecursive", opButtons );
  opButtons->insert( rbListRecursive, ListRecursive );
  hbLayout->addWidget( rbListRecursive, 5 );

  rbStat = new TQRadioButton( "Stat", opButtons );
  opButtons->insert( rbStat, Stat );
  hbLayout->addWidget( rbStat, 5 );

  rbGet = new TQRadioButton( "Get", opButtons );
  opButtons->insert( rbGet, Get );
  hbLayout->addWidget( rbGet, 5 );

  rbPut = new TQRadioButton( "Put", opButtons );
  opButtons->insert( rbPut, Put );
  hbLayout->addWidget( rbPut, 5 );

  rbCopy = new TQRadioButton( "Copy", opButtons );
  opButtons->insert( rbCopy, Copy );
  hbLayout->addWidget( rbCopy, 5 );

  rbMove = new TQRadioButton( "Move", opButtons );
  opButtons->insert( rbMove, Move );
  hbLayout->addWidget( rbMove, 5 );

  rbDelete = new TQRadioButton( "Delete", opButtons );
  opButtons->insert( rbDelete, Delete );
  hbLayout->addWidget( rbDelete, 5 );

  rbShred = new TQRadioButton( "Shred", opButtons );
  opButtons->insert( rbShred, Shred );
  hbLayout->addWidget( rbShred, 5 );

  rbMkdir = new TQRadioButton( "Mkdir", opButtons );
  opButtons->insert( rbMkdir, Mkdir );
  hbLayout->addWidget( rbMkdir, 5 );

  rbMimetype = new TQRadioButton( "Mimetype", opButtons );
  opButtons->insert( rbMimetype, Mimetype );
  hbLayout->addWidget( rbMimetype, 5 );

  opButtons->setButton( op );
  changeOperation( op );

  // Progress groupbox & buttons
  progressButtons = new TQButtonGroup( "Progress dialog mode", main_widget );
  topLayout->addWidget( progressButtons, 10 );
  connect( progressButtons, TQ_SIGNAL(clicked(int)), TQ_SLOT(changeProgressMode(int)) );

  hbLayout = new TQHBoxLayout( progressButtons, 15 );

  rbProgressNone = new TQRadioButton( "None", progressButtons );
  progressButtons->insert( rbProgressNone, ProgressNone );
  hbLayout->addWidget( rbProgressNone, 5 );

  rbProgressDefault = new TQRadioButton( "Default", progressButtons );
  progressButtons->insert( rbProgressDefault, ProgressDefault );
  hbLayout->addWidget( rbProgressDefault, 5 );

  rbProgressStatus = new TQRadioButton( "Status", progressButtons );
  progressButtons->insert( rbProgressStatus, ProgressStatus );
  hbLayout->addWidget( rbProgressStatus, 5 );

  progressButtons->setButton( pr );
  changeProgressMode( pr );

  // statusbar progress widget
  statusProgress = new StatusbarProgress( statusBar() );
  statusBar()->addWidget( statusProgress, 0, true );

  // run & stop butons
  hbLayout = new TQHBoxLayout( topLayout, 15 );

  pbStart = new TQPushButton( "&Start", main_widget );
  pbStart->setFixedSize( pbStart->sizeHint() );
  connect( pbStart, TQ_SIGNAL(clicked()), TQ_SLOT(startJob()) );
  hbLayout->addWidget( pbStart, 5 );

  pbStop = new TQPushButton( "Sto&p", main_widget );
  pbStop->setFixedSize( pbStop->sizeHint() );
  pbStop->setEnabled( false );
  connect( pbStop, TQ_SIGNAL(clicked()), TQ_SLOT(stopJob()) );
  hbLayout->addWidget( pbStop, 5 );

  // close button
  close = new TQPushButton( "&Close", main_widget );
  close->setFixedSize( close->sizeHint() );
  connect(close, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotQuit()));

  topLayout->addWidget( close, 5 );

  main_widget->setMinimumSize( main_widget->sizeHint() );
  setCentralWidget( main_widget );

  slave = 0;
//  slave = TDEIO::Scheduler::getConnectedSlave(KURL("ftp://ftp.kde.org"));
  TDEIO::Scheduler::connect(TQ_SIGNAL(slaveConnected(TDEIO::Slave*)),
	this, TQ_SLOT(slotSlaveConnected()));
  TDEIO::Scheduler::connect(TQ_SIGNAL(slaveError(TDEIO::Slave*,int,const TQString&)),
	this, TQ_SLOT(slotSlaveError()));
}


void KioslaveTest::closeEvent( TQCloseEvent * ){
  slotQuit();
}


void KioslaveTest::slotQuit(){
  if ( job ) {
    job->kill( true );  // kill the job quietly
  }
  if (slave)
    TDEIO::Scheduler::disconnectSlave(slave);
  kapp->quit();
}


void KioslaveTest::changeOperation( int id ) {
  // only two urls for copy and move
  bool enab = rbCopy->isChecked() || rbMove->isChecked();

  le_dest->setEnabled( enab );

  selectedOperation = id;
}


void KioslaveTest::changeProgressMode( int id ) {
  progressMode = id;

  if ( progressMode == ProgressStatus ) {
    statusBar()->show();
  } else {
    statusBar()->hide();
  }
}


void KioslaveTest::startJob() {
  TQString sCurrent = TQDir::currentDirPath()+"/";
  KURL::encode_string(sCurrent);
  TQString sSrc( le_source->text() );
  KURL src( sCurrent, sSrc );

  if ( !src.isValid() ) {
    TQMessageBox::critical(this, "Kioslave Error Message", "Source URL is malformed" );
    return;
  }

  TQString sDest( le_dest->text() );
  KURL dest( sCurrent, sDest );

  if ( !dest.isValid() &&
       ( selectedOperation == Copy || selectedOperation == Move ) ) {
    TQMessageBox::critical(this, "Kioslave Error Message",
                       "Destination URL is malformed" );
    return;
  }

  pbStart->setEnabled( false );

  bool observe = true;
  if (progressMode != ProgressDefault) {
    observe = false;
  }

  SimpleJob *myJob = 0;

  switch ( selectedOperation ) {
  case List:
    myJob = TDEIO::listDir( src );
    connect(myJob, TQ_SIGNAL( entries( TDEIO::Job*, const TDEIO::UDSEntryList&)),
            TQ_SLOT( slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList&)));
    break;

  case ListRecursive:
    myJob = TDEIO::listRecursive( src );
    connect(myJob, TQ_SIGNAL( entries( TDEIO::Job*, const TDEIO::UDSEntryList&)),
            TQ_SLOT( slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList&)));
    break;

  case Stat:
    myJob = TDEIO::stat( src, false, 2 );
    break;

  case Get:
    myJob = TDEIO::get( src, true );
    connect(myJob, TQ_SIGNAL( data( TDEIO::Job*, const TQByteArray &)),
            TQ_SLOT( slotData( TDEIO::Job*, const TQByteArray &)));
    break;

  case Put:
    putBuffer = 0;
    myJob = TDEIO::put( src, -1, true, false);
    connect(myJob, TQ_SIGNAL( dataReq( TDEIO::Job*, TQByteArray &)),
            TQ_SLOT( slotDataReq( TDEIO::Job*, TQByteArray &)));
    break;

  case Copy:
    job = TDEIO::copy( src, dest, observe );
    break;

  case Move:
    job = TDEIO::move( src, dest, observe );
    break;

  case Delete:
    job = TDEIO::del( src, false, observe );
    break;

  case Shred:
    job = TDEIO::del(src, true, observe);
    break;

  case Mkdir:
    myJob = TDEIO::mkdir( src );
    break;

  case Mimetype:
    myJob = TDEIO::mimetype( src );
    break;
  }
  if (myJob)
  {
    if (slave)
      TDEIO::Scheduler::assignJobToSlave(slave, myJob);
    job = myJob;
  }

  connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
           TQ_SLOT( slotResult( TDEIO::Job * ) ) );

  connect( job, TQ_SIGNAL( canceled( TDEIO::Job * ) ),
           TQ_SLOT( slotResult( TDEIO::Job * ) ) );

  if (progressMode == ProgressStatus) {
    statusProgress->setJob( job );
  }

  pbStop->setEnabled( true );
}


void KioslaveTest::slotResult( TDEIO::Job * _job )
{
  if ( _job->error() )
  {
    _job->showErrorDialog();
  }
  else if ( selectedOperation == Stat )
  {
      UDSEntry entry = ((TDEIO::StatJob*)_job)->statResult();
      printUDSEntry( entry );
  }
  else if ( selectedOperation == Mimetype )
  {
      kdDebug() << "mimetype is " << ((TDEIO::MimetypeJob*)_job)->mimetype() << endl;
  }

  if (job == _job)
     job = 0L;
  pbStart->setEnabled( true );
  pbStop->setEnabled( false );
}

void KioslaveTest::slotSlaveConnected()
{
   kdDebug() << "Slave connected." << endl;
}

void KioslaveTest::slotSlaveError()
{
   kdDebug() << "Error connected." << endl;
   slave = 0;
}

static void printACL( const TQString& acl )
{
  KACL kacl( acl );
  kdDebug() << "According to KACL: " << endl << kacl.asString() << endl;
  kdDebug() << "Owner: " << kacl.ownerPermissions() << endl;
  kdDebug() << "Owning group: " << kacl.owningGroupPermissions() << endl;
  kdDebug() << "Others: " << kacl.othersPermissions() << endl;
}


void KioslaveTest::printUDSEntry( const TDEIO::UDSEntry & entry )
{
    TDEIO::UDSEntry::ConstIterator it = entry.begin();
    for( ; it != entry.end(); it++ ) {
        switch ((*it).m_uds) {
            case TDEIO::UDS_FILE_TYPE:
                kdDebug() << "File Type : " << (mode_t)((*it).m_long) << endl;
                if ( S_ISDIR( (mode_t)((*it).m_long) ) )
                {
                    kdDebug() << "is a dir" << endl;
                }
                break;
            case TDEIO::UDS_ACCESS:
                kdDebug() << "Access permissions : " << (*it).m_long << endl;
                break;
            case TDEIO::UDS_EXTENDED_ACL:
                if( (*it).m_long == 1 )
                  kdDebug() << "Has extended ACL information." << endl;
                break;
            case TDEIO::UDS_ACL_STRING:
                kdDebug() << "ACL: " << ( (*it).m_str.ascii() ) << endl;
                printACL( (*it).m_str );
                break;
            case TDEIO::UDS_DEFAULT_ACL_STRING:
                kdDebug() << "Default ACL: " << ( (*it).m_str.ascii() ) << endl;
                printACL( (*it).m_str );
                break;
            case TDEIO::UDS_USER:
                kdDebug() << "User : " << ((*it).m_str.ascii() ) << endl;
                break;
            case TDEIO::UDS_GROUP:
                kdDebug() << "Group : " << ((*it).m_str.ascii() ) << endl;
                break;
            case TDEIO::UDS_NAME:
                kdDebug() << "Name : " << ((*it).m_str.ascii() ) << endl;
                //m_strText = decodeFileName( (*it).m_str );
                break;
            case TDEIO::UDS_URL:
                kdDebug() << "URL : " << ((*it).m_str.ascii() ) << endl;
                break;
            case TDEIO::UDS_MIME_TYPE:
                kdDebug() << "MimeType : " << ((*it).m_str.ascii() ) << endl;
                break;
            case TDEIO::UDS_LINK_DEST:
                kdDebug() << "LinkDest : " << ((*it).m_str.ascii() ) << endl;
                break;
            case TDEIO::UDS_SIZE:
                kdDebug() << "Size: " << TDEIO::convertSize((*it).m_long) << endl;
                break;
        }
    }
}

void KioslaveTest::slotEntries(TDEIO::Job* job, const TDEIO::UDSEntryList& list) {

    KURL url = static_cast<TDEIO::ListJob*>( job )->url();
    KProtocolInfo::ExtraFieldList extraFields = KProtocolInfo::extraFields(url);
    UDSEntryListConstIterator it=list.begin();
    for (; it != list.end(); ++it) {
        // For each file...
        KProtocolInfo::ExtraFieldList::Iterator extraFieldsIt = extraFields.begin();
        UDSEntry::ConstIterator it2 = (*it).begin();
        for( ; it2 != (*it).end(); it2++ ) {
            if ((*it2).m_uds == UDS_NAME)
                kdDebug() << "" << ( *it2 ).m_str << endl;
            else if ( (*it2).m_uds == UDS_EXTRA) {
                Q_ASSERT( extraFieldsIt != extraFields.end() );
                TQString column = (*extraFieldsIt).name;
                //TQString type = (*extraFieldsIt).type;
                kdDebug() << "  Extra data (" << column << ") :" << ( *it2 ).m_str << endl;
                ++extraFieldsIt;
            }
        }
    }
}

void KioslaveTest::slotData(TDEIO::Job*, const TQByteArray &data)
{
    if (data.size() == 0)
    {
       kdDebug(0) << "Data: <End>" << endl;
    }
    else
    {
       kdDebug(0) << "Data: \"" << TQCString(data, data.size()+1) << "\"" << endl;
    }
}

void KioslaveTest::slotDataReq(TDEIO::Job*, TQByteArray &data)
{
    const char *fileDataArray[] =
       {
         "Hello world\n",
         "This is a test file\n",
         "You can safely delete it.\n",
         "BIG\n",
         0
       };
    const char *fileData = fileDataArray[putBuffer++];

    if (!fileData)
    {
       kdDebug(0) << "DataReq: <End>" << endl;
       return;
    }
    if (!strcmp(fileData, "BIG\n"))
	data.fill(0, 29*1024*1024);
    else
	data.duplicate(fileData, strlen(fileData));
    kdDebug(0) << "DataReq: \"" << fileData << "\"" << endl;
}

void KioslaveTest::stopJob() {
  kdDebug() << "KioslaveTest::stopJob()" << endl;
  job->kill();
  job = 0L;

  pbStop->setEnabled( false );
  pbStart->setEnabled( true );
}

static const char version[] = "v0.0.0 0000";   // :-)
static const char description[] = "Test for tdeioslaves";
static TDECmdLineOptions options[] =
{
 { "s", 0, 0 },
 { "src <src>", "Source URL", "" },
 { "d", 0, 0 },
 { "dest <dest>", "Destination URL", "" },
 { "o", 0, 0 },
 { "operation <operation>", "Operation (list,listrecursive,stat,get,put,copy,move,del,shred,mkdir)", "copy" },
 { "p", 0, 0 },
 { "progress <progress>", "Progress Type (none,default,status)", "default" },
 TDECmdLineLastOption
};

int main(int argc, char **argv) {
  TDECmdLineArgs::init( argc, argv, "tdeioslavetest", description, version );
  TDECmdLineArgs::addCmdLineOptions( options );
  TDEApplication app;

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

  TQString src = args->getOption("src");
  TQString dest = args->getOption("dest");

  uint op = 0;
  uint pr = 0;

  TQString tmps;

  tmps = args->getOption("operation");
  if ( tmps == "list") {
    op = KioslaveTest::List;
  } else if ( tmps == "listrecursive") {
    op = KioslaveTest::ListRecursive;
  } else if ( tmps == "stat") {
    op = KioslaveTest::Stat;
  } else if ( tmps == "get") {
    op = KioslaveTest::Get;
  } else if ( tmps == "put") {
    op = KioslaveTest::Put;
  } else if ( tmps == "copy") {
    op = KioslaveTest::Copy;
  } else if ( tmps == "move") {
    op = KioslaveTest::Move;
  } else if ( tmps == "del") {
    op = KioslaveTest::Delete;
  } else if ( tmps == "shred") {
    op = KioslaveTest::Shred;
  } else if ( tmps == "mkdir") {
    op = KioslaveTest::Mkdir;
  } else TDECmdLineArgs::usage("unknown operation");

  tmps = args->getOption("progress");
  if ( tmps == "none") {
    pr = KioslaveTest::ProgressNone;
  } else if ( tmps == "default") {
    pr = KioslaveTest::ProgressDefault;
  } else if ( tmps == "status") {
    pr = KioslaveTest::ProgressStatus;
  } else TDECmdLineArgs::usage("unknown progress mode");

  args->clear(); // Free up memory

  KioslaveTest test( src, dest, op, pr );
  test.show();
  // Bug in KTMW / Qt / layouts ?
  test.resize( test.sizeHint() );

  app.setMainWidget(&test);
  app.exec();
}


#include "tdeioslavetest.moc"
