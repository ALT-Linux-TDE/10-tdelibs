/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2006 Maksim Orlovich (maksim@kde.org)
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

#include <tdecompletionbox.h>
#include <kcursor.h>
#include <kdebug.h>
#include <tdefiledialog.h>
#include <kfind.h>
#include <kfinddialog.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kreplace.h>
#include <kreplacedialog.h>
#include <tdespell.h>
#include <kurlcompletion.h>
#include <twin.h>

#include <tqstyle.h>

#include "misc/helper.h"
#include "xml/dom2_eventsimpl.h"
#include "html/html_formimpl.h"
#include "misc/htmlhashes.h"

#include "rendering/render_form.h"
#include <assert.h>

#include "tdehtmlview.h"
#include "tdehtml_ext.h"
#include "xml/dom_docimpl.h"

#include <tqpopupmenu.h>
#include <tqbitmap.h>

using namespace tdehtml;

RenderFormElement::RenderFormElement(HTMLGenericFormElementImpl *element)
    : RenderWidget(element)
{
    // init RenderObject attributes
    setInline(true);   // our object is Inline

    m_state = 0;
}

RenderFormElement::~RenderFormElement()
{
}

short RenderFormElement::baselinePosition( bool f ) const
{
    return RenderWidget::baselinePosition( f ) - 2 - style()->fontMetrics().descent();
}

void RenderFormElement::updateFromElement()
{
    m_widget->setEnabled(!element()->disabled());
    RenderWidget::updateFromElement();
}

void RenderFormElement::layout()
{
    TDEHTMLAssert( needsLayout() );
    TDEHTMLAssert( minMaxKnown() );

    // minimum height
    m_height = 0;

    calcWidth();
    calcHeight();

    if ( m_widget )
        resizeWidget(m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                     m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom());

    setNeedsLayout(false);
}

TQt::AlignmentFlags RenderFormElement::textAlignment() const
{
    switch (style()->textAlign()) {
        case LEFT:
        case TDEHTML_LEFT:
            return TQt::AlignLeft;
        case RIGHT:
        case TDEHTML_RIGHT:
            return TQt::AlignRight;
        case CENTER:
        case TDEHTML_CENTER:
            return TQt::AlignHCenter;
        case JUSTIFY:
            // Just fall into the auto code for justify.
        case TAAUTO:
            return style()->direction() == RTL ? TQt::AlignRight : TQt::AlignLeft;
    }
    assert(false); // Should never be reached.
    return TQt::AlignLeft;
}

// -------------------------------------------------------------------------

RenderButton::RenderButton(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{
}

short RenderButton::baselinePosition( bool f ) const
{
    return RenderWidget::baselinePosition( f ) - 2;
}

// -------------------------------------------------------------------------------


RenderCheckBox::RenderCheckBox(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    TQCheckBox* b = new TQCheckBox(view()->viewport(), "__tdehtml");
    b->setAutoMask(true);
    b->setMouseTracking(true);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b,TQ_SIGNAL(stateChanged(int)),this,TQ_SLOT(slotStateChanged(int)));
}


void RenderCheckBox::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown() );

    TQCheckBox *cb = static_cast<TQCheckBox *>( m_widget );
    TQSize s( cb->style().pixelMetric( TQStyle::PM_IndicatorWidth ),
             cb->style().pixelMetric( TQStyle::PM_IndicatorHeight ) );
    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderCheckBox::updateFromElement()
{
    widget()->setChecked(element()->checked());

    RenderButton::updateFromElement();
}

void RenderCheckBox::slotStateChanged(int state)
{
    element()->setChecked(state == TQButton::On);
    element()->setIndeterminate(state == TQButton::NoChange);

    ref();
    element()->onChange();
    deref();
}

// -------------------------------------------------------------------------------

RenderRadioButton::RenderRadioButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    TQRadioButton* b = new TQRadioButton(view()->viewport(), "__tdehtml");
    b->setMouseTracking(true);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b,TQ_SIGNAL(toggled(bool)),this,TQ_SLOT(slotToggled(bool)));
}

void RenderRadioButton::updateFromElement()
{
    widget()->setChecked(element()->checked());

    RenderButton::updateFromElement();
}

void RenderRadioButton::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown() );

    TQRadioButton *rb = static_cast<TQRadioButton *>( m_widget );
    TQSize s( rb->style().pixelMetric( TQStyle::PM_ExclusiveIndicatorWidth ),
             rb->style().pixelMetric( TQStyle::PM_ExclusiveIndicatorHeight ) );
    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderRadioButton::slotToggled(bool activated)
{
    if(activated) {
      ref();
      element()->onChange();
      deref();
    }
}

// -------------------------------------------------------------------------------


RenderSubmitButton::RenderSubmitButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
    TQPushButton* p = new TQPushButton(view()->viewport(), "__tdehtml");
    setQWidget(p);
    p->setAutoMask(true);
    p->setMouseTracking(true);
}

TQString RenderSubmitButton::rawText()
{
    TQString value = element()->valueWithDefault().string();
    value = value.stripWhiteSpace();
    TQString raw;
    for(unsigned int i = 0; i < value.length(); i++) {
        raw += value[i];
        if(value[i] == '&')
            raw += '&';
    }
    return raw;
}

void RenderSubmitButton::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown() );

    TQString raw = rawText();
    TQPushButton* pb = static_cast<TQPushButton*>(m_widget);
    pb->setText(raw);
    pb->setFont(style()->font());

    bool empty = raw.isEmpty();
    if ( empty )
        raw = TQString::fromLatin1("X");
    TQFontMetrics fm = pb->fontMetrics();
    TQSize ts = fm.size( ShowPrefix, raw);
    TQSize s(pb->style().sizeFromContents( TQStyle::CT_PushButton, pb, ts )
            .expandedTo(TQApplication::globalStrut()));
    int margin = pb->style().pixelMetric( TQStyle::PM_ButtonMargin, pb) +
		 pb->style().pixelMetric( TQStyle::PM_DefaultFrameWidth, pb ) * 2;
    int w = ts.width() + margin;
    int h = s.height();
    if (pb->isDefault() || pb->autoDefault()) {
	int dbw = pb->style().pixelMetric( TQStyle::PM_ButtonDefaultIndicator, pb ) * 2;
	w += dbw;
    }

    // add 30% margins to the width (heuristics to make it look similar to IE)
    s = TQSize( w*13/10, h ).expandedTo(TQApplication::globalStrut());

    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderButton::calcMinMaxWidth();
}

void RenderSubmitButton::updateFromElement()
{
    TQString oldText = static_cast<TQPushButton*>(m_widget)->text();
    TQString newText = rawText();
    static_cast<TQPushButton*>(m_widget)->setText(newText);
    if ( oldText != newText )
        setNeedsLayoutAndMinMaxRecalc();
    RenderFormElement::updateFromElement();
}

short RenderSubmitButton::baselinePosition( bool f ) const
{
    return RenderFormElement::baselinePosition( f );
}

// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(HTMLInputElementImpl *element)
    : RenderSubmitButton(element)
{
}

