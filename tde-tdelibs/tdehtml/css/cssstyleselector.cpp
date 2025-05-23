/**
 * This file is part of the CSS implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2003-2004 Apple Computer, Inc.
 *           (C) 2004-2006 Allan Sandfeld Jensen (kde@carewolf.com)
 *           (C) 2004 Germain Garand (germain@ebooksfrance.org)
 *           (C) 2005, 2006 Apple Computer, Inc.
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

#include "css/cssstyleselector.h"
#include "rendering/render_style.h"
#include "css/css_stylesheetimpl.h"
#include "css/css_ruleimpl.h"
#include "css/css_valueimpl.h"
#include "css/csshelper.h"
#include "rendering/render_object.h"
#include "html/html_documentimpl.h"
#include "html/html_elementimpl.h"
#include "xml/dom_elementimpl.h"
#include "xml/dom_restyler.h"
#include "dom/css_rule.h"
#include "dom/css_value.h"
#include "tdehtml_factory.h"
#include "tdehtmlpart_p.h"
using namespace tdehtml;
using namespace DOM;

#include "css/cssproperties.h"
#include "css/cssvalues.h"

#include "misc/tdehtmllayout.h"
#include "tdehtml_settings.h"
#include "misc/htmlhashes.h"
#include "misc/helper.h"
#include "misc/loader.h"

#include "rendering/font.h"

#include "tdehtmlview.h"
#include "tdehtml_part.h"

#include <kstandarddirs.h>
#include <kcharsets.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <tqfile.h>
#include <tqvaluelist.h>
#include <tqstring.h>
#include <tqtooltip.h>
#include <kdebug.h>
#include <kurl.h>
#include <assert.h>
#include <tqpaintdevicemetrics.h>
#include <stdlib.h>

#define HANDLE_INHERIT(prop, Prop) \
if (isInherit) \
{\
    style->set##Prop(parentStyle->prop());\
    return;\
}

#define HANDLE_INHERIT_AND_INITIAL(prop, Prop) \
HANDLE_INHERIT(prop, Prop) \
else if (isInitial) \
{\
    style->set##Prop(RenderStyle::initial##Prop());\
    return;\
}

#define HANDLE_INHERIT_AND_INITIAL_WITH_VALUE(prop, Prop, Value) \
HANDLE_INHERIT(prop, Prop) \
else if (isInitial) \
{\
    style->set##Prop(RenderStyle::initial##Value());\
    return;\
}

#define HANDLE_BACKGROUND_INHERIT_AND_INITIAL(prop, Prop) \
if (isInherit) { \
    BackgroundLayer* currChild = style->accessBackgroundLayers(); \
    BackgroundLayer* prevChild = 0; \
    const BackgroundLayer* currParent = parentStyle->backgroundLayers(); \
    while (currParent && currParent->is##Prop##Set()) { \
        if (!currChild) { \
            /* Need to make a new layer.*/ \
            currChild = new BackgroundLayer(); \
            prevChild->setNext(currChild); \
        } \
        currChild->set##Prop(currParent->prop()); \
        prevChild = currChild; \
        currChild = prevChild->next(); \
        currParent = currParent->next(); \
    } \
    \
    while (currChild) { \
        /* Reset any remaining layers to not have the property set. */ \
        currChild->clear##Prop(); \
        currChild = currChild->next(); \
    } \
    return; \
} \
if (isInitial) { \
    BackgroundLayer* currChild = style->accessBackgroundLayers(); \
    currChild->set##Prop(RenderStyle::initial##Prop()); \
    for (currChild = currChild->next(); currChild; currChild = currChild->next()) \
        currChild->clear##Prop(); \
    return; \
}

#define HANDLE_BACKGROUND_VALUE(prop, Prop, value) { \
HANDLE_BACKGROUND_INHERIT_AND_INITIAL(prop, Prop) \
if (!value->isPrimitiveValue() && !value->isValueList()) \
    return; \
BackgroundLayer* currChild = style->accessBackgroundLayers(); \
BackgroundLayer* prevChild = 0; \
if (value->isPrimitiveValue()) { \
    map##Prop(currChild, value); \
    currChild = currChild->next(); \
} \
else { \
    /* Walk each value and put it into a layer, creating new layers as needed. */ \
    CSSValueListImpl* valueList = static_cast<CSSValueListImpl*>(value); \
    for (unsigned int i = 0; i < valueList->length(); i++) { \
        if (!currChild) { \
            /* Need to make a new layer to hold this value */ \
            currChild = new BackgroundLayer(); \
            prevChild->setNext(currChild); \
        } \
        map##Prop(currChild, valueList->item(i)); \
        prevChild = currChild; \
        currChild = currChild->next(); \
    } \
} \
while (currChild) { \
    /* Reset all remaining layers to not have the property set. */ \
    currChild->clear##Prop(); \
    currChild = currChild->next(); \
} }

#define HANDLE_INHERIT_COND(propID, prop, Prop) \
if (id == propID) \
{\
    style->set##Prop(parentStyle->prop());\
    return;\
}

#define HANDLE_INITIAL_COND(propID, Prop) \
if (id == propID) \
{\
    style->set##Prop(RenderStyle::initial##Prop());\
    return;\
}

#define HANDLE_INITIAL_COND_WITH_VALUE(propID, Prop, Value) \
if (id == propID) \
{\
    style->set##Prop(RenderStyle::initial##Value());\
    return;\
}

#define HANDLE_INHERIT_ON_INHERITED_PROPERTY(prop, Prop) \
if (isInherit) \
{\
    style->set##Prop(parentStyle->prop());\
    return;\
}

#define HANDLE_INITIAL(prop, Prop) \
if (isInitial) \
{\
    style->set##Prop(RenderStyle::initial##Prop());\
    return;\
}

#define HANDLE_INITIAL_AND_INHERIT_ON_INHERITED_PROPERTY(prop, Prop) \
HANDLE_INITIAL(prop, Prop) \
else \
HANDLE_INHERIT_ON_INHERITED_PROPERTY(prop, Prop)


