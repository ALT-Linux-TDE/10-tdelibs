/* This file is part of the KDE libraries
    Copyright (C) 1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <sys/stat.h>
#include <unistd.h>

#include <tqstring.h>
#include <tqtooltip.h>
#include <tqapplication.h>

#include <tdeaccel.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdirselectdialog.h>
#include <tdefiledialog.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <kurlcompletion.h>
#include <kurldrag.h>
#include <kprotocolinfo.h>

#include "kurlrequester.h"


class KURLDragPushButton : public KPushButton
{
public:
    KURLDragPushButton( TQWidget *parent, const char *name=0 )
	: KPushButton( parent, name ) {
    	setDragEnabled( true );
    }
    ~KURLDragPushButton() {}

    void setURL( const KURL& url ) {
	m_urls.clear();
	m_urls.append( url );
    }

    /* not needed so far
    void setURLs( const KURL::List& urls ) {
	m_urls = urls;
    }
    const KURL::List& urls() const { return m_urls; }
    */

protected:
    virtual TQDragObject *dragObject() {
	if ( m_urls.isEmpty() )
	    return 0L;

	TQDragObject *drag = new KURLDrag( m_urls, this, "url drag" );
	return drag;
    }

private:
    KURL::List m_urls;

};


/*
*************************************************************************
*/

class KURLRequester::KURLRequesterPrivate
{
public:
    KURLRequesterPrivate() {
	edit = 0L;
	combo = 0L;
        fileDialogMode = KFile::File | KFile::ExistingOnly | KFile::LocalOnly;
    }

    void setText( const TQString& text ) {
	if ( combo )
	{
	    if (combo->editable())
	    {
               combo->setEditText( text );
            }
            else
            {
               combo->insertItem( text );
               combo->setCurrentItem( combo->count()-1 );
            }
        }
	else
	{
	    edit->setText( text );
	}
    }

    void connectSignals( TQObject *receiver ) {
	TQObject *sender;
	if ( combo )
	    sender = combo;
	else
	    sender = edit;

	connect( sender, TQ_SIGNAL( textChanged( const TQString& )),
		 receiver, TQ_SIGNAL( textChanged( const TQString& )));
	connect( sender, TQ_SIGNAL( returnPressed() ),
		 receiver, TQ_SIGNAL( returnPressed() ));
	connect( sender, TQ_SIGNAL( returnPressed( const TQString& ) ),
		 receiver, TQ_SIGNAL( returnPressed( const TQString& ) ));
    }

    void setCompletionObject( TDECompletion *comp ) {
	if ( combo )
	    combo->setCompletionObject( comp );
	else
	    edit->setCompletionObject( comp );
    }

    /**
     * replaces ~user or $FOO, if necessary
     */
    TQString url() {
        TQString txt = combo ? combo->currentText() : edit->text();
        KURLCompletion *comp;
        if ( combo )
            comp = dynamic_cast<KURLCompletion*>(combo->completionObject());
        else
            comp = dynamic_cast<KURLCompletion*>(edit->completionObject());

        if ( comp )
            return comp->replacedPath( txt );
        else
            return txt;
    }

    KLineEdit *edit;
    KComboBox *combo;
    int fileDialogMode;
    TQString fileDialogFilter;
};



KURLRequester::KURLRequester( TQWidget *editWidget, TQWidget *parent,
			      const char *name )
  : TQHBox( parent, name )
{
    d = new KURLRequesterPrivate;

    // must have this as parent
    editWidget->reparent( this, 0, TQPoint(0,0) );
    d->edit = dynamic_cast<KLineEdit*>( editWidget );
    d->combo = dynamic_cast<KComboBox*>( editWidget );

    init();
}


KURLRequester::KURLRequester( TQWidget *parent, const char *name )
  : TQHBox( parent, name )
{
    d = new KURLRequesterPrivate;
    init();
}


KURLRequester::KURLRequester( const TQString& url, TQWidget *parent,
			      const char *name )
  : TQHBox( parent, name )
{
    d = new KURLRequesterPrivate;
    init();
    setKURL( KURL::fromPathOrURL( url ) );
}


KURLRequester::~KURLRequester()
{
    delete myCompletion;
    delete myFileDialog;
    delete d;
}


void KURLRequester::init()
{
    myFileDialog    = 0L;
    myShowLocalProt = false;

    if ( !d->combo && !d->edit )
	d->edit = new KLineEdit( this, "line edit" );

    myButton = new KURLDragPushButton( this, "tdefile button");
    TQIconSet iconSet = SmallIconSet(TQString::fromLatin1("document-open"));
    TQPixmap pixMap = iconSet.pixmap( TQIconSet::Small, TQIconSet::Normal );
    myButton->setIconSet( iconSet );
    myButton->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
    TQToolTip::add(myButton, i18n("Open file dialog"));

    connect( myButton, TQ_SIGNAL( pressed() ), TQ_SLOT( slotUpdateURL() ));

    setSpacing( KDialog::spacingHint() );

    TQWidget *widget = d->combo ? (TQWidget*) d->combo : (TQWidget*) d->edit;
    widget->installEventFilter( this );
    setFocusProxy( widget );

    d->connectSignals( this );
    connect( myButton, TQ_SIGNAL( clicked() ), this, TQ_SLOT( slotOpenDialog() ));

    myCompletion = new KURLCompletion();
    d->setCompletionObject( myCompletion );

    TDEAccel *accel = new TDEAccel( this );
    accel->insert( TDEStdAccel::Open, this, TQ_SLOT( slotOpenDialog() ));
    accel->readSettings();
}


