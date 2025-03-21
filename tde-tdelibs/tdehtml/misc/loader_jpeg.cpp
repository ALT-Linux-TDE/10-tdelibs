/*
    This file is part of the KDE libraries

    Copyright (C) 2000 Dirk Mueller (mueller@kde.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBJPEG
// on some systems, libjpeg installs its config.h file which causes a conflict
// and makes the compiler barf with "XXX already defined".
#ifdef HAVE_STDLIB_H
#undef HAVE_STDLIB_H
#endif
#include "loader_jpeg.h"

#include <stdio.h>
#include <setjmp.h>
#include <tqdatetime.h>
#include <tdeglobal.h>

extern "C" {
#define XMD_H
#include <jpeglib.h>
#undef const
}


#undef BUFFER_DEBUG
//#define BUFFER_DEBUG

#undef JPEG_DEBUG
//#define JPEG_DEBUG

// -----------------------------------------------------------------------------

struct tdehtml_error_mgr : public jpeg_error_mgr {
    jmp_buf setjmp_buffer;
};

extern "C" {

    static
    void tdehtml_error_exit (j_common_ptr cinfo)
    {
        tdehtml_error_mgr* myerr = (tdehtml_error_mgr*) cinfo->err;
        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);
#ifdef JPEG_DEBUG
        tqWarning("%s", buffer);
#endif
        longjmp(myerr->setjmp_buffer, 1);
    }
}

static const int max_buf = 32768;
static const int max_consumingtime = 2000;

struct tdehtml_jpeg_source_mgr : public jpeg_source_mgr {
    JOCTET buffer[max_buf];

    int valid_buffer_len;
    size_t skip_input_bytes;
    int ateof;
    TQRect change_rect;
    TQRect old_change_rect;
    TQTime decoder_timestamp;
    bool final_pass;
    bool decoding_done;
    bool do_progressive;

public:
    tdehtml_jpeg_source_mgr() TDE_NO_EXPORT;
};


extern "C" {

    static
    void tdehtml_j_decompress_dummy(j_decompress_ptr)
    {
    }

    static
    boolean tdehtml_fill_input_buffer(j_decompress_ptr cinfo)
    {
#ifdef BUFFER_DEBUG
        tqDebug("tdehtml_fill_input_buffer called!");
#endif

        tdehtml_jpeg_source_mgr* src = (tdehtml_jpeg_source_mgr*)cinfo->src;

        if ( src->ateof )
        {
            /* Insert a fake EOI marker - as per jpeglib recommendation */
            src->buffer[0] = (JOCTET) 0xFF;
            src->buffer[1] = (JOCTET) JPEG_EOI;
            src->bytes_in_buffer = 2;
            src->next_input_byte = (JOCTET *) src->buffer;
#ifdef BUFFER_DEBUG
            tqDebug("...returning true!");
#endif
            return true;
        }
        else
            return false;  /* I/O suspension mode */
    }

    static
    void tdehtml_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
    {
        if(num_bytes <= 0)
            return; /* required noop */

#ifdef BUFFER_DEBUG
        tqDebug("tdehtml_skip_input_data (%d) called!", num_bytes);
#endif

        tdehtml_jpeg_source_mgr* src = (tdehtml_jpeg_source_mgr*)cinfo->src;
        src->skip_input_bytes += num_bytes;

        unsigned int skipbytes = kMin(src->bytes_in_buffer, src->skip_input_bytes);

#ifdef BUFFER_DEBUG
        tqDebug("skip_input_bytes is now %d", src->skip_input_bytes);
        tqDebug("skipbytes is now %d", skipbytes);
        tqDebug("valid_buffer_len is before %d", src->valid_buffer_len);
        tqDebug("bytes_in_buffer is %d", src->bytes_in_buffer);
#endif

        if(skipbytes < src->bytes_in_buffer)
            memmove(src->buffer, src->next_input_byte+skipbytes, src->bytes_in_buffer - skipbytes);

        src->bytes_in_buffer -= skipbytes;
        src->valid_buffer_len = src->bytes_in_buffer;
        src->skip_input_bytes -= skipbytes;

        /* adjust data for jpeglib */
        cinfo->src->next_input_byte = (JOCTET *) src->buffer;
        cinfo->src->bytes_in_buffer = (size_t) src->valid_buffer_len;
#ifdef BUFFER_DEBUG
        tqDebug("valid_buffer_len is afterwards %d", src->valid_buffer_len);
        tqDebug("skip_input_bytes is now %d", src->skip_input_bytes);
#endif
    }
}