namespace tdehtml {

CSSStyleSelectorList *CSSStyleSelector::s_defaultStyle;
CSSStyleSelectorList *CSSStyleSelector::s_defaultQuirksStyle;
CSSStyleSelectorList *CSSStyleSelector::s_defaultPrintStyle;
CSSStyleSheetImpl *CSSStyleSelector::s_defaultSheet;
RenderStyle* CSSStyleSelector::styleNotYetAvailable;
CSSStyleSheetImpl *CSSStyleSelector::s_quirksSheet;

enum PseudoState { PseudoUnknown, PseudoNone, PseudoLink, PseudoVisited};
static PseudoState pseudoState;


CSSStyleSelector::CSSStyleSelector( DocumentImpl* doc, TQString userStyleSheet, StyleSheetListImpl *styleSheets,
                                    const KURL &url, bool _strictParsing )
{
    TDEHTMLView* view = doc->view();

    init(view ? view->part()->settings() : 0, doc);

    strictParsing = _strictParsing;
    m_medium = view ? view->mediaType() : TQString("all");

    selectors = 0;
    selectorCache = 0;
    properties = 0;
    userStyle = 0;
    userSheet = 0;
    paintDeviceMetrics = doc->paintDeviceMetrics();

    if(paintDeviceMetrics) // this may be null, not everyone uses tdehtmlview (Niko)
        computeFontSizes(paintDeviceMetrics, view ? view->part()->zoomFactor() : 100);

    if ( !userStyleSheet.isEmpty() ) {
        userSheet = new DOM::CSSStyleSheetImpl(doc);
        userSheet->parseString( DOMString( userStyleSheet ) );

        userStyle = new CSSStyleSelectorList();
        userStyle->append( userSheet, m_medium );
    }

    // add stylesheets from document
    authorStyle = new CSSStyleSelectorList();


    TQPtrListIterator<StyleSheetImpl> it( styleSheets->styleSheets );
    for ( ; it.current(); ++it ) {
        if ( it.current()->isCSSStyleSheet() && !it.current()->disabled()) {
            authorStyle->append( static_cast<CSSStyleSheetImpl*>( it.current() ), m_medium );
        }
    }

    buildLists();

    //kdDebug( 6080 ) << "number of style sheets in document " << authorStyleSheets.count() << endl;
    //kdDebug( 6080 ) << "CSSStyleSelector: author style has " << authorStyle->count() << " elements"<< endl;

    KURL u = url;

    u.setQuery( TQString::null );
    u.setRef( TQString::null );
    encodedurl.file = u.url();
    int pos = encodedurl.file.findRev('/');
    encodedurl.path = encodedurl.file;
    if ( pos > 0 ) {
	encodedurl.path.truncate( pos );
	encodedurl.path += '/';
    }
    u.setPath( TQString::null );
    encodedurl.host = u.url();

    //kdDebug() << "CSSStyleSelector::CSSStyleSelector encoded url " << encodedurl.path << endl;
}

CSSStyleSelector::CSSStyleSelector( CSSStyleSheetImpl *sheet )
{
    init(0L, 0L);

    TDEHTMLView *view = sheet->doc()->view();
    m_medium = view ? view->mediaType() : "screen";

    authorStyle = new CSSStyleSelectorList();
    authorStyle->append( sheet, m_medium );
}

void CSSStyleSelector::init(const TDEHTMLSettings* _settings, DocumentImpl* doc)
{
    element = 0;
    settings = _settings;
    paintDeviceMetrics = 0;
    propsToApply = (CSSOrderedProperty **)malloc(128*sizeof(CSSOrderedProperty *));
    pseudoProps = (CSSOrderedProperty **)malloc(128*sizeof(CSSOrderedProperty *));
    propsToApplySize = 128;
    pseudoPropsSize = 128;
    if(!s_defaultStyle) loadDefaultStyle(settings, doc);

    defaultStyle = s_defaultStyle;
    defaultPrintStyle = s_defaultPrintStyle;
    defaultQuirksStyle = s_defaultQuirksStyle;
}

CSSStyleSelector::~CSSStyleSelector()
{
    clearLists();
    delete authorStyle;
    delete userStyle;
    delete userSheet;
    free(propsToApply);
    free(pseudoProps);
}

void CSSStyleSelector::addSheet( CSSStyleSheetImpl *sheet )
{
    TDEHTMLView *view = sheet->doc()->view();
    m_medium = view ? view->mediaType() : "screen";
    authorStyle->append( sheet, m_medium );
}

void CSSStyleSelector::loadDefaultStyle(const TDEHTMLSettings *s, DocumentImpl *doc)
{
    if(s_defaultStyle) return;

    {
	TQFile f(locate( "data", "tdehtml/css/html4.css" ) );
	f.open(IO_ReadOnly);

	TQCString file( f.size()+1 );
	int readbytes = f.readBlock( file.data(), f.size() );
	f.close();
	if ( readbytes >= 0 )
	    file[readbytes] = '\0';

	TQString style = TQString::fromLatin1( file.data() );
	if(s)
	    style += s->settingsToCSS();
	DOMString str(style);

	s_defaultSheet = new DOM::CSSStyleSheetImpl(doc);
	s_defaultSheet->parseString( str );

	// Collect only strict-mode rules.
	s_defaultStyle = new CSSStyleSelectorList();
	s_defaultStyle->append( s_defaultSheet, "screen" );

	s_defaultPrintStyle = new CSSStyleSelectorList();
	s_defaultPrintStyle->append( s_defaultSheet, "print" );
    }
    {
	TQFile f(locate( "data", "tdehtml/css/quirks.css" ) );
	f.open(IO_ReadOnly);

	TQCString file( f.size()+1 );
	int readbytes = f.readBlock( file.data(), f.size() );
	f.close();
	if ( readbytes >= 0 )
	    file[readbytes] = '\0';

	TQString style = TQString::fromLatin1( file.data() );
	DOMString str(style);

	s_quirksSheet = new DOM::CSSStyleSheetImpl(doc);
	s_quirksSheet->parseString( str );

	// Collect only quirks-mode rules.
	s_defaultQuirksStyle = new CSSStyleSelectorList();
	s_defaultQuirksStyle->append( s_quirksSheet, "screen" );
    }

    //kdDebug() << "CSSStyleSelector: default style has " << defaultStyle->count() << " elements"<< endl;
}

void CSSStyleSelector::clear()
{
    delete s_defaultStyle;
    delete s_defaultQuirksStyle;
    delete s_defaultPrintStyle;
    delete s_defaultSheet;
    delete styleNotYetAvailable;
    s_defaultStyle = 0;
    s_defaultQuirksStyle = 0;
    s_defaultPrintStyle = 0;
    s_defaultSheet = 0;
    styleNotYetAvailable = 0;
}

void CSSStyleSelector::reparseConfiguration()
{
    // nice leak, but best we can do right now. hopefully its only rare.
    s_defaultStyle = 0;
    s_defaultQuirksStyle = 0;
    s_defaultPrintStyle = 0;
    s_defaultSheet = 0;
}

#define MAXFONTSIZES 8

void CSSStyleSelector::computeFontSizes(TQPaintDeviceMetrics* paintDeviceMetrics,  int zoomFactor)
{
    computeFontSizesFor(paintDeviceMetrics, zoomFactor, m_fontSizes, false);
    computeFontSizesFor(paintDeviceMetrics, zoomFactor, m_fixedFontSizes, true);
}

void CSSStyleSelector::computeFontSizesFor(TQPaintDeviceMetrics* paintDeviceMetrics, int zoomFactor, TQValueVector<int>& fontSizes, bool isFixed)
{
#ifdef APPLE_CHANGES
    // We don't want to scale the settings by the dpi.
    const float toPix = 1;
#else
    Q_UNUSED( isFixed );

    // ### get rid of float / double
    float toPix = paintDeviceMetrics->logicalDpiY()/72.;
    if (toPix  < 96./72.) toPix = 96./72.;
#endif // ######### fix isFixed code again.

    fontSizes.resize( MAXFONTSIZES );
    float scale = 1.0;
    static const float fontFactors[] =      {3./5., 3./4., 8./9., 1., 6./5., 3./2., 2., 3.};
    static const float smallFontFactors[] = {3./4., 5./6., 8./9., 1., 6./5., 3./2., 2., 3.};
    float mediumFontSize, minFontSize, factor;
    if (!tdehtml::printpainter) {
        scale *= zoomFactor / 100.0;
#ifdef APPLE_CHANGES
	if (isFixed)
	    mediumFontSize = settings->mediumFixedFontSize() * toPix;
	else
#endif
	    mediumFontSize = settings->mediumFontSize() * toPix;
        minFontSize = settings->minFontSize() * toPix;
    }
    else {
        // ## depending on something / configurable ?
        mediumFontSize = 12;
        minFontSize = 6;
    }
    const float* factors = scale*mediumFontSize >= 12.5 ? fontFactors : smallFontFactors;
    for ( int i = 0; i < MAXFONTSIZES; i++ ) {
        factor = scale*factors[i];
        fontSizes[i] = int(KMAX( mediumFontSize*factor +.5f, minFontSize));
        //kdDebug( 6080 ) << "index: " << i << " factor: " << factors[i] << " font pix size: " << int(KMAX( mediumFontSize*factor +.5f, minFontSize)) << endl;
    }
}

#undef MAXFONTSIZES

static inline void bubbleSort( CSSOrderedProperty **b, CSSOrderedProperty **e )
{
    while( b < e ) {
	bool swapped = false;
        CSSOrderedProperty **y = e+1;
	CSSOrderedProperty **x = e;
        CSSOrderedProperty **swappedPos = 0;
	do {
	    if ( !((**(--x)) < (**(--y))) ) {
		swapped = true;
                swappedPos = x;
                CSSOrderedProperty *tmp = *y;
                *y = *x;
                *x = tmp;
	    }
	} while( x != b );
	if ( !swapped ) break;
        b = swappedPos + 1;
    }
}

RenderStyle *CSSStyleSelector::styleForElement(ElementImpl *e)
{
    if (!e->getDocument()->haveStylesheetsLoaded() || !e->getDocument()->view()) {
        if (!styleNotYetAvailable) {
            styleNotYetAvailable = new RenderStyle();
            styleNotYetAvailable->setDisplay(NONE);
            styleNotYetAvailable->ref();
        }
        return styleNotYetAvailable;
    }

    // set some variables we will need
    pseudoState = PseudoUnknown;

    element = e;
    parentNode = e->parentNode();
    parentStyle = ( parentNode && parentNode->renderer()) ? parentNode->renderer()->style() : 0;
    view = element->getDocument()->view();
    part = view->part();
    settings = part->settings();
    paintDeviceMetrics = element->getDocument()->paintDeviceMetrics();

    // reset dynamic DOM dependencies
    e->getDocument()->dynamicDomRestyler().resetDependencies(e);

    style = new RenderStyle();
    if( parentStyle )
        style->inheritFrom( parentStyle );
    else
	parentStyle = style;

    unsigned int numPropsToApply = 0;
    unsigned int numPseudoProps = 0;

    // try to sort out most style rules as early as possible.
    TQ_UINT16 cssTagId = localNamePart(element->id());
    int smatch = 0;
    int schecked = 0;

    for ( unsigned int i = 0; i < selectors_size; i++ ) {
	TQ_UINT16 tag = localNamePart(selectors[i]->tag);
	if ( cssTagId == tag || tag == anyLocalName ) {
	    ++schecked;

	    checkSelector( i, e );

	    if ( selectorCache[i].state == Applies ) {
		++smatch;

// 		tqDebug("adding property" );
		for ( unsigned int p = 0; p < selectorCache[i].props_size; p += 2 )
		    for ( unsigned int j = 0; j < (unsigned int )selectorCache[i].props[p+1]; ++j ) {
                        if (numPropsToApply >= propsToApplySize ) {
                            propsToApplySize *= 2;
			    propsToApply = (CSSOrderedProperty **)realloc( propsToApply, propsToApplySize*sizeof( CSSOrderedProperty * ) );
			}
			propsToApply[numPropsToApply++] = properties[selectorCache[i].props[p]+j];
		    }
	    } else if ( selectorCache[i].state == AppliesPseudo ) {
		for ( unsigned int p = 0; p < selectorCache[i].props_size; p += 2 )
		    for ( unsigned int j = 0; j < (unsigned int) selectorCache[i].props[p+1]; ++j ) {
                        if (numPseudoProps >= pseudoPropsSize ) {
                            pseudoPropsSize *= 2;
			    pseudoProps = (CSSOrderedProperty **)realloc( pseudoProps, pseudoPropsSize*sizeof( CSSOrderedProperty * ) );
			}
			pseudoProps[numPseudoProps++] = properties[selectorCache[i].props[p]+j];
			properties[selectorCache[i].props[p]+j]->pseudoId = (RenderStyle::PseudoId) selectors[i]->pseudoId;
		    }
	    }
	}
	else
	    selectorCache[i].state = Invalid;

    }

    // inline style declarations, after all others. non css hints
    // count as author rules, and come before all other style sheets, see hack in append()
    numPropsToApply = addInlineDeclarations( e, e->m_styleDecls, numPropsToApply );

//     tqDebug( "styleForElement( %s )", e->tagName().string().latin1() );
//     tqDebug( "%d selectors, %d checked,  %d match,  %d properties ( of %d )",
// 	    selectors_size, schecked, smatch, numPropsToApply, properties_size );

    bubbleSort( propsToApply, propsToApply+numPropsToApply-1 );
    bubbleSort( pseudoProps, pseudoProps+numPseudoProps-1 );

    // we can't apply style rules without a view() and a part. This
    // tends to happen on delayed destruction of widget Renderobjects
    if ( part ) {
        fontDirty = false;

        if (numPropsToApply ) {
            CSSStyleSelector::style = style;
            for (unsigned int i = 0; i < numPropsToApply; ++i) {
		if ( fontDirty && propsToApply[i]->priority >= (1 << 30) ) {
		    // we are past the font properties, time to update to the
		    // correct font
#ifdef APPLE_CHANGES
		    checkForGenericFamilyChange(style, parentStyle);
#endif
		    CSSStyleSelector::style->htmlFont().update( paintDeviceMetrics );
		    fontDirty = false;
		}
		DOM::CSSProperty *prop = propsToApply[i]->prop;
//		if (prop->m_id == CSS_PROP__KONQ_USER_INPUT) kdDebug(6080) << "El: "<<e->nodeName().string() << " user-input: "<<((CSSPrimitiveValueImpl *)prop->value())->getIdent() << endl;
//		if (prop->m_id == CSS_PROP_TEXT_TRANSFORM) kdDebug(6080) << "El: "<<e->nodeName().string() << endl;
                applyRule( prop->m_id, prop->value() );
	    }
	    if ( fontDirty ) {
#ifdef APPLE_CHANGES
	        checkForGenericFamilyChange(style, parentStyle);
#endif
		CSSStyleSelector::style->htmlFont().update( paintDeviceMetrics );
            }
        }

        // Clean up our style object's display and text decorations (among other fixups).
        adjustRenderStyle(style, e);

        if ( numPseudoProps ) {
	    fontDirty = false;
            //tqDebug("%d applying %d pseudo props", e->cssTagId(), pseudoProps->count() );
            for (unsigned int i = 0; i < numPseudoProps; ++i) {
		if ( fontDirty && pseudoProps[i]->priority >= (1 << 30) ) {
		    // we are past the font properties, time to update to the
		    // correct font
		    //We have to do this for all pseudo styles
		    RenderStyle *pseudoStyle = style->pseudoStyle;
		    while ( pseudoStyle ) {
			pseudoStyle->htmlFont().update( paintDeviceMetrics );
			pseudoStyle = pseudoStyle->pseudoStyle;
		    }
		    fontDirty = false;
		}

                RenderStyle *pseudoStyle;
                pseudoStyle = style->getPseudoStyle(pseudoProps[i]->pseudoId);
                if (!pseudoStyle)
                {
                    pseudoStyle = style->addPseudoStyle(pseudoProps[i]->pseudoId);
                    if (pseudoStyle)
                        pseudoStyle->inheritFrom( style );
                }

                RenderStyle* oldStyle = style;
                RenderStyle* oldParentStyle = parentStyle;
                parentStyle = style;
		style = pseudoStyle;
                if ( pseudoStyle ) {
		    DOM::CSSProperty *prop = pseudoProps[i]->prop;
		    applyRule( prop->m_id, prop->value() );
		}
                style = oldStyle;
                parentStyle = oldParentStyle;
            }

	    if ( fontDirty ) {
		RenderStyle *pseudoStyle = style->pseudoStyle;
		while ( pseudoStyle ) {
		    pseudoStyle->htmlFont().update( paintDeviceMetrics );
		    pseudoStyle = pseudoStyle->pseudoStyle;
		}
	    }
        }
    }

    // Now adjust all our pseudo-styles.
    RenderStyle *pseudoStyle = style->pseudoStyle;
    while (pseudoStyle) {
        adjustRenderStyle(pseudoStyle, 0);
        pseudoStyle = pseudoStyle->pseudoStyle;
    }

    // Now return the style.
    return style;
}

void CSSStyleSelector::adjustRenderStyle(RenderStyle* style, DOM::ElementImpl *e)
{
     // Cache our original display.
     style->setOriginalDisplay(style->display());

    if (style->display() != NONE) {
        // If we have a <td> that specifies a float property, in quirks mode we just drop the float
        // property.
        // Sites also commonly use display:inline/block on <td>s and <table>s.  In quirks mode we force
        // these tags to retain their display types.
        if (!strictParsing && e) {
            if (e->id() == ID_TD) {
                style->setDisplay(TABLE_CELL);
                style->setFloating(FNONE);
            }
            else if (e->id() == ID_TABLE)
                style->setDisplay(style->isDisplayInlineType() ? INLINE_TABLE : TABLE);
        }

        // Table headers with a text-align of auto will change the text-align to center.
        if (e && e->id() == ID_TH && style->textAlign() == TAAUTO)
            style->setTextAlign(CENTER);

        // Mutate the display to BLOCK or TABLE for certain cases, e.g., if someone attempts to
        // position or float an inline, compact, or run-in.  Cache the original display, since it
        // may be needed for positioned elements that have to compute their static normal flow
        // positions.  We also force inline-level roots to be block-level.
        if (style->display() != BLOCK && style->display() != TABLE /*&& style->display() != BOX*/ &&
            (style->position() == ABSOLUTE || style->position() == FIXED || style->floating() != FNONE ||
             (e && e->getDocument()->documentElement() == e))) {
             if (style->display() == INLINE_TABLE)
                 style->setDisplay(TABLE);
//             else if (style->display() == INLINE_BOX)
//                 style->setDisplay(BOX);
            else if (style->display() == LIST_ITEM) {
                // It is a WinIE bug that floated list items lose their bullets, so we'll emulate the quirk,
                // but only in quirks mode.
                if (!strictParsing && style->floating() != FNONE)
                    style->setDisplay(BLOCK);
            }
            else
                style->setDisplay(BLOCK);
        }

        // After performing the display mutation, check our position.  We do not honor position:relative on
        // table rows and some other table displays. This is undefined behaviour in CSS2.1 (cf. 9.3.1)
        if (style->position() == RELATIVE) {
            switch (style->display()) {
              case TABLE_ROW_GROUP:
              case TABLE_HEADER_GROUP:
              case TABLE_FOOTER_GROUP:
              case TABLE_ROW:
                style->setPosition(STATIC);
              default:
                break;
            }
        }
    }

    // Frames and framesets never honor position:relative or position:absolute.  This is necessary to
    // fix a crash where a site tries to position these objects.
    if ( e ) {
        // ignore display: none for <frame>
        if ( e->id() == ID_FRAME ) {
            style->setPosition( STATIC );
            style->setDisplay( BLOCK );
        }
        else if ( e->id() == ID_FRAMESET ) {
            style->setPosition( STATIC );
        }
    }

    // Finally update our text decorations in effect, but don't allow text-decoration to percolate through
    // tables, inline blocks, inline tables, or run-ins.
    if (style->display() == TABLE || style->display() == INLINE_TABLE || style->display() == RUN_IN
        || style->display() == INLINE_BLOCK /*|| style->display() == INLINE_BOX*/)
        style->setTextDecorationsInEffect(style->textDecoration());
    else
        style->addToTextDecorationsInEffect(style->textDecoration());

    // If either overflow value is not visible, change to auto.
    if (style->overflowX() == OMARQUEE && style->overflowY() != OMARQUEE)
        style->setOverflowY(OMARQUEE);
    else if (style->overflowY() == OMARQUEE && style->overflowX() != OMARQUEE)
        style->setOverflowX(OMARQUEE);
    else if (style->overflowX() == OVISIBLE && style->overflowY() != OVISIBLE)
        style->setOverflowX(OAUTO);
    else if (style->overflowY() == OVISIBLE && style->overflowX() != OVISIBLE)
        style->setOverflowY(OAUTO);

    // Table rows, sections and the table itself will support overflow:hidden and will ignore scroll/auto.
    // FIXME: Eventually table sections will support auto and scroll.
    if (style->display() == TABLE || style->display() == INLINE_TABLE ||
        style->display() == TABLE_ROW_GROUP || style->display() == TABLE_ROW) {
        if (style->overflowX() != OVISIBLE && style->overflowX() != OHIDDEN)
            style->setOverflowX(OVISIBLE);
        if (style->overflowY() != OVISIBLE && style->overflowY() != OHIDDEN)
            style->setOverflowY(OVISIBLE);
    }

    // Cull out any useless layers and also repeat patterns into additional layers.
    style->adjustBackgroundLayers();

    // Only use slow repaints if we actually have a background image.
    // FIXME: We only need to invalidate the fixed regions when scrolling.  It's total overkill to
    // prevent the entire view from blitting on a scroll.
    if (style->hasFixedBackgroundImage() && view)
        view->useSlowRepaints();
}

unsigned int CSSStyleSelector::addInlineDeclarations(DOM::ElementImpl* e,
                                                     DOM::CSSStyleDeclarationImpl *decl,
                                                     unsigned int numProps)
{
    CSSStyleDeclarationImpl* addDecls = 0;
#ifdef APPLE_CHANGES
    if (e->id() == ID_TD || e->id() == ID_TH)     // For now only TableCellElement implements the
        addDecls = e->getAdditionalStyleDecls();  // virtual function for shared cell rules.
#else
    Q_UNUSED( e );
#endif

    if (!decl && !addDecls)
        return numProps;

    TQPtrList<CSSProperty>* values = decl ? decl->values() : 0;
    TQPtrList<CSSProperty>* addValues = addDecls ? addDecls->values() : 0;
    if (!values && !addValues)
        return numProps;

    int firstLen = values ? values->count() : 0;
    int secondLen = addValues ? addValues->count() : 0;
    int totalLen = firstLen + secondLen;

    if (inlineProps.size() < (uint)totalLen)
        inlineProps.resize(totalLen + 1);

    if (numProps + totalLen >= propsToApplySize ) {
        propsToApplySize += propsToApplySize;
        propsToApply = (CSSOrderedProperty **)realloc( propsToApply, propsToApplySize*sizeof( CSSOrderedProperty * ) );
    }

    CSSOrderedProperty *array = (CSSOrderedProperty *)inlineProps.data();
    for(int i = 0; i < totalLen; i++)
    {
        if (i == firstLen)
            values = addValues;

        CSSProperty *prop = values->at(i >= firstLen ? i - firstLen : i);
	Source source = Inline;

        if( prop->m_important ) source = InlineImportant;
	if( prop->nonCSSHint ) source = NonCSSHint;

	bool first;
        // give special priority to font-xxx, color properties
        switch(prop->m_id)
        {
        case CSS_PROP_FONT_STYLE:
	case CSS_PROP_FONT_SIZE:
	case CSS_PROP_FONT_WEIGHT:
        case CSS_PROP_FONT_FAMILY:
        case CSS_PROP_FONT_VARIANT:
        case CSS_PROP_FONT:
        case CSS_PROP_COLOR:
        case CSS_PROP_DIRECTION:
        case CSS_PROP_DISPLAY:
            // these have to be applied first, because other properties use the computed
            // values of these properties.
	    first = true;
            break;
        default:
            first = false;
            break;
        }

	array->prop = prop;
	array->pseudoId = RenderStyle::NOPSEUDO;
	array->selector = 0;
	array->position = i;
	array->priority = (!first << 30) | (source << 24);
	propsToApply[numProps++] = array++;
    }
    return numProps;
}

// modified version of the one in kurl.cpp
static void cleanpath(TQString &path)
{
    int pos;
    while ( (pos = path.find( "/../" )) != -1 ) {
        int prev = 0;
        if ( pos > 0 )
            prev = path.findRev( "/", pos -1 );
        // don't remove the host, i.e. http://foo.org/../foo.html
        if (prev < 0 || (prev > 3 && path.findRev("://", prev-1) == prev-2))
            path.remove( pos, 3);
        else
            // matching directory found ?
            path.remove( prev, pos- prev + 3 );
    }
    pos = 0;

    // Don't remove "//" from an anchor identifier. -rjw
    // Set refPos to -2 to mean "I haven't looked for the anchor yet".
    // We don't want to waste a function call on the search for the anchor
    // in the vast majority of cases where there is no "//" in the path.
    int refPos = -2;
    while ( (pos = path.find( "//", pos )) != -1) {
        if (refPos == -2)
            refPos = path.find("#", 0);
        if (refPos > 0 && pos >= refPos)
            break;

        if ( pos == 0 || path[pos-1] != ':' )
            path.remove( pos, 1 );
        else
            pos += 2;
    }
    while ( (pos = path.find( "/./" )) != -1)
        path.remove( pos, 2 );
    //kdDebug() << "checkPseudoState " << path << endl;
}

static PseudoState checkPseudoState( const CSSStyleSelector::Encodedurl& encodedurl, DOM::ElementImpl *e )
{
    if( e->id() != ID_A ) {
        return PseudoNone;
    }
    DOMString attr = e->getAttribute(ATTR_HREF);
    if( attr.isNull() ) {
        return PseudoNone;
    }
    TQConstString cu(attr.unicode(), attr.length());
    TQString u = cu.string();
    if ( !u.contains("://") ) {
        if ( u[0] == '/' )
            u = encodedurl.host + u;
        else if ( u[0] == '#' )
            u = encodedurl.file + u;
        else
            u = encodedurl.path + u;
        cleanpath( u );
    }
    //completeURL( attr.string() );
    bool contains = TDEHTMLFactory::vLinks()->contains( u );
    if ( !contains && u.contains('/')==2 )
      contains = TDEHTMLFactory::vLinks()->contains( u+'/' );
    return contains ? PseudoVisited : PseudoLink;
}

// a helper function for parsing nth-arguments
static bool matchNth(int count, const TQString& nth)
{
    if (nth.isEmpty()) return false;
    int a = 0;
    int b = 0;
    if (nth == "odd") {
        a = 2;
        b = 1;
    }
    else if (nth == "even") {
        a = 2;
        b = 0;
    }
    else {
        int n = nth.find('n');
        if (n != -1) {
            if (nth[0] == '-')
                if (n==1)
                    a = -1;
                else
                    a = nth.mid(1,n-1).toInt();
            else
                if (n==0)
                    a = 1;
                else
                    a = nth.left(n).toInt();

            int p = nth.find('+');
            if (p != -1)
                b = nth.mid(p+1).toInt();
            else {
                p = nth.find('-');
                b = -nth.mid(p+1).toInt();
            }
        }
        else {
            b = nth.toInt();
        }
    }
    if (a == 0)
        return count == b;
    else if (a > 0)
        if (count < b)
            return false;
        else
            return (count - b) % a == 0;
    else if (a < 0) {
        if (count > b)
            return false;
        else
            return (b - count) % (-a) == 0;
    }
    return false;
}


// Recursively work the combinator to compute static attribute dependency, similar to
//structure of checkSubSelectors
static void precomputeAttributeDependenciesAux(DOM::DocumentImpl* doc, DOM::CSSSelector* sel, bool isAncestor, bool isSubject)
{
    if(sel->attr)
    {
        // Sets up global dependencies of attributes
        if (isSubject)
            doc->dynamicDomRestyler().addDependency(sel->attr, PersonalDependency);
        else if (isAncestor)
            doc->dynamicDomRestyler().addDependency(sel->attr, AncestorDependency);
        else
            doc->dynamicDomRestyler().addDependency(sel->attr, PredecessorDependency);
    }
    if(sel->match == CSSSelector::PseudoClass)
    {
	switch (sel->pseudoType()) {
            case CSSSelector::PseudoNot:
                precomputeAttributeDependenciesAux(doc, sel->simpleSelector, isAncestor, true);
                break;
            default:
                break;
        }
    }
    CSSSelector::Relation relation = sel->relation;
    sel = sel->tagHistory;
    if (!sel) return;

    switch(relation)
    {
    case CSSSelector::Descendant:
    case CSSSelector::Child:
        precomputeAttributeDependenciesAux(doc, sel, true, false);
        break;
    case CSSSelector::IndirectAdjacent:
    case CSSSelector::DirectAdjacent:
        precomputeAttributeDependenciesAux(doc, sel, false, false);
        break;
    case CSSSelector::SubSelector:
        precomputeAttributeDependenciesAux(doc, sel, isAncestor, isSubject);
        break;
    }
}

void CSSStyleSelector::precomputeAttributeDependencies(DOM::DocumentImpl* doc, DOM::CSSSelector* sel)
{
    precomputeAttributeDependenciesAux(doc, sel, false, true);
}

// Recursive check of selectors and combinators
// It can return 3 different values:
// * SelectorMatches - the selector is match for the node e
// * SelectorFailsLocal - the selector fails for the node e
// * SelectorFails - the selector fails for e and any sibling or ancestor of e
CSSStyleSelector::SelectorMatch CSSStyleSelector::checkSelector(DOM::CSSSelector *sel, DOM::ElementImpl *e, bool isAncestor, bool isSubSelector)
{
    // The simple selector has to match
    if(!checkSimpleSelector(sel, e, isAncestor, isSubSelector)) return SelectorFailsLocal;

    // The rest of the selectors has to match
    CSSSelector::Relation relation = sel->relation;

    // Prepare next sel
    sel = sel->tagHistory;
    if (!sel) return SelectorMatches;

    switch(relation) {
        case CSSSelector::Descendant:
        {
            while(true)
            {
                DOM::NodeImpl* n = e->parentNode();
                if(!n || !n->isElementNode()) return SelectorFails;
                e = static_cast<ElementImpl *>(n);
                SelectorMatch match = checkSelector(sel, e, true);
                if (match != SelectorFailsLocal)
                    return match;
            }
            break;
        }
        case CSSSelector::Child:
        {
            DOM::NodeImpl* n = e->parentNode();
            if (!strictParsing)
                while (n && n->implicitNode()) n = n->parentNode();
            if(!n || !n->isElementNode()) return SelectorFails;
            e = static_cast<ElementImpl *>(n);
            return checkSelector(sel, e, true);
        }
        case CSSSelector::IndirectAdjacent:
        {
            // Sibling selectors always generate structural dependencies
            // because newly inserted element might fullfill them.
            if (e->parentNode()->isElementNode())
                addDependency(StructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
            while(true)
            {
                DOM::NodeImpl* n = e->previousSibling();
                while( n && !n->isElementNode() )
                    n = n->previousSibling();
                if( !n ) return SelectorFailsLocal;
                e = static_cast<ElementImpl *>(n);
                SelectorMatch match = checkSelector(sel, e, false);
                if (match != SelectorFailsLocal)
                    return match;
            };
            break;
        }
        case CSSSelector::DirectAdjacent:
        {
            if (e->parentNode()->isElementNode())
                addDependency(StructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
            DOM::NodeImpl* n = e->previousSibling();
            while( n && !n->isElementNode() )
                n = n->previousSibling();
            if( !n ) return SelectorFailsLocal;
            e = static_cast<ElementImpl *>(n);
            return checkSelector(sel, e, false);
        }
        case CSSSelector::SubSelector:
            return checkSelector(sel, e, isAncestor, true);
    }
    assert(false); // never reached
    return SelectorFails;
}

void CSSStyleSelector::checkSelector(int selIndex, DOM::ElementImpl * e)
{
    assert(e == element); // yes, actually

    dynamicPseudo = RenderStyle::NOPSEUDO;

    selectorCache[ selIndex ].state = Invalid;
    CSSSelector *sel = selectors[ selIndex ];

    // Check the selector
     SelectorMatch match = checkSelector(sel, e, true);
     if(match != SelectorMatches) return;

    if ( dynamicPseudo != RenderStyle::NOPSEUDO ) {
	selectorCache[selIndex].state = AppliesPseudo;
	selectors[ selIndex ]->pseudoId = dynamicPseudo;
    } else
	selectorCache[ selIndex ].state = Applies;
    //tqDebug( "selector %d applies", selIndex );
    //selectors[ selIndex ]->print();
    return;
}

void CSSStyleSelector::addDependency(StructuralDependencyType dependencyType, ElementImpl* dependency)
{
    element->getDocument()->dynamicDomRestyler().addDependency(element, dependency, dependencyType);
}

bool CSSStyleSelector::checkSimpleSelector(DOM::CSSSelector *sel, DOM::ElementImpl *e, bool isAncestor, bool isSubSelector)
{
    if(!e)
        return false;

    if (sel->tag != anyQName) {
        int eltID = e->id();
        TQ_UINT16 localName = localNamePart(eltID);
        TQ_UINT16 ns = namespacePart(eltID);
        TQ_UINT16 selLocalName = localNamePart(sel->tag);
        TQ_UINT16 selNS = namespacePart(sel->tag);

        if (localName <= ID_LAST_TAG && ns == defaultNamespace) {
            assert(e->isHTMLElement());
            ns = xhtmlNamespace;
        }

        // match on local
        if (selLocalName != anyLocalName && localName != selLocalName) return false;
        // match on namespace
        if (selNS != anyNamespace && ns != selNS) return false;
    }

    if(sel->attr)
    {
        DOMStringImpl* value = e->getAttributeImpl(sel->attr);
        if(!value) return false; // attribute is not set

        // attributes are always case-sensitive in XHTML
        // attributes are sometimes case-sensitive in HTML
        // we only treat id and class selectors as case-sensitive in HTML strict
        // for compatibility reasons
        bool caseSensitive = e->getDocument()->htmlMode() == DocumentImpl::XHtml;
        bool caseSensitive_alt = strictParsing || caseSensitive;
        caseSensitive |= (sel->attr > ATTR_LAST_CI_ATTR);

        switch(sel->match)
        {
        case CSSSelector::Set:
            // True if we make it this far
            break;
        case CSSSelector::Id:
            caseSensitive = caseSensitive_alt;
            // no break
        case CSSSelector::Exact:
            return (caseSensitive && !strcmp(sel->value, value)) ||
                   (!caseSensitive && !strcasecmp(sel->value, value));
            break;
        case CSSSelector::Class:
            caseSensitive = caseSensitive_alt;
            // no break
        case CSSSelector::List:
        {
            int sel_len = sel->value.length();
            int val_len = value->length();
            // Be smart compare on length first
            if (sel_len > val_len) return false;
            // Selector string may not contain spaces
            if ((sel->attr != ATTR_CLASS || e->hasClassList()) && sel->value.find(' ') != -1) return false;
            if (sel_len == val_len)
                return (caseSensitive && !strcmp(sel->value, value)) ||
		       (!caseSensitive && !strcasecmp(sel->value, value));
            // else the value is longer and can be a list
            if ( sel->match == CSSSelector::Class && !e->hasClassList() ) return false;

            TQChar* sel_uc = sel->value.unicode();
            TQChar* val_uc = value->unicode();

            TQConstString sel_str(sel_uc, sel_len);
            TQConstString val_str(val_uc, val_len);

            int pos = 0;
            for ( ;; ) {
                pos = val_str.string().find(sel_str.string(), pos, caseSensitive);
                if ( pos == -1 ) return false;
                if ( pos == 0 || val_uc[pos-1].isSpace() ) {
                    int endpos = pos + sel_len;
                    if ( endpos >= val_len || val_uc[endpos].isSpace() )
                        break; // We have a match.
                }
                ++pos;
            }
            break;
        }
        case CSSSelector::Contain:
        {
            //kdDebug( 6080 ) << "checking for contains match" << endl;
            TQConstString val_str(value->unicode(), value->length());
            TQConstString sel_str(sel->value.unicode(), sel->value.length());
            return val_str.string().contains(sel_str.string(), caseSensitive);
        }
        case CSSSelector::Begin:
        {
            //kdDebug( 6080 ) << "checking for beginswith match" << endl;
            TQConstString val_str(value->unicode(), value->length());
            TQConstString sel_str(sel->value.unicode(), sel->value.length());
            return val_str.string().startsWith(sel_str.string(), caseSensitive);
        }
        case CSSSelector::End:
        {
            //kdDebug( 6080 ) << "checking for endswith match" << endl;
            TQConstString val_str(value->unicode(), value->length());
            TQConstString sel_str(sel->value.unicode(), sel->value.length());
            return val_str.string().endsWith(sel_str.string(), caseSensitive);
        }
        case CSSSelector::Hyphen:
        {
            //kdDebug( 6080 ) << "checking for hyphen match" << endl;
            TQConstString val_str(value->unicode(), value->length());
            TQConstString sel_str(sel->value.unicode(), sel->value.length());
            const TQString& str = val_str.string();
            const TQString& selStr = sel_str.string();
            if(str.length() < selStr.length()) return false;
            // Check if str begins with selStr:
            if(str.find(selStr, 0, caseSensitive) != 0) return false;
            // It does. Check for exact match or following '-':
            if(str.length() != selStr.length()
                && str[selStr.length()] != '-') return false;
            break;
        }
        case CSSSelector::PseudoClass:
        case CSSSelector::PseudoElement:
        case CSSSelector::None:
            break;
        }
    }

    if(sel->match == CSSSelector::PseudoClass || sel->match == CSSSelector::PseudoElement)
    {
	switch (sel->pseudoType()) {
        // Pseudo classes:
	case CSSSelector::PseudoEmpty:
	    addDependency(BackwardsStructuralDependency, e);
            // If e is not closed yet we don't know the number of children
            if (!e->closed()) {
                return false;
            }
            if (!e->firstChild())
                return true;
            else {
                // check for empty text nodes
                NodeImpl *t = e->firstChild();

                while (t && t->isTextNode() && static_cast<TextImpl*>(t)->length() == 0) t = t->nextSibling();

                if (t == 0)
                    return true;
                else
                    return false;
            }
            break;
	case CSSSelector::PseudoFirstChild: {
	    // first-child matches the first child that is an element!
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                // Handle dynamic DOM changes
                addDependency(StructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                DOM::NodeImpl* n = e->previousSibling();
                while ( n && !n->isElementNode() )
                    n = n->previousSibling();
                if ( !n )
                    return true;
            }
            break;
        }
        case CSSSelector::PseudoLastChild: {
            // last-child matches the last child that is an element!
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                // Handle unfinished parsing and dynamic DOM changes
                addDependency(BackwardsStructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                if (!e->parentNode()->closed()) {
//                     kdDebug(6080) << e->nodeName().string() << "::last-child: Parent unclosed" << endl;
                    return false;
                }
                DOM::NodeImpl* n = e->nextSibling();
                while ( n && !n->isElementNode() )
                    n = n->nextSibling();
                if ( !n )
                    return true;
            }
            break;
        }
        case CSSSelector::PseudoOnlyChild: {
            // If both first-child and last-child apply, then only-child applies.
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                addDependency(BackwardsStructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                if (!e->parentNode()->closed()) {
                    return false;
                }
                DOM::NodeImpl* n = e->previousSibling();
                while ( n && !n->isElementNode() )
                    n = n->previousSibling();
                if ( !n ) {
                    n = e->nextSibling();
                    while ( n && !n->isElementNode() )
                        n = n->nextSibling();
                    if ( !n )
                        return true;
                }
            }
            break;
        }
        case CSSSelector::PseudoNthChild: {
	    // nth-child matches every (a*n+b)th element!
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                addDependency(StructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                int count = 1;
                DOM::NodeImpl* n = e->previousSibling();
                while ( n ) {
                    if (n->isElementNode()) count++;
                    n = n->previousSibling();
                }
//                 kdDebug(6080) << "NthChild " << count << "=" << sel->string_arg << endl;
                if (matchNth(count,sel->string_arg.string()))
                    return true;
            }
            break;
        }
        case CSSSelector::PseudoNthLastChild: {
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                addDependency(BackwardsStructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                if (!e->parentNode()->closed()) {
                    return false;
                }
                int count = 1;
                DOM::NodeImpl* n = e->nextSibling();
                while ( n ) {
                    if (n->isElementNode()) count++;
                    n = n->nextSibling();
                }
//                kdDebug(6080) << "NthLastChild " << count << "=" << sel->string_arg << endl;
                if (matchNth(count,sel->string_arg.string()))
                    return true;
            }
            break;
        }
	case CSSSelector::PseudoFirstOfType: {
	    // first-of-type matches the first element of its type!
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                addDependency(StructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                const DOMString& type = e->tagName();
                DOM::NodeImpl* n = e->previousSibling();
                while ( n ) {
                    if (n->isElementNode())
                        if (static_cast<ElementImpl*>(n)->tagName() == type) break;
                    n = n->previousSibling();
                }
                if ( !n )
                    return true;
            }
            break;
        }
        case CSSSelector::PseudoLastOfType: {
            // last-child matches the last child that is an element!
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                addDependency(BackwardsStructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                if (!e->parentNode()->closed()) {
                    return false;
                }
                const DOMString& type = e->tagName();
                DOM::NodeImpl* n = e->nextSibling();
                while ( n ) {
                    if (n->isElementNode())
                        if (static_cast<ElementImpl*>(n)->tagName() == type) break;
                    n = n->nextSibling();
                }
                if ( !n )
                    return true;
            }
            break;
        }
        case CSSSelector::PseudoOnlyOfType: {
            // If both first-of-type and last-of-type apply, then only-of-type applies.
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                addDependency(BackwardsStructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                if (!e->parentNode()->closed()) {
                    return false;
                }
                const DOMString& type = e->tagName();
                DOM::NodeImpl* n = e->previousSibling();
                while ( n && !(n->isElementNode() && static_cast<ElementImpl*>(n)->tagName() == type))
                    n = n->previousSibling();
                if ( !n ) {
                    n = e->nextSibling();
                    while ( n && !(n->isElementNode() && static_cast<ElementImpl*>(n)->tagName() == type))
                        n = n->nextSibling();
                    if ( !n )
                        return true;
	        }
            }
	    break;
        }
        case CSSSelector::PseudoNthOfType: {
	    // nth-of-type matches every (a*n+b)th element of this type!
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                addDependency(StructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                int count = 1;
                const DOMString& type = e->tagName();
                DOM::NodeImpl* n = e->previousSibling();
                while ( n ) {
                    if (n->isElementNode() && static_cast<ElementImpl*>(n)->tagName() == type) count++;
                    n = n->previousSibling();
                }
//                kdDebug(6080) << "NthOfType " << count << "=" << sel->string_arg << endl;
                if (matchNth(count,sel->string_arg.string()))
                    return true;
            }
            break;
        }
        case CSSSelector::PseudoNthLastOfType: {
            if (e->parentNode() && e->parentNode()->isElementNode()) {
                addDependency(BackwardsStructuralDependency, static_cast<ElementImpl*>(e->parentNode()));
                if (!e->parentNode()->closed()) {
                    return false;
                }
                int count = 1;
                const DOMString& type = e->tagName();
                DOM::NodeImpl* n = e->nextSibling();
                while ( n ) {
                    if (n->isElementNode() && static_cast<ElementImpl*>(n)->tagName() == type) count++;
                    n = n->nextSibling();
                }
//                kdDebug(6080) << "NthLastOfType " << count << "=" << sel->string_arg << endl;
                if (matchNth(count,sel->string_arg.string()))
                    return true;
            }
            break;
        }
        case CSSSelector::PseudoTarget:
            if (e == e->getDocument()->getCSSTarget())
                return true;
            break;
        case CSSSelector::PseudoRoot:
            if (e == e->getDocument()->documentElement())
                return true;
            break;
	case CSSSelector::PseudoLink:
	    if (e == element) {
	       // cache pseudoState
	       if ( pseudoState == PseudoUnknown )
                    pseudoState = checkPseudoState( encodedurl, e );
               if ( pseudoState == PseudoLink )
                    return true;
            } else
                return checkPseudoState( encodedurl, e ) == PseudoLink;
	    break;
	case CSSSelector::PseudoVisited:
	    if (e == element) {
                // cache pseudoState
                if ( pseudoState == PseudoUnknown )
                    pseudoState = checkPseudoState( encodedurl, e );
                if ( pseudoState == PseudoVisited )
                    return true;
            } else
                return checkPseudoState( encodedurl, e ) == PseudoVisited;
	    break;
        case CSSSelector::PseudoHover: {
	    // If we're in quirks mode, then *:hover should only match focusable elements.
	    if (strictParsing || (sel->tag != anyQName) || isSubSelector || e->isFocusable() ) {
                addDependency(HoverDependency, e);

                if (e->hovered())
                    return true;
            }
            break;
        }
	case CSSSelector::PseudoActive:
	    // If we're in quirks mode, then *:active should only match focusable elements
	    if (strictParsing || (sel->tag != anyQName) || isSubSelector || e->isFocusable()) {
                addDependency(ActiveDependency, e);

		if (e->active())
		    return true;
	    }
	    break;
	case CSSSelector::PseudoFocus:
	    if (e != element && e->isFocusable()) {
                // *:focus is a default style, no need to track it.
                addDependency(OtherStateDependency, e);
            }
            if (e->focused()) return true;
            break;
        case CSSSelector::PseudoLang: {
            // Set dynamic attribute dependency
            if (e == element) {
                e->getDocument()->dynamicDomRestyler().addDependency(ATTR_LANG, PersonalDependency);
                e->getDocument()->dynamicDomRestyler().addDependency(ATTR_LANG, AncestorDependency);
            }
            else if (isAncestor)
                e->getDocument()->dynamicDomRestyler().addDependency(ATTR_LANG, AncestorDependency);
            else
                e->getDocument()->dynamicDomRestyler().addDependency(ATTR_LANG, PredecessorDependency);
            // ### check xml:lang attribute in XML and XHTML documents
            DOMString value = e->getAttribute(ATTR_LANG);
            // The LANG attribute is inherited like a property
            NodeImpl *n = e->parent();;
            while (n && value.isEmpty()) {
                if (n->isElementNode()) {
                    value = static_cast<ElementImpl*>(n)->getAttribute(ATTR_LANG);
                } else
                if (n->isDocumentNode()) {
                    value = static_cast<DocumentImpl*>(n)->contentLanguage();
                }
                n = n->parent();
            }
            if (value.isEmpty()) return false;

            TQString langAttr = value.string();
            TQString langSel = sel->string_arg.string();

            if(langAttr.length() < langSel.length()) return false;

            langAttr = langAttr.lower();
            langSel = langSel.lower();
//             kdDebug(6080) << ":lang " << langAttr << "=" << langSel << "?" << endl;
            return (langAttr == langSel || langAttr.startsWith(langSel+"-"));
        }
        case CSSSelector::PseudoNot: {
            // check the simple selector
            for (CSSSelector* subSel = sel->simpleSelector; subSel;
                 subSel = subSel->tagHistory) {
                // :not cannot nest.  I don't really know why this is a restriction in CSS3,
                // but it is, so let's honor it.
                if (subSel->simpleSelector)
                    break;
                if (!checkSimpleSelector(subSel, e, isAncestor, true))
                    return true;
            }
            break;
        }
        case CSSSelector::PseudoEnabled: {
            if (e->isGenericFormElement()) {
                addDependency(OtherStateDependency, e);
                HTMLGenericFormElementImpl *form;
                form = static_cast<HTMLGenericFormElementImpl*>(e);
                return !form->disabled();
            }
            break;
        }
        case CSSSelector::PseudoDisabled: {
            if (e->isGenericFormElement()) {
                addDependency(OtherStateDependency, e);
                HTMLGenericFormElementImpl *form;
                form = static_cast<HTMLGenericFormElementImpl*>(e);
                return form->disabled();
            }
            break;
        }
        case CSSSelector::PseudoContains: {
            if (e->isHTMLElement()) {
                addDependency(BackwardsStructuralDependency, e);
                if (!e->closed()) {
                   return false;
                }
                HTMLElementImpl *elem;
                elem = static_cast<HTMLElementImpl*>(e);
                DOMString s = elem->innerText();
                TQString selStr = sel->string_arg.string();
//                kdDebug(6080) << ":contains(\"" << selStr << "\")" << " on \"" << s << "\"" << endl;
                return s.string().contains(selStr);
            }
            break;
        }
	case CSSSelector::PseudoChecked: {
           if (e->isHTMLElement() && e->id() == ID_INPUT) {
               addDependency(OtherStateDependency, e);
               return (static_cast<HTMLInputElementImpl*>(e)->checked());
           }
           return false;
        }
 	case CSSSelector::PseudoIndeterminate: {
#if 0
           if (e->isHTMLElement() && e->id() == ID_INPUT) {
               return (static_cast<HTMLInputElementImpl*>(e)->indeterminate() &&
                      !static_cast<HTMLInputElementImpl*>(e)->checked());
           }
           return false;
#endif
        }
	case CSSSelector::PseudoOther:
	    break;

	// Pseudo-elements:
	case CSSSelector::PseudoFirstLine:
	case CSSSelector::PseudoFirstLetter:
	case CSSSelector::PseudoSelection:
	case CSSSelector::PseudoBefore:
	case CSSSelector::PseudoAfter:
	case CSSSelector::PseudoMarker:
	case CSSSelector::PseudoReplaced:
	    // Pseudo-elements can only apply to subject
	    if ( e == element ) {
                // Pseudo-elements has to be the last sub-selector on subject
                if (sel->tagHistory && sel->relation == CSSSelector::SubSelector) return false;

                assert(dynamicPseudo == RenderStyle::NOPSEUDO);

                switch (sel->pseudoType()) {
                case CSSSelector::PseudoFirstLine:
                    dynamicPseudo = RenderStyle::FIRST_LINE;
                    break;
                case CSSSelector::PseudoFirstLetter:
                    dynamicPseudo = RenderStyle::FIRST_LETTER;
                    break;
                case CSSSelector::PseudoSelection:
                    dynamicPseudo = RenderStyle::SELECTION;
                    break;
                case CSSSelector::PseudoBefore:
                    dynamicPseudo = RenderStyle::BEFORE;
                    break;
                case CSSSelector::PseudoAfter:
                    dynamicPseudo = RenderStyle::AFTER;
                    break;
                case CSSSelector::PseudoMarker:
                    dynamicPseudo = RenderStyle::MARKER;
                    break;
                case CSSSelector::PseudoReplaced:
                    dynamicPseudo = RenderStyle::REPLACED;
                    break;
                default:
                    assert(false);
                }
		return true;
	    }
	    break;
	case CSSSelector::PseudoNotParsed:
	    assert(false);
	    break;
	}
	return false;
    }
    // ### add the rest of the checks...
    return true;
}

void CSSStyleSelector::clearLists()
{
    delete [] selectors;
    if ( selectorCache ) {
        for ( unsigned int i = 0; i < selectors_size; i++ )
            delete [] selectorCache[i].props;

        delete [] selectorCache;
    }
    if ( properties ) {
	CSSOrderedProperty **prop = properties;
	while ( *prop ) {
	    delete (*prop);
	    prop++;
	}
        delete [] properties;
    }
    selectors = 0;
    properties = 0;
    selectorCache = 0;
}


void CSSStyleSelector::buildLists()
{
    clearLists();
    // collect all selectors and Properties in lists. Then transfer them to the array for faster lookup.

    TQPtrList<CSSSelector> selectorList;
    CSSOrderedPropertyList propertyList;

    if(m_medium == "print" && defaultPrintStyle)
      defaultPrintStyle->collect( &selectorList, &propertyList, Default,
        Default );
    else if(defaultStyle) defaultStyle->collect( &selectorList, &propertyList,
      Default, Default );

    if (!strictParsing && defaultQuirksStyle)
        defaultQuirksStyle->collect( &selectorList, &propertyList, Default, Default );

    if(userStyle) userStyle->collect(&selectorList, &propertyList, User, UserImportant );
    if(authorStyle) authorStyle->collect(&selectorList, &propertyList, Author, AuthorImportant );

    selectors_size = selectorList.count();
    selectors = new CSSSelector *[selectors_size];
    CSSSelector *s = selectorList.first();
    CSSSelector **sel = selectors;
    while ( s ) {
	*sel = s;
	s = selectorList.next();
	++sel;
    }

    selectorCache = new SelectorCache[selectors_size];
    for ( unsigned int i = 0; i < selectors_size; i++ ) {
        selectorCache[i].state = Unknown;
        selectorCache[i].props_size = 0;
        selectorCache[i].props = 0;
    }

    // presort properties. Should make the sort() calls in styleForElement faster.
    propertyList.sort();
    properties_size = propertyList.count() + 1;
    properties = new CSSOrderedProperty *[ properties_size ];
    CSSOrderedProperty *p = propertyList.first();
    CSSOrderedProperty **prop = properties;
    while ( p ) {
	*prop = p;
	p = propertyList.next();
	++prop;
    }
    *prop = 0;

    unsigned int* offsets = new unsigned int[selectors_size];
    if(properties[0])
	offsets[properties[0]->selector] = 0;
    for(unsigned int p = 1; p < properties_size; ++p) {

	if(!properties[p] || (properties[p]->selector != properties[p - 1]->selector)) {
	    unsigned int sel = properties[p - 1]->selector;
            int* newprops = new int[selectorCache[sel].props_size+2];
            for ( unsigned int i=0; i < selectorCache[sel].props_size; i++ )
                newprops[i] = selectorCache[sel].props[i];

	    newprops[selectorCache[sel].props_size] = offsets[sel];
	    newprops[selectorCache[sel].props_size+1] = p - offsets[sel];
            delete [] selectorCache[sel].props;
            selectorCache[sel].props = newprops;
            selectorCache[sel].props_size += 2;

	    if(properties[p]) {
		sel = properties[p]->selector;
		offsets[sel] = p;
            }
        }
    }
    delete [] offsets;
}


// ----------------------------------------------------------------------


CSSOrderedRule::CSSOrderedRule(DOM::CSSStyleRuleImpl *r, DOM::CSSSelector *s, int _index)
{
    rule = r;
    if(rule) r->ref();
    index = _index;
    selector = s;
}

CSSOrderedRule::~CSSOrderedRule()
{
    if(rule) rule->deref();
}

// -----------------------------------------------------------------

CSSStyleSelectorList::CSSStyleSelectorList()
    : TQPtrList<CSSOrderedRule>()
{
    setAutoDelete(true);
}
CSSStyleSelectorList::~CSSStyleSelectorList()
{
}

void CSSStyleSelectorList::append( CSSStyleSheetImpl *sheet,
                                   const DOMString &medium )
{
    if(!sheet || !sheet->isCSSStyleSheet()) return;

    // No media implies "all", but if a medialist exists it must
    // contain our current medium
    if( sheet->media() && !sheet->media()->contains( medium ) )
        return; // style sheet not applicable for this medium

    int len = sheet->length();

    for(int i = 0; i< len; i++)
    {
        StyleBaseImpl *item = sheet->item(i);
        if(item->isStyleRule())
        {
            CSSStyleRuleImpl *r = static_cast<CSSStyleRuleImpl *>(item);
            TQPtrList<CSSSelector> *s = r->selector();
            for(int j = 0; j < (int)s->count(); j++)
            {
                CSSOrderedRule *rule = new CSSOrderedRule(r, s->at(j), count());
		TQPtrList<CSSOrderedRule>::append(rule);
                //kdDebug( 6080 ) << "appending StyleRule!" << endl;
            }
        }
        else if(item->isImportRule())
        {
            CSSImportRuleImpl *import = static_cast<CSSImportRuleImpl *>(item);

            //kdDebug( 6080 ) << "@import: Media: "
            //                << import->media()->mediaText().string() << endl;

            if( !import->media() || import->media()->contains( medium ) )
            {
                CSSStyleSheetImpl *importedSheet = import->styleSheet();
                append( importedSheet, medium );
            }
        }
        else if( item->isMediaRule() )
        {
            CSSMediaRuleImpl *r = static_cast<CSSMediaRuleImpl *>( item );
            CSSRuleListImpl *rules = r->cssRules();

            //DOMString mediaText = media->mediaText();
            //kdDebug( 6080 ) << "@media: Media: "
            //                << r->media()->mediaText().string() << endl;

            if( ( !r->media() || r->media()->contains( medium ) ) && rules)
            {
                // Traverse child elements of the @import rule. Since
                // many elements are not allowed as child we do not use
                // a recursive call to append() here
                for( unsigned j = 0; j < rules->length(); j++ )
                {
                    //kdDebug( 6080 ) << "*** Rule #" << j << endl;

                    CSSRuleImpl *childItem = rules->item( j );
                    if( childItem->isStyleRule() )
                    {
                        // It is a StyleRule, so append it to our list
                        CSSStyleRuleImpl *styleRule =
                                static_cast<CSSStyleRuleImpl *>( childItem );

                        TQPtrList<CSSSelector> *s = styleRule->selector();
                        for( int j = 0; j < ( int ) s->count(); j++ )
                        {
                            CSSOrderedRule *orderedRule = new CSSOrderedRule(
                                            styleRule, s->at( j ), count() );
                	    TQPtrList<CSSOrderedRule>::append( orderedRule );
                        }
                    }
                    else
                    {
                        //kdDebug( 6080 ) << "Ignoring child rule of "
                        //    "ImportRule: rule is not a StyleRule!" << endl;
                    }
                }   // for rules
            }   // if rules
            else
            {
                //kdDebug( 6080 ) << "CSSMediaRule not rendered: "
                //                << "rule empty or wrong medium!" << endl;
            }
        }
        // ### include other rules
    }
}


void CSSStyleSelectorList::collect( TQPtrList<CSSSelector> *selectorList, CSSOrderedPropertyList *propList,
				    Source regular, Source important )
{
    CSSOrderedRule *r = first();
    while( r ) {
	CSSSelector *sel = selectorList->first();
	int selectorNum = 0;
	while( sel ) {
	    if ( *sel == *(r->selector) )
		break;
	    sel = selectorList->next();
	    selectorNum++;
	}
	if ( !sel )
	    selectorList->append( r->selector );
//	else
//	    tqDebug("merged one selector");
	propList->append(r->rule->declaration(), selectorNum, r->selector->specificity(), regular, important );
	r = next();
    }
}

// -------------------------------------------------------------------------

int CSSOrderedPropertyList::compareItems(TQPtrCollection::Item i1, TQPtrCollection::Item i2)
{
    int diff =  static_cast<CSSOrderedProperty *>(i1)->priority
        - static_cast<CSSOrderedProperty *>(i2)->priority;
    return diff ? diff : static_cast<CSSOrderedProperty *>(i1)->position
        - static_cast<CSSOrderedProperty *>(i2)->position;
}

void CSSOrderedPropertyList::append(DOM::CSSStyleDeclarationImpl *decl, uint selector, uint specificity,
				    Source regular, Source important )
{
    TQPtrList<CSSProperty> *values = decl->values();
    if(!values) return;
    int len = values->count();
    for(int i = 0; i < len; i++)
    {
        CSSProperty *prop = values->at(i);
	Source source = regular;

	if( prop->m_important ) source = important;
	if( prop->nonCSSHint ) source = NonCSSHint;

	bool first = false;
        // give special priority to font-xxx, color properties
        switch(prop->m_id)
        {
        case CSS_PROP_FONT_STYLE:
	case CSS_PROP_FONT_SIZE:
	case CSS_PROP_FONT_WEIGHT:
        case CSS_PROP_FONT_FAMILY:
        case CSS_PROP_FONT_VARIANT:
        case CSS_PROP_FONT:
        case CSS_PROP_COLOR:
        case CSS_PROP_DIRECTION:
        case CSS_PROP_DISPLAY:
            // these have to be applied first, because other properties use the computed
            // values of these porperties.
	    first = true;
            break;
        default:
            break;
        }

	TQPtrList<CSSOrderedProperty>::append(new CSSOrderedProperty(prop, selector,
								 first, source, specificity,
								 count() ));
    }
}

// -------------------------------------------------------------------------------------
// this is mostly boring stuff on how to apply a certain rule to the renderstyle...

static Length convertToLength( CSSPrimitiveValueImpl *primitiveValue, RenderStyle *style, TQPaintDeviceMetrics *paintDeviceMetrics, bool *ok = 0 )
{
    Length l;
    if ( !primitiveValue ) {
	if ( ok )
            *ok = false;
    } else {
	int type = primitiveValue->primitiveType();
	if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
	    l = Length(primitiveValue->computeLength(style, paintDeviceMetrics), Fixed);
	else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
	    l = Length(int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE)), Percent);
	else if(type == CSSPrimitiveValue::CSS_NUMBER)
	    l = Length(int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_NUMBER)*100), Percent);
	else if (type == CSSPrimitiveValue::CSS_HTML_RELATIVE)
	    l = Length(int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_HTML_RELATIVE)), Relative);
	else if ( ok )
	    *ok = false;
    }
    return l;
}


