/*
    This file is part of the KDE libraries

    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001-2003 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2003 Apple Computer, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.

    // regarding the LRU:
    // http://www.is.kyusan-u.ac.jp/~chengk/pub/papers/compsac00_A07-07.pdf
*/

#undef CACHE_DEBUG
//#define CACHE_DEBUG

#ifdef CACHE_DEBUG
#define CDEBUG kdDebug(6060)
#else
#define CDEBUG kndDebug()
#endif

#undef LOADER_DEBUG
//#define LOADER_DEBUG

#include <assert.h>

#include "misc/loader.h"
#include "misc/seed.h"

// default cache size
#define DEFCACHESIZE 2096*1024
#define MAX_JOB_COUNT 32

#include <tqasyncio.h>
#include <tqasyncimageio.h>
#include <tqpainter.h>
#include <tqbitmap.h>
#include <tqmovie.h>
#include <tqwidget.h>

#include <tdeapplication.h>
#include <tdeio/job.h>
#include <tdeio/jobclasses.h>
#include <tdeglobal.h>
#include <kimageio.h>
#include <kcharsets.h>
#include <kiconloader.h>
#include <scheduler.h>
#include <kdebug.h>
#include "tdehtml_factory.h"
#include "tdehtml_part.h"

#ifdef IMAGE_TITLES
#include <tqfile.h>
#include <tdefilemetainfo.h>
#include <tdetempfile.h>
#endif

#include "html/html_documentimpl.h"
#include "css/css_stylesheetimpl.h"
#include "xml/dom_docimpl.h"

#include "blocked_icon.cpp"

using namespace tdehtml;
using namespace DOM;

#define MAX_LRU_LISTS 20
struct LRUList {
    CachedObject* m_head;
    CachedObject* m_tail;

    LRUList() : m_head(0), m_tail(0) {}
};

static LRUList m_LRULists[MAX_LRU_LISTS];
static LRUList* getLRUListFor(CachedObject* o);

CachedObjectClient::~CachedObjectClient()
{
}

CachedObject::~CachedObject()
{
    Cache::removeFromLRUList(this);
}

void CachedObject::finish()
{
    m_status = Cached;
}

bool CachedObject::isExpired() const
{
    if (!m_expireDate) return false;
    time_t now = time(0);
    return (difftime(now, m_expireDate) >= 0);
}

void CachedObject::setRequest(Request *_request)
{
    if ( _request && !m_request )
        m_status = Pending;

    if ( allowInLRUList() )
        Cache::removeFromLRUList( this );

    m_request = _request;

    if ( allowInLRUList() )
        Cache::insertInLRUList( this );
}

void CachedObject::ref(CachedObjectClient *c)
{
    // unfortunately we can be ref'ed multiple times from the
    // same object,  because it uses e.g. the same foreground
    // and the same background picture. so deal with it.
    m_clients.insert(c,c);
    Cache::removeFromLRUList(this);
    m_accessCount++;
}

void CachedObject::deref(CachedObjectClient *c)
{
    assert( c );
    assert( m_clients.count() );
    assert( !canDelete() );
    assert( m_clients.find( c ) );

    Cache::flush();

    m_clients.remove(c);

    if (allowInLRUList())
        Cache::insertInLRUList(this);
}

void CachedObject::setSize(int size)
{
    bool sizeChanged;

    if ( !m_next && !m_prev && getLRUListFor(this)->m_head != this )
        sizeChanged = false;
    else
        sizeChanged = ( size - m_size ) != 0;

    // The object must now be moved to a different queue,
    // since its size has been changed.
    if ( sizeChanged  && allowInLRUList())
        Cache::removeFromLRUList(this);

    m_size = size;

    if ( sizeChanged && allowInLRUList())
        Cache::insertInLRUList(this);
}

TQTextCodec* CachedObject::codecForBuffer( const TQString& charset, const TQByteArray& buffer ) const
{
    // we don't use heuristicContentMatch here since it is a) far too slow and
    // b) having too much functionality for our case.

    uchar* d = ( uchar* ) buffer.data();
    int s = buffer.size();

    // BOM
    if ( s >= 3 &&
         d[0] == 0xef && d[1] == 0xbb && d[2] == 0xbf)
         return TQTextCodec::codecForMib( 106 ); // UTF-8

    if ( s >= 2 && ((d[0] == 0xff && d[1] == 0xfe) ||
                    (d[0] == 0xfe && d[1] == 0xff)))
        return TQTextCodec::codecForMib( 1000 ); // UCS-2

    // Link or @charset
    if(!charset.isEmpty())
    {
	TQTextCodec* c = TDEGlobal::charsets()->codecForName(charset);
        if(c->mibEnum() == 11)  {
            // iso8859-8 (visually ordered)
            c = TQTextCodec::codecForName("iso8859-8-i");
        }
        return c;
    }

    // Default
    return TQTextCodec::codecForMib( 4 ); // latin 1
}

// -------------------------------------------------------------------------------------------

CachedCSSStyleSheet::CachedCSSStyleSheet(DocLoader* dl, const DOMString &url, TDEIO::CacheControl _cachePolicy,
					 const char *accept)
    : CachedObject(url, CSSStyleSheet, _cachePolicy, 0)
{
    // Set the type we want (probably css or xml)
    TQString ah = TQString::fromLatin1( accept );
    if ( !ah.isEmpty() )
        ah += ",";
    ah += "*/*;q=0.1";
    setAccept( ah );
    m_hadError = false;
    m_wasBlocked = false;
    m_err = 0;
    // load the file
    Cache::loader()->load(dl, this, false);
    m_loading = true;
}

CachedCSSStyleSheet::CachedCSSStyleSheet(const DOMString &url, const TQString &stylesheet_data)
    : CachedObject(url, CSSStyleSheet, TDEIO::CC_Verify, stylesheet_data.length())
{
    m_loading = false;
    m_status = Persistent;
    m_sheet = DOMString(stylesheet_data);
}


void CachedCSSStyleSheet::ref(CachedObjectClient *c)
{
    CachedObject::ref(c);

    if (!m_loading) {
	if (m_hadError)
	    c->error( m_err, m_errText );
	else
	    c->setStyleSheet( m_url, m_sheet, m_charset );
    }
}

