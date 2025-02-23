/* This file is part of the KDE libraries
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

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

#include "kdualcolorbutton.h"
#include "kcolordialog.h"
#include "kcolordrag.h"
#include "dcolorarrow.xbm"
#include "dcolorreset.xpm"
#include <tdeglobalsettings.h>
#include <tqpainter.h>
#include <tqbitmap.h>
#include <tqdrawutil.h>

class KDualColorButton::KDualColorPrivate
{
public:
    TQWidget* dialogParent;
};

KDualColorButton::KDualColorButton(TQWidget *parent, const char *name, TQWidget* dialogParent)
  : TQWidget(parent, name),
    d (new KDualColorPrivate)
{
    if (!dialogParent && parent) {
	d->dialogParent = parent;
    } else {
	d->dialogParent = dialogParent;
    }

    arrowBitmap = new TQBitmap(dcolorarrow_width, dcolorarrow_height,
                              (const unsigned char *)dcolorarrow_bits, true);
    arrowBitmap->setMask(*arrowBitmap); // heh
    resetPixmap = new TQPixmap((const char **)dcolorreset_xpm);
    fg = TQBrush(TQt::black, TQt::SolidPattern);
    bg = TQBrush(TQt::white, TQt::SolidPattern);
    curColor = Foreground;
    dragFlag = false;
    miniCtlFlag = false;
    if(sizeHint().isValid())
        setMinimumSize(sizeHint());
    setAcceptDrops(true);
}

KDualColorButton::KDualColorButton(const TQColor &fgColor, const TQColor &bgColor,
                                   TQWidget *parent, const char *name, TQWidget* dialogParent)
  : TQWidget(parent, name),
    d (new KDualColorPrivate)
{
    d->dialogParent = dialogParent;

    arrowBitmap = new TQBitmap(dcolorarrow_width, dcolorarrow_height,
                              (const unsigned char *)dcolorarrow_bits, true);
    arrowBitmap->setMask(*arrowBitmap);
    resetPixmap = new TQPixmap((const char **)dcolorreset_xpm);
    fg = TQBrush(fgColor, TQt::SolidPattern);
    bg = TQBrush(bgColor, TQt::SolidPattern);
    curColor = Foreground;
    dragFlag = false;
    miniCtlFlag = false;
    if(sizeHint().isValid())
        setMinimumSize(sizeHint());
    setAcceptDrops(true);
}

KDualColorButton::~KDualColorButton()
{
  delete d;
  delete arrowBitmap;
  delete resetPixmap;
}

TQColor KDualColorButton::foreground() const
{
    return fg.color();
}

TQColor KDualColorButton::background() const
{
    return bg.color();
}

KDualColorButton::DualColor KDualColorButton::current() const
{
    return curColor;
}

TQColor KDualColorButton::currentColor() const
{
    return (curColor == Background ? bg.color() : fg.color());
}

TQSize KDualColorButton::sizeHint() const
{
    return TQSize(34, 34);
}

void KDualColorButton::setForeground(const TQColor &c)
{
    fg = TQBrush(c, TQt::SolidPattern);
    repaint(false);

    emit fgChanged(fg.color());
}

void KDualColorButton::setBackground(const TQColor &c)
{
    bg = TQBrush(c, TQt::SolidPattern);
    repaint(false);

    emit bgChanged(bg.color());
}

void KDualColorButton::setCurrentColor(const TQColor &c)
{
    if(curColor == Background)
        bg = TQBrush(c, TQt::SolidPattern);
    else
        fg = TQBrush(c, TQt::SolidPattern);
    repaint(false);
}

void KDualColorButton::setCurrent(DualColor s)
{
    curColor = s;
    repaint(false);
}

void KDualColorButton::metrics(TQRect &fgRect, TQRect &bgRect)
{
    fgRect = TQRect(0, 0, width()-14, height()-14);
    bgRect = TQRect(14, 14, width()-14, height()-14);
}

void KDualColorButton::paintEvent(TQPaintEvent *)
{
    TQRect fgRect, bgRect;
    TQPainter p(this);

    metrics(fgRect, bgRect);
    TQBrush defBrush = colorGroup().brush(TQColorGroup::Button);

    qDrawShadeRect(&p, bgRect, colorGroup(), curColor == Background, 2, 0,
                   isEnabled() ? &bg : &defBrush);
    qDrawShadeRect(&p, fgRect, colorGroup(), curColor == Foreground, 2, 0,
                   isEnabled() ? &fg : &defBrush);
    p.setPen(colorGroup().shadow());
    p.drawPixmap(fgRect.right()+2, 0, *arrowBitmap);
    p.drawPixmap(0, fgRect.bottom()+2, *resetPixmap);

}

void KDualColorButton::dragEnterEvent(TQDragEnterEvent *ev)
{
    ev->accept(isEnabled() && KColorDrag::canDecode(ev));
}

void KDualColorButton::dropEvent(TQDropEvent *ev)
{
    TQColor c;
    if(KColorDrag::decode(ev, c)){
        if(curColor == Foreground){
            fg.setColor(c);
            emit fgChanged(c);
        }
        else{
            bg.setColor(c);
            emit(bgChanged(c));
        }
        repaint(false);
    }
}

void KDualColorButton::mousePressEvent(TQMouseEvent *ev)
{
    TQRect fgRect, bgRect;
    metrics(fgRect, bgRect);
    mPos = ev->pos();
    tmpColor = curColor;
    dragFlag = false;
    if(fgRect.contains(mPos)){
        curColor = Foreground;
        miniCtlFlag = false;
    }
    else if(bgRect.contains(mPos)){
        curColor = Background;
        miniCtlFlag = false;
   }
    else if(ev->pos().x() > fgRect.width()){
        // We handle the swap and reset controls as soon as the mouse is
        // is pressed and ignore further events on this click (mosfet).
        TQBrush c = fg;
        fg = bg;
        bg = c;
        emit fgChanged(fg.color());
        emit bgChanged(bg.color());
        miniCtlFlag = true;
    }
    else if(ev->pos().x() < bgRect.x()){
        fg.setColor(TQt::black);
        bg.setColor(TQt::white);
        emit fgChanged(fg.color());
        emit bgChanged(bg.color());
        miniCtlFlag = true;
    }
    repaint(false);
}


void KDualColorButton::mouseMoveEvent(TQMouseEvent *ev)
{
    if(!miniCtlFlag){
        int delay = TDEGlobalSettings::dndEventDelay();
        if(ev->x() >= mPos.x()+delay || ev->x() <= mPos.x()-delay ||
           ev->y() >= mPos.y()+delay || ev->y() <= mPos.y()-delay) {
            KColorDrag *d = new KColorDrag( curColor == Foreground ?
                                            fg.color() : bg.color(),
                                            this);
            d->dragCopy();
            dragFlag = true;
        }
    }
}

void KDualColorButton::mouseReleaseEvent(TQMouseEvent *ev)
{
    if(!miniCtlFlag){
        TQRect fgRect, bgRect;

        metrics(fgRect, bgRect);
        if(dragFlag)
            curColor = tmpColor;
        else if(fgRect.contains(ev->pos()) && curColor == Foreground){
            if(tmpColor == Background){
                curColor = Foreground;
                emit currentChanged(Foreground);
            }
            else{
                TQColor newColor = fg.color();
                if(KColorDialog::getColor(newColor, d->dialogParent) != TQDialog::Rejected){
                    fg.setColor(newColor);
                    emit fgChanged(newColor);
                }
            }
        }
        else if(bgRect.contains(ev->pos()) && curColor == Background){
            if(tmpColor == Foreground){
                curColor = Background;
                emit currentChanged(Background);
            }
            else{
                TQColor newColor = bg.color();
                if(KColorDialog::getColor(newColor, d->dialogParent) != TQDialog::Rejected){
                    bg.setColor(newColor);
                    emit bgChanged(newColor);
                }
            }
        }
        repaint(false);
        dragFlag = false;
    }
    else
        miniCtlFlag = false;
}

void KDualColorButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdualcolorbutton.moc"
