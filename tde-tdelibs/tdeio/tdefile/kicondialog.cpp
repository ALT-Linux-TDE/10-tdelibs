/*
 *
 * This file is part of the KDE project, module tdefile.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *           (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include "kicondialog.h"

#include <config.h>

#include <assert.h>

#include <kiconviewsearchline.h>

#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kprogress.h>
#include <kiconview.h>
#include <tdefiledialog.h>
#include <kimagefilepreview.h>

#include <tqlayout.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqsortedlist.h>
#include <tqimage.h>
#include <tqpixmap.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqtimer.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>
#include <tqfileinfo.h>
#include <tqtoolbutton.h>
#include <tqwhatsthis.h>

#ifdef HAVE_LIBART
#include <svgicons/ksvgiconengine.h>
#include <svgicons/ksvgiconpainter.h>
#endif

class TDEIconCanvas::TDEIconCanvasPrivate
{
  public:
    TDEIconCanvasPrivate() { m_bLoading = false; }
    ~TDEIconCanvasPrivate() {}
    bool m_bLoading;
};

/**
 * Helper class for sorting icon paths by icon name
 */
class IconPath : public TQString
{
protected:
 TQString m_iconName;

public:
 IconPath(const TQString &ip) : TQString (ip)
 {
   int n = findRev('/');
   m_iconName = (n==-1) ? static_cast<TQString>(*this) : mid(n+1);
 }


 IconPath() : TQString ()
 { }

 bool operator== (const IconPath &ip) const
 { return m_iconName == ip.m_iconName; }

 bool operator< (const IconPath &ip) const
 { return m_iconName < ip.m_iconName; }

};

/*
 * TDEIconCanvas: Iconview for the iconloader dialog.
 */

TDEIconCanvas::TDEIconCanvas(TQWidget *parent, const char *name)
    : TDEIconView(parent, name)
{
    d = new TDEIconCanvasPrivate;
    mpTimer = new TQTimer(this);
    connect(mpTimer, TQ_SIGNAL(timeout()), TQ_SLOT(slotLoadFiles()));
    connect(this, TQ_SIGNAL(currentChanged(TQIconViewItem *)),
	    TQ_SLOT(slotCurrentChanged(TQIconViewItem *)));
    setGridX(80);
    setWordWrapIconText(false);
    setShowToolTips(true);
}

TDEIconCanvas::~TDEIconCanvas()
{
    delete mpTimer;
    delete d;
}

void TDEIconCanvas::loadFiles(const TQStringList& files)
{
    clear();
    mFiles = files;
    emit startLoading(mFiles.count());
    mpTimer->start(10, true); // #86680
    d->m_bLoading = false;
}

void TDEIconCanvas::slotLoadFiles()
{
    setResizeMode(Fixed);
    TQApplication::setOverrideCursor(TQt::waitCursor);

    // disable updates to not trigger paint events when adding child items
    setUpdatesEnabled( false );

#ifdef HAVE_LIBART
    KSVGIconEngine *svgEngine = new KSVGIconEngine();
#endif

    d->m_bLoading = true;
    int i;
    TQStringList::ConstIterator it;
    uint emitProgress = 10; // so we will emit it once in the beginning
    TQStringList::ConstIterator end(mFiles.end());
    for (it=mFiles.begin(), i=0; it!=end; ++it, i++)
    {
	// Calling kapp->processEvents() makes the iconview flicker like hell
	// (it's being repainted once for every new item), so we don't do this.
	// Instead, we directly repaint the progress bar without going through
	// the event-loop. We do that just once for every 10th item so that
	// the progress bar doesn't flicker in turn. (pfeiffer)
	if ( emitProgress >= 10 ) {
	    emit progress(i);
            emitProgress = 0;
        }

        emitProgress++;
//	kapp->processEvents();
        if ( !d->m_bLoading ) // user clicked on a button that will load another set of icons
            break;
	TQImage img;

	// Use the extension as the format. Works for XPM and PNG, but not for SVG
	TQString path= *it;
	TQString ext = path.right(3).upper();

	if (ext != "SVG" && ext != "VGZ")
	    img.load(*it);
#ifdef HAVE_LIBART
	else
	    if (svgEngine->load(60, 60, *it))
		img = *svgEngine->painter()->image();
#endif

	if (img.isNull())
	    continue;
	if (img.width() > 60 || img.height() > 60)
	{
	    if (img.width() > img.height())
	    {
		int height = (int) ((60.0 / img.width()) * img.height());
		img = img.smoothScale(60, height);
	    } else
	    {
		int width = (int) ((60.0 / img.height()) * img.width());
		img = img.smoothScale(width, 60);
	    }
	}
	TQPixmap pm;
	pm.convertFromImage(img);
	TQFileInfo fi(*it);
	TQIconViewItem *item = new TQIconViewItem(this, fi.baseName(true), pm);
	item->setKey(*it);
	item->setDragEnabled(false);
	item->setDropEnabled(false);
    }

#ifdef HAVE_LIBART
    delete svgEngine;
#endif

    // enable updates since we have to draw the whole view now
    setUpdatesEnabled( true );

    TQApplication::restoreOverrideCursor();
    d->m_bLoading = false;
    emit finished();
    setResizeMode(Adjust);
}