void CachedCSSStyleSheet::data( TQBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    setSize(buffer.buffer().size());

//     TQString charset = checkCharset( buffer.buffer() );
    TQTextCodec* c = 0;
    if (!m_charset.isEmpty()) {
        c = TDEGlobal::charsets()->codecForName(m_charset);
        if(c->mibEnum() == 11)  c = TQTextCodec::codecForName("iso8859-8-i");
    }
    else {
        c = codecForBuffer( m_charsetHint, buffer.buffer() );
        m_charset = c->name();
    }
    TQString data = c->toUnicode( buffer.buffer().data(), m_size );
    // workaround TQt bugs
    m_sheet = static_cast<TQChar>(data[0]) == TQChar::byteOrderMark ? DOMString(data.mid( 1 ) ) : DOMString(data);
    m_loading = false;

    checkNotify();
}

void CachedCSSStyleSheet::checkNotify()
{
    if(m_loading || m_hadError) return;

    CDEBUG << "CachedCSSStyleSheet:: finishedLoading " << m_url.string() << endl;

    // it() first increments, then returnes the current item.
    // this avoids skipping an item when setStyleSheet deletes the "current" one.
    for (TQPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
        it()->setStyleSheet( m_url, m_sheet, m_charset );
}


void CachedCSSStyleSheet::error( int err, const char* text )
{
    m_hadError = true;
    m_err = err;
    m_errText = text;
    m_loading = false;

    // it() first increments, then returnes the current item.
    // this avoids skipping an item when setStyleSheet deletes the "current" one.
    for (TQPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
        it()->error( m_err, m_errText );
}

#if 0
TQString CachedCSSStyleSheet::checkCharset(const TQByteArray& buffer ) const
{
    int s = buffer.size();
    if (s <= 12) return m_charset;

    // @charset has to be first or directly after BOM.
    // CSS 2.1 says @charset should win over BOM, but since more browsers support BOM
    // than @charset, we default to that.
    const char* d = (const char*) buffer.data();
    if (strncmp(d, "@charset \"",10) == 0)
    {
        // the string until "; is the charset name
        char *p = strchr(d+10, '"');
        if (p == 0) return m_charset;
        TQString charset = TQString::fromAscii(d+10, p-(d+10));
	return charset;
    }
    return m_charset;
}
#endif

// -------------------------------------------------------------------------------------------

CachedScript::CachedScript(DocLoader* dl, const DOMString &url, TDEIO::CacheControl _cachePolicy, const char*)
    : CachedObject(url, Script, _cachePolicy, 0)
{
    // It's javascript we want.
    // But some websites think their scripts are <some wrong mimetype here>
    // and refuse to serve them if we only accept application/x-javascript.
    setAccept( TQString::fromLatin1("*/*") );
    // load the file
    Cache::loader()->load(dl, this, false);
    m_loading = true;
}

CachedScript::CachedScript(const DOMString &url, const TQString &script_data)
    : CachedObject(url, Script, TDEIO::CC_Verify, script_data.length())
{
    m_loading = false;
    m_status = Persistent;
    m_script = DOMString(script_data);
}

void CachedScript::ref(CachedObjectClient *c)
{
    CachedObject::ref(c);

    if(!m_loading) c->notifyFinished(this);
}

void CachedScript::data( TQBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    setSize(buffer.buffer().size());

    TQTextCodec* c = codecForBuffer( m_charset, buffer.buffer() );
    TQString data = c->toUnicode( buffer.buffer().data(), m_size );
    m_script = static_cast<TQChar>(data[0]) == TQChar::byteOrderMark ? DOMString(data.mid( 1 ) ) : DOMString(data);
    m_loading = false;
    checkNotify();
}

void CachedScript::checkNotify()
{
    if(m_loading) return;

    for (TQPtrDictIterator<CachedObjectClient> it( m_clients); it.current();)
        it()->notifyFinished(this);
}

void CachedScript::error( int /*err*/, const char* /*text*/ )
{
    m_loading = false;
    checkNotify();
}

// ------------------------------------------------------------------------------------------

namespace tdehtml
{

class ImageSource : public TQDataSource
{
public:
    ImageSource(TQByteArray buf)
        : buffer( buf ), pos( 0 ), eof( false ), rew(false ), rewable( true )
        {}

    int readyToSend()
    {
       if(eof && pos == buffer.size())
           return -1;

        return  buffer.size() - pos;
    }

    void sendTo(TQDataSink* sink, int n)
    {
        sink->receive((const uchar*)&buffer.at(pos), n);

        pos += n;

        // buffer is no longer needed
        if(eof && pos == buffer.size() && !rewable)
        {
            buffer.resize(0);
            pos = 0;
        }
    }

    /**
     * Sets the EOF state.
     */
    void setEOF( bool state ) { eof = state; }

    bool rewindable() const { return rewable; }
    void enableRewind(bool on) { rew = on; }

    /*
      Calls reset() on the TQIODevice.
    */
    void rewind()
    {
        pos = 0;
        if (!rew) {
            TQDataSource::rewind();
        } else
            ready();
    }

    /*
      Indicates that the buffered data is no longer
      needed.
    */
    void cleanBuffer()
    {
        // if we need to be able to rewind, buffer is needed
        if(rew)
            return;

        rewable = false;

        // buffer is no longer needed
        if(eof && pos == buffer.size())
        {
            buffer.resize(0);
            pos = 0;
        }
    }

    TQByteArray buffer;
    unsigned int pos;
private:
    bool eof     : 1;
    bool rew     : 1;
    bool rewable : 1;
};

} // end namespace

static TQString buildAcceptHeader()
{
    return "image/png, image/jpeg, video/x-mng, image/jp2, image/gif;q=0.5,*/*;q=0.1";
}

// -------------------------------------------------------------------------------------

CachedImage::CachedImage(DocLoader* dl, const DOMString &url, TDEIO::CacheControl _cachePolicy, const char*)
    : TQObject(), CachedObject(url, Image, _cachePolicy, 0)
{
    static const TQString &acceptHeader = TDEGlobal::staticQString( buildAcceptHeader() );

    m = 0;
    p = 0;
    pixPart = 0;
    bg = 0;
    scaled = 0;
    bgColor = tqRgba( 0, 0, 0, 0xFF );
    typeChecked = false;
    isFullyTransparent = false;
    monochrome = false;
    formatType = 0;
    m_status = Unknown;
    imgSource = 0;
    setAccept( acceptHeader );
    m_showAnimations = dl->showAnimations();

    if ( TDEHTMLFactory::defaultHTMLSettings()->isAdFiltered( url.string() ) ) {
        m_wasBlocked = true;
        CachedObject::finish();
    }
}

CachedImage::~CachedImage()
{
    clear();
}

void CachedImage::ref( CachedObjectClient *c )
{
    CachedObject::ref(c);

    if( m ) {
        m->unpause();
        if( m->finished() || m_clients.count() == 1 )
            m->restart();
    }

    // for mouseovers, dynamic changes
    if ( m_status >= Persistent && !valid_rect().isNull() ) {
        c->setPixmap( pixmap(), valid_rect(), this);
        c->notifyFinished( this );
    }
}

void CachedImage::deref( CachedObjectClient *c )
{
    CachedObject::deref(c);
    if(m && m_clients.isEmpty() && m->running())
        m->pause();
}

#define BGMINWIDTH      32
#define BGMINHEIGHT     32

const TQPixmap &CachedImage::tiled_pixmap(const TQColor& newc, int xWidth, int xHeight)
{
    static TQRgb bgTransparent = tqRgba( 0, 0, 0, 0xFF );

    TQSize s(pixmap_size());
    int w = xWidth;
    int h = xHeight;
    if (w == -1) xWidth = w = s.width();
    if (h == -1) xHeight = h = s.height();

    if ( ( (bgColor != bgTransparent) && (bgColor != newc.rgb()) ) ||
         ( bgSize != TQSize(xWidth, xHeight)) )
    {
        delete bg; bg = 0;
    }

    if (bg)
        return *bg;

    const TQPixmap &r = pixmap();

    if (r.isNull()) return r;

    // no error indication for background images
    if(m_hadError||m_wasBlocked) return *Cache::nullPixmap;

    bool isvalid = newc.isValid();

    const TQPixmap* src; //source for pretiling, if any

    //See whether we should scale
    if (xWidth != s.width() || xHeight != s.height()) {
        src = &scaled_pixmap(xWidth, xHeight);
    } else {
        src = &r;
    }

    bgSize = TQSize(xWidth, xHeight);

    //See whether we can - and should - pre-blend
    if (isvalid && (r.hasAlphaChannel() || r.mask() )) {
        bg = new TQPixmap(xWidth, xHeight, r.depth());
        bg->fill(newc);
        bitBlt(bg, 0, 0, src);
        bgColor = newc.rgb();
        src     = bg;
    } else {
        bgColor = bgTransparent;
    }

    //See whether to pre-tile.
    if ( w*h < 8192 )
    {
        if ( r.width() < BGMINWIDTH )
            w = ((BGMINWIDTH-1) / xWidth + 1) * xWidth;
        if ( r.height() < BGMINHEIGHT )
            h = ((BGMINHEIGHT-1) / xHeight + 1) * xHeight;
    }
    if ( w != xWidth  || h != xHeight )
    {
//         kdDebug() << "pre-tiling " << s.width() << "," << s.height() << " to " << w << "," << h << endl;
        TQPixmap* oldbg = bg;
        bg = new TQPixmap(w, h, r.depth());

        //Tile horizontally on the first stripe
        for (int x = 0; x < w; x += xWidth)
            copyBlt(bg, x, 0, src, 0, 0, xWidth, xHeight);

        //Copy first stripe down
        for (int y = xHeight; y < h; y += xHeight)
            copyBlt(bg, 0, y, bg, 0, 0, w, xHeight);

        if ( src == oldbg )
            delete oldbg;
    }

    if (bg)
        return *bg;

    return *src;
}

const TQPixmap &CachedImage::scaled_pixmap( int xWidth, int xHeight )
{
    if (scaled) {
        if (scaled->width() == xWidth && scaled->height() == xHeight)
            return *scaled;
        delete scaled;
    }
    const TQPixmap &r = pixmap();
    if (r.isNull()) return r;

//     kdDebug() << "scaling " << r.width() << "," << r.height() << " to " << xWidth << "," << xHeight << endl;

    TQImage image = TQImage(r.convertToImage()).smoothScale(xWidth, xHeight);

    scaled = new TQPixmap(xWidth, xHeight, r.depth());
    scaled->convertFromImage(image);

    return *scaled;
}


const TQPixmap &CachedImage::pixmap( ) const
{
    if(m_hadError)
        return *Cache::brokenPixmap;

    if(m_wasBlocked)
        return *Cache::blockedPixmap;

    if(m)
    {
        if(m->framePixmap().size() != m->getValidRect().size())
        {
            // pixmap is not yet completely loaded, so we
            // return a clipped version. asserting here
            // that the valid rect is always from 0/0 to fullwidth/ someheight
            if(!pixPart) pixPart = new TQPixmap();

            (*pixPart) = m->framePixmap();
            if (m->getValidRect().size().isValid())
                pixPart->resize(m->getValidRect().size());
            else
                pixPart->resize(0, 0);
            return *pixPart;
        }
        else
            return m->framePixmap();
    }
    else if(p)
        return *p;

    return *Cache::nullPixmap;
}


TQSize CachedImage::pixmap_size() const
{
    if (m_wasBlocked) return Cache::blockedPixmap->size();
    return (m_hadError ? Cache::brokenPixmap->size() : m ? m->framePixmap().size() : ( p ? p->size() : TQSize()));
}


TQRect CachedImage::valid_rect() const
{
    if (m_wasBlocked) return Cache::blockedPixmap->rect();
    return (m_hadError ? Cache::brokenPixmap->rect() : m ? TQRect(m->getValidRect()) : ( p ? TQRect(p->rect()) : TQRect()) );
}


void CachedImage::do_notify(const TQPixmap& p, const TQRect& r)
{
    for (TQPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
        it()->setPixmap( p, r, this);
}


void CachedImage::movieUpdated( const TQRect& r )
{
#ifdef LOADER_DEBUG
    tqDebug("movie updated %d/%d/%d/%d, pixmap size %d/%d", r.x(), r.y(), r.right(), r.bottom(),
           m->framePixmap().size().width(), m->framePixmap().size().height());
#endif

    do_notify(m->framePixmap(), r);
}

void CachedImage::movieStatus(int status)
{
#ifdef LOADER_DEBUG
    tqDebug("movieStatus(%d)", status);
#endif

    // ### the html image objects are supposed to send the load event after every frame (according to
    // netscape). We have a problem though where an image is present, and js code creates a new Image object,
    // which uses the same CachedImage, the one in the document is not supposed to be notified

    // just another TQt 2.2.0 bug. we cannot call
    // TQMovie::frameImage if we're after TQMovie::EndOfMovie
    if(status == TQMovie::EndOfFrame)
    {
        const TQImage& im = m->frameImage();
        monochrome = ( ( im.depth() <= 8 ) && ( im.numColors() - int( im.hasAlphaBuffer() ) <= 2 ) );
        for (int i = 0; monochrome && i < im.numColors(); ++i)
            if (im.colorTable()[i] != tqRgb(0xff, 0xff, 0xff) &&
                im.colorTable()[i] != tqRgb(0x00, 0x00, 0x00))
                monochrome = false;
        if( (im.width() < 5 || im.height() < 5) && im.hasAlphaBuffer()) // only evaluate for small images
        {
            TQImage am = im.createAlphaMask();
            if(am.depth() == 1)
            {
                bool solid = false;
                for(int y = 0; y < am.height(); y++)
                    for(int x = 0; x < am.width(); x++)
                        if(am.pixelIndex(x, y)) {
                            solid = true;
                            break;
                        }
                isFullyTransparent = (!solid);
            }
        }

        // we have to delete our tiled bg variant here
        // because the frame has changed (in order to keep it in sync)
        delete bg;
        bg = 0;
    }

    if((status == TQMovie::EndOfMovie && (!m || m->frameNumber() <= 1)) ||
       ((status == TQMovie::EndOfLoop) && (m_showAnimations == TDEHTMLSettings::KAnimationLoopOnce)) ||
       ((status == TQMovie::EndOfFrame) && (m_showAnimations == TDEHTMLSettings::KAnimationDisabled))
      )
    {
        if(imgSource)
        {
            setShowAnimations( TDEHTMLSettings::KAnimationDisabled );

            // monochrome alphamasked images are usually about 10000 times
            // faster to draw, so this is worth the hack
            if (p && monochrome && p->depth() > 1)
            {
                TQPixmap* pix = new TQPixmap;
                pix->convertFromImage( TQImage(p->convertToImage()).convertDepth( 1 ), MonoOnly|AvoidDither );
                if ( p->mask() )
                    pix->setMask( *p->mask() );
                delete p;
                p = pix;
                monochrome = false;
            }
        }
        for (TQPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
            it()->notifyFinished( this );
	m_status = Cached; //all done
    }

#if 0
    if((status == TQMovie::EndOfFrame) || (status == TQMovie::EndOfMovie))
    {
#ifdef LOADER_DEBUG
        TQRect r(valid_rect());
        tqDebug("movie Status frame update %d/%d/%d/%d, pixmap size %d/%d", r.x(), r.y(), r.right(), r.bottom(),
               pixmap().size().width(), pixmap().size().height());
#endif
        do_notify(pixmap(), valid_rect());
    }
#endif
}

void CachedImage::movieResize(const TQSize& /*s*/)
{
    do_notify(m->framePixmap(), TQRect());
}

void CachedImage::setShowAnimations( TDEHTMLSettings::KAnimationAdvice showAnimations )
{
    m_showAnimations = showAnimations;
    if ( (m_showAnimations == TDEHTMLSettings::KAnimationDisabled) && imgSource ) {
        imgSource->cleanBuffer();
        delete p;
        p = new TQPixmap(m->framePixmap());
        m->disconnectUpdate( this, TQ_SLOT( movieUpdated( const TQRect &) ));
        m->disconnectStatus( this, TQ_SLOT( movieStatus( int ) ));
        m->disconnectResize( this, TQ_SLOT( movieResize( const TQSize& ) ) );
        TQTimer::singleShot(0, this, TQ_SLOT( deleteMovie()));
        imgSource = 0;
    }
}

void CachedImage::pauseAnimations()
{
    if ( m ) m->pause();
}

void CachedImage::resumeAnimations()
{
    if ( m ) m->unpause();
}


void CachedImage::deleteMovie()
{
    delete m; m = 0;
}

void CachedImage::clear()
{
    delete m;   m = 0;
    delete p;   p = 0;
    delete bg;  bg = 0;
    delete scaled;  scaled = 0;
    bgColor = tqRgba( 0, 0, 0, 0xff );
    bgSize = TQSize(-1,-1);
    delete pixPart; pixPart = 0;

    formatType = 0;
    typeChecked = false;
    setSize(0);

    // No need to delete imageSource - TQMovie does it for us
    imgSource = 0;
}

void CachedImage::data ( TQBuffer &_buffer, bool eof )
{
#ifdef LOADER_DEBUG
    kdDebug( 6060 ) << this << "in CachedImage::data(buffersize " << _buffer.buffer().size() <<", eof=" << eof << endl;
#endif
    if ( !typeChecked )
    {
        // don't attempt incremental loading if we have all the data already
        if (!eof)
        {
            formatType = TQImageDecoder::formatName( (const uchar*)_buffer.buffer().data(), _buffer.size());
            if ( formatType && strcmp( formatType, "PNG" ) == 0 )
                formatType = 0; // Some png files contain multiple images, we want to show only the first one
        }

        typeChecked = true;

        if ( formatType )  // movie format exists
        {
            imgSource = new ImageSource( _buffer.buffer());
            m = new TQMovie( imgSource, 8192 );
            m->connectUpdate( this, TQ_SLOT( movieUpdated( const TQRect &) ));
            m->connectStatus( this, TQ_SLOT( movieStatus(int)));
            m->connectResize( this, TQ_SLOT( movieResize( const TQSize& ) ) );
        }
    }

    if ( imgSource )
    {
        imgSource->setEOF(eof);
        imgSource->maybeReady();
    }

    if(eof)
    {
        // TQMovie currently doesn't support all kinds of image formats
        // so we need to use a TQPixmap here when we finished loading the complete
        // picture and display it then all at once.
        if(typeChecked && !formatType)
        {
#ifdef CACHE_DEBUG
            kdDebug(6060) << "CachedImage::data(): reloading as pixmap:" << endl;
#endif
            p = new TQPixmap;
            {
            	TQBuffer buffer(_buffer.buffer());
            	buffer.open(IO_ReadOnly);
                TQImageIO io( &buffer, 0 );
                io.setGamma(2.2); // hardcoded "reasonable value"
                bool result = io.read();
                if (result) p->convertFromImage(io.image(), 0);
            }

            // set size of image.
#ifdef CACHE_DEBUG
            kdDebug(6060) << "CachedImage::data(): image is null: " << p->isNull() << endl;
#endif
            if(p->isNull())
            {
                m_hadError = true;
                do_notify(pixmap(), TQRect(0, 0, 16, 16)); // load "broken image" icon
            }
            else
                do_notify(*p, p->rect());

            for (TQPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
                it()->notifyFinished( this );
            m_status = Cached; //all done
        }
    }
}

void CachedImage::finish()
{
    Status oldStatus = m_status;
    CachedObject::finish();
    if ( oldStatus != m_status ) {
	const TQPixmap &pm = pixmap();
	do_notify( pm, pm.rect() );
    }
    TQSize s = pixmap_size();
    setSize( s.width() * s.height() * 2);
}


void CachedImage::error( int /*err*/, const char* /*text*/ )
{
    clear();
    typeChecked = true;
    m_hadError = true;
    m_loading = false;
    do_notify(pixmap(), TQRect(0, 0, 16, 16));
    for (TQPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
        it()->notifyFinished(this);
}

// ------------------------------------------------------------------------------------------

Request::Request(DocLoader* dl, CachedObject *_object, bool _incremental)
{
    object = _object;
    object->setRequest(this);
    incremental = _incremental;
    m_docLoader = dl;
}

Request::~Request()
{
    object->setRequest(0);
}

// ------------------------------------------------------------------------------------------

DocLoader::DocLoader(TDEHTMLPart* part, DocumentImpl* doc)
{
    m_cachePolicy = TDEIO::CC_Verify;
    m_expireDate = 0;
    m_creationDate = time(0);
    m_bautoloadImages = true;
    m_showAnimations = TDEHTMLSettings::KAnimationEnabled;
    m_part = part;
    m_doc = doc;

    Cache::docloader->append( this );
}

DocLoader::~DocLoader()
{
    Cache::loader()->cancelRequests( this );
    Cache::docloader->remove( this );
}

void DocLoader::setCacheCreationDate(time_t _creationDate)
{
    if (_creationDate)
       m_creationDate = _creationDate;
    else
       m_creationDate = time(0); // Now
}

void DocLoader::setExpireDate(time_t _expireDate, bool relative)
{
    if (relative)
       m_expireDate = _expireDate + m_creationDate; // Relative date
    else
       m_expireDate = _expireDate; // Absolute date
#ifdef CACHE_DEBUG
    kdDebug(6061) << "docLoader: " << m_expireDate - time(0) << " seconds left until reload required.\n";
#endif
}

void DocLoader::insertCachedObject( CachedObject* o ) const
{
    if ( m_docObjects.find(o) )
        return;
    m_docObjects.insert( o, o );
    if ( m_docObjects.count() > 3 * m_docObjects.size() )
        m_docObjects.resize(tdehtml::nextSeed( m_docObjects.size() ) );
}

bool DocLoader::needReload(CachedObject *existing, const TQString& fullURL)
{
    bool reload = false;
    if (m_cachePolicy == TDEIO::CC_Verify)
    {
       if (!m_reloadedURLs.contains(fullURL))
       {
          if (existing && existing->isExpired())
          {
             Cache::removeCacheEntry(existing);
             m_reloadedURLs.append(fullURL);
             reload = true;
          }
       }
    }
    else if ((m_cachePolicy == TDEIO::CC_Reload) || (m_cachePolicy == TDEIO::CC_Refresh))
    {
       if (!m_reloadedURLs.contains(fullURL))
       {
          if (existing)
          {
             Cache::removeCacheEntry(existing);
          }
          m_reloadedURLs.append(fullURL);
          reload = true;
       }
    }
    return reload;
}

#define DOCLOADER_SECCHECK(doRedirectCheck) \
    KURL fullURL (m_doc->completeURL( url.string() )); \
    if ( !fullURL.isValid() || \
         ( m_part && m_part->onlyLocalReferences() && fullURL.protocol() != "file" && fullURL.protocol() != "data") || \
         doRedirectCheck && ( kapp && m_doc && !kapp->authorizeURLAction("redirect", m_doc->URL(), fullURL))) \
         return 0L;

CachedImage *DocLoader::requestImage( const DOM::DOMString &url)
{
    DOCLOADER_SECCHECK(true);

    CachedImage* i = Cache::requestObject<CachedImage, CachedObject::Image>( this, fullURL, 0);

    if (i && i->status() == CachedObject::Unknown && autoloadImages())
        Cache::loader()->load(this, i, true);

    return i;
}

CachedCSSStyleSheet *DocLoader::requestStyleSheet( const DOM::DOMString &url, const TQString& charset,
						   const char *accept, bool userSheet )
{
    DOCLOADER_SECCHECK(!userSheet);

    CachedCSSStyleSheet* s = Cache::requestObject<CachedCSSStyleSheet, CachedObject::CSSStyleSheet>( this, fullURL, accept );
    if ( s && !charset.isEmpty() ) {
        s->setCharsetHint( charset );
    }
    return s;
}

CachedScript *DocLoader::requestScript( const DOM::DOMString &url, const TQString& charset)
{
    DOCLOADER_SECCHECK(true);
    if ( ! TDEHTMLFactory::defaultHTMLSettings()->isJavaScriptEnabled(fullURL.host()) ||
           TDEHTMLFactory::defaultHTMLSettings()->isAdFiltered(fullURL.url()))
	return 0L;

    CachedScript* s = Cache::requestObject<CachedScript, CachedObject::Script>( this, fullURL, 0 );
    if ( s && !charset.isEmpty() )
        s->setCharset( charset );
    return s;
}

#undef DOCLOADER_SECCHECK

void DocLoader::setAutoloadImages( bool enable )
{
    if ( enable == m_bautoloadImages )
        return;

    m_bautoloadImages = enable;

    if ( !m_bautoloadImages ) return;

    for ( TQPtrDictIterator<CachedObject> it( m_docObjects ); it.current(); ++it )
        if ( it.current()->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( it.current()) );

            CachedObject::Status status = img->status();
            if ( status != CachedObject::Unknown )
                continue;

            Cache::loader()->load(this, img, true);
        }
}

void DocLoader::setShowAnimations( TDEHTMLSettings::KAnimationAdvice showAnimations )
{
    if ( showAnimations == m_showAnimations ) return;
    m_showAnimations = showAnimations;

    for ( TQPtrDictIterator<CachedObject> it( m_docObjects ); it.current(); ++it )
        if ( it.current()->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( it.current() ) );

            img->setShowAnimations( m_showAnimations );
        }
}

void DocLoader::pauseAnimations()
{
    for ( TQPtrDictIterator<CachedObject> it( m_docObjects ); it.current(); ++it )
        if ( it.current()->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( it.current() ) );

            img->pauseAnimations();
        }
}

void DocLoader::resumeAnimations()
{
    for ( TQPtrDictIterator<CachedObject> it( m_docObjects ); it.current(); ++it )
        if ( it.current()->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( it.current() ) );

            img->resumeAnimations();
        }
}

// ------------------------------------------------------------------------------------------

Loader::Loader() : TQObject()
{
    m_requestsPending.setAutoDelete( true );
    m_requestsLoading.setAutoDelete( true );
    connect(&m_timer, TQ_SIGNAL(timeout()), this, TQ_SLOT( servePendingRequests() ) );
}

void Loader::load(DocLoader* dl, CachedObject *object, bool incremental)
{
    Request *req = new Request(dl, object, incremental);
    m_requestsPending.append(req);

    emit requestStarted( req->m_docLoader, req->object );

    m_timer.start(0, true);
}

void Loader::servePendingRequests()
{
    while ( (m_requestsPending.count() != 0) && (m_requestsLoading.count() < MAX_JOB_COUNT) )
    {
        // get the first pending request
        Request *req = m_requestsPending.take(0);

#ifdef LOADER_DEBUG
  kdDebug( 6060 ) << "starting Loader url=" << req->object->url().string() << endl;
#endif

        KURL u(req->object->url().string());
        TDEIO::TransferJob* job = TDEIO::get( u, false, false /*no GUI*/);

        job->addMetaData("cache", TDEIO::getCacheControlString(req->object->cachePolicy()));
        if (!req->object->accept().isEmpty())
            job->addMetaData("accept", req->object->accept());
        if ( req->m_docLoader )
        {
            job->addMetaData( "referrer",  req->m_docLoader->doc()->URL().url() );

            TDEHTMLPart *part = req->m_docLoader->part();
            if (part )
            {
                job->addMetaData( "cross-domain", part->toplevelURL().url() );
                if (part->widget())
                    job->setWindow (part->widget()->topLevelWidget());
            }
        }

        connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ), this, TQ_SLOT( slotFinished( TDEIO::Job * ) ) );
        connect( job, TQ_SIGNAL( data( TDEIO::Job*, const TQByteArray &)),
                 TQ_SLOT( slotData( TDEIO::Job*, const TQByteArray &)));

        if ( req->object->schedule() )
            TDEIO::Scheduler::scheduleJob( job );

        m_requestsLoading.insert(job, req);
    }
}

