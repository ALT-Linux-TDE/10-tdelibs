/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
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
#ifndef RENDER_FORM_H
#define RENDER_FORM_H

#include "rendering/render_replaced.h"
#include "rendering/render_image.h"
#include "rendering/render_flow.h"
#include "rendering/render_style.h"
#include "html/html_formimpl.h"

class TQWidget;
class TQLineEdit;
class QListboxItem;

#include <ktextedit.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqpushbutton.h>
#include <tqhbox.h>
#include <tdelistbox.h>
#include <kcombobox.h>
#include "dom/dom_misc.h"

class TDEHTMLPartBrowserExtension;
class KSpell;
class KFindDialog;
class KReplaceDialog;
class KFind;
class KReplace;
class TDEAction;
class KURLRequester;

namespace DOM {
    class HTMLFormElementImpl;
    class HTMLInputElementImpl;
    class HTMLSelectElementImpl;
    class HTMLGenericFormElementImpl;
    class HTMLTextAreaElementImpl;
}

namespace tdehtml {

class DocLoader;

// -------------------------------------------------------------------------

class RenderFormElement : public tdehtml::RenderWidget
{
public:
    RenderFormElement(DOM::HTMLGenericFormElementImpl* node);
    virtual ~RenderFormElement();

    virtual const char *renderName() const { return "RenderForm"; }

    virtual bool isFormElement() const { return true; }

    // form elements never have padding
    virtual int paddingTop() const { return 0; }
    virtual int paddingBottom() const { return 0; }
    virtual int paddingLeft() const { return 0; }
    virtual int paddingRight() const { return 0; }

    virtual void updateFromElement();

    virtual void layout();
    virtual short baselinePosition( bool ) const;

    DOM::HTMLGenericFormElementImpl *element() const
    { return static_cast<DOM::HTMLGenericFormElementImpl*>(RenderObject::element()); }

protected:
    virtual bool isRenderButton() const { return false; }
    virtual bool isEditable() const { return false; }
    TQt::AlignmentFlags textAlignment() const;

    TQPoint m_mousePos;
    int m_state;
};

// -------------------------------------------------------------------------

// generic class for all buttons
class RenderButton : public RenderFormElement
{
    TQ_OBJECT
public:
    RenderButton(DOM::HTMLGenericFormElementImpl* node);

    virtual const char *renderName() const { return "RenderButton"; }

    virtual short baselinePosition( bool ) const;

    // don't even think about making this method virtual!
    DOM::HTMLInputElementImpl* element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }

protected:
    virtual bool isRenderButton() const { return true; }
};

// -------------------------------------------------------------------------

class RenderCheckBox : public RenderButton
{
    TQ_OBJECT
public:
    RenderCheckBox(DOM::HTMLInputElementImpl* node);

    virtual const char *renderName() const { return "RenderCheckBox"; }
    virtual void updateFromElement();
    virtual void calcMinMaxWidth();

    virtual bool handleEvent(const DOM::EventImpl&) { return false; }

    TQCheckBox *widget() const { return static_cast<TQCheckBox*>(m_widget); }

public slots:
    virtual void slotStateChanged(int state);
};

// -------------------------------------------------------------------------

class RenderRadioButton : public RenderButton
{
    TQ_OBJECT
public:
    RenderRadioButton(DOM::HTMLInputElementImpl* node);

    virtual const char *renderName() const { return "RenderRadioButton"; }

    virtual void calcMinMaxWidth();
    virtual void updateFromElement();

    virtual bool handleEvent(const DOM::EventImpl&) { return false; }

    TQRadioButton *widget() const { return static_cast<TQRadioButton*>(m_widget); }

public slots:
    virtual void slotToggled(bool);
};

// -------------------------------------------------------------------------

class RenderSubmitButton : public RenderButton
{
public:
    RenderSubmitButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderSubmitButton"; }

    virtual void calcMinMaxWidth();
    virtual void updateFromElement();
    virtual short baselinePosition( bool ) const;
private:
    TQString rawText();
};

// -------------------------------------------------------------------------

class RenderImageButton : public RenderImage
{
public:
    RenderImageButton(DOM::HTMLInputElementImpl *element)
        : RenderImage(element) {}

    virtual const char *renderName() const { return "RenderImageButton"; }
};


// -------------------------------------------------------------------------

class RenderResetButton : public RenderSubmitButton
{
public:
    RenderResetButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderResetButton"; }

};

