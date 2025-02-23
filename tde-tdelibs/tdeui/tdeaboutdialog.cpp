/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm <mirko@kde.org> and
 *  Espen Sand <espensa@online.no>
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
 *
 */

#include <tqclipboard.h>
#include <tqimage.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <ktextedit.h>
#include <tqobjectlist.h>
#include <tqpainter.h>
#include <tqrect.h>
#include <tqtabwidget.h>
#include <tqtabbar.h>

#include <tdeapplication.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <tdelocale.h>
#include <ktextbrowser.h>
#include <kurllabel.h>
#include <tdeaboutdialog.h>
#include <kaboutdialog_private.h>
#include <kdebug.h>

//MOC_SKIP_BEGIN
template class TQPtrList<TDEAboutContributor>;
//MOC_SKIP_END

#define WORKTEXT_IDENTATION 16
#define Grid 3

// ##############################################################
// MOC OUTPUT FILES:
#include "tdeaboutdialog.moc"
#include "kaboutdialog_private.moc"
// ##############################################################

class TDEAboutTabWidget : public TQTabWidget
{
public:
    TDEAboutTabWidget( TQWidget* parent ) : TQTabWidget( parent ) {}
    TQSize sizeHint() const {
	return TQTabWidget::sizeHint().expandedTo( tabBar()->sizeHint() + TQSize(4,4) );
    }
};




TDEAboutContributor::TDEAboutContributor( TQWidget *_parent, const char *wname,
			              const TQString &_name,const TQString &_email,
			              const TQString &_url, const TQString &_work,
			              bool showHeader, bool showFrame,
				      bool showBold )
  : TQFrame( _parent, wname ), mShowHeader(showHeader), mShowBold(showBold), d(0)
{
  if( showFrame )
  {
    setFrameStyle(TQFrame::Panel | TQFrame::Raised);
  }

  mLabel[0] = new TQLabel( this );
  mLabel[1] = new TQLabel( this );
  mLabel[2] = new TQLabel( this );
  mLabel[3] = new TQLabel( this );
  mText[0] = new TQLabel( this );
  mText[1] = new KURLLabel( this );
  mText[2] = new KURLLabel( this );
  mText[3] = new TQLabel( this );

  setName( _name, i18n("Author"), false );
  setEmail( _email, i18n("Email"), false );
  setURL( _url, i18n("Homepage"), false );
  setWork( _work, i18n("Task"), false );

  KURLLabel *kurl = static_cast<KURLLabel *>(mText[1]);
  kurl->setFloat(true);
  kurl->setUnderline(true);
  kurl->setMargin(0);
  connect(kurl, TQ_SIGNAL(leftClickedURL(const TQString &)),
	  TQ_SLOT(emailClickedSlot(const TQString &)));

  kurl = static_cast<KURLLabel *>(mText[2]);
  kurl->setFloat(true);
  kurl->setUnderline(true);
  kurl->setMargin(0);
  connect(kurl, TQ_SIGNAL(leftClickedURL(const TQString &)),
	  TQ_SLOT(urlClickedSlot(const TQString &)));

  mLabel[3]->setAlignment( AlignTop );

  fontChange( font() );
  updateLayout();
}


void TDEAboutContributor::setName( const TQString &_text, const TQString &_header,
				 bool _update )
{
  mLabel[0]->setText(_header);
  mText[0]->setText(_text);
  if( _update ) { updateLayout(); }
}


void TDEAboutContributor::setEmail( const TQString &_text, const TQString &_header,
				  bool _update )
{
  mLabel[1]->setText(_header);
  KURLLabel* const kurl = static_cast<KURLLabel *>(mText[1]);
  kurl->setText(_text);
  kurl->setURL(_text);
  if( _update ) { updateLayout(); }
}


void TDEAboutContributor::setURL( const TQString &_text, const TQString &_header,
				bool _update )
{
  mLabel[2]->setText(_header);
  KURLLabel* const kurl = static_cast<KURLLabel *>(mText[2]);
  kurl->setText(_text);
  kurl->setURL(_text);
  if( _update ) { updateLayout(); }
}


void TDEAboutContributor::setWork( const TQString &_text, const TQString &_header,
				 bool _update )
{
  mLabel[3]->setText(_header);
  mText[3]->setText(_text);
  if( _update ) { updateLayout(); }
}


TQString TDEAboutContributor::getName( void ) const
{
  return mText[0]->text();
}


TQString TDEAboutContributor::getEmail( void ) const
{
  return mText[1]->text();
}


TQString TDEAboutContributor::getURL( void ) const
{
  return mText[2]->text();
}


TQString TDEAboutContributor::getWork( void ) const
{
  return mText[3]->text();
}



void TDEAboutContributor::updateLayout( void )
{
  delete layout();

  int row = 0;
  if( !mText[0]->text().isEmpty() ) { ++row; }
  if( !mText[1]->text().isEmpty() ) { ++row; }
  if( !mText[2]->text().isEmpty() ) { ++row; }
  if( !mText[3]->text().isEmpty() ) { ++row; }


  TQGridLayout *gbox;
  if( row == 0 )
  {
    gbox = new TQGridLayout( this, 1, 1, 0 );
    for( int i=0; i<4; ++i )
    {
      mLabel[i]->hide();
      mText[i]->hide();
    }
  }
  else
  {
    if( mText[0]->text().isEmpty() && !mShowHeader )
    {
      gbox = new TQGridLayout( this, row, 1, frameWidth()+1, 2 );
    }
    else
    {
      gbox = new TQGridLayout( this, row, 2, frameWidth()+1, 2 );
      if( !mShowHeader )
      {
	gbox->addColSpacing( 0, KDialog::spacingHint()*2 );
      }
      gbox->setColStretch( 1, 10 );
    }

    for( int i=0, r=0; i<4; ++i )
    {
      mLabel[i]->setFixedHeight( fontMetrics().lineSpacing() );
      if( i != 3 )
      {
	mText[i]->setFixedHeight( fontMetrics().lineSpacing() );
      }

      if( !mText[i]->text().isEmpty() )
      {
	if( mShowHeader )
	{
	  gbox->addWidget( mLabel[i], r, 0, (TQt::AlignmentFlags)AlignLeft );
	  gbox->addWidget( mText[i], r, 1, (TQt::AlignmentFlags)AlignLeft  );
	  mLabel[i]->show();
	  mText[i]->show();
	}
	else
	{
	  mLabel[i]->hide();
	  if( !i )
	  {
	    gbox->addMultiCellWidget( mText[i], r, r, 0, 1, (TQt::AlignmentFlags)AlignLeft );
	  }
	  else
	  {
	    gbox->addWidget( mText[i], r, 1, (TQt::AlignmentFlags)AlignLeft  );
	  }
	  mText[i]->show();
	}
	++r;
      }
      else
      {
	mLabel[i]->hide();
	mText[i]->hide();
      }
    }
  }

  gbox->activate();
  setMinimumSize( sizeHint() );
}