void Loader::slotFinished( TDEIO::Job* job )
{
  Request *r = m_requestsLoading.take( job );
  TDEIO::TransferJob* j = static_cast<TDEIO::TransferJob*>(job);

  if ( !r )
    return;

  if (j->error() || j->isErrorPage())
  {
#ifdef LOADER_DEBUG
      kdDebug(6060) << "Loader::slotFinished, with error. job->error()= " << j->error() << " job->isErrorPage()=" << j->isErrorPage() << endl;
#endif
      r->object->error( job->error(), job->errorText().ascii() );
      emit requestFailed( r->m_docLoader, r->object );
  }
  else
  {
      TQString cs = j->queryMetaData("charset");
      if (!cs.isEmpty()) r->object->setCharset(cs);
      r->object->data(r->m_buffer, true);
      emit requestDone( r->m_docLoader, r->object );
      time_t expireDate = j->queryMetaData("expire-date").toLong();
#ifdef LOADER_DEBUG
      kdDebug(6060) << "Loader::slotFinished, url = " << j->url().url() << endl;
#endif
      r->object->setExpireDate( expireDate );

      if ( r->object->type() == CachedObject::Image ) {
          TQString fn = j->queryMetaData("content-disposition");
          static_cast<CachedImage*>( r->object )->setSuggestedFilename(fn);
#ifdef IMAGE_TITLES
          static_cast<CachedImage*>( r->object )->setSuggestedTitle(fn);
          KTempFile tf;
          tf.setAutoDelete(true);
          tf.file()->writeBlock((const char*)r->m_buffer.buffer().data(), r->m_buffer.size());
          tf.sync();
          KFileMetaInfo kfmi(tf.name());
          if (!kfmi.isEmpty()) {
              KFileMetaInfoItem i = kfmi.item("Name");
              if (i.isValid()) {
                  static_cast<CachedImage*>(r->object)->setSuggestedTitle(i.string());
              } else {
                  i = kfmi.item("Title");
                  if (i.isValid()) {
                      static_cast<CachedImage*>(r->object)->setSuggestedTitle(i.string());
                  }
              }
          }
#endif
      }
  }

  r->object->finish();

#ifdef LOADER_DEBUG
  kdDebug( 6060 ) << "Loader:: JOB FINISHED " << r->object << ": " << r->object->url().string() << endl;
#endif

  delete r;

  if ( (m_requestsPending.count() != 0) && (m_requestsLoading.count() < MAX_JOB_COUNT / 2) )
      m_timer.start(0, true);
}