void KURLRequester::setURL( const TQString& url )
{
    if ( myShowLocalProt )
    {
        d->setText( url );
    }
    else
    {
        // ### This code is broken (e.g. for paths with '#')
        if ( url.startsWith("file://") )
            d->setText( url.mid( 7 ) );
        else if ( url.startsWith("file:") )
            d->setText( url.mid( 5 ) );
        else
            d->setText( url );
    }
}

void KURLRequester::setKURL( const KURL& url )
{
    if ( myShowLocalProt )
        d->setText( url.url() );
    else
        d->setText( url.pathOrURL() );
}

void KURLRequester::setCaption( const TQString& caption )
{
   TQWidget::setCaption( caption );
   if (myFileDialog)
      myFileDialog->setCaption( caption );
}

TQString KURLRequester::url() const
{
    return d->url();
}

void KURLRequester::slotOpenDialog()
{
    KURL newurl;
    if ( (d->fileDialogMode & KFile::Directory) && !(d->fileDialogMode & KFile::File) ||
         /* catch possible fileDialog()->setMode( KFile::Directory ) changes */
         (myFileDialog && ( (myFileDialog->mode() & KFile::Directory) &&
         (myFileDialog->mode() & (KFile::File | KFile::Files)) == 0 ) ) )
    {
        newurl = KDirSelectDialog::selectDirectory(url(), d->fileDialogMode & KFile::LocalOnly);
        if ( !newurl.isValid() )
        {
            return;
        }
    }
    else
    {
      emit openFileDialog( this );

      KFileDialog *dlg = fileDialog();
      if ( !d->url().isEmpty() ) {
          KURL u( url() );
          // If we won't be able to list it (e.g. http), then don't try :)
          if ( KProtocolInfo::supportsListing( u ) )
              dlg->setSelection( u.url() );
      }

      if ( dlg->exec() != TQDialog::Accepted )
      {
          return;
      }

      newurl = dlg->selectedURL();
    }

    setKURL( newurl );
    emit urlSelected( d->url() );
}

void KURLRequester::setMode(uint mode)
{
    Q_ASSERT( (mode & KFile::Files) == 0 );
    d->fileDialogMode = mode;
    if ( (mode & KFile::Directory) && !(mode & KFile::File) )
        myCompletion->setMode( KURLCompletion::DirCompletion );

    if (myFileDialog)
       myFileDialog->setMode( d->fileDialogMode );
}

unsigned int KURLRequester::mode() const
{
    return d->fileDialogMode;
}

void KURLRequester::setFilter(const TQString &filter)
{
    d->fileDialogFilter = filter;
    if (myFileDialog)
       myFileDialog->setFilter( d->fileDialogFilter );
}

TQString KURLRequester::filter( ) const
{
    return d->fileDialogFilter;
}


KFileDialog * KURLRequester::fileDialog() const
{
    if ( !myFileDialog ) {
        TQWidget *p = parentWidget();
        myFileDialog = new KFileDialog( TQString::null, d->fileDialogFilter, p,
                                        "file dialog", true );

        myFileDialog->setMode( d->fileDialogMode );
        myFileDialog->setCaption( caption() );
    }

    return myFileDialog;
}


void KURLRequester::setShowLocalProtocol( bool b )
{
    if ( myShowLocalProt == b )
	return;

    myShowLocalProt = b;
    setKURL( url() );
}

void KURLRequester::clear()
{
    d->setText( TQString::null );
}

KLineEdit * KURLRequester::lineEdit() const
{
    return d->edit;
}

KComboBox * KURLRequester::comboBox() const
{
    return d->combo;
}

void KURLRequester::slotUpdateURL()
{
    // bin compat, myButton is declared as QPushButton
    KURL u;
    u = KURL( KURL( TQDir::currentDirPath() + '/' ), url() );
    (static_cast<KURLDragPushButton *>( myButton ))->setURL( u );
}

bool KURLRequester::eventFilter( TQObject *obj, TQEvent *ev )
{
    if ( ( d->edit == obj ) || ( d->combo == obj ) )
    {
        if (( ev->type() == TQEvent::FocusIn ) || ( ev->type() == TQEvent::FocusOut ))
            // Forward focusin/focusout events to the urlrequester; needed by file form element in tdehtml
            TQApplication::sendEvent( this, ev );
    }
    return TQWidget::eventFilter( obj, ev );
}

KPushButton * KURLRequester::button() const
{
    return myButton;
}

KEditListBox::CustomEditor KURLRequester::customEditor()
{
    setSizePolicy(TQSizePolicy( TQSizePolicy::Preferred,
                               TQSizePolicy::Fixed));

    KLineEdit *edit = d->edit;
    if ( !edit && d->combo )
        edit = dynamic_cast<KLineEdit*>( d->combo->lineEdit() );

#ifndef NDEBUG
    if ( !edit )
        kdWarning() << "KURLRequester's lineedit is not a KLineEdit!??\n";
#endif

    KEditListBox::CustomEditor editor( this, edit );
    return editor;
}

void KURLRequester::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

KURLComboRequester::KURLComboRequester( TQWidget *parent,
			      const char *name )
  : KURLRequester( new KComboBox(false), parent, name)
{
}

#include "kurlrequester.moc"
