/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
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

#include <tdeconfig.h>
#include <tdeapplication.h>
#include <kdialog.h>
#include <kwhatsthismanager_p.h>
#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kiconloader.h>
#include <tdeglobalsettings.h>
#include <tdelocale.h>

#include <tqlayout.h>
#include <tqobjectlist.h>
#include <tqguardedptr.h>
#include <tqlineedit.h>
#include <tqvaluelist.h>
#include <tqtimer.h>
#include <tqcursor.h>
#include <tqlabel.h>
#include <tqstyle.h>
#include <tqimage.h>
#include <tqpushbutton.h>

#include "config.h"
#ifdef TQ_WS_X11
#include <netwm.h>
#endif

const int KDialog::mMarginSize = 11;
const int KDialog::mSpacingSize = 6;

template class TQPtrList<TQLayoutItem>;

KDialog::KDialog(TQWidget *parent, const char *name, bool modal, WFlags f)
  : TQDialog(parent, name, modal, f), d(0)
{
    KWhatsThisManager::init ();
}

//
// Grab QDialogs keypresses if non-modal.
//
void KDialog::keyPressEvent(TQKeyEvent *e)
{
  if ( e->state() == 0 )
  {
    switch ( e->key() )
    {
      case Key_Escape:
      case Key_Enter:
      case Key_Return:
      {
        if(testWFlags((WFlags)(WType_Dialog | WShowModal)))
	{
          TQDialog::keyPressEvent(e);
	}
        else
        {
   	  e->ignore();
        }
      }
      break;
      default:
	e->ignore();
	return;
    }
  }
  else
  {
      // accept the dialog when Ctrl-Return is pressed
      if ( e->state() == ControlButton &&
           (e->key() == Key_Return || e->key() == Key_Enter) )
      {
          e->accept();
          accept();
      }
      else
      {
          e->ignore();
      }
  }
}


int KDialog::marginHint()
{
  return mMarginSize;
}


int KDialog::spacingHint()
{
  return mSpacingSize;
}

// KDE4: Remove me
void KDialog::polish()
{
  TQDialog::polish();
}


void KDialog::setCaption( const TQString &_caption )
{
  TQString caption = kapp ? kapp->makeStdCaption( _caption ) : _caption;
  setPlainCaption( caption );
}


void KDialog::setPlainCaption( const TQString &caption )
{
  TQDialog::setCaption( caption );

#ifdef TQ_WS_X11
  NETWinInfo info( tqt_xdisplay(), winId(), tqt_xrootwin(), 0 );
  info.setName( caption.utf8().data() );
#endif
}


void KDialog::resizeLayout( TQWidget *w, int margin, int spacing )
{
  if( w->layout() )
  {
    resizeLayout( w->layout(), margin, spacing );
  }

  if( !w->childrenListObject().isEmpty() )
  {
    const TQObjectList l = w->childrenListObject();
    TQObjectListIterator itr(l);
    TQObject *o;
    while ((o = itr.current()) != 0) {
      if( o->isWidgetType() )
      {
	resizeLayout( (TQWidget*)o, margin, spacing );
      }
      ++itr;
    }
  }
}


void KDialog::resizeLayout( TQLayoutItem *lay, int margin, int spacing )
{
  TQLayoutIterator it = lay->iterator();
  TQLayoutItem *child;
  while ( (child = it.current() ) )
  {
    resizeLayout( child, margin, spacing );
    ++it;
  }
  if( lay->layout() )
  {
    lay->layout()->setMargin( margin );
    lay->layout()->setSpacing( spacing );
  }
}

static TQRect screenRect( TQWidget *w, int screen )
{
  TQDesktopWidget *desktop = TQApplication::desktop();
  TDEConfig gc("kdeglobals", false, false);
  gc.setGroup("Windows");
  if (desktop->isVirtualDesktop() &&
      gc.readBoolEntry("XineramaEnabled", true) &&
      gc.readBoolEntry("XineramaPlacementEnabled", true)) {
    if ( screen < 0 || screen >= desktop->numScreens() ) {
      if ( screen == -1 ) {
        screen = desktop->primaryScreen();
      } else if ( screen == -3 ) {
        screen = desktop->screenNumber( TQCursor::pos() );
      } else {
        screen = desktop->screenNumber( w );
      }
    }
    return desktop->availableGeometry(screen);
  } else {
    return desktop->geometry();
  }
}