// -------------------------------------------------------------------------

class RenderPushButton : public RenderSubmitButton
{
public:
    RenderPushButton(DOM::HTMLInputElementImpl *element)
        : RenderSubmitButton(element) {}

};

// -------------------------------------------------------------------------

class RenderLineEdit : public RenderFormElement
{
    TQ_OBJECT
public:
    RenderLineEdit(DOM::HTMLInputElementImpl *element);

    virtual void calcMinMaxWidth();

    virtual const char *renderName() const { return "RenderLineEdit"; }
    virtual void updateFromElement();
    virtual void setStyle(RenderStyle *style);

    void select();

    KLineEdit *widget() const { return static_cast<KLineEdit*>(m_widget); }
    DOM::HTMLInputElementImpl* element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }
    void highLightWord( unsigned int length, unsigned int pos );

    long selectionStart();
    long selectionEnd();
    void setSelectionStart(long pos);
    void setSelectionEnd(long pos);
    void setSelectionRange(long start, long end);
public slots:
    void slotReturnPressed();
    void slotTextChanged(const TQString &string);
protected:
    virtual void handleFocusOut();

private:
    virtual bool isEditable() const { return true; }
    virtual bool canHaveBorder() const { return true; }
};

// -------------------------------------------------------------------------

class LineEditWidget : public KLineEdit
{
    TQ_OBJECT
public:
    LineEditWidget(DOM::HTMLInputElementImpl* input,
                   TDEHTMLView* view, TQWidget* parent);
    ~LineEditWidget();
    void highLightWord( unsigned int length, unsigned int pos );

protected:
    virtual bool event( TQEvent *e );
    virtual void mouseMoveEvent(TQMouseEvent *e);
    virtual TQPopupMenu *createPopupMenu();
private slots:
    void extendedMenuActivated( int id);
    void slotCheckSpelling();
    void slotSpellCheckReady( KSpell *s );
    void slotSpellCheckDone( const TQString &s );
    void spellCheckerMisspelling( const TQString &text, const TQStringList &, unsigned int pos);
    void spellCheckerCorrected( const TQString &, const TQString &, unsigned int );
    void spellCheckerFinished();
    void slotRemoveFromHistory( const TQString & );

private:
    enum LineEditMenuID {
        ClearHistory,
        EditHistory
    };
    DOM::HTMLInputElementImpl* m_input;
    TDEHTMLView* m_view;
    KSpell *m_spell;
    TDEAction *m_spellAction;
};

// -------------------------------------------------------------------------

class RenderFieldset : public RenderBlock
{
public:
    RenderFieldset(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderFieldSet"; }
    virtual RenderObject* layoutLegend(bool relayoutChildren);
    virtual void setStyle(RenderStyle* _style);

protected:
    virtual void paintBoxDecorations(PaintInfo& pI, int _tx, int _ty);
    void paintBorderMinusLegend(TQPainter *p, int _tx, int _ty, int w,
                                  int h, const RenderStyle *style, int lx, int lw);
    RenderObject* findLegend();
};

// -------------------------------------------------------------------------

class RenderFileButton : public RenderFormElement
{
    TQ_OBJECT
public:
    RenderFileButton(DOM::HTMLInputElementImpl *element);

    virtual const char *renderName() const { return "RenderFileButton"; }
    virtual void calcMinMaxWidth();
    virtual void updateFromElement();
    void select();

    KURLRequester *widget() const { return static_cast<KURLRequester*>(m_widget); }

    DOM::HTMLInputElementImpl *element() const
    { return static_cast<DOM::HTMLInputElementImpl*>(RenderObject::element()); }

public slots:
    void slotReturnPressed();
    void slotTextChanged(const TQString &string);
    void slotUrlSelected(const TQString &string);

protected:
    virtual void handleFocusOut();

    virtual bool isEditable() const { return true; }
    virtual bool canHaveBorder() const { return true; }
    virtual bool acceptsSyntheticEvents() const { return false; }

    bool m_clicked;
    bool m_haveFocus;
};


// -------------------------------------------------------------------------

class RenderLabel : public RenderFormElement
{
public:
    RenderLabel(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderLabel"; }

protected:
    virtual bool canHaveBorder() const { return true; }
};


// -------------------------------------------------------------------------

class RenderLegend : public RenderBlock
{
public:
    RenderLegend(DOM::HTMLGenericFormElementImpl *element);

