/*  This file is part of the KDE libraries

    Copyright (C) 1997 Torben Weis <weis@stud.uni-frankfurt.de>
    Copyright (C) 1999 Dirk Mueller <mueller@kde.org>
    Portions copyright (C) 1999 Preston Brown <pbrown@kde.org>

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

#include <tqfile.h>
#include <tqdir.h>
#include <tqdialog.h>
#include <tqimage.h>
#include <tqpixmap.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqtoolbutton.h>
#include <tqcheckbox.h>
#include <tqtooltip.h>
#include <tqstyle.h>
#include <tqwhatsthis.h>

#include <tdeapplication.h>
#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kdesktopfile.h>
#include <kdialog.h>
#include <tdeglobal.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <kmimemagic.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kuserprofile.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <dcopclient.h>
#include <kmimetype.h>
#include <kservicegroup.h>
#include <tdelistview.h>
#include <tdesycoca.h>
#include <kstdguiitem.h>

#include "kopenwith.h"
#include "kopenwith_p.h"

#include <kdebug.h>
#include <assert.h>
#include <stdlib.h>

#define SORT_SPEC (TQDir::DirsFirst | TQDir::Name | TQDir::IgnoreCase)


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( TDEListView* parent, const TQString & name,
                                    const TQPixmap& pixmap, bool parse, bool dir,
                                    const TQString &p, const TQString &c, const TQString &dp )
    : TQListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c, dp);
}


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( TQListViewItem* parent, const TQString & name,
                                    const TQPixmap& pixmap, bool parse, bool dir,
                                    const TQString &p, const TQString &c, const TQString &dp )
    : TQListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c, dp);
}


// ----------------------------------------------------------------------

void KAppTreeListItem::init(const TQPixmap& pixmap, bool parse, bool dir, 
                            const TQString &_path, const TQString &_exec, const TQString &_desktopPath)
{
    setPixmap(0, pixmap);
    parsed = parse;
    directory = dir;
    path = _path; // relative path
    exec = _exec; // executable command
    desktopPath = _desktopPath;  // .desktop file path
}


/* Ensures that directories sort before non-directories */
int KAppTreeListItem::compare(TQListViewItem *i, int col, bool ascending) const
{
	KAppTreeListItem *other = dynamic_cast<KAppTreeListItem *>(i);

	// Directories sort first
	if (directory && !other->directory)
		return -1;

	else if (!directory && other->directory)
		return 1;

	else // both directories or both not
		return TQListViewItem::compare(i, col, ascending);
}

// ----------------------------------------------------------------------
// Ensure that case is ignored
TQString KAppTreeListItem::key(int column, bool /*ascending*/) const
{
        return text(column).upper();
}

void KAppTreeListItem::activate()
{
    if ( directory )
        setOpen(!isOpen());
}

void KAppTreeListItem::setOpen( bool o )
{
    if( o && !parsed ) { // fill the children before opening
        ((TDEApplicationTree *) parent())->addDesktopGroup( path, this );
        parsed = true;
    }
    TQListViewItem::setOpen( o );
}

bool KAppTreeListItem::isDirectory()
{
    return directory;
}

// ----------------------------------------------------------------------

TDEApplicationTree::TDEApplicationTree( TQWidget *parent )
    : TDEListView( parent ), currentitem(0)
{
    addColumn( i18n("Known Applications") );
    setRootIsDecorated( true );

    addDesktopGroup( TQString::null );
    cleanupTree();

    connect( this, TQ_SIGNAL( currentChanged(TQListViewItem*) ),
            TQ_SLOT( slotItemHighlighted(TQListViewItem*) ) );
    connect( this, TQ_SIGNAL( selectionChanged(TQListViewItem*) ),
            TQ_SLOT( slotSelectionChanged(TQListViewItem*) ) );
}

// ----------------------------------------------------------------------

bool TDEApplicationTree::isDirSel()
{
    if (!currentitem) return false; // if currentitem isn't set
    return currentitem->isDirectory();
}

// ----------------------------------------------------------------------

static TQPixmap appIcon(const TQString &iconName)
{
    TQPixmap normal = TDEGlobal::iconLoader()->loadIcon(iconName, TDEIcon::Small, 0, TDEIcon::DefaultState, 0L, true);
    // make sure they are not larger than 20x20
    if (normal.width() > 20 || normal.height() > 20)
    {
       TQImage tmp = normal.convertToImage();
       tmp = tmp.smoothScale(20, 20);
       normal.convertFromImage(tmp);
    }
    return normal;
}

