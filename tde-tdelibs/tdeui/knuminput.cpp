/*
 * knuminput.cpp
 *
 * Initial implementation:
 *     Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 * Rewritten and maintained by:
 *     Copyright (c) 2000 Dirk A. Mueller <mueller@kde.org>
 * KDoubleSpinBox:
 *     Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 *
 *  Requires the Qt widget libraries, available at no cost at
 *  http://www.troll.no/
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

#include <config.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <assert.h>
#include <math.h>
#include <algorithm>

#include <tqapplication.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqsize.h>
#include <tqslider.h>
#include <tqspinbox.h>
#include <tqstyle.h>

#include <tdeglobal.h>
#include <tdelocale.h>
#include <kdebug.h>

#include "kdialog.h"
#include "knumvalidator.h"
#include "knuminput.h"

static inline int calcDiffByTen( int x, int y ) {
    // calculate ( x - y ) / 10 without overflowing ints:
    return ( x / 10 ) - ( y / 10 )  +  ( x % 10 - y % 10 ) / 10;
}

// ----------------------------------------------------------------------------

KNumInput::KNumInput(TQWidget* parent, const char* name)
    : TQWidget(parent, name)
{
    init();
}

KNumInput::KNumInput(KNumInput* below, TQWidget* parent, const char* name)
    : TQWidget(parent, name)
{
    init();

    if(below) {
        m_next = below->m_next;
        m_prev = below;
        below->m_next = this;
        if(m_next)
            m_next->m_prev = this;
    }
}

void KNumInput::init()
{
    m_prev = m_next = 0;
    m_colw1 = m_colw2 = 0;

    m_label = 0;
    m_slider = 0;
    m_alignment = 0;
}

KNumInput::~KNumInput()
{
    if(m_prev)
        m_prev->m_next = m_next;

    if(m_next)
        m_next->m_prev = m_prev;
}

void KNumInput::setLabel(const TQString & label, int a)
{
    if(label.isEmpty()) {
        delete m_label;
        m_label = 0;
        m_alignment = 0;
    }
    else {
        if (m_label) m_label->setText(label);
        else m_label = new TQLabel(label, this, "KNumInput::TQLabel");
        m_label->setAlignment((a & (~(AlignTop|AlignBottom|AlignVCenter)))
                              | AlignVCenter);
        // if no vertical alignment set, use Top alignment
        if(!(a & (AlignTop|AlignBottom|AlignVCenter)))
           a |= AlignTop;
        m_alignment = a;
    }

    layout(true);
}

TQString KNumInput::label() const
{
    if (m_label) return m_label->text();
    return TQString::null;
}

void KNumInput::layout(bool deep)
{
    int w1 = m_colw1;
    int w2 = m_colw2;

    // label sizeHint
    m_sizeLabel = (m_label ? m_label->sizeHint() : TQSize(0,0));

    if(m_label && (m_alignment & AlignVCenter))
        m_colw1 = m_sizeLabel.width() + 4;
    else
        m_colw1 = 0;

    // slider sizeHint
    m_sizeSlider = (m_slider ? m_slider->sizeHint() : TQSize(0, 0));

    doLayout();

    if(!deep) {
        m_colw1 = w1;
        m_colw2 = w2;
        return;
    }

    KNumInput* p = this;
    while(p) {
        p->doLayout();
        w1 = TQMAX(w1, p->m_colw1);
        w2 = TQMAX(w2, p->m_colw2);
        p = p->m_prev;
    }

    p = m_next;
    while(p) {
        p->doLayout();
        w1 = TQMAX(w1, p->m_colw1);
        w2 = TQMAX(w2, p->m_colw2);
        p = p->m_next;
    }

    p = this;
    while(p) {
        p->m_colw1 = w1;
        p->m_colw2 = w2;
        p = p->m_prev;
    }

    p = m_next;
    while(p) {
        p->m_colw1 = w1;
        p->m_colw2 = w2;
        p = p->m_next;
    }

//    kdDebug() << "w1 " << w1 << " w2 " << w2 << endl;
}

TQSizePolicy KNumInput::sizePolicy() const
{
    return TQSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Fixed );
}

TQSize KNumInput::sizeHint() const
{
    return minimumSizeHint();
}

void KNumInput::setSteps(int minor, int major)
{
    if(m_slider)
        m_slider->setSteps( minor, major );
}


// ----------------------------------------------------------------------------

KIntSpinBox::KIntSpinBox(TQWidget *parent, const char *name)
    : TQSpinBox(0, 99, 1, parent, name)
{
    editor()->setAlignment(AlignRight);
    val_base = 10;
    setValidator(new KIntValidator(this, val_base));
    setValue(0);
}

KIntSpinBox::~KIntSpinBox()
{
}

KIntSpinBox::KIntSpinBox(int lower, int upper, int step, int value, int base,
                         TQWidget* parent, const char* name)
    : TQSpinBox(lower, upper, step, parent, name)
{
    editor()->setAlignment(AlignRight);
    val_base = base;
    setValidator(new KIntValidator(this, val_base));
    setValue(value);
}

void KIntSpinBox::setBase(int base)
{
    const KIntValidator* kvalidator = dynamic_cast<const KIntValidator*>(validator());
    if (kvalidator) {
    	const_cast<KIntValidator*>(kvalidator)->setBase(base);
    }
    val_base = base;
}


int KIntSpinBox::base() const
{
    return val_base;
}

TQString KIntSpinBox::mapValueToText(int v)
{
    return TQString::number(v, val_base);
}

int KIntSpinBox::mapTextToValue(bool* ok)
{
    return cleanText().toInt(ok, val_base);
}

void KIntSpinBox::setEditFocus(bool mark)
{
    editor()->setFocus();
    if(mark)
        editor()->selectAll();
}


// ----------------------------------------------------------------------------

class KIntNumInput::KIntNumInputPrivate {
public:
    int referencePoint;
    short blockRelative;
    KIntNumInputPrivate( int r )
	: referencePoint( r ),
	  blockRelative( 0 ) {}
};


KIntNumInput::KIntNumInput(KNumInput* below, int val, TQWidget* parent,
                           int _base, const char* name)
    : KNumInput(below, parent, name)
{
    init(val, _base);
}

KIntNumInput::KIntNumInput(TQWidget *parent, const char *name)
    : KNumInput(parent, name)
{
    init(0, 10);
}

KIntNumInput::KIntNumInput(int val, TQWidget *parent, int _base, const char *name)
    : KNumInput(parent, name)
{
    init(val, _base);

}

void KIntNumInput::init(int val, int _base)
{
    d = new KIntNumInputPrivate( val );
    m_spin = new KIntSpinBox(INT_MIN, INT_MAX, 1, val, _base, this, "KIntNumInput::KIntSpinBox");
    // the KIntValidator is broken beyond believe for
    // spinboxes which have suffix or prefix texts, so
    // better don't use it unless absolutely necessary
    if (_base != 10)
        m_spin->setValidator(new KIntValidator(this, _base, "KNumInput::KIntValidtr"));

    connect(m_spin, TQ_SIGNAL(valueChanged(int)), TQ_SLOT(spinValueChanged(int)));
    connect(this, TQ_SIGNAL(valueChanged(int)),
	    TQ_SLOT(slotEmitRelativeValueChanged(int)));

    setFocusProxy(m_spin);
    layout(true);
}

void KIntNumInput::setReferencePoint( int ref ) {
    // clip to valid range:
    ref = kMin( maxValue(), kMax( minValue(),  ref ) );
    d->referencePoint = ref;
}

int KIntNumInput::referencePoint() const {
    return d->referencePoint;
}

void KIntNumInput::spinValueChanged(int val)
{
    if(m_slider)
        m_slider->setValue(val);

    emit valueChanged(val);
}

void KIntNumInput::slotEmitRelativeValueChanged( int value ) {
    if ( d->blockRelative || !d->referencePoint ) return;
    emit relativeValueChanged( double( value ) / double( d->referencePoint ) );
}

void KIntNumInput::setRange(int lower, int upper, int step, bool slider)
{
    upper = kMax(upper, lower);
    lower = kMin(upper, lower);
    m_spin->setMinValue(lower);
    m_spin->setMaxValue(upper);
    m_spin->setLineStep(step);

    step = m_spin->lineStep(); // maybe TQRangeControl didn't like out lineStep?

    if(slider) {
	if (m_slider)
	    m_slider->setRange(lower, upper);
	else {
	    m_slider = new TQSlider(lower, upper, step, m_spin->value(),
				   TQt::Horizontal, this);
	    m_slider->setTickmarks(TQSlider::Below);
	    connect(m_slider, TQ_SIGNAL(valueChanged(int)),
		    m_spin, TQ_SLOT(setValue(int)));
	}

	// calculate (upper-lower)/10 without overflowing int's:
        int major = calcDiffByTen( upper, lower );
	if ( major==0 ) major = step; // #### workaround Qt bug in 2.1-beta4

        m_slider->setSteps(step, major);
        m_slider->setTickInterval(major);
    }
    else {
        delete m_slider;
        m_slider = 0;
    }

    // check that reference point is still inside valid range:
    setReferencePoint( referencePoint() );

    layout(true);
}

void KIntNumInput::setMinValue(int min)
{
    setRange(min, m_spin->maxValue(), m_spin->lineStep(), m_slider);
}

int KIntNumInput::minValue() const
{
    return m_spin->minValue();
}

void KIntNumInput::setMaxValue(int max)
{
    setRange(m_spin->minValue(), max, m_spin->lineStep(), m_slider);
}

int KIntNumInput::maxValue() const
{
    return m_spin->maxValue();
}

void KIntNumInput::setSuffix(const TQString &suffix)
{
    m_spin->setSuffix(suffix);

    layout(true);
}

TQString KIntNumInput::suffix() const
{
    return m_spin->suffix();
}

void KIntNumInput::setPrefix(const TQString &prefix)
{
    m_spin->setPrefix(prefix);

    layout(true);
}

TQString KIntNumInput::prefix() const
{
    return m_spin->prefix();
}

void KIntNumInput::setEditFocus(bool mark)
{
    m_spin->setEditFocus(mark);
}

TQSize KIntNumInput::minimumSizeHint() const
{
    constPolish();

    int w;
    int h;

    h = 2 + TQMAX(m_sizeSpin.height(), m_sizeSlider.height());

    // if in extra row, then count it here
    if(m_label && (m_alignment & (AlignBottom|AlignTop)))
        h += 4 + m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
        h = TQMAX(h, m_sizeLabel.height() + 2);

    w = m_slider ? m_slider->sizeHint().width() + 8 : 0;
    w += m_colw1 + m_colw2;

    if(m_alignment & (AlignTop|AlignBottom))
        w = TQMAX(w, m_sizeLabel.width() + 4);

    return TQSize(w, h);
}

void KIntNumInput::doLayout()
{
    m_sizeSpin = m_spin->sizeHint();
    m_colw2 = m_sizeSpin.width();

    if (m_label)
        m_label->setBuddy(m_spin);
}

void KIntNumInput::resizeEvent(TQResizeEvent* e)
{
    int w = m_colw1;
    int h = 0;

    if(m_label && (m_alignment & AlignTop)) {
        m_label->setGeometry(0, 0, e->size().width(), m_sizeLabel.height());
        h += m_sizeLabel.height() + KDialog::spacingHint();
    }

    if(m_label && (m_alignment & AlignVCenter))
        m_label->setGeometry(0, 0, w, m_sizeSpin.height());

    if (tqApp->reverseLayout())
    {
        m_spin->setGeometry(w, h, m_slider ? m_colw2 : TQMAX(m_colw2, e->size().width() - w), m_sizeSpin.height());
        w += m_colw2 + 8;

        if(m_slider)
            m_slider->setGeometry(w, h, e->size().width() - w, m_sizeSpin.height());
    }
    else if(m_slider) {
        m_slider->setGeometry(w, h, e->size().width() - (w + m_colw2 + KDialog::spacingHint()), m_sizeSpin.height());
        m_spin->setGeometry(w + m_slider->size().width() + KDialog::spacingHint(), h, m_colw2, m_sizeSpin.height());
    }
    else {
        m_spin->setGeometry(w, h, TQMAX(m_colw2, e->size().width() - w), m_sizeSpin.height());
    }

    h += m_sizeSpin.height() + 2;

    if(m_label && (m_alignment & AlignBottom))
        m_label->setGeometry(0, h, m_sizeLabel.width(), m_sizeLabel.height());
}

KIntNumInput::~KIntNumInput()
{
	delete d;
}

void KIntNumInput::setValue(int val)
{
    m_spin->setValue(val);
    // slider value is changed by spinValueChanged
}

void KIntNumInput::setRelativeValue( double r ) {
    if ( !d->referencePoint ) return;
    ++d->blockRelative;
    setValue( int( d->referencePoint * r + 0.5 ) );
    --d->blockRelative;
}

double KIntNumInput::relativeValue() const {
    if ( !d->referencePoint ) return 0;
    return double( value() ) / double ( d->referencePoint );
}

int  KIntNumInput::value() const
{
    return m_spin->value();
}

void KIntNumInput::setSpecialValueText(const TQString& text)
{
    m_spin->setSpecialValueText(text);
    layout(true);
}

TQString KIntNumInput::specialValueText() const
{
    return m_spin->specialValueText();
}

void KIntNumInput::setLabel(const TQString & label, int a)
{
    KNumInput::setLabel(label, a);

    if(m_label)
        m_label->setBuddy(m_spin);
}

// ----------------------------------------------------------------------------

class KDoubleNumInput::KDoubleNumInputPrivate {
public:
    KDoubleNumInputPrivate( double r )
	: spin( 0 ),
	  referencePoint( r ),
	  blockRelative ( 0 ) {}
    KDoubleSpinBox * spin;
    double referencePoint;
    short blockRelative;
};

KDoubleNumInput::KDoubleNumInput(TQWidget *parent, const char *name)
    : KNumInput(parent, name)
{
    init(0.0, 0.0, 9999.0, 0.01, 2);
}

KDoubleNumInput::KDoubleNumInput(double lower, double upper, double value,
				 double step, int precision, TQWidget* parent,
				 const char *name)
    : KNumInput(parent, name)
{
    init(value, lower, upper, step, precision);
}

KDoubleNumInput::KDoubleNumInput(KNumInput *below,
				 double lower, double upper, double value,
				 double step, int precision, TQWidget* parent,
				 const char *name)
    : KNumInput(below, parent, name)
{
    init(value, lower, upper, step, precision);
}

KDoubleNumInput::KDoubleNumInput(double value, TQWidget *parent, const char *name)
    : KNumInput(parent, name)
{
    init(value, kMin(0.0, value), kMax(0.0, value), 0.01, 2 );
}

KDoubleNumInput::KDoubleNumInput(KNumInput* below, double value, TQWidget* parent,
                                 const char* name)
    : KNumInput(below, parent, name)
{
    init( value, kMin(0.0, value), kMax(0.0, value), 0.01, 2 );
}

KDoubleNumInput::~KDoubleNumInput()
{
	delete d;
}

// ### remove when BIC changes are allowed again:

bool KDoubleNumInput::eventFilter( TQObject * o, TQEvent * e ) {
    return KNumInput::eventFilter( o, e );
}

void KDoubleNumInput::resetEditBox() {

}

// ### end stuff to remove when BIC changes are allowed again



void KDoubleNumInput::init(double value, double lower, double upper,
			   double step, int precision )
{
    // ### init no longer used members:
    edit = 0;
    m_range = true;
    m_value = 0.0;
    m_precision = 2;
    // ### end

    d = new KDoubleNumInputPrivate( value );

    d->spin = new KDoubleSpinBox( lower, upper, step, value, precision,
				  this, "KDoubleNumInput::d->spin" );
    setFocusProxy(d->spin);
    connect( d->spin, TQ_SIGNAL(valueChanged(double)),
	     this, TQ_SIGNAL(valueChanged(double)) );
    connect( this, TQ_SIGNAL(valueChanged(double)),
	     this, TQ_SLOT(slotEmitRelativeValueChanged(double)) );

    updateLegacyMembers();

    layout(true);
}

void KDoubleNumInput::updateLegacyMembers() {
    // ### update legacy members that are either not private or for
    // which an inlined getter exists:
    m_lower = minValue();
    m_upper = maxValue();
    m_step = d->spin->lineStep();
    m_specialvalue = specialValueText();
}


double KDoubleNumInput::mapSliderToSpin( int val ) const
{
    // map [slidemin,slidemax] to [spinmin,spinmax]
    double spinmin = d->spin->minValue();
    double spinmax = d->spin->maxValue();
    double slidemin = m_slider->minValue(); // cast int to double to avoid
    double slidemax = m_slider->maxValue(); // overflow in rel denominator
    double rel = ( double(val) - slidemin ) / ( slidemax - slidemin );
    return spinmin + rel * ( spinmax - spinmin );
}

void KDoubleNumInput::sliderMoved(int val)
{
    d->spin->setValue( mapSliderToSpin( val ) );
}

void KDoubleNumInput::slotEmitRelativeValueChanged( double value )
{
    if ( !d->referencePoint ) return;
    emit relativeValueChanged( value / d->referencePoint );
}

TQSize KDoubleNumInput::minimumSizeHint() const
{
    constPolish();

    int w;
    int h;

    h = 2 + TQMAX(m_sizeEdit.height(), m_sizeSlider.height());

    // if in extra row, then count it here
    if(m_label && (m_alignment & (AlignBottom|AlignTop)))
        h += 4 + m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
	h = TQMAX(h, m_sizeLabel.height() + 2);

    w = m_slider ? m_slider->sizeHint().width() + 8 : 0;
    w += m_colw1 + m_colw2;

    if(m_alignment & (AlignTop|AlignBottom))
        w = TQMAX(w, m_sizeLabel.width() + 4);

    return TQSize(w, h);
}

void KDoubleNumInput::resizeEvent(TQResizeEvent* e)
{
    int w = m_colw1;
    int h = 0;

    if(m_label && (m_alignment & AlignTop)) {
        m_label->setGeometry(0, 0, e->size().width(), m_sizeLabel.height());
        h += m_sizeLabel.height() + 4;
    }

    if(m_label && (m_alignment & AlignVCenter))
        m_label->setGeometry(0, 0, w, m_sizeEdit.height());

    if (tqApp->reverseLayout())
    {
        d->spin->setGeometry(w, h, m_slider ? m_colw2
                                            : e->size().width() - w, m_sizeEdit.height());
        w += m_colw2 + KDialog::spacingHint();

        if(m_slider)
            m_slider->setGeometry(w, h, e->size().width() - w, m_sizeEdit.height());
    }
    else if(m_slider) {
        m_slider->setGeometry(w, h, e->size().width() -
                                    (m_colw1 + m_colw2 + KDialog::spacingHint()),
                              m_sizeEdit.height());
        d->spin->setGeometry(w + m_slider->width() + KDialog::spacingHint(), h,
                             m_colw2, m_sizeEdit.height());
    }
    else {
        d->spin->setGeometry(w, h, e->size().width() - w, m_sizeEdit.height());
    }

    h += m_sizeEdit.height() + 2;

    if(m_label && (m_alignment & AlignBottom))
        m_label->setGeometry(0, h, m_sizeLabel.width(), m_sizeLabel.height());
}

void KDoubleNumInput::doLayout()
{
    m_sizeEdit = d->spin->sizeHint();
    m_colw2 = m_sizeEdit.width();
}

void KDoubleNumInput::setValue(double val)
{
    d->spin->setValue( val );
}

void KDoubleNumInput::setRelativeValue( double r )
{
    if ( !d->referencePoint ) return;
    ++d->blockRelative;
    setValue( r * d->referencePoint );
    --d->blockRelative;
}

void KDoubleNumInput::setReferencePoint( double ref )
{
    // clip to valid range:
    ref = kMin( maxValue(), kMax( minValue(), ref ) );
    d->referencePoint = ref;
}

void KDoubleNumInput::setRange(double lower, double upper, double step,
                                                           bool slider)
{
    if( m_slider ) {
	// don't update the slider to avoid an endless recursion
	TQSpinBox * spin = d->spin;
	disconnect(spin, TQ_SIGNAL(valueChanged(int)),
		m_slider, TQ_SLOT(setValue(int)) );
    }
    d->spin->setRange( lower, upper, step, d->spin->precision() );

    if(slider) {
	// upcast to base type to get the min/maxValue in int form:
	TQSpinBox * spin = d->spin;
        int slmax = spin->maxValue();
	int slmin = spin->minValue();
        int slvalue = spin->value();
	int slstep = spin->lineStep();
        if (m_slider) {
            m_slider->setRange(slmin, slmax);
            m_slider->setValue(slvalue);
        } else {
            m_slider = new TQSlider(slmin, slmax, slstep, slvalue,
                                   TQt::Horizontal, this);
            m_slider->setTickmarks(TQSlider::Below);
	    // feedback line: when one moves, the other moves, too:
            connect(m_slider, TQ_SIGNAL(valueChanged(int)),
                    TQ_SLOT(sliderMoved(int)) );
        }
	connect(spin, TQ_SIGNAL(valueChanged(int)),
			m_slider, TQ_SLOT(setValue(int)) );
	// calculate ( slmax - slmin ) / 10 without overflowing ints:
	int major = calcDiffByTen( slmax, slmin );
	if ( !major ) major = slstep; // ### needed?
	m_slider->setSteps(slstep, major);
        m_slider->setTickInterval(major);
    } else {
        delete m_slider;
        m_slider = 0;
    }

    setReferencePoint( referencePoint() );

    layout(true);
    updateLegacyMembers();
}

void KDoubleNumInput::setMinValue(double min)
{
    setRange(min, maxValue(), d->spin->lineStep(), m_slider);
}

double KDoubleNumInput::minValue() const
{
    return d->spin->minValue();
}

void KDoubleNumInput::setMaxValue(double max)
{
    setRange(minValue(), max, d->spin->lineStep(), m_slider);
}

double KDoubleNumInput::maxValue() const
{
    return d->spin->maxValue();
}

double  KDoubleNumInput::value() const
{
    return d->spin->value();
}

double KDoubleNumInput::relativeValue() const
{
    if ( !d->referencePoint ) return 0;
    return value() / d->referencePoint;
}

double KDoubleNumInput::referencePoint() const
{
    return d->referencePoint;
}

TQString KDoubleNumInput::suffix() const
{
    return d->spin->suffix();
}

TQString KDoubleNumInput::prefix() const
{
    return d->spin->prefix();
}

void KDoubleNumInput::setSuffix(const TQString &suffix)
{
    d->spin->setSuffix( suffix );

    layout(true);
}

void KDoubleNumInput::setPrefix(const TQString &prefix)
{
    d->spin->setPrefix( prefix );

    layout(true);
}

void KDoubleNumInput::setPrecision(int precision)
{
    d->spin->setPrecision( precision );
    if(m_slider) {
        // upcast to base type to get the min/maxValue in int form:
        TQSpinBox * spin = d->spin;
        m_slider->setRange(spin->minValue(), spin->maxValue());
        m_slider->setValue(spin->value());
        int major = calcDiffByTen(spin->maxValue(), spin->minValue());
        if ( !major ) major = spin->lineStep();
        m_slider->setSteps(spin->lineStep(), major);
        m_slider->setTickInterval(major);
    }

    layout(true);
}

int KDoubleNumInput::precision() const
{
    return d->spin->precision();
}

void KDoubleNumInput::setSpecialValueText(const TQString& text)
{
    d->spin->setSpecialValueText( text );

    layout(true);
    updateLegacyMembers();
}

void KDoubleNumInput::setLabel(const TQString & label, int a)
{
    KNumInput::setLabel(label, a);

    if(m_label)
        m_label->setBuddy(d->spin);

}

// ----------------------------------------------------------------------------


class KDoubleSpinBoxValidator : public KDoubleValidator
{
public:
    KDoubleSpinBoxValidator( double bottom, double top, int decimals, KDoubleSpinBox* sb, const char *name )
        : KDoubleValidator( bottom, top, decimals, sb, name ), spinBox( sb ) { }

    virtual State validate( TQString& str, int& pos ) const;

private:
    KDoubleSpinBox *spinBox;
};

TQValidator::State KDoubleSpinBoxValidator::validate( TQString& str, int& pos ) const
{
    TQString pref = spinBox->prefix();
    TQString suff = spinBox->suffix();
    TQString suffStriped = suff.stripWhiteSpace();
    uint overhead = pref.length() + suff.length();
    State state = Invalid;

    if ( overhead == 0 ) {
        state = KDoubleValidator::validate( str, pos );
    } else {
        bool stripedVersion = false;
        if ( str.length() >= overhead && str.startsWith(pref)
             && (str.endsWith(suff)
                 || (stripedVersion = str.endsWith(suffStriped))) ) {
            if ( stripedVersion )
                overhead = pref.length() + suffStriped.length();
            TQString core = str.mid( pref.length(), str.length() - overhead );
            int corePos = pos - pref.length();
            state = KDoubleValidator::validate( core, corePos );
            pos = corePos + pref.length();
            str.replace( pref.length(), str.length() - overhead, core );
        } else {
            state = KDoubleValidator::validate( str, pos );
            if ( state == Invalid ) {
                // stripWhiteSpace(), cf. TQSpinBox::interpretText()
                TQString special = spinBox->specialValueText().stripWhiteSpace();
                TQString candidate = str.stripWhiteSpace();

                if ( special.startsWith(candidate) ) {
                    if ( candidate.length() == special.length() ) {
                        state = Acceptable;
                    } else {
                        state = Intermediate;
                    }
                }
            }
        }
    }
    return state;
}

// We use a kind of fixed-point arithmetic to represent the range of
// doubles [mLower,mUpper] in steps of 10^(-mPrecision). Thus, the
// following relations hold:
//
// 1. factor = 10^mPrecision
// 2. basicStep = 1/factor = 10^(-mPrecision);
// 3. lowerInt = lower * factor;
// 4. upperInt = upper * factor;
// 5. lower = lowerInt * basicStep;
// 6. upper = upperInt * basicStep;
class KDoubleSpinBox::Private {
public:
  Private( int precision=1 )
    : mPrecision( precision ),
      mValidator( 0 )
  {
  }

  int factor() const {
    int f = 1;
    for ( int i = 0 ; i < mPrecision ; ++i ) f *= 10;
    return f;
  }

  double basicStep() const {
    return 1.0/double(factor());
  }

  int mapToInt( double value, bool * ok ) const {
    assert( ok );
    const double f = factor();
    if ( value > double(INT_MAX) / f ) {
      kdWarning() << "KDoubleSpinBox: can't represent value " << value
		  << "in terms of fixed-point numbers with precision "
		  << mPrecision << endl;
      *ok = false;
      return INT_MAX;
    } else if ( value < double(INT_MIN) / f ) {
      kdWarning() << "KDoubleSpinBox: can't represent value " << value
		  << "in terms of fixed-point numbers with precision "
		  << mPrecision << endl;
      *ok = false;
      return INT_MIN;
    } else {
      *ok = true;
      return int( value * f + ( value < 0 ? -0.5 : 0.5 ) );
    }
  }

  double mapToDouble( int value ) const {
    return double(value) * basicStep();
  }

  int mPrecision;
  KDoubleSpinBoxValidator * mValidator;
};

KDoubleSpinBox::KDoubleSpinBox( TQWidget * parent, const char * name )
  : TQSpinBox( parent, name )
{
  editor()->setAlignment( TQt::AlignRight );
  d = new Private();
  updateValidator();
  connect( this, TQ_SIGNAL(valueChanged(int)), TQ_SLOT(slotValueChanged(int)) );
}

KDoubleSpinBox::KDoubleSpinBox( double lower, double upper, double step,
				double value, int precision,
				TQWidget * parent, const char * name )
  : TQSpinBox( parent, name )
{
  editor()->setAlignment( TQt::AlignRight );
  d = new Private();
  setRange( lower, upper, step, precision );
  setValue( value );
  connect( this, TQ_SIGNAL(valueChanged(int)), TQ_SLOT(slotValueChanged(int)) );
}

KDoubleSpinBox::~KDoubleSpinBox() {
  delete d; d = 0;
}

bool KDoubleSpinBox::acceptLocalizedNumbers() const {
  if ( !d->mValidator ) return true; // we'll set one that does;
                                     // can't do it now, since we're const
  return d->mValidator->acceptLocalizedNumbers();
}

void KDoubleSpinBox::setAcceptLocalizedNumbers( bool accept ) {
  if ( !d->mValidator ) updateValidator();
  d->mValidator->setAcceptLocalizedNumbers( accept );
}

void KDoubleSpinBox::setRange( double lower, double upper, double step,
			       int precision ) {
  lower = kMin(upper, lower);
  upper = kMax(upper, lower);
  setPrecision( precision, true ); // disable bounds checking, since
  setMinValue( lower );            // it's done in set{Min,Max}Value
  setMaxValue( upper );            // anyway and we want lower, upper
  setLineStep( step );             // and step to have the right precision
}

int KDoubleSpinBox::precision() const {
  return d->mPrecision;
}

void KDoubleSpinBox::setPrecision( int precision ) {
    setPrecision( precision, false );
}

void KDoubleSpinBox::setPrecision( int precision, bool force ) {
  if ( precision < 0 ) return;
  if ( !force ) {
    int maxPrec = maxPrecision();
    if ( precision > maxPrec )
    {
      precision = maxPrec;
    }
  }
  // Update minValue, maxValue, value and lineStep to match the precision change
  int oldPrecision = d->mPrecision;
  double oldValue = value();
  double oldMinValue = minValue();
  double oldMaxValue = maxValue();
  double oldLineStep = lineStep();
  d->mPrecision = precision;
  if (precision != oldPrecision)
  {
    setMinValue(oldMinValue);
    setMaxValue(oldMaxValue);
    setValue(oldValue);
    setLineStep(oldLineStep);
  }
  updateValidator();
}

int KDoubleSpinBox::maxPrecision() const {
    // INT_MAX must be > maxAbsValue * 10^precision
    // ==> 10^precision < INT_MAX / maxAbsValue
    // ==> precision < log10 ( INT_MAX / maxAbsValue )
    // ==> maxPrecision = floor( log10 ( INT_MAX / maxAbsValue ) );
    double maxAbsValue = kMax( fabs(minValue()), fabs(maxValue()) );
    if ( maxAbsValue == 0 ) return 6; // return arbitrary value to avoid dbz...

    return int( floor( log10( double(INT_MAX) / maxAbsValue ) ) );
}

double KDoubleSpinBox::value() const {
  return d->mapToDouble( base::value() );
}

void KDoubleSpinBox::setValue( double value ) {
    if ( value == this->value() ) return;
    if ( value < minValue() )
	base::setValue( base::minValue() );
    else if ( value > maxValue() )
	base::setValue( base::maxValue() );
    else {
	bool ok = false;
	base::setValue( d->mapToInt( value, &ok ) );
	assert( ok );
    }
}

double KDoubleSpinBox::minValue() const {
  return d->mapToDouble( base::minValue() );
}

void KDoubleSpinBox::setMinValue( double value ) {
  bool ok = false;
  int min = d->mapToInt( value, &ok );
  base::setMinValue( min );
  updateValidator();
}


double KDoubleSpinBox::maxValue() const {
  return d->mapToDouble( base::maxValue() );
}

void KDoubleSpinBox::setMaxValue( double value ) {
  bool ok = false;
  int max = d->mapToInt( value, &ok );
  base::setMaxValue( max );
  updateValidator();
}

double KDoubleSpinBox::lineStep() const {
  return d->mapToDouble( base::lineStep() );
}

void KDoubleSpinBox::setLineStep( double step ) {
  bool ok = false;
  if ( step > maxValue() - minValue() )
    base::setLineStep( 1 );
  else
    base::setLineStep( kMax( d->mapToInt( step, &ok ), 1 ) );
}

TQString KDoubleSpinBox::mapValueToText( int value ) {
  if ( acceptLocalizedNumbers() )
    return TDEGlobal::locale()
      ->formatNumber( d->mapToDouble( value ), d->mPrecision );
  else
    return TQString().setNum( d->mapToDouble( value ), 'f', d->mPrecision );
}

int KDoubleSpinBox::mapTextToValue( bool * ok ) {
  double value;
  if ( acceptLocalizedNumbers() )
    value = TDEGlobal::locale()->readNumber( cleanText(), ok );
  else
    value = cleanText().toDouble( ok );
  if ( !*ok ) return 0;
  if ( value > maxValue() )
    value = maxValue();
  else if ( value < minValue() )
    value = minValue();
  return d->mapToInt( value, ok );
}

void KDoubleSpinBox::setValidator( const TQValidator * ) {
  // silently discard the new validator. We don't want another one ;-)
}

void KDoubleSpinBox::slotValueChanged( int value ) {
  emit valueChanged( d->mapToDouble( value ) );
}

void KDoubleSpinBox::updateValidator() {
  if ( !d->mValidator ) {
    d->mValidator =  new KDoubleSpinBoxValidator( minValue(), maxValue(), precision(),
					   this, "d->mValidator" );
    base::setValidator( d->mValidator );
  } else
    d->mValidator->setRange( minValue(), maxValue(), precision() );
}

void KNumInput::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KIntNumInput::virtual_hook( int id, void* data )
{ KNumInput::virtual_hook( id, data ); }

void KDoubleNumInput::virtual_hook( int id, void* data )
{ KNumInput::virtual_hook( id, data ); }

void KIntSpinBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KDoubleSpinBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "knuminput.moc"