// -------------------------------------------------------------------------------

LineEditWidget::LineEditWidget(DOM::HTMLInputElementImpl* input, TDEHTMLView* view, TQWidget* parent)
    : KLineEdit(parent, "__tdehtml"), m_input(input), m_view(view), m_spell(0)
{
    setMouseTracking(true);
    TDEActionCollection *ac = new TDEActionCollection(this);
    m_spellAction = KStdAction::spelling( this, TQ_SLOT( slotCheckSpelling() ), ac );
}

LineEditWidget::~LineEditWidget()
{
    delete m_spell;
    m_spell = 0L;
}

void LineEditWidget::slotCheckSpelling()
{
    if ( text().isEmpty() ) {
        return;
    }

    delete m_spell;
    m_spell = new KSpell( this, i18n( "Spell Checking" ), this, TQ_SLOT( slotSpellCheckReady( KSpell *) ), 0, true, true);

    connect( m_spell, TQ_SIGNAL( death() ),this, TQ_SLOT( spellCheckerFinished() ) );
    connect( m_spell, TQ_SIGNAL( misspelling( const TQString &, const TQStringList &, unsigned int ) ),this, TQ_SLOT( spellCheckerMisspelling( const TQString &, const TQStringList &, unsigned int ) ) );
    connect( m_spell, TQ_SIGNAL( corrected( const TQString &, const TQString &, unsigned int ) ),this, TQ_SLOT( spellCheckerCorrected( const TQString &, const TQString &, unsigned int ) ) );
}

void LineEditWidget::spellCheckerMisspelling( const TQString &_text, const TQStringList &, unsigned int pos)
{
    highLightWord( _text.length(),pos );
}

void LineEditWidget::highLightWord( unsigned int length, unsigned int pos )
{
    setSelection ( pos, length );
}

void LineEditWidget::spellCheckerCorrected( const TQString &old, const TQString &corr, unsigned int pos )
{
    if( old!= corr )
    {
        setSelection ( pos, old.length() );
        insert( corr );
        setSelection ( pos, corr.length() );
    }
}

void LineEditWidget::spellCheckerFinished()
{
}

void LineEditWidget::slotSpellCheckReady( KSpell *s )
{
    s->check( text() );
    connect( s, TQ_SIGNAL( done( const TQString & ) ), this, TQ_SLOT( slotSpellCheckDone( const TQString & ) ) );
}

void LineEditWidget::slotSpellCheckDone( const TQString &s )
{
    if( s != text() )
        setText( s );
}


TQPopupMenu *LineEditWidget::createPopupMenu()
{
    TQPopupMenu *popup = KLineEdit::createPopupMenu();

    if ( !popup ) {
        return 0L;
    }

    connect( popup, TQ_SIGNAL( activated( int ) ),
             this, TQ_SLOT( extendedMenuActivated( int ) ) );

    if (m_input->autoComplete()) {
        popup->insertSeparator();
        int id = popup->insertItem( SmallIconSet("edit"), i18n("&Edit History..."), EditHistory );
        popup->setItemEnabled( id, (compObj() && !compObj()->isEmpty()) );
        id = popup->insertItem( SmallIconSet("history_clear"), i18n("Clear &History"), ClearHistory );
        popup->setItemEnabled( id, (compObj() && !compObj()->isEmpty()) );
    }

    if (echoMode() == TQLineEdit::Normal &&
        !isReadOnly()) {
        popup->insertSeparator();

        m_spellAction->plug(popup);
        m_spellAction->setEnabled( !text().isEmpty() );
    }

    return popup;
}


void LineEditWidget::extendedMenuActivated( int id)
{
    switch ( id )
    {
    case ClearHistory:
        m_view->clearCompletionHistory(m_input->name().string());
        if (compObj())
          compObj()->clear();
    case EditHistory:
      {
        KHistoryComboEditor dlg( compObj() ? compObj()->items() : TQStringList(), this );
        connect( &dlg, TQ_SIGNAL( removeFromHistory(const TQString&) ), TQ_SLOT( slotRemoveFromHistory(const TQString&)) );
        dlg.exec();
      }
    default:
        break;
    }
}

void LineEditWidget::slotRemoveFromHistory(const TQString &entry)
{
    m_view->removeFormCompletionItem(m_input->name().string(), entry);
    if (compObj())
       compObj()->removeItem(entry);
}


bool LineEditWidget::event( TQEvent *e )
{
    if (KLineEdit::event(e))
	return true;

    if ( e->type() == TQEvent::AccelAvailable && isReadOnly() ) {
        TQKeyEvent* ke = (TQKeyEvent*) e;
        if ( ke->state() & ControlButton ) {
            switch ( ke->key() ) {
                case Key_Left:
                case Key_Right:
                case Key_Up:
                case Key_Down:
                case Key_Home:
                case Key_End:
                    ke->accept();
                default:
                break;
            }
        }
    }
    return false;
}

void LineEditWidget::mouseMoveEvent(TQMouseEvent *e)
{
    // hack to prevent Qt from calling setCursor on the widget
    setDragEnabled(false);
    KLineEdit::mouseMoveEvent(e);
    setDragEnabled(true);
}


// -----------------------------------------------------------------------------

RenderLineEdit::RenderLineEdit(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
    LineEditWidget *edit = new LineEditWidget(element, view(), view()->viewport());
    connect(edit,TQ_SIGNAL(returnPressed()), this, TQ_SLOT(slotReturnPressed()));
    connect(edit,TQ_SIGNAL(textChanged(const TQString &)),this,TQ_SLOT(slotTextChanged(const TQString &)));

    if(element->inputType() == HTMLInputElementImpl::PASSWORD)
        edit->setEchoMode( TQLineEdit::Password );

    if ( element->autoComplete() ) {
        TQStringList completions = view()->formCompletionItems(element->name().string());
        if (completions.count()) {
            edit->completionObject()->setItems(completions);
            edit->setContextMenuEnabled(true);
            edit->completionBox()->setTabHandling( false );
        }
    }

    setQWidget(edit);
}

void RenderLineEdit::setStyle(RenderStyle* _style)
{
    RenderFormElement::setStyle( _style );

    widget()->setAlignment(textAlignment());
}

void RenderLineEdit::highLightWord( unsigned int length, unsigned int pos )
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    if ( w )
        w->highLightWord( length, pos );
}


void RenderLineEdit::slotReturnPressed()
{
    // don't submit the form when return was pressed in a completion-popup
    TDECompletionBox *box = widget()->completionBox(false);

    if ( box && box->isVisible() && box->currentItem() != -1 ) {
      box->hide();
      return;
    }

    // Emit onChange if necessary
    // Works but might not be enough, dirk said he had another solution at
    // hand (can't remember which) - David
    handleFocusOut();

    HTMLFormElementImpl* fe = element()->form();
    if ( fe )
        fe->submitFromKeyboard();
}

void RenderLineEdit::handleFocusOut()
{
    if ( widget() && widget()->edited() ) {
        element()->onChange();
        widget()->setEdited( false );
    }
}