// color mapping code
struct colorMap {
    int css_value;
    TQRgb color;
};

static const colorMap cmap[] = {
    { CSS_VAL_AQUA, 0xFF00FFFF },
    { CSS_VAL_BLACK, 0xFF000000 },
    { CSS_VAL_BLUE, 0xFF0000FF },
    { CSS_VAL_CRIMSON, 0xFFDC143C },
    { CSS_VAL_FUCHSIA, 0xFFFF00FF },
    { CSS_VAL_GRAY, 0xFF808080 },
    { CSS_VAL_GREEN, 0xFF008000  },
    { CSS_VAL_INDIGO, 0xFF4B0082 },
    { CSS_VAL_LIME, 0xFF00FF00 },
    { CSS_VAL_MAROON, 0xFF800000 },
    { CSS_VAL_NAVY, 0xFF000080 },
    { CSS_VAL_OLIVE, 0xFF808000  },
    { CSS_VAL_ORANGE, 0xFFFFA500 },
    { CSS_VAL_PURPLE, 0xFF800080 },
    { CSS_VAL_RED, 0xFFFF0000 },
    { CSS_VAL_SILVER, 0xFFC0C0C0 },
    { CSS_VAL_TEAL, 0xFF008080  },
    { CSS_VAL_WHITE, 0xFFFFFFFF },
    { CSS_VAL_YELLOW, 0xFFFFFF00 },
    { CSS_VAL_INVERT, invertedColor },
    { CSS_VAL_TRANSPARENT, transparentColor },
    { CSS_VAL_GREY, 0xff808080 },
    { 0, 0 }
};