void TDEAboutContributor::fontChange( const TQFont &/*oldFont*/ )
{
  if( mShowBold )
  {
    TQFont f( font() );
    f.setBold( true );
    mText[0]->setFont( f );
  }
  update();
}


TQSize TDEAboutContributor::sizeHint( void ) const
{
  return minimumSizeHint();
}


void TDEAboutContributor::urlClickedSlot( const TQString &u )
{
  emit openURL(u);
}


void TDEAboutContributor::emailClickedSlot( const TQString &e )
{
  emit sendEmail( mText[0]->text(), e ) ;
}


//
// Internal widget for the TDEAboutDialog class.
//
TDEAboutContainerBase::TDEAboutContainerBase( int layoutType, TQWidget *_parent,
					  char *_name )
  : TQWidget( _parent, _name ),
    mImageLabel(0), mTitleLabel(0), mIconLabel(0),mVersionLabel(0),
    mAuthorLabel(0), mImageFrame(0),mPageTab(0),mPlainSpace(0),d(0)
{
  mTopLayout = new TQVBoxLayout( this, 0, KDialog::spacingHint() );
  if( !mTopLayout ) { return; }

  if( layoutType & AbtImageOnly )
  {
    layoutType &= ~(AbtImageLeft|AbtImageRight|AbtTabbed|AbtPlain);
  }
  if( layoutType & AbtImageLeft )
  {
    layoutType &= ~AbtImageRight;
  }

  if( layoutType & AbtTitle )
  {
    mTitleLabel = new TQLabel( this, "title" );
    mTitleLabel->setAlignment(AlignCenter);
    mTopLayout->addWidget( mTitleLabel );
    mTopLayout->addSpacing( KDialog::spacingHint() );
  }

  if( layoutType & AbtProduct )
  {
    TQWidget* const productArea = new  TQWidget( this, "area" );
    mTopLayout->addWidget( productArea, 0, TQApplication::reverseLayout() ? AlignRight : AlignLeft );

    TQHBoxLayout* const hbox = new TQHBoxLayout(productArea,0,KDialog::spacingHint());
    if( !hbox ) { return; }

    mIconLabel = new TQLabel( productArea );
    hbox->addWidget( mIconLabel, 0, AlignLeft|AlignHCenter );

    TQVBoxLayout* const vbox = new TQVBoxLayout();
    if( !vbox ) { return; }
    hbox->addLayout( vbox );

    mVersionLabel = new TQLabel( productArea, "version" );
    mAuthorLabel  = new TQLabel( productArea, "author" );
    vbox->addWidget( mVersionLabel );
    vbox->addWidget( mAuthorLabel );
    hbox->activate();

    mTopLayout->addSpacing( KDialog::spacingHint() );
  }

  TQHBoxLayout* const hbox = new TQHBoxLayout();
  if( !hbox ) { return; }
  mTopLayout->addLayout( hbox, 10 );

  if( layoutType & AbtImageLeft )
  {
    TQVBoxLayout* vbox = new TQVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new TQFrame( this );
    setImageFrame( true );
    vbox->addWidget( mImageFrame );
    vbox->addSpacing(1);

    vbox = new TQVBoxLayout( mImageFrame, 1 );
    mImageLabel = new KImageTrackLabel( mImageFrame );
    connect( mImageLabel, TQ_SIGNAL(mouseTrack( int, const TQMouseEvent * )),
	     TQ_SLOT( slotMouseTrack( int, const TQMouseEvent * )) );
    vbox->addStretch(10);
    vbox->addWidget( mImageLabel );
    vbox->addStretch(10);
    vbox->activate();
  }

  if( layoutType & AbtTabbed )
  {
    mPageTab = new TDEAboutTabWidget( this );
    if( !mPageTab ) { return; }
    hbox->addWidget( mPageTab, 10 );
  }
  else if( layoutType & AbtImageOnly )
  {
    mImageFrame = new TQFrame( this );
    setImageFrame( true );
    hbox->addWidget( mImageFrame, 10 );

    TQGridLayout* const gbox = new TQGridLayout(mImageFrame, 3, 3, 1, 0 );
    gbox->setRowStretch( 0, 10 );
    gbox->setRowStretch( 2, 10 );
    gbox->setColStretch( 0, 10 );
    gbox->setColStretch( 2, 10 );

    mImageLabel = new KImageTrackLabel( mImageFrame );
    connect( mImageLabel, TQ_SIGNAL(mouseTrack( int, const TQMouseEvent * )),
	     TQ_SLOT( slotMouseTrack( int, const TQMouseEvent * )) );
    gbox->addWidget( mImageLabel, 1, 1 );
    gbox->activate();
  }
  else
  {
    mPlainSpace = new TQFrame( this );
    if( !mPlainSpace ) { return; }
    hbox->addWidget( mPlainSpace, 10 );
  }

  if( layoutType & AbtImageRight )
  {
    TQVBoxLayout *vbox = new TQVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new TQFrame( this );
    setImageFrame( true );
    vbox->addWidget( mImageFrame );
    vbox->addSpacing(1);

    vbox = new TQVBoxLayout( mImageFrame, 1 );
    mImageLabel = new KImageTrackLabel( mImageFrame );
    connect( mImageLabel, TQ_SIGNAL(mouseTrack( int, const TQMouseEvent * )),
	     TQ_SLOT( slotMouseTrack( int, const TQMouseEvent * )) );
    vbox->addStretch(10);
    vbox->addWidget( mImageLabel );
    vbox->addStretch(10);
    vbox->activate();
  }

  fontChange( font() );
}


void TDEAboutContainerBase::show( void )
{
    TQWidget::show();
}

TQSize TDEAboutContainerBase::sizeHint( void ) const
{
    return minimumSize().expandedTo( TQSize( TQWidget::sizeHint().width(), 0 ) );
}

void TDEAboutContainerBase::fontChange( const TQFont &/*oldFont*/ )
{
  if( mTitleLabel )
  {
    TQFont f( TDEGlobalSettings::generalFont() );
    f.setBold( true );
    int fs = f.pointSize();
    if (fs == -1)
       fs = TQFontInfo(f).pointSize();
    f.setPointSize( fs+2 ); // Lets not make it too big
    mTitleLabel->setFont(f);
  }

  if( mVersionLabel )
  {
    TQFont f( TDEGlobalSettings::generalFont() );
    f.setBold( true );
    mVersionLabel->setFont(f);
    mAuthorLabel->setFont(f);
    mVersionLabel->parentWidget()->layout()->activate();
  }

  update();
}