TQString TDEIconCanvas::getCurrent() const
{
    if (!currentItem())
	return TQString::null;
    return currentItem()->key();
}

void TDEIconCanvas::stopLoading()
{
    d->m_bLoading = false;
}

void TDEIconCanvas::slotCurrentChanged(TQIconViewItem *item)
{
    emit nameChanged((item != 0L) ? item->text() : TQString::null);
}

class TDEIconDialog::TDEIconDialogPrivate
{
  public:
    TDEIconDialogPrivate() {
        m_bStrictIconSize = true;
	m_bLockUser = false;
	m_bLockCustomDir = false;
	searchLine = 0;
    }
    ~TDEIconDialogPrivate() {}
    bool m_bStrictIconSize, m_bLockUser, m_bLockCustomDir;
    TQString custom;
    TQString customLocation;
    TDEIconViewSearchLine *searchLine;
};

/*
 * TDEIconDialog: Dialog for selecting icons. Both system and user
 * specified icons can be chosen.
 */

TDEIconDialog::TDEIconDialog(TQWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Ok|Cancel, Ok)
{
    d = new TDEIconDialogPrivate;
    mpLoader = TDEGlobal::iconLoader();
    init();
}

TDEIconDialog::TDEIconDialog(TDEIconLoader *loader, TQWidget *parent,
	const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Ok|Cancel, Ok)
{
    d = new TDEIconDialogPrivate;
    mpLoader = loader;
    init();
}