void KDialog::centerOnScreen( TQWidget *w, int screen )
{
  if ( !w )
    return;
  TQRect r = screenRect( w, screen );

  w->move( r.center().x() - w->width()/2,
           r.center().y() - w->height()/2 );
}

bool KDialog::avoidArea( TQWidget *w, const TQRect& area, int screen )
{
  if ( !w )
    return false;
  TQRect fg = w->frameGeometry();
  if ( !fg.intersects( area ) )
    return true; // nothing to do.

  TQRect scr = screenRect( w, screen );
  TQRect avoid( area ); // let's add some margin
  avoid.moveBy( -5, -5 );
  avoid.rRight() += 10;
  avoid.rBottom() += 10;

  if ( TQMAX( fg.top(), avoid.top() ) <= TQMIN( fg.bottom(), avoid.bottom() ) )
  {
    // We need to move the widget up or down
    int spaceAbove = TQMAX(0, avoid.top() - scr.top());
    int spaceBelow = TQMAX(0, scr.bottom() - avoid.bottom());
    if ( spaceAbove > spaceBelow ) // where's the biggest side?
      if ( fg.height() <= spaceAbove ) // big enough?
        fg.setY( avoid.top() - fg.height() );
      else
        return false;
    else
      if ( fg.height() <= spaceBelow ) // big enough?
        fg.setY( avoid.bottom() );
      else
        return false;
  }

  if ( TQMAX( fg.left(), avoid.left() ) <= TQMIN( fg.right(), avoid.right() ) )
  {
    // We need to move the widget left or right
    int spaceLeft = TQMAX(0, avoid.left() - scr.left());
    int spaceRight = TQMAX(0, scr.right() - avoid.right());
    if ( spaceLeft > spaceRight ) // where's the biggest side?
      if ( fg.width() <= spaceLeft ) // big enough?
        fg.setX( avoid.left() - fg.width() );
      else
        return false;
    else
      if ( fg.width() <= spaceRight ) // big enough?
        fg.setX( avoid.right() );
      else
        return false;
  }
  //kdDebug() << "Moving window to " << fg.x() << "," << fg.y() << endl;
  w->move(fg.x(), fg.y());
  return true;
}

class KDialogQueuePrivate
{
public:
  TQValueList< TQGuardedPtr<TQDialog> > queue;
  bool busy;
};

static KStaticDeleter<KDialogQueue> ksdkdq;

KDialogQueue *KDialogQueue::_self=0;

KDialogQueue* KDialogQueue::self()
{
   if (!_self)
      _self = ksdkdq.setObject(_self, new KDialogQueue);
   return _self;
}

KDialogQueue::KDialogQueue() : d(new KDialogQueuePrivate)
{
   d->busy = false;
}

KDialogQueue::~KDialogQueue()
{
   delete d;
   _self = 0;
}

// static
void KDialogQueue::queueDialog(TQDialog *dialog)
{
   KDialogQueue *_this = self();
   _this->d->queue.append(dialog);
   TQTimer::singleShot(0, _this, TQ_SLOT(slotShowQueuedDialog()));
}

void KDialogQueue::slotShowQueuedDialog()
{
   if (d->busy)
      return;
   TQDialog *dialog;
   do {
       if(d->queue.isEmpty())
         return;
      dialog = d->queue.first();
      d->queue.pop_front();
   }
   while(!dialog);

   d->busy = true;
   dialog->exec();
   d->busy = false;
   delete dialog;

   if (!d->queue.isEmpty())
      TQTimer::singleShot(20, this, TQ_SLOT(slotShowQueuedDialog()));
   else
      ksdkdq.destructObject(); // Suicide.
}

void KDialog::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