tdehtml_jpeg_source_mgr::tdehtml_jpeg_source_mgr()
{
    jpeg_source_mgr::init_source = tdehtml_j_decompress_dummy;
    jpeg_source_mgr::fill_input_buffer = tdehtml_fill_input_buffer;
    jpeg_source_mgr::skip_input_data = tdehtml_skip_input_data;
    jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
    jpeg_source_mgr::term_source = tdehtml_j_decompress_dummy;
    bytes_in_buffer = 0;
    valid_buffer_len = 0;
    skip_input_bytes = 0;
    ateof = 0;
    next_input_byte = buffer;
    final_pass = false;
    decoding_done = false;
}



// -----------------------------------------------------------------------------

class KJPEGFormat : public TQImageFormat
{
public:
    KJPEGFormat();

    virtual ~KJPEGFormat();

    virtual int decode(TQImage& img, TQImageConsumer* consumer,
                       const uchar* buffer, int length);
private:

    enum {
        Init,
        readHeader,
        startDecompress,
        decompressStarted,
        consumeInput,
        prepareOutputScan,
        doOutputScan,
        readDone,
        invalid
    } state;

    // structs for the jpeglib
    struct jpeg_decompress_struct cinfo;
    struct tdehtml_error_mgr jerr;
    struct tdehtml_jpeg_source_mgr jsrc;
};


// -----------------------------------------------------------------------------

KJPEGFormat::KJPEGFormat()
{
    memset(&cinfo, 0, sizeof(cinfo));
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = tdehtml_error_exit;
    cinfo.src = &jsrc;
    state = Init;
}


KJPEGFormat::~KJPEGFormat()
{
    (void) jpeg_destroy_decompress(&cinfo);
}

/*
 * return  > 0 means "consumed x bytes, need more"
 * return == 0 means "end of frame reached"
 * return  < 0 means "fatal error in image decoding, don't call me ever again"
 */

