/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espensa@online.no)
 *  Copyright (C) 2003 Ravikiran Rajagopal (ravi@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <tqbitmap.h>
#include <tqgrid.h>
#include <tqhbox.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqobjectlist.h>
#include <tqpixmap.h>
#include <tqsplitter.h>
#include <tqtabwidget.h>
#include <tqvbox.h>
#include <tqwidgetstack.h>
#include <tqpainter.h>
#include <tqstyle.h>

#include <tdeapplication.h>
#include <kdialog.h> // Access to some static members
#include <tdelocale.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kseparator.h>
#include <kdebug.h>
#include "kjanuswidget.h"
#include <tdelistview.h>
#include "kpushbutton.h"
#include "kguiitem.h"

class KJanusWidget::IconListItem : public TQListBoxItem
{
  public:
    IconListItem( TQListBox *listbox, const TQPixmap &pixmap,
                  const TQString &text );
    virtual int height( const TQListBox *lb ) const;
    virtual int width( const TQListBox *lb ) const;
    int expandMinimumWidth( int width );
    void highlight( bool erase );        

  protected:
    const TQPixmap &defaultPixmap();
    void paint( TQPainter *painter );
    
  private:
    void paintContents( TQPainter *painter );  
  
    TQPixmap mPixmap;
    int mMinimumWidth;
};

class KJanusWidget::KJanusWidgetPrivate
{
public:
  KJanusWidgetPrivate() : mNextPageIndex(0), mListFrame( 0 ) { }

  int mNextPageIndex; // The next page index.

  // Dictionary for multipage modes.
  TQMap<int,TQWidget*> mIntToPage;
  // Reverse dictionary. Used because showPage() may be performance critical.
  TQMap<TQWidget*,int> mPageToInt;
  // Dictionary of title string associated with page.
  TQMap<int, TQString> mIntToTitle;

  TQWidget * mListFrame;
  TQSplitter * mSplitter;
};

template class TQPtrList<TQListViewItem>;


KJanusWidget::KJanusWidget( TQWidget *parent, const char *name, int face )
  : TQWidget( parent, name, 0 ),
    mValid(false), mPageList(0),
    mTitleList(0), mFace(face), mTitleLabel(0), mActivePageWidget(0),
    mShowIconsInTreeList(false), d(0)
{
  TQVBoxLayout *topLayout = new TQVBoxLayout( this );

  if( mFace == TreeList || mFace == IconList )
  {
    d = new KJanusWidgetPrivate;
    d->mSplitter = 0;

    TQFrame *page;
    if( mFace == TreeList )
    {
      d->mSplitter = new TQSplitter( this );
      topLayout->addWidget( d->mSplitter, 10 );
      mTreeListResizeMode = TQSplitter::KeepSize;

      d->mListFrame = new TQWidget( d->mSplitter );
      TQVBoxLayout *dummy = new TQVBoxLayout( d->mListFrame, 0, KDialog::spacingHint() );
      dummy->setAutoAdd( true );
      mTreeList = new TDEListView( d->mListFrame );
      mTreeList->addColumn( TQString::null );
      mTreeList->header()->hide();
      mTreeList->setRootIsDecorated(true);
      mTreeList->setSorting( -1 );
      connect( mTreeList, TQ_SIGNAL(selectionChanged()), TQ_SLOT(slotShowPage()) );
      connect( mTreeList, TQ_SIGNAL(clicked(TQListViewItem *)), TQ_SLOT(slotItemClicked(TQListViewItem *)));

      //
      // Page area. Title at top with a separator below and a pagestack using
      // all available space at bottom.
      //
      TQFrame *p = new TQFrame( d->mSplitter );

      TQHBoxLayout *hbox = new TQHBoxLayout( p, 0, 0 );

      page = new TQFrame( p );
      hbox->addWidget( page, 10 );
    }
    else
    {
      TQHBoxLayout *hbox = new TQHBoxLayout( topLayout );
      d->mListFrame = new TQWidget( this );
      hbox->addWidget( d->mListFrame );

      ( new TQVBoxLayout( d->mListFrame, 0, 0 ) )->setAutoAdd( true );
      mIconList = new IconListBox( d->mListFrame );

      TQFont listFont( mIconList->font() );
      listFont.setBold( true );
      mIconList->setFont( listFont );

      mIconList->verticalScrollBar()->installEventFilter( this );
      connect( mIconList, TQ_SIGNAL(selectionChanged()), TQ_SLOT(slotShowPage()));
      connect( mIconList, TQ_SIGNAL(onItem(TQListBoxItem *)), TQ_SLOT(slotOnItem(TQListBoxItem *)));

      hbox->addSpacing( KDialog::marginHint() );
      page = new TQFrame( this );
      hbox->addWidget( page, 10 );
    }

    //
    // Rest of page area. Title at top with a separator below and a
    // pagestack using all available space at bottom.
    //

    TQVBoxLayout *vbox = new TQVBoxLayout( page, 0, KDialog::spacingHint() );

    mTitleLabel = new TQLabel( i18n("Empty Page"), page, "KJanusWidgetTitleLabel" );
    vbox->addWidget( mTitleLabel, 0, TQApplication::reverseLayout() ? AlignRight : AlignLeft );

    TQFont titleFont( mTitleLabel->font() );
    titleFont.setBold( true );
    mTitleLabel->setFont( titleFont );

    mTitleSep = new KSeparator( page );
    mTitleSep->setFrameStyle( TQFrame::HLine|TQFrame::Plain );
    vbox->addWidget( mTitleSep );

    mPageStack = new TQWidgetStack( page );
    connect(mPageStack, TQ_SIGNAL(aboutToShow(TQWidget *)),
            TQ_SIGNAL(aboutToShowPage(TQWidget *)));
    vbox->addWidget( mPageStack, 10 );
  }
  else if( mFace == Tabbed )
  {
    d = new KJanusWidgetPrivate;

    mTabControl = new TQTabWidget( this );
    mTabControl->setMargin (KDialog::marginHint());
    connect(mTabControl, TQ_SIGNAL(currentChanged(TQWidget *)),
            TQ_SIGNAL(aboutToShowPage(TQWidget *)));
    topLayout->addWidget( mTabControl, 10 );
  }
  else if( mFace == Swallow )
  {
    mSwallowPage = new TQWidget( this );
    topLayout->addWidget( mSwallowPage, 10 );
  }
  else
  {
    mFace = Plain;
    mPlainPage = new TQFrame( this );
    topLayout->addWidget( mPlainPage, 10 );
  }

  if ( kapp )
    connect(kapp,TQ_SIGNAL(tdedisplayFontChanged()),TQ_SLOT(slotFontChanged()));
  mValid = true;

  setSwallowedWidget(0); // Set default size if 'mFace' is Swallow.
}


KJanusWidget::~KJanusWidget()
{
  delete d;
}


bool KJanusWidget::isValid() const
{
  return mValid;
}


TQFrame *KJanusWidget::plainPage()
{
  return mPlainPage;
}


int KJanusWidget::face() const
{
  return mFace;
}

TQWidget *KJanusWidget::FindParent()
{
  if( mFace == Tabbed ) {
    return mTabControl;
  }
  else {
    return this;
  }
}

TQFrame *KJanusWidget::addPage( const TQStringList &items, const TQString &header,
			       const TQPixmap &pixmap )
{
  if( !mValid )
  {
    kdDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  TQFrame *page = new TQFrame( FindParent(), "page" );
  addPageWidget( page, items, header, pixmap );

  return page;
}

void KJanusWidget::pageGone( TQObject *obj )
{
  removePage( static_cast<TQWidget*>( obj ) );
}

void KJanusWidget::slotReopen( TQListViewItem * item )
{
  if( item )
    item->setOpen( true );
}

TQFrame *KJanusWidget::addPage( const TQString &itemName, const TQString &header,
          const TQPixmap &pixmap )
{
  TQStringList items;
  items << itemName;
  return addPage(items, header, pixmap);
}



TQVBox *KJanusWidget::addVBoxPage( const TQStringList &items,
          const TQString &header,
          const TQPixmap &pixmap )
{
  if( !mValid )
  {
    kdDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  TQVBox *page = new TQVBox(FindParent() , "page" );
  page->setSpacing( KDialog::spacingHint() );
  addPageWidget( page, items, header, pixmap );

  return page;
}

TQVBox *KJanusWidget::addVBoxPage( const TQString &itemName,
				  const TQString &header,
				  const TQPixmap &pixmap )
{
  TQStringList items;
  items << itemName;
  return addVBoxPage(items, header, pixmap);
}

TQHBox *KJanusWidget::addHBoxPage( const TQStringList &items,
				  const TQString &header,
				  const TQPixmap &pixmap )
{
  if( !mValid ) {
    kdDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  TQHBox *page = new TQHBox(FindParent(), "page");
  page->setSpacing( KDialog::spacingHint() );
  addPageWidget( page, items, header, pixmap );

  return page;
}

TQHBox *KJanusWidget::addHBoxPage( const TQString &itemName,
				  const TQString &header,
				  const TQPixmap &pixmap )
{
  TQStringList items;
  items << itemName;
  return addHBoxPage(items, header, pixmap);
}

TQGrid *KJanusWidget::addGridPage( int n, Orientation dir,
				  const TQStringList &items,
				  const TQString &header,
				  const TQPixmap &pixmap )
{
  if( !mValid )
  {
    kdDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  TQGrid *page = new TQGrid( n, dir, FindParent(), "page" );
  page->setSpacing( KDialog::spacingHint() );
  addPageWidget( page, items, header, pixmap );

  return page;
}


TQGrid *KJanusWidget::addGridPage( int n, Orientation dir,
				  const TQString &itemName,
				  const TQString &header,
				  const TQPixmap &pixmap )
{
  TQStringList items;
  items << itemName;
  return addGridPage(n, dir, items, header, pixmap);
}

void KJanusWidget::InsertTreeListItem(const TQStringList &items, const TQPixmap &pixmap, TQFrame *page)
{
  bool isTop = true;
  TQListViewItem *curTop = 0, *child, *last, *newChild;
  unsigned int index = 1;
  TQStringList curPath;

  for ( TQStringList::ConstIterator it = items.begin(); it != items.end(); ++it, index++ ) {
    TQString name = (*it);
    bool isPath = ( index != items.count() );

    // Find the first child.
    if (isTop) {
      child = mTreeList->firstChild();
    }
    else {
      child = curTop->firstChild();
    }

    // Now search for a child with the current Name, and if it we doesn't
    // find it, then remember the location of the last child.
    for (last = 0; child && child->text(0) != name ; last = child, child = child->nextSibling());

    if (!last && !child) {
      // This node didn't have any children at all, lets just insert the
      // new child.
      if (isTop)
        newChild = new TQListViewItem(mTreeList, name);
      else
        newChild = new TQListViewItem(curTop, name);

    }
    else if (child) {
      // we found the given name in this child.
      if (!isPath) {
        kdDebug() << "The element inserted was already in the TreeList box!" << endl;
        return;
      }
      else {
        // Ok we found the folder
        newChild  = child;
      }
    }
    else {
      // the node had some children, but we didn't find the given name
      if (isTop)
        newChild = new TQListViewItem(mTreeList, last, name);
      else
        newChild = new TQListViewItem(curTop, last, name);
    }

    // Now make the element expandable if it is a path component, and make
    // ready for next loop
    if (isPath) {
      newChild->setExpandable(true);
      curTop = newChild;
      isTop = false;
      curPath << name;

      TQString key = curPath.join("_/_");
      if (mFolderIconMap.contains(key)) {
        TQPixmap p = mFolderIconMap[key];
        newChild->setPixmap(0,p);
      }
    }
    else {
      if (mShowIconsInTreeList) {
        newChild->setPixmap(0, pixmap);
      }
      mTreeListToPageStack.insert(newChild, page);
    }
  }
}

void KJanusWidget::addPageWidget( TQFrame *page, const TQStringList &items,
				  const TQString &header,const TQPixmap &pixmap )
{
  connect(page, TQ_SIGNAL(destroyed(TQObject*)), TQ_SLOT(pageGone(TQObject*)));

  if( mFace == Tabbed )
  {
    mTabControl->addTab (page, items.last());
    d->mIntToPage[d->mNextPageIndex] = page;
    d->mPageToInt[page] = d->mNextPageIndex;
    d->mNextPageIndex++;
  }
  else if( mFace == TreeList || mFace == IconList )
  {
    d->mIntToPage[d->mNextPageIndex] = page;
    d->mPageToInt[page] = d->mNextPageIndex;
    mPageStack->addWidget( page, 0 );

    if (items.isEmpty()) {
      kdDebug() << "Invalid TQStringList, with zero items" << endl;
      return;
    }

    if( mFace == TreeList )
    {
      InsertTreeListItem(items, pixmap, page);
    }
    else // mFace == IconList
    {
      TQString itemName = items.last();
      IconListItem *item = new IconListItem( mIconList, pixmap, itemName );
      mIconListToPageStack.insert(item, page);
      mIconList->invalidateHeight();
      mIconList->invalidateWidth();

      if (mIconList->isVisible())
        mIconList->updateWidth();
    }

    //
    // Make sure the title label is sufficiently wide
    //
    TQString lastName = items.last();
    const TQString &title = (!header.isNull() ? header : lastName);
    TQRect r = mTitleLabel->fontMetrics().boundingRect( title );
    if( mTitleLabel->minimumWidth() < r.width() )
    {
      mTitleLabel->setMinimumWidth( r.width() );
    }
    d->mIntToTitle[d->mNextPageIndex] = title;
    if( d->mIntToTitle.count() == 1 )
    {
      showPage(0);
    }
    d->mNextPageIndex++;
  }
  else
  {
    kdDebug() << "KJanusWidget::addPageWidget: can only add a page in Tabbed, TreeList or IconList modes" << endl;
  }

}

void KJanusWidget::setFolderIcon(const TQStringList &path, const TQPixmap &pixmap)
{
  TQString key = path.join("_/_");
  mFolderIconMap.insert(key,pixmap);
}



bool KJanusWidget::setSwallowedWidget( TQWidget *widget )
{
  if( mFace != Swallow || !mValid )
  {
    return false;
  }

  //
  // Remove current layout and make a new.
  //
  delete mSwallowPage->layout();

  TQGridLayout *gbox = new TQGridLayout( mSwallowPage, 1, 1, 0 );

  //
  // Hide old children
  //
  TQObjectList l = mSwallowPage->childrenListObject(); // silence please
  for( uint i=0; i < l.count(); i++ )
  {
    TQObject *o = l.at(i);
    if( o->isWidgetType() )
    {
      ((TQWidget*)o)->hide();
    }
  }

  //
  // Add new child or make default size
  //
  if( !widget )
  {
    gbox->addRowSpacing(0,100);
    gbox->addColSpacing(0,100);
    mSwallowPage->setMinimumSize(100,100);
  }
  else
  {
    if( widget->parent() != mSwallowPage )
    {
      widget->reparent( mSwallowPage, 0, TQPoint(0,0) );
    }
    gbox->addWidget(widget, 0, 0 );
    gbox->activate();
    mSwallowPage->setMinimumSize( widget->minimumSize() );
  }

  return true;
}

bool KJanusWidget::slotShowPage()
{
  if( !mValid )
  {
    return false;
  }

  if( mFace == TreeList )
  {
    TQListViewItem *node = mTreeList->selectedItem();
    if( !node ) { return false; }

    TQWidget *stackItem = mTreeListToPageStack[node];
    // Make sure to call through the virtual function showPage(int)
    return showPage(d->mPageToInt[stackItem]);
  }
  else if( mFace == IconList )
  {
    TQListBoxItem *node = mIconList->item( mIconList->currentItem() );
    if( !node ) { return false; }
    TQWidget *stackItem = mIconListToPageStack[node];
    // Make sure to call through the virtual function showPage(int)
    return showPage(d->mPageToInt[stackItem]);
  }

  return false;
}


bool KJanusWidget::showPage( int index )
{
  if( !d || !mValid )
  {
    return false;
  }
  else
  {
    return showPage(d->mIntToPage[index]);
  }
}


bool KJanusWidget::showPage( TQWidget *w )
{
  if( !w || !mValid )
  {
    return false;
  }

  if( mFace == TreeList || mFace == IconList )
  {
    mPageStack->raiseWidget( w );
    mActivePageWidget = w;

    int index = d->mPageToInt[w];
    mTitleLabel->setText( d->mIntToTitle[index] );
    if( mFace == TreeList )
    {
      TQMap<TQListViewItem *, TQWidget *>::Iterator it;
      for (it = mTreeListToPageStack.begin(); it != mTreeListToPageStack.end(); ++it){
        TQListViewItem *key = it.key();
        TQWidget *val = it.data();
        if (val == w) {
          mTreeList->setSelected(key, true );
          break;
        }
      }
    }
    else
    {
      TQMap<TQListBoxItem *, TQWidget *>::Iterator it;
      for (it = mIconListToPageStack.begin(); it != mIconListToPageStack.end(); ++it){
        TQListBoxItem *key = it.key();
        TQWidget *val = it.data();
        if (val == w) {
          mIconList->setSelected( key, true );
          break;
        }
      }
    }
  }
  else if( mFace == Tabbed )
  {
    mTabControl->showPage(w);
    mActivePageWidget = w;
  }
  else
  {
    return false;
  }

  return true;
}


int KJanusWidget::activePageIndex() const
{
  if( mFace == TreeList) {
    TQListViewItem *node = mTreeList->selectedItem();
    if( !node ) { return -1; }
    TQWidget *stackItem = mTreeListToPageStack[node];
    return d->mPageToInt[stackItem];
  }
  else if (mFace == IconList) {
    TQListBoxItem *node = mIconList->item( mIconList->currentItem() );
    if( !node ) { return false; }
    TQWidget *stackItem = mIconListToPageStack[node];
    return d->mPageToInt[stackItem];
  }
  else if( mFace == Tabbed ) {
    TQWidget *widget = mTabControl->currentPage();
    return ( !widget ? -1 : d->mPageToInt[widget] );
  }
  else {
    return -1;
  }
}


int KJanusWidget::pageIndex( TQWidget *widget ) const
{
  if( !widget )
  {
    return -1;
  }
  else if( mFace == TreeList || mFace == IconList )
  {
    return d->mPageToInt[widget];
  }
  else if( mFace == Tabbed )
  {
    //
    // The user gets the real page widget with addVBoxPage(), addHBoxPage()
    // and addGridPage() but not with addPage() which returns a child of
    // the toplevel page. addPage() returns a TQFrame so I check for that.
    //
    if( widget->isA("TQFrame") )
    {
      return d->mPageToInt[widget->parentWidget()];
    }
    else
    {
      return d->mPageToInt[widget];
    }
  }
  else
  {
    return -1;
  }
}

void KJanusWidget::slotFontChanged()
{
  if( mTitleLabel )
  {
    mTitleLabel->setFont( TDEGlobalSettings::generalFont() );
    TQFont titleFont( mTitleLabel->font() );
    titleFont.setBold( true );
    mTitleLabel->setFont( titleFont );
  }

  if( mFace == IconList )
  {
    TQFont listFont( mIconList->font() );
    listFont.setBold( true );
    mIconList->setFont( listFont );
    mIconList->invalidateHeight();
    mIconList->invalidateWidth();
  }
}

// makes the treelist behave like the list of kcontrol
void KJanusWidget::slotItemClicked(TQListViewItem *it)
{
  if(it && (it->childCount()>0))
    it->setOpen(!it->isOpen());
}

// hack because qt does not support TQ_OBJECT in nested classes
void KJanusWidget::slotOnItem(TQListBoxItem *qitem)
{
  mIconList->slotOnItem( qitem );
}  

void KJanusWidget::setFocus()
{
  if( !mValid ) { return; }
  if( mFace == TreeList )
  {
    mTreeList->setFocus();
  }
  if( mFace == IconList )
  {
    mIconList->setFocus();
  }
  else if( mFace == Tabbed )
  {
    mTabControl->setFocus();
  }
  else if( mFace == Swallow )
  {
    mSwallowPage->setFocus();
  }
  else if( mFace == Plain )
  {
    mPlainPage->setFocus();
  }
}


TQSize KJanusWidget::minimumSizeHint() const
{
  if( mFace == TreeList || mFace == IconList )
  {
    TQSize s1( KDialog::spacingHint(), KDialog::spacingHint()*2 );
    TQSize s2(0,0);
    TQSize s3(0,0);
    TQSize s4( mPageStack->sizeHint() );

    if( mFace == TreeList )
    {
      s1.rwidth() += style().pixelMetric( TQStyle::PM_SplitterWidth );
      s2 = mTreeList->minimumSize();
    }
    else
    {
      mIconList->updateMinimumHeight();
      mIconList->updateWidth();
      s2 = mIconList->minimumSize();
    }

    if( mTitleLabel->isVisible() )
    {
      s3 += mTitleLabel->sizeHint();
      s3.rheight() += mTitleSep->minimumSize().height();
    }

    //
    // Select the tallest item. It has only effect in IconList mode
    //
    int h1 = s1.rheight() + s3.rheight() + s4.height();
    int h2 = TQMAX( h1, s2.rheight() );

    return TQSize( s1.width()+s2.width()+TQMAX(s3.width(),s4.width()), h2 );
  }
  else if( mFace == Tabbed )
  {
    return mTabControl->sizeHint();
  }
  else if( mFace == Swallow )
  {
    return mSwallowPage->minimumSize();
  }
  else if( mFace == Plain )
  {
    return mPlainPage->sizeHint();
  }
  else
  {
    return TQSize( 100, 100 ); // Should never happen though.
  }

}


TQSize KJanusWidget::sizeHint() const
{
  return minimumSizeHint();
}


void KJanusWidget::setTreeListAutoResize( bool state )
{
  if( mFace == TreeList )
  {
    mTreeListResizeMode = !state ?
      TQSplitter::KeepSize : TQSplitter::Stretch;
    if( d->mSplitter )
      d->mSplitter->setResizeMode( d->mListFrame, mTreeListResizeMode );
  }
}


void KJanusWidget::setIconListAllVisible( bool state )
{
  if( mFace == IconList )
  {
    mIconList->setShowAll( state );
  }
}

void KJanusWidget::setShowIconsInTreeList( bool state )
{
  mShowIconsInTreeList = state;
}

void KJanusWidget::setRootIsDecorated( bool state )
{
  if( mFace == TreeList ) {
    mTreeList->setRootIsDecorated(state);
  }
}

void KJanusWidget::unfoldTreeList( bool persist )
{
  if( mFace == TreeList )
  {
    if( persist )
      connect( mTreeList, TQ_SIGNAL( collapsed( TQListViewItem * ) ), this, TQ_SLOT( slotReopen( TQListViewItem * ) ) );
    else
      disconnect( mTreeList, TQ_SIGNAL( collapsed( TQListViewItem * ) ), this, TQ_SLOT( slotReopen( TQListViewItem * ) ) );

    for( TQListViewItem * item = mTreeList->firstChild(); item; item = item->itemBelow() )
      item->setOpen( true );
  }
}

void KJanusWidget::addWidgetBelowList( TQWidget * widget )
{
  if( ( mFace == TreeList || mFace == IconList ) && d->mListFrame )
  {
    widget->reparent( d->mListFrame, TQPoint() );
  }
}

void KJanusWidget::addButtonBelowList( const TQString & text, TQObject * recv, const char * slot )
{
  if( ( mFace == TreeList || mFace == IconList ) && d->mListFrame )
  {
    TQPushButton * button = new TQPushButton( text, d->mListFrame, "KJanusWidget::buttonBelowList" );
    connect( button, TQ_SIGNAL( clicked() ), recv, slot );
  }
}

void KJanusWidget::addButtonBelowList( const KGuiItem & item, TQObject * recv, const char * slot )
{
  if( ( mFace == TreeList || mFace == IconList ) && d->mListFrame )
  {
    KPushButton * button = new KPushButton( item, d->mListFrame, "KJanusWidget::buttonBelowList" );
    connect( button, TQ_SIGNAL( clicked() ), recv, slot );
  }
}

void KJanusWidget::showEvent( TQShowEvent * )
{
  if( mFace == TreeList )
  {
    if( d->mSplitter )
      d->mSplitter->setResizeMode( d->mListFrame, mTreeListResizeMode );
  }
}


//
// 2000-13-02 Espen Sand
// It should be obvious that this eventfilter must only be
// be installed on the vertical scrollbar of the mIconList.
//
bool KJanusWidget::eventFilter( TQObject *o, TQEvent *e )
{
  if( e->type() == TQEvent::Show )
  {
    IconListItem *item = (IconListItem*)mIconList->item(0);
    if( item )
    {
      int lw = item->width( mIconList );
      int sw = mIconList->verticalScrollBar()->sizeHint().width();
      mIconList->setFixedWidth( lw+sw+mIconList->frameWidth()*2 );
    }
  }
  else if( e->type() == TQEvent::Hide )
  {
    IconListItem *item = (IconListItem*)mIconList->item(0);
    if( item )
    {
      int lw = item->width( mIconList );
      mIconList->setFixedWidth( lw+mIconList->frameWidth()*2 );
    }
  }
  return TQWidget::eventFilter( o, e );
}



//
// Code for the icon list box
//


KJanusWidget::IconListBox::IconListBox( TQWidget *parent, const char *name,
					WFlags f )
  :TDEListBox( parent, name, f ), mShowAll(false), mHeightValid(false),
   mWidthValid(false),
   mOldItem(0) 
{
}

void KJanusWidget::IconListBox::updateMinimumHeight()
{
  if( mShowAll && !mHeightValid )
  {
    int h = frameWidth()*2;
    for( TQListBoxItem *i = item(0); i; i = i->next() )
    {
      h += i->height( this );
    }
    setMinimumHeight( h );
    mHeightValid = true;
  }
}


void KJanusWidget::IconListBox::updateWidth()
{
  if( !mWidthValid )
  {
    int maxWidth = 10;
    for( TQListBoxItem *i = item(0); i; i = i->next() )
    {
      int w = ((IconListItem *)i)->width(this);
      maxWidth = TQMAX( w, maxWidth );
    }

    for( TQListBoxItem *i = item(0); i; i = i->next() )
    {
      ((IconListItem *)i)->expandMinimumWidth( maxWidth );
    }

    if( verticalScrollBar()->isVisible() )
    {
      maxWidth += verticalScrollBar()->sizeHint().width();
    }

    setFixedWidth( maxWidth + frameWidth()*2 );
    mWidthValid = true;
  }
}


void KJanusWidget::IconListBox::invalidateHeight()
{
  mHeightValid = false;
}


void KJanusWidget::IconListBox::invalidateWidth()
{
  mWidthValid = false;
}


void KJanusWidget::IconListBox::setShowAll( bool showAll )
{
  mShowAll = showAll;
  mHeightValid = false;
}


void KJanusWidget::IconListBox::leaveEvent( TQEvent *ev )
{
  TDEListBox::leaveEvent( ev ); 

  if ( mOldItem && !mOldItem->isSelected() )
  {
    ((KJanusWidget::IconListItem *) mOldItem)->highlight( true );
    mOldItem = 0;
  }
} 

// hack because qt does not support TQ_OBJECT in nested classes
void KJanusWidget::IconListBox::slotOnItem(TQListBoxItem *qitem)
{
  TDEListBox::slotOnItem( qitem );

  if ( qitem == mOldItem )
  {
    return;
  }
 
  if ( mOldItem && !mOldItem->isSelected() )
  {
    ((KJanusWidget::IconListItem *) mOldItem)->highlight( true );
  }

  KJanusWidget::IconListItem *item = dynamic_cast< KJanusWidget::IconListItem * >( qitem );
  if ( item && !item->isSelected() )
  {      
    item->highlight( false );
    mOldItem = item;
  }
  else
  {
    mOldItem = 0;
  }
}  



KJanusWidget::IconListItem::IconListItem( TQListBox *listbox, const TQPixmap &pixmap,
                                          const TQString &text )
  : TQListBoxItem( listbox )
{
  mPixmap = pixmap;
  if( mPixmap.isNull() )
  {
    mPixmap = defaultPixmap();
  }
  setText( text );
  setCustomHighlighting( true );
  mMinimumWidth = 0;
}


int KJanusWidget::IconListItem::expandMinimumWidth( int width )
{
  mMinimumWidth = TQMAX( mMinimumWidth, width );
  return mMinimumWidth;
}


void KJanusWidget::IconListItem::highlight( bool erase )
{
   // FIXME: Add configuration option to disable highlighting
   // For now, always disable highlighting
   erase = true;

   TQRect r = listBox()->itemRect( this );
   r.addCoords( 1, 1, -1, -1 );

   TQPainter p( listBox()->viewport() );
   p.setClipRegion( r );

   const TQColorGroup &cg = listBox()->colorGroup();
   if ( erase )
   {
      p.setPen( cg.base() );
      p.setBrush( cg.base() );
      p.drawRect( r );
   }
   else
   {
      p.setBrush( cg.highlight().light( 120 ) );
      p.drawRect( r );

      p.setPen( cg.highlight().dark( 140 ) );
      p.drawRect( r );
   }

   p.setPen( cg.foreground() );
   p.translate( r.x() - 1, r.y() - 1 );
   paintContents( &p );
}


const TQPixmap &KJanusWidget::IconListItem::defaultPixmap()
{
  static TQPixmap *pix=0;
  if( !pix )
  {
    pix = new TQPixmap( 32, 32 );
    TQPainter p( pix );
    p.eraseRect( 0, 0, pix->width(), pix->height() );
    p.setPen( TQt::red );
    p.drawRect ( 0, 0, pix->width(), pix->height() );
    p.end();

    TQBitmap mask( pix->width(), pix->height(), true );
    mask.fill( TQt::black );
    p.begin( &mask );
    p.setPen( TQt::white );
    p.drawRect ( 0, 0, pix->width(), pix->height() );
    p.end();

    pix->setMask( mask );
  }
  return *pix;
}


void KJanusWidget::IconListItem::paint( TQPainter *painter )
{
  TQRect itemPaintRegion( listBox()->itemRect( this ) );
  TQRect r( 1, 1, itemPaintRegion.width() - 2, itemPaintRegion.height() - 2);

  if ( isSelected() )
  {
    painter->eraseRect( r );

    painter->save();
    painter->setPen( listBox()->colorGroup().highlight().dark( 160 ) );
    painter->drawRect( r );
    painter->restore();
  }

  paintContents( painter );
}


void KJanusWidget::IconListItem::paintContents( TQPainter *painter )
{
  TQFontMetrics fm = painter->fontMetrics();
  int ht = fm.boundingRect( 0, 0, 0, 0, TQt::AlignCenter, text() ).height();
  int wp = mPixmap.width();
  int hp = mPixmap.height();
  painter->drawPixmap( (mMinimumWidth - wp) / 2, 5, mPixmap );

  if( !text().isEmpty() )
  {
    painter->drawText( 1, hp + 7, mMinimumWidth - 2, ht, TQt::AlignCenter, text() );
  }
}

int KJanusWidget::IconListItem::height( const TQListBox *lb ) const
{
  if( text().isEmpty() )
  {
    return mPixmap.height();
  }
  else
  {
    int ht = lb->fontMetrics().boundingRect( 0, 0, 0, 0, TQt::AlignCenter, text() ).height();
    return (mPixmap.height() + ht + 10);
  }
}


int KJanusWidget::IconListItem::width( const TQListBox *lb ) const
{
  int wt = lb->fontMetrics().boundingRect( 0, 0, 0, 0, TQt::AlignCenter, text() ).width() + 10;
  int wp = mPixmap.width() + 10;
  int w  = TQMAX( wt, wp );
  return TQMAX( w, mMinimumWidth );
}


void KJanusWidget::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


// TODO: In TreeList, if the last child of a node is removed, and there is no corrsponding widget for that node, allow the caller to
// delete the node.
void KJanusWidget::removePage( TQWidget *page )
{
  if (!d || !d->mPageToInt.contains(page))
    return;

  int index = d->mPageToInt[page];

  if ( mFace == TreeList )
  {
    TQMap<TQListViewItem*, TQWidget *>::Iterator i;
    for( i = mTreeListToPageStack.begin(); i != mTreeListToPageStack.end(); ++i )
      if (i.data()==page)
      {
        delete i.key();
        mPageStack->removeWidget(page);
        mTreeListToPageStack.remove(i);
        d->mIntToTitle.remove(index);
        d->mPageToInt.remove(page);
        d->mIntToPage.remove(index);
        break;
      }
  }
  else if ( mFace == IconList )
  {
    TQMap<TQListBoxItem*, TQWidget *>::Iterator i;
    for( i = mIconListToPageStack.begin(); i != mIconListToPageStack.end(); ++i )
      if (i.data()==page)
      {
        delete i.key();
        mPageStack->removeWidget(page);
        mIconListToPageStack.remove(i);
        d->mIntToTitle.remove(index);
        d->mPageToInt.remove(page);
        d->mIntToPage.remove(index);
        break;
      }
  }
  else // Tabbed
  {
    mTabControl->removePage(page);
    d->mPageToInt.remove(page);
    d->mIntToPage.remove(index);
  }
}


TQString KJanusWidget::pageTitle(int index) const
{
  if (!d || !d->mIntToTitle.contains(index))
    return TQString::null;
  else
    return d->mIntToTitle[index];
}


TQWidget *KJanusWidget::pageWidget(int index) const
{
  if (!d || !d->mIntToPage.contains(index))
    return 0;
  else
    return d->mIntToPage[index];
}

#include "kjanuswidget.moc"
