/*
 *
 *
 * This file is part of the KDE project, module tdeui.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>.
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 *
 * kpixmapio.cpp: Fast pixmap <-> image conversion.
 */

#include "kpixmapio.h"
#include "config.h"

#include <tqimage.h>
#include <tqpixmap.h>
#include <tqcolor.h>
#include <tqglobal.h>

#include <tdeglobal.h>
#include <tdeconfig.h>
#include <kdebug.h>

#include <sys/types.h>
#ifdef Q_OS_UNIX
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#ifdef TQ_WS_X11
#include <X11/X.h> 
#include <X11/Xlib.h> 
#include <X11/Xutil.h> 
#ifdef HAVE_MITSHM
#include <X11/extensions/XShm.h> 
#endif
#ifdef __osf__
extern "C" int XShmQueryExtension(Display *display);
#endif
#else
#undef HAVE_MITSHM
#endif

// d pointer

struct KPixmapIOPrivate
{
    int shmsize;
    int shmpolicy;
    int threshold;
    int bpp;
    int byteorder;
#ifdef TQ_WS_X11
    XImage *ximage;
#ifdef HAVE_MITSHM
    XShmSegmentInfo *shminfo;
    bool first_try;
#endif
#else
    void *ximage;
#endif
};


//  From Qt: Returns the position of the lowest set bit in val.

typedef unsigned char uchar;
typedef unsigned int uint;

#ifdef HAVE_MITSHM
static int lowest_bit(uint val)
{
    int i;
    uint test = 1;
    for (i=0; (!(val & test)) && i<32; i++, test<<=1);
    return (i == 32) ? -1 : i;
}
#endif

/*** KPixmapIO ***/

KPixmapIO::KPixmapIO()
{
    m_bShm = false;
    d = new KPixmapIOPrivate;

#ifdef HAVE_MITSHM
    setShmPolicy(ShmDontKeep);
    TDEConfig *config = TDEGlobal::config();
    if (!config->readBoolEntry("UseMitShm", true))
	return;

    int ignore;
    if (XQueryExtension(tqt_xdisplay(), "MIT-SHM", &ignore, &ignore, &ignore))
    {
	if (XShmQueryExtension(tqt_xdisplay()))
	    m_bShm = true;
    }
    if (!m_bShm)
    {
	kdDebug(290) << k_lineinfo << "MIT-SHM not available!\n";
        d->ximage = 0;
	d->shminfo = 0;
	d->shmsize = 0;
	return;
    }

    // Sort out bit format. Create a temporary XImage for this.
    d->shminfo = new XShmSegmentInfo;
    d->ximage = XShmCreateImage(tqt_xdisplay(), (Visual *) TQPaintDevice::x11AppVisual(),
	    TQPaintDevice::x11AppDepth(), ZPixmap, 0L, d->shminfo, 10, 10);
    d->bpp = d->ximage->bits_per_pixel;
    d->first_try = true;
    int bpp = d->bpp;
    if (d->ximage->byte_order == LSBFirst)
	bpp++;
    int red_shift = lowest_bit(d->ximage->red_mask);
    int green_shift = lowest_bit(d->ximage->green_mask);
    int blue_shift = lowest_bit(d->ximage->blue_mask);
    XDestroyImage(d->ximage); d->ximage = 0L;
    d->shmsize = 0;

    // Offer discrete possibilities for the bitformat. Each will have its
    // own routine. The general algorithm using bitshifts is much too slow;
    // this has to be done for every pixel!

    if ((bpp == 32) && (red_shift == 16) && (green_shift == 8) &&
	    (blue_shift == 0))
	d->byteorder = bo32_ARGB;
    else if ((bpp == 32) && (red_shift == 0) && (green_shift == 8) &&
           (blue_shift == 16))
       d->byteorder = bo32_BGRA;
    else if ((bpp == 33) && (red_shift == 16) && (green_shift == 8) &&
	    (blue_shift == 0))
	d->byteorder = bo32_BGRA;
    else if ((bpp == 24) && (red_shift == 16) && (green_shift == 8) &&
	    (blue_shift == 0))
	d->byteorder = bo24_RGB;
    else if ((bpp == 24) && (red_shift == 0) && (green_shift == 8) &&
           (blue_shift == 16))
       d->byteorder = bo24_BGR;
    else if ((bpp == 25) && (red_shift == 16) && (green_shift == 8) &&
	    (blue_shift == 0))
	d->byteorder = bo24_BGR;
    else if ((bpp == 16) && (red_shift == 11) && (green_shift == 5) &&
	    (blue_shift == 0))
	d->byteorder = bo16_RGB_565;
    else if ((bpp == 16) && (red_shift == 10) && (green_shift == 5) &&
	    (blue_shift == 0))
	d->byteorder = bo16_RGB_555;
    else if ((bpp == 17) && (red_shift == 11) && (green_shift == 5) &&
	    (blue_shift == 0))
	d->byteorder = bo16_BGR_565;
    else if ((bpp == 17) && (red_shift == 10) && (green_shift == 5) &&
	    (blue_shift == 0))
	d->byteorder = bo16_BGR_555;
    else if ((bpp == 8) || (bpp == 9))
	d->byteorder = bo8;
    else
    {
	m_bShm = false;
	kdWarning(290) << "Byte order not supported!" << endl;
	kdWarning(290) << "red = " << red_shift
		<< ", green = " << green_shift
		<< ", blue = " << blue_shift << endl;
	kdWarning(290) << "Please report to <jansen@kde.org>\n";
    }
#else
    d->shmsize = 0;
    d->ximage = 0;
#endif
}