void RenderLineEdit::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown() );

    const TQFontMetrics &fm = style()->fontMetrics();
    TQSize s;

    int size = element()->size();

    int h = fm.lineSpacing();
    int w = fm.width( 'x' ) * (size > 0 ? size+1 : 17); // "some"
    s = TQSize(w + 2 + 2*widget()->frameWidth(),
              kMax(h, 14) + 2 + 2*widget()->frameWidth())
        .expandedTo(TQApplication::globalStrut());

    setIntrinsicWidth( s.width() );
    setIntrinsicHeight( s.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderLineEdit::updateFromElement()
{
    int ml = element()->maxLength();
    if ( ml < 0 )
        ml = 32767;

    if ( widget()->maxLength() != ml )  {
        widget()->setMaxLength( ml );
    }

    if (element()->value().string() != widget()->text()) {
        widget()->blockSignals(true);
        int pos = widget()->cursorPosition();
        widget()->setText(element()->value().string());

        widget()->setEdited( false );

        widget()->setCursorPosition(pos);
        widget()->blockSignals(false);
    }
    widget()->setReadOnly(element()->readOnly());

    RenderFormElement::updateFromElement();
}

void RenderLineEdit::slotTextChanged(const TQString &string)
{
    // don't use setValue here!
    element()->m_value = string;
    element()->m_unsubmittedFormChange = true;
}

void RenderLineEdit::select()
{
    static_cast<LineEditWidget*>(m_widget)->selectAll();
}

long RenderLineEdit::selectionStart()
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    if (w->hasSelectedText())
        return w->selectionStart();
    else
        return w->cursorPosition();
}


long RenderLineEdit::selectionEnd()
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    if (w->hasSelectedText())
        return w->selectionStart() + w->selectedText().length();
    else
        return w->cursorPosition();
}

void RenderLineEdit::setSelectionStart(long pos)
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    //See whether we have a non-empty selection now.
    long end = selectionEnd();
    if (end > pos)
        w->setSelection(pos, end - pos);
    w->setCursorPosition(pos);
}

void RenderLineEdit::setSelectionEnd(long pos)
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    //See whether we have a non-empty selection now.
    long start = selectionStart();
    if (start < pos)
        w->setSelection(start, pos - start);

    w->setCursorPosition(pos);
}

void RenderLineEdit::setSelectionRange(long start, long end)
{
    LineEditWidget* w = static_cast<LineEditWidget*>(m_widget);
    w->setCursorPosition(end);
    w->setSelection(start, end - start);
}

// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(HTMLGenericFormElementImpl *element)
    : RenderBlock(element)
{
}

RenderObject* RenderFieldset::layoutLegend(bool relayoutChildren)
{
    RenderObject* legend = findLegend();
    if (legend) {
        if (relayoutChildren)
            legend->setNeedsLayout(true);
        legend->layoutIfNeeded();

        int xPos = borderLeft() + paddingLeft() + legend->marginLeft();
        if (style()->direction() == RTL)
            xPos = m_width - paddingRight() - borderRight() - legend->width() - legend->marginRight();
        int b = borderTop();
        int h = legend->height();
        legend->setPos(xPos, kMax((b-h)/2, 0));
        m_height = kMax(b,h) + paddingTop();
    }
    return legend;
}

RenderObject* RenderFieldset::findLegend()
{
    for (RenderObject* legend = firstChild(); legend; legend = legend->nextSibling()) {
      if (!legend->isFloatingOrPositioned() && legend->element() &&
          legend->element()->id() == ID_LEGEND)
        return legend;
    }
    return 0;
}

void RenderFieldset::paintBoxDecorations(PaintInfo& pI, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::paintDecorations()" << endl;

    RenderObject* legend = findLegend();
    if (!legend)
        return RenderBlock::paintBoxDecorations(pI, _tx, _ty);

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    int yOff = (legend->yPos() > 0) ? 0 : (legend->height()-borderTop())/2;
    h -= yOff;
    _ty += yOff - borderTopExtra();

    int my = kMax(_ty,pI.r.y());
    int end = kMin( pI.r.y() + pI.r.height(),  _ty + h );
    int mh = end - my;

    paintBackground(pI.p, style()->backgroundColor(), style()->backgroundLayers(), my, mh, _tx, _ty, w, h);

    if ( style()->hasBorder() )
	    paintBorderMinusLegend(pI.p, _tx, _ty, w, h, style(), legend->xPos(), legend->width());
}

void RenderFieldset::paintBorderMinusLegend(TQPainter *p, int _tx, int _ty, int w, int h,
                                            const RenderStyle* style, int lx, int lw)
{

    const TQColor& tc = style->borderTopColor();
    const TQColor& bc = style->borderBottomColor();

    EBorderStyle ts = style->borderTopStyle();
    EBorderStyle bs = style->borderBottomStyle();
    EBorderStyle ls = style->borderLeftStyle();
    EBorderStyle rs = style->borderRightStyle();

    bool render_t = ts > BHIDDEN;
    bool render_l = ls > BHIDDEN;
    bool render_r = rs > BHIDDEN;
    bool render_b = bs > BHIDDEN;

    if(render_t) {
        drawBorder(p, _tx, _ty, _tx + lx, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE)?style->borderLeftWidth():0), 0);
        drawBorder(p, _tx+lx+lw, _ty, _tx + w, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   0, (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE)?style->borderRightWidth():0));
    }

    if(render_b)
        drawBorder(p, _tx, _ty + h - style->borderBottomWidth(), _tx + w, _ty + h, BSBottom, bc, style->color(), bs,
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE)?style->borderLeftWidth():0),
                   (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE)?style->borderRightWidth():0));

    if(render_l)
    {
	const TQColor& lc = style->borderLeftColor();

	bool ignore_top =
	  (tc == lc) &&
	  (ls >= OUTSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == lc) &&
	  (ls >= OUTSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx, _ty, _tx + style->borderLeftWidth(), _ty + h, BSLeft, lc, style->color(), ls,
		   ignore_top?0:style->borderTopWidth(),
		   ignore_bottom?0:style->borderBottomWidth());
    }

    if(render_r)
    {
	const TQColor& rc = style->borderRightColor();

	bool ignore_top =
	  (tc == rc) &&
	  (rs >= DOTTED || rs == INSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == rc) &&
	  (rs >= DOTTED || rs == INSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx + w - style->borderRightWidth(), _ty, _tx + w, _ty + h, BSRight, rc, style->color(), rs,
		   ignore_top?0:style->borderTopWidth(),
		   ignore_bottom?0:style->borderBottomWidth());
    }
}

void RenderFieldset::setStyle(RenderStyle* _style)
{
    RenderBlock::setStyle(_style);

    // WinIE renders fieldsets with display:inline like they're inline-blocks.  For us,
    // an inline-block is just a block element with replaced set to true and inline set
    // to true.  Ensure that if we ended up being inline that we set our replaced flag
    // so that we're treated like an inline-block.
    if (isInline())
        setReplaced(true);
}