KSMModalDialogHeader::KSMModalDialogHeader(TQWidget* parent)
  : TQWidget( parent, "", TQt::WDestructiveClose )
{
	TQVBoxLayout* vbox = new TQVBoxLayout( this );

	TQFrame* frame = new TQFrame( this );
	frame->setFrameStyle( TQFrame::NoFrame );
	frame->setLineWidth( 0 );
	// we need to set the minimum size for the window
	frame->setMinimumWidth(400);
	vbox->addWidget( frame );
	TQGridLayout* gbox = new TQGridLayout( frame, 1, 1, 0, KDialog::spacingHint() );
	TQHBoxLayout* centerbox = new TQHBoxLayout( KDialog::spacingHint() );
	TQHBoxLayout* seperatorbox = new TQHBoxLayout( 0 );
	centerbox->setMargin(0);
	seperatorbox->setMargin(0);

	TQWidget* ticon = new TQWidget( frame );
	TDEIconLoader * ldr = TDEGlobal::iconLoader();
	TQPixmap trinityPixmap = ldr->loadIcon("kmenu", TDEIcon::Panel, TDEIcon::SizeLarge, TDEIcon::DefaultState, 0L, true);

	// Manually draw the alpha portions of the icon onto the widget background color...
	TQRgb backgroundRgb = ticon->paletteBackgroundColor().rgb();
	TQImage correctedImage = trinityPixmap.convertToImage();
	correctedImage = correctedImage.convertDepth(32);
	correctedImage.setAlphaBuffer(true);
	int w = correctedImage.width();
	int h = correctedImage.height();
	for (int y = 0; y < h; ++y) {
		TQRgb *ls = (TQRgb *)correctedImage.scanLine( y );
		for (int x = 0; x < w; ++x) {
			TQRgb l = ls[x];
			float alpha_adjust = tqAlpha( l )/255.0;
			int r = int( (tqRed( l ) * alpha_adjust) + (tqRed( backgroundRgb ) * (1.0-alpha_adjust)) );
			int g = int( (tqGreen( l ) * alpha_adjust) + (tqGreen( backgroundRgb ) * (1.0-alpha_adjust)) );
			int b = int( (tqBlue( l ) * alpha_adjust) + (tqBlue( backgroundRgb ) * (1.0-alpha_adjust)) );
			int a = int( 255 );
			ls[x] = tqRgba( r, g, b, a );
		}
	}
	trinityPixmap.convertFromImage(correctedImage);

	ticon->setBackgroundPixmap(trinityPixmap);
	ticon->setMinimumSize(trinityPixmap.size());
	ticon->setMaximumSize(trinityPixmap.size());
	ticon->resize(trinityPixmap.size());
	centerbox->addWidget( ticon, AlignCenter );

	TQWidget* swidget = new TQWidget( frame );
	swidget->resize(2, frame->sizeHint().width());
	swidget->setBackgroundColor(TQt::black);
	seperatorbox->addWidget( swidget, AlignCenter );

	TQLabel* label = new TQLabel( i18n("Trinity Desktop Environment"), frame );
	TQFont fnt = label->font();
	fnt.setBold( true );
	fnt.setPointSize( fnt.pointSize() * 3 / 2 );
	label->setFont( fnt );
	centerbox->addWidget( label, AlignCenter );

	gbox->addLayout(centerbox, 0, 0);
	gbox->addLayout(seperatorbox, 1, 0);

	setFixedSize( sizeHint() );
}

KSMModalDialogHeader::~KSMModalDialogHeader()
{
}

KSMModalDialog::KSMModalDialog(TQWidget* parent)
  : TQWidget( 0, "systemmodaldialogclass", TQt::WStyle_Customize | TQt::WType_Dialog | TQt::WStyle_Title | TQt::WStyle_StaysOnTop | TQt::WDestructiveClose ), m_keepOnTopTimer(NULL), m_allowClose(false)