TQFrame *TDEAboutContainerBase::addTextPage( const TQString &title,
					  const TQString &text,
					  bool richText, int numLines )
{
  TQFrame* const page = addEmptyPage( title );
  if( !page ) { return 0; }
  if( numLines <= 0 ) { numLines = 10; }

  TQVBoxLayout* const vbox = new TQVBoxLayout( page, KDialog::spacingHint() );

  if( richText )
  {
    KTextBrowser* const browser = new KTextBrowser( page, "browser" );
    browser->setHScrollBarMode( TQScrollView::AlwaysOff );
    browser->setText( text );
    browser->setMinimumHeight( fontMetrics().lineSpacing()*numLines );

    vbox->addWidget(browser);
    connect(browser, TQ_SIGNAL(urlClick(const TQString &)),
	    TQ_SLOT(slotUrlClick(const TQString &)));
    connect(browser, TQ_SIGNAL(mailClick(const TQString &,const TQString &)),
	    TQ_SLOT(slotMailClick(const TQString &,const TQString &)));
  }
  else
  {
    KTextEdit* const textEdit = new KTextEdit( page, "text" );
    textEdit->setReadOnly( true );
    textEdit->setMinimumHeight( fontMetrics().lineSpacing()*numLines );
    textEdit->setWordWrap( TQTextEdit::NoWrap );
    vbox->addWidget( textEdit );
  }

  return page;
}

TQFrame *TDEAboutContainerBase::addLicensePage( const TQString &title,
					  const TQString &text, int numLines)
{
  TQFrame* const page = addEmptyPage( title );
  if( !page ) { return 0; }
  if( numLines <= 0 ) { numLines = 10; }

  TQVBoxLayout* const vbox = new TQVBoxLayout( page, KDialog::spacingHint() );

  KTextEdit* const textEdit = new KTextEdit( page, "license" );
  textEdit->setFont( TDEGlobalSettings::fixedFont() );
  textEdit->setReadOnly( true );
  textEdit->setWordWrap( TQTextEdit::NoWrap );
  textEdit->setText( text );
  textEdit->setMinimumHeight( fontMetrics().lineSpacing()*numLines );
  vbox->addWidget( textEdit );
  return page;
}


TDEAboutContainer *TDEAboutContainerBase::addContainerPage( const TQString &title,
							int childAlignment,
							int innerAlignment )
{
  if( !mPageTab )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  TDEAboutContainer* const container = new TDEAboutContainer( mPageTab, "container",
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
						  innerAlignment );
  mPageTab->addTab( container, title );

  connect(container, TQ_SIGNAL(urlClick(const TQString &)),
	  TQ_SLOT(slotUrlClick(const TQString &)));
  connect(container, TQ_SIGNAL(mailClick(const TQString &,const TQString &)),
	  TQ_SLOT(slotMailClick(const TQString &,const TQString &)));

  return container;
}


TDEAboutContainer *TDEAboutContainerBase::addScrolledContainerPage(
				      const TQString &title,
				      int childAlignment,
				      int innerAlignment )
{
  if( !mPageTab )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  TQFrame* const page = addEmptyPage( title );
  TQVBoxLayout* const vbox = new TQVBoxLayout( page, KDialog::spacingHint() );
  TQScrollView* const scrollView = new TQScrollView( page );
  scrollView->viewport()->setBackgroundMode( PaletteBackground );
  vbox->addWidget( scrollView );

  TDEAboutContainer* const container = new TDEAboutContainer( scrollView, "container",
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
    innerAlignment );
  scrollView->addChild( container );


  connect(container, TQ_SIGNAL(urlClick(const TQString &)),
	  TQ_SLOT(slotUrlClick(const TQString &)));
  connect(container, TQ_SIGNAL(mailClick(const TQString &,const TQString &)),
	  TQ_SLOT(slotMailClick(const TQString &,const TQString &)));

  return container;
}


TQFrame *TDEAboutContainerBase::addEmptyPage( const TQString &title )
{
  if( !mPageTab )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  TQFrame* const page = new TQFrame( mPageTab, title.latin1() );
  page->setFrameStyle( TQFrame::NoFrame );

  mPageTab->addTab( page, title );
  return page;
}


TDEAboutContainer *TDEAboutContainerBase::addContainer( int childAlignment,
						    int innerAlignment )
{
  TDEAboutContainer* const container = new TDEAboutContainer( this, "container",
    0, KDialog::spacingHint(), childAlignment, innerAlignment );
  mTopLayout->addWidget( container, 0, childAlignment );

  connect(container, TQ_SIGNAL(urlClick(const TQString &)),
	  TQ_SLOT(slotUrlClick(const TQString &)));
  connect(container, TQ_SIGNAL(mailClick(const TQString &,const TQString &)),
	  TQ_SLOT(slotMailClick(const TQString &,const TQString &)));

  return container;
}



void TDEAboutContainerBase::setTitle( const TQString &title )
{
  if( !mTitleLabel )
  {
    kdDebug(291) << "setTitle: " << "Invalid layout" << endl;
    return;
  }
  mTitleLabel->setText(title);
}


void TDEAboutContainerBase::setImage( const TQString &fileName )
{
  if( !mImageLabel )
  {
    kdDebug(291) << "setImage: " << "Invalid layout" << endl;
    return;
  }
  if( fileName.isNull() )
  {
    return;
  }

  const TQPixmap logo( fileName );
  if( !logo.isNull() )
    mImageLabel->setPixmap( logo );

  mImageFrame->layout()->activate();
}

void TDEAboutContainerBase::setProgramLogo( const TQString &fileName )
{
  if( fileName.isNull() )
  {
    return;
  }

  const TQPixmap logo( fileName );
  setProgramLogo( logo );
}

void TDEAboutContainerBase::setProgramLogo( const TQPixmap &pixmap )
{
  if( !mIconLabel )
  {
    kdDebug(291) << "setProgramLogo: " << "Invalid layout" << endl;
    return;
  }
  if( !pixmap.isNull() )
  {
    mIconLabel->setPixmap( pixmap );
  }
}

void TDEAboutContainerBase::setImageBackgroundColor( const TQColor &color )
{
  if( mImageFrame )
  {
    mImageFrame->setBackgroundColor( color );
  }
}