int KJPEGFormat::decode(TQImage& image, TQImageConsumer* consumer, const uchar* buffer, int length)
{
#ifdef JPEG_DEBUG
    tqDebug("KJPEGFormat::decode(%08lx, %08lx, %08lx, %d)",
           &image, consumer, buffer, length);
#endif

    if(jsrc.ateof) {
#ifdef JPEG_DEBUG
        tqDebug("ateof, eating");
#endif
        return length;
    }


    if(setjmp(jerr.setjmp_buffer))
    {
#ifdef JPEG_DEBUG
        tqDebug("jump into state invalid");
#endif
        if(consumer)
            consumer->end();

        // this is fatal
        return -1;
    }

    int consumed = kMin(length, max_buf - jsrc.valid_buffer_len);

#ifdef BUFFER_DEBUG
    tqDebug("consuming %d bytes", consumed);
#endif

    // filling buffer with the new data
    memcpy(jsrc.buffer + jsrc.valid_buffer_len, buffer, consumed);
    jsrc.valid_buffer_len += consumed;

    if(jsrc.skip_input_bytes)
    {
#ifdef BUFFER_DEBUG
        tqDebug("doing skipping");
        tqDebug("valid_buffer_len %d", jsrc.valid_buffer_len);
        tqDebug("skip_input_bytes %d", jsrc.skip_input_bytes);
#endif
        int skipbytes = kMin((size_t) jsrc.valid_buffer_len, jsrc.skip_input_bytes);

        if(skipbytes < jsrc.valid_buffer_len)
            memmove(jsrc.buffer, jsrc.buffer+skipbytes, jsrc.valid_buffer_len - skipbytes);

        jsrc.valid_buffer_len -= skipbytes;
        jsrc.skip_input_bytes -= skipbytes;

        // still more bytes to skip
        if(jsrc.skip_input_bytes) {
            if(consumed <= 0) tqDebug("ERROR!!!");
            return consumed;
        }

    }

    cinfo.src->next_input_byte = (JOCTET *) jsrc.buffer;
    cinfo.src->bytes_in_buffer = (size_t) jsrc.valid_buffer_len;

#ifdef BUFFER_DEBUG
    tqDebug("buffer contains %d bytes", jsrc.valid_buffer_len);
#endif

    if(state == Init)
    {
        if(jpeg_read_header(&cinfo, true) != JPEG_SUSPENDED) {
            // do some simple memory requirements limitations
            // as long as we use that stupid TQt stuff
            int s = cinfo.image_width * cinfo.image_height;
            if ( s > 16384 * 12388 )
                cinfo.scale_denom = 8;
            else if ( s > 8192 * 6144 )
                cinfo.scale_denom = 4;
            else if ( s > 4096 * 3072 )
                cinfo.scale_denom = 2;

            if ( consumer )
                consumer->setSize(cinfo.image_width/cinfo.scale_denom,
                                  cinfo.image_height/cinfo.scale_denom);

            state = startDecompress;
        }
    }

    if(state == startDecompress)
    {
        jsrc.do_progressive = jpeg_has_multiple_scans( &cinfo );

#ifdef JPEG_DEBUG
        tqDebug( "**** DOPROGRESSIVE: %d",  jsrc.do_progressive );
#endif
        if ( jsrc.do_progressive )
            cinfo.buffered_image = true;
        else
            cinfo.buffered_image = false;

        // setup image sizes
        jpeg_calc_output_dimensions( &cinfo );

        if ( cinfo.jpeg_color_space == JCS_YCbCr )
            cinfo.out_color_space = JCS_RGB;

        cinfo.do_fancy_upsampling = true;
        cinfo.do_block_smoothing = false;
        cinfo.quantize_colors = false;

        // false: IO suspension
        if(jpeg_start_decompress(&cinfo)) {
            if ( cinfo.output_components == 3 || cinfo.output_components == 4) {
                image.create( cinfo.output_width, cinfo.output_height, 32 );
            } else if ( cinfo.output_components == 1 ) {
                image.create( cinfo.output_width, cinfo.output_height, 8, 256 );
                for (int i=0; i<256; i++)
                    image.setColor(i, tqRgb(i,i,i));
            }

#ifdef JPEG_DEBUG
            tqDebug("will create a picture %d/%d in size", cinfo.output_width, cinfo.output_height);
#endif

#ifdef JPEG_DEBUG
            tqDebug("ok, going to decompressStarted");
#endif

            jsrc.decoder_timestamp.start();
            state = jsrc.do_progressive ? decompressStarted : doOutputScan;
        }
    }

again:

    if(state == decompressStarted) {
        state =  (!jsrc.final_pass && jsrc.decoder_timestamp.elapsed() < max_consumingtime)
                ? consumeInput : prepareOutputScan;
    }

    if(state == consumeInput)
    {
        int retval;

        do {
            retval = jpeg_consume_input(&cinfo);
        } while (retval != JPEG_SUSPENDED && retval != JPEG_REACHED_EOI
                 && (retval != JPEG_REACHED_SOS || jsrc.decoder_timestamp.elapsed() < max_consumingtime));

        if(jsrc.decoder_timestamp.elapsed() >= max_consumingtime ||
           jsrc.final_pass ||
           retval == JPEG_REACHED_EOI || retval == JPEG_REACHED_SOS)
            state = prepareOutputScan;
    }

    if(state == prepareOutputScan)
    {
        if ( jpeg_start_output(&cinfo, cinfo.input_scan_number) )
            state = doOutputScan;
    }

    if(state == doOutputScan)
    {
        if(image.isNull() || jsrc.decoding_done)
        {
#ifdef JPEG_DEBUG
            tqDebug("complete in doOutputscan, eating..");
#endif
            return consumed;
        }
        uchar** lines = image.jumpTable();
        int oldoutput_scanline = cinfo.output_scanline;

        while(cinfo.output_scanline < cinfo.output_height &&
              jpeg_read_scanlines(&cinfo, lines+cinfo.output_scanline, cinfo.output_height))
            ; // here happens all the magic of decoding

        int completed_scanlines = cinfo.output_scanline - oldoutput_scanline;
#ifdef JPEG_DEBUG
        tqDebug("completed now %d scanlines", completed_scanlines);
#endif

        if ( cinfo.output_components == 3 ) {
	    // Expand 24->32 bpp.
	    for (int j=oldoutput_scanline; j<oldoutput_scanline+completed_scanlines; j++) {
		uchar *in = image.scanLine(j) + cinfo.output_width * 3;
		TQRgb *out = (TQRgb*)image.scanLine(j);

		for (uint i=cinfo.output_width; i--; ) {
		    in-=3;
		    out[i] = tqRgb(in[0], in[1], in[2]);
		}
	    }
	}

        if(consumer && completed_scanlines)
        {
            TQRect r(0, oldoutput_scanline, cinfo.output_width, completed_scanlines);
#ifdef JPEG_DEBUG
            tqDebug("changing %d/%d %d/%d", r.x(), r.y(), r.width(), r.height());
#endif
            jsrc.change_rect |= r;

            if ( jsrc.decoder_timestamp.elapsed() >= max_consumingtime ) {
                if( !jsrc.old_change_rect.isEmpty()) {
                    consumer->changed(jsrc.old_change_rect);
                    jsrc.old_change_rect = TQRect();
                }
                consumer->changed(jsrc.change_rect);
                jsrc.change_rect = TQRect();
                jsrc.decoder_timestamp.restart();
            }
        }

        if(cinfo.output_scanline >= cinfo.output_height)
        {
            if ( jsrc.do_progressive ) {
                jpeg_finish_output(&cinfo);
                jsrc.final_pass = jpeg_input_complete(&cinfo);
                jsrc.decoding_done = jsrc.final_pass && cinfo.input_scan_number == cinfo.output_scan_number;
                if ( !jsrc.decoding_done ) {
                    jsrc.old_change_rect |= jsrc.change_rect;
                    jsrc.change_rect =  TQRect();
                }
            }
            else
                jsrc.decoding_done = true;

#ifdef JPEG_DEBUG
            tqDebug("one pass is completed, final_pass = %d, dec_done: %d, complete: %d",
                   jsrc.final_pass, jsrc.decoding_done, jpeg_input_complete(&cinfo));
#endif
            if(!jsrc.decoding_done)
            {
#ifdef JPEG_DEBUG
                tqDebug("starting another one, input_scan_number is %d/%d", cinfo.input_scan_number,
                       cinfo.output_scan_number);
#endif
                jsrc.decoder_timestamp.restart();
                state = decompressStarted;
                // don't return until necessary!
                goto again;
            }
        }

        if(state == doOutputScan && jsrc.decoding_done) {
#ifdef JPEG_DEBUG
            tqDebug("input is complete, cleaning up, returning..");
#endif
            if ( consumer && !jsrc.change_rect.isEmpty() )
                consumer->changed( jsrc.change_rect );

            if(consumer)
                consumer->end();

            jsrc.ateof = true;

            (void) jpeg_finish_decompress(&cinfo);
            (void) jpeg_destroy_decompress(&cinfo);

            state = readDone;

            return 0;
        }
    }

#ifdef BUFFER_DEBUG
    tqDebug("valid_buffer_len is now %d", jsrc.valid_buffer_len);
    tqDebug("bytes_in_buffer is now %d", jsrc.bytes_in_buffer);
    tqDebug("consumed %d bytes", consumed);
#endif
    if(jsrc.bytes_in_buffer && jsrc.buffer != jsrc.next_input_byte)
        memmove(jsrc.buffer, jsrc.next_input_byte, jsrc.bytes_in_buffer);
    jsrc.valid_buffer_len = jsrc.bytes_in_buffer;
    return consumed;
}

// -----------------------------------------------------------------------------
// This is the factory that teaches TQt about progressive JPEG's

TQImageFormat* tdehtml::KJPEGFormatType::decoderFor(const unsigned char* buffer, int length)
{
    if(length < 3) return 0;

    if(buffer[0] == 0377 &&
       buffer[1] == 0330 &&
       buffer[2] == 0377)
         return new KJPEGFormat;

    return 0;
}

const char* tdehtml::KJPEGFormatType::formatName() const
{
    return "JPEG";
}

#else
#ifdef __GNUC__
#warning You don't seem to have libJPEG. jpeg support in tdehtml won't work
#endif
#endif // HAVE_LIBJPEG

// -----------------------------------------------------------------------------

