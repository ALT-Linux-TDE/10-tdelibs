/*
 * This file is part of the KDE Libraries
 * Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
 * Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef _TDEABOUTDIALOG_PRIVATE_H_
#define _TDEABOUTDIALOG_PRIVATE_H_

#include <tqlabel.h>
class TQFrame;
class TQTabWidget;
class TQVBoxLayout;

/**
 * Used internally by TDEAboutContainerBase
 * @internal
 */
class KImageTrackLabel : public TQLabel
{
  TQ_OBJECT

  public:
    enum MouseMode
    {
      MousePress = 1,
      MouseRelease,
      MouseDoubleClick,
      MouseMove
    };

  public:
    KImageTrackLabel( TQWidget * parent, const char * name=0, WFlags f=0 );

  signals:
    void mouseTrack( int mode, const TQMouseEvent *e );

  protected:
    virtual void mousePressEvent( TQMouseEvent *e );
    virtual void mouseReleaseEvent( TQMouseEvent *e );
    virtual void mouseDoubleClickEvent( TQMouseEvent *e );
    virtual void mouseMoveEvent ( TQMouseEvent *e );
};

class TDEAboutContainer;

class TDEAboutContainerBasePrivate;

/**
 * Used internally by TDEAboutDialog
 * @internal
 */
class TDEAboutContainerBase : public TQWidget
{
  TQ_OBJECT

  public:
    enum LayoutType
    {
      AbtPlain         = 0x0001,
      AbtTabbed        = 0x0002,
      AbtTitle         = 0x0004,
      AbtImageLeft     = 0x0008,
      AbtImageRight    = 0x0010,
      AbtImageOnly     = 0x0020,
      AbtProduct       = 0x0040,
      AbtKDEStandard   = AbtTabbed|AbtTitle|AbtImageLeft,
      AbtAppStandard   = AbtTabbed|AbtTitle|AbtProduct,
      AbtImageAndTitle = AbtPlain|AbtTitle|AbtImageOnly
    };

  public:
    TDEAboutContainerBase( int layoutType, TQWidget *parent = 0, char *name = 0 );
    virtual void show( void );
    virtual TQSize sizeHint( void ) const;

    void setTitle( const TQString &title );
    void setImage( const TQString &fileName );
    void setImageBackgroundColor( const TQColor &color );
    void setImageFrame( bool state );
    void setProgramLogo( const TQString &fileName );
    void setProgramLogo( const TQPixmap &pixmap );
    void setProduct( const TQString &appName, const TQString &version,
		     const TQString &author, const TQString &year );

    TQFrame *addTextPage( const TQString &title, const TQString &text,
			 bool richText=false, int numLines=10 );
    TQFrame *addLicensePage( const TQString &title, const TQString &text,
			 int numLines=10 );
    TDEAboutContainer *addContainerPage( const TQString &title,
      int childAlignment = AlignCenter, int innerAlignment = AlignCenter );
    TDEAboutContainer *addScrolledContainerPage( const TQString &title,
      int childAlignment = AlignCenter, int innerAlignment = AlignCenter );

    TQFrame *addEmptyPage( const TQString &title );

    TDEAboutContainer *addContainer( int childAlignment, int innerAlignment );

  public slots:
    virtual void slotMouseTrack( int mode, const TQMouseEvent *e );
    virtual void slotUrlClick( const TQString &url );
    virtual void slotMailClick( const TQString &name, const TQString &address );

  protected:
    virtual void fontChange( const TQFont &oldFont );

  signals:
    void mouseTrack( int mode, const TQMouseEvent *e );
    void urlClick( const TQString &url );
    void mailClick( const TQString &name, const TQString &address );

  private:
    TQVBoxLayout *mTopLayout;
    KImageTrackLabel *mImageLabel;
    TQLabel  *mTitleLabel;
    TQLabel  *mIconLabel;
    TQLabel  *mVersionLabel;
    TQLabel  *mAuthorLabel;
    TQFrame  *mImageFrame;
    TQTabWidget *mPageTab;
    TQFrame  *mPlainSpace;

    TDEAboutContainerBasePrivate* const d;
};


#endif