void TDEAboutContainerBase::setImageFrame( bool state )
{
  if( mImageFrame )
  {
    if( state )
    {
      mImageFrame->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
      mImageFrame->setLineWidth(1);
    }
    else
    {
      mImageFrame->setFrameStyle( TQFrame::NoFrame );
      mImageFrame->setLineWidth(0);
    }
  }
}


void TDEAboutContainerBase::setProduct( const TQString &appName,
				      const TQString &version,
				      const TQString &author,
				      const TQString &year )
{
  if( !mIconLabel )
  {
    kdDebug(291) << "setProduct: " << "Invalid layout" << endl;
    return;
  }

  if ( kapp )
  {
    mIconLabel->setPixmap( kapp->icon() );
    kdDebug(291) << "setPixmap (iconName): " << kapp->iconName() << endl;
  }
  else
    kdDebug(291) << "no kapp" << endl;

  const TQString msg1 = i18n("%1 %2 (Using Trinity %3)").arg(appName).arg(version).
    arg(TQString::fromLatin1(TDE_VERSION_STRING));
  const TQString msg2 = !year.isEmpty() ? i18n("%1 %2, %3").arg('©').arg(year).
    arg(author) : TQString::fromLatin1("");

  //if (!year.isEmpty())
  //  msg2 = i18n("%1 %2, %3").arg('©').arg(year).arg(author);

  mVersionLabel->setText( msg1 );
  mAuthorLabel->setText( msg2 );
  if( msg2.isEmpty() )
  {
    mAuthorLabel->hide();
  }

  mIconLabel->parentWidget()->layout()->activate();
}


void TDEAboutContainerBase::slotMouseTrack( int mode, const TQMouseEvent *e )
{
  emit mouseTrack( mode, e );
}


void TDEAboutContainerBase::slotUrlClick( const TQString &url )
{
  emit urlClick( url );
}

void TDEAboutContainerBase::slotMailClick( const TQString &_name,
					 const TQString &_address )
{
  emit mailClick( _name, _address );
}



TDEAboutContainer::TDEAboutContainer( TQWidget *_parent, const char *_name,
				  int _margin, int _spacing,
				  int childAlignment, int innerAlignment )
  : TQFrame( _parent, _name ), d(0)
{
  mAlignment = innerAlignment;

  TQGridLayout* const gbox = new TQGridLayout( this, 3, 3, _margin, _spacing );
  if( childAlignment & AlignHCenter )
  {
    gbox->setColStretch( 0, 10 );
    gbox->setColStretch( 2, 10 );
  }
  else if( childAlignment & AlignRight )
  {
    gbox->setColStretch( 0, 10 );
  }
  else
  {
    gbox->setColStretch( 2, 10 );
  }

  if( childAlignment & AlignVCenter )
  {
    gbox->setRowStretch( 0, 10 );
    gbox->setRowStretch( 2, 10 );
  }
  else if( childAlignment & AlignRight )
  {
    gbox->setRowStretch( 0, 10 );
  }
  else
  {
    gbox->setRowStretch( 2, 10 );
  }

  mVbox = new TQVBoxLayout( _spacing );
  gbox->addLayout( mVbox, 1, 1 );
  gbox->activate();
}


void TDEAboutContainer::childEvent( TQChildEvent *e )
{
  if( !e->inserted() || !e->child()->isWidgetType() )
  {
    return;
  }

  TQWidget* const w = static_cast<TQWidget *>(e->child());
  mVbox->addWidget( w, 0, mAlignment );
  const TQSize s( sizeHint() );
  setMinimumSize( s );

  TQObjectList const l = childrenListObject(); // silence please
  TQObjectListIterator itr( l );
  TQObject * o;
  while ( (o = itr.current()) ) {
    ++itr;
    if( o->isWidgetType() )
    {
        static_cast<TQWidget*>(o)->setMinimumWidth( s.width() );
    }
  }
}


TQSize TDEAboutContainer::sizeHint( void ) const
{
  //
  // The size is computed by adding the sizeHint().height() of all
  // widget children and taking the width of the widest child and adding
  // layout()->margin() and layout()->spacing()
  //

  TQSize total_size;

  int numChild = 0;
  TQObjectList const l = childrenListObject(); // silence please

  TQObjectListIterator itr( l );
  TQObject * o;
  while ( (o = itr.current()) ) {
    ++itr;
    if( o->isWidgetType() )
    {
      ++numChild;
      TQWidget* const w= static_cast<TQWidget*>(o);

      TQSize s = w->minimumSize();
      if( s.isEmpty() )
      {
	s = w->minimumSizeHint();
	if( s.isEmpty() )
	{
	  s = w->sizeHint();
	  if( s.isEmpty() )
	  {
	    s = TQSize( 100, 100 ); // Default size
	  }
	}
      }
      total_size.setHeight( total_size.height() + s.height() );
      if( s.width() > total_size.width() ) { total_size.setWidth( s.width() ); }
    }
  }

  if( numChild > 0 )
  {
    //
    // Seems I have to add 1 to the height to properly show the border
    // of the last entry if layout()->margin() is 0
    //

    total_size.setHeight( total_size.height() + layout()->spacing()*(numChild-1) );
    total_size += TQSize( layout()->margin()*2, layout()->margin()*2 + 1 );
  }
  else
  {
    total_size = TQSize( 1, 1 );
  }
  return total_size;
}


TQSize TDEAboutContainer::minimumSizeHint( void ) const
{
  return sizeHint();
}


void TDEAboutContainer::addWidget( TQWidget *widget )
{
  widget->reparent( this, 0, TQPoint(0,0) );
}


void TDEAboutContainer::addPerson( const TQString &_name, const TQString &_email,
				 const TQString &_url, const TQString &_task,
				 bool showHeader, bool showFrame,bool showBold)
{

  TDEAboutContributor* const cont = new TDEAboutContributor( this, "pers",
    _name, _email, _url, _task, showHeader, showFrame, showBold );
  connect( cont, TQ_SIGNAL( openURL(const TQString&)),
	   this, TQ_SIGNAL( urlClick(const TQString &)));
  connect( cont, TQ_SIGNAL( sendEmail(const TQString &, const TQString &)),
	   this, TQ_SIGNAL( mailClick(const TQString &, const TQString &)));
}


void TDEAboutContainer::addTitle( const TQString &title, int alignment,
				bool showFrame, bool showBold )
{

  TQLabel* const label = new TQLabel( title, this, "title" );
  if( showBold  )
  {
    TQFont labelFont( font() );
    labelFont.setBold( true );
    label->setFont( labelFont );
  }
  if( showFrame )
  {
    label->setFrameStyle(TQFrame::Panel | TQFrame::Raised);
  }
  label->setAlignment( alignment );
}


