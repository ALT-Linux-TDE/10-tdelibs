/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kactivelabel.h"

#include <tdeapplication.h>
#include <tqregexp.h>
#include <tqwhatsthis.h>
#include <tqsimplerichtext.h>
#include <kdebug.h>

KActiveLabel::KActiveLabel(TQWidget * parent, const char * name)
 : TQTextBrowser(parent, name)
{
   init();
}

KActiveLabel::KActiveLabel(const TQString &text, TQWidget * parent, const char * name)
 : TQTextBrowser(parent, name)
{
   init();
   setText(text);
}

void KActiveLabel::init()
{
   setTextFormat(TQt::RichText);
   setVScrollBarMode(TQScrollView::AlwaysOff);
   setHScrollBarMode(TQScrollView::AlwaysOff);
   setFrameStyle(TQFrame::NoFrame);
   setFocusPolicy( TQWidget::TabFocus );
   paletteChanged();

   connect(this, TQ_SIGNAL(linkClicked(const TQString &)),
           this, TQ_SLOT(openLink(const TQString &)));
   if (kapp)
   {
      connect(kapp, TQ_SIGNAL(tdedisplayPaletteChanged()),
              this, TQ_SLOT(paletteChanged()));
   }
}

void KActiveLabel::paletteChanged()
{
   TQPalette p = kapp ? kapp->palette() : palette();
   p.setBrush(TQColorGroup::Base, p.brush(TQPalette::Normal, TQColorGroup::Background));
   p.setColor(TQColorGroup::Text, p.color(TQPalette::Normal, TQColorGroup::Foreground));
   setPalette(p);
}

void KActiveLabel::openLink(const TQString & link)
{
   TQRegExp whatsthis("whatsthis:/*([^/].*)");
   if (whatsthis.exactMatch(link)) {
      TQWhatsThis::display(whatsthis.cap(1));
      return;
   }

   TQStringList args;
   args << "exec" << link;
   kapp->tdeinitExec("kfmclient", args);
}

void KActiveLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KActiveLabel::focusInEvent( TQFocusEvent* fe )
{
   TQTextBrowser::focusInEvent(fe);
   if(fe->reason() == TQFocusEvent::Tab || fe->reason() == TQFocusEvent::Backtab)
      selectAll(true);
}

void KActiveLabel::focusOutEvent( TQFocusEvent* fe )
{
   TQTextBrowser::focusOutEvent(fe);
   if(fe->reason() == TQFocusEvent::Tab || fe->reason() == TQFocusEvent::Backtab)
      selectAll(false);
}

void KActiveLabel::keyPressEvent( TQKeyEvent *e )
{
    switch ( e->key() )
    {
    case Key_Down:
    case Key_Up:
    case Key_Left:
    case Key_Right:
        // jump over QTextEdit's key navigation breakage.
        // we're not interested in keyboard navigation within the text
        TQWidget::keyPressEvent( e );
        break;
    default:
        TQTextBrowser::keyPressEvent( e );
    }
}

TQSize KActiveLabel::minimumSizeHint() const
{
   TQSize ms = minimumSize();
   if ((ms.width() > 0) && (ms.height() > 0))
      return ms;

   int w = 400;
   if (ms.width() > 0)
      w = ms.width();

   TQString txt = text();
   TQSimpleRichText rt(txt, font());
   rt.setWidth(w - 2*frameWidth() - 10);
   w = 10 + rt.widthUsed() + 2*frameWidth();
   if (w < ms.width())
      w = ms.width();
   int h = rt.height() + 2*frameWidth();
   if ( h < ms.height())
      h = ms.height();

   return TQSize(w, h);
}

TQSize KActiveLabel::sizeHint() const
{
   return minimumSizeHint();
}

#include "kactivelabel.moc"