void Loader::slotData( TDEIO::Job*job, const TQByteArray &data )
{
    Request *r = m_requestsLoading[job];
    if(!r) {
        kdDebug( 6060 ) << "got data for unknown request!" << endl;
        return;
    }

    if ( !r->m_buffer.isOpen() )
        r->m_buffer.open( IO_WriteOnly );

    r->m_buffer.writeBlock( data.data(), data.size() );

    if(r->incremental)
        r->object->data( r->m_buffer, false );
}

int Loader::numRequests( DocLoader* dl ) const
{
    int res = 0;

    TQPtrListIterator<Request> pIt( m_requestsPending );
    for (; pIt.current(); ++pIt )
        if ( pIt.current()->m_docLoader == dl )
            res++;

    TQPtrDictIterator<Request> lIt( m_requestsLoading );
    for (; lIt.current(); ++lIt )
        if ( lIt.current()->m_docLoader == dl )
            res++;

    return res;
}

void Loader::cancelRequests( DocLoader* dl )
{
    TQPtrListIterator<Request> pIt( m_requestsPending );
    while ( pIt.current() ) {
        if ( pIt.current()->m_docLoader == dl )
        {
            CDEBUG << "canceling pending request for " << pIt.current()->object->url().string() << endl;
            Cache::removeCacheEntry( pIt.current()->object );
            m_requestsPending.remove( pIt );
        }
        else
            ++pIt;
    }

    //kdDebug( 6060 ) << "got " << m_requestsLoading.count() << "loading requests" << endl;

    TQPtrDictIterator<Request> lIt( m_requestsLoading );
    while ( lIt.current() )
    {
        if ( lIt.current()->m_docLoader == dl )
        {
            //kdDebug( 6060 ) << "canceling loading request for " << lIt.current()->object->url().string() << endl;
            TDEIO::Job *job = static_cast<TDEIO::Job *>( lIt.currentKey() );
            Cache::removeCacheEntry( lIt.current()->object );
            m_requestsLoading.remove( lIt.currentKey() );
            job->kill();
            //emit requestFailed( dl, pIt.current()->object );
        }
        else
            ++lIt;
    }
}

