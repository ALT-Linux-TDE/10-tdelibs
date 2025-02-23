/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Stephan Kulow <coolo@kde.org>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <unistd.h>

#include <tqdir.h>
#include <tqapplication.h>
#include <tqdialog.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqpopupmenu.h>
#include <tqregexp.h>
#include <tqtimer.h>
#include <tqvbox.h>

#include <tdeaction.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kdirlister.h>
#include <kinputdialog.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tdepopupmenu.h>
#include <kprogress.h>
#include <kstdaction.h>
#include <tdeio/job.h>
#include <tdeio/jobclasses.h>
#include <tdeio/netaccess.h>
#include <tdeio/previewjob.h>
#include <tdeio/renamedlg.h>
#include <kpropertiesdialog.h>
#include <kservicetypefactory.h>
#include <tdestdaccel.h>
#include <kde_file.h>

#include "config-tdefile.h"
#include "kcombiview.h"
#include "tdediroperator.h"
#include "tdefiledetailview.h"
#include "tdefileiconview.h"
#include "tdefilepreview.h"
#include "tdefileview.h"
#include "tdefileitem.h"
#include "tdefilemetapreview.h"


template class TQPtrStack<KURL>;
template class TQDict<KFileItem>;


class KDirOperator::KDirOperatorPrivate
{
public:
    KDirOperatorPrivate() {
        onlyDoubleClickSelectsFiles = false;
        progressDelayTimer = 0L;
        dirHighlighting = false;
        config = 0L;
        dropOptions = 0;
    }

    ~KDirOperatorPrivate() {
        delete progressDelayTimer;
    }

    bool dirHighlighting;
    TQString lastURL; // used for highlighting a directory on cdUp
    bool onlyDoubleClickSelectsFiles;
    TQTimer *progressDelayTimer;
    TDEActionSeparator *viewActionSeparator;
    int dropOptions;

    TDEConfig *config;
    TQString configGroup;
};

KDirOperator::KDirOperator(const KURL& _url,
                           TQWidget *parent, const char* _name)
    : TQWidget(parent, _name),
      dir(0),
      m_fileView(0),
      progress(0)
{
    myPreview = 0L;
    myMode = KFile::File;
    m_viewKind = KFile::Simple;
    mySorting = static_cast<TQDir::SortSpec>(TQDir::Name | TQDir::DirsFirst);
    d = new KDirOperatorPrivate;

    if (_url.isEmpty()) { // no dir specified -> current dir
        TQString strPath = TQDir::currentDirPath();
        strPath.append('/');
        currUrl = KURL();
        currUrl.setProtocol(TQString::fromLatin1("file"));
        currUrl.setPath(strPath);
    }
    else {
        currUrl = _url;
        if ( currUrl.protocol().isEmpty() )
            currUrl.setProtocol(TQString::fromLatin1("file"));

        currUrl.addPath("/"); // make sure we have a trailing slash!
    }

    setDirLister( new KDirLister( true ) );

    connect(&myCompletion, TQ_SIGNAL(match(const TQString&)),
            TQ_SLOT(slotCompletionMatch(const TQString&)));

    progress = new KProgress(this, "progress");
    progress->adjustSize();
    progress->move(2, height() - progress->height() -2);

    d->progressDelayTimer = new TQTimer( this, "progress delay timer" );
    connect( d->progressDelayTimer, TQ_SIGNAL( timeout() ),
	     TQ_SLOT( slotShowProgress() ));

    myCompleteListDirty = false;

    backStack.setAutoDelete( true );
    forwardStack.setAutoDelete( true );

    // action stuff
    setupActions();
    setupMenu();

    setFocusPolicy(TQWidget::WheelFocus);

    installEventFilter(this);
}

KDirOperator::~KDirOperator()
{
    resetCursor();
    if ( m_fileView )
    {
        if ( d->config )
            m_fileView->writeConfig( d->config, d->configGroup );

        delete m_fileView;
        m_fileView = 0L;
    }

    delete myPreview;
    delete dir;
    delete d;
}


void KDirOperator::setSorting( TQDir::SortSpec spec )
{
    if ( m_fileView )
        m_fileView->setSorting( spec );
    mySorting = spec;
    updateSortActions();
}

void KDirOperator::resetCursor()
{
   TQApplication::restoreOverrideCursor();
   progress->hide();
}

void KDirOperator::insertViewDependentActions()
{
   // If we have a new view actionCollection(), insert its actions
   // into viewActionMenu.

   if( !m_fileView )
      return;

   if ( (viewActionMenu->popupMenu()->count() == 0) || 			// Not yet initialized or...
        (viewActionCollection != m_fileView->actionCollection()) )	// ...changed since.
   {
      if (viewActionCollection)
      {
         disconnect( viewActionCollection, TQ_SIGNAL( inserted( TDEAction * )),
               this, TQ_SLOT( slotViewActionAdded( TDEAction * )));
         disconnect( viewActionCollection, TQ_SIGNAL( removed( TDEAction * )),
               this, TQ_SLOT( slotViewActionRemoved( TDEAction * )));
      }

      viewActionMenu->popupMenu()->clear();
//      viewActionMenu->insert( shortAction );
//      viewActionMenu->insert( detailedAction );
//      viewActionMenu->insert( actionSeparator );
      viewActionMenu->insert( myActionCollection->action( "short view" ) );
      viewActionMenu->insert( myActionCollection->action( "detailed view" ) );
      viewActionMenu->insert( actionSeparator );
      viewActionMenu->insert( showHiddenAction );
//      viewActionMenu->insert( myActionCollection->action( "single" ));
      viewActionMenu->insert( separateDirsAction );
      // Warning: adjust slotViewActionAdded() and slotViewActionRemoved()
      // when you add/remove actions here!

      viewActionCollection = m_fileView->actionCollection();
      if (!viewActionCollection)
         return;

      if ( !viewActionCollection->isEmpty() )
      {
         viewActionMenu->insert( d->viewActionSeparator );

         // first insert the normal actions, then the grouped ones
         TQStringList groups = viewActionCollection->groups();
         groups.prepend( TQString::null ); // actions without group
         TQStringList::ConstIterator git = groups.begin();
         TDEActionPtrList list;
         TDEAction *sep = actionCollection()->action("separator");
         for ( ; git != groups.end(); ++git )
         {
            if ( git != groups.begin() )
               viewActionMenu->insert( sep );

            list = viewActionCollection->actions( *git );
            TDEActionPtrList::ConstIterator it = list.begin();
            for ( ; it != list.end(); ++it )
               viewActionMenu->insert( *it );
         }
      }

      connect( viewActionCollection, TQ_SIGNAL( inserted( TDEAction * )),
               TQ_SLOT( slotViewActionAdded( TDEAction * )));
      connect( viewActionCollection, TQ_SIGNAL( removed( TDEAction * )),
               TQ_SLOT( slotViewActionRemoved( TDEAction * )));
   }
}