KPixmapIO::~KPixmapIO()
{
    destroyXImage();
    destroyShmSegment();
#ifdef HAVE_MITSHM
    delete d->shminfo;
#endif
    delete d;
}


TQPixmap KPixmapIO::convertToPixmap(const TQImage &img)
{
    int size = img.width() * img.height();
    if (m_bShm && (img.depth() > 1) && (d->bpp > 8) && (size > d->threshold))
    {
	TQPixmap dst(img.width(), img.height());
	putImage(&dst, 0, 0, &img);
	return dst;
    } else
    {
	TQPixmap dst;
	dst.convertFromImage(img);
	return dst;
    }

}


TQImage KPixmapIO::convertToImage(const TQPixmap &pm)
{
    TQImage image;
    int size = pm.width() * pm.height();
    if (m_bShm && (d->bpp >= 8) && (size > d->threshold))
	image = getImage(&pm, 0, 0, pm.width(), pm.height());
    else
	image = pm.convertToImage();
    return image;
}


void KPixmapIO::putImage(TQPixmap *dst, const TQPoint &offset,
    const TQImage *src)
{
    putImage(dst, offset.x(), offset.y(), src);
}


void KPixmapIO::putImage(TQPixmap *dst, int dx, int dy, const TQImage *src)
{
    int size = src->width() * src->height();
    bool fallback = true;
    if (m_bShm && (src->depth() > 1) && (d->bpp > 8) && (size > d->threshold))
    {
#ifdef HAVE_MITSHM
	if( initXImage(src->width(), src->height()))
	{
	    convertToXImage(*src);
	    XShmPutImage(tqt_xdisplay(), dst->handle(), tqt_xget_temp_gc(tqt_xscreen(), false), d->ximage,
		    dx, dy, 0, 0, src->width(), src->height(), false);
            // coolo: do we really need this here? I see no good for it
	    XSync(tqt_xdisplay(), false);
	    doneXImage();
	    fallback  = false;
	}
#endif
    }
    if( fallback )
    {
	TQPixmap pix;
	pix.convertFromImage(*src);
	bitBlt(dst, dx, dy, &pix, 0, 0, pix.width(), pix.height());
    }
}


TQImage KPixmapIO::getImage(const TQPixmap *src, const TQRect &rect)
{
    return getImage(src, rect.x(), rect.y(), rect.width(), rect.height());
}