TDEIO::Job *Loader::jobForRequest( const DOM::DOMString &url ) const
{
    TQPtrDictIterator<Request> it( m_requestsLoading );

    for (; it.current(); ++it )
    {
        CachedObject *obj = it.current()->object;

        if ( obj && obj->url() == url )
            return static_cast<TDEIO::Job *>( it.currentKey() );
    }

    return 0;
}

// ----------------------------------------------------------------------------


TQDict<CachedObject> *Cache::cache = 0;
TQPtrList<DocLoader>* Cache::docloader = 0;
TQPtrList<CachedObject> *Cache::freeList = 0;
Loader *Cache::m_loader = 0;

int Cache::maxSize = DEFCACHESIZE;
int Cache::totalSizeOfLRU;

TQPixmap *Cache::nullPixmap = 0;
TQPixmap *Cache::brokenPixmap = 0;
TQPixmap *Cache::blockedPixmap = 0;

void Cache::init()
{
    if ( !cache )
        cache = new TQDict<CachedObject>(401, true);

    if ( !docloader )
        docloader = new TQPtrList<DocLoader>;

    if ( !nullPixmap )
        nullPixmap = new TQPixmap;

    if ( !brokenPixmap )
        brokenPixmap = new TQPixmap(TDEHTMLFactory::instance()->iconLoader()->loadIcon("file_broken", TDEIcon::Desktop, 16, TDEIcon::DisabledState));

    if ( !blockedPixmap ) {
        blockedPixmap = new TQPixmap();
        blockedPixmap->loadFromData(blocked_icon_data, blocked_icon_len);
    }

    if ( !m_loader )
        m_loader = new Loader();

    if ( !freeList ) {
        freeList = new TQPtrList<CachedObject>;
        freeList->setAutoDelete(true);
    }
}