// -------------------------------------------------------------------------

RenderFileButton::RenderFileButton(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
    KURLRequester* w = new KURLRequester( view()->viewport(), "__tdehtml" );

    w->setMode(KFile::File | KFile::ExistingOnly);
    w->completionObject()->setDir(TDEGlobalSettings::documentPath());

    connect(w->lineEdit(), TQ_SIGNAL(returnPressed()), this, TQ_SLOT(slotReturnPressed()));
    connect(w->lineEdit(), TQ_SIGNAL(textChanged(const TQString &)),this,TQ_SLOT(slotTextChanged(const TQString &)));
    connect(w, TQ_SIGNAL(urlSelected(const TQString &)),this,TQ_SLOT(slotUrlSelected(const TQString &)));

    setQWidget(w);
    m_haveFocus = false;
}



void RenderFileButton::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown() );

    const TQFontMetrics &fm = style()->fontMetrics();
    int size = element()->size();

    int h = fm.lineSpacing();
    int w = fm.width( 'x' ) * (size > 0 ? size+1 : 17); // "some"
    KLineEdit* edit = static_cast<KURLRequester*>( m_widget )->lineEdit();
    TQSize s = edit->style().sizeFromContents(TQStyle::CT_LineEdit,
                                             edit,
          TQSize(w + 2 + 2*edit->frameWidth(), kMax(h, 14) + 2 + 2*edit->frameWidth()))
        .expandedTo(TQApplication::globalStrut());
    TQSize bs = static_cast<KURLRequester*>( m_widget )->minimumSizeHint() - edit->minimumSizeHint();

    setIntrinsicWidth( s.width() + bs.width() );
    setIntrinsicHeight( kMax(s.height(), bs.height()) );

    RenderFormElement::calcMinMaxWidth();
}

void RenderFileButton::handleFocusOut()
{
    if ( widget()->lineEdit() && widget()->lineEdit()->edited() ) {
        element()->onChange();
        widget()->lineEdit()->setEdited( false );
    }
}

void RenderFileButton::updateFromElement()
{
    KLineEdit* edit = widget()->lineEdit();
    edit->blockSignals(true);
    edit->setText(element()->value().string());
    edit->blockSignals(false);
    edit->setEdited( false );

    RenderFormElement::updateFromElement();
}

void RenderFileButton::slotReturnPressed()
{
    handleFocusOut();

    if (element()->form())
	element()->form()->submitFromKeyboard();
}

void RenderFileButton::slotTextChanged(const TQString &/*string*/)
{
   element()->m_value = KURL( widget()->url() ).prettyURL( 0, KURL::StripFileProtocol );
}

void RenderFileButton::slotUrlSelected(const TQString &)
{
	element()->onChange();
}

void RenderFileButton::select()
{
    widget()->lineEdit()->selectAll();
}

// -------------------------------------------------------------------------

RenderLabel::RenderLabel(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{

}

// -------------------------------------------------------------------------

RenderLegend::RenderLegend(HTMLGenericFormElementImpl *element)
    : RenderBlock(element)
{
}

// -------------------------------------------------------------------------------

ComboBoxWidget::ComboBoxWidget(TQWidget *parent)
    : KComboBox(false, parent, "__tdehtml")
{
    setAutoMask(true);
    if (listBox()) listBox()->installEventFilter(this);
    setMouseTracking(true);
}

bool ComboBoxWidget::event(TQEvent *e)
{
    if (KComboBox::event(e))
	return true;
    if (e->type()==TQEvent::KeyPress)
    {
	TQKeyEvent *ke = static_cast<TQKeyEvent*>(e);
	switch(ke->key())
	{
	case Key_Return:
	case Key_Enter:
	    popup();
	    ke->accept();
	    return true;
	default:
	    return false;
	}
    }
    return false;
}

bool ComboBoxWidget::eventFilter(TQObject *dest, TQEvent *e)
{
    if (dest==listBox() &&  e->type()==TQEvent::KeyPress)
    {
	TQKeyEvent *ke = static_cast<TQKeyEvent*>(e);
	bool forward = false;
	switch(ke->key())
	{
	case Key_Tab:
	    forward=true;
	case Key_BackTab:
	    // ugly hack. emulate popdownlistbox() (private in TQComboBox)
	    // we re-use ke here to store the reference to the generated event.
	    ke = new TQKeyEvent(TQEvent::KeyPress, Key_Escape, 0, 0);
	    TQApplication::sendEvent(dest,ke);
	    focusNextPrevChild(forward);
	    delete ke;
	    return true;
	default:
	    return KComboBox::eventFilter(dest, e);
	}
    }
    return KComboBox::eventFilter(dest, e);
}

// -------------------------------------------------------------------------

RenderSelect::RenderSelect(HTMLSelectElementImpl *element)
    : RenderFormElement(element)
{
    m_ignoreSelectEvents = false;
    m_multiple = element->multiple();
    m_size = element->size();
    m_useListBox = (m_multiple || m_size > 1);
    m_selectionChanged = true;
    m_optionsChanged = true;

    if(m_useListBox)
        setQWidget(createListBox());
    else
        setQWidget(createComboBox());
}

void RenderSelect::updateFromElement()
{
    m_ignoreSelectEvents = true;

    // change widget type
    bool oldMultiple = m_multiple;
    unsigned oldSize = m_size;
    bool oldListbox = m_useListBox;

    m_multiple = element()->multiple();
    m_size = element()->size();
    m_useListBox = (m_multiple || m_size > 1);

    if (oldMultiple != m_multiple || oldSize != m_size) {
        if (m_useListBox != oldListbox) {
            // type of select has changed
            if(m_useListBox)
                setQWidget(createListBox());
            else
                setQWidget(createComboBox());
        }

        if (m_useListBox && oldMultiple != m_multiple) {
            static_cast<TDEListBox*>(m_widget)->setSelectionMode(m_multiple ? TQListBox::Extended : TQListBox::Single);
        }
        m_selectionChanged = true;
        m_optionsChanged = true;
    }

    // update contents listbox/combobox based on options in m_element
    if ( m_optionsChanged ) {
        if (element()->m_recalcListItems)
            element()->recalcListItems();
        TQMemArray<HTMLGenericFormElementImpl*> listItems = element()->listItems();
        int listIndex;

        if(m_useListBox) {
            static_cast<TDEListBox*>(m_widget)->clear();
        }

        else
            static_cast<KComboBox*>(m_widget)->clear();

        for (listIndex = 0; listIndex < int(listItems.size()); listIndex++) {
            if (listItems[listIndex]->id() == ID_OPTGROUP) {
                DOMString text = listItems[listIndex]->getAttribute(ATTR_LABEL);
                if (text.isNull())
                    text = "";

                if(m_useListBox) {
                    TQListBoxText *item = new TQListBoxText(TQString(text.implementation()->s, text.implementation()->l));
                    static_cast<TDEListBox*>(m_widget)
                        ->insertItem(item, listIndex);
                    item->setSelectable(false);
                }
                else {
                    static_cast<KComboBox*>(m_widget)
                        ->insertItem(TQString(text.implementation()->s, text.implementation()->l), listIndex);
		    static_cast<KComboBox*>(m_widget)->listBox()->item(listIndex)->setSelectable(false);
		}
            }
            else if (listItems[listIndex]->id() == ID_OPTION) {
                HTMLOptionElementImpl* optElem = static_cast<HTMLOptionElementImpl*>(listItems[listIndex]);
                TQString text = optElem->text().string();
                if (optElem->parentNode()->id() == ID_OPTGROUP)
                {
                    // Prefer label if set
                    DOMString label = optElem->getAttribute(ATTR_LABEL);
                    if (!label.isEmpty())
                        text = label.string();
                    text = TQString::fromLatin1("    ")+text;
                }

                if(m_useListBox) {
                    TDEListBox *l = static_cast<TDEListBox*>(m_widget);
                    l->insertItem(text, listIndex);
                    DOMString disabled = optElem->getAttribute(ATTR_DISABLED);
                    if (!disabled.isNull() && l->item( listIndex )) {
                        l->item( listIndex )->setSelectable( false );
                    }
                }  else
                    static_cast<KComboBox*>(m_widget)->insertItem(text, listIndex);
            }
            else
                TDEHTMLAssert(false);
            m_selectionChanged = true;
        }

        // TQComboBox caches the size hint unless you call setFont (ref: TT docu)
        if(!m_useListBox) {
            KComboBox *that = static_cast<KComboBox*>(m_widget);
            that->setFont( that->font() );
        }
        setNeedsLayoutAndMinMaxRecalc();
        m_optionsChanged = false;
    }

    // update selection
    if (m_selectionChanged) {
        updateSelection();
    }


    m_ignoreSelectEvents = false;

    RenderFormElement::updateFromElement();
}

void RenderSelect::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown() );

    if (m_optionsChanged)
        updateFromElement();

    // ### ugly HACK FIXME!!!
    setMinMaxKnown();
    layoutIfNeeded();
    setNeedsLayoutAndMinMaxRecalc();
    // ### end FIXME

    RenderFormElement::calcMinMaxWidth();
}