void KDirOperator::activatedMenu( const KFileItem *, const TQPoint& pos )
{
    setupMenu();
    updateSelectionDependentActions();

    actionMenu->popup( pos );
}

void KDirOperator::updateSelectionDependentActions()
{
    bool hasSelection = m_fileView && m_fileView->selectedItems() &&
                        !m_fileView->selectedItems()->isEmpty();
    myActionCollection->action( "trash" )->setEnabled( hasSelection );
    myActionCollection->action( "delete" )->setEnabled( hasSelection );
    myActionCollection->action( "properties" )->setEnabled( hasSelection );
}

void KDirOperator::setPreviewWidget(const TQWidget *w)
{
    if(w != 0L)
        m_viewKind = (m_viewKind | KFile::PreviewContents);
    else
        m_viewKind = (m_viewKind & ~KFile::PreviewContents);

    delete myPreview;
    myPreview = w;

    TDEToggleAction *preview = static_cast<TDEToggleAction*>(myActionCollection->action("preview"));
    preview->setEnabled( w != 0L );
    preview->setChecked( w != 0L );
    setView( static_cast<KFile::FileView>(m_viewKind) );
}

int KDirOperator::numDirs() const
{
    return m_fileView ? m_fileView->numDirs() : 0;
}

int KDirOperator::numFiles() const
{
    return m_fileView ? m_fileView->numFiles() : 0;
}

void KDirOperator::slotDetailedView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (m_viewKind & ~KFile::Simple) | KFile::Detail );
    setView( view );
}

void KDirOperator::slotSimpleView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (m_viewKind & ~KFile::Detail) | KFile::Simple );
    setView( view );
}