TQImage KPixmapIO::getImage(const TQPixmap *src, int sx, int sy, int sw, int sh)
{
    TQImage image;
    int size = src->width() * src->height();
    bool fallback = true;
    if ((m_bShm) && (d->bpp >= 8) && (size > d->threshold))
    {
#ifdef HAVE_MITSHM
	if( initXImage(sw, sh))
	{
	    XShmGetImage(tqt_xdisplay(), src->handle(), d->ximage, sx, sy, AllPlanes);
	    image = convertFromXImage();
	    doneXImage();
	    fallback = false;
	}
#endif
    }
    if( fallback )
    {
	TQPixmap pix(sw, sh);
	bitBlt(&pix, 0, 0, src, sx, sy, sw, sh);
	image = pix.convertToImage();
    }
    return image;
}


#ifdef HAVE_MITSHM

void KPixmapIO::preAllocShm(int size)
{
    destroyXImage();
    createShmSegment(size);
}


void KPixmapIO::setShmPolicy(int policy)
{
    switch (policy)
    {
    case ShmDontKeep:
	d->shmpolicy = ShmDontKeep;
	d->threshold = 5000;
	break;
    case ShmKeepAndGrow:
	d->shmpolicy = ShmKeepAndGrow;
	d->threshold = 2000;
	break;
    default:
	break;
    }
}


bool KPixmapIO::initXImage(int w, int h)
{
    if (d->ximage && (w == d->ximage->width) && (h == d->ximage->height))
	return true;

    if( !createXImage(w, h))
	return false;
    int size = d->ximage->bytes_per_line * d->ximage->height;
    if (size > d->shmsize)
    {
	if( !createShmSegment(size))
	{
	    destroyXImage();
	    return false;
	}
    }
    d->ximage->data = d->shminfo->shmaddr;
    return true;
}


void KPixmapIO::doneXImage()
{
    if (d->shmpolicy == ShmDontKeep)
    {
	destroyXImage();
	destroyShmSegment();
    }
}


void KPixmapIO::destroyXImage()
{
    if (d->ximage)
    {
	XDestroyImage(d->ximage);
	d->ximage = 0L;
    }
}


bool KPixmapIO::createXImage(int w, int h)
{
    destroyXImage();
    d->ximage = XShmCreateImage(tqt_xdisplay(), (Visual *) TQPaintDevice::x11AppVisual(),
	    TQPaintDevice::x11AppDepth(), ZPixmap, 0L, d->shminfo, w, h);
    return d->ximage != None;
}


void KPixmapIO::destroyShmSegment()
{
    if (d->shmsize)
    {
	XShmDetach(tqt_xdisplay(), d->shminfo);
	shmdt(d->shminfo->shmaddr);
        shmctl(d->shminfo->shmid, IPC_RMID, 0);
	d->shmsize = 0;
    }
}

static bool use_xshm = true;
static unsigned long kpixmapio_serial;
static int (*old_errhandler)(Display *dpy, XErrorEvent *ev) = 0;

static int kpixmapio_errorhandler(Display *dpy, XErrorEvent *ev)
{
    if(ev->serial == kpixmapio_serial) {
        /* assuming that xshm errors mean it can't be used at all
           (e.g. remote display) */
        use_xshm = false;
        kdDebug(290) << "Disabling Xshm" << endl;
        return 0;
    } else {
        // another error
        return old_errhandler(dpy, ev);
    }
}

bool KPixmapIO::createShmSegment(int size)
{
    destroyShmSegment();
    d->shminfo->shmid = shmget(IPC_PRIVATE, size, IPC_CREAT|0600);
    if ( d->shminfo->shmid < 0)
    {
	kdWarning(290) << "Could not get shared memory segment.\n";
	m_bShm = false;
	return false;
    }

    d->shminfo->shmaddr = (char *) shmat(d->shminfo->shmid, 0, 0);
    if (d->shminfo->shmaddr == (char *)-1)
    {
	kdWarning(290) << "Could not attach shared memory segment.\n";
	m_bShm = false;
	shmctl(d->shminfo->shmid, IPC_RMID, 0);
	return false;
    }

    d->shminfo->readOnly = false;

    if (d->first_try) {
        // make sure that we don't get errors of old stuff
        XSync(tqt_xdisplay(), False);
        old_errhandler = XSetErrorHandler(kpixmapio_errorhandler);
        kpixmapio_serial = NextRequest(tqt_xdisplay());
    }

    if ( !XShmAttach(tqt_xdisplay(), d->shminfo))
    {
	kdWarning() << "X-Server could not attach shared memory segment.\n";
	m_bShm = false;
	shmdt(d->shminfo->shmaddr);
	shmctl(d->shminfo->shmid, IPC_RMID, 0);
    }

    if (d->first_try) {
        XSync(tqt_xdisplay(), false);

        if (!use_xshm)
            m_bShm = false;

        XSetErrorHandler(old_errhandler);
        d->first_try = false;
    }
    d->shmsize = size;

    return m_bShm;
}


