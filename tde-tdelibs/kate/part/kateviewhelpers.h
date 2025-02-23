/* This file is part of the KDE libraries
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 Anders Lund <anders@alweb.dk>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __KATE_VIEW_HELPERS_H__
#define __KATE_VIEW_HELPERS_H__

#include <tdeaction.h>
#include <klineedit.h>

#include <tqwidget.h>
#include <tqpixmap.h>
#include <tqcolor.h>
#include <tqscrollbar.h>
#include <tqintdict.h>

class KateDocument;
class KateView;
class KateViewInternal;

namespace Kate {
  class Command;
}

/**
 * This class is required because QScrollBar's sliderMoved() signal is
 * really supposed to be a sliderDragged() signal... so this way we can capture
 * MMB slider moves as well
 *
 * Also, it adds some usefull indicators on the scrollbar.
 */
class KateScrollBar : public TQScrollBar
{
  TQ_OBJECT

  public:
    KateScrollBar(Orientation orientation, class KateViewInternal *parent, const char* name = 0L);

    inline bool showMarks() { return m_showMarks; };
    inline void setShowMarks(bool b) { m_showMarks = b; update(); };

  signals:
    void sliderMMBMoved(int value);

  protected:
    virtual void mousePressEvent(TQMouseEvent* e);
    virtual void mouseReleaseEvent(TQMouseEvent* e);
    virtual void mouseMoveEvent (TQMouseEvent* e);
    virtual void paintEvent(TQPaintEvent *);
    virtual void resizeEvent(TQResizeEvent *);
    virtual void styleChange(TQStyle &oldStyle);
    virtual void valueChange();
    virtual void rangeChange();

  protected slots:
    void sliderMaybeMoved(int value);
    void marksChanged();

  private:
    void redrawMarks();
    void recomputeMarksPositions(bool forceFullUpdate = false);
    void watchScrollBarSize();

  bool m_middleMouseDown;

    KateView *m_view;
    KateDocument *m_doc;
    class KateViewInternal *m_viewInternal;

    int m_topMargin;
    int m_bottomMargin;
    uint m_savVisibleLines;

    TQIntDict<TQColor> m_lines;

    bool m_showMarks;
};

class KateCmdLine : public KLineEdit
{
  TQ_OBJECT

  public:
    KateCmdLine (KateView *view);

  private slots:
    void slotReturnPressed ( const TQString& cmd );
    void hideMe ();

  protected:
    void focusInEvent ( TQFocusEvent *ev );
    void keyPressEvent( TQKeyEvent *ev );

  private:
    void fromHistory( bool up );
    KateView *m_view;
    bool m_msgMode;
    TQString m_oldText;
    uint m_histpos; ///< position in the history
    uint m_cmdend; ///< the point where a command ends in the text, if we have a valid one.
    Kate::Command *m_command; ///< For completing flags/args and interactiveness
    class TDECompletion *m_oldCompletionObject; ///< save while completing command args.
    class KateCmdLnWhatsThis *m_help;
};

class KateIconBorder : public TQWidget
{
  TQ_OBJECT

  public:
    KateIconBorder( KateViewInternal* internalView, TQWidget *parent );

    // VERY IMPORTANT ;)
    virtual TQSize sizeHint() const;

    void updateFont();
    int lineNumberWidth() const;

    void setIconBorderOn(     bool enable );
    void setLineNumbersOn(    bool enable );
    void setDynWrapIndicators(int state );
    int dynWrapIndicators()  const { return m_dynWrapIndicators; }
    bool dynWrapIndicatorsOn() const { return m_dynWrapIndicatorsOn; }
    void setFoldingMarkersOn( bool enable );
    void toggleIconBorder()     { setIconBorderOn(     !iconBorderOn() );     }
    void toggleLineNumbers()    { setLineNumbersOn(    !lineNumbersOn() );    }
    void toggleFoldingMarkers() { setFoldingMarkersOn( !foldingMarkersOn() ); }
    bool iconBorderOn()       const { return m_iconBorderOn;     }
    bool lineNumbersOn()      const { return m_lineNumbersOn;    }
    bool foldingMarkersOn()   const { return m_foldingMarkersOn; }

    enum BorderArea { None, LineNumbers, IconBorder, FoldingMarkers };
    BorderArea positionToArea( const TQPoint& ) const;

  signals:
    void toggleRegionVisibility( unsigned int );

  private:
    void paintEvent( TQPaintEvent* );
    void paintBorder (int x, int y, int width, int height);

    void mousePressEvent( TQMouseEvent* );
    void mouseMoveEvent( TQMouseEvent* );
    void mouseReleaseEvent( TQMouseEvent* );
    void mouseDoubleClickEvent( TQMouseEvent* );

    void showMarkMenu( uint line, const TQPoint& pos );

    KateView *m_view;
    KateDocument *m_doc;
    KateViewInternal *m_viewInternal;

    bool m_iconBorderOn:1;
    bool m_lineNumbersOn:1;
    bool m_foldingMarkersOn:1;
    bool m_dynWrapIndicatorsOn:1;
    int m_dynWrapIndicators;

    uint m_lastClickedLine;

    int m_cachedLNWidth;

    int m_maxCharWidth;

    mutable TQPixmap m_arrow;
    mutable TQColor m_oldBackgroundColor;
};

class KateViewEncodingAction : public TDEActionMenu
{
  TQ_OBJECT

  public:
    KateViewEncodingAction(KateDocument *_doc, KateView *_view, const TQString& text, TQObject* parent = 0, const char* name = 0);

    ~KateViewEncodingAction(){;};

  private:
    KateDocument* doc;
    KateView *view;

  public  slots:
    void slotAboutToShow();

  private slots:
    void setMode (int mode);
};

#endif
