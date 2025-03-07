/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2004, 2005, 2006 Apple Computer, Inc.
 *           (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
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
 */

#include "dom/css_value.h"
#include "dom/dom_exception.h"
#include "dom/dom_string.h"

#include "css/css_valueimpl.h"
#include "css/css_ruleimpl.h"
#include "css/css_stylesheetimpl.h"
#include "css/cssparser.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"

#include "xml/dom_stringimpl.h"
#include "xml/dom_docimpl.h"

#include "misc/loader.h"

#include "rendering/font.h"
#include "rendering/render_style.h"

#include <kdebug.h>
#include <tqregexp.h>
#include <tqpaintdevice.h>
#include <tqpaintdevicemetrics.h>

// Hack for debugging purposes
extern DOM::DOMString getPropertyName(unsigned short id);

using tdehtml::FontDef;

using namespace DOM;

// Quotes the string if it needs quoting.
static DOMString quoteStringIfNeeded(const DOMString &string)
{
    // FIXME: Also need to transform control characters into \ sequences.
    TQString s = string.string();
    s.replace('\\', "\\\\");
    s.replace('\'', "\\'");
    return '\'' + s + '\'';
}


CSSStyleDeclarationImpl::CSSStyleDeclarationImpl(CSSRuleImpl *parent)
    : StyleBaseImpl(parent)
{
    m_lstValues = 0;
    m_node = 0;
}

CSSStyleDeclarationImpl::CSSStyleDeclarationImpl(CSSRuleImpl *parent, TQPtrList<CSSProperty> *lstValues)
    : StyleBaseImpl(parent)
{
    m_lstValues = lstValues;
    m_node = 0;
}

CSSStyleDeclarationImpl&  CSSStyleDeclarationImpl::operator= (const CSSStyleDeclarationImpl& o)
{
    // don't attach it to the same node, just leave the current m_node value
    delete m_lstValues;
    m_lstValues = 0;
    if (o.m_lstValues) {
        m_lstValues = new TQPtrList<CSSProperty>;
        m_lstValues->setAutoDelete( true );

        TQPtrListIterator<CSSProperty> lstValuesIt(*o.m_lstValues);
        for (lstValuesIt.toFirst(); lstValuesIt.current(); ++lstValuesIt)
            m_lstValues->append(new CSSProperty(*lstValuesIt.current()));
    }

    return *this;
}

CSSStyleDeclarationImpl::~CSSStyleDeclarationImpl()
{
    delete m_lstValues;
    // we don't use refcounting for m_node, to avoid cyclic references (see ElementImpl)
}

DOMString CSSStyleDeclarationImpl::getPropertyValue( int propertyID ) const
{
    if(!m_lstValues) return DOMString();
    CSSValueImpl* value = getPropertyCSSValue( propertyID );
    if ( value )
        return value->cssText();

    // Shorthand and 4-values properties
    switch ( propertyID ) {
    case CSS_PROP_BACKGROUND_POSITION:
    {
        // ## Is this correct? The code in cssparser.cpp is confusing
        const int properties[2] = { CSS_PROP_BACKGROUND_POSITION_X,
                                    CSS_PROP_BACKGROUND_POSITION_Y };
        return getShortHandValue( properties, 2 );
    }
    case CSS_PROP_BACKGROUND:
    {
        const int properties[5] = { CSS_PROP_BACKGROUND_IMAGE, CSS_PROP_BACKGROUND_REPEAT,
                                    CSS_PROP_BACKGROUND_ATTACHMENT, CSS_PROP_BACKGROUND_POSITION,
                                    CSS_PROP_BACKGROUND_COLOR };
        return getShortHandValue( properties, 5 );
    }
    case CSS_PROP_BORDER:
    {
        const int properties[3] = { CSS_PROP_BORDER_WIDTH, CSS_PROP_BORDER_STYLE,
                                    CSS_PROP_BORDER_COLOR };
        return getShortHandValue( properties, 3 );
    }
    case CSS_PROP_BORDER_TOP:
    {
        const int properties[3] = { CSS_PROP_BORDER_TOP_WIDTH, CSS_PROP_BORDER_TOP_STYLE,
                                    CSS_PROP_BORDER_TOP_COLOR};
        return getShortHandValue( properties, 3 );
    }
    case CSS_PROP_BORDER_RIGHT:
    {
        const int properties[3] = { CSS_PROP_BORDER_RIGHT_WIDTH, CSS_PROP_BORDER_RIGHT_STYLE,
                                    CSS_PROP_BORDER_RIGHT_COLOR};
        return getShortHandValue( properties, 3 );
    }
    case CSS_PROP_BORDER_BOTTOM:
    {
        const int properties[3] = { CSS_PROP_BORDER_BOTTOM_WIDTH, CSS_PROP_BORDER_BOTTOM_STYLE,
                                    CSS_PROP_BORDER_BOTTOM_COLOR};
        return getShortHandValue( properties, 3 );
    }
    case CSS_PROP_BORDER_LEFT:
    {
        const int properties[3] = { CSS_PROP_BORDER_LEFT_WIDTH, CSS_PROP_BORDER_LEFT_STYLE,
                                    CSS_PROP_BORDER_LEFT_COLOR};
        return getShortHandValue( properties, 3 );
    }
    case CSS_PROP_OUTLINE:
    {
        const int properties[3] = { CSS_PROP_OUTLINE_WIDTH, CSS_PROP_OUTLINE_STYLE,
                                    CSS_PROP_OUTLINE_COLOR };
        return getShortHandValue( properties, 3 );
    }
    case CSS_PROP_BORDER_COLOR:
    {
        const int properties[4] = { CSS_PROP_BORDER_TOP_COLOR, CSS_PROP_BORDER_RIGHT_COLOR,
                                    CSS_PROP_BORDER_BOTTOM_COLOR, CSS_PROP_BORDER_LEFT_COLOR };
        return get4Values( properties );
    }
    case CSS_PROP_BORDER_WIDTH:
    {
        const int properties[4] = { CSS_PROP_BORDER_TOP_WIDTH, CSS_PROP_BORDER_RIGHT_WIDTH,
                                    CSS_PROP_BORDER_BOTTOM_WIDTH, CSS_PROP_BORDER_LEFT_WIDTH };
        return get4Values( properties );
    }
    case CSS_PROP_BORDER_STYLE:
    {
        const int properties[4] = { CSS_PROP_BORDER_TOP_STYLE, CSS_PROP_BORDER_RIGHT_STYLE,
                                    CSS_PROP_BORDER_BOTTOM_STYLE, CSS_PROP_BORDER_LEFT_STYLE };
        return get4Values( properties );
    }
    case CSS_PROP_MARGIN:
    {
        const int properties[4] = { CSS_PROP_MARGIN_TOP, CSS_PROP_MARGIN_RIGHT,
                                    CSS_PROP_MARGIN_BOTTOM, CSS_PROP_MARGIN_LEFT };
        return get4Values( properties );
    }
    case CSS_PROP_PADDING:
    {
        const int properties[4] = { CSS_PROP_PADDING_TOP, CSS_PROP_PADDING_RIGHT,
                                    CSS_PROP_PADDING_BOTTOM, CSS_PROP_PADDING_LEFT };
        return get4Values( properties );
    }
    case CSS_PROP_LIST_STYLE:
    {
        const int properties[3] = { CSS_PROP_LIST_STYLE_TYPE, CSS_PROP_LIST_STYLE_POSITION,
                                    CSS_PROP_LIST_STYLE_IMAGE };
        return getShortHandValue( properties, 3 );
    }
    }
    //kdDebug() << k_funcinfo << "property not found:" << propertyID << endl;
    return DOMString();
}

DOMString CSSStyleDeclarationImpl::get4Values( const int* properties ) const
{
    DOMString res;
    for ( int i = 0 ; i < 4 ; ++i ) {
        CSSValueImpl* value = getPropertyCSSValue( properties[i] );
        if ( !value ) { // apparently all 4 properties must be specified.
            return DOMString();
        }
        if ( i > 0 )
            res += " ";
        res += value->cssText();
    }
    return res;
}

DOMString CSSStyleDeclarationImpl::getShortHandValue( const int* properties, int number ) const
{
    DOMString res;
    for ( int i = 0 ; i < number ; ++i ) {
        CSSValueImpl* value = getPropertyCSSValue( properties[i] );
        if ( value ) { // TODO provide default value if !value
            if ( !res.isNull() )
                res += " ";
            res += value->cssText();
        }
    }
    return res;
}

 CSSValueImpl *CSSStyleDeclarationImpl::getPropertyCSSValue( int propertyID ) const
{
    if(!m_lstValues) return 0;

    TQPtrListIterator<CSSProperty> lstValuesIt(*m_lstValues);
    CSSProperty *current;
    for ( lstValuesIt.toLast(); (current = lstValuesIt.current()); --lstValuesIt )
        if (current->m_id == propertyID && !current->nonCSSHint)
            return current->value();
    return 0;
}

DOMString CSSStyleDeclarationImpl::removeProperty( int propertyID, bool NonCSSHint )
{
    if(!m_lstValues) return DOMString();
    DOMString value;

    TQPtrListIterator<CSSProperty> lstValuesIt(*m_lstValues);
     CSSProperty *current;
     for ( lstValuesIt.toLast(); (current = lstValuesIt.current()); --lstValuesIt )  {
         if (current->m_id == propertyID && NonCSSHint == current->nonCSSHint) {
             value = current->value()->cssText();
             m_lstValues->removeRef(current);
             setChanged();
	     break;
        }
     }

    return value;
}

void CSSStyleDeclarationImpl::setChanged()
{
    if (m_node) {
        m_node->setChanged();
        return;
    }

    // ### quick&dirty hack for KDE 3.0... make this MUCH better! (Dirk)
    for (StyleBaseImpl* stylesheet = this; stylesheet; stylesheet = stylesheet->parent())
        if (stylesheet->isCSSStyleSheet()) {
            static_cast<CSSStyleSheetImpl*>(stylesheet)->doc()->updateStyleSelector();
            break;
        }
}

void CSSStyleDeclarationImpl::removeCSSHints()
{
    if (!m_lstValues)
	return;

    for (int i = (int)m_lstValues->count()-1; i >= 0; i--) {
	if (!m_lstValues->at(i)->nonCSSHint)
	    m_lstValues->remove(i);
    }
}

bool CSSStyleDeclarationImpl::getPropertyPriority( int propertyID ) const
{
    if ( m_lstValues) {
	TQPtrListIterator<CSSProperty> lstValuesIt(*m_lstValues);
	CSSProperty *current;
	for ( lstValuesIt.toFirst(); (current = lstValuesIt.current()); ++lstValuesIt ) {
	    if( propertyID == current->m_id )
		return current->m_important;
	}
    }
    return false;
}

bool CSSStyleDeclarationImpl::setProperty(int id, const DOMString &value, bool important, bool nonCSSHint)
{
    if(!m_lstValues) {
	m_lstValues = new TQPtrList<CSSProperty>;
	m_lstValues->setAutoDelete(true);
    }

    CSSParser parser( strictParsing );
    bool success = parser.parseValue( this, id, value, important, nonCSSHint );
    if(!success)
	kdDebug( 6080 ) << "CSSStyleDeclarationImpl::setProperty invalid property: [" << getPropertyName(id).string()
			<< "] value: [" << value.string() << "]"<< endl;
    else
        setChanged();
    return success;
}

void CSSStyleDeclarationImpl::setProperty(int id, int value, bool important, bool nonCSSHint)
{
    if(!m_lstValues) {
	m_lstValues = new TQPtrList<CSSProperty>;
	m_lstValues->setAutoDelete(true);
    }
    removeProperty(id, nonCSSHint );

    CSSValueImpl * cssValue = new CSSPrimitiveValueImpl(value);
    setParsedValue(id, cssValue, important, nonCSSHint, m_lstValues);
    setChanged();
}

void CSSStyleDeclarationImpl::setLengthProperty(int id, const DOM::DOMString &value, bool important, bool nonCSSHint, bool _multiLength )
{
    bool parseMode = strictParsing;
    strictParsing = false;
    multiLength = _multiLength;
    setProperty( id, value, important, nonCSSHint);
    strictParsing = parseMode;
    multiLength = false;
}

void CSSStyleDeclarationImpl::setProperty ( const DOMString &propertyString)
{
    if(!m_lstValues) {
	m_lstValues = new TQPtrList<CSSProperty>;
	m_lstValues->setAutoDelete( true );
    }

    CSSParser parser( strictParsing );
    parser.parseDeclaration( this, propertyString, false );
    setChanged();
}

unsigned long CSSStyleDeclarationImpl::length() const
{
    return m_lstValues ? m_lstValues->count() : 0;
}

DOMString CSSStyleDeclarationImpl::item( unsigned long index ) const
{
    if(m_lstValues && index < m_lstValues->count() && m_lstValues->at(index))
	return getPropertyName(m_lstValues->at(index)->m_id);
    return DOMString();
}

CSSRuleImpl *CSSStyleDeclarationImpl::parentRule() const
{
    return (m_parent && m_parent->isRule() ) ?
	static_cast<CSSRuleImpl *>(m_parent) : 0;
}

DOM::DOMString CSSStyleDeclarationImpl::cssText() const
{
    DOMString result;

    if ( m_lstValues) {
	TQPtrListIterator<CSSProperty> lstValuesIt(*m_lstValues);
	CSSProperty *current;
	for ( lstValuesIt.toFirst(); (current = lstValuesIt.current()); ++lstValuesIt ) {
	    result += current->cssText();
	}
    }

    return result;
}

void CSSStyleDeclarationImpl::setCssText(DOM::DOMString text)
{
    if (m_lstValues) {
	m_lstValues->clear();
    } else {
	m_lstValues = new TQPtrList<CSSProperty>;
	m_lstValues->setAutoDelete( true );
    }

    CSSParser parser( strictParsing );
    parser.parseDeclaration( this, text, false );
    setChanged();
}

bool CSSStyleDeclarationImpl::parseString( const DOMString &/*string*/, bool )
{
    kdDebug() << "WARNING: CSSStyleDeclarationImpl::parseString, unimplemented, was called" << endl;
    return false;
    // ###
}


// --------------------------------------------------------------------------------------

unsigned short CSSInheritedValueImpl::cssValueType() const
{
    return CSSValue::CSS_INHERIT;
}

DOM::DOMString CSSInheritedValueImpl::cssText() const
{
    return DOMString("inherit");
}

unsigned short CSSInitialValueImpl::cssValueType() const
{
    return CSSValue::CSS_INITIAL;
}

DOM::DOMString CSSInitialValueImpl::cssText() const
{
    return DOMString("initial");
}

// ----------------------------------------------------------------------------------------

CSSValueListImpl::~CSSValueListImpl()
{
    CSSValueImpl *val = m_values.first();
    while( val ) {
	val->deref();
	val = m_values.next();
    }
}

unsigned short CSSValueListImpl::cssValueType() const
{
    return CSSValue::CSS_VALUE_LIST;
}

void CSSValueListImpl::append(CSSValueImpl *val)
{
    m_values.append(val);
    val->ref();
}

DOM::DOMString CSSValueListImpl::cssText() const
{
    DOMString result = "";

    for (TQPtrListIterator<CSSValueImpl> iterator(m_values); iterator.current(); ++iterator) {
	result += iterator.current()->cssText();
    }

    return result;
}

// -------------------------------------------------------------------------------------

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl()
    : CSSValueImpl()
{
    m_type = 0;
}

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl(int ident)
    : CSSValueImpl()
{
    m_value.ident = ident;
    m_type = CSSPrimitiveValue::CSS_IDENT;
}

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl(double num, CSSPrimitiveValue::UnitTypes type)
{
    m_value.num = num;
    m_type = type;
}

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl(const DOMString &str, CSSPrimitiveValue::UnitTypes type)
{
    m_value.string = str.implementation();
    if(m_value.string) m_value.string->ref();
    m_type = type;
}

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl(CounterImpl *c)
{
    m_value.counter = c;
    if (m_value.counter)
	m_value.counter->ref();
    m_type = CSSPrimitiveValue::CSS_COUNTER;
}

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl( RectImpl *r)
{
    m_value.rect = r;
    if (m_value.rect)
	m_value.rect->ref();
    m_type = CSSPrimitiveValue::CSS_RECT;
}

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl(TQRgb color)
{
    m_value.rgbcolor = color;
    m_type = CSSPrimitiveValue::CSS_RGBCOLOR;
}

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl(PairImpl *p)
{
    m_value.pair = p;
    if (m_value.pair)
	m_value.pair->ref();
    m_type = CSSPrimitiveValue::CSS_PAIR;
}


CSSPrimitiveValueImpl::~CSSPrimitiveValueImpl()
{
    cleanup();
}

void CSSPrimitiveValueImpl::cleanup()
{
    switch(m_type) {
    case CSSPrimitiveValue::CSS_STRING:
    case CSSPrimitiveValue::CSS_URI:
    case CSSPrimitiveValue::CSS_ATTR:
	if(m_value.string) m_value.string->deref();
        break;
    case CSSPrimitiveValue::CSS_COUNTER:
	m_value.counter->deref();
        break;
    case CSSPrimitiveValue::CSS_RECT:
	m_value.rect->deref();
        break;
    case CSSPrimitiveValue::CSS_PAIR:
        m_value.pair->deref();
        break;
    default:
        break;
    }

    m_type = 0;
}

int CSSPrimitiveValueImpl::computeLength( tdehtml::RenderStyle *style, TQPaintDeviceMetrics *devMetrics )
{
    double result = computeLengthFloat( style, devMetrics );
    // This conversion is imprecise, often resulting in values of, e.g., 44.99998.  We
    // need to go ahead and round if we're really close to the next integer value.
    int intResult = (int)(result + (result < 0 ? -0.01 : +0.01));
    return intResult;
}

double CSSPrimitiveValueImpl::computeLengthFloat( tdehtml::RenderStyle *style, TQPaintDeviceMetrics *devMetrics )
{
    unsigned short type = primitiveType();

    double dpiY = 72.; // fallback
    if ( devMetrics )
        dpiY = devMetrics->logicalDpiY();
    if ( !tdehtml::printpainter && dpiY < 96 )
        dpiY = 96.;

    double factor = 1.;
    switch(type)
    {
    case CSSPrimitiveValue::CSS_EMS:
        factor = style->font().pixelSize();
        break;
    case CSSPrimitiveValue::CSS_EXS:
	{
        TQFontMetrics fm = style->fontMetrics();
#ifdef APPLE_CHANGES
        factor = fm.xHeight();
#else
        TQRect b = fm.boundingRect('x');
        factor = b.height();
#endif
        break;
	}
    case CSSPrimitiveValue::CSS_PX:
        break;
    case CSSPrimitiveValue::CSS_CM:
	factor = dpiY/2.54; //72dpi/(2.54 cm/in)
        break;
    case CSSPrimitiveValue::CSS_MM:
	factor = dpiY/25.4;
        break;
    case CSSPrimitiveValue::CSS_IN:
        factor = dpiY;
        break;
    case CSSPrimitiveValue::CSS_PT:
        factor = dpiY/72.;
        break;
    case CSSPrimitiveValue::CSS_PC:
        // 1 pc == 12 pt
        factor = dpiY*12./72.;
        break;
    default:
        return -1;
    }

    return floatValue(type)*factor;
}

void CSSPrimitiveValueImpl::setFloatValue( unsigned short unitType, double floatValue, int &exceptioncode )
{
    exceptioncode = 0;
    cleanup();
    // ### check if property supports this type
    if(m_type > CSSPrimitiveValue::CSS_DIMENSION) {
	exceptioncode = CSSException::SYNTAX_ERR + CSSException::_EXCEPTION_OFFSET;
	return;
    }
    //if(m_type > CSSPrimitiveValue::CSS_DIMENSION) throw DOMException(DOMException::INVALID_ACCESS_ERR);
    m_value.num = floatValue;
    m_type = unitType;
}

void CSSPrimitiveValueImpl::setStringValue( unsigned short stringType, const DOMString &stringValue, int &exceptioncode )
{
    exceptioncode = 0;
    cleanup();
    //if(m_type < CSSPrimitiveValue::CSS_STRING) throw DOMException(DOMException::INVALID_ACCESS_ERR);
    //if(m_type > CSSPrimitiveValue::CSS_ATTR) throw DOMException(DOMException::INVALID_ACCESS_ERR);
    if(m_type < CSSPrimitiveValue::CSS_STRING || m_type > CSSPrimitiveValue::CSS_ATTR) {
	exceptioncode = CSSException::SYNTAX_ERR + CSSException::_EXCEPTION_OFFSET;
	return;
    }
    if(stringType != CSSPrimitiveValue::CSS_IDENT)
    {
	m_value.string = stringValue.implementation();
	m_value.string->ref();
	m_type = stringType;
    }
    // ### parse ident
}

unsigned short CSSPrimitiveValueImpl::cssValueType() const
{
    return CSSValue::CSS_PRIMITIVE_VALUE;
}

bool CSSPrimitiveValueImpl::parseString( const DOMString &/*string*/, bool )
{
    // ###
    kdDebug() << "WARNING: CSSPrimitiveValueImpl::parseString, unimplemented, was called" << endl;
    return false;
}

int CSSPrimitiveValueImpl::getIdent()
{
    if(m_type != CSSPrimitiveValue::CSS_IDENT) return 0;
    return m_value.ident;
}

DOM::DOMString CSSPrimitiveValueImpl::cssText() const
{
    // ### return the original value instead of a generated one (e.g. color
    // name if it was specified) - check what spec says about this
    DOMString text;
    switch ( m_type ) {
	case CSSPrimitiveValue::CSS_UNKNOWN:
	    // ###
	    break;
	case CSSPrimitiveValue::CSS_NUMBER:
	    text = DOMString(TQString::number( (int)m_value.num ));
	    break;
	case CSSPrimitiveValue::CSS_PERCENTAGE:
	    text = DOMString(TQString::number( m_value.num ) + "%");
	    break;
	case CSSPrimitiveValue::CSS_EMS:
	    text = DOMString(TQString::number( m_value.num ) + "em");
	    break;
	case CSSPrimitiveValue::CSS_EXS:
	    text = DOMString(TQString::number( m_value.num ) + "ex");
	    break;
	case CSSPrimitiveValue::CSS_PX:
	    text = DOMString(TQString::number( m_value.num ) + "px");
	    break;
	case CSSPrimitiveValue::CSS_CM:
	    text = DOMString(TQString::number( m_value.num ) + "cm");
	    break;
	case CSSPrimitiveValue::CSS_MM:
	    text = DOMString(TQString::number( m_value.num ) + "mm");
	    break;
	case CSSPrimitiveValue::CSS_IN:
	    text = DOMString(TQString::number( m_value.num ) + "in");
	    break;
	case CSSPrimitiveValue::CSS_PT:
	    text = DOMString(TQString::number( m_value.num ) + "pt");
	    break;
	case CSSPrimitiveValue::CSS_PC:
	    text = DOMString(TQString::number( m_value.num ) + "pc");
	    break;
	case CSSPrimitiveValue::CSS_DEG:
	    text = DOMString(TQString::number( m_value.num ) + "deg");
	    break;
	case CSSPrimitiveValue::CSS_RAD:
	    text = DOMString(TQString::number( m_value.num ) + "rad");
	    break;
	case CSSPrimitiveValue::CSS_GRAD:
	    text = DOMString(TQString::number( m_value.num ) + "grad");
	    break;
	case CSSPrimitiveValue::CSS_MS:
	    text = DOMString(TQString::number( m_value.num ) + "ms");
	    break;
	case CSSPrimitiveValue::CSS_S:
	    text = DOMString(TQString::number( m_value.num ) + "s");
	    break;
	case CSSPrimitiveValue::CSS_HZ:
	    text = DOMString(TQString::number( m_value.num ) + "hz");
	    break;
	case CSSPrimitiveValue::CSS_KHZ:
	    text = DOMString(TQString::number( m_value.num ) + "khz");
	    break;
	case CSSPrimitiveValue::CSS_DIMENSION:
	    // ###
	    break;
	case CSSPrimitiveValue::CSS_STRING:
	    text = quoteStringIfNeeded(m_value.string);
	    break;
	case CSSPrimitiveValue::CSS_URI:
            text  = "url(";
	    text += DOMString( m_value.string );
            text += ")";
	    break;
	case CSSPrimitiveValue::CSS_IDENT:
	    text = getValueName(m_value.ident);
	    break;
	case CSSPrimitiveValue::CSS_ATTR:
            text = "attr(";
            text += DOMString( m_value.string );
            text += ")";
	    break;
	case CSSPrimitiveValue::CSS_COUNTER:
            text = "counter(";
            text += m_value.counter->m_identifier;
            text += ")";
            // ### add list-style and separator
	    break;
	case CSSPrimitiveValue::CSS_RECT:
        {
            RectImpl* rectVal = getRectValue();
            text = "rect(";
            text += rectVal->top()->cssText() + " ";
            text += rectVal->right()->cssText() + " ";
            text += rectVal->bottom()->cssText() + " ";
            text += rectVal->left()->cssText() + ")";
            break;
        }
	case CSSPrimitiveValue::CSS_RGBCOLOR:
	    if (tqAlpha(m_value.rgbcolor) != 0xFF) {
		if (m_value.rgbcolor == tdehtml::transparentColor)
		    text = "transparent";
		else
		    text = "rgba(" + TQString::number(tqRed  (m_value.rgbcolor)) + "," 
				   + TQString::number(tqBlue (m_value.rgbcolor)) + "," 
				   + TQString::number(tqGreen(m_value.rgbcolor)) + "," 
				   + TQString::number(tqAlpha(m_value.rgbcolor)/255.0) + ")";
	    } else {
		text = TQString(TQColor(m_value.rgbcolor).name());
	    }
	    break;
        case CSSPrimitiveValue::CSS_PAIR:
            text = m_value.pair->first()->cssText();
            text += " ";
            text += m_value.pair->second()->cssText();
            break;
	default:
	    break;
    }
    return text;
}

// -----------------------------------------------------------------

RectImpl::RectImpl()
{
    m_top = 0;
    m_right = 0;
    m_bottom = 0;
    m_left = 0;
}

RectImpl::~RectImpl()
{
    if (m_top) m_top->deref();
    if (m_right) m_right->deref();
    if (m_bottom) m_bottom->deref();
    if (m_left) m_left->deref();
}

void RectImpl::setTop( CSSPrimitiveValueImpl *top )
{
    if( top ) top->ref();
    if ( m_top ) m_top->deref();
    m_top = top;
}

void RectImpl::setRight( CSSPrimitiveValueImpl *right )
{
    if( right ) right->ref();
    if ( m_right ) m_right->deref();
    m_right = right;
}

void RectImpl::setBottom( CSSPrimitiveValueImpl *bottom )
{
    if( bottom ) bottom->ref();
    if ( m_bottom ) m_bottom->deref();
    m_bottom = bottom;
}

void RectImpl::setLeft( CSSPrimitiveValueImpl *left )
{
    if( left ) left->ref();
    if ( m_left ) m_left->deref();
    m_left = left;
}

// -----------------------------------------------------------------

PairImpl::~PairImpl()
{
    if (m_first) m_first->deref(); if (m_second) m_second->deref();
}

void PairImpl::setFirst(CSSPrimitiveValueImpl* first)
{
    if (first == m_first) return;
    if (m_first) m_first->deref();
    m_first = first;
    if (m_first) m_first->ref();
}

void PairImpl::setSecond(CSSPrimitiveValueImpl* second)
{
    if (second == m_second) return;
    if (m_second) m_second->deref();
    m_second = second;
    if (m_second) m_second->ref();
}

// -----------------------------------------------------------------

CSSImageValueImpl::CSSImageValueImpl(const DOMString &url, const StyleBaseImpl* style)
    : CSSPrimitiveValueImpl(url, CSSPrimitiveValue::CSS_URI)
{
    tdehtml::DocLoader *docLoader = 0;
    const StyleBaseImpl *root = style;
    while (root->parent())
	root = root->parent();
    if (root->isCSSStyleSheet())
	docLoader = static_cast<const CSSStyleSheetImpl*>(root)->docLoader();

    m_image = docLoader->requestImage(url);
    if(m_image) m_image->ref(this);
}

CSSImageValueImpl::CSSImageValueImpl()
    : CSSPrimitiveValueImpl(CSS_VAL_NONE)
{
    m_image = 0;
}

CSSImageValueImpl::~CSSImageValueImpl()
{
    if(m_image) m_image->deref(this);
}

// ------------------------------------------------------------------------

FontFamilyValueImpl::FontFamilyValueImpl( const TQString &string)
: CSSPrimitiveValueImpl( DOMString(string), CSSPrimitiveValue::CSS_STRING)
{
    static const TQRegExp parenReg(" \\(.*\\)$");
    static const TQRegExp braceReg(" \\[.*\\]$");

    parsedFontName = string;
    // a language tag is often added in braces at the end. Remove it.
    parsedFontName.replace(parenReg, TQString());
    // remove [Xft] qualifiers
    parsedFontName.replace(braceReg, TQString());

#ifndef APPLE_CHANGES
    const TQString &available = TDEHTMLSettings::availableFamilies();

    parsedFontName = parsedFontName.lower();
    // kdDebug(0) << "searching for face '" << parsedFontName << "'" << endl;

    int pos = available.find( ',' + parsedFontName + ',', 0, false );
    if ( pos == -1 ) {
        // many pages add extra MSs to make sure it's windows only ;(
        if ( parsedFontName.startsWith( "ms " ) )
            parsedFontName = parsedFontName.mid( 3 );
        if ( parsedFontName.endsWith( " ms" ) )
            parsedFontName.truncate( parsedFontName.length() - 3 );
        pos = available.find( ",ms " + parsedFontName + ',', 0, false );
        if ( pos == -1 )
            pos = available.find( ',' + parsedFontName + " ms,", 0, false );
    }

    if ( pos != -1 ) {
       ++pos;
       int p = available.find(',', pos);
       assert( p != -1 ); // available is supposed to start and end with ,
       parsedFontName = available.mid( pos, p - pos);
       // kdDebug(0) << "going for '" << parsedFontName << "'" << endl;
    } else
        parsedFontName = TQString::null;

#endif // !APPLE_CHANGES
}

FontValueImpl::FontValueImpl()
    : style(0), variant(0), weight(0), size(0), lineHeight(0), family(0)
{
}

FontValueImpl::~FontValueImpl()
{
    delete style;
    delete variant;
    delete weight;
    delete size;
    delete lineHeight;
    delete family;
}

DOMString FontValueImpl::cssText() const
{
    // font variant weight size / line-height family

    DOMString result("");

    if (style) {
	result += style->cssText();
    }
    if (variant) {
	if (result.length() > 0) {
	    result += " ";
	}
	result += variant->cssText();
    }
    if (weight) {
	if (result.length() > 0) {
	    result += " ";
	}
	result += weight->cssText();
    }
    if (size) {
	if (result.length() > 0) {
	    result += " ";
	}
	result += size->cssText();
    }
    if (lineHeight) {
	if (!size) {
	    result += " ";
	}
	result += "/";
	result += lineHeight->cssText();
    }
    if (family) {
	if (result.length() > 0) {
	    result += " ";
	}
	result += family->cssText();
    }

    return result;
}

QuotesValueImpl::QuotesValueImpl()
    : levels(0)
{
}

DOMString QuotesValueImpl::cssText() const
{
    return "\"" + data.join("\" \"") + "\"";
}

void QuotesValueImpl::addLevel(const TQString& open, const TQString& close)
{
    data.append(open);
    data.append(close);
    levels++;
}

TQString QuotesValueImpl::openQuote(int level) const
{
    if (levels == 0) return "";
    level--; // increments are calculated before openQuote is called
//     kdDebug( 6080 ) << "Open quote level:" << level << endl;
    if (level < 0) level = 0;
    else
    if (level >= (int) levels) level = (int) (levels-1);
    return data[level*2];
}

TQString QuotesValueImpl::closeQuote(int level) const
{
    if (levels == 0) return "";
//     kdDebug( 6080 ) << "Close quote level:" << level << endl;
    if (level < 0) level = 0;
    else
    if (level >= (int) levels) level = (int) (levels-1);
    return data[level*2+1];
}

// Used for text-shadow and box-shadow
ShadowValueImpl::ShadowValueImpl(CSSPrimitiveValueImpl* _x, CSSPrimitiveValueImpl* _y,
                                 CSSPrimitiveValueImpl* _blur, CSSPrimitiveValueImpl* _color)
    :x(_x), y(_y), blur(_blur), color(_color)
{}

ShadowValueImpl::~ShadowValueImpl()
{
    delete x;
    delete y;
    delete blur;
    delete color;
}

DOMString ShadowValueImpl::cssText() const
{
    DOMString text("");
    if (color) {
        text += color->cssText();
    }
    if (x) {
        if (text.length() > 0) {
            text += " ";
        }
        text += x->cssText();
    }
    if (y) {
        if (text.length() > 0) {
            text += " ";
        }
        text += y->cssText();
    }
    if (blur) {
        if (text.length() > 0) {
            text += " ";
        }
        text += blur->cssText();
    }

    return text;
}

DOMString CounterActImpl::cssText() const
{
    DOMString text(m_counter);
    text += DOMString(TQString::number(m_value));

    return text;
}

DOMString CSSProperty::cssText() const
{
    return getPropertyName(m_id) + DOMString(": ") + m_value->cssText() + (m_important ? DOMString(" !important") : DOMString()) + DOMString("; ");
}