struct uiColors {
    int css_value;
    const char * configGroup;
    const char * configEntry;
TQPalette::ColorGroup group;
TQColorGroup::ColorRole role;
};

const char * const wmgroup = "WM";
const char * const generalgroup = "General";

/* Mapping system settings to CSS 2
* Tried hard to get an appropriate mapping - schlpbch
*/
static const uiColors uimap[] = {
	// Active window border.
    { CSS_VAL_ACTIVEBORDER, wmgroup, "background", TQPalette::Active, TQColorGroup::Light },
	// Active window caption.
    { CSS_VAL_ACTIVECAPTION, wmgroup, "background", TQPalette::Active, TQColorGroup::Text },
        // Text in caption, size box, and scrollbar arrow box.
    { CSS_VAL_CAPTIONTEXT, wmgroup, "activeForeground", TQPalette::Active, TQColorGroup::Text },
	// Face color for three-dimensional display elements.
    { CSS_VAL_BUTTONFACE, wmgroup, 0, TQPalette::Inactive, TQColorGroup::Button },
	// Dark shadow for three-dimensional display elements (for edges facing away from the light source).
    { CSS_VAL_BUTTONHIGHLIGHT, wmgroup, 0, TQPalette::Inactive, TQColorGroup::Light },
	// Shadow color for three-dimensional display elements.
    { CSS_VAL_BUTTONSHADOW, wmgroup, 0, TQPalette::Inactive, TQColorGroup::Shadow },
	// Text on push buttons.
    { CSS_VAL_BUTTONTEXT, wmgroup, "buttonForeground", TQPalette::Inactive, TQColorGroup::ButtonText },
	// Dark shadow for three-dimensional display elements.
    { CSS_VAL_THREEDDARKSHADOW, wmgroup, 0, TQPalette::Inactive, TQColorGroup::Dark },
	// Face color for three-dimensional display elements.
    { CSS_VAL_THREEDFACE, wmgroup, 0, TQPalette::Inactive, TQColorGroup::Button },
	// Highlight color for three-dimensional display elements.
    { CSS_VAL_THREEDHIGHLIGHT, wmgroup, 0, TQPalette::Inactive, TQColorGroup::Light },
	// Light color for three-dimensional display elements (for edges facing the light source).
    { CSS_VAL_THREEDLIGHTSHADOW, wmgroup, 0, TQPalette::Inactive, TQColorGroup::Midlight },
	// Dark shadow for three-dimensional display elements.
    { CSS_VAL_THREEDSHADOW, wmgroup, 0, TQPalette::Inactive, TQColorGroup::Shadow },

    // Inactive window border.
    { CSS_VAL_INACTIVEBORDER, wmgroup, "background", TQPalette::Disabled, TQColorGroup::Background },
    // Inactive window caption.
    { CSS_VAL_INACTIVECAPTION, wmgroup, "inactiveBackground", TQPalette::Disabled, TQColorGroup::Background },
    // Color of text in an inactive caption.
    { CSS_VAL_INACTIVECAPTIONTEXT, wmgroup, "inactiveForeground", TQPalette::Disabled, TQColorGroup::Text },
    { CSS_VAL_GRAYTEXT, wmgroup, 0, TQPalette::Disabled, TQColorGroup::Text },

	// Menu background
    { CSS_VAL_MENU, generalgroup, "background", TQPalette::Inactive, TQColorGroup::Background },
	// Text in menus
    { CSS_VAL_MENUTEXT, generalgroup, "foreground", TQPalette::Inactive, TQColorGroup::Background },

        // Text of item(s) selected in a control.
    { CSS_VAL_HIGHLIGHT, generalgroup, "selectBackground", TQPalette::Inactive, TQColorGroup::Background },

    // Text of item(s) selected in a control.
    { CSS_VAL_HIGHLIGHTTEXT, generalgroup, "selectForeground", TQPalette::Inactive, TQColorGroup::Background },

	// Background color of multiple document interface.
    { CSS_VAL_APPWORKSPACE, generalgroup, "background", TQPalette::Inactive, TQColorGroup::Text },

	// Scroll bar gray area.
    { CSS_VAL_SCROLLBAR, generalgroup, "background", TQPalette::Inactive, TQColorGroup::Background },

	// Window background.
    { CSS_VAL_WINDOW, generalgroup, "windowBackground", TQPalette::Inactive, TQColorGroup::Background },
	// Window frame.
    { CSS_VAL_WINDOWFRAME, generalgroup, "windowBackground", TQPalette::Inactive, TQColorGroup::Background },
        // WindowText
    { CSS_VAL_WINDOWTEXT, generalgroup, "windowForeground", TQPalette::Inactive, TQColorGroup::Text },
    { CSS_VAL_TEXT, generalgroup, 0, TQPalette::Inactive, TQColorGroup::Text },
    { 0, 0, 0, TQPalette::NColorGroups, TQColorGroup::NColorRoles }
};

static TQColor colorForCSSValue( int css_value )
{
    // try the regular ones first
    const colorMap *col = cmap;
    while ( col->css_value && col->css_value != css_value )
	++col;
    if ( col->css_value )
	return col->color;

    const uiColors *uicol = uimap;
    while ( uicol->css_value && uicol->css_value != css_value )
	++uicol;
#ifndef APPLE_CHANGES
    if ( !uicol->css_value ) {
	if ( css_value == CSS_VAL_INFOBACKGROUND )
	    return TQToolTip::palette().inactive().background();
	else if ( css_value == CSS_VAL_INFOTEXT )
	    return TQToolTip::palette().inactive().foreground();
	else if ( css_value == CSS_VAL_BACKGROUND ) {
	    TDEConfig bckgrConfig("kdesktoprc", true, false); // No multi-screen support
	    bckgrConfig.setGroup("Desktop0");
	    // Desktop background.
	    return bckgrConfig.readColorEntry("Color1", &tqApp->palette().disabled().background());
	}
	return TQColor();
    }
#endif

    const TQPalette &pal = tqApp->palette();
    TQColor c = pal.color( uicol->group, uicol->role );
#ifndef APPLE_CHANGES
    if ( uicol->configEntry ) {
	TDEConfig *globalConfig = TDEGlobal::config();
	globalConfig->setGroup( uicol->configGroup );
	c = globalConfig->readColorEntry( uicol->configEntry, &c );
    }
#endif

    return c;
}

static inline int nextFontSize(const TQValueVector<int>& a, int v, bool smaller)
{
    // return the nearest bigger/smaller value in scale a, when v is in range.
    // otherwise increase/decrease value using a 1.2 fixed ratio
    int m, l = 0, r = a.count()-1;
    while (l <= r) {
        m = (l+r)/2;
        if (a[m] == v)
            return smaller ? ( m ? a[m-1] : (v*5)/6 ) :
                             ( m+1<int(a.count()) ? a[m+1] : (v*6)/5 );
        else if (v < a[m])
            r = m-1;
        else
            l = m+1;
    }
    if (!l)
        return smaller ? (v*5)/6 : kMin((v*6)/5, a[0]);
    if (l == int(a.count()))
        return smaller ? kMax((v*5)/6, a[r]) : (v*6)/5;

    return smaller ? a[r] : a[l];
}