{
	// Signal that we do not want any window controls to be shown at all
	Atom kde_wm_system_modal_notification;
	kde_wm_system_modal_notification = XInternAtom(tqt_xdisplay(), "_TDE_WM_MODAL_SYS_NOTIFICATION", False);
	XChangeProperty(tqt_xdisplay(), winId(), kde_wm_system_modal_notification, XA_INTEGER, 32, PropModeReplace, (unsigned char *) "TRUE", 1L);

	TQVBoxLayout* vbox = new TQVBoxLayout( this );

	TQFrame* frame = new TQFrame( this );
	frame->setFrameStyle( TQFrame::NoFrame );
	frame->setLineWidth( style().pixelMetric( TQStyle::PM_DefaultFrameWidth, frame ) );
	// we need to set the minimum size for the window
	frame->setMinimumWidth(400);
	vbox->addWidget( frame );
	m_gridlayout = new TQGridLayout( frame, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );
	TQHBoxLayout* centerbox = new TQHBoxLayout( KDialog::spacingHint() );

	m_statusLabel = new TQLabel( i18n("Pondering what to do next").append("..."), frame );
	TQFont fnt = m_statusLabel->font();
	fnt.setBold( false );
	fnt.setPointSize( fnt.pointSize() * 1 );
	m_statusLabel->setFont( fnt );
	m_gridlayout->addMultiCellWidget( m_statusLabel, 2, 2, 0, 2, AlignLeft | AlignVCenter );

	KSMModalDialogHeader *theader = new KSMModalDialogHeader(this);
	centerbox->addWidget( theader, AlignCenter );

	m_gridlayout->addLayout(centerbox, 0, 0);

	m_buttonframe = new TQFrame( this );
	m_buttonframe->setFrameStyle( TQFrame::NoFrame );
	m_buttonframe->setLineWidth( style().pixelMetric( TQStyle::PM_DefaultFrameWidth, frame ) );
	m_buttonbox = new TQHBoxLayout( m_buttonframe, 0, KDialog::spacingHint() );

	m_button1 = new TQPushButton( m_buttonframe );
	m_button2 = new TQPushButton( m_buttonframe );
	m_button3 = new TQPushButton( m_buttonframe );
	m_buttonbox->addWidget( m_button1 );
	m_buttonbox->addWidget( m_button2 );
	m_buttonbox->addWidget( m_button3 );
	m_button1->hide();
	m_button2->hide();
	m_button3->hide();

	m_buttonframe->hide();
	m_gridlayout->addMultiCellWidget( m_buttonframe, 3, 3, 0, 2, 0 );

	setFixedSize( sizeHint() );
	setCaption( i18n("Please wait...") );

	// Center the dialog
	TQSize sh = sizeHint();
	TQRect rect = TDEGlobalSettings::desktopGeometry(TQCursor::pos());
	move(rect.x() + (rect.width() - sh.width())/2, rect.y() + (rect.height() - sh.height())/2);

	show();
	keepMeOnTop();
}

void KSMModalDialog::keepMeOnTop()
{
	if (!m_keepOnTopTimer) {
		m_keepOnTopTimer = new TQTimer();
		connect(m_keepOnTopTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(keepMeOnTop()));
		m_keepOnTopTimer->start(100, FALSE);
	}
	setActiveWindow();
	raise();
	setFocus();
}

KSMModalDialog::~KSMModalDialog()
{
	m_keepOnTopTimer->stop();
	delete m_keepOnTopTimer;
}

void KSMModalDialog::setStatusMessage(TQString message)
{
    if (message == "") {
        m_statusLabel->setText(i18n("Pondering what to do next").append("..."));
    }
    else {
        m_statusLabel->setText(message);
    }
}

void KSMModalDialog::closeSMDialog()
{
	m_allowClose = true;
	close();
}

void KSMModalDialog::closeEvent(TQCloseEvent *e)
{
	//---------------------------------------------
	// Don't call the base function because
	// we want to ignore the close event
	//---------------------------------------------

	if (m_allowClose)
		TQWidget::closeEvent(e);
}

void KSMModalDialog::setStartupPhase(TQString msg)
{
	if (msg == TQString("dcop")) setStatusMessage(i18n("Starting DCOP").append("..."));
	if (msg == TQString("kded")) setStatusMessage(i18n("Starting TDE daemon").append("..."));
	if (msg == TQString("kcminit")) setStatusMessage(i18n("Starting services").append("..."));
	if (msg == TQString("ksmserver")) setStatusMessage(i18n("Starting session").append("..."));
	if (msg == TQString("wm started")) setStatusMessage(i18n("Initializing window manager").append("..."));
	if (msg == TQString("kdesktop")) setStatusMessage(i18n("Loading desktop").append("..."));
	if (msg == TQString("kicker")) setStatusMessage(i18n("Loading panels").append("..."));
	if (msg == TQString("session ready")) setStatusMessage(i18n("Restoring applications").append("..."));
}

#include "kdialog.moc"