void TDEIconDialog::init()
{
    mGroupOrSize = TDEIcon::Desktop;
    mContext = TDEIcon::Any;
    mType = 0;
    mFileList = TDEGlobal::dirs()->findAllResources("appicon", TQString::fromLatin1("*.png"));

    TQWidget *main = new TQWidget( this );
    setMainWidget(main);

    TQVBoxLayout *top = new TQVBoxLayout(main);
    top->setSpacing( spacingHint() );

    TQButtonGroup *bgroup = new TQButtonGroup(0, TQt::Vertical, i18n("Icon Source"), main);
    bgroup->layout()->setSpacing(KDialog::spacingHint());
    bgroup->layout()->setMargin(KDialog::marginHint());
    top->addWidget(bgroup);
    connect(bgroup, TQ_SIGNAL(clicked(int)), TQ_SLOT(slotButtonClicked(int)));
    TQGridLayout *grid = new TQGridLayout(bgroup->layout(), 3, 2);
    mpRb1 = new TQRadioButton(i18n("S&ystem icons:"), bgroup);
    grid->addWidget(mpRb1, 1, 0);
    mpCombo = new TQComboBox(bgroup);
    connect(mpCombo, TQ_SIGNAL(activated(int)), TQ_SLOT(slotContext(int)));
    grid->addWidget(mpCombo, 1, 1);
    mpRb2 = new TQRadioButton(i18n("O&ther icons:"), bgroup);
    grid->addWidget(mpRb2, 2, 0);
    mpBrowseBut = new TQPushButton(i18n("&Browse..."), bgroup);
    grid->addWidget(mpBrowseBut, 2, 1);

    //
    // ADD SEARCHLINE
    //
    TQHBoxLayout *searchLayout = new TQHBoxLayout(0, 0, KDialog::spacingHint());
    top->addLayout(searchLayout);

    TQToolButton *clearSearch = new TQToolButton(main);
    clearSearch->setTextLabel(i18n("Clear Search"), true);
    clearSearch->setIconSet(SmallIconSet(TQApplication::reverseLayout() ? "clear_left" :"locationbar_erase"));
    searchLayout->addWidget(clearSearch);

    TQLabel *searchLabel = new TQLabel(i18n("&Search:"), main);
    searchLayout->addWidget(searchLabel);

    d->searchLine = new TDEIconViewSearchLine(main, "searchLine");
    searchLayout->addWidget(d->searchLine);
    searchLabel->setBuddy(d->searchLine);


    // signals and slots connections
    connect(clearSearch, TQ_SIGNAL(clicked()), d->searchLine, TQ_SLOT(clear()));

    TQString wtstr = i18n("Search interactively for icon names (e.g. folder).");
    TQWhatsThis::add(searchLabel, wtstr);
    TQWhatsThis::add(d->searchLine, wtstr);


    mpCanvas = new TDEIconCanvas(main);
    connect(mpCanvas, TQ_SIGNAL(executed(TQIconViewItem *)), TQ_SLOT(slotAcceptIcons()));
    connect(mpCanvas, TQ_SIGNAL(returnPressed(TQIconViewItem *)), TQ_SLOT(slotAcceptIcons()));
    mpCanvas->setMinimumSize(400, 125);
    top->addWidget(mpCanvas);
    d->searchLine->setIconView(mpCanvas);

    mpProgress = new KProgress(main);
    top->addWidget(mpProgress);
    connect(mpCanvas, TQ_SIGNAL(startLoading(int)), TQ_SLOT(slotStartLoading(int)));
    connect(mpCanvas, TQ_SIGNAL(progress(int)), TQ_SLOT(slotProgress(int)));
    connect(mpCanvas, TQ_SIGNAL(finished()), TQ_SLOT(slotFinished()));

    // When pressing Ok or Cancel, stop loading icons
    connect(this, TQ_SIGNAL(hidden()), mpCanvas, TQ_SLOT(stopLoading()));

    static const char* const context_text[] = {
        I18N_NOOP( "Actions" ),
        I18N_NOOP( "Animations" ),
        I18N_NOOP( "Applications" ),
        I18N_NOOP( "Categories" ),
        I18N_NOOP( "Devices" ),
        I18N_NOOP( "Emblems" ),
        I18N_NOOP( "Emotes" ),
        I18N_NOOP( "Filesystems" ),
        I18N_NOOP( "International" ),
        I18N_NOOP( "Mimetypes" ),
        I18N_NOOP( "Places" ),
        I18N_NOOP( "Status" ) };
    static const TDEIcon::Context context_id[] = {
        TDEIcon::Action,
        TDEIcon::Animation,
        TDEIcon::Application,
        TDEIcon::Category,
        TDEIcon::Device,
        TDEIcon::Emblem,
        TDEIcon::Emote,
        TDEIcon::FileSystem,
        TDEIcon::International,
        TDEIcon::MimeType,
        TDEIcon::Place,
        TDEIcon::StatusIcon };
    mNumContext = 0;
    int cnt = sizeof( context_text ) / sizeof( context_text[ 0 ] );
    // check all 3 arrays have same sizes
    assert( cnt == sizeof( context_id ) / sizeof( context_id[ 0 ] )
            && cnt == sizeof( mContextMap ) / sizeof( mContextMap[ 0 ] ));
    for( int i = 0;
         i < cnt;
         ++i )
    {
        if( mpLoader->hasContext( context_id[ i ] ))
        {
            mpCombo->insertItem(i18n( context_text[ i ] ));
            mContextMap[ mNumContext++ ] = context_id[ i ];
        }
    }
    mpCombo->setFixedSize(mpCombo->sizeHint());

    mpBrowseBut->setFixedWidth(mpCombo->width());

    // Make the dialog a little taller
    incInitialSize(TQSize(0,100));
}


TDEIconDialog::~TDEIconDialog()
{
    delete d;
}

void TDEIconDialog::slotAcceptIcons()
{
  d->custom=TQString::null;
  slotOk();
}

void TDEIconDialog::showIcons()
{
    mpCanvas->clear();
    TQStringList filelist;
    if (mType == 0)
	if (d->m_bStrictIconSize)
            filelist=mpLoader->queryIcons(mGroupOrSize, mContext);
        else
            filelist=mpLoader->queryIconsByContext(mGroupOrSize, mContext);
    else if ( !d->customLocation.isNull() )
	filelist=mpLoader->queryIconsByDir( d->customLocation );
    else
	filelist=mFileList;

    TQSortedList <IconPath>iconlist;
    iconlist.setAutoDelete(true);
    TQStringList::Iterator it;
    for( it = filelist.begin(); it != filelist.end(); ++it )
       iconlist.append(new IconPath(*it));

    iconlist.sort();
    filelist.clear();

    for ( IconPath *ip=iconlist.first(); ip != 0; ip=iconlist.next() )
       filelist.append(*ip);

    d->searchLine->clear();
    mpCanvas->loadFiles(filelist);
}