void Cache::clear()
{
    if ( !cache ) return;
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "Cache: CLEAR!" << endl;
    statistics();
#endif
    cache->setAutoDelete( true );

#ifndef NDEBUG
    bool crash = false;
    for (TQDictIterator<CachedObject> it(*cache); it.current(); ++it) {
        if (!it.current()->canDelete()) {
            kdDebug( 6060 ) << " Object in cache still linked to" << endl;
            kdDebug( 6060 ) << " -> URL: " << it.current()->url() << endl;
            kdDebug( 6060 ) << " -> #clients: " << it.current()->count() << endl;
            crash = true;
//         assert(it.current()->canDelete());
        }
    }
    for (freeList->first(); freeList->current(); freeList->next()) {
        if (!freeList->current()->canDelete()) {
            kdDebug( 6060 ) << " Object in freelist still linked to" << endl;
            kdDebug( 6060 ) << " -> URL: " << freeList->current()->url() << endl;
            kdDebug( 6060 ) << " -> #clients: " << freeList->current()->count() << endl;
            crash = true;
            /*
            TQPtrDictIterator<CachedObjectClient> it(freeList->current()->m_clients);
            for(;it.current(); ++it) {
                if (dynamic_cast<RenderObject*>(it.current())) {
                    kdDebug( 6060 ) << " --> RenderObject" << endl;
                } else
                    kdDebug( 6060 ) << " --> Something else" << endl;
            }*/
        }
//         assert(freeList->current()->canDelete());
    }
    assert(!crash);
#endif

    delete cache; cache = 0;
    delete nullPixmap; nullPixmap = 0;
    delete brokenPixmap; brokenPixmap = 0;
    delete blockedPixmap; blockedPixmap = 0;
    delete m_loader;  m_loader = 0;
    delete docloader; docloader = 0;
    delete freeList; freeList = 0;
}