void CSSStyleSelector::applyRule( int id, DOM::CSSValueImpl *value )
{
//     kdDebug( 6080 ) << "applying property " << getPropertyName(id) << endl;

    CSSPrimitiveValueImpl *primitiveValue = 0;
    if(value->isPrimitiveValue()) primitiveValue = static_cast<CSSPrimitiveValueImpl *>(value);

    Length l;
    bool apply = false;

    bool isInherit = (parentNode && value->cssValueType() == CSSValue::CSS_INHERIT);
    bool isInitial = (value->cssValueType() == CSSValue::CSS_INITIAL) ||
                     (!parentNode && value->cssValueType() == CSSValue::CSS_INHERIT);

    // These properties are used to set the correct margins/padding on RTL lists.
    if (id == CSS_PROP__TDEHTML_MARGIN_START)
        id = style->direction() == LTR ? CSS_PROP_MARGIN_LEFT : CSS_PROP_MARGIN_RIGHT;
    else if (id == CSS_PROP__TDEHTML_PADDING_START)
        id = style->direction() == LTR ? CSS_PROP_PADDING_LEFT : CSS_PROP_PADDING_RIGHT;

    // What follows is a list that maps the CSS properties into their corresponding front-end
    // RenderStyle values.  Shorthands (e.g. border, background) occur in this list as well and
    // are only hit when mapping "inherit" or "initial" into front-end values.
    switch(id)
    {
// ident only properties
    case CSS_PROP_BACKGROUND_ATTACHMENT:
        HANDLE_BACKGROUND_VALUE(backgroundAttachment, BackgroundAttachment, value)
        break;
    case CSS_PROP__TDEHTML_BACKGROUND_CLIP:
        HANDLE_BACKGROUND_VALUE(backgroundClip, BackgroundClip, value)
        break;
    case CSS_PROP__TDEHTML_BACKGROUND_ORIGIN:
        HANDLE_BACKGROUND_VALUE(backgroundOrigin, BackgroundOrigin, value)
        break;
    case CSS_PROP_BACKGROUND_REPEAT:
        HANDLE_BACKGROUND_VALUE(backgroundRepeat, BackgroundRepeat, value)
        break;
    case CSS_PROP__TDEHTML_BACKGROUND_SIZE:
        HANDLE_BACKGROUND_VALUE(backgroundSize, BackgroundSize, value)
        break;
    case CSS_PROP_BORDER_COLLAPSE:
        HANDLE_INHERIT_AND_INITIAL(borderCollapse, BorderCollapse)
        if(!primitiveValue) break;
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL_COLLAPSE:
            style->setBorderCollapse(true);
            break;
        case CSS_VAL_SEPARATE:
            style->setBorderCollapse(false);
            break;
        default:
            return;
        }
        break;

    case CSS_PROP_BORDER_TOP_STYLE:
        HANDLE_INHERIT_AND_INITIAL_WITH_VALUE(borderTopStyle, BorderTopStyle, BorderStyle)
        if (!primitiveValue) return;
        style->setBorderTopStyle((EBorderStyle)(primitiveValue->getIdent() - CSS_VAL__TDEHTML_NATIVE));
        break;
    case CSS_PROP_BORDER_RIGHT_STYLE:
        HANDLE_INHERIT_AND_INITIAL_WITH_VALUE(borderRightStyle, BorderRightStyle, BorderStyle)
        if (!primitiveValue) return;
        style->setBorderRightStyle((EBorderStyle)(primitiveValue->getIdent() - CSS_VAL__TDEHTML_NATIVE));
        break;
    case CSS_PROP_BORDER_BOTTOM_STYLE:
        HANDLE_INHERIT_AND_INITIAL_WITH_VALUE(borderBottomStyle, BorderBottomStyle, BorderStyle)
        if (!primitiveValue) return;
        style->setBorderBottomStyle((EBorderStyle)(primitiveValue->getIdent() - CSS_VAL__TDEHTML_NATIVE));
        break;
    case CSS_PROP_BORDER_LEFT_STYLE:
        HANDLE_INHERIT_AND_INITIAL_WITH_VALUE(borderLeftStyle, BorderLeftStyle, BorderStyle)
        if (!primitiveValue) return;
        style->setBorderLeftStyle((EBorderStyle)(primitiveValue->getIdent() - CSS_VAL__TDEHTML_NATIVE));
        break;
    case CSS_PROP_OUTLINE_STYLE:
        HANDLE_INHERIT_AND_INITIAL_WITH_VALUE(outlineStyle, OutlineStyle, BorderStyle)
        if (!primitiveValue) return;
        style->setOutlineStyle((EBorderStyle)(primitiveValue->getIdent() - CSS_VAL__TDEHTML_NATIVE));
        break;
    case CSS_PROP_CAPTION_SIDE:
    {
        HANDLE_INHERIT_AND_INITIAL(captionSide, CaptionSide)
        if(!primitiveValue) break;
        ECaptionSide c = RenderStyle::initialCaptionSide();
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL_LEFT:
            c = CAPLEFT; break;
        case CSS_VAL_RIGHT:
            c = CAPRIGHT; break;
        case CSS_VAL_TOP:
            c = CAPTOP; break;
        case CSS_VAL_BOTTOM:
            c = CAPBOTTOM; break;
        default:
            return;
        }
        style->setCaptionSide(c);
        return;
    }
    case CSS_PROP_CLEAR:
    {
        HANDLE_INHERIT_AND_INITIAL(clear, Clear)
        if(!primitiveValue) break;
        EClear c = CNONE;
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL_LEFT:
            c = CLEFT; break;
        case CSS_VAL_RIGHT:
            c = CRIGHT; break;
        case CSS_VAL_BOTH:
            c = CBOTH; break;
        case CSS_VAL_NONE:
            c = CNONE; break;
        default:
            return;
        }
        style->setClear(c);
        return;
    }
    case CSS_PROP_DIRECTION:
    {
        HANDLE_INHERIT_AND_INITIAL(direction, Direction)
        if(!primitiveValue) break;
        style->setDirection( (EDirection) (primitiveValue->getIdent() - CSS_VAL_LTR) );
        return;
    }
    case CSS_PROP_DISPLAY:
    {
        HANDLE_INHERIT_AND_INITIAL(display, Display)
        if(!primitiveValue) break;
	int id = primitiveValue->getIdent();
        style->setDisplay( id == CSS_VAL_NONE ? NONE : EDisplay(id - CSS_VAL_INLINE) );
        break;
    }

    case CSS_PROP_EMPTY_CELLS:
    {
        HANDLE_INHERIT(emptyCells, EmptyCells);
        if (!primitiveValue) break;
        int id = primitiveValue->getIdent();
        if (id == CSS_VAL_SHOW)
            style->setEmptyCells(SHOW);
        else if (id == CSS_VAL_HIDE)
            style->setEmptyCells(HIDE);
        break;
    }
    case CSS_PROP_FLOAT:
    {
        HANDLE_INHERIT_AND_INITIAL(floating, Floating)
        if(!primitiveValue) return;
        EFloat f;
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL__TDEHTML_LEFT:
            f = FLEFT_ALIGN; break;
        case CSS_VAL_LEFT:
            f = FLEFT; break;
        case CSS_VAL__TDEHTML_RIGHT:
            f = FRIGHT_ALIGN; break;
        case CSS_VAL_RIGHT:
            f = FRIGHT; break;
        case CSS_VAL_NONE:
        case CSS_VAL_CENTER:  //Non standart CSS-Value
            f = FNONE; break;
        default:
            return;
        }
        if (f!=FNONE && style->display()==LIST_ITEM)
            style->setDisplay(BLOCK);

        style->setFloating(f);
        break;
    }

    case CSS_PROP_FONT_STYLE:
    {
        FontDef fontDef = style->htmlFont().fontDef;
        if (isInherit)
            fontDef.italic = parentStyle->htmlFont().fontDef.italic;
	else if (isInitial)
            fontDef.italic = false;
        else {
	    if(!primitiveValue) return;
	    switch(primitiveValue->getIdent()) {
		case CSS_VAL_OBLIQUE:
		// ### oblique is the same as italic for the moment...
		case CSS_VAL_ITALIC:
		    fontDef.italic = true;
		    break;
		case CSS_VAL_NORMAL:
		    fontDef.italic = false;
		    break;
		default:
		    return;
	    }
	}
        fontDirty |= style->setFontDef( fontDef );
        break;
    }


    case CSS_PROP_FONT_VARIANT:
    {
        FontDef fontDef = style->htmlFont().fontDef;
        if (isInherit)
            fontDef.smallCaps = parentStyle->htmlFont().fontDef.smallCaps;
        else if (isInitial)
            fontDef.smallCaps = false;
        else {
	    if(!primitiveValue) return;
	    int id = primitiveValue->getIdent();
	    if ( id == CSS_VAL_NORMAL )
		fontDef.smallCaps = false;
	    else if ( id == CSS_VAL_SMALL_CAPS )
		fontDef.smallCaps = true;
	    else
		return;
	}
	fontDirty |= style->setFontDef( fontDef );
	break;
    }

    case CSS_PROP_FONT_WEIGHT:
    {
        FontDef fontDef = style->htmlFont().fontDef;
        if (isInherit)
            fontDef.weight = parentStyle->htmlFont().fontDef.weight;
        else if (isInitial)
            fontDef.weight = TQFont::Normal;
        else {
	    if(!primitiveValue) return;
	    if(primitiveValue->getIdent())
	    {
		switch(primitiveValue->getIdent()) {
		    // ### we just support normal and bold fonts at the moment...
		    // setWeight can actually accept values between 0 and 99...
		case CSS_VAL_BOLD:
		case CSS_VAL_BOLDER:
		case CSS_VAL_600:
		case CSS_VAL_700:
		case CSS_VAL_800:
		case CSS_VAL_900:
		    fontDef.weight = TQFont::Bold;
		    break;
		case CSS_VAL_NORMAL:
		case CSS_VAL_LIGHTER:
		case CSS_VAL_100:
		case CSS_VAL_200:
		case CSS_VAL_300:
		case CSS_VAL_400:
		case CSS_VAL_500:
		    fontDef.weight = TQFont::Normal;
		    break;
		default:
		    return;
		}
	    }
	    else
	    {
		// ### fix parsing of 100-900 values in parser, apply them here
	    }
	}
        fontDirty |= style->setFontDef( fontDef );
        break;
    }

    case CSS_PROP_LIST_STYLE_POSITION:
    {
        HANDLE_INHERIT_AND_INITIAL(listStylePosition, ListStylePosition)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent())
            style->setListStylePosition( (EListStylePosition) (primitiveValue->getIdent() - CSS_VAL_OUTSIDE) );
        return;
    }

    case CSS_PROP_LIST_STYLE_TYPE:
    {
        HANDLE_INHERIT_AND_INITIAL(listStyleType, ListStyleType)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent())
        {
            EListStyleType t;
	    int id = primitiveValue->getIdent();
	    if ( id == CSS_VAL_NONE) { // important!!
	      t = LNONE;
	    } else {
	      t = EListStyleType(id - CSS_VAL_DISC);
	    }
            style->setListStyleType(t);
        }
        return;
    }

    case CSS_PROP_OVERFLOW:
    {
        if (isInherit) {
            style->setOverflowX(parentStyle->overflowX());
            style->setOverflowY(parentStyle->overflowY());
            return;
        }

        if (isInitial) {
            style->setOverflowX(RenderStyle::initialOverflowX());
            style->setOverflowY(RenderStyle::initialOverflowY());
            return;
        }

        if (!primitiveValue) return;
        EOverflow o;
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL_VISIBLE:
            o = OVISIBLE; break;
        case CSS_VAL_HIDDEN:
            o = OHIDDEN; break;
        case CSS_VAL_SCROLL:
	    o = OSCROLL; break;
        case CSS_VAL_AUTO:
	    o = OAUTO; break;
        case CSS_VAL_MARQUEE:
            o = OMARQUEE; break;
        default:
            return;
        }
        style->setOverflowX(o);
        style->setOverflowY(o);
        return;
    }
    case CSS_PROP_OVERFLOW_X:
    {
        HANDLE_INHERIT_AND_INITIAL(overflowX, OverflowX)
        if (!primitiveValue) return;
        EOverflow o;
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL_VISIBLE:
            o = OVISIBLE; break;
        case CSS_VAL_HIDDEN:
            o = OHIDDEN; break;
        case CSS_VAL_SCROLL:
	    o = OSCROLL; break;
        case CSS_VAL_AUTO:
	    o = OAUTO; break;
        default:
            return;
        }
        style->setOverflowX(o);
        return;
    }
    case CSS_PROP_OVERFLOW_Y:
    {
        HANDLE_INHERIT_AND_INITIAL(overflowY, OverflowY)
        if (!primitiveValue) return;
        EOverflow o;
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL_VISIBLE:
            o = OVISIBLE; break;
        case CSS_VAL_HIDDEN:
            o = OHIDDEN; break;
        case CSS_VAL_SCROLL:
            o = OSCROLL; break;
        case CSS_VAL_AUTO:
            o = OAUTO; break;
        default:
            return;
        }
        style->setOverflowY(o);
        return;
    }
    case CSS_PROP_PAGE_BREAK_BEFORE:
    {
        HANDLE_INHERIT_AND_INITIAL_WITH_VALUE(pageBreakBefore, PageBreakBefore, PageBreak)
        if (!primitiveValue) return;
        switch (primitiveValue->getIdent()) {
            case CSS_VAL_AUTO:
                style->setPageBreakBefore(PBAUTO);
                break;
            case CSS_VAL_LEFT:
            case CSS_VAL_RIGHT:
                // CSS2.1: "Conforming user agents may map left/right to always."
            case CSS_VAL_ALWAYS:
                style->setPageBreakBefore(PBALWAYS);
                break;
            case CSS_VAL_AVOID:
                style->setPageBreakBefore(PBAVOID);
                break;
        }
        break;
    }

    case CSS_PROP_PAGE_BREAK_AFTER:
    {
        HANDLE_INHERIT_AND_INITIAL_WITH_VALUE(pageBreakAfter, PageBreakAfter, PageBreak)
        if (!primitiveValue) return;
        switch (primitiveValue->getIdent()) {
            case CSS_VAL_AUTO:
                style->setPageBreakAfter(PBAUTO);
                break;
            case CSS_VAL_LEFT:
            case CSS_VAL_RIGHT:
                // CSS2.1: "Conforming user agents may map left/right to always."
            case CSS_VAL_ALWAYS:
                style->setPageBreakAfter(PBALWAYS);
                break;
            case CSS_VAL_AVOID:
                style->setPageBreakAfter(PBAVOID);
                break;
        }
        break;
    }

    case CSS_PROP_PAGE_BREAK_INSIDE: {
        HANDLE_INHERIT_AND_INITIAL(pageBreakInside, PageBreakInside)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent() == CSS_VAL_AUTO)
            style->setPageBreakInside(true);
        else if (primitiveValue->getIdent() == CSS_VAL_AVOID)
            style->setPageBreakInside(false);
        return;
    }
//    case CSS_PROP_PAUSE_AFTER:
//    case CSS_PROP_PAUSE_BEFORE:
        break;

    case CSS_PROP_POSITION:
    {
        HANDLE_INHERIT_AND_INITIAL(position, Position)
        if (!primitiveValue) return;
        EPosition p;
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL_STATIC:
            p = STATIC; break;
        case CSS_VAL_RELATIVE:
            p = RELATIVE; break;
        case CSS_VAL_ABSOLUTE:
            p = ABSOLUTE; break;
        case CSS_VAL_FIXED:
            {
                view->useSlowRepaints();
                p = FIXED;
                break;
            }
        default:
            return;
        }
        style->setPosition(p);
        return;
    }

    case CSS_PROP_TABLE_LAYOUT: {
        HANDLE_INHERIT_AND_INITIAL(tableLayout, TableLayout)

	if ( !primitiveValue )
	    return;

	ETableLayout l = RenderStyle::initialTableLayout();
	switch( primitiveValue->getIdent() ) {
	case CSS_VAL_FIXED:
	    l = TFIXED;
	    // fall through
	case CSS_VAL_AUTO:
	    style->setTableLayout( l );
	default:
	    break;
	}
	break;
    }

    case CSS_PROP_UNICODE_BIDI: {
        HANDLE_INHERIT_AND_INITIAL(unicodeBidi, UnicodeBidi)
        if(!primitiveValue) break;
        switch (primitiveValue->getIdent()) {
            case CSS_VAL_NORMAL:
                style->setUnicodeBidi(UBNormal);
                break;
            case CSS_VAL_EMBED:
                style->setUnicodeBidi(Embed);
                break;
            case CSS_VAL_BIDI_OVERRIDE:
                style->setUnicodeBidi(Override);
                break;
            default:
                return;
        }
	break;
    }
    case CSS_PROP_TEXT_TRANSFORM: {
        HANDLE_INHERIT_AND_INITIAL(textTransform, TextTransform)

        if(!primitiveValue) break;
        if(!primitiveValue->getIdent()) return;

        ETextTransform tt;
        switch(primitiveValue->getIdent()) {
        case CSS_VAL_CAPITALIZE:  tt = CAPITALIZE;  break;
        case CSS_VAL_UPPERCASE:   tt = UPPERCASE;   break;
        case CSS_VAL_LOWERCASE:   tt = LOWERCASE;   break;
        case CSS_VAL_NONE:
        default:                  tt = TTNONE;      break;
        }
        style->setTextTransform(tt);
        break;
        }

    case CSS_PROP_VISIBILITY:
    {
        HANDLE_INHERIT_AND_INITIAL(visibility, Visibility)

        if(!primitiveValue) break;
        switch( primitiveValue->getIdent() ) {
        case CSS_VAL_HIDDEN:
            style->setVisibility( HIDDEN );
            break;
        case CSS_VAL_VISIBLE:
            style->setVisibility( VISIBLE );
            break;
        case CSS_VAL_COLLAPSE:
            style->setVisibility( COLLAPSE );
        default:
            break;
        }
        break;
    }
    case CSS_PROP_WHITE_SPACE:
        HANDLE_INHERIT_AND_INITIAL(whiteSpace, WhiteSpace)

        if(!primitiveValue) break;
        if(!primitiveValue->getIdent()) return;

        EWhiteSpace s;
        switch(primitiveValue->getIdent()) {
        case CSS_VAL__TDEHTML_NOWRAP:
            s = TDEHTML_NOWRAP;
            break;
        case CSS_VAL_NOWRAP:
            s = NOWRAP;
            break;
        case CSS_VAL_PRE:
            s = PRE;
            break;
        case CSS_VAL_PRE_WRAP:
            s = PRE_WRAP;
            break;
        case CSS_VAL_PRE_LINE:
            s = PRE_LINE;
            break;
        case CSS_VAL_NORMAL:
        default:
            s = NORMAL;
            break;
        }
        style->setWhiteSpace(s);
        break;

    case CSS_PROP_BACKGROUND_POSITION:
        HANDLE_BACKGROUND_INHERIT_AND_INITIAL(backgroundXPosition, BackgroundXPosition);
        HANDLE_BACKGROUND_INHERIT_AND_INITIAL(backgroundYPosition, BackgroundYPosition);
        break;
    case CSS_PROP_BACKGROUND_POSITION_X: {
        HANDLE_BACKGROUND_VALUE(backgroundXPosition, BackgroundXPosition, value)
        break;
    }
    case CSS_PROP_BACKGROUND_POSITION_Y: {
        HANDLE_BACKGROUND_VALUE(backgroundYPosition, BackgroundYPosition, value)
        break;
    }
    case CSS_PROP_BORDER_SPACING: {
        if(value->cssValueType() != CSSValue::CSS_INHERIT || !parentNode) return;
        style->setBorderHorizontalSpacing(parentStyle->borderHorizontalSpacing());
        style->setBorderVerticalSpacing(parentStyle->borderVerticalSpacing());
        break;
    }
    case CSS_PROP__TDEHTML_BORDER_HORIZONTAL_SPACING: {
        HANDLE_INHERIT_AND_INITIAL(borderHorizontalSpacing, BorderHorizontalSpacing)
        if (!primitiveValue) break;
        short spacing =  primitiveValue->computeLength(style, paintDeviceMetrics);
        style->setBorderHorizontalSpacing(spacing);
        break;
    }
    case CSS_PROP__TDEHTML_BORDER_VERTICAL_SPACING: {
        HANDLE_INHERIT_AND_INITIAL(borderVerticalSpacing, BorderVerticalSpacing)
        if (!primitiveValue) break;
        short spacing =  primitiveValue->computeLength(style, paintDeviceMetrics);
        style->setBorderVerticalSpacing(spacing);
        break;
    }

    case CSS_PROP_CURSOR:
        HANDLE_INHERIT_AND_INITIAL(cursor, Cursor)
        if(primitiveValue)
	    style->setCursor( (ECursor) (primitiveValue->getIdent() - CSS_VAL_AUTO) );
        break;