void TDEIconDialog::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

bool TDEIconDialog::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void TDEIconDialog::setIconSize( int size )
{
    // see TDEIconLoader, if you think this is weird
    if ( size == 0 )
        mGroupOrSize = TDEIcon::Desktop; // default Group
    else
        mGroupOrSize = -size; // yes, TDEIconLoader::queryIconsByContext is weird
}

int TDEIconDialog::iconSize() const
{
    // 0 or any other value ==> mGroupOrSize is a group, so we return 0
    return (mGroupOrSize < 0) ? -mGroupOrSize : 0;
}

#ifndef KDE_NO_COMPAT
TQString TDEIconDialog::selectIcon(TDEIcon::Group group, TDEIcon::Context context, bool user)
{
    setup( group, context, false, 0, user );
    return openDialog();
}
#endif

void TDEIconDialog::setup(TDEIcon::Group group, TDEIcon::Context context,
                        bool strictIconSize, int iconSize, bool user )
{
    d->m_bStrictIconSize = strictIconSize;
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mType = user ? 1 : 0;
    mpRb1->setChecked(!user);
    mpRb2->setChecked(user);
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled(user);
    setContext( context );
}

void TDEIconDialog::setup(TDEIcon::Group group, TDEIcon::Context context,
                        bool strictIconSize, int iconSize, bool user,
                        bool lockUser, bool lockCustomDir )
{
    d->m_bStrictIconSize = strictIconSize;
    d->m_bLockUser = lockUser;
    d->m_bLockCustomDir = lockCustomDir;
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mType = user ? 1 : 0;
    mpRb1->setChecked(!user);
    mpRb1->setEnabled( !lockUser || !user );
    mpRb2->setChecked(user);
    mpRb2->setEnabled( !lockUser || user );
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled( user && !lockCustomDir );
    setContext( context );
}

void TDEIconDialog::setContext( TDEIcon::Context context )
{
    mContext = context;
    for( int i = 0;
         i < mNumContext;
         ++i )
        if( mContextMap[ i ] == context )
        {
            mpCombo->setCurrentItem( i );
            return;
        }
}

void TDEIconDialog::setCustomLocation( const TQString& location )
{
    d->customLocation = location;
}

TQString TDEIconDialog::openDialog()
{
    showIcons();

    if ( exec() == Accepted )
    {
        if (!d->custom.isNull())
            return d->custom;
	TQString name = mpCanvas->getCurrent();
	if (name.isEmpty() || (mType == 1))
	    return name;
	TQFileInfo fi(name);
	return fi.baseName(true);
    }
    return TQString::null;
}

void TDEIconDialog::showDialog()
{
    setModal(false);
    showIcons();
    show();
}

void TDEIconDialog::slotOk()
{
    TQString name;
    if (!d->custom.isNull())
    {
        name = d->custom;
    }
    else
    {
        name = mpCanvas->getCurrent();
        if (!name.isEmpty() && (mType != 1))
        {
            TQFileInfo fi(name);
            name = fi.baseName(true);
        }
    }

    emit newIconName(name);
    KDialogBase::slotOk();
}

TQString TDEIconDialog::getIcon(TDEIcon::Group group, TDEIcon::Context context,
                             bool strictIconSize, int iconSize, bool user,
                             TQWidget *parent, const TQString &caption)
{
    TDEIconDialog dlg(parent, "icon dialog");
    dlg.setup( group, context, strictIconSize, iconSize, user );
    if (!caption.isNull())
        dlg.setCaption(caption);

    return dlg.openDialog();
}

void TDEIconDialog::slotButtonClicked(int id)
{
    TQString file;

    switch (id)
    {
    case 0:
        if(mType!=0)
        {
            mType = 0;
            mpBrowseBut->setEnabled(false);
            mpCombo->setEnabled(true);
            showIcons();
        }
	break;

    case 1:
        if(mType!=1)
        {
            mType = 1;
            mpBrowseBut->setEnabled( !d->m_bLockCustomDir );
            mpCombo->setEnabled(false);
            showIcons();
        }
        break;
    case 2:
        {
            // Create a file dialog to select a PNG, XPM or SVG file,
            // with the image previewer shown.
            // KFileDialog::getImageOpenURL doesn't allow svg.
            KFileDialog dlg(TQString::null, i18n("*.png *.xpm *.svg *.svgz|Icon Files (*.png *.xpm *.svg *.svgz)"),
                            this, "filedialog", true);
            dlg.setOperationMode( KFileDialog::Opening );
            dlg.setCaption( i18n("Open") );
            dlg.setMode( KFile::File );

            KImageFilePreview *ip = new KImageFilePreview( &dlg );
            dlg.setPreviewWidget( ip );
            dlg.exec();

            file = dlg.selectedFile();
            if (!file.isEmpty())
            {
                d->custom = file;
                if ( mType == 1 )
                  d->customLocation = TQFileInfo( file ).dirPath( true );
                slotOk();
            }
        }
        break;
    }
}