void RenderSelect::layout( )
{
    TDEHTMLAssert(needsLayout());
    TDEHTMLAssert(minMaxKnown());

    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)

    // calculate size
    if(m_useListBox) {
        TDEListBox* w = static_cast<TDEListBox*>(m_widget);

        TQListBoxItem* p = w->firstItem();
        int width = 0;
        int height = 0;
        while(p) {
            width = kMax(width, p->width(p->listBox()));
            height = kMax(height, p->height(p->listBox()));
            p = p->next();
        }
        if ( !height )
            height = w->fontMetrics().height();
        if ( !width )
            width = w->fontMetrics().width( 'x' );

        int size = m_size;
        // check if multiple and size was not given or invalid
        // Internet Exploder sets size to kMin(number of elements, 4)
        // Netscape seems to simply set it to "number of elements"
        // the average of that is IMHO kMin(number of elements, 10)
        // so I did that ;-)
        if(size < 1)
            size = kMin(static_cast<TDEListBox*>(m_widget)->count(), 10u);

        width += 2*w->frameWidth() + w->verticalScrollBar()->sizeHint().width();
        height = size*height + 2*w->frameWidth();

        setIntrinsicWidth( width );
        setIntrinsicHeight( height );
    }
    else {
        TQSize s(m_widget->sizeHint());
        setIntrinsicWidth( s.width() );
        setIntrinsicHeight( s.height() );
    }

    /// uuh, ignore the following line..
    setNeedsLayout(true);
    RenderFormElement::layout();

    // and now disable the widget in case there is no <option> given
    TQMemArray<HTMLGenericFormElementImpl*> listItems = element()->listItems();

    bool foundOption = false;
    for (uint i = 0; i < listItems.size() && !foundOption; i++)
	foundOption = (listItems[i]->id() == ID_OPTION);

    m_widget->setEnabled(foundOption && ! element()->disabled());
}

void RenderSelect::slotSelected(int index) // emitted by the combobox only
{
    if ( m_ignoreSelectEvents ) return;

    TDEHTMLAssert( !m_useListBox );

    TQMemArray<HTMLGenericFormElementImpl*> listItems = element()->listItems();
    if(index >= 0 && index < int(listItems.size()))
    {
        bool found = ( listItems[index]->id() == ID_OPTION );

        if ( !found ) {
            // this one is not selectable,  we need to find an option element
            while ( ( unsigned ) index < listItems.size() ) {
                if ( listItems[index]->id() == ID_OPTION ) {
                    found = true;
                    break;
                }
                ++index;
            }

            if ( !found ) {
                while ( index >= 0 ) {
                    if ( listItems[index]->id() == ID_OPTION ) {
                        found = true;
                        break;
                    }
                    --index;
                }
            }
        }

        if ( found ) {
            bool changed = false;

            for ( unsigned int i = 0; i < listItems.size(); ++i )
                if ( listItems[i]->id() == ID_OPTION && i != (unsigned int) index )
                {
                    HTMLOptionElementImpl* opt = static_cast<HTMLOptionElementImpl*>( listItems[i] );
                    changed |= (opt->m_selected == true);
                    opt->m_selected = false;
                }

            HTMLOptionElementImpl* opt = static_cast<HTMLOptionElementImpl*>(listItems[index]);
            changed |= (opt->m_selected == false);
            opt->m_selected = true;

            if ( index != static_cast<ComboBoxWidget*>( m_widget )->currentItem() )
                static_cast<ComboBoxWidget*>( m_widget )->setCurrentItem( index );

            // When selecting an optgroup item, and we move forward to we
            // shouldn't emit onChange. Hence this bool, the if above doesn't do it.
            if ( changed )
            {
		ref();
                element()->onChange();
                deref();
            }
        }
    }
}


void RenderSelect::slotSelectionChanged() // emitted by the listbox only
{
    if ( m_ignoreSelectEvents ) return;

    // don't use listItems() here as we have to avoid recalculations - changing the
    // option list will make use update options not in the way the user expects them
    TQMemArray<HTMLGenericFormElementImpl*> listItems = element()->m_listItems;
    for ( unsigned i = 0; i < listItems.count(); i++ )
        // don't use setSelected() here because it will cause us to be called
        // again with updateSelection.
        if ( listItems[i]->id() == ID_OPTION )
            static_cast<HTMLOptionElementImpl*>( listItems[i] )
                ->m_selected = static_cast<TDEListBox*>( m_widget )->isSelected( i );

    ref();
    element()->onChange();
    deref();
}

void RenderSelect::setOptionsChanged(bool _optionsChanged)
{
    m_optionsChanged = _optionsChanged;
}