// colors || inherit
    case CSS_PROP_BACKGROUND_COLOR:
    case CSS_PROP_BORDER_TOP_COLOR:
    case CSS_PROP_BORDER_RIGHT_COLOR:
    case CSS_PROP_BORDER_BOTTOM_COLOR:
    case CSS_PROP_BORDER_LEFT_COLOR:
    case CSS_PROP_COLOR:
    case CSS_PROP_OUTLINE_COLOR:
        // this property is an extension used to get HTML4 <font> right.
	case CSS_PROP_SCROLLBAR_BASE_COLOR:
    case CSS_PROP_SCROLLBAR_FACE_COLOR:
    case CSS_PROP_SCROLLBAR_SHADOW_COLOR:
    case CSS_PROP_SCROLLBAR_HIGHLIGHT_COLOR:
    case CSS_PROP_SCROLLBAR_3DLIGHT_COLOR:
    case CSS_PROP_SCROLLBAR_DARKSHADOW_COLOR:
    case CSS_PROP_SCROLLBAR_TRACK_COLOR:
    case CSS_PROP_SCROLLBAR_ARROW_COLOR:
    {
        TQColor col;
        if (isInherit) {
            HANDLE_INHERIT_COND(CSS_PROP_BACKGROUND_COLOR, backgroundColor, BackgroundColor)
            HANDLE_INHERIT_COND(CSS_PROP_BORDER_TOP_COLOR, borderTopColor, BorderTopColor)
            HANDLE_INHERIT_COND(CSS_PROP_BORDER_BOTTOM_COLOR, borderBottomColor, BorderBottomColor)
            HANDLE_INHERIT_COND(CSS_PROP_BORDER_RIGHT_COLOR, borderRightColor, BorderRightColor)
            HANDLE_INHERIT_COND(CSS_PROP_BORDER_LEFT_COLOR, borderLeftColor, BorderLeftColor)
            HANDLE_INHERIT_COND(CSS_PROP_COLOR, color, Color)
            HANDLE_INHERIT_COND(CSS_PROP_OUTLINE_COLOR, outlineColor, OutlineColor)
            return;
        } else if (isInitial) {
            // The border/outline colors will just map to the invalid color |col| above.  This will have the
            // effect of forcing the use of the currentColor when it comes time to draw the borders (and of
            // not painting the background since the color won't be valid).
            if (id == CSS_PROP_COLOR)
                col = RenderStyle::initialColor();
        } else {
	    if(!primitiveValue )
		return;
	    int ident = primitiveValue->getIdent();
	    if ( ident ) {
		if ( ident == CSS_VAL__TDEHTML_TEXT )
		    col = element->getDocument()->textColor();
  		// ### should be eliminated
		else if ( ident == CSS_VAL_TRANSPARENT
                	&& id != CSS_PROP_BORDER_TOP_COLOR
                        && id != CSS_PROP_BORDER_RIGHT_COLOR
      			&& id != CSS_PROP_BORDER_BOTTOM_COLOR
      			&& id != CSS_PROP_BORDER_LEFT_COLOR )
		    col = TQColor();
		else
		    col = colorForCSSValue( ident );
	    } else if ( primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_RGBCOLOR ) {
#ifndef APPLE_CHANGES
		if(tqAlpha(primitiveValue->getRGBColorValue()))
#endif
		    col.setRgb(primitiveValue->getRGBColorValue());
	    } else {
		return;
	    }
	}
        //kdDebug( 6080 ) << "applying color " << col.isValid() << endl;
        switch(id)
        {
        case CSS_PROP_BACKGROUND_COLOR:
	    style->setBackgroundColor(col); break;
        case CSS_PROP_BORDER_TOP_COLOR:
            style->setBorderTopColor(col); break;
        case CSS_PROP_BORDER_RIGHT_COLOR:
            style->setBorderRightColor(col); break;
        case CSS_PROP_BORDER_BOTTOM_COLOR:
            style->setBorderBottomColor(col); break;
        case CSS_PROP_BORDER_LEFT_COLOR:
            style->setBorderLeftColor(col); break;
        case CSS_PROP_COLOR:
            style->setColor(col); break;
        case CSS_PROP_OUTLINE_COLOR:
            style->setOutlineColor(col); break;
#ifndef APPLE_CHANGES
        case CSS_PROP_SCROLLBAR_FACE_COLOR:
            style->setPaletteColor(TQPalette::Active, TQColorGroup::Button, col);
            style->setPaletteColor(TQPalette::Inactive, TQColorGroup::Button, col);
            break;
        case CSS_PROP_SCROLLBAR_SHADOW_COLOR:
            style->setPaletteColor(TQPalette::Active, TQColorGroup::Shadow, col);
            style->setPaletteColor(TQPalette::Inactive, TQColorGroup::Shadow, col);
            break;
        case CSS_PROP_SCROLLBAR_HIGHLIGHT_COLOR:
            style->setPaletteColor(TQPalette::Active, TQColorGroup::Light, col);
            style->setPaletteColor(TQPalette::Inactive, TQColorGroup::Light, col);
            break;
        case CSS_PROP_SCROLLBAR_3DLIGHT_COLOR:
            break;
        case CSS_PROP_SCROLLBAR_DARKSHADOW_COLOR:
            style->setPaletteColor(TQPalette::Active, TQColorGroup::Dark, col);
            style->setPaletteColor(TQPalette::Inactive, TQColorGroup::Dark, col);
            break;
        case CSS_PROP_SCROLLBAR_TRACK_COLOR:
            style->setPaletteColor(TQPalette::Active, TQColorGroup::Mid, col);
            style->setPaletteColor(TQPalette::Inactive, TQColorGroup::Mid, col);
            style->setPaletteColor(TQPalette::Active, TQColorGroup::Background, col);
            style->setPaletteColor(TQPalette::Inactive, TQColorGroup::Background, col);
            // fall through
        case CSS_PROP_SCROLLBAR_BASE_COLOR:
            style->setPaletteColor(TQPalette::Active, TQColorGroup::Base, col);
            style->setPaletteColor(TQPalette::Inactive, TQColorGroup::Base, col);
            break;
        case CSS_PROP_SCROLLBAR_ARROW_COLOR:
            style->setPaletteColor(TQPalette::Active, TQColorGroup::ButtonText, col);
            style->setPaletteColor(TQPalette::Inactive, TQColorGroup::ButtonText, col);
            break;
#endif
        default:
            return;
        }
        return;
    }
    break;
// uri || inherit
    case CSS_PROP_BACKGROUND_IMAGE:
        HANDLE_BACKGROUND_VALUE(backgroundImage, BackgroundImage, value)
        break;
    case CSS_PROP_LIST_STYLE_IMAGE:
    {
        HANDLE_INHERIT_AND_INITIAL(listStyleImage, ListStyleImage)
        if (!primitiveValue) return;
	style->setListStyleImage(static_cast<CSSImageValueImpl *>(primitiveValue)->image());
        //kdDebug( 6080 ) << "setting image in list to " << image->image() << endl;
        break;
    }

// length
    case CSS_PROP_BORDER_TOP_WIDTH:
    case CSS_PROP_BORDER_RIGHT_WIDTH:
    case CSS_PROP_BORDER_BOTTOM_WIDTH:
    case CSS_PROP_BORDER_LEFT_WIDTH:
    case CSS_PROP_OUTLINE_WIDTH:
    {
	if (isInherit) {
            HANDLE_INHERIT_COND(CSS_PROP_BORDER_TOP_WIDTH, borderTopWidth, BorderTopWidth)
            HANDLE_INHERIT_COND(CSS_PROP_BORDER_RIGHT_WIDTH, borderRightWidth, BorderRightWidth)
            HANDLE_INHERIT_COND(CSS_PROP_BORDER_BOTTOM_WIDTH, borderBottomWidth, BorderBottomWidth)
            HANDLE_INHERIT_COND(CSS_PROP_BORDER_LEFT_WIDTH, borderLeftWidth, BorderLeftWidth)
            HANDLE_INHERIT_COND(CSS_PROP_OUTLINE_WIDTH, outlineWidth, OutlineWidth)
            return;
        }
        else if (isInitial) {
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_BORDER_TOP_WIDTH, BorderTopWidth, BorderWidth)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_BORDER_RIGHT_WIDTH, BorderRightWidth, BorderWidth)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_BORDER_BOTTOM_WIDTH, BorderBottomWidth, BorderWidth)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_BORDER_LEFT_WIDTH, BorderLeftWidth, BorderWidth)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_OUTLINE_WIDTH, OutlineWidth, BorderWidth)
            return;
        }

        if(!primitiveValue) break;
        short width = 3;
        switch(primitiveValue->getIdent())
        {
        case CSS_VAL_THIN:
            width = 1;
            break;
        case CSS_VAL_MEDIUM:
            width = 3;
            break;
        case CSS_VAL_THICK:
            width = 5;
            break;
        case CSS_VAL_INVALID:
        {
            double widthd = primitiveValue->computeLengthFloat(style, paintDeviceMetrics);
            width = (int)widthd;
            // somewhat resemble Mozilla's granularity
            // this makes border-width: 0.5pt borders visible
            if (width == 0 && widthd >= 0.025) width++;
            break;
        }
        default:
            return;
        }

        if(width < 0) return;
        switch(id)
        {
        case CSS_PROP_BORDER_TOP_WIDTH:
            style->setBorderTopWidth(width);
            break;
        case CSS_PROP_BORDER_RIGHT_WIDTH:
            style->setBorderRightWidth(width);
            break;
        case CSS_PROP_BORDER_BOTTOM_WIDTH:
            style->setBorderBottomWidth(width);
            break;
        case CSS_PROP_BORDER_LEFT_WIDTH:
            style->setBorderLeftWidth(width);
            break;
        case CSS_PROP_OUTLINE_WIDTH:
            style->setOutlineWidth(width);
            break;
        default:
            return;
        }
        return;
    }

    case CSS_PROP_LETTER_SPACING:
    case CSS_PROP_WORD_SPACING:
    {
        if (isInherit) {
            HANDLE_INHERIT_COND(CSS_PROP_LETTER_SPACING, letterSpacing, LetterSpacing)
            HANDLE_INHERIT_COND(CSS_PROP_WORD_SPACING, wordSpacing, WordSpacing)
            return;
        } else if (isInitial) {
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_LETTER_SPACING, LetterSpacing, LetterWordSpacing)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_WORD_SPACING, WordSpacing, LetterWordSpacing)
            return;
        }
        if(!primitiveValue) return;

        int width = 0;
        if (primitiveValue->getIdent() != CSS_VAL_NORMAL)
	    width = primitiveValue->computeLength(style, paintDeviceMetrics);

        switch(id)
        {
        case CSS_PROP_LETTER_SPACING:
            style->setLetterSpacing(width);
            break;
        case CSS_PROP_WORD_SPACING:
            style->setWordSpacing(width);
            break;
            // ### needs the definitions in renderstyle
        default: break;
        }
        return;
    }

        // length, percent
    case CSS_PROP_MAX_WIDTH:
        // +none +inherit
        if(primitiveValue && primitiveValue->getIdent() == CSS_VAL_NONE) {
            apply = true;
            l = Length(UNDEFINED, Fixed);
        }
    case CSS_PROP_TOP:
    case CSS_PROP_LEFT:
    case CSS_PROP_RIGHT:
    case CSS_PROP_BOTTOM:
    case CSS_PROP_WIDTH:
    case CSS_PROP_MIN_WIDTH:
    case CSS_PROP_MARGIN_TOP:
    case CSS_PROP_MARGIN_RIGHT:
    case CSS_PROP_MARGIN_BOTTOM:
    case CSS_PROP_MARGIN_LEFT:
        // +inherit +auto
        if(id != CSS_PROP_MAX_WIDTH && primitiveValue &&
           primitiveValue->getIdent() == CSS_VAL_AUTO)
        {
            //kdDebug( 6080 ) << "found value=auto" << endl;
            apply = true;
        }
    case CSS_PROP_PADDING_TOP:
    case CSS_PROP_PADDING_RIGHT:
    case CSS_PROP_PADDING_BOTTOM:
    case CSS_PROP_PADDING_LEFT:
    case CSS_PROP_TEXT_INDENT:
        // +inherit
    {
        if (isInherit) {
            HANDLE_INHERIT_COND(CSS_PROP_MAX_WIDTH, maxWidth, MaxWidth)
            HANDLE_INHERIT_COND(CSS_PROP_BOTTOM, bottom, Bottom)
            HANDLE_INHERIT_COND(CSS_PROP_TOP, top, Top)
            HANDLE_INHERIT_COND(CSS_PROP_LEFT, left, Left)
            HANDLE_INHERIT_COND(CSS_PROP_RIGHT, right, Right)
            HANDLE_INHERIT_COND(CSS_PROP_WIDTH, width, Width)
            HANDLE_INHERIT_COND(CSS_PROP_MIN_WIDTH, minWidth, MinWidth)
            HANDLE_INHERIT_COND(CSS_PROP_PADDING_TOP, paddingTop, PaddingTop)
            HANDLE_INHERIT_COND(CSS_PROP_PADDING_RIGHT, paddingRight, PaddingRight)
            HANDLE_INHERIT_COND(CSS_PROP_PADDING_BOTTOM, paddingBottom, PaddingBottom)
            HANDLE_INHERIT_COND(CSS_PROP_PADDING_LEFT, paddingLeft, PaddingLeft)
            HANDLE_INHERIT_COND(CSS_PROP_MARGIN_TOP, marginTop, MarginTop)
            HANDLE_INHERIT_COND(CSS_PROP_MARGIN_RIGHT, marginRight, MarginRight)
            HANDLE_INHERIT_COND(CSS_PROP_MARGIN_BOTTOM, marginBottom, MarginBottom)
            HANDLE_INHERIT_COND(CSS_PROP_MARGIN_LEFT, marginLeft, MarginLeft)
            HANDLE_INHERIT_COND(CSS_PROP_TEXT_INDENT, textIndent, TextIndent)
            return;
        } else if (isInitial) {
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_MAX_WIDTH, MaxWidth, MaxSize)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_BOTTOM, Bottom, Offset)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_TOP, Top, Offset)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_LEFT, Left, Offset)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_RIGHT, Right, Offset)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_WIDTH, Width, Size)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_MIN_WIDTH, MinWidth, MinSize)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_PADDING_TOP, PaddingTop, Padding)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_PADDING_RIGHT, PaddingRight, Padding)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_PADDING_BOTTOM, PaddingBottom, Padding)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_PADDING_LEFT, PaddingLeft, Padding)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_MARGIN_TOP, MarginTop, Margin)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_MARGIN_RIGHT, MarginRight, Margin)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_MARGIN_BOTTOM, MarginBottom, Margin)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_MARGIN_LEFT, MarginLeft, Margin)
            HANDLE_INITIAL_COND(CSS_PROP_TEXT_INDENT, TextIndent)
            return;
        }

        if (primitiveValue && !apply) {
            int type = primitiveValue->primitiveType();
            if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
                // Handle our quirky margin units if we have them.
                l = Length(primitiveValue->computeLength(style, paintDeviceMetrics), Fixed,
                           primitiveValue->isQuirkValue());
            else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
                l = Length((int)primitiveValue->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
	    else if (type == CSSPrimitiveValue::CSS_HTML_RELATIVE)
		l = Length(int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_HTML_RELATIVE)), Relative);
            else
                return;
            apply = true;
        }
        if(!apply) return;
        switch(id)
            {
            case CSS_PROP_MAX_WIDTH:
                style->setMaxWidth(l); break;
            case CSS_PROP_BOTTOM:
                style->setBottom(l); break;
            case CSS_PROP_TOP:
                style->setTop(l); break;
            case CSS_PROP_LEFT:
                style->setLeft(l); break;
            case CSS_PROP_RIGHT:
                style->setRight(l); break;
            case CSS_PROP_WIDTH:
                style->setWidth(l); break;
            case CSS_PROP_MIN_WIDTH:
                style->setMinWidth(l); break;
            case CSS_PROP_PADDING_TOP:
                style->setPaddingTop(l); break;
            case CSS_PROP_PADDING_RIGHT:
                style->setPaddingRight(l); break;
            case CSS_PROP_PADDING_BOTTOM:
                style->setPaddingBottom(l); break;
            case CSS_PROP_PADDING_LEFT:
                style->setPaddingLeft(l); break;
            case CSS_PROP_MARGIN_TOP:
                style->setMarginTop(l); break;
            case CSS_PROP_MARGIN_RIGHT:
                style->setMarginRight(l); break;
            case CSS_PROP_MARGIN_BOTTOM:
                style->setMarginBottom(l); break;
            case CSS_PROP_MARGIN_LEFT:
                style->setMarginLeft(l); break;
            case CSS_PROP_TEXT_INDENT:
                style->setTextIndent(l); break;
            default: break;
            }
        return;
    }

    case CSS_PROP_MAX_HEIGHT:
        if(primitiveValue && primitiveValue->getIdent() == CSS_VAL_NONE) {
            apply = true;
            l = Length(UNDEFINED, Fixed);
        }
    case CSS_PROP_HEIGHT:
    case CSS_PROP_MIN_HEIGHT:
        if(id != CSS_PROP_MAX_HEIGHT && primitiveValue &&
           primitiveValue->getIdent() == CSS_VAL_AUTO)
            apply = true;
        if (isInherit) {
            HANDLE_INHERIT_COND(CSS_PROP_MAX_HEIGHT, maxHeight, MaxHeight)
            HANDLE_INHERIT_COND(CSS_PROP_HEIGHT, height, Height)
            HANDLE_INHERIT_COND(CSS_PROP_MIN_HEIGHT, minHeight, MinHeight)
            return;
        }
        else if (isInitial) {
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_MAX_HEIGHT, MaxHeight, MaxSize)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_HEIGHT, Height, Size)
            HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_MIN_HEIGHT, MinHeight, MinSize)
            return;
        }

        if (primitiveValue && !apply)
        {
            int type = primitiveValue->primitiveType();
            if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
                l = Length(primitiveValue->computeLength(style, paintDeviceMetrics), Fixed);
            else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
                l = Length((int)primitiveValue->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
            else
                return;
            apply = true;
        }
        if(!apply) return;
        switch(id)
        {
        case CSS_PROP_MAX_HEIGHT:
            style->setMaxHeight(l); break;
        case CSS_PROP_HEIGHT:
            style->setHeight(l); break;
        case CSS_PROP_MIN_HEIGHT:
            style->setMinHeight(l); break;
        default:
            return;
        }
        return;

        break;

    case CSS_PROP_VERTICAL_ALIGN:
        HANDLE_INHERIT_AND_INITIAL(verticalAlign, VerticalAlign)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent()) {
	  tdehtml::EVerticalAlign align;

	  switch(primitiveValue->getIdent())
	    {
		case CSS_VAL_TOP:
		    align = TOP; break;
		case CSS_VAL_BOTTOM:
		    align = BOTTOM; break;
		case CSS_VAL_MIDDLE:
		    align = MIDDLE; break;
		case CSS_VAL_BASELINE:
		    align = BASELINE; break;
		case CSS_VAL_TEXT_BOTTOM:
		    align = TEXT_BOTTOM; break;
		case CSS_VAL_TEXT_TOP:
		    align = TEXT_TOP; break;
		case CSS_VAL_SUB:
		    align = SUB; break;
		case CSS_VAL_SUPER:
		    align = SUPER; break;
		case CSS_VAL__TDEHTML_BASELINE_MIDDLE:
		    align = BASELINE_MIDDLE; break;
		default:
		    return;
	    }
	  style->setVerticalAlign(align);
	  return;
        } else {
	  int type = primitiveValue->primitiveType();
	  Length l;
	  if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
	    l = Length(primitiveValue->computeLength(style, paintDeviceMetrics), Fixed );
	  else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
	    l = Length( int( primitiveValue->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE) ), Percent );

	  style->setVerticalAlign( LENGTH );
	  style->setVerticalAlignLength( l );
	}
        break;

    case CSS_PROP_FONT_SIZE:
    {
        FontDef fontDef = style->htmlFont().fontDef;
        int oldSize;
        int size = 0;

	float toPix = paintDeviceMetrics->logicalDpiY()/72.;
	if (toPix  < 96./72.) toPix = 96./72.;

        int minFontSize = int(settings->minFontSize() * toPix);

        if(parentNode) {
            oldSize = parentStyle->font().pixelSize();
        } else
            oldSize = m_fontSizes[3];

        if (isInherit )
            size = oldSize;
        else if (isInitial)
            size = m_fontSizes[3];
        else if(primitiveValue->getIdent()) {
	    // keywords are being used.  Pick the correct default
	    // based off the font family.
#ifdef APPLE_CHANGES
	    const TQValueVector<int>& fontSizes = (fontDef.genericFamily == FontDef::eMonospace) ?
					 m_fixedFontSizes : m_fontSizes;
#else
	    const TQValueVector<int>& fontSizes = m_fontSizes;
#endif
            switch(primitiveValue->getIdent())
            {
            case CSS_VAL_XX_SMALL: size = int( fontSizes[0] ); break;
            case CSS_VAL_X_SMALL:  size = int( fontSizes[1] ); break;
            case CSS_VAL_SMALL:    size = int( fontSizes[2] ); break;
            case CSS_VAL_MEDIUM:   size = int( fontSizes[3] ); break;
            case CSS_VAL_LARGE:    size = int( fontSizes[4] ); break;
            case CSS_VAL_X_LARGE:  size = int( fontSizes[5] ); break;
            case CSS_VAL_XX_LARGE: size = int( fontSizes[6] ); break;
            case CSS_VAL__TDEHTML_XXX_LARGE: size = int( fontSizes[7] ); break;
            case CSS_VAL_LARGER:
                size = nextFontSize(fontSizes, oldSize, false);
                break;
            case CSS_VAL_SMALLER:
                size = nextFontSize(fontSizes, oldSize, true);
                break;
            default:
                return;
            }

        } else {
            int type = primitiveValue->primitiveType();
            if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG) {
                if ( !tdehtml::printpainter && type != CSSPrimitiveValue::CSS_EMS && type != CSSPrimitiveValue::CSS_EXS &&
                     view && view->part())
                    size = int( primitiveValue->computeLengthFloat(parentStyle, paintDeviceMetrics) *
                                view->part()->zoomFactor() ) / 100;
		else
                    size = int( primitiveValue->computeLengthFloat(parentStyle, paintDeviceMetrics) );
            }
            else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
                size = int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE)
                        * parentStyle->font().pixelSize()) / 100;
            else
                return;
        }

        if (size < 0) return;

        // we never want to get smaller than the minimum font size to keep fonts readable
        // do not however maximize zero as that is commonly used for fancy layouting purposes
        if (size && size < minFontSize) size = minFontSize;

        //kdDebug( 6080 ) << "computed raw font size: " << size << endl;

	fontDef.size = size;
        fontDirty |= style->setFontDef( fontDef );
        return;
    }

    case CSS_PROP_Z_INDEX:
    {
        HANDLE_INHERIT(zIndex, ZIndex)
        else if (isInitial) {
            style->setHasAutoZIndex();
            return;
        }

        if (!primitiveValue)
            return;

        if (primitiveValue->getIdent() == CSS_VAL_AUTO) {
            style->setHasAutoZIndex();
            return;
        }

        if (primitiveValue->primitiveType() != CSSPrimitiveValue::CSS_NUMBER)
            return; // Error case.

        style->setZIndex((int)primitiveValue->floatValue(CSSPrimitiveValue::CSS_NUMBER));
        return;
    }

    case CSS_PROP_WIDOWS:
    {
        HANDLE_INHERIT_AND_INITIAL(widows, Widows)
        if (!primitiveValue || primitiveValue->primitiveType() != CSSPrimitiveValue::CSS_NUMBER)
            return;
        style->setWidows((int)primitiveValue->floatValue(CSSPrimitiveValue::CSS_NUMBER));
        break;
     }

    case CSS_PROP_ORPHANS:
    {
        HANDLE_INHERIT_AND_INITIAL(orphans, Orphans)
        if (!primitiveValue || primitiveValue->primitiveType() != CSSPrimitiveValue::CSS_NUMBER)
            return;
        style->setOrphans((int)primitiveValue->floatValue(CSSPrimitiveValue::CSS_NUMBER));
        break;
    }