template<typename CachedObjectType, enum CachedObject::Type CachedType>
CachedObjectType* Cache::requestObject( DocLoader* dl, const KURL& kurl, const char* accept )
{
    TDEIO::CacheControl cachePolicy = dl ? dl->cachePolicy() : TDEIO::CC_Verify;

    TQString url = kurl.url();
    CachedObject* o = cache->find(url);

    if ( o && o->type() != CachedType ) {
        removeCacheEntry( o );
        o = 0;
    }

    if ( o && dl->needReload( o, url ) ) {
        o = 0;
        assert( cache->find( url ) == 0 );
    }

    if(!o)
    {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "Cache: new: " << kurl.url() << endl;
#endif
        CachedObjectType* cot = new CachedObjectType(dl, url, cachePolicy, accept);
        cache->insert( url, cot );
        if ( cot->allowInLRUList() )
            insertInLRUList( cot );
        o = cot;
    }
#ifdef CACHE_DEBUG
    else {
    kdDebug( 6060 ) << "Cache: using pending/cached: " << kurl.url() << endl;
    }
#endif


    dl->insertCachedObject( o );

    return static_cast<CachedObjectType *>(o);
}

void Cache::preloadStyleSheet( const TQString &url, const TQString &stylesheet_data)
{
    CachedObject *o = cache->find(url);
    if(o)
        removeCacheEntry(o);

    CachedCSSStyleSheet *stylesheet = new CachedCSSStyleSheet(url, stylesheet_data);
    cache->insert( url, stylesheet );
}