void TDEApplicationTree::addDesktopGroup( const TQString &relPath, KAppTreeListItem *item)
{
   KServiceGroup::Ptr root = KServiceGroup::group(relPath);
   if (!root || !root->isValid()) return;

   KServiceGroup::List list = root->entries();

   KAppTreeListItem * newItem;
   for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
   {
      TQString icon, text, relPath, exec, desktopPath;
      bool isDir = false;
      KSycocaEntry *p = (*it);
      if (p->isType(KST_KService))
      {
         KService *service = static_cast<KService *>(p);

         if (service->noDisplay())
            continue;

         icon = service->icon();
         text = service->name();
         exec = service->exec();
         desktopPath = service->desktopEntryPath();
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup *serviceGroup = static_cast<KServiceGroup *>(p);

         if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0)
            continue;

         icon = serviceGroup->icon();
         text = serviceGroup->caption();
         relPath = serviceGroup->relPath();
         isDir = true;
      }
      else
      {
         kdWarning(250) << "KServiceGroup: Unexpected object in list!" << endl;
         continue;
      }

      TQPixmap pixmap = appIcon( icon );
      
      if (item)
         newItem = new KAppTreeListItem( item, text, pixmap, false, isDir,
                                         relPath, exec, desktopPath );
      else
         newItem = new KAppTreeListItem( this, text, pixmap, false, isDir,
                                         relPath, exec, desktopPath );
      if (isDir)
         newItem->setExpandable( true );
   }
}


// ----------------------------------------------------------------------

void TDEApplicationTree::slotItemHighlighted(TQListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KAppTreeListItem *item = (KAppTreeListItem *) i;

    currentitem = item;

    if( (!item->directory ) && (!item->exec.isEmpty()) )
        emit highlighted( item->text(0), item->exec, item->desktopPath );
}


// ----------------------------------------------------------------------

void TDEApplicationTree::slotSelectionChanged(TQListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KAppTreeListItem *item = (KAppTreeListItem *) i;

    currentitem = item;

    if( ( !item->directory ) && (!item->exec.isEmpty() ) )
        emit selected( item->text(0), item->exec, item->desktopPath );
}

// ----------------------------------------------------------------------

void TDEApplicationTree::resizeEvent( TQResizeEvent * e)
{
    setColumnWidth(0, width()-TQApplication::style().pixelMetric(TQStyle::PM_ScrollBarExtent)
                         -2*TQApplication::style().pixelMetric(TQStyle::PM_DefaultFrameWidth));
    TDEListView::resizeEvent(e);
}

// Prune empty directories from the tree
void TDEApplicationTree::cleanupTree()
{
	TQListViewItem *item=firstChild();
	while(item!=0)
	{
		if(item->isExpandable())
		{
			TQListViewItem *temp=item->itemBelow();
			if(item->text(0)!=i18n("Applications"))
				item->setOpen(false);
			item=temp;
			continue;
		}
		item=item->itemBelow();
	}
}

/***************************************************************
 *
 * KOpenWithDlg
 *
 ***************************************************************/
class KOpenWithDlgPrivate
{
public:
    KOpenWithDlgPrivate() : saveNewApps(false) { };
    TQPushButton* ok;
    bool saveNewApps;
    KService::Ptr curService;
};

KOpenWithDlg::KOpenWithDlg( const KURL::List& _urls, TQWidget* parent )
             :TQDialog( parent, "openwith", true )
{
    setCaption( i18n( "Open With" ) );
    TQString text;
    if( _urls.count() == 1 )
    {
        text = i18n("<qt>Select the program that should be used to open <b>%1</b>. "
                     "If the program is not listed, enter the name or click "
                     "the browse button.</qt>").arg( _urls.first().fileName() );
    }
    else
        // Should never happen ??
        text = i18n( "Choose the name of the program with which to open the selected files." );
    setServiceType( _urls );
    init( text, TQString() );
}

KOpenWithDlg::KOpenWithDlg( const KURL::List& _urls, const TQString&_text,
                            const TQString& _value, TQWidget *parent)
             :TQDialog( parent, "openwith", true )
{
  TQString caption = KStringHandler::csqueeze( _urls.first().prettyURL() );
  if (_urls.count() > 1)
      caption += TQString::fromLatin1("...");
  setCaption(caption);
  setServiceType( _urls );
  init( _text, _value );
}