void TDEAboutContainer::addImage( const TQString &fileName, int alignment )
{
  if( fileName.isNull() )
  {
    return;
  }

  KImageTrackLabel* const label = new KImageTrackLabel( this, "image" );
  const TQImage logo( fileName );
  if( !logo.isNull() )
  {
    TQPixmap pix;
    pix = logo;
    label->setPixmap( pix );
  }
  label->setAlignment( alignment );
}

#if 0
//MOC_SKIP_BEGIN

/** Every person displayed is stored in a TDEAboutContributor object.
 *  Every contributor, the author and/or the maintainer of the application are
 *  stored in objects of this local class. Every single field may be empty.
 *  To add a contributor, create a TDEAboutContributor object as a child of your
 *  @ref TDEAboutDialog, set its contents and add it using add addContributor. */
class TDEAboutContributor : public QFrame
{
  // ############################################################################
  TQ_OBJECT
  // ----------------------------------------------------------------------------
public:
  /** The Qt constructor. */
  TDEAboutContributor(TQWidget* parent=0, const char* name=0);
  /** Set the name (a literal string). */
  void setName(const TQString&);
  /** Get the name. */
  TQString getName();
  /** The email address (dito). */
  void setEmail(const TQString&);
  /** Get the email address. */
  TQString getEmail();
  /** The URL (dito). */
  void setURL(const TQString&);
  /** Get the URL. */
  TQString getURL();
  /** The tasks the person worked on (a literal string). More than one line is
   *  possible, but very long texts might look ugly. */
  void setWork(const TQString&);
  /** The size hint. Very important here, since TDEAboutWidget relies on it for
   *  geometry management. */
  TQSize sizeHint();
  TQSize minimumSizeHint(void);
  virtual void show( void );

  // ----------------------------------------------------------------------------
protected:
  // events:
  /** The resize event. */
  void resizeEvent(TQResizeEvent*);
  /** The paint event. */
  void paintEvent(TQPaintEvent*);
  /** The label showing the program version. */
  TQLabel *name;
  /** The clickable URL label showing the email address. It is only visible if
   *  its text is not empty. */
  KURLLabel *email;
  /** Another interactive part that displays the homepage URL. */
  KURLLabel *url;
  /** The description of the contributions of the person. */
  TQString work;
  // ----------------------------------------------------------------------------
protected slots:
  /** The homepage URL has been clicked. */
  void urlClickedSlot(const TQString&);
  /** The email address has been clicked. */
  void emailClickedSlot(const TQString& emailaddress);
  // ----------------------------------------------------------------------------
signals:
  /** The email address has been clicked. */
  void sendEmail(const TQString& name, const TQString& email);
  /** The URL has been clicked. */
  void openURL(const TQString& url);
  // ############################################################################
};



TDEAboutContributor::TDEAboutContributor(TQWidget* parent, const char* n)
  : TQFrame(parent, n),
    name(new TQLabel(this)),
    email(new KURLLabel(this)),
    url(new KURLLabel(this))
{
  // ############################################################
  if(name==0 || email==0)
    { // this will nearly never happen (out of memory in about box?)
      kdDebug() << "TDEAboutContributor::TDEAboutContributor: Out of memory." << endl;
      tqApp->quit();
    }
  setFrameStyle(TQFrame::Panel | TQFrame::Raised);
  // -----
  connect(email, TQ_SIGNAL(leftClickedURL(const TQString&)),
	  TQ_SLOT(emailClickedSlot(const TQString&)));
  connect(url, TQ_SIGNAL(leftClickedURL(const TQString&)),
	  TQ_SLOT(urlClickedSlot(const TQString&)));
  // ############################################################
}

void
TDEAboutContributor::setName(const TQString& n)
{
  // ############################################################
  name->setText(n);
  // ############################################################
}

TQString
TDEAboutContributor::getName()
{
  // ###########################################################
  return name->text();
  // ###########################################################
}
void
TDEAboutContributor::setURL(const TQString& u)
{
  // ###########################################################
  url->setText(u);
  // ###########################################################
}

TQString
TDEAboutContributor::getURL()
{
  // ###########################################################
  return url->text();
  // ###########################################################
}

void
TDEAboutContributor::setEmail(const TQString& e)
{
  // ###########################################################
  email->setText(e);
  // ###########################################################
}

TQString
TDEAboutContributor::getEmail()
{
  // ###########################################################
  return email->text();
  // ###########################################################
}

void
TDEAboutContributor::emailClickedSlot(const TQString& e)
{
  // ###########################################################
  kdDebug() << "TDEAboutContributor::emailClickedSlot: called." << endl;
  emit(sendEmail(name->text(), e));
  // ###########################################################
}

void
TDEAboutContributor::urlClickedSlot(const TQString& u)
{
  // ###########################################################
  kdDebug() << "TDEAboutContributor::urlClickedSlot: called." << endl;
  emit(openURL(u));
  // ###########################################################
}

void
TDEAboutContributor::setWork(const TQString& w)
{
  // ###########################################################
  work=w;
  // ###########################################################
}

#endif


#if 0
QSize
TDEAboutContributor::sizeHint()
{
  // ############################################################################
  const int FrameWidth=frameWidth();
  const int WorkTextWidth=200;
  int maxx, maxy;
  TQRect rect;
  // ----- first calculate name and email width:
  maxx=name->sizeHint().width();
  maxx=TQMAX(maxx, email->sizeHint().width()+WORKTEXT_IDENTATION);
  // ----- now determine "work" text rectangle:
  if(!work.isEmpty()) // save time
    {
      rect=fontMetrics().boundingRect
	(0, 0, WorkTextWidth, 32000, WordBreak | AlignLeft, work);
    }
  if(maxx<rect.width())
  {
    maxx=WorkTextWidth+WORKTEXT_IDENTATION;
  }
  maxx=TQMAX(maxx, url->sizeHint().width()+WORKTEXT_IDENTATION);
  // -----
  maxy=2*(name->sizeHint().height()+Grid); // need a space above the KURLLabels
  maxy+=/* email */ name->sizeHint().height();
  maxy+=rect.height();
  // -----
  maxx+=2*FrameWidth;
  maxy+=2*FrameWidth;
  return TQSize(maxx, maxy);
  // ############################################################################
}

TQSize TDEAboutContributor::minimumSizeHint(void)
{
  return( sizeHint() );
}


void TDEAboutContributor::show( void )
{
  TQFrame::show();
  setMinimumSize( sizeHint() );
}



