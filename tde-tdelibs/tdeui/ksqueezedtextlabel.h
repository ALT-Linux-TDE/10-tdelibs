/* This file is part of the KDE libraries
   Copyright (C) 2000 Ronny Standtke <Ronny.Standtke@gmx.de>

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

#ifndef KSQUEEZEDTEXTLABEL_H
#define KSQUEEZEDTEXTLABEL_H

#include <tqlabel.h>

#include <tdelibs_export.h>

/**
 * @short A replacement for TQLabel that squeezes its text
 *
 * A label class that squeezes its text into the label
 *
 * If the text is too long to fit into the label it is divided into
 * remaining left and right parts which are separated by three dots.
 *
 * Example:
 * http://www.kde.org/documentation/index.html could be squeezed to
 * http://www.kde...ion/index.html
 *
 * \image html ksqueezedtextlabel.png "KSqueezedTextLabel Widget"
 *
 * @author Ronny Standtke <Ronny.Standtke@gmx.de>
 */

/*
 * QLabel
 */
class TDEUI_EXPORT KSqueezedTextLabel : public TQLabel {
  TQ_OBJECT

public:
  /**
   * Default constructor.
   */
  KSqueezedTextLabel( TQWidget *parent, const char *name = 0 );
  KSqueezedTextLabel( const TQString &text, TQWidget *parent, const char *name = 0 );

  virtual TQSize minimumSizeHint() const;
  virtual TQSize sizeHint() const;
  /**
   * Overridden for internal reasons; the API remains unaffected.
   */
  virtual void setAlignment( int );

public slots:
  void setText( const TQString & );

protected:
  /**
   * used when widget is resized
   */
  void resizeEvent( TQResizeEvent * );
  /**
   * does the dirty work
   */
  void squeezeTextToLabel();
  TQString fullText;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KSqueezedTextLabelPrivate;
  KSqueezedTextLabelPrivate *d;
};

#endif // KSQUEEZEDTEXTLABEL_H