// length, percent, number
    case CSS_PROP_LINE_HEIGHT:
    {
        HANDLE_INHERIT_AND_INITIAL(lineHeight, LineHeight)
        if(!primitiveValue) return;
        Length lineHeight;
        int type = primitiveValue->primitiveType();
        if (primitiveValue->getIdent() == CSS_VAL_NORMAL)
            lineHeight = Length( -100, Percent );
        else if (type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG) {
            // Scale for the font zoom factor only for types other than "em" and "ex", since those are
            // already based on the font size.
		if ( !tdehtml::printpainter && type != CSSPrimitiveValue::CSS_EMS && type != CSSPrimitiveValue::CSS_EXS &&
                    view && view->part())
                    lineHeight = Length(primitiveValue->computeLength(style, paintDeviceMetrics) *
                                        view->part()->zoomFactor()/100, Fixed );
                else
                    lineHeight = Length(primitiveValue->computeLength(style, paintDeviceMetrics), Fixed );
        } else if (type == CSSPrimitiveValue::CSS_PERCENTAGE)
            lineHeight = Length( ( style->font().pixelSize() * int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE)) ) / 100, Fixed );
        else if (type == CSSPrimitiveValue::CSS_NUMBER)
            lineHeight = Length(int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_NUMBER)*100), Percent);
        else
            return;
        style->setLineHeight(lineHeight);
        return;
    }

// string
    case CSS_PROP_TEXT_ALIGN:
    {
        HANDLE_INHERIT_AND_INITIAL(textAlign, TextAlign)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent())
            style->setTextAlign( (ETextAlign) (primitiveValue->getIdent() - CSS_VAL__TDEHTML_AUTO) );
	return;
    }

// rect
    case CSS_PROP_CLIP:
    {
        Length top = Length();
        Length right = Length();
        Length bottom = Length();
        Length left = Length();

        bool hasClip = false;

        if (isInherit && parentStyle->hasClip()) {
            hasClip = true;
	    top = parentStyle->clipTop();
	    right = parentStyle->clipRight();
	    bottom = parentStyle->clipBottom();
	    left = parentStyle->clipLeft();
        } else if (primitiveValue && primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_RECT) {
            RectImpl *rect = primitiveValue->getRectValue();
            if (rect) {
                hasClip = true;
                top = convertToLength( rect->top(), style, paintDeviceMetrics );
                right = convertToLength( rect->right(), style, paintDeviceMetrics );
                bottom = convertToLength( rect->bottom(), style, paintDeviceMetrics );
                left = convertToLength( rect->left(), style, paintDeviceMetrics );
            }
        }

        style->setClip(top, right, bottom, left);
        style->setHasClip(hasClip);

        // rect, ident
        break;
    }

// lists
    case CSS_PROP_CONTENT:
        // list of string, uri, counter, attr, i
    {
        // FIXME: In CSS3, it will be possible to inherit content.  In CSS2 it is not.  This
        // note is a reminder that eventually "inherit" needs to be supported.

        // not allowed on non-generated pseudo-elements:
        if ( style->styleType()==RenderStyle::FIRST_LETTER ||
             style->styleType()==RenderStyle::FIRST_LINE ||
             style->styleType()==RenderStyle::SELECTION )
            break;

        if (isInitial) {
            style->setContentNormal();
            return;
        }

        if (primitiveValue && primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_IDENT) {
            // normal | none
            if (primitiveValue->getIdent() == CSS_VAL_NORMAL)
                style->setContentNormal();
            else
            if (primitiveValue->getIdent() == CSS_VAL_NONE)
                style->setContentNone();
            else
                assert(false);
            return;
        }

        if(!value->isValueList()) return;
        CSSValueListImpl *list = static_cast<CSSValueListImpl *>(value);
        int len = list->length();

        style->setContentNormal(); // clear the content

        for(int i = 0; i < len; i++) {
            CSSValueImpl *item = list->item(i);
            if(!item->isPrimitiveValue()) continue;
            CSSPrimitiveValueImpl *val = static_cast<CSSPrimitiveValueImpl *>(item);
            if(val->primitiveType()==CSSPrimitiveValue::CSS_STRING)
            {
                style->addContent(val->getStringValue());
            }
            else if (val->primitiveType()==CSSPrimitiveValue::CSS_ATTR)
            {
                // TODO: setup dynamic attribute dependencies
                int attrID = element->getDocument()->getId(NodeImpl::AttributeId, val->getStringValue(), false, true);
                if (attrID)
                    style->addContent(element->getAttribute(attrID).implementation());
                else
                    kdDebug(6080) << "Attribute \"" << val->getStringValue() << "\" not found" << endl;
            }
            else if (val->primitiveType()==CSSPrimitiveValue::CSS_URI)
            {
                CSSImageValueImpl *image = static_cast<CSSImageValueImpl *>(val);
                style->addContent(image->image());
            }
            else if (val->primitiveType()==CSSPrimitiveValue::CSS_COUNTER)
            {
                style->addContent(val->getCounterValue());
            }
            else if (val->primitiveType()==CSSPrimitiveValue::CSS_IDENT)
            {
                EQuoteContent quote;
                switch (val->getIdent()) {
                    case CSS_VAL_OPEN_QUOTE:
                        quote = OPEN_QUOTE;
                        break;
                    case CSS_VAL_NO_OPEN_QUOTE:
                        quote = NO_OPEN_QUOTE;
                        break;
                    case CSS_VAL_CLOSE_QUOTE:
                        quote = CLOSE_QUOTE;
                        break;
                    case CSS_VAL_NO_CLOSE_QUOTE:
                        quote = NO_CLOSE_QUOTE;
                        break;
                    default:
                        assert(false);
                }
                style->addContent(quote);
            } else
                kdDebug(6080) << "Unrecognized CSS content" << endl;

        }
        break;
    }

    case CSS_PROP_COUNTER_INCREMENT: {
        if(!value->isValueList()) return;

        CSSValueListImpl *list = static_cast<CSSValueListImpl *>(value);
        style->setCounterIncrement(list);
        break;
    }
    case CSS_PROP_COUNTER_RESET: {
        if(!value->isValueList()) return;

        CSSValueListImpl *list = static_cast<CSSValueListImpl *>(value);
        style->setCounterReset(list);
        break;
    }
    case CSS_PROP_FONT_FAMILY:
        // list of strings and ids
    {
        if (isInherit) {
            FontDef parentFontDef = parentStyle->htmlFont().fontDef;
            FontDef fontDef = style->htmlFont().fontDef;
            fontDef.family = parentFontDef.family;
            if (style->setFontDef(fontDef))
                fontDirty = true;
            return;
        }
        else if (isInitial) {
            FontDef fontDef = style->htmlFont().fontDef;
            FontDef initialDef = FontDef();
#ifdef APPLE_CHANGES
            fontDef.family = initialDef.firstFamily();
#else
            fontDef.family = TQString::null;
#endif
            if (style->setFontDef(fontDef))
                fontDirty = true;
            return;
        }
        if(!value->isValueList()) return;
	FontDef fontDef = style->htmlFont().fontDef;
        CSSValueListImpl *list = static_cast<CSSValueListImpl *>(value);
        int len = list->length();
        for(int i = 0; i < len; i++) {
            CSSValueImpl *item = list->item(i);
            if(!item->isPrimitiveValue()) continue;
            CSSPrimitiveValueImpl *val = static_cast<CSSPrimitiveValueImpl *>(item);
	    TQString face;
            if( val->primitiveType() == CSSPrimitiveValue::CSS_STRING )
		face = static_cast<FontFamilyValueImpl *>(val)->fontName();
	    else if ( val->primitiveType() == CSSPrimitiveValue::CSS_IDENT ) {
		switch( val->getIdent() ) {
		case CSS_VAL_SERIF:
		    face = settings->serifFontName();
		    break;
		case CSS_VAL_SANS_SERIF:
		    face = settings->sansSerifFontName();
		    break;
		case CSS_VAL_CURSIVE:
		    face = settings->cursiveFontName();
		    break;
		case CSS_VAL_FANTASY:
		    face = settings->fantasyFontName();
		    break;
		case CSS_VAL_MONOSPACE:
		    face = settings->fixedFontName();
		    break;
		default:
		    return;
		}
	    } else {
		return;
	    }
	    if ( !face.isEmpty() ) {
		fontDef.family = face;
		fontDirty |= style->setFontDef( fontDef );
                return;
	    }
	}
        break;
    }
    case CSS_PROP_QUOTES:
        HANDLE_INHERIT_AND_INITIAL(quotes, Quotes)
        if(primitiveValue && primitiveValue->getIdent() == CSS_VAL_NONE) {
            // set a set of empty quotes
            QuotesValueImpl* quotes = new QuotesValueImpl();
            style->setQuotes(quotes);
        } else {
            QuotesValueImpl* quotes = static_cast<QuotesValueImpl *>(value);
            style->setQuotes(quotes);
        }
        break;
    case CSS_PROP_SIZE:
        // ### look up
      break;
    case CSS_PROP_TEXT_DECORATION: {
        // list of ident
        HANDLE_INHERIT_AND_INITIAL(textDecoration, TextDecoration)
        int t = RenderStyle::initialTextDecoration();
        if(primitiveValue && primitiveValue->getIdent() == CSS_VAL_NONE) {
	    // do nothing
	} else {
	    if(!value->isValueList()) return;
	    CSSValueListImpl *list = static_cast<CSSValueListImpl *>(value);
	    int len = list->length();
	    for(int i = 0; i < len; i++)
	    {
		CSSValueImpl *item = list->item(i);
		if(!item->isPrimitiveValue()) continue;
		primitiveValue = static_cast<CSSPrimitiveValueImpl *>(item);
		switch(primitiveValue->getIdent())
		{
		    case CSS_VAL_NONE:
			t = TDNONE; break;
		    case CSS_VAL_UNDERLINE:
			t |= UNDERLINE; break;
		    case CSS_VAL_OVERLINE:
			t |= OVERLINE; break;
		    case CSS_VAL_LINE_THROUGH:
			t |= LINE_THROUGH; break;
		    case CSS_VAL_BLINK:
			t |= BLINK; break;
		    default:
			return;
		}
	    }
        }
	style->setTextDecoration(t);
        break;
    }
    case CSS_PROP__TDEHTML_FLOW_MODE:
        HANDLE_INHERIT_AND_INITIAL(flowAroundFloats, FlowAroundFloats)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent()) {
            style->setFlowAroundFloats( primitiveValue->getIdent() == CSS_VAL__TDEHTML_AROUND_FLOATS );
            return;
        }
        break;
    case CSS_PROP__TDEHTML_USER_INPUT: {
        if(value->cssValueType() == CSSValue::CSS_INHERIT)
        {
            if(!parentNode) return;
            style->setUserInput(parentStyle->userInput());
//	    kdDebug() << "UI erm" << endl;
            return;
        }
        if(!primitiveValue) return;
        int id = primitiveValue->getIdent();
	if (id == CSS_VAL_NONE)
	    style->setUserInput(UI_NONE);
	else
	    style->setUserInput(EUserInput(id - CSS_VAL_ENABLED));
//	kdDebug(6080) << "userInput: " << style->userEdit() << endl;
	return;
    }