void
TDEAboutContributor::resizeEvent(TQResizeEvent*)
{ // the widgets are simply aligned from top to bottom, since the parent is
  // expected to respect the size hint
  // ############################################################################
  int framewidth=frameWidth(), childwidth=width()-2*framewidth;
  int cy=framewidth;
  // -----
  name->setGeometry
    (framewidth, framewidth, childwidth, name->sizeHint().height());
  cy=name->height()+Grid;
  email->setGeometry
    (framewidth+WORKTEXT_IDENTATION, cy,
     childwidth-WORKTEXT_IDENTATION, /* email */ name->sizeHint().height());
  cy+=name->height()+Grid;
  url->setGeometry
    (framewidth+WORKTEXT_IDENTATION, cy,
     childwidth-WORKTEXT_IDENTATION, /* url */ name->sizeHint().height());
  // the work text is drawn in the paint event
  // ############################################################################
}


void
TDEAboutContributor::paintEvent(TQPaintEvent* e)
{ // the widgets are simply aligned from top to bottom, since the parent is
  // expected to respect the size hint (the widget is only used locally by now)
  // ############################################################################
  int cy=frameWidth()+name->height()+email->height()+Grid+url->height()+Grid;
  int h=height()-cy-frameWidth();
  int w=width()-WORKTEXT_IDENTATION-2*frameWidth();
  // -----
  TQFrame::paintEvent(e);
  if(work.isEmpty()) return;
  TQPainter paint(this); // construct painter only if there is something to draw
  // -----
  paint.drawText(WORKTEXT_IDENTATION, cy, w, h, AlignLeft | WordBreak, work);
  // ############################################################################
}
// MOC_SKIP_END
#endif


#if 0
TQSize TDEAboutContributor::sizeHint( void )
{
  int s = KDialog::spacingHint();
  int h = fontMetrics().lineSpacing()*3 + 2*s;
  int m = frameWidth();

  int w = name->sizeHint().width();
  w = TQMAX( w, email->sizeHint().width()+s);
  w = TQMAX( w, url->sizeHint().width()+s);

  if( work.isEmpty() == false )
  {
    const int WorkTextWidth=200;
    TQRect r = fontMetrics().boundingRect
      (0, 0, WorkTextWidth, 32000, WordBreak | AlignLeft, work);
    if( w < r.width() )
    {
      w = TQMAX( w, WorkTextWidth+s );
    }
    h += TQMAX( fontMetrics().lineSpacing(), r.height() ) + s;
  }
  return( TQSize( w + 2*m, h + 2*m ) );


  /*
  int s = 3;
  int m = frameWidth() + KDialog::spacingHint();
  int h = ls * 3 + s * 2;
  int w = name->sizeHint().width();

  w = TQMAX( w, email->sizeHint().width()+WORKTEXT_IDENTATION);
  w = TQMAX( w, url->sizeHint().width()+WORKTEXT_IDENTATION);
  if( work.isEmpty() == false )
  {
    const int WorkTextWidth=200;

    TQRect r = fontMetrics().boundingRect
      (0, 0, WorkTextWidth, 32000, WordBreak | AlignLeft, work);
    if( w < r.width() )
    {
      w = TQMAX( w, WorkTextWidth + WORKTEXT_IDENTATION );
    }
    h += r.height() + s;
  }
  return( TQSize( w + 2*m, h + 2*m ) );
  */
}


//
// The widgets are simply aligned from top to bottom, since the parent is
// expected to respect the size hint
//
void TDEAboutContributor::resizeEvent(TQResizeEvent*)
{
  int x = frameWidth();
  int s = KDialog::spacingHint();
  int h = fontMetrics().lineSpacing();
  int w = width() - 2*x;
  int y = x;

  name->setGeometry( x, y, w, h );
  y += h + s;
  email->setGeometry( x+s, y, w-s, h );
  y += h + s;
  url->setGeometry( x+s, y, w-s, h );

  /*
  int x = frameWidth() + KDialog::spacingHint();
  int y = x;
  int w = width() - 2*x;
  int h = name->sizeHint().height();
  int s = 3;

  name->setGeometry( x, y, w, h );
  y += h + s;
  email->setGeometry( x+WORKTEXT_IDENTATION, y, w-WORKTEXT_IDENTATION, h );
  y += h + s;
  url->setGeometry( x+WORKTEXT_IDENTATION, y, w-WORKTEXT_IDENTATION, h );
  //
  // the work text is drawn in the paint event
  //
  */
}



void TDEAboutContributor::paintEvent( TQPaintEvent *e )
{
  TQFrame::paintEvent(e);
  if(work.isEmpty()) return;

  int x = frameWidth() + KDialog::spacingHint();
  int h = fontMetrics().lineSpacing();
  int y = height() - frameWidth() - fontMetrics().lineSpacing();
  int w = width() - frameWidth()*2 - KDialog::spacingHint();

  TQPainter paint( this );
  paint.drawText( x, y, w, h, AlignLeft | WordBreak, work );

  /*

  int s = 3;
  int x = frameWidth() + KDialog::spacingHint() + WORKTEXT_IDENTATION;
  int w = width()-WORKTEXT_IDENTATION-2*(frameWidth()+KDialog::spacingHint());
  int y = frameWidth()+KDialog::spacingHint()+(name->sizeHint().height()+s)*3;
  int h = height()-y-frameWidth();

  TQPainter paint( this );
  paint.drawText( x, y, w, h, AlignLeft | WordBreak, work );
  */
}
#endif






TDEAboutWidget::TDEAboutWidget(TQWidget *_parent, const char *_name)
  : TQWidget(_parent, _name),
    version(new TQLabel(this)),
    cont(new TQLabel(this)),
    logo(new TQLabel(this)),
    author(new TDEAboutContributor(this)),
    maintainer(new TDEAboutContributor(this)),
    showMaintainer(false),
    d(0)
{
  // #################################################################
  if( !version || !cont || !logo || !author || !maintainer )
  {
    // this will nearly never happen (out of memory in about box?)
    kdDebug() << "TDEAboutWidget::TDEAboutWidget: Out of memory." << endl;
    tqApp->quit();
  }
  // -----
  cont->setText(i18n("Other Contributors:"));
  logo->setText(i18n("(No logo available)"));
  logo->setFrameStyle(TQFrame::Panel | TQFrame::Raised);
  version->setAlignment(AlignCenter);
  // -----
  connect(author, TQ_SIGNAL(sendEmail(const TQString&, const TQString&)),
	  TQ_SLOT(sendEmailSlot(const TQString&, const TQString&)));
  connect(author, TQ_SIGNAL(openURL(const TQString&)),
	  TQ_SLOT(openURLSlot(const TQString&)));
  connect(maintainer, TQ_SIGNAL(sendEmail(const TQString&, const TQString&)),
	  TQ_SLOT(sendEmailSlot(const TQString&, const TQString&)));
  connect(maintainer, TQ_SIGNAL(openURL(const TQString&)),
	  TQ_SLOT(openURLSlot(const TQString&)));
  // #################################################################
}