TDEListBox* RenderSelect::createListBox()
{
    TDEListBox *lb = new TDEListBox(view()->viewport(), "__tdehtml");
    lb->setSelectionMode(m_multiple ? TQListBox::Extended : TQListBox::Single);
    // ### looks broken
    //lb->setAutoMask(true);
    connect( lb, TQ_SIGNAL( selectionChanged() ), this, TQ_SLOT( slotSelectionChanged() ) );
//     connect( lb, TQ_SIGNAL( clicked( TQListBoxItem * ) ), this, TQ_SLOT( slotClicked() ) );
    m_ignoreSelectEvents = false;
    lb->setMouseTracking(true);

    return lb;
}

ComboBoxWidget *RenderSelect::createComboBox()
{
    ComboBoxWidget *cb = new ComboBoxWidget(view()->viewport());
    connect(cb, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotSelected(int)));
    return cb;
}

void RenderSelect::updateSelection()
{
    TQMemArray<HTMLGenericFormElementImpl*> listItems = element()->listItems();
    int i;
    if (m_useListBox) {
        // if multi-select, we select only the new selected index
        TDEListBox *listBox = static_cast<TDEListBox*>(m_widget);
        for (i = 0; i < int(listItems.size()); i++)
            listBox->setSelected(i,listItems[i]->id() == ID_OPTION &&
                                 static_cast<HTMLOptionElementImpl*>(listItems[i])->selected());
    }
    else {
        bool found = false;
        unsigned firstOption = listItems.size();
        i = listItems.size();
        while (i--)
            if (listItems[i]->id() == ID_OPTION) {
                if (found)
                    static_cast<HTMLOptionElementImpl*>(listItems[i])->m_selected = false;
                else if (static_cast<HTMLOptionElementImpl*>(listItems[i])->selected()) {
                    static_cast<KComboBox*>( m_widget )->setCurrentItem(i);
                    found = true;
                }
                firstOption = i;
            }

        Q_ASSERT(firstOption == listItems.size() || found);
    }

    m_selectionChanged = false;
}


// -------------------------------------------------------------------------

TextAreaWidget::TextAreaWidget(int wrap, TQWidget* parent)
    : KTextEdit(parent, "__tdehtml"), m_findDlg(0), m_find(0), m_repDlg(0), m_replace(0)
{
    if(wrap != DOM::HTMLTextAreaElementImpl::ta_NoWrap) {
        setWordWrap(TQTextEdit::WidgetWidth);
        setHScrollBarMode( AlwaysOff );
        setVScrollBarMode( AlwaysOn );
    }
    else {
        setWordWrap(TQTextEdit::NoWrap);
        setHScrollBarMode( Auto );
        setVScrollBarMode( Auto );
    }
    KCursor::setAutoHideCursor(viewport(), true);
    setTextFormat(TQTextEdit::PlainText);
    setAutoMask(true);
    setMouseTracking(true);

    TDEActionCollection *ac = new TDEActionCollection(this);
    m_findAction = KStdAction::find( this, TQ_SLOT( slotFind() ), ac );
    m_findNextAction = KStdAction::findNext( this, TQ_SLOT( slotFindNext() ), ac );
    m_replaceAction = KStdAction::replace( this, TQ_SLOT( slotReplace() ), ac );
}


TextAreaWidget::~TextAreaWidget()
{
    delete m_replace;
    m_replace = 0L;
    delete m_find;
    m_find = 0L;
    delete m_repDlg;
    m_repDlg = 0L;
    delete m_findDlg;
    m_findDlg = 0L;
}


TQPopupMenu *TextAreaWidget::createPopupMenu(const TQPoint& pos)
{
    TQPopupMenu *popup = KTextEdit::createPopupMenu(pos);

    if ( !popup ) {
        return 0L;
    }

    if (!isReadOnly()) {
        popup->insertSeparator();

        m_findAction->plug(popup);
        m_findAction->setEnabled( !text().isEmpty() );

        m_findNextAction->plug(popup);
        m_findNextAction->setEnabled( m_find != 0 );

        m_replaceAction->plug(popup);
        m_replaceAction->setEnabled( !text().isEmpty() );
    }

    return popup;
}


void TextAreaWidget::slotFindHighlight(const TQString& text, int matchingIndex, int matchingLength)
{
    Q_UNUSED(text)
    //kdDebug() << "Highlight: [" << text << "] mi:" << matchingIndex << " ml:" << matchingLength << endl;
    if (sender() == m_replace) {
        setSelection(m_repPara, matchingIndex, m_repPara, matchingIndex + matchingLength);
        setCursorPosition(m_repPara, matchingIndex);
    } else {
        setSelection(m_findPara, matchingIndex, m_findPara, matchingIndex + matchingLength);
        setCursorPosition(m_findPara, matchingIndex);
    }
    ensureCursorVisible();
}


void TextAreaWidget::slotReplaceText(const TQString &text, int replacementIndex, int /*replacedLength*/, int matchedLength) {
    Q_UNUSED(text)
    //kdDebug() << "Replace: [" << text << "] ri:" << replacementIndex << " rl:" << replacedLength << " ml:" << matchedLength << endl;
    setSelection(m_repPara, replacementIndex, m_repPara, replacementIndex + matchedLength);
    removeSelectedText();
    insertAt(m_repDlg->replacement(), m_repPara, replacementIndex);
    if (m_replace->options() & KReplaceDialog::PromptOnReplace) {
        ensureCursorVisible();
    }
}


void TextAreaWidget::slotDoReplace()
{
    if (!m_repDlg) {
        // Should really assert()
        return;
    }

    delete m_replace;
    m_replace = new KReplace(m_repDlg->pattern(), m_repDlg->replacement(), m_repDlg->options(), this);
    if (m_replace->options() & KFindDialog::FromCursor) {
        getCursorPosition(&m_repPara, &m_repIndex);
    } else if (m_replace->options() & KFindDialog::FindBackwards) {
        m_repPara = paragraphs() - 1;
        m_repIndex = paragraphLength(m_repPara) - 1;
    } else {
        m_repPara = 0;
        m_repIndex = 0;
    }

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(m_replace, TQ_SIGNAL(highlight(const TQString &, int, int)),
            this, TQ_SLOT(slotFindHighlight(const TQString &, int, int)));
    connect(m_replace, TQ_SIGNAL(findNext()), this, TQ_SLOT(slotReplaceNext()));
    connect(m_replace, TQ_SIGNAL(replace(const TQString &, int, int, int)),
            this, TQ_SLOT(slotReplaceText(const TQString &, int, int, int)));

    m_repDlg->close();
    slotReplaceNext();
}