void KDirOperator::slotToggleHidden( bool show )
{
    dir->setShowingDotFiles( show );
    updateDir();
    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::slotSeparateDirs()
{
    if (separateDirsAction->isChecked())
    {
        KFile::FileView view = static_cast<KFile::FileView>( m_viewKind | KFile::SeparateDirs );
        setView( view );
    }
    else
    {
        KFile::FileView view = static_cast<KFile::FileView>( m_viewKind & ~KFile::SeparateDirs );
        setView( view );
    }
}

void KDirOperator::slotDefaultPreview()
{
    m_viewKind = m_viewKind | KFile::PreviewContents;
    if ( !myPreview ) {
        myPreview = new KFileMetaPreview( this );
        (static_cast<TDEToggleAction*>( myActionCollection->action("preview") ))->setChecked(true);
    }

    setView( static_cast<KFile::FileView>(m_viewKind) );
}

void KDirOperator::slotSortByName()
{
    int sorting = (m_fileView->sorting()) & ~TQDir::SortByMask;
    m_fileView->setSorting( static_cast<TQDir::SortSpec>( sorting | TQDir::Name ));
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( true );
}

void KDirOperator::slotSortBySize()
{
    int sorting = (m_fileView->sorting()) & ~TQDir::SortByMask;
    m_fileView->setSorting( static_cast<TQDir::SortSpec>( sorting | TQDir::Size ));
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( false );
}

void KDirOperator::slotSortByDate()
{
    int sorting = (m_fileView->sorting()) & ~TQDir::SortByMask;
    m_fileView->setSorting( static_cast<TQDir::SortSpec>( sorting | TQDir::Time ));
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( false );
}

void KDirOperator::slotSortReversed()
{
    if ( m_fileView )
        m_fileView->sortReversed();
}

void KDirOperator::slotToggleDirsFirst()
{
    TQDir::SortSpec sorting = m_fileView->sorting();
    if ( !KFile::isSortDirsFirst( sorting ) )
        m_fileView->setSorting( static_cast<TQDir::SortSpec>( sorting | TQDir::DirsFirst ));
    else
        m_fileView->setSorting( static_cast<TQDir::SortSpec>( sorting & ~TQDir::DirsFirst));
    mySorting = m_fileView->sorting();
}

void KDirOperator::slotToggleIgnoreCase()
{
    TQDir::SortSpec sorting = m_fileView->sorting();
    if ( !KFile::isSortCaseInsensitive( sorting ) )
        m_fileView->setSorting( static_cast<TQDir::SortSpec>( sorting | TQDir::IgnoreCase ));
    else
        m_fileView->setSorting( static_cast<TQDir::SortSpec>( sorting & ~TQDir::IgnoreCase));
    mySorting = m_fileView->sorting();
}

void KDirOperator::mkdir()
{
    bool ok;
    TQString where = url().pathOrURL();
    TQString name = i18n( "New Folder" );
    if ( url().isLocalFile() && TQFileInfo( url().path(+1) + name ).exists() )
         name = TDEIO::RenameDlg::suggestName( url(), name );

    TQString dir = KInputDialog::getText( i18n( "New Folder" ),
                                         i18n( "Create new folder in:\n%1" ).arg( where ),
                                         name, &ok, this);
    if (ok)
      mkdir( TDEIO::encodeFileName( dir ), true );
}

bool KDirOperator::mkdir( const TQString& directory, bool enterDirectory )
{
    // Creates "directory", relative to the current directory (currUrl).
    // The given path may contain any number directories, existant or not.
    // They will all be created, if possible.

    bool writeOk = false;
    bool exists = false;
    KURL url( currUrl );

    TQStringList dirs = TQStringList::split( TQDir::separator(), directory );
    TQStringList::ConstIterator it = dirs.begin();

    for ( ; it != dirs.end(); ++it )
    {
        url.addPath( *it );
        exists = TDEIO::NetAccess::exists( url, false, 0 );
        writeOk = !exists && TDEIO::NetAccess::mkdir( url, topLevelWidget() );
    }

    if ( exists ) // url was already existant
    {
        KMessageBox::sorry(viewWidget(), i18n("A file or folder named %1 already exists.").arg(url.pathOrURL()));
        enterDirectory = false;
    }
    else if ( !writeOk ) {
        KMessageBox::sorry(viewWidget(), i18n("You do not have permission to "
                                              "create that folder." ));
    }
    else if ( enterDirectory ) {
        setURL( url, true );
    }

    return writeOk;
}

TDEIO::DeleteJob * KDirOperator::del( const KFileItemList& items,
                                    bool ask, bool showProgress )
{
    return del( items, this, ask, showProgress );
}

TDEIO::DeleteJob * KDirOperator::del( const KFileItemList& items,
                                    TQWidget *parent,
                                    bool ask, bool showProgress )
{
    if ( items.isEmpty() ) {
        KMessageBox::information( parent,
                                i18n("You did not select a file to delete."),
                                i18n("Nothing to Delete") );
        return 0L;
    }

    KURL::List urls;
    TQStringList files;
    KFileItemListIterator it( items );

    for ( ; it.current(); ++it ) {
        KURL url = (*it)->url();
        urls.append( url );
        if ( url.isLocalFile() )
            files.append( url.path() );
        else
            files.append( url.prettyURL() );
    }

    bool doIt = !ask;
    if ( ask ) {
        int ret;
        if ( items.count() == 1 ) {
            ret = KMessageBox::warningContinueCancel( parent,
                i18n( "<qt>Do you really want to delete\n <b>'%1'</b>?</qt>" )
                .arg( files.first() ),
                                                      i18n("Delete File"),
                                                      KStdGuiItem::del(), "AskForDelete" );
        }
        else
            ret = KMessageBox::warningContinueCancelList( parent,
                i18n("Do you really want to delete this item?", "Do you really want to delete these %n items?", items.count() ),
                                                    files,
                                                    i18n("Delete Files"),
                                                    KStdGuiItem::del(), "AskForDelete" );
        doIt = (ret == KMessageBox::Continue);
    }

    if ( doIt ) {
        TDEIO::DeleteJob *job = TDEIO::del( urls, false, showProgress );
        job->setWindow (topLevelWidget());
        job->setAutoErrorHandlingEnabled( true, parent );
        return job;
    }

    return 0L;
}

void KDirOperator::deleteSelected()
{
    if ( !m_fileView )
        return;

    const KFileItemList *list = m_fileView->selectedItems();
    if ( list )
        del( *list );
}

TDEIO::CopyJob * KDirOperator::trash( const KFileItemList& items,
                                    TQWidget *parent,
                                    bool ask, bool showProgress )
{
    if ( items.isEmpty() ) {
        KMessageBox::information( parent,
                                i18n("You did not select a file to trash."),
                                i18n("Nothing to Trash") );
        return 0L;
    }

    KURL::List urls;
    TQStringList files;
    KFileItemListIterator it( items );

    for ( ; it.current(); ++it ) {
        KURL url = (*it)->url();
        urls.append( url );
        if ( url.isLocalFile() )
            files.append( url.path() );
        else
            files.append( url.prettyURL() );
    }

    bool doIt = !ask;
    if ( ask ) {
        int ret;
        if ( items.count() == 1 ) {
            ret = KMessageBox::warningContinueCancel( parent,
                i18n( "<qt>Do you really want to trash\n <b>'%1'</b>?</qt>" )
                .arg( files.first() ),
                                                      i18n("Trash File"),
                                                      KGuiItem(i18n("to trash", "&Trash"),"edittrash"), "AskForTrash" );
        }
        else
            ret = KMessageBox::warningContinueCancelList( parent,
                i18n("translators: not called for n == 1", "Do you really want to trash these %n items?", items.count() ),
                                                    files,
                                                    i18n("Trash Files"),
                                                    KGuiItem(i18n("to trash", "&Trash"),"edittrash"), "AskForTrash" );
        doIt = (ret == KMessageBox::Continue);
    }

    if ( doIt ) {
        TDEIO::CopyJob *job = TDEIO::trash( urls, showProgress );
        job->setWindow (topLevelWidget());
        job->setAutoErrorHandlingEnabled( true, parent );
        return job;
    }

    return 0L;
}

void KDirOperator::trashSelected(TDEAction::ActivationReason reason, TQt::ButtonState state)
{
    if ( !m_fileView )
        return;

    if ( reason == TDEAction::PopupMenuActivation && ( state & ShiftButton ) ) {
        deleteSelected();
	return;
    }

    const KFileItemList *list = m_fileView->selectedItems();
    if ( list )
        trash( *list, this );
}

void KDirOperator::close()
{
    resetCursor();
    pendingMimeTypes.clear();
    myCompletion.clear();
    myDirCompletion.clear();
    myCompleteListDirty = true;
    dir->stop();
}

void KDirOperator::checkPath(const TQString &, bool /*takeFiles*/) // SLOT
{
#if 0
    // copy the argument in a temporary string
    TQString text = _txt;
    // it's unlikely to happen, that at the beginning are spaces, but
    // for the end, it happens quite often, I guess.
    text = text.stripWhiteSpace();
    // if the argument is no URL (the check is quite fragil) and it's
    // no absolute path, we add the current directory to get a correct url
    if (text.find(':') < 0 && text[0] != '/')
        text.insert(0, currUrl);

    // in case we have a selection defined and someone patched the file-
    // name, we check, if the end of the new name is changed.
    if (!selection.isNull()) {
        int position = text.findRev('/');
        ASSERT(position >= 0); // we already inserted the current dir in case
        TQString filename = text.mid(position + 1, text.length());
        if (filename != selection)
            selection = TQString::null;
    }

    KURL u(text); // I have to take care of entered URLs
    bool filenameEntered = false;

    if (u.isLocalFile()) {
        // the empty path is kind of a hack
        KFileItem i("", u.path());
        if (i.isDir())
            setURL(text, true);
        else {
            if (takeFiles)
                if (acceptOnlyExisting && !i.isFile())
                    warning("you entered an invalid URL");
                else
                    filenameEntered = true;
        }
    } else
        setURL(text, true);

    if (filenameEntered) {
        filename_ = u.url();
        emit fileSelected(filename_);

        TQApplication::restoreOverrideCursor();

        accept();
    }
#endif
    kdDebug(tdefile_area) << "TODO KDirOperator::checkPath()" << endl;
}

void KDirOperator::setURL(const KURL& _newurl, bool clearforward)
{
    KURL newurl;

    if ( !_newurl.isValid() )
	newurl.setPath( TQDir::homeDirPath() );
    else
	newurl = _newurl;

    TQString pathstr = newurl.path(+1);
    newurl.setPath(pathstr);

    // already set
    if ( newurl.equals( currUrl, true ) )
        return;

    if ( !isReadable( newurl ) ) {
        // maybe newurl is a file? check its parent directory
        newurl.cd(TQString::fromLatin1(".."));
        if ( !isReadable( newurl ) ) {
            resetCursor();
            KMessageBox::error(viewWidget(),
                               i18n("The specified folder does not exist "
                                    "or was not readable."));
            return;
        }
    }

    if (clearforward) {
        // autodelete should remove this one
        backStack.push(new KURL(currUrl));
        forwardStack.clear();
    }

    d->lastURL = currUrl.url(-1);
    currUrl = newurl;

    pathChanged();
    emit urlEntered(newurl);

    // enable/disable actions
    forwardAction->setEnabled( !forwardStack.isEmpty() );
    backAction->setEnabled( !backStack.isEmpty() );
    upAction->setEnabled( !isRoot() );

    openURL( newurl );
}

void KDirOperator::updateDir()
{
    dir->emitChanges();
    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::rereadDir()
{
    pathChanged();
    openURL( currUrl, false, true );
}


bool KDirOperator::openURL( const KURL& url, bool keep, bool reload )
{
    bool result = dir->openURL( url, keep, reload );
    if ( !result ) // in that case, neither completed() nor canceled() will be emitted by KDL
        slotCanceled();

    return result;
}

// Protected
void KDirOperator::pathChanged()
{
    if (!m_fileView)
        return;

    pendingMimeTypes.clear();
    m_fileView->clear();
    myCompletion.clear();
    myDirCompletion.clear();

    // it may be, that we weren't ready at this time
    TQApplication::restoreOverrideCursor();

    // when TDEIO::Job emits finished, the slot will restore the cursor
    TQApplication::setOverrideCursor( TQt::waitCursor );

    if ( !isReadable( currUrl )) {
        KMessageBox::error(viewWidget(),
                           i18n("The specified folder does not exist "
                                "or was not readable."));
        if (backStack.isEmpty())
            home();
        else
            back();
    }
}

void KDirOperator::slotRedirected( const KURL& newURL )
{
    currUrl = newURL;
    pendingMimeTypes.clear();
    myCompletion.clear();
    myDirCompletion.clear();
    myCompleteListDirty = true;
    emit urlEntered( newURL );
}

// Code pinched from kfm then hacked
void KDirOperator::back()
{
    if ( backStack.isEmpty() )
        return;

    forwardStack.push( new KURL(currUrl) );

    KURL *s = backStack.pop();

    setURL(*s, false);
    delete s;
}

// Code pinched from kfm then hacked
void KDirOperator::forward()
{
    if ( forwardStack.isEmpty() )
        return;

    backStack.push(new KURL(currUrl));

    KURL *s = forwardStack.pop();
    setURL(*s, false);
    delete s;
}

KURL KDirOperator::url() const
{
    return currUrl;
}

void KDirOperator::cdUp()
{
    KURL tmp(currUrl);
    tmp.cd(TQString::fromLatin1(".."));
    setURL(tmp, true);
}

void KDirOperator::home()
{
    KURL u;
    u.setPath( TQDir::homeDirPath() );
    setURL(u, true);
}

void KDirOperator::clearFilter()
{
    dir->setNameFilter( TQString::null );
    dir->clearMimeFilter();
    checkPreviewSupport();
}

void KDirOperator::setNameFilter(const TQString& filter)
{
    dir->setNameFilter(filter);
    checkPreviewSupport();
}

void KDirOperator::setMimeFilter( const TQStringList& mimetypes )
{
    dir->setMimeFilter( mimetypes );
    checkPreviewSupport();
}

bool KDirOperator::checkPreviewSupport()
{
    TDEToggleAction *previewAction = static_cast<TDEToggleAction*>( myActionCollection->action( "preview" ));

    bool hasPreviewSupport = false;
    TDEConfig *kc = TDEGlobal::config();
    TDEConfigGroupSaver cs( kc, ConfigGroup );
    if ( kc->readBoolEntry( "Show Default Preview", true ) )
        hasPreviewSupport = checkPreviewInternal();

    previewAction->setEnabled( hasPreviewSupport );
    return hasPreviewSupport;
}

bool KDirOperator::checkPreviewInternal() const
{
    TQStringList supported = TDEIO::PreviewJob::supportedMimeTypes();
    // no preview support for directories?
    if ( dirOnlyMode() && supported.findIndex( "inode/directory" ) == -1 )
        return false;

    TQStringList mimeTypes = dir->mimeFilters();
    TQStringList nameFilter = TQStringList::split( " ", dir->nameFilter() );

    if ( mimeTypes.isEmpty() && nameFilter.isEmpty() && !supported.isEmpty() )
        return true;
    else {
        TQRegExp r;
        r.setWildcard( true ); // the "mimetype" can be "image/*"

        if ( !mimeTypes.isEmpty() ) {
            TQStringList::Iterator it = supported.begin();

            for ( ; it != supported.end(); ++it ) {
                r.setPattern( *it );

                TQStringList result = mimeTypes.grep( r );
                if ( !result.isEmpty() ) { // matches! -> we want previews
                    return true;
                }
            }
        }

        if ( !nameFilter.isEmpty() ) {
            // find the mimetypes of all the filter-patterns and
            KServiceTypeFactory *fac = KServiceTypeFactory::self();
            TQStringList::Iterator it1 = nameFilter.begin();
            for ( ; it1 != nameFilter.end(); ++it1 ) {
                if ( (*it1) == "*" ) {
                    return true;
                }

                KMimeType *mt = fac->findFromPattern( *it1 );
                if ( !mt )
                    continue;
                TQString mime = mt->name();
                delete mt;

                // the "mimetypes" we get from the PreviewJob can be "image/*"
                // so we need to check in wildcard mode
                TQStringList::Iterator it2 = supported.begin();
                for ( ; it2 != supported.end(); ++it2 ) {
                    r.setPattern( *it2 );
                    if ( r.search( mime ) != -1 ) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

KFileView* KDirOperator::createView( TQWidget* parent, KFile::FileView view )
{
    KFileView* new_view = 0L;
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview = ( KFile::isPreviewInfo(view) || KFile::isPreviewContents( view ) );

    if ( separateDirs || preview ) {
        KCombiView *combi = 0L;
        if (separateDirs)
        {
            combi = new KCombiView( parent, "combi view" );
            combi->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
        }

        KFileView* v = 0L;
        if ( KFile::isSimpleView( view ) )
            v = createView( combi, KFile::Simple );
        else
            v = createView( combi, KFile::Detail );

        v->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);

        if (combi)
            combi->setRight( v );

        if (preview)
        {
            KFilePreview* pView = new KFilePreview( combi ? combi : v, parent, "preview" );
            pView->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
            new_view = pView;
        }
        else
            new_view = combi;
    }
    else if ( KFile::isDetailView( view ) && !preview ) {
        new_view = new KFileDetailView( parent, "detail view");
        new_view->setViewName( i18n("Detailed View") );
    }
    else /* if ( KFile::isSimpleView( view ) && !preview ) */ {
        KFileIconView *iconView =  new KFileIconView( parent, "simple view");
        new_view = iconView;
        new_view->setViewName( i18n("Short View") );
    }

    new_view->widget()->setAcceptDrops(acceptDrops());
    return new_view;
}

void KDirOperator::setAcceptDrops(bool b)
{
    if (m_fileView)
       m_fileView->widget()->setAcceptDrops(b);
    TQWidget::setAcceptDrops(b);
}

void KDirOperator::setDropOptions(int options)
{
    d->dropOptions = options;
    if (m_fileView)
       m_fileView->setDropOptions(options);
}

void KDirOperator::setView( KFile::FileView view )
{
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview=( KFile::isPreviewInfo(view) || KFile::isPreviewContents( view ) );

    if (view == KFile::Default) {
        if ( KFile::isDetailView( (KFile::FileView) defaultView ) )
            view = KFile::Detail;
        else
            view = KFile::Simple;

        separateDirs = KFile::isSeparateDirs( static_cast<KFile::FileView>(defaultView) );
        preview = ( KFile::isPreviewInfo( static_cast<KFile::FileView>(defaultView) ) ||
                    KFile::isPreviewContents( static_cast<KFile::FileView>(defaultView) ) )
                  && myActionCollection->action("preview")->isEnabled();

        if ( preview ) { // instantiates KFileMetaPreview and calls setView()
            m_viewKind = defaultView;
            slotDefaultPreview();
            return;
        }
        else if ( !separateDirs )
            separateDirsAction->setChecked(true);
    }

    // if we don't have any files, we can't separate dirs from files :)
    if ( (mode() & KFile::File) == 0 &&
         (mode() & KFile::Files) == 0 ) {
        separateDirs = false;
        separateDirsAction->setEnabled( false );
    }

    m_viewKind = static_cast<int>(view) | (separateDirs ? KFile::SeparateDirs : 0);
    view = static_cast<KFile::FileView>(m_viewKind);

    KFileView *new_view = createView( this, view );
    if ( preview ) {
        // we keep the preview-_widget_ around, but not the KFilePreview.
        // KFilePreview::setPreviewWidget handles the reparenting for us
        static_cast<KFilePreview*>(new_view)->setPreviewWidget(myPreview, url());
    }

    setView( new_view );
}


void KDirOperator::connectView(KFileView *view)
{
    // TODO: do a real timer and restart it after that
    pendingMimeTypes.clear();
    bool listDir = true;

    if ( dirOnlyMode() )
         view->setViewMode(KFileView::Directories);
    else
        view->setViewMode(KFileView::All);

    if ( myMode & KFile::Files )
        view->setSelectionMode( KFile::Extended );
    else
        view->setSelectionMode( KFile::Single );

    if (m_fileView)
    {
        if ( d->config ) // save and restore the views' configuration
        {
            m_fileView->writeConfig( d->config, d->configGroup );
            view->readConfig( d->config, d->configGroup );
        }

        // transfer the state from old view to new view
        view->clear();
        view->addItemList( *m_fileView->items() );
        listDir = false;

        if ( m_fileView->widget()->hasFocus() )
            view->widget()->setFocus();

        KFileItem *oldCurrentItem = m_fileView->currentFileItem();
        if ( oldCurrentItem ) {
            view->setCurrentItem( oldCurrentItem );
            view->setSelected( oldCurrentItem, false );
            view->ensureItemVisible( oldCurrentItem );
        }

        const KFileItemList *oldSelected = m_fileView->selectedItems();
        if ( !oldSelected->isEmpty() ) {
            KFileItemListIterator it( *oldSelected );
            for ( ; it.current(); ++it )
                view->setSelected( it.current(), true );
        }

        m_fileView->widget()->hide();
        delete m_fileView;
    }

    else
    {
        if ( d->config )
            view->readConfig( d->config, d->configGroup );
    }

    m_fileView = view;
    m_fileView->widget()->installEventFilter(this);
    if (m_fileView->widget()->inherits("TQScrollView"))
    {
        (static_cast<TQScrollView *>(m_fileView->widget()))->viewport()->installEventFilter(this);
    }
    m_fileView->setDropOptions(d->dropOptions);
    viewActionCollection = 0L;
    KFileViewSignaler *sig = view->signaler();

    connect(sig, TQ_SIGNAL( activatedMenu(const KFileItem *, const TQPoint& ) ),
            this, TQ_SLOT( activatedMenu(const KFileItem *, const TQPoint& )));
    connect(sig, TQ_SIGNAL( dirActivated(const KFileItem *) ),
            this, TQ_SLOT( selectDir(const KFileItem*) ) );
    connect(sig, TQ_SIGNAL( fileSelected(const KFileItem *) ),
            this, TQ_SLOT( selectFile(const KFileItem*) ) );
    connect(sig, TQ_SIGNAL( fileHighlighted(const KFileItem *) ),
            this, TQ_SLOT( highlightFile(const KFileItem*) ));
    connect(sig, TQ_SIGNAL( sortingChanged( TQDir::SortSpec ) ),
            this, TQ_SLOT( slotViewSortingChanged( TQDir::SortSpec )));
    connect(sig, TQ_SIGNAL( dropped(const KFileItem *, TQDropEvent*, const KURL::List&) ),
            this, TQ_SIGNAL( dropped(const KFileItem *, TQDropEvent*, const KURL::List&)) );

    if ( reverseAction->isChecked() != m_fileView->isReversed() )
        slotSortReversed();

    updateViewActions();
    m_fileView->widget()->resize(size());
    m_fileView->widget()->show();

    if ( listDir ) {
        TQApplication::setOverrideCursor( TQt::waitCursor );
        openURL( currUrl );
    }
    else
        view->listingCompleted();
}

KFile::Mode KDirOperator::mode() const
{
    return myMode;
}

void KDirOperator::setMode(KFile::Mode m)
{
    if (myMode == m)
        return;

    myMode = m;

    dir->setDirOnlyMode( dirOnlyMode() );

    // reset the view with the different mode
    setView( static_cast<KFile::FileView>(m_viewKind) );
}

void KDirOperator::setView(KFileView *view)
{
    if ( view == m_fileView ) {
        return;
    }

    setFocusProxy(view->widget());
    view->setSorting( mySorting );
    view->setOnlyDoubleClickSelectsFiles( d->onlyDoubleClickSelectsFiles );
    connectView(view); // also deletes the old view

    emit viewChanged( view );
}

void KDirOperator::setDirLister( KDirLister *lister )
{
    if ( lister == dir ) // sanity check
        return;

    delete dir;
    dir = lister;

    dir->setAutoUpdate( true );

    TQWidget* mainWidget = topLevelWidget();
    dir->setMainWindow (mainWidget);
    kdDebug (tdefile_area) << "mainWidget=" << mainWidget << endl;

    connect( dir, TQ_SIGNAL( percent( int )),
             TQ_SLOT( slotProgress( int ) ));
    connect( dir, TQ_SIGNAL(started( const KURL& )), TQ_SLOT(slotStarted()));
    connect( dir, TQ_SIGNAL(newItems(const KFileItemList &)),
             TQ_SLOT(insertNewFiles(const KFileItemList &)));
    connect( dir, TQ_SIGNAL(completed()), TQ_SLOT(slotIOFinished()));
    connect( dir, TQ_SIGNAL(canceled()), TQ_SLOT(slotCanceled()));
    connect( dir, TQ_SIGNAL(deleteItem(KFileItem *)),
             TQ_SLOT(itemDeleted(KFileItem *)));
    connect( dir, TQ_SIGNAL(redirection( const KURL& )),
	     TQ_SLOT( slotRedirected( const KURL& )));
    connect( dir, TQ_SIGNAL( clear() ), TQ_SLOT( slotClearView() ));
    connect( dir, TQ_SIGNAL( refreshItems( const KFileItemList& ) ),
             TQ_SLOT( slotRefreshItems( const KFileItemList& ) ) );
}

void KDirOperator::insertNewFiles(const KFileItemList &newone)
{
    if ( newone.isEmpty() || !m_fileView )
        return;

    myCompleteListDirty = true;
    m_fileView->addItemList( newone );
    emit updateInformation(m_fileView->numDirs(), m_fileView->numFiles());

    KFileItem *item;
    KFileItemListIterator it( newone );

    while ( (item = it.current()) ) {
	// highlight the dir we come from, if possible
	if ( d->dirHighlighting && item->isDir() &&
	     item->url().url(-1) == d->lastURL ) {
	    m_fileView->setCurrentItem( item );
	    m_fileView->ensureItemVisible( item );
	}

	++it;
    }

    TQTimer::singleShot(200, this, TQ_SLOT(resetCursor()));
}

void KDirOperator::selectDir(const KFileItem *item)
{
    setURL(item->url(), true);
}

void KDirOperator::itemDeleted(KFileItem *item)
{
    pendingMimeTypes.removeRef( item );
    if ( m_fileView )
    {
        m_fileView->removeItem( static_cast<KFileItem *>( item ));
        emit updateInformation(m_fileView->numDirs(), m_fileView->numFiles());
    }
}

void KDirOperator::selectFile(const KFileItem *item)
{
    TQApplication::restoreOverrideCursor();

    emit fileSelected( item );
}

void KDirOperator::setCurrentItem( const TQString& filename )
{
    if ( m_fileView ) {
        const KFileItem *item = 0L;

        if ( !filename.isNull() )
            item = static_cast<KFileItem *>(dir->findByName( filename ));

        m_fileView->clearSelection();
        if ( item ) {
            m_fileView->setCurrentItem( item );
            m_fileView->setSelected( item, true );
            m_fileView->ensureItemVisible( item );
        }
    }
}

TQString KDirOperator::makeCompletion(const TQString& string)
{
    if ( string.isEmpty() ) {
        m_fileView->clearSelection();
        return TQString::null;
    }

    prepareCompletionObjects();
    return myCompletion.makeCompletion( string );
}

TQString KDirOperator::makeDirCompletion(const TQString& string)
{
    if ( string.isEmpty() ) {
        m_fileView->clearSelection();
        return TQString::null;
    }

    prepareCompletionObjects();
    return myDirCompletion.makeCompletion( string );
}

void KDirOperator::prepareCompletionObjects()
{
    if ( !m_fileView )
	return;

    if ( myCompleteListDirty ) { // create the list of all possible completions
        KFileItemListIterator it( *(m_fileView->items()) );
        for( ; it.current(); ++it ) {
            KFileItem *item = it.current();

            myCompletion.addItem( item->name() );
            if ( item->isDir() )
                myDirCompletion.addItem( item->name() );
        }
        myCompleteListDirty = false;
    }
}

void KDirOperator::slotCompletionMatch(const TQString& match)
{
    setCurrentItem( match );
    emit completion( match );
}

void KDirOperator::setupActions()
{
    myActionCollection = new TDEActionCollection( topLevelWidget(), this, "KDirOperator::myActionCollection" );

    actionMenu = new TDEActionMenu( i18n("Menu"), myActionCollection, "popupMenu" );
    upAction = KStdAction::up( this, TQ_SLOT( cdUp() ), myActionCollection, "up" );
    upAction->setText( i18n("Parent Folder") );
    backAction = KStdAction::back( this, TQ_SLOT( back() ), myActionCollection, "back" );
    forwardAction = KStdAction::forward( this, TQ_SLOT(forward()), myActionCollection, "forward" );
    homeAction = KStdAction::home( this, TQ_SLOT( home() ), myActionCollection, "home" );
    homeAction->setText(i18n("Home Folder"));
    reloadAction = KStdAction::redisplay( this, TQ_SLOT(rereadDir()), myActionCollection, "reload" );
    actionSeparator = new TDEActionSeparator( myActionCollection, "separator" );
    d->viewActionSeparator = new TDEActionSeparator( myActionCollection,
                                                   "viewActionSeparator" );
    mkdirAction = new TDEAction( i18n("New Folder..."), 0,
                                 this, TQ_SLOT( mkdir() ), myActionCollection, "mkdir" );
    TDEAction* trash = new TDEAction( i18n( "Move to Trash" ), "edittrash", Key_Delete, myActionCollection, "trash" );
    connect( trash, TQ_SIGNAL( activated( TDEAction::ActivationReason, TQt::ButtonState ) ),
	     this, TQ_SLOT( trashSelected( TDEAction::ActivationReason, TQt::ButtonState ) ) );
    new TDEAction( i18n( "Delete" ), "edit-delete", SHIFT+Key_Delete, this,
                  TQ_SLOT( deleteSelected() ), myActionCollection, "delete" );
    mkdirAction->setIcon( TQString::fromLatin1("folder-new") );
    reloadAction->setText( i18n("Reload") );
    reloadAction->setShortcut( TDEStdAccel::shortcut( TDEStdAccel::Reload ));


    // the sort menu actions
    sortActionMenu = new TDEActionMenu( i18n("Sorting"), myActionCollection, "sorting menu");
    byNameAction = new TDERadioAction( i18n("By Name"), 0,
                                     this, TQ_SLOT( slotSortByName() ),
                                     myActionCollection, "by name" );
    byDateAction = new TDERadioAction( i18n("By Date"), 0,
                                     this, TQ_SLOT( slotSortByDate() ),
                                     myActionCollection, "by date" );
    bySizeAction = new TDERadioAction( i18n("By Size"), 0,
                                     this, TQ_SLOT( slotSortBySize() ),
                                     myActionCollection, "by size" );
    reverseAction = new TDEToggleAction( i18n("Reverse"), 0,
                                       this, TQ_SLOT( slotSortReversed() ),
                                       myActionCollection, "reversed" );

    TQString sortGroup = TQString::fromLatin1("sort");
    byNameAction->setExclusiveGroup( sortGroup );
    byDateAction->setExclusiveGroup( sortGroup );
    bySizeAction->setExclusiveGroup( sortGroup );


    dirsFirstAction = new TDEToggleAction( i18n("Folders First"), 0,
                                         myActionCollection, "dirs first");
    caseInsensitiveAction = new TDEToggleAction(i18n("Case Insensitive"), 0,
                                              myActionCollection, "case insensitive" );

    connect( dirsFirstAction, TQ_SIGNAL( toggled( bool ) ),
             TQ_SLOT( slotToggleDirsFirst() ));
    connect( caseInsensitiveAction, TQ_SIGNAL( toggled( bool ) ),
             TQ_SLOT( slotToggleIgnoreCase() ));



    // the view menu actions
    viewActionMenu = new TDEActionMenu( i18n("&View"), myActionCollection, "view menu" );
    connect( viewActionMenu->popupMenu(), TQ_SIGNAL( aboutToShow() ),
             TQ_SLOT( insertViewDependentActions() ));

    shortAction = new TDERadioAction( i18n("Short View"), "view_multicolumn",
                                    TDEShortcut(), myActionCollection, "short view" );
    detailedAction = new TDERadioAction( i18n("Detailed View"), "view_detailed",
                                       TDEShortcut(), myActionCollection, "detailed view" );

    showHiddenAction = new TDEToggleAction( i18n("Show Hidden Files"), TDEShortcut(),
                                          myActionCollection, "show hidden" );
//    showHiddenAction->setCheckedState( i18n("Hide Hidden Files") );
    separateDirsAction = new TDEToggleAction( i18n("Separate Folders"), TDEShortcut(),
                                            this,
                                            TQ_SLOT(slotSeparateDirs()),
                                            myActionCollection, "separate dirs" );
    TDEToggleAction *previewAction = new TDEToggleAction(i18n("Show Preview"),
                                                     "thumbnail", TDEShortcut(),
                                                     myActionCollection,
                                                     "preview" );
    previewAction->setCheckedState(i18n("Hide Preview"));
    connect( previewAction, TQ_SIGNAL( toggled( bool )),
             TQ_SLOT( togglePreview( bool )));


    TQString viewGroup = TQString::fromLatin1("view");
    shortAction->setExclusiveGroup( viewGroup );
    detailedAction->setExclusiveGroup( viewGroup );

    connect( shortAction, TQ_SIGNAL( activated() ),
             TQ_SLOT( slotSimpleView() ));
    connect( detailedAction, TQ_SIGNAL( activated() ),
             TQ_SLOT( slotDetailedView() ));
    connect( showHiddenAction, TQ_SIGNAL( toggled( bool ) ),
             TQ_SLOT( slotToggleHidden( bool ) ));

    new TDEAction( i18n("Properties"), TDEShortcut(ALT+Key_Return), this,
                 TQ_SLOT(slotProperties()), myActionCollection, "properties" );
}

void KDirOperator::setupMenu()
{
    setupMenu(AllActions);
}

void KDirOperator::setupMenu(int whichActions)
{
    // first fill the submenus (sort and view)
    sortActionMenu->popupMenu()->clear();
    sortActionMenu->insert( byNameAction );
    sortActionMenu->insert( byDateAction );
    sortActionMenu->insert( bySizeAction );
    sortActionMenu->insert( actionSeparator );
    sortActionMenu->insert( reverseAction );
    sortActionMenu->insert( dirsFirstAction );
    sortActionMenu->insert( caseInsensitiveAction );

    // now plug everything into the popupmenu
    actionMenu->popupMenu()->clear();
    if (whichActions & NavActions)
    {
        actionMenu->insert( upAction );
        actionMenu->insert( backAction );
        actionMenu->insert( forwardAction );
        actionMenu->insert( homeAction );
        actionMenu->insert( actionSeparator );
    }

    if (whichActions & FileActions)
    {
        actionMenu->insert( mkdirAction );
        if (currUrl.isLocalFile() && !(TDEApplication::keyboardMouseState() & TQt::ShiftButton))
            actionMenu->insert( myActionCollection->action( "trash" ) );
        TDEConfig *globalconfig = TDEGlobal::config();
        TDEConfigGroupSaver cs( globalconfig, TQString::fromLatin1("KDE") );
        if (!currUrl.isLocalFile() || (TDEApplication::keyboardMouseState() & TQt::ShiftButton) ||
            globalconfig->readBoolEntry("ShowDeleteCommand", false))
            actionMenu->insert( myActionCollection->action( "delete" ) );
        actionMenu->insert( actionSeparator );
    }

    if (whichActions & SortActions)
    {
        actionMenu->insert( sortActionMenu );
        actionMenu->insert( actionSeparator );
    }

    if (whichActions & ViewActions)
    {
        actionMenu->insert( viewActionMenu );
        actionMenu->insert( actionSeparator );
    }

    if (whichActions & FileActions)
    {
        actionMenu->insert( myActionCollection->action( "properties" ) );
    }
}

void KDirOperator::updateSortActions()
{
    if ( KFile::isSortByName( mySorting ) )
        byNameAction->setChecked( true );
    else if ( KFile::isSortByDate( mySorting ) )
        byDateAction->setChecked( true );
    else if ( KFile::isSortBySize( mySorting ) )
        bySizeAction->setChecked( true );

    dirsFirstAction->setChecked( KFile::isSortDirsFirst( mySorting ) );
    caseInsensitiveAction->setChecked( KFile::isSortCaseInsensitive(mySorting) );
    caseInsensitiveAction->setEnabled( KFile::isSortByName( mySorting ) );

    if ( m_fileView )
        reverseAction->setChecked( m_fileView->isReversed() );
}

void KDirOperator::updateViewActions()
{
    KFile::FileView fv = static_cast<KFile::FileView>( m_viewKind );

    separateDirsAction->setChecked( KFile::isSeparateDirs( fv ) &&
                                    separateDirsAction->isEnabled() );

    shortAction->setChecked( KFile::isSimpleView( fv ));
    detailedAction->setChecked( KFile::isDetailView( fv ));
}

void KDirOperator::readConfig( TDEConfig *kc, const TQString& group )
{
    if ( !kc )
        return;
    TQString oldGroup = kc->group();
    if ( !group.isEmpty() )
        kc->setGroup( group );

    defaultView = 0;
    int sorting = 0;

    TQString viewStyle = kc->readEntry( TQString::fromLatin1("View Style"),
                                       TQString::fromLatin1("Simple") );
    if ( viewStyle == TQString::fromLatin1("Detail") )
        defaultView |= KFile::Detail;
    else
        defaultView |= KFile::Simple;
    if ( kc->readBoolEntry( TQString::fromLatin1("Separate Directories"),
                            DefaultMixDirsAndFiles ) )
        defaultView |= KFile::SeparateDirs;
    if ( kc->readBoolEntry(TQString::fromLatin1("Show Preview"), false))
        defaultView |= KFile::PreviewContents;

    if ( kc->readBoolEntry( TQString::fromLatin1("Sort case insensitively"),
                            DefaultCaseInsensitive ) )
        sorting |= TQDir::IgnoreCase;
    if ( kc->readBoolEntry( TQString::fromLatin1("Sort directories first"),
                            DefaultDirsFirst ) )
        sorting |= TQDir::DirsFirst;


    TQString name = TQString::fromLatin1("Name");
    TQString sortBy = kc->readEntry( TQString::fromLatin1("Sort by"), name );
    if ( sortBy == name )
        sorting |= TQDir::Name;
    else if ( sortBy == TQString::fromLatin1("Size") )
        sorting |= TQDir::Size;
    else if ( sortBy == TQString::fromLatin1("Date") )
        sorting |= TQDir::Time;

    mySorting = static_cast<TQDir::SortSpec>( sorting );
    setSorting( mySorting );


    if ( kc->readBoolEntry( TQString::fromLatin1("Show hidden files"),
                            DefaultShowHidden ) ) {
         showHiddenAction->setChecked( true );
         dir->setShowingDotFiles( true );
    }
    if ( kc->readBoolEntry( TQString::fromLatin1("Sort reversed"),
                            DefaultSortReversed ) )
        reverseAction->setChecked( true );

    kc->setGroup( oldGroup );
}

void KDirOperator::writeConfig( TDEConfig *kc, const TQString& group )
{
    if ( !kc )
        return;

    const TQString oldGroup = kc->group();

    if ( !group.isEmpty() )
        kc->setGroup( group );

    TQString sortBy = TQString::fromLatin1("Name");
    if ( KFile::isSortBySize( mySorting ) )
        sortBy = TQString::fromLatin1("Size");
    else if ( KFile::isSortByDate( mySorting ) )
        sortBy = TQString::fromLatin1("Date");
    kc->writeEntry( TQString::fromLatin1("Sort by"), sortBy );

    kc->writeEntry( TQString::fromLatin1("Sort reversed"),
                    reverseAction->isChecked() );
    kc->writeEntry( TQString::fromLatin1("Sort case insensitively"),
                    caseInsensitiveAction->isChecked() );
    kc->writeEntry( TQString::fromLatin1("Sort directories first"),
                    dirsFirstAction->isChecked() );

    // don't save the separate dirs or preview when an application specific
    // preview is in use.
    bool appSpecificPreview = false;
    if ( myPreview ) {
        TQWidget *preview = const_cast<TQWidget*>( myPreview ); // grmbl
        KFileMetaPreview *tmp = dynamic_cast<KFileMetaPreview*>( preview );
        appSpecificPreview = (tmp == 0L);
    }

    if ( !appSpecificPreview ) {
        if ( separateDirsAction->isEnabled() )
            kc->writeEntry( TQString::fromLatin1("Separate Directories"),
                            separateDirsAction->isChecked() );

        TDEToggleAction *previewAction = static_cast<TDEToggleAction*>(myActionCollection->action("preview"));
        if ( previewAction->isEnabled() ) {
            bool hasPreview = previewAction->isChecked();
            kc->writeEntry( TQString::fromLatin1("Show Preview"), hasPreview );
        }
    }

    kc->writeEntry( TQString::fromLatin1("Show hidden files"),
                    showHiddenAction->isChecked() );

    KFile::FileView fv = static_cast<KFile::FileView>( m_viewKind );
    TQString style;
    if ( KFile::isDetailView( fv ) )
        style = TQString::fromLatin1("Detail");
    else if ( KFile::isSimpleView( fv ) )
        style = TQString::fromLatin1("Simple");
    kc->writeEntry( TQString::fromLatin1("View Style"), style );

    kc->setGroup( oldGroup );
}


void KDirOperator::resizeEvent( TQResizeEvent * )
{
    if (m_fileView)
        m_fileView->widget()->resize( size() );

    if ( progress->parent() == this ) // might be reparented into a statusbar
	progress->move(2, height() - progress->height() -2);
}

void KDirOperator::setOnlyDoubleClickSelectsFiles( bool enable )
{
    d->onlyDoubleClickSelectsFiles = enable;
    if ( m_fileView )
        m_fileView->setOnlyDoubleClickSelectsFiles( enable );
}

bool KDirOperator::onlyDoubleClickSelectsFiles() const
{
    return d->onlyDoubleClickSelectsFiles;
}

void KDirOperator::slotStarted()
{
    progress->setProgress( 0 );
    // delay showing the progressbar for one second
    d->progressDelayTimer->start( 1000, true );
}

void KDirOperator::slotShowProgress()
{
    progress->raise();
    progress->show();
    TQApplication::flushX();
}

void KDirOperator::slotProgress( int percent )
{
    progress->setProgress( percent );
    // we have to redraw this as fast as possible
    if ( progress->isVisible() )
	TQApplication::flushX();
}


void KDirOperator::slotIOFinished()
{
    d->progressDelayTimer->stop();
    slotProgress( 100 );
    progress->hide();
    emit finishedLoading();
    resetCursor();

    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::slotCanceled()
{
    emit finishedLoading();
    resetCursor();

    if ( m_fileView )
        m_fileView->listingCompleted();
}

KProgress * KDirOperator::progressBar() const
{
    return progress;
}

void KDirOperator::clearHistory()
{
    backStack.clear();
    backAction->setEnabled( false );
    forwardStack.clear();
    forwardAction->setEnabled( false );
}

void KDirOperator::slotViewActionAdded( TDEAction *action )
{
    if ( viewActionMenu->popupMenu()->count() == 5 ) // need to add a separator
	viewActionMenu->insert( d->viewActionSeparator );

    viewActionMenu->insert( action );
}

void KDirOperator::slotViewActionRemoved( TDEAction *action )
{
    viewActionMenu->remove( action );

    if ( viewActionMenu->popupMenu()->count() == 6 ) // remove the separator
	viewActionMenu->remove( d->viewActionSeparator );
}

void KDirOperator::slotViewSortingChanged( TQDir::SortSpec sort )
{
    mySorting = sort;
    updateSortActions();
}

void KDirOperator::setEnableDirHighlighting( bool enable )
{
    d->dirHighlighting = enable;
}

bool KDirOperator::dirHighlighting() const
{
    return d->dirHighlighting;
}

void KDirOperator::slotProperties()
{
    if ( m_fileView ) {
        const KFileItemList *list = m_fileView->selectedItems();
        if ( !list->isEmpty() )
            (void) new KPropertiesDialog( *list, this, "props dlg", true);
    }
}

void KDirOperator::slotClearView()
{
    if ( m_fileView )
        m_fileView->clearView();
}

// ### temporary code
#include <dirent.h>
bool KDirOperator::isReadable( const KURL& url )
{
    if ( !url.isLocalFile() )
	return true; // what else can we say?

    KDE_struct_stat buf;
    TQString ts = url.path(+1);
    bool readable = ( KDE_stat( TQFile::encodeName( ts ), &buf) == 0 );
    if (readable) { // further checks
	DIR *test;
	test = opendir( TQFile::encodeName( ts )); // we do it just to test here
	readable = (test != 0);
	if (test)
	    closedir(test);
    }
    return readable;
}

void KDirOperator::togglePreview( bool on )
{
    if ( on )
        slotDefaultPreview();
    else
        setView( (KFile::FileView) (m_viewKind & ~(KFile::PreviewContents|KFile::PreviewInfo)) );
}

void KDirOperator::slotRefreshItems( const KFileItemList& items )
{
    if ( !m_fileView )
        return;

    KFileItemListIterator it( items );
    for ( ; it.current(); ++it )
        m_fileView->updateView( it.current() );
}

void KDirOperator::setViewConfig( TDEConfig *config, const TQString& group )
{
    d->config = config;
    d->configGroup = group;
}

TDEConfig * KDirOperator::viewConfig()
{
    return d->config;
}

TQString KDirOperator::viewConfigGroup() const
{
    return d->configGroup;
}

bool KDirOperator::eventFilter(TQObject *obj, TQEvent *ev)
{
    if (ev->type() == TQEvent::MouseButtonRelease)
    {
        TQMouseEvent *mouseEv = static_cast<TQMouseEvent *>(ev);
        switch (mouseEv->button())
        {
            case TQMouseEvent::HistoryBackButton:
                back();
                return true;
            case TQMouseEvent::HistoryForwardButton:
                forward();
                return true;
        }
    }
    return false;
}

void KDirOperator::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdediroperator.moc"