void
TDEAboutWidget::adjust()
{
  // #################################################################
  int cx, cy, tempx;
  int maintWidth, maintHeight;
  TQSize total_size;
  // -----
  if(showMaintainer)
    {
      total_size=maintainer->sizeHint();
      maintWidth=total_size.width();
      maintHeight=total_size.height();
    } else {
      maintWidth=0;
      maintHeight=0;
    }
  total_size=author->sizeHint();
  logo->adjustSize();
  cy=version->sizeHint().height()+Grid;
  cx=logo->width();
  tempx=TQMAX(total_size.width(), maintWidth);
  cx+=Grid+tempx;
  cx=TQMAX(cx, version->sizeHint().width());
  cy+=TQMAX(logo->height(),
	   total_size.height()+(showMaintainer ? Grid+maintHeight : 0));
  // -----
  if(!contributors.isEmpty())
    {
      cx=TQMAX(cx, cont->sizeHint().width());
      cy+=cont->sizeHint().height()+Grid;
      TQPtrListIterator<TDEAboutContributor> _pos(contributors);
      TDEAboutContributor* currEntry;
      while ( (currEntry = _pos.current()) )
	{
	  ++_pos;
	  cy+=currEntry->sizeHint().height();
	}
    }
  // -----
  setMinimumSize(cx, cy);
  // #################################################################
}

void
TDEAboutWidget::setLogo(const TQPixmap& i)
{
  // ############################################################################
  logo->setPixmap(i);
  // ############################################################################
}

void TDEAboutWidget::sendEmailSlot(const TQString &_name, const TQString &_email)
{
  emit(sendEmail(_name, _email));
}

void TDEAboutWidget::openURLSlot(const TQString& _url)
{
  emit(openURL(_url));
}

void
TDEAboutWidget::setAuthor(const TQString &_name, const TQString &_email,
			const TQString &_url, const TQString &_w)
{
  // ############################################################################
  author->setName(_name);
  author->setEmail(_email);
  author->setURL(_url);
  author->setWork(_w);
  // ############################################################################
}

void
TDEAboutWidget::setMaintainer(const TQString &_name, const TQString &_email,
			    const TQString &_url, const TQString &_w)
{
  // ############################################################################
  maintainer->setName(_name);
  maintainer->setEmail(_email);
  maintainer->setWork(_w);
  maintainer->setURL(_url);
  showMaintainer=true;
  // ############################################################################
}

void
TDEAboutWidget::addContributor(const TQString &_name, const TQString &_email,
			     const TQString &_url, const TQString &_w)
{
  // ############################################################################
  TDEAboutContributor* const c=new TDEAboutContributor(this);
  // -----
  c->setName(_name);
  c->setEmail(_email);
  c->setURL(_url);
  c->setWork(_w);
  contributors.append(c);
  connect(c, TQ_SIGNAL(sendEmail(const TQString&, const TQString&)),
	  TQ_SLOT(sendEmailSlot(const TQString&, const TQString&)));
  connect(c, TQ_SIGNAL(openURL(const TQString&)), TQ_SLOT(openURLSlot(const TQString&)));
  // ############################################################################
}

void
TDEAboutWidget::setVersion(const TQString &_name)
{
  // ############################################################################
  version->setText(_name);
  // ############################################################################
}

void
TDEAboutWidget::resizeEvent(TQResizeEvent*)
{
  // ############################################################################
  int _x=0, _y, cx, tempx, tempy;
  // ----- set version label geometry:
  version->setGeometry(0, 0, width(), version->sizeHint().height());
  _y=version->height()+Grid;
  // ----- move logo to correct position:
  logo->adjustSize();
  logo->move(0, _y);
  // ----- move author and maintainer right to it:
  tempx=logo->width()+Grid;
  cx=width()-tempx;
  author->setGeometry
    (tempx, _y, cx, author->sizeHint().height());
  maintainer->setGeometry
    (tempx, _y+author->height()+Grid, cx, maintainer->sizeHint().height());

  _y+=TQMAX(logo->height(),
	  author->height()+(showMaintainer ? Grid+maintainer->height() : 0));
  // -----
  if(!contributors.isEmpty())
    {
      tempy=cont->sizeHint().height();
      cont->setGeometry(0, _y, width(), tempy);
      cont->show();
      _y+=tempy+Grid;
    } else {
      cont->hide();
    }
  TQPtrListIterator<TDEAboutContributor> _pos(contributors);
  TDEAboutContributor* currEntry;
  while( (currEntry = _pos.current()) )
    {
      ++_pos;
      tempy=currEntry->sizeHint().height();
      // y+=Grid;
      currEntry->setGeometry(_x, _y, width(), tempy);
      _y+=tempy;
    }
  if(showMaintainer)
    {
      maintainer->show();
    } else {
      maintainer->hide();
    }
  // ############################################################################
}

TDEAboutDialog::TDEAboutDialog(TQWidget *_parent, const char *_name, bool modal)
  : KDialogBase(_parent, _name, modal, TQString::null, Ok, Ok ),
    about(new TDEAboutWidget(this)), mContainerBase(0), d(0)
{
  // #################################################################
  if(!about)
  {
    // this will nearly never happen (out of memory in about box?)
    kdDebug() << "TDEAboutDialog::TDEAboutDialog: Out of memory." << endl;
    tqApp->quit();
  }
  setMainWidget(about);
  connect(about, TQ_SIGNAL(sendEmail(const TQString&, const TQString&)),
	  TQ_SLOT(sendEmailSlot(const TQString&, const TQString&)));
  connect(about, TQ_SIGNAL(openURL(const TQString&)),
	  TQ_SLOT(openURLSlot(const TQString&)));
  // #################################################################
}