KOpenWithDlg::KOpenWithDlg( const TQString &serviceType, const TQString& value,
                            TQWidget *parent)
             :TQDialog( parent, "openwith", true )
{
   setCaption(i18n("Choose Application for %1").arg(serviceType));
  TQString text = i18n("<qt>Select the program for the file type: <b>%1</b>. "
                      "If the program is not listed, enter the name or click "
                      "the browse button.</qt>").arg(serviceType);
  qServiceType = serviceType;
  init( text, value );
  if (remember)
      remember->hide();
}

KOpenWithDlg::KOpenWithDlg( TQWidget *parent)
             :TQDialog( parent, "openwith", true )
{
  setCaption(i18n("Choose Application"));
  TQString text = i18n("<qt>Select a program. "
                      "If the program is not listed, enter the name or click "
                      "the browse button.</qt>");
  qServiceType = TQString::null;
  init( text, TQString::null );
}

void KOpenWithDlg::setServiceType( const KURL::List& _urls )
{
  if ( _urls.count() == 1 )
  {
    qServiceType = KMimeType::findByURL( _urls.first())->name();
    if (qServiceType == TQString::fromLatin1("application/octet-stream"))
      qServiceType = TQString::null;
  }
  else
      qServiceType = TQString::null;
}

void KOpenWithDlg::init( const TQString& _text, const TQString& _value )
{
  d = new KOpenWithDlgPrivate;
  bool bReadOnly = kapp && !kapp->authorize("shell_access");
  m_terminaldirty = false;
  m_pTree = 0L;
  m_pService = 0L;
  d->curService = 0L;

  TQBoxLayout *topLayout = new TQVBoxLayout( this, KDialog::marginHint(),
          KDialog::spacingHint() );
  label = new TQLabel( _text, this );
  topLayout->addWidget(label);

  TQHBoxLayout* hbox = new TQHBoxLayout(topLayout);

  TQToolButton *clearButton = new TQToolButton( this );
  clearButton->setIconSet( BarIcon( "locationbar_erase" ) );
  clearButton->setFixedSize( clearButton->sizeHint() );
  connect( clearButton, TQ_SIGNAL( clicked() ), TQ_SLOT( slotClear() ) );
  TQToolTip::add( clearButton, i18n( "Clear input field" ) );

  hbox->addWidget( clearButton );

  if (!bReadOnly)
  {
    // init the history combo and insert it into the URL-Requester
    KHistoryCombo *combo = new KHistoryCombo();
    combo->setDuplicatesEnabled( false );
    TDEConfig *kc = TDEGlobal::config();
    TDEConfigGroupSaver ks( kc, TQString::fromLatin1("Open-with settings") );
    int max = kc->readNumEntry( TQString::fromLatin1("Maximum history"), 15 );
    combo->setMaxCount( max );
    int mode = kc->readNumEntry(TQString::fromLatin1("CompletionMode"),
				TDEGlobalSettings::completionMode());
    combo->setCompletionMode((TDEGlobalSettings::Completion)mode);
    TQStringList list = kc->readListEntry( TQString::fromLatin1("History") );
    combo->setHistoryItems( list, true );
    edit = new KURLRequester( combo, this );
  }
  else
  {
    clearButton->hide();
    edit = new KURLRequester( this );
    edit->lineEdit()->setReadOnly(true);
    edit->button()->hide();
  }

  edit->setURL( _value );
  TQWhatsThis::add(edit,i18n(
    "Following the command, you can have several place holders which will be replaced "
    "with the actual values when the actual program is run:\n"
    "%f - a single file name\n"
    "%F - a list of files; use for applications that can open several local files at once\n"
    "%u - a single URL\n"
    "%U - a list of URLs\n"
    "%d - the directory of the file to open\n"
    "%D - a list of directories\n"
    "%i - the icon\n"
    "%m - the mini-icon\n"
    "%c - the comment"));

  hbox->addWidget(edit);

  if ( edit->comboBox() ) {
    KURLCompletion *comp = new KURLCompletion( KURLCompletion::ExeCompletion );
    edit->comboBox()->setCompletionObject( comp );
    edit->comboBox()->setAutoDeleteCompletionObject( true );
  }

  connect ( edit, TQ_SIGNAL(returnPressed()), TQ_SLOT(slotOK()) );
  connect ( edit, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotTextChanged()) );

  m_pTree = new TDEApplicationTree( this );
  topLayout->addWidget(m_pTree);

  connect( m_pTree, TQ_SIGNAL( selected( const TQString&, const TQString&, const TQString& ) ),
           TQ_SLOT( slotSelected( const TQString&, const TQString&, const TQString& ) ) );
  connect( m_pTree, TQ_SIGNAL( highlighted( const TQString&, const TQString&, const TQString& ) ),
           TQ_SLOT( slotHighlighted( const TQString&, const TQString&, const TQString& ) ) );
  connect( m_pTree, TQ_SIGNAL( doubleClicked(TQListViewItem*) ),
           TQ_SLOT( slotDbClick() ) );

  terminal = new TQCheckBox( i18n("Run in &terminal"), this );
  if (bReadOnly)
     terminal->hide();
  connect(terminal, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotTerminalToggled(bool)));

  topLayout->addWidget(terminal);

  TQBoxLayout* nocloseonexitLayout = new TQHBoxLayout( 0, 0, KDialog::spacingHint() );
  TQSpacerItem* spacer = new TQSpacerItem( 20, 0, TQSizePolicy::Fixed, TQSizePolicy::Minimum );
  nocloseonexitLayout->addItem( spacer );

  nocloseonexit = new TQCheckBox( i18n("&Do not close when command exits"), this );
  nocloseonexit->setChecked( false );
  nocloseonexit->setDisabled( true );

  // check to see if we use konsole if not disable the nocloseonexit
  // because we don't know how to do this on other terminal applications
  TDEConfigGroup confGroup( TDEGlobal::config(), TQString::fromLatin1("General") );
  TQString preferredTerminal = confGroup.readPathEntry("TerminalApplication", TQString::fromLatin1("konsole"));

  if (bReadOnly || preferredTerminal != "konsole")
     nocloseonexit->hide();

  nocloseonexitLayout->addWidget( nocloseonexit );
  topLayout->addLayout( nocloseonexitLayout );

  if (!qServiceType.isNull())
  {
    remember = new TQCheckBox(i18n("&Remember application association for this type of file"), this);
    //    remember->setChecked(true);
    topLayout->addWidget(remember);
  }
  else
    remember = 0L;

  // Use KButtonBox for the aligning pushbuttons nicely
  KButtonBox* b = new KButtonBox( this );
  b->addStretch( 2 );

  d->ok = b->addButton( KStdGuiItem::ok() );
  d->ok->setDefault( true );
  connect(  d->ok, TQ_SIGNAL( clicked() ), TQ_SLOT( slotOK() ) );

  TQPushButton* cancel = b->addButton(  KStdGuiItem::cancel() );
  connect(  cancel, TQ_SIGNAL( clicked() ), TQ_SLOT( reject() ) );

  b->layout();
  topLayout->addWidget( b );

  //edit->setText( _value );
  // This is what caused "can't click on items before clicking on Name header".
  // Probably due to the resizeEvent handler using width().
  //resize( minimumWidth(), sizeHint().height() );
  edit->setFocus();
  slotTextChanged();
}