/*
 * The following functions convertToXImage/convertFromXImage are a little
 * long. This is because of speed, I want to get as much out of the inner
 * loop as possible.
 */

TQImage KPixmapIO::convertFromXImage()
{
    int x, y;
    int width = d->ximage->width, height = d->ximage->height;
    int bpl = d->ximage->bytes_per_line;
    char *data = d->ximage->data;

    TQImage image;
    if (d->bpp == 8)
    {
	image.create(width, height, 8);

	// Query color map. Don't remove unused entries as a speed
	// optmization.
	int i, ncells = 256;
	XColor *cmap = new XColor[ncells];
	for (i=0; i<ncells; i++)
	    cmap[i].pixel = i;
	XQueryColors(tqt_xdisplay(), TQPaintDevice::x11AppColormap(),
		cmap, ncells);
	image.setNumColors(ncells);
	for (i=0; i<ncells; i++)
	    image.setColor(i, tqRgb(cmap[i].red, cmap[i].green, cmap[i].blue >> 8));
    } else
	image.create(width, height, 32);

    switch (d->byteorder)
    {

    case bo8:
    {
	for (y=0; y<height; y++)
	    memcpy(image.scanLine(y), data + y*bpl, width);
	break;
    }

    case bo16_RGB_565:
    case bo16_BGR_565:
    {
	TQ_INT32 pixel, *src;
	TQRgb *dst, val;
	for (y=0; y<height; y++)
	{
	    src = (TQ_INT32 *) (data + y*bpl);
	    dst = (TQRgb *) image.scanLine(y);
	    for (x=0; x<width/2; x++)
	    {
		pixel = *src++;
		val = ((pixel & 0xf800) << 8) | ((pixel & 0x7e0) << 5) |
			((pixel & 0x1f) << 3);
		*dst++ = val;
		pixel >>= 16;
		val = ((pixel & 0xf800) << 8) | ((pixel & 0x7e0) << 5) |
			((pixel & 0x1f) << 3);
		*dst++ = val;
	    }
	    if (width%2)
	    {
		pixel = *src++;
		val = ((pixel & 0xf800) << 8) | ((pixel & 0x7e0) << 5) |
			((pixel & 0x1f) << 3);
		*dst++ = val;
	    }
	}
	break;
    }

    case bo16_RGB_555:
    case bo16_BGR_555:
    {
	TQ_INT32 pixel, *src;
	TQRgb *dst, val;
	for (y=0; y<height; y++)
	{
	    src = (TQ_INT32 *) (data + y*bpl);
	    dst = (TQRgb *) image.scanLine(y);
	    for (x=0; x<width/2; x++)
	    {
		pixel = *src++;
		val = ((pixel & 0x7c00) << 9) | ((pixel & 0x3e0) << 6) |
			((pixel & 0x1f) << 3);
		*dst++ = val;
		pixel >>= 16;
		val = ((pixel & 0x7c00) << 9) | ((pixel & 0x3e0) << 6) |
			((pixel & 0x1f) << 3);
		*dst++ = val;
	    }
	    if (width%2)
	    {
		pixel = *src++;
		val = ((pixel & 0x7c00) << 9) | ((pixel & 0x3e0) << 6) |
			((pixel & 0x1f) << 3);
		*dst++ = val;
	    }
	}
	break;
    }

    case bo24_RGB:
    {
	char *src;
	TQRgb *dst;
	int w1 = width/4;
	TQ_INT32 d1, d2, d3;
	for (y=0; y<height; y++)
	{
	    src = data + y*bpl;
	    dst = (TQRgb *) image.scanLine(y);
	    for (x=0; x<w1; x++)
	    {
		d1 = *((TQ_INT32 *)src);
		d2 = *((TQ_INT32 *)src + 1);
		d3 = *((TQ_INT32 *)src + 2);
		src += 12;
		*dst++ = d1;
		*dst++ = (d1 >> 24) | (d2 << 8);
		*dst++ = (d3 << 16) | (d2 >> 16);
		*dst++ = d3 >> 8;
	    }
	    for (x=w1*4; x<width; x++)
	    {
		d1 = *src++ << 16;
		d1 += *src++ << 8;
		d1 += *src++;
		*dst++ = d1;
	    }
	}
	break;
    }

    case bo24_BGR:
    {
	char *src;
	TQRgb *dst;
	int w1 = width/4;
	TQ_INT32 d1, d2, d3;
	for (y=0; y<height; y++)
	{
	    src = data + y*bpl;
	    dst = (TQRgb *) image.scanLine(y);
	    for (x=0; x<w1; x++)
	    {
		d1 = *((TQ_INT32 *)src);
		d2 = *((TQ_INT32 *)src + 1);
		d3 = *((TQ_INT32 *)src + 2);
		src += 12;
		*dst++ = d1;
		*dst++ = (d1 >> 24) | (d2 << 8);
		*dst++ = (d3 << 16) | (d2 >> 16);
		*dst++ = d3 >> 8;
	    }
	    for (x=w1*4; x<width; x++)
	    {
		d1 = *src++;
		d1 += *src++ << 8;
		d1 += *src++ << 16;
		*dst++ = d1;
	    }
	}
	break;
    }

    case bo32_ARGB:
    case bo32_BGRA:
    {
	for (y=0; y<height; y++)
	    memcpy(image.scanLine(y), data + y*bpl, width*4);
	break;
    }

    }

    return image;
}