void TextAreaWidget::slotReplaceNext()
{
    if (!m_replace) {
        // assert?
        return;
    }

    if (!(m_replace->options() & KReplaceDialog::PromptOnReplace)) {
        viewport()->setUpdatesEnabled(false);
    }

    KFind::Result res = KFind::NoMatch;
    while (res == KFind::NoMatch) {
        // If we're done.....
        if (m_replace->options() & KFindDialog::FindBackwards) {
            if (m_repIndex == 0 && m_repPara == 0) {
                break;
            }
        } else {
            if (m_repPara == paragraphs() - 1 &&
                m_repIndex == paragraphLength(m_repPara) - 1) {
                break;
            }
        }

        if (m_replace->needData()) {
            m_replace->setData(text(m_repPara), m_repIndex);
        }

        res = m_replace->replace();

        if (res == KFind::NoMatch) {
            if (m_replace->options() & KFindDialog::FindBackwards) {
                if (m_repPara == 0) {
                    m_repIndex = 0;
                } else {
                    m_repPara--;
                    m_repIndex = paragraphLength(m_repPara) - 1;
                }
            } else {
                if (m_repPara == paragraphs() - 1) {
                    m_repIndex = paragraphLength(m_repPara) - 1;
                } else {
                    m_repPara++;
                    m_repIndex = 0;
                }
            }
        }
    }

    if (!(m_replace->options() & KReplaceDialog::PromptOnReplace)) {
        viewport()->setUpdatesEnabled(true);
        repaintChanged();
    }

    if (res == KFind::NoMatch) { // at end
        m_replace->displayFinalDialog();
        delete m_replace;
        m_replace = 0;
        ensureCursorVisible();
        //or           if ( m_replace->shouldRestart() ) { reinit (w/o FromCursor) and call slotReplaceNext(); }
    } else {
        //m_replace->closeReplaceNextDialog();
    }
}


void TextAreaWidget::slotDoFind()
{
    if (!m_findDlg) {
        // Should really assert()
        return;
    }

    delete m_find;
    m_find = new KFind(m_findDlg->pattern(), m_findDlg->options(), this);
    if (m_find->options() & KFindDialog::FromCursor) {
        getCursorPosition(&m_findPara, &m_findIndex);
    } else if (m_find->options() & KFindDialog::FindBackwards) {
        m_findPara = paragraphs() - 1;
        m_findIndex = paragraphLength(m_findPara) - 1;
    } else {
        m_findPara = 0;
        m_findIndex = 0;
    }

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(m_find, TQ_SIGNAL(highlight(const TQString &, int, int)),
            this, TQ_SLOT(slotFindHighlight(const TQString &, int, int)));
    connect(m_find, TQ_SIGNAL(findNext()), this, TQ_SLOT(slotFindNext()));

    m_findDlg->close();
    m_find->closeFindNextDialog();
    slotFindNext();
}


void TextAreaWidget::slotFindNext()
{
    if (!m_find) {
        // assert?
        return;
    }

    KFind::Result res = KFind::NoMatch;
    while (res == KFind::NoMatch) {
        // If we're done.....
        if (m_find->options() & KFindDialog::FindBackwards) {
            if (m_findIndex == 0 && m_findPara == 0) {
                break;
            }
        } else {
            if (m_findPara == paragraphs() - 1 &&
                m_findIndex == paragraphLength(m_findPara) - 1) {
                break;
            }
        }

        if (m_find->needData()) {
            m_find->setData(text(m_findPara), m_findIndex);
        }

        res = m_find->find();

        if (res == KFind::NoMatch) {
            if (m_find->options() & KFindDialog::FindBackwards) {
                if (m_findPara == 0) {
                    m_findIndex = 0;
                } else {
                    m_findPara--;
                    m_findIndex = paragraphLength(m_findPara) - 1;
                }
            } else {
                if (m_findPara == paragraphs() - 1) {
                    m_findIndex = paragraphLength(m_findPara) - 1;
                } else {
                    m_findPara++;
                    m_findIndex = 0;
                }
            }
        }
    }

    if (res == KFind::NoMatch) { // at end
        m_find->displayFinalDialog();
        delete m_find;
        m_find = 0;
        //or           if ( m_find->shouldRestart() ) { reinit (w/o FromCursor) and call slotFindNext(); }
    } else {
        //m_find->closeFindNextDialog();
    }
}


void TextAreaWidget::slotFind()
{
    if( text().isEmpty() )  // saves having to track the text changes
        return;

    if ( m_findDlg ) {
      KWin::activateWindow( m_findDlg->winId() );
    } else {
      m_findDlg = new KFindDialog(false, this, "TDEHTML Text Area Find Dialog");
      connect( m_findDlg, TQ_SIGNAL(okClicked()), this, TQ_SLOT(slotDoFind()) );
    }
    m_findDlg->show();
}


void TextAreaWidget::slotReplace()
{
    if( text().isEmpty() )  // saves having to track the text changes
        return;

    if ( m_repDlg ) {
      KWin::activateWindow( m_repDlg->winId() );
    } else {
      m_repDlg = new KReplaceDialog(this, "TDEHTMLText Area Replace Dialog", 0,
                                    TQStringList(), TQStringList(), false);
      connect( m_repDlg, TQ_SIGNAL(okClicked()), this, TQ_SLOT(slotDoReplace()) );
    }
    m_repDlg->show();
}


bool TextAreaWidget::event( TQEvent *e )
{
    if ( e->type() == TQEvent::AccelAvailable && isReadOnly() ) {
        TQKeyEvent* ke = (TQKeyEvent*) e;
        if ( ke->state() & ControlButton ) {
            switch ( ke->key() ) {
                case Key_Left:
                case Key_Right:
                case Key_Up:
                case Key_Down:
                case Key_Home:
                case Key_End:
                    ke->accept();
                default:
                break;
            }
        }
    }
    return KTextEdit::event( e );
}

// -------------------------------------------------------------------------

RenderTextArea::RenderTextArea(HTMLTextAreaElementImpl *element)
    : RenderFormElement(element)
{
    scrollbarsStyled = false;

    TextAreaWidget *edit = new TextAreaWidget(element->wrap(), view());
    setQWidget(edit);
    const TDEHTMLSettings *settings = view()->part()->settings();
    edit->setCheckSpellingEnabled( settings->autoSpellCheck() );
    edit->setTabChangesFocus( ! settings->allowTabulation() );

    connect(edit,TQ_SIGNAL(textChanged()),this,TQ_SLOT(slotTextChanged()));
}

RenderTextArea::~RenderTextArea()
{
    if ( element()->m_dirtyvalue ) {
        element()->m_value = text();
        element()->m_dirtyvalue = false;
    }
}

void RenderTextArea::handleFocusOut()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    if ( w && element()->m_dirtyvalue ) {
        element()->m_value = text();
        element()->m_dirtyvalue = false;
    }

    if ( w && element()->m_changed ) {
        element()->m_changed = false;
        element()->onChange();
    }
}

void RenderTextArea::calcMinMaxWidth()
{
    TDEHTMLAssert( !minMaxKnown() );

    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    const TQFontMetrics &m = style()->fontMetrics();
    w->setTabStopWidth(8 * m.width(" "));
    TQSize size( kMax(element()->cols(), 1L)*m.width('x') + w->frameWidth() +
                w->verticalScrollBar()->sizeHint().width(),
                kMax(element()->rows(), 1L)*m.lineSpacing() + w->frameWidth()*4 +
                (w->wordWrap() == TQTextEdit::NoWrap ?
                 w->horizontalScrollBar()->sizeHint().height() : 0)
        );

    setIntrinsicWidth( size.width() );
    setIntrinsicHeight( size.height() );

    RenderFormElement::calcMinMaxWidth();
}