void Cache::preloadScript( const TQString &url, const TQString &script_data)
{
    CachedObject *o = cache->find(url);
    if(o)
        removeCacheEntry(o);

    CachedScript *script = new CachedScript(url, script_data);
    cache->insert( url, script );
}

void Cache::flush(bool force)
{
    init();

    if ( force || totalSizeOfLRU > maxSize + maxSize/4) {
        for ( int i = MAX_LRU_LISTS-1; i >= 0 && totalSizeOfLRU > maxSize; --i )
            while ( totalSizeOfLRU > maxSize && m_LRULists[i].m_tail )
                removeCacheEntry( m_LRULists[i].m_tail );

#ifdef CACHE_DEBUG
        statistics();
#endif
    }

    for ( freeList->first(); freeList->current(); ) {
        CachedObject* p = freeList->current();
        if ( p->canDelete() )
            freeList->remove();
        else
            freeList->next();
    }

}

void Cache::setSize( int bytes )
{
    maxSize = bytes;
    flush(true /* force */);
}

void Cache::statistics()
{
    CachedObject *o;
    // this function is for debugging purposes only
    init();

    int size = 0;
    int msize = 0;
    int movie = 0;
    int images = 0;
    int scripts = 0;
    int stylesheets = 0;
    TQDictIterator<CachedObject> it(*cache);
    for(it.toFirst(); it.current(); ++it)
    {
        o = it.current();
        switch(o->type()) {
        case CachedObject::Image:
        {
            CachedImage *im = static_cast<CachedImage *>(o);
            images++;
            if(im->m != 0)
            {
                movie++;
                msize += im->size();
            }
            break;
        }
        case CachedObject::CSSStyleSheet:
            stylesheets++;
            break;
        case CachedObject::Script:
            scripts++;
            break;
        }
        size += o->size();
    }
    size /= 1024;

    kdDebug( 6060 ) << "------------------------- image cache statistics -------------------" << endl;
    kdDebug( 6060 ) << "Number of items in cache: " << cache->count() << endl;
    kdDebug( 6060 ) << "Number of cached images: " << images << endl;
    kdDebug( 6060 ) << "Number of cached movies: " << movie << endl;
    kdDebug( 6060 ) << "Number of cached scripts: " << scripts << endl;
    kdDebug( 6060 ) << "Number of cached stylesheets: " << stylesheets << endl;
    kdDebug( 6060 ) << "pixmaps:   allocated space approx. " << size << " kB" << endl;
    kdDebug( 6060 ) << "movies :   allocated space approx. " << msize/1024 << " kB" << endl;
    kdDebug( 6060 ) << "--------------------------------------------------------------------" << endl;
}

void Cache::removeCacheEntry( CachedObject *object )
{
    TQString key = object->url().string();

    cache->remove( key );
    removeFromLRUList( object );

    for (const DocLoader* dl=docloader->first(); dl; dl=docloader->next() )
        dl->removeCachedObject( object );

    if ( !object->free() ) {
        Cache::freeList->append( object );
        object->m_free = true;
    }
}

static inline int FastLog2(unsigned int j)
{
   unsigned int log2;
   log2 = 0;
   if (j & (j-1))
       log2 += 1;
   if (j >> 16)
       log2 += 16, j >>= 16;
   if (j >> 8)
       log2 += 8, j >>= 8;
   if (j >> 4)
       log2 += 4, j >>= 4;
   if (j >> 2)
       log2 += 2, j >>= 2;
   if (j >> 1)
       log2 += 1;

   return log2;
}

static LRUList* getLRUListFor(CachedObject* o)
{
    int accessCount = o->accessCount();
    int queueIndex;
    if (accessCount == 0) {
        queueIndex = 0;
   } else {
        int sizeLog = FastLog2(o->size());
        queueIndex = sizeLog/o->accessCount() - 1;
        if (queueIndex < 0)
            queueIndex = 0;
        if (queueIndex >= MAX_LRU_LISTS)
            queueIndex = MAX_LRU_LISTS-1;
    }
   return &m_LRULists[queueIndex];
}

void Cache::removeFromLRUList(CachedObject *object)
{
    CachedObject *next = object->m_next;
    CachedObject *prev = object->m_prev;

    LRUList* list = getLRUListFor(object);
    CachedObject *&head = getLRUListFor(object)->m_head;

    if (next == 0 && prev == 0 && head != object) {
        return;
    }

    object->m_next = 0;
    object->m_prev = 0;

    if (next)
        next->m_prev = prev;
    else if (list->m_tail == object)
       list->m_tail = prev;

    if (prev)
        prev->m_next = next;
    else if (head == object)
        head = next;

    totalSizeOfLRU -= object->size();
}

void Cache::insertInLRUList(CachedObject *object)
{
    removeFromLRUList(object);

    assert( object );
    assert( !object->free() );
    assert( object->canDelete() );
    assert( object->allowInLRUList() );

    LRUList* list = getLRUListFor(object);

    CachedObject *&head = list->m_head;

    object->m_next = head;
    if (head)
        head->m_prev = object;
    head = object;

    if (object->m_next == 0)
        list->m_tail = object;

    totalSizeOfLRU += object->size();
}

// --------------------------------------

void CachedObjectClient::setPixmap(const TQPixmap &, const TQRect&, CachedImage *) {}
void CachedObjectClient::setStyleSheet(const DOM::DOMString &/*url*/, const DOM::DOMString &/*sheet*/, const DOM::DOMString &/*charset*/) {}
void CachedObjectClient::notifyFinished(CachedObject * /*finishedObj*/) {}
void CachedObjectClient::error(int /*err*/, const TQString &/*text*/) {}

#undef CDEBUG

#include "loader.moc"