TDEAboutDialog::TDEAboutDialog( int layoutType, const TQString &_caption,
			    int buttonMask, ButtonCode defaultButton,
			    TQWidget *_parent, const char *_name, bool modal,
			    bool separator, const TQString &user1,
			    const TQString &user2, const TQString &user3 )
  :KDialogBase( _parent, _name, modal, TQString::null, buttonMask, defaultButton,
		separator, user1, user2, user3 ),
   about(0), d(0)
{
  setPlainCaption( i18n("About %1").arg(_caption) );

  mContainerBase = new TDEAboutContainerBase( layoutType, this );
  setMainWidget(mContainerBase);

  connect( mContainerBase, TQ_SIGNAL(urlClick(const TQString &)),
	   this, TQ_SLOT(openURLSlot(const TQString &)));
  connect( mContainerBase, TQ_SIGNAL(mailClick(const TQString &,const TQString &)),
	   this, TQ_SLOT(sendEmailSlot(const TQString &,const TQString &)));
  connect( mContainerBase, TQ_SIGNAL(mouseTrack(int, const TQMouseEvent *)),
	   this, TQ_SLOT(mouseTrackSlot(int, const TQMouseEvent *)));
}


void TDEAboutDialog::show( void )
{
  adjust();
  if( mContainerBase ) { mContainerBase->show(); }
  TQDialog::show();
}


void TDEAboutDialog::show( TQWidget * /*centerParent*/ )
{
  adjust();
  if( mContainerBase ) { mContainerBase->show(); }
  TQDialog::show();
}


void TDEAboutDialog::adjust()
{
  if( !about ) { return; }
  about->adjust();
  //initializeGeometry();
  resize( sizeHint() );
}


void TDEAboutDialog::setLogo(const TQPixmap& i)
{
  if( !about ) { return; }
  about->setLogo(i);
}


void TDEAboutDialog::setMaintainer(const TQString &_name, const TQString &_email,
				 const TQString &_url, const TQString &_w)
{
  // #################################################################
  if( !about ) { return; }
  about->setMaintainer(_name, _email, _url, _w);
  // #################################################################
}

void TDEAboutDialog::setAuthor(const TQString &_name, const TQString &_email,
			     const TQString &_url, const TQString &_work)
{
  // #################################################################
  if( !about ) { return; }
  about->setAuthor(_name, _email, _url, _work);
  // #################################################################
}

void TDEAboutDialog::addContributor(const TQString &_name, const TQString &_email,
				  const TQString &_url, const TQString &_w)
{
  // #################################################################
  if( !about ) { return; }
  about->addContributor(_name, _email, _url, _w);
  // #################################################################
}

void TDEAboutDialog::setVersion(const TQString &_name)
{
  // #################################################################
  if( !about ) { return; }
  about->setVersion(_name);
  // #################################################################
}

void TDEAboutDialog::sendEmailSlot(const TQString& /*name*/, const TQString& email)
{
  if ( kapp )
      kapp->invokeMailer( email, TQString::null );
  /*
  kdDebug() << "TDEAboutDialog::sendEmailSlot: request to send an email to "
	<< name << ", " << email << endl;
  emit(sendEmail(name, email));
  */
}

void TDEAboutDialog::openURLSlot(const TQString& url)
{
  if ( kapp )
      kapp->invokeBrowser( url );
  //kdDebug() << "TDEAboutDialog::openURLSlot: request to open URL " << url << endl;
  //emit(openURL(url));
}


void TDEAboutDialog::mouseTrackSlot( int /*mode*/, const TQMouseEvent * /*e*/ )
{
  // By default we do nothing. This method must be reimplemented.
}


TQFrame *TDEAboutDialog::addTextPage( const TQString &title, const TQString &text,
				   bool richText, int numLines )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addTextPage( title, text, richText, numLines );
}

TQFrame *TDEAboutDialog::addLicensePage( const TQString &title, const TQString &text,
				   int numLines )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addLicensePage( title, text, numLines );
}


TDEAboutContainer *TDEAboutDialog::addContainerPage( const TQString &title,
				  int childAlignment, int innerAlignment )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addContainerPage( title, childAlignment,
					    innerAlignment);
}


TDEAboutContainer *TDEAboutDialog::addScrolledContainerPage( const TQString &title,
				  int childAlignment, int innerAlignment )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addScrolledContainerPage( title, childAlignment,
						    innerAlignment);
}



TQFrame *TDEAboutDialog::addPage( const TQString &title )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addEmptyPage( title );
}


TDEAboutContainer *TDEAboutDialog::addContainer( int childAlignment,
					     int innerAlignment )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addContainer( childAlignment, innerAlignment );
}


void TDEAboutDialog::setTitle( const TQString &title )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setTitle( title );
}


void TDEAboutDialog::setImage( const TQString &fileName )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setImage( fileName );
}

// KDE4: remove
void TDEAboutDialog::setIcon( const TQString &fileName )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setProgramLogo( fileName );
}

void TDEAboutDialog::setProgramLogo( const TQString &fileName )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setProgramLogo( fileName );
}

void TDEAboutDialog::setProgramLogo( const TQPixmap &pixmap )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setProgramLogo( pixmap );
}

void TDEAboutDialog::setImageBackgroundColor( const TQColor &color )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setImageBackgroundColor( color );
}


void TDEAboutDialog::setImageFrame( bool state )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setImageFrame( state );
}


void TDEAboutDialog::setProduct( const TQString &appName, const TQString &version,
			       const TQString &author, const TQString &year )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setProduct( appName, version, author, year );
}



void TDEAboutDialog::imageURL( TQWidget *_parent, const TQString &_caption,
			     const TQString &_path, const TQColor &_imageColor,
			     const TQString &_url )
{
  TDEAboutDialog a( AbtImageOnly, TQString::null, Close, Close, _parent, "image", true );
  a.setPlainCaption( _caption );
  a.setImage( _path );
  a.setImageBackgroundColor( _imageColor );

  TDEAboutContainer* const c = a.addContainer( AlignCenter, AlignCenter );
  if( c )
  {
    c->addPerson( TQString::null, TQString::null, _url, TQString::null );
  }
  a.exec();
}




//
// A class that can can monitor mouse movements on the image
//
KImageTrackLabel::KImageTrackLabel( TQWidget *_parent, const char *_name, WFlags f )
  : TQLabel( _parent, _name, f )
{
  setText( i18n("Image missing"));
}

void KImageTrackLabel::mousePressEvent( TQMouseEvent *e )
{
  emit mouseTrack( MousePress, e );
}

void KImageTrackLabel::mouseReleaseEvent( TQMouseEvent *e )
{
  emit mouseTrack( MouseRelease, e );
}

void KImageTrackLabel::mouseDoubleClickEvent( TQMouseEvent *e )
{
  emit mouseTrack( MouseDoubleClick, e );
}

void KImageTrackLabel::mouseMoveEvent ( TQMouseEvent *e )
{
  emit mouseTrack( MouseDoubleClick, e );
}

void TDEAboutDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