void RenderTextArea::setStyle(RenderStyle* _style)
{
    bool unsubmittedFormChange = element()->m_unsubmittedFormChange;

    RenderFormElement::setStyle(_style);

    widget()->blockSignals(true);
    widget()->setAlignment(textAlignment());
    widget()->blockSignals(false);

    scrollbarsStyled = false;

    element()->m_unsubmittedFormChange = unsubmittedFormChange;
}

void RenderTextArea::layout()
{
    TDEHTMLAssert( needsLayout() );

    RenderFormElement::layout();

    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);

    if (!scrollbarsStyled) {
        w->horizontalScrollBar()->setPalette(style()->palette());
        w->verticalScrollBar()->setPalette(style()->palette());
        scrollbarsStyled=true;
    }
}

void RenderTextArea::updateFromElement()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    w->setReadOnly(element()->readOnly());
    TQString elementText = element()->value().string();
    if ( elementText != text() )
    {
        w->blockSignals(true);
        int line, col;
        w->getCursorPosition( &line, &col );
        int cx = w->contentsX();
        int cy = w->contentsY();
        w->setText( elementText );
        w->setCursorPosition( line, col );
        w->scrollBy( cx, cy );
        w->blockSignals(false);
    }
    element()->m_dirtyvalue = false;

    RenderFormElement::updateFromElement();
}

void RenderTextArea::close( )
{
    element()->setValue( element()->defaultValue() );

    RenderFormElement::close();
}


TQString RenderTextArea::text()
{
    TQString txt;
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);

    if(element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical) {
        // yeah, TQTextEdit has no accessor for getting the visually wrapped text
        for (int p=0; p < w->paragraphs(); ++p) {
            int ll = 0;
            int lindex = w->lineOfChar(p, 0);
            TQString paragraphText = w->text(p);
            int pl = w->paragraphLength(p);
            paragraphText = paragraphText.left(pl); //Snip invented space.
            for (int l = 0; l < pl; ++l) {
                if (lindex != w->lineOfChar(p, l)) {
                    paragraphText.insert(l+ll++, TQString::fromLatin1("\n"));
                    lindex = w->lineOfChar(p, l);
                }
            }
            txt += paragraphText;
            if (p < w->paragraphs() - 1)
                txt += TQString::fromLatin1("\n");
        }
    }
    else
        txt = w->text();

    return txt;
}

int RenderTextArea::queryParagraphInfo(int para, Mode m, int param) {
    /* We have to be a bit careful here, as we need to match up the positions
    to what our value returns here*/
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    int        length = 0;

    bool physWrap     = element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical;

    TQString paragraphText = w->text(para);
    int pl                = w->paragraphLength(para);
    int physicalPL        = pl;
    if (m == ParaPortionLength)
        pl = param;

    if (physWrap) {
        //Go through all the chars of paragraph, and count line changes, chars, etc.
        int lindex = w->lineOfChar(para, 0);
        for (int c = 0; c < pl; ++c) {
            ++length;
            // Is there a change after this char?
            if (c+1 < physicalPL && lindex != w->lineOfChar(para, c+1)) {
                lindex =  w->lineOfChar(para, c+1);
                ++length;
            }
            if (m == ParaPortionOffset && length > param)
                return c;
        }
    } else {
        //Make sure to count the LF, CR as appropriate. ### this is stupid now, simplify
        for (int c = 0; c < pl; ++c) {
            ++length;
            if (m == ParaPortionOffset && length > param)
                return c;
        }
    }
    if (m == ParaPortionOffset)
        return pl;
    if (m == ParaPortionLength)
        return length;
    return length + 1;
}

long RenderTextArea::computeCharOffset(int para, int index) {
    if (para < 0)
        return 0;

    long pos = 0;
    for (int cp = 0; cp < para; ++cp)
        pos += queryParagraphInfo(cp, ParaLength);

    if (index >= 0)
        pos += queryParagraphInfo(para, ParaPortionLength, index);
    return pos;
}

void RenderTextArea::computeParagraphAndIndex(long offset, int* para, int* index) {
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);

    if (!w->paragraphs()) {
        *para  = -1;
        *index = -1;
        return;
    }

    //Find the paragraph that contains us..
    int containingPar = 0;
    long endPos       = 0;
    long startPos     = 0;
    for (int p = 0; p < w->paragraphs(); ++p) {
        int len = queryParagraphInfo(p, ParaLength);
        endPos += len;
        if (endPos > offset) {
            containingPar = p;
            break;
        }
        startPos += len;
    }

    *para = containingPar;

    //Now, scan within the paragraph to find the position..
    long localOffset = offset - startPos;

    *index = queryParagraphInfo(containingPar, ParaPortionOffset, localOffset);
}

void RenderTextArea::highLightWord( unsigned int length, unsigned int pos )
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    if ( w )
        w->highLightWord( length, pos );
}


void RenderTextArea::slotTextChanged()
{
    element()->m_dirtyvalue = true;
    element()->m_changed    = true;
    if (element()->m_value != text())
        element()->m_unsubmittedFormChange = true;
}

void RenderTextArea::select()
{
    static_cast<TextAreaWidget *>(m_widget)->selectAll();
}

long RenderTextArea::selectionStart()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    int para, index, dummy1, dummy2;
    w->getSelection(&para, &index, &dummy1, &dummy2);
    if (para == -1 || index == -1)
        w->getCursorPosition(&para, &index);

    return computeCharOffset(para, index);
}

long RenderTextArea::selectionEnd()
{
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    int para, index, dummy1, dummy2;
    w->getSelection(&dummy1, &dummy2, &para, &index);
    if (para == -1 || index == -1)
        w->getCursorPosition(&para, &index);

    return computeCharOffset(para, index);
}

void RenderTextArea::setSelectionStart(long offset) {
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    int fromPara, fromIndex, toPara, toIndex;
    w->getSelection(&fromPara, &fromIndex, &toPara, &toIndex);
    computeParagraphAndIndex(offset, &fromPara, &fromIndex);
    if (toPara == -1 || toIndex == -1) {
        toPara  = fromPara;
        toIndex = fromIndex;
    }
    w->setSelection(fromPara, fromIndex, toPara, toIndex);
}

void RenderTextArea::setSelectionEnd(long offset) {
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    int fromPara, fromIndex, toPara, toIndex;
    w->getSelection(&fromPara, &fromIndex, &toPara, &toIndex);
    computeParagraphAndIndex(offset, &toPara, &toIndex);
    w->setSelection(fromPara, fromIndex, toPara, toIndex);
}

void RenderTextArea::setSelectionRange(long start, long end) {
    TextAreaWidget* w = static_cast<TextAreaWidget*>(m_widget);
    int fromPara, fromIndex, toPara, toIndex;
    computeParagraphAndIndex(start, &fromPara, &fromIndex);
    computeParagraphAndIndex(end,   &toPara,   &toIndex);
    w->setSelection(fromPara, fromIndex, toPara, toIndex);
}
// ---------------------------------------------------------------------------

#include "render_form.moc"