// shorthand properties
    case CSS_PROP_BACKGROUND:
        if (isInitial) {
            style->clearBackgroundLayers();
            return;
        }
        else if (isInherit) {
            if (parentStyle)
                style->inheritBackgroundLayers(*parentStyle->backgroundLayers());
            else
                style->clearBackgroundLayers();
            return;
        }
        break;
    case CSS_PROP_BORDER:
    case CSS_PROP_BORDER_STYLE:
    case CSS_PROP_BORDER_WIDTH:
    case CSS_PROP_BORDER_COLOR:
        if(id == CSS_PROP_BORDER || id == CSS_PROP_BORDER_COLOR)
        {
             if (isInherit) {
                style->setBorderTopColor(parentStyle->borderTopColor());
                style->setBorderBottomColor(parentStyle->borderBottomColor());
                style->setBorderLeftColor(parentStyle->borderLeftColor());
                style->setBorderRightColor(parentStyle->borderRightColor());
            }
            else if (isInitial) {
                style->setBorderTopColor(TQColor()); // Reset to invalid color so currentColor is used instead.
                style->setBorderBottomColor(TQColor());
                style->setBorderLeftColor(TQColor());
                style->setBorderRightColor(TQColor());
            }
        }
        if (id == CSS_PROP_BORDER || id == CSS_PROP_BORDER_STYLE)
        {
            if (isInherit) {
                style->setBorderTopStyle(parentStyle->borderTopStyle());
                style->setBorderBottomStyle(parentStyle->borderBottomStyle());
                style->setBorderLeftStyle(parentStyle->borderLeftStyle());
                style->setBorderRightStyle(parentStyle->borderRightStyle());
            }
            else if (isInitial) {
                style->setBorderTopStyle(RenderStyle::initialBorderStyle());
                style->setBorderBottomStyle(RenderStyle::initialBorderStyle());
                style->setBorderLeftStyle(RenderStyle::initialBorderStyle());
                style->setBorderRightStyle(RenderStyle::initialBorderStyle());
            }
        }
        if (id == CSS_PROP_BORDER || id == CSS_PROP_BORDER_WIDTH)
        {
            if (isInherit) {
                style->setBorderTopWidth(parentStyle->borderTopWidth());
                style->setBorderBottomWidth(parentStyle->borderBottomWidth());
                style->setBorderLeftWidth(parentStyle->borderLeftWidth());
                style->setBorderRightWidth(parentStyle->borderRightWidth());
            }
            else if (isInitial) {
                style->setBorderTopWidth(RenderStyle::initialBorderWidth());
                style->setBorderBottomWidth(RenderStyle::initialBorderWidth());
                style->setBorderLeftWidth(RenderStyle::initialBorderWidth());
                style->setBorderRightWidth(RenderStyle::initialBorderWidth());
            }
        }
        return;
    case CSS_PROP_BORDER_TOP:
        if ( isInherit ) {
            style->setBorderTopColor(parentStyle->borderTopColor());
            style->setBorderTopStyle(parentStyle->borderTopStyle());
            style->setBorderTopWidth(parentStyle->borderTopWidth());
        } else if (isInitial)
            style->resetBorderTop();
        return;
    case CSS_PROP_BORDER_RIGHT:
        if (isInherit) {
            style->setBorderRightColor(parentStyle->borderRightColor());
            style->setBorderRightStyle(parentStyle->borderRightStyle());
            style->setBorderRightWidth(parentStyle->borderRightWidth());
        }
        else if (isInitial)
            style->resetBorderRight();
        return;
    case CSS_PROP_BORDER_BOTTOM:
        if (isInherit) {
            style->setBorderBottomColor(parentStyle->borderBottomColor());
            style->setBorderBottomStyle(parentStyle->borderBottomStyle());
            style->setBorderBottomWidth(parentStyle->borderBottomWidth());
        }
        else if (isInitial)
            style->resetBorderBottom();
        return;
    case CSS_PROP_BORDER_LEFT:
        if (isInherit) {
            style->setBorderLeftColor(parentStyle->borderLeftColor());
            style->setBorderLeftStyle(parentStyle->borderLeftStyle());
            style->setBorderLeftWidth(parentStyle->borderLeftWidth());
        }
        else if (isInitial)
            style->resetBorderLeft();
        return;
    case CSS_PROP_MARGIN:
        if (isInherit) {
            style->setMarginTop(parentStyle->marginTop());
            style->setMarginBottom(parentStyle->marginBottom());
            style->setMarginLeft(parentStyle->marginLeft());
            style->setMarginRight(parentStyle->marginRight());
        }
        else if (isInitial)
            style->resetMargin();
        return;
    case CSS_PROP_PADDING:
        if (isInherit) {
            style->setPaddingTop(parentStyle->paddingTop());
            style->setPaddingBottom(parentStyle->paddingBottom());
            style->setPaddingLeft(parentStyle->paddingLeft());
            style->setPaddingRight(parentStyle->paddingRight());
        }
        else if (isInitial)
            style->resetPadding();
        return;
    case CSS_PROP_FONT:
        if ( isInherit ) {
            FontDef fontDef = parentStyle->htmlFont().fontDef;
	    style->setLineHeight( parentStyle->lineHeight() );
	    fontDirty |= style->setFontDef( fontDef );
        } else if (isInitial) {
            FontDef fontDef;
            style->setLineHeight(RenderStyle::initialLineHeight());
            if (style->setFontDef( fontDef ))
                fontDirty = true;
	} else if ( value->isFontValue() ) {
	    FontValueImpl *font = static_cast<FontValueImpl *>(value);
	    if ( !font->style || !font->variant || !font->weight ||
		 !font->size || !font->lineHeight || !font->family )
		return;
	    applyRule( CSS_PROP_FONT_STYLE, font->style );
	    applyRule( CSS_PROP_FONT_VARIANT, font->variant );
	    applyRule( CSS_PROP_FONT_WEIGHT, font->weight );
	    applyRule( CSS_PROP_FONT_SIZE, font->size );

            // Line-height can depend on font().pixelSize(), so we have to update the font
            // before we evaluate line-height, e.g., font: 1em/1em.  FIXME: Still not
            // good enough: style="font:1em/1em; font-size:36px" should have a line-height of 36px.
            if (fontDirty)
                CSSStyleSelector::style->htmlFont().update( paintDeviceMetrics );

	    applyRule( CSS_PROP_LINE_HEIGHT, font->lineHeight );
	    applyRule( CSS_PROP_FONT_FAMILY, font->family );
	}
	return;

    case CSS_PROP_LIST_STYLE:
        if (isInherit) {
            style->setListStyleType(parentStyle->listStyleType());
            style->setListStyleImage(parentStyle->listStyleImage());
            style->setListStylePosition(parentStyle->listStylePosition());
        }
        else if (isInitial) {
            style->setListStyleType(RenderStyle::initialListStyleType());
            style->setListStyleImage(RenderStyle::initialListStyleImage());
            style->setListStylePosition(RenderStyle::initialListStylePosition());
        }
        break;
    case CSS_PROP_OUTLINE:
        if (isInherit) {
            style->setOutlineWidth(parentStyle->outlineWidth());
            style->setOutlineColor(parentStyle->outlineColor());
            style->setOutlineStyle(parentStyle->outlineStyle());
        }
        else if (isInitial)
            style->resetOutline();
        break;
    /* CSS3 properties */
    case CSS_PROP_BOX_SIZING:
        HANDLE_INHERIT(boxSizing, BoxSizing)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent() == CSS_VAL_CONTENT_BOX)
            style->setBoxSizing(CONTENT_BOX);
        else
        if (primitiveValue->getIdent() == CSS_VAL_BORDER_BOX)
            style->setBoxSizing(BORDER_BOX);
        break;
    case CSS_PROP_OUTLINE_OFFSET: {
        HANDLE_INHERIT_AND_INITIAL(outlineOffset, OutlineOffset)

        int offset = primitiveValue->computeLength(style, paintDeviceMetrics);
        if (offset < 0) return;

        style->setOutlineOffset(offset);
        break;
    }
    case CSS_PROP_TEXT_SHADOW: {
        if (isInherit) {
            style->setTextShadow(parentStyle->textShadow() ? new ShadowData(*parentStyle->textShadow()) : 0);
            return;
        }
        else if (isInitial) {
            style->setTextShadow(0);
            return;
        }

        if (primitiveValue) { // none
            style->setTextShadow(0);
            return;
        }

        if (!value->isValueList()) return;
        CSSValueListImpl *list = static_cast<CSSValueListImpl *>(value);
        int len = list->length();
        for (int i = 0; i < len; i++) {
            ShadowValueImpl *item = static_cast<ShadowValueImpl*>(list->item(i));

            int x = item->x->computeLength(style, paintDeviceMetrics);
            int y = item->y->computeLength(style, paintDeviceMetrics);
            int blur = item->blur ? item->blur->computeLength(style, paintDeviceMetrics) : 0;
            TQColor col = tdehtml::transparentColor;
            if (item->color) {
                int ident = item->color->getIdent();
                if (ident)
                    col = colorForCSSValue( ident );
                else if (item->color->primitiveType() == CSSPrimitiveValue::CSS_RGBCOLOR)
                    col.setRgb(item->color->getRGBColorValue());
            }
            ShadowData* shadowData = new ShadowData(x, y, blur, col);
            style->setTextShadow(shadowData, i != 0);
        }

        break;
    }
    case CSS_PROP_OPACITY:
        HANDLE_INHERIT_AND_INITIAL(opacity, Opacity)
        if (!primitiveValue || primitiveValue->primitiveType() != CSSPrimitiveValue::CSS_NUMBER)
            return; // Error case.

        // Clamp opacity to the range 0-1
        style->setOpacity(kMin(1.0f, kMax(0.0f, (float)primitiveValue->floatValue(CSSPrimitiveValue::CSS_NUMBER))));
        break;
    case CSS_PROP__TDEHTML_MARQUEE:
        if (value->cssValueType() != CSSValue::CSS_INHERIT || !parentNode) return;
        style->setMarqueeDirection(parentStyle->marqueeDirection());
        style->setMarqueeIncrement(parentStyle->marqueeIncrement());
        style->setMarqueeSpeed(parentStyle->marqueeSpeed());
        style->setMarqueeLoopCount(parentStyle->marqueeLoopCount());
        style->setMarqueeBehavior(parentStyle->marqueeBehavior());
        break;
    case CSS_PROP__TDEHTML_MARQUEE_REPETITION: {
        HANDLE_INHERIT_AND_INITIAL(marqueeLoopCount, MarqueeLoopCount)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent() == CSS_VAL_INFINITE)
            style->setMarqueeLoopCount(-1); // -1 means repeat forever.
        else if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_NUMBER)
            style->setMarqueeLoopCount((int)(primitiveValue->floatValue(CSSPrimitiveValue::CSS_NUMBER)));
        break;
    }
    case CSS_PROP__TDEHTML_MARQUEE_SPEED: {
        HANDLE_INHERIT_AND_INITIAL(marqueeSpeed, MarqueeSpeed)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent()) {
            switch (primitiveValue->getIdent())
            {
                case CSS_VAL_SLOW:
                    style->setMarqueeSpeed(500); // 500 msec.
                    break;
                case CSS_VAL_NORMAL:
                    style->setMarqueeSpeed(85); // 85msec. The WinIE default.
                    break;
                case CSS_VAL_FAST:
                    style->setMarqueeSpeed(10); // 10msec. Super fast.
                    break;
            }
        }
        else if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_S)
            style->setMarqueeSpeed(int(1000*primitiveValue->floatValue(CSSPrimitiveValue::CSS_S)));
        else if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_MS)
            style->setMarqueeSpeed(int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_MS)));
        else if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_NUMBER) // For scrollamount support.
            style->setMarqueeSpeed(int(primitiveValue->floatValue(CSSPrimitiveValue::CSS_NUMBER)));
        break;
    }
    case CSS_PROP__TDEHTML_MARQUEE_INCREMENT: {
        HANDLE_INHERIT_AND_INITIAL(marqueeIncrement, MarqueeIncrement)
        if (!primitiveValue) return;
        if (primitiveValue->getIdent()) {
            switch (primitiveValue->getIdent())
            {
                case CSS_VAL_SMALL:
                    style->setMarqueeIncrement(Length(1, Fixed)); // 1px.
                    break;
                case CSS_VAL_NORMAL:
                    style->setMarqueeIncrement(Length(6, Fixed)); // 6px. The WinIE default.
                    break;
                case CSS_VAL_LARGE:
                    style->setMarqueeIncrement(Length(36, Fixed)); // 36px.
                    break;
            }
        }
        else {
            bool ok = true;
            Length l = convertToLength(primitiveValue, style, paintDeviceMetrics, &ok);
            if (ok)
                style->setMarqueeIncrement(l);
        }
        break;
    }
    case CSS_PROP__TDEHTML_MARQUEE_STYLE: {
        HANDLE_INHERIT_AND_INITIAL(marqueeBehavior, MarqueeBehavior)
        if (!primitiveValue || !primitiveValue->getIdent()) return;
        switch (primitiveValue->getIdent())
        {
            case CSS_VAL_NONE:
                style->setMarqueeBehavior(MNONE);
                break;
            case CSS_VAL_SCROLL:
                style->setMarqueeBehavior(MSCROLL);
                break;
            case CSS_VAL_SLIDE:
                style->setMarqueeBehavior(MSLIDE);
                break;
            case CSS_VAL_ALTERNATE:
                style->setMarqueeBehavior(MALTERNATE);
                break;
            case CSS_VAL_UNFURL:
                style->setMarqueeBehavior(MUNFURL);
                break;
        }
        break;
    }
    case CSS_PROP__TDEHTML_MARQUEE_DIRECTION: {
        HANDLE_INHERIT_AND_INITIAL(marqueeDirection, MarqueeDirection)
        if (!primitiveValue || !primitiveValue->getIdent()) return;
        switch (primitiveValue->getIdent())
        {
            case CSS_VAL_FORWARDS:
                style->setMarqueeDirection(MFORWARD);
                break;
            case CSS_VAL_BACKWARDS:
                style->setMarqueeDirection(MBACKWARD);
                break;
            case CSS_VAL_AUTO:
                style->setMarqueeDirection(MAUTO);
                break;
            case CSS_VAL_AHEAD:
            case CSS_VAL_UP: // We don't support vertical languages, so AHEAD just maps to UP.
                style->setMarqueeDirection(MUP);
                break;
            case CSS_VAL_REVERSE:
            case CSS_VAL_DOWN: // REVERSE just maps to DOWN, since we don't do vertical text.
                style->setMarqueeDirection(MDOWN);
                break;
            case CSS_VAL_LEFT:
                style->setMarqueeDirection(MLEFT);
                break;
            case CSS_VAL_RIGHT:
                style->setMarqueeDirection(MRIGHT);
                break;
        }
        break;
    case CSS_PROP_TEXT_OVERFLOW: {
        // This property is supported by WinIE, and so we leave off the "-khtml-" in order to
        // work with WinIE-specific pages that use the property.
        HANDLE_INHERIT_AND_INITIAL(textOverflow, TextOverflow)
        if (!primitiveValue || !primitiveValue->getIdent())
            return;
        style->setTextOverflow(primitiveValue->getIdent() == CSS_VAL_ELLIPSIS);
        break;
    }
    }
    case CSS_PROP_WORD_WRAP: {
        HANDLE_INITIAL_AND_INHERIT_ON_INHERITED_PROPERTY(wordWrap, WordWrap)
        if (!primitiveValue)
            return;
        style->setWordWrap(primitiveValue->getIdent() == CSS_VAL_NORMAL ? WWNORMAL : WWBREAKWORD);
        break;
    }
    default:
        return;
    }
}

void CSSStyleSelector::mapBackgroundAttachment(BackgroundLayer* layer, DOM::CSSValueImpl* value)
{
    if (value->cssValueType() == CSSValue::CSS_INITIAL) {
        layer->setBackgroundAttachment(RenderStyle::initialBackgroundAttachment());
        return;
    }

    if (!value->isPrimitiveValue()) return;
    CSSPrimitiveValueImpl* primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);
    switch (primitiveValue->getIdent()) {
        case CSS_VAL_FIXED:
            layer->setBackgroundAttachment(false);
            break;
        case CSS_VAL_SCROLL:
            layer->setBackgroundAttachment(true);
            break;
        default:
            return;
    }
}

void CSSStyleSelector::mapBackgroundClip(BackgroundLayer* layer, CSSValueImpl* value)
{
    if (value->cssValueType() == CSSValue::CSS_INITIAL) {
        layer->setBackgroundClip(RenderStyle::initialBackgroundClip());
        return;
    }

    if (!value->isPrimitiveValue()) return;
    CSSPrimitiveValueImpl* primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);
    switch (primitiveValue->getIdent()) {
        case CSS_VAL_BORDER:
            layer->setBackgroundClip(BGBORDER);
            break;
        case CSS_VAL_PADDING:
            layer->setBackgroundClip(BGPADDING);
            break;
        default: // CSS_VAL_CONTENT
            layer->setBackgroundClip(BGCONTENT);
            break;
    }
}

void CSSStyleSelector::mapBackgroundOrigin(BackgroundLayer* layer, CSSValueImpl* value)
{
    if (value->cssValueType() == CSSValue::CSS_INITIAL) {
        layer->setBackgroundOrigin(RenderStyle::initialBackgroundOrigin());
        return;
    }

    if (!value->isPrimitiveValue()) return;
    CSSPrimitiveValueImpl* primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);
    switch (primitiveValue->getIdent()) {
        case CSS_VAL_BORDER:
            layer->setBackgroundOrigin(BGBORDER);
            break;
        case CSS_VAL_PADDING:
            layer->setBackgroundOrigin(BGPADDING);
            break;
        default: // CSS_VAL_CONTENT
            layer->setBackgroundOrigin(BGCONTENT);
            break;
    }
}

void CSSStyleSelector::mapBackgroundImage(BackgroundLayer* layer, DOM::CSSValueImpl* value)
{
    if (value->cssValueType() == CSSValue::CSS_INITIAL) {
        layer->setBackgroundImage(RenderStyle::initialBackgroundImage());
        return;
    }

    if (!value->isPrimitiveValue()) return;
    CSSPrimitiveValueImpl* primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);
    layer->setBackgroundImage(static_cast<CSSImageValueImpl *>(primitiveValue)->image());
}

void CSSStyleSelector::mapBackgroundRepeat(BackgroundLayer* layer, DOM::CSSValueImpl* value)
{
    if (value->cssValueType() == CSSValue::CSS_INITIAL) {
        layer->setBackgroundRepeat(RenderStyle::initialBackgroundRepeat());
        return;
    }

    if (!value->isPrimitiveValue()) return;
    CSSPrimitiveValueImpl* primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);
    switch(primitiveValue->getIdent()) {
	case CSS_VAL_REPEAT:
	    layer->setBackgroundRepeat(REPEAT);
	    break;
	case CSS_VAL_REPEAT_X:
	    layer->setBackgroundRepeat(REPEAT_X);
	    break;
	case CSS_VAL_REPEAT_Y:
	    layer->setBackgroundRepeat(REPEAT_Y);
	    break;
	case CSS_VAL_NO_REPEAT:
	    layer->setBackgroundRepeat(NO_REPEAT);
	    break;
	default:
	    return;
    }
}


void CSSStyleSelector::mapBackgroundSize(BackgroundLayer* layer, CSSValueImpl* value)
{
    LengthSize b = RenderStyle::initialBackgroundSize();

    if (value->cssValueType() == CSSValue::CSS_INITIAL) {
        layer->setBackgroundSize(b);
        return;
    }

    if (!value->isPrimitiveValue())
        return;

    CSSPrimitiveValueImpl* primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);
    PairImpl* pair = primitiveValue->getPairValue();
    if (!pair)
        return;

    CSSPrimitiveValueImpl* first = static_cast<CSSPrimitiveValueImpl*>(pair->first());
    CSSPrimitiveValueImpl* second = static_cast<CSSPrimitiveValueImpl*>(pair->second());

    if (!first || !second)
        return;

    Length firstLength, secondLength;
    int firstType = first->primitiveType();
    int secondType = second->primitiveType();

    if (firstType == CSSPrimitiveValue::CSS_UNKNOWN)
        firstLength = Length(Variable);
    else if (firstType > CSSPrimitiveValue::CSS_PERCENTAGE && firstType < CSSPrimitiveValue::CSS_DEG)
        firstLength = Length(first->computeLength(style, paintDeviceMetrics), Fixed);
    else if (firstType == CSSPrimitiveValue::CSS_PERCENTAGE)
        firstLength = Length((int)first->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
    else
        return;

    if (secondType == CSSPrimitiveValue::CSS_UNKNOWN)
        secondLength = Length(Variable);
    else if (secondType > CSSPrimitiveValue::CSS_PERCENTAGE && secondType < CSSPrimitiveValue::CSS_DEG)
        secondLength = Length(second->computeLength(style, paintDeviceMetrics), Fixed);
    else if (secondType == CSSPrimitiveValue::CSS_PERCENTAGE)
        secondLength = Length((int)second->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
    else
        return;

    b.width = firstLength;
    b.height = secondLength;
    layer->setBackgroundSize(b);
}

void CSSStyleSelector::mapBackgroundXPosition(BackgroundLayer* layer, DOM::CSSValueImpl* value)
{
    if (value->cssValueType() == CSSValue::CSS_INITIAL) {
        layer->setBackgroundXPosition(RenderStyle::initialBackgroundXPosition());
        return;
    }

    if (!value->isPrimitiveValue()) return;
    CSSPrimitiveValueImpl* primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);
    Length l;
    int type = primitiveValue->primitiveType();
    if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
        l = Length(primitiveValue->computeLength(style, paintDeviceMetrics), Fixed);
    else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
        l = Length((int)primitiveValue->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
    else
        return;
    layer->setBackgroundXPosition(l);
}

void CSSStyleSelector::mapBackgroundYPosition(BackgroundLayer* layer, DOM::CSSValueImpl* value)
{
    if (value->cssValueType() == CSSValue::CSS_INITIAL) {
        layer->setBackgroundYPosition(RenderStyle::initialBackgroundYPosition());
        return;
    }

    if (!value->isPrimitiveValue()) return;
    CSSPrimitiveValueImpl* primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);
    Length l;
    int type = primitiveValue->primitiveType();
    if(type > CSSPrimitiveValue::CSS_PERCENTAGE && type < CSSPrimitiveValue::CSS_DEG)
        l = Length(primitiveValue->computeLength(style, paintDeviceMetrics), Fixed);
    else if(type == CSSPrimitiveValue::CSS_PERCENTAGE)
        l = Length((int)primitiveValue->floatValue(CSSPrimitiveValue::CSS_PERCENTAGE), Percent);
    else
        return;
    layer->setBackgroundYPosition(l);
}

#ifdef APPLE_CHANGES
void CSSStyleSelector::checkForGenericFamilyChange(RenderStyle* aStyle, RenderStyle* aParentStyle)
{
  const FontDef& childFont = aStyle->htmlFont().fontDef;

  if (childFont.sizeSpecified || !aParentStyle)
    return;

  const FontDef& parentFont = aParentStyle->htmlFont().fontDef;

  if (childFont.genericFamily == parentFont.genericFamily)
    return;

  // For now, lump all families but monospace together.
  if (childFont.genericFamily != FontDef::eMonospace &&
      parentFont.genericFamily != FontDef::eMonospace)
    return;

  // We know the parent is monospace or the child is monospace, and that font
  // size was unspecified.  We want to alter our font size to use the correct
  // "medium" font for our family.
  float size = 0;
  int minFontSize = settings->minFontSize();
  size = (childFont.genericFamily == FontDef::eMonospace) ? m_fixedFontSizes[3] : m_fontSizes[3];
  int isize = (int)size;
  if (isize < minFontSize)
    isize = minFontSize;

  FontDef newFontDef(childFont);
  newFontDef.size = isize;
  aStyle->setFontDef(newFontDef);
}
#endif

} // namespace tdehtml