// ----------------------------------------------------------------------

KOpenWithDlg::~KOpenWithDlg()
{
    delete d;
    d = 0;
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotClear()
{
    edit->setURL(TQString::null);
    edit->setFocus();
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotSelected( const TQString& /*_name*/, const TQString& _exec, const TQString& /*_desktopPath*/ )
{
    kdDebug(250)<<"KOpenWithDlg::slotSelected"<<endl;
    KService::Ptr pService = d->curService;
    edit->setURL( _exec ); // calls slotTextChanged :(
    d->curService = pService;
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotHighlighted( const TQString& _name, const TQString& _exec, const TQString& _desktopPath )
{
    kdDebug(250)<<"KOpenWithDlg::slotHighlighted"<<endl;
    qName = _name;
    // Look up by desktop path first, to avoid returning wrong results in case multiple .desktop files
    // contain the same name for different services (such as Konsole)
    // Try by name only if first search fails (this should never happen normally)
    d->curService = KService::serviceByDesktopPath( _desktopPath );
    if (!d->curService)
    { 
    	d->curService = KService::serviceByName( qName );
    }
    if (!m_terminaldirty)
    {
        // ### indicate that default value was restored
        terminal->setChecked(d->curService->terminal());
        TQString terminalOptions = d->curService->terminalOptions();
        nocloseonexit->setChecked( (terminalOptions.contains( "--noclose" ) > 0) );
        m_terminaldirty = false; // slotTerminalToggled changed it
    }
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotTextChanged()
{
    kdDebug(250)<<"KOpenWithDlg::slotTextChanged"<<endl;
    // Forget about the service
    d->curService = 0L;
    d->ok->setEnabled( !edit->url().isEmpty());
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotTerminalToggled(bool)
{
    // ### indicate that default value was overridden
    m_terminaldirty = true;
    nocloseonexit->setDisabled( ! terminal->isChecked() );
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotDbClick()
{
   if (m_pTree->isDirSel() ) return; // check if a directory is selected
   slotOK();
}

void KOpenWithDlg::setSaveNewApplications(bool b)
{
  d->saveNewApps = b;
}

void KOpenWithDlg::slotOK()
{
  TQString typedExec(edit->url());
  TQString fullExec(typedExec);

  TQString serviceName;
  TQString initialServiceName;
  TQString preferredTerminal;
  m_pService = d->curService;
  if (!m_pService) {
    // No service selected - check the command line

    // Find out the name of the service from the command line, removing args and paths
    serviceName = KRun::binaryName( typedExec, true );
    if (serviceName.isEmpty())
    {
      // TODO add a KMessageBox::error here after the end of the message freeze
      return;
    }
    initialServiceName = serviceName;
    kdDebug(250) << "initialServiceName=" << initialServiceName << endl;
    int i = 1; // We have app, app-2, app-3... Looks better for the user.
    bool ok = false;
    // Check if there's already a service by that name, with the same Exec line
    do {
        kdDebug(250) << "looking for service " << serviceName << endl;
        KService::Ptr serv = KService::serviceByDesktopName( serviceName );
        ok = !serv; // ok if no such service yet
        // also ok if we find the exact same service (well, "kwrite" == "kwrite %U"
        if ( serv && serv->type() == "Application")
        {
            TQString exec = serv->exec();
            fullExec = exec;
            exec.replace("%u", "", false);
            exec.replace("%f", "", false);
            exec.replace("-caption %c", "");
            exec.replace("-caption \"%c\"", "");
            exec.replace("%i", "");
            exec.replace("%m", "");
            exec = exec.simplifyWhiteSpace();
            if (exec == typedExec)
            {
                ok = true;
                m_pService = serv;
                kdDebug(250) << k_funcinfo << "OK, found identical service: " << serv->desktopEntryPath() << endl;
            }
        }
        if (!ok) // service was found, but it was different -> keep looking
        {
            ++i;
            serviceName = initialServiceName + "-" + TQString::number(i);
        }
    }
    while (!ok);
  }
  if ( m_pService )
  {
    // Existing service selected
    serviceName = m_pService->name();
    initialServiceName = serviceName;
    fullExec = m_pService->exec();
  }

  if (terminal->isChecked())
  {
    TDEConfigGroup confGroup( TDEGlobal::config(), TQString::fromLatin1("General") );
    preferredTerminal = confGroup.readPathEntry("TerminalApplication", TQString::fromLatin1("konsole"));
    m_command = preferredTerminal;
    // only add --noclose when we are sure it is konsole we're using
    if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
      m_command += TQString::fromLatin1(" --noclose");
    m_command += TQString::fromLatin1(" -e ");
    m_command += edit->url();
    kdDebug(250) << "Setting m_command to " << m_command << endl;
  }
  if ( m_pService && terminal->isChecked() != m_pService->terminal() )
      m_pService = 0L; // It's not exactly this service we're running

  bool bRemember = remember && remember->isChecked();

  if ( !bRemember && m_pService)
  {
    accept();
    return;
  }

  if (!bRemember && !d->saveNewApps)
  {
    // Create temp service
    m_pService = new KService(initialServiceName, fullExec, TQString::null);
    if (terminal->isChecked())
    {
      m_pService->setTerminal(true);
      // only add --noclose when we are sure it is konsole we're using
      if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
         m_pService->setTerminalOptions("--noclose");
    }
    accept();
    return;
  }

  // if we got here, we can't seem to find a service for what they
  // wanted.  The other possibility is that they have asked for the
  // association to be remembered.  Create/update service.

  TQString newPath;
  TQString oldPath;
  TQString menuId;
  if (m_pService)
  {
    oldPath = m_pService->desktopEntryPath();
    newPath = m_pService->locateLocal();
    menuId = m_pService->menuId();
    kdDebug(250) << "Updating exitsing service " << m_pService->desktopEntryPath() << " ( " << newPath << " ) " << endl;
  }
  else
  {
    newPath = KService::newServicePath(false /* hidden */, serviceName, &menuId);
    kdDebug(250) << "Creating new service " << serviceName << " ( " << newPath << " ) " << endl;
  }

  int maxPreference = 1;
  if (!qServiceType.isEmpty())
  {
    KServiceTypeProfile::OfferList offerList = KServiceTypeProfile::offers( qServiceType );
    if (!offerList.isEmpty())
      maxPreference = offerList.first().preference();
  }

  KDesktopFile *desktop = 0;
  if (!oldPath.isEmpty() && (oldPath != newPath))
  {
     KDesktopFile orig(oldPath, true);
     desktop = orig.copyTo(newPath);
  }
  else
  {
     desktop = new KDesktopFile(newPath);
  }
  desktop->writeEntry("Type", TQString::fromLatin1("Application"));
  desktop->writeEntry("Name", initialServiceName);
  desktop->writePathEntry("Exec", fullExec);
  if (terminal->isChecked())
  {
    desktop->writeEntry("Terminal", true);
    // only add --noclose when we are sure it is konsole we're using
    if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
      desktop->writeEntry("TerminalOptions", "--noclose");
  }
  else
  {
    desktop->writeEntry("Terminal", false);
  }
  desktop->writeEntry("X-TDE-InitialPreference", maxPreference + 1);


  if (bRemember || d->saveNewApps)
  {
    TQStringList mimeList = desktop->readListEntry("MimeType", ';');
    if (!qServiceType.isEmpty() && !mimeList.contains(qServiceType))
      mimeList.append(qServiceType);
    desktop->writeEntry("MimeType", mimeList, ';');

    if ( !qServiceType.isEmpty() )
    {
      // Also make sure the "auto embed" setting for this mimetype is off
      KDesktopFile mimeDesktop( locateLocal( "mime", qServiceType + ".desktop" ) );
      mimeDesktop.writeEntry( "X-TDE-AutoEmbed", false );
      mimeDesktop.sync();
    }
  }

  // write it all out to the file
  desktop->sync();
  delete desktop;

  KService::rebuildKSycoca(this);

  m_pService = KService::serviceByMenuId( menuId );

  Q_ASSERT( m_pService );

  accept();
}

TQString KOpenWithDlg::text() const
{
    if (!m_command.isEmpty())
        return m_command;
    else
        return edit->url();
}

void KOpenWithDlg::hideNoCloseOnExit()
{
    // uncheck the checkbox because the value could be used when "Run in Terminal" is selected
    nocloseonexit->setChecked( false );
    nocloseonexit->hide();
}

void KOpenWithDlg::hideRunInTerminal()
{
    terminal->hide();
    hideNoCloseOnExit();
}

void KOpenWithDlg::accept()
{
    KHistoryCombo *combo = static_cast<KHistoryCombo*>( edit->comboBox() );
    if ( combo ) {
        combo->addToHistory( edit->url() );

        TDEConfig *kc = TDEGlobal::config();
        TDEConfigGroupSaver ks( kc, TQString::fromLatin1("Open-with settings") );
        kc->writeEntry( TQString::fromLatin1("History"), combo->historyItems() );
	kc->writeEntry(TQString::fromLatin1("CompletionMode"),
		       combo->completionMode());
        // don't store the completion-list, as it contains all of KURLCompletion's
        // executables
        kc->sync();
    }

    TQDialog::accept();
}


///////////////

#ifndef KDE_NO_COMPAT
bool KFileOpenWithHandler::displayOpenWithDialog( const KURL::List& urls )
{
    KOpenWithDlg l( urls, i18n("Open with:"), TQString::null, 0L );
    if ( l.exec() )
    {
      KService::Ptr service = l.service();
      if ( !!service )
        return KRun::run( *service, urls );

      kdDebug(250) << "No service set, running " << l.text() << endl;
      return KRun::run( l.text(), urls );
    }
    return false;
}
#endif

#include "kopenwith.moc"
#include "kopenwith_p.moc"