    virtual const char *renderName() const { return "RenderLegend"; }
};

// -------------------------------------------------------------------------

class ComboBoxWidget : public KComboBox
{
public:
    ComboBoxWidget(TQWidget *parent);

protected:
    virtual bool event(TQEvent *);
    virtual bool eventFilter(TQObject *dest, TQEvent *e);
};

// -------------------------------------------------------------------------

class RenderSelect : public RenderFormElement
{
    TQ_OBJECT
public:
    RenderSelect(DOM::HTMLSelectElementImpl *element);

    virtual const char *renderName() const { return "RenderSelect"; }

    virtual void calcMinMaxWidth();
    virtual void layout();

    void setOptionsChanged(bool _optionsChanged);

    bool selectionChanged() { return m_selectionChanged; }
    void setSelectionChanged(bool _selectionChanged) { m_selectionChanged = _selectionChanged; }
    virtual void updateFromElement();

    void updateSelection();

    DOM::HTMLSelectElementImpl *element() const
    { return static_cast<DOM::HTMLSelectElementImpl*>(RenderObject::element()); }

protected:
    TDEListBox *createListBox();
    ComboBoxWidget *createComboBox();

    unsigned  m_size;
    bool m_multiple;
    bool m_useListBox;
    bool m_selectionChanged;
    bool m_ignoreSelectEvents;
    bool m_optionsChanged;

protected slots:
    void slotSelected(int index);
    void slotSelectionChanged();
};

// -------------------------------------------------------------------------
class TextAreaWidget : public KTextEdit
{
    TQ_OBJECT
public:
    TextAreaWidget(int wrap, TQWidget* parent);
    virtual ~TextAreaWidget();

protected:
    virtual bool event (TQEvent *e );
    virtual TQPopupMenu *createPopupMenu(const TQPoint& pos);
    virtual TQPopupMenu* createPopupMenu() { return KTextEdit::createPopupMenu(); }
private slots:
    void slotFind();
    void slotDoFind();
    void slotFindNext();
    void slotReplace();
    void slotDoReplace();
    void slotReplaceNext();
    void slotReplaceText(const TQString&, int, int, int);
    void slotFindHighlight(const TQString&, int, int);
private:
    KFindDialog *m_findDlg;
    KFind *m_find;
    KReplaceDialog *m_repDlg;
    KReplace *m_replace;
    TDEAction *m_findAction;
    TDEAction *m_findNextAction;
    TDEAction *m_replaceAction;
    int m_findIndex, m_findPara;
    int m_repIndex, m_repPara;
};


// -------------------------------------------------------------------------

class RenderTextArea : public RenderFormElement
{
    TQ_OBJECT
public:
    RenderTextArea(DOM::HTMLTextAreaElementImpl *element);
    ~RenderTextArea();

    virtual const char *renderName() const { return "RenderTextArea"; }
    virtual void calcMinMaxWidth();
    virtual void layout();
    virtual void setStyle(RenderStyle *style);

    virtual void close ( );
    virtual void updateFromElement();

    // don't even think about making this method virtual!
    TextAreaWidget *widget() const { return static_cast<TextAreaWidget*>(m_widget); }
    DOM::HTMLTextAreaElementImpl* element() const
    { return static_cast<DOM::HTMLTextAreaElementImpl*>(RenderObject::element()); }

    TQString text();
    void highLightWord( unsigned int length, unsigned int pos );

    void select();

    long selectionStart();
    long selectionEnd();
    void setSelectionStart(long pos);
    void setSelectionEnd(long pos);
    void setSelectionRange(long start, long end);
protected slots:
    void slotTextChanged();

protected:
    virtual void handleFocusOut();

    virtual bool isEditable() const { return true; }
    virtual bool canHaveBorder() const { return true; }

    bool scrollbarsStyled;
private:
    //Convert para, index -> offset
    long computeCharOffset(int para, int index);

    //Convert offset -> para, index
    void computeParagraphAndIndex(long offset, int* para, int* index);

    //Helper for doing the conversion..
    enum Mode { ParaLength,     //Returns the length of the entire paragraph
           ParaPortionLength,   //Return length of paragraph portion set by threshold
           ParaPortionOffset }; //Return offset that matches the length threshold.
    int queryParagraphInfo(int para, Mode m, int param = -1);
};

// -------------------------------------------------------------------------

} //namespace

#endif