void KPixmapIO::convertToXImage(const TQImage &img)
{
    int x, y;
    int width = d->ximage->width, height = d->ximage->height;
    int bpl = d->ximage->bytes_per_line;
    char *data = d->ximage->data;

    switch (d->byteorder)
    {

    case bo16_RGB_555:
    case bo16_BGR_555:

	if (img.depth() == 32)
	{
	    TQRgb *src, pixel;
	    TQ_INT32 *dst, val;
	    for (y=0; y<height; y++)
	    {
		src = (TQRgb *) img.scanLine(y);
		dst = (TQ_INT32 *) (data + y*bpl);
		for (x=0; x<width/2; x++)
		{
		    pixel = *src++;
		    val = ((pixel & 0xf80000) >> 9) | ((pixel & 0xf800) >> 6) |
			     ((pixel & 0xff) >> 3);
		    pixel = *src++;
		    val |= (((pixel & 0xf80000) >> 9) | ((pixel & 0xf800) >> 6) |
			    ((pixel & 0xff) >> 3)) << 16;
		    *dst++ = val;
		}
		if (width%2)
		{
		    pixel = *src++;
		    *((TQ_INT16 *)dst) = ((pixel & 0xf80000) >> 9) |
			    ((pixel & 0xf800) >> 6) | ((pixel & 0xff) >> 3);
		}
	    }
	} else
	{
	    uchar *src;
	    TQ_INT32 val, *dst;
	    TQRgb pixel, *clut = img.colorTable();
	    for (y=0; y<height; y++)
	    {
		src = const_cast<TQImage&>(img).scanLine(y);
		dst = (TQ_INT32 *) (data + y*bpl);
		for (x=0; x<width/2; x++)
		{
		    pixel = clut[*src++];
		    val = ((pixel & 0xf80000) >> 9) | ((pixel & 0xf800) >> 6) |
			    ((pixel & 0xff) >> 3);
		    pixel = clut[*src++];
		    val |= (((pixel & 0xf80000) >> 9) | ((pixel & 0xf800) >> 6) |
			    ((pixel & 0xff) >> 3)) << 16;
		    *dst++ = val;
		}
		if (width%2)
		{
		    pixel = clut[*src++];
		    *((TQ_INT16 *)dst) = ((pixel & 0xf80000) >> 9) |
			    ((pixel & 0xf800) >> 6) | ((pixel & 0xff) >> 3);
		}
	    }
	}
	break;

    case bo16_RGB_565:
    case bo16_BGR_565:

	if (img.depth() == 32)
	{
	    TQRgb *src, pixel;
	    TQ_INT32 *dst, val;
	    for (y=0; y<height; y++)
	    {
		src = (TQRgb *) img.scanLine(y);
		dst = (TQ_INT32 *) (data + y*bpl);
		for (x=0; x<width/2; x++)
		{
		    pixel = *src++;
		    val = ((pixel & 0xf80000) >> 8) | ((pixel & 0xfc00) >> 5) |
			     ((pixel & 0xff) >> 3);
		    pixel = *src++;
		    val |= (((pixel & 0xf80000) >> 8) | ((pixel & 0xfc00) >> 5) |
			    ((pixel & 0xff) >> 3)) << 16;
		    *dst++ = val;
		}
		if (width%2)
		{
		    pixel = *src++;
		    *((TQ_INT16 *)dst) = ((pixel & 0xf80000) >> 8) |
			    ((pixel & 0xfc00) >> 5) | ((pixel & 0xff) >> 3);
		}
	    }
	} else
	{
	    uchar *src;
	    TQ_INT32 val, *dst;
	    TQRgb pixel, *clut = img.colorTable();
	    for (y=0; y<height; y++)
	    {
		src = const_cast<TQImage&>(img).scanLine(y);
		dst = (TQ_INT32 *) (data + y*bpl);
		for (x=0; x<width/2; x++)
		{
		    pixel = clut[*src++];
		    val = ((pixel & 0xf80000) >> 8) | ((pixel & 0xfc00) >> 5) |
			    ((pixel & 0xff) >> 3);
		    pixel = clut[*src++];
		    val |= (((pixel & 0xf80000) >> 8) | ((pixel & 0xfc00) >> 5) |
			    ((pixel & 0xff) >> 3)) << 16;
		    *dst++ = val;
		}
		if (width%2)
		{
		    pixel = clut[*src++];
		    *((TQ_INT16 *)dst) = ((pixel & 0xf80000) >> 8) |
			    ((pixel & 0xfc00) >> 5) | ((pixel & 0xff) >> 3);
		}
	    }
	}
	break;

    case bo24_RGB:

	if (img.depth() == 32)
	{
	    char *dst;
	    int w1 = width/4;
	    TQRgb *src, d1, d2, d3, d4;
	    for (y=0; y<height; y++)
	    {
		src = (TQRgb *) img.scanLine(y);
		dst = data + y*bpl;
		for (x=0; x<w1; x++)
		{
		    d1 = (*src++ & 0xffffff);
		    d2 = (*src++ & 0xffffff);
		    d3 = (*src++ & 0xffffff);
		    d4 = (*src++ & 0xffffff);
		    *((TQ_INT32 *)dst) = d1 | (d2 << 24);
		    *((TQ_INT32 *)dst+1) = (d2 >> 8) | (d3 << 16);
		    *((TQ_INT32 *)dst+2) = (d4 << 8) | (d3 >> 16);
		    dst += 12;
		}
		for (x=w1*4; x<width; x++)
		{
		    d1 = *src++;
		    *dst++ = tqRed(d1);
		    *dst++ = tqGreen(d1);
		    *dst++ = tqBlue(d1);
		}
	    }
	} else
	{
	    uchar *src, *dst;
	    int w1 = width/4;
	    TQRgb *clut = img.colorTable(), d1, d2, d3, d4;
	    for (y=0; y<height; y++)
	    {
		src = const_cast<TQImage&>(img).scanLine(y);
		dst = (uchar *) data + y*bpl;
		for (x=0; x<w1; x++)
		{
		    d1 = (clut[*src++] & 0xffffff);
		    d2 = (clut[*src++] & 0xffffff);
		    d3 = (clut[*src++] & 0xffffff);
		    d4 = (clut[*src++] & 0xffffff);
		    *((TQ_INT32 *)dst) = d1 | (d2 << 24);
		    *((TQ_INT32 *)dst+1) = (d2 >> 8) | (d3 << 16);
		    *((TQ_INT32 *)dst+2) = (d4 << 8) | (d3 >> 16);
		    dst += 12;
		}
		for (x=w1*4; x<width; x++)
		{
		    d1 = clut[*src++];
		    *dst++ = tqRed(d1);
		    *dst++ = tqGreen(d1);
		    *dst++ = tqBlue(d1);
		}
	    }
	}
	break;

    case bo24_BGR:

	if (img.depth() == 32)
	{
	    char *dst;
	    TQRgb *src, d1, d2, d3, d4;
	    int w1 = width/4;
	    for (y=0; y<height; y++)
	    {
		src = (TQRgb *) img.scanLine(y);
		dst = data + y*bpl;
		for (x=0; x<w1; x++)
		{
		    d1 = (*src++ & 0xffffff);
		    d2 = (*src++ & 0xffffff);
		    d3 = (*src++ & 0xffffff);
		    d4 = (*src++ & 0xffffff);
		    *((TQ_INT32 *)dst) = d1 | (d2 << 24);
		    *((TQ_INT32 *)dst+1) = (d2 >> 8) | (d3 << 16);
		    *((TQ_INT32 *)dst+2) = (d4 << 8) | (d3 >> 16);
		    dst += 12;
		}
		for (x=w1*4; x<width; x++)
		{
		    d1 = *src++;
		    *dst++ = tqBlue(d1);
		    *dst++ = tqGreen(d1);
		    *dst++ = tqRed(d1);
		}
	    }
	} else
	{
	    uchar *src, *dst;
	    int w1 = width/4;
	    TQRgb *clut = img.colorTable(), d1, d2, d3, d4;
	    for (y=0; y<height; y++)
	    {
		src = const_cast<TQImage&>(img).scanLine(y);
		dst = (uchar *) data + y*bpl;
		for (x=0; x<w1; x++)
		{
		    d1 = (clut[*src++] & 0xffffff);
		    d2 = (clut[*src++] & 0xffffff);
		    d3 = (clut[*src++] & 0xffffff);
		    d4 = (clut[*src++] & 0xffffff);
		    *((TQ_INT32 *)dst) = d1 | (d2 << 24);
		    *((TQ_INT32 *)dst+1) = (d2 >> 8) | (d3 << 16);
		    *((TQ_INT32 *)dst+2) = (d4 << 8) | (d3 >> 16);
		    dst += 12;
		}
		for (x=w1*4; x<width; x++)
		{
		    d1 = clut[*src++];
		    *dst++ = tqBlue(d1);
		    *dst++ = tqGreen(d1);
		    *dst++ = tqRed(d1);
		}
	    }
	}
	break;

    case bo32_ARGB:
    case bo32_BGRA:

	if (img.depth() == 32)
	{
	    for (y=0; y<height; y++)
		memcpy(data + y*bpl, img.scanLine(y), width*4);
	} else
	{
	    uchar *src;
	    TQRgb *dst, *clut = img.colorTable();
	    for (y=0; y<height; y++)
	    {
		src = const_cast<TQImage&>(img).scanLine(y);
		dst = (TQRgb *) (data + y*bpl);
		for (x=0; x<width; x++)
		    *dst++ = clut[*src++];
	    }
	}
	break;

    }
}

#else

void KPixmapIO::preAllocShm(int) {}
void KPixmapIO::setShmPolicy(int) {}
bool KPixmapIO::initXImage(int, int) { return false; }
void KPixmapIO::doneXImage() {}
bool KPixmapIO::createXImage(int, int) { return false; }
void KPixmapIO::destroyXImage() {}
bool KPixmapIO::createShmSegment(int) { return false; }
void KPixmapIO::destroyShmSegment() {}
TQImage KPixmapIO::convertFromXImage() { return TQImage(); }
void KPixmapIO::convertToXImage(const TQImage &) {}

#endif // HAVE_MITSHM