void TDEIconDialog::slotContext(int id)
{
    mContext = static_cast<TDEIcon::Context>( mContextMap[ id ] );
    showIcons();
}

void TDEIconDialog::slotStartLoading(int steps)
{
    if (steps < 10)
	mpProgress->hide();
    else
    {
        mpProgress->setTotalSteps(steps);
        mpProgress->setProgress(0);
        mpProgress->show();
    }
}

void TDEIconDialog::slotProgress(int p)
{
    mpProgress->setProgress(p);
    // commented out the following since setProgress already paints ther
    // progress bar. ->repaint() only makes it flicker
    //mpProgress->repaint();
}

void TDEIconDialog::slotFinished()
{
    mpProgress->hide();
}

class TDEIconButton::TDEIconButtonPrivate
{
  public:
    TDEIconButtonPrivate() {
        m_bStrictIconSize = false;
        iconSize = 0; // let TDEIconLoader choose the default
    }
    ~TDEIconButtonPrivate() {}
    bool m_bStrictIconSize;
    int iconSize;
};


/*
 * TDEIconButton: A "choose icon" pushbutton.
 */

TDEIconButton::TDEIconButton(TQWidget *parent, const char *name)
    : TQPushButton(parent, name)
{
    init( TDEGlobal::iconLoader() );
}

TDEIconButton::TDEIconButton(TDEIconLoader *loader,
	TQWidget *parent, const char *name)
    : TQPushButton(parent, name)
{
    init( loader );
}

void TDEIconButton::init( TDEIconLoader *loader )
{
    d = new TDEIconButtonPrivate;
    mGroup = TDEIcon::Desktop;
    mContext = TDEIcon::Application;
    mbUser = false;

    mpLoader = loader;
    mpDialog = 0L;
    connect(this, TQ_SIGNAL(clicked()), TQ_SLOT(slotChangeIcon()));
}

TDEIconButton::~TDEIconButton()
{
    delete mpDialog;
    delete d;
}

void TDEIconButton::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

bool TDEIconButton::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void TDEIconButton::setIconSize( int size )
{
    d->iconSize = size;
}

int TDEIconButton::iconSize() const
{
    return d->iconSize;
}

void TDEIconButton::setIconType(TDEIcon::Group group, TDEIcon::Context context, bool user)
{
    mGroup = group;
    mContext = context;
    mbUser = user;
}

void TDEIconButton::setIcon(const TQString& icon)
{
    mIcon = icon;
    setIconSet(mpLoader->loadIconSet(mIcon, mGroup, d->iconSize));

    if (!mpDialog)
    {
        mpDialog = new TDEIconDialog(mpLoader, this);
        connect(mpDialog, TQ_SIGNAL(newIconName(const TQString&)), TQ_SLOT(newIconName(const TQString&)));
    }

    if ( mbUser )
      mpDialog->setCustomLocation( TQFileInfo( mpLoader->iconPath(mIcon, mGroup, true) ).dirPath( true ) );
}

void TDEIconButton::resetIcon()
{
    mIcon = TQString::null;
    setIconSet(TQIconSet());
}

void TDEIconButton::slotChangeIcon()
{
    if (!mpDialog)
    {
        mpDialog = new TDEIconDialog(mpLoader, this);
        connect(mpDialog, TQ_SIGNAL(newIconName(const TQString&)), TQ_SLOT(newIconName(const TQString&)));
    }

    mpDialog->setup( mGroup, mContext, d->m_bStrictIconSize, d->iconSize, mbUser );
    mpDialog->showDialog();
}

void TDEIconButton::newIconName(const TQString& name)
{
    if (name.isEmpty())
        return;

    TQIconSet iconset = mpLoader->loadIconSet(name, mGroup, d->iconSize);
    setIconSet(iconset);
    mIcon = name;

    if ( mbUser )
      mpDialog->setCustomLocation( TQFileInfo( mpLoader->iconPath(mIcon, mGroup, true) ).dirPath( true ) );

    emit iconChanged(name);
}

void TDEIconCanvas::virtual_hook( int id, void* data )
{ TDEIconView::virtual_hook( id, data ); }

void TDEIconDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kicondialog.moc"
