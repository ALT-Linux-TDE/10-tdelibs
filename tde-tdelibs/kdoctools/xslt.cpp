#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <libxml/catalog.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <tqdir.h>
#include <tqregexp.h>
#include <xslt.h>
#include <kinstance.h>
#include "tdeio_help.h"
#include <tdelocale.h>
#include <assert.h>
#include <kfilterbase.h>
#include <kfilterdev.h>
#include <tqtextcodec.h>
#include <stdlib.h>
#include <config.h>
#include <stdarg.h>
#include <klibloader.h>
#include <kcharsets.h>
#include <gzip/kgzipfilter.h>
#include <bzip2/kbzip2filter.h>
#include <klibloader.h>
#include <tqvaluevector.h>

#if !defined( SIMPLE_XSLT )
extern HelpProtocol *slave;
#define INFO( x ) if (slave) slave->infoMessage(x);
#else
#define INFO( x )
#endif

int writeToQString(void * context, const char * buffer, int len)
{
    TQString *t = (TQString*)context;
    *t += TQString::fromUtf8(buffer, len);
    return len;
}

int closeQString(void * context) {
    TQString *t = (TQString*)context;
    *t += '\n';
    return 0;
}

TQString transform( const TQString &pat, const TQString& tss,
                   const TQValueVector<const char *> &params )
{
    TQString parsed;

    INFO(i18n("Parsing stylesheet"));

    xsltStylesheetPtr style_sheet =
        xsltParseStylesheetFile((const xmlChar *)tss.latin1());

    if ( !style_sheet ) {
        return parsed;
    }

    if (style_sheet->indent == 1)
        xmlIndentTreeOutput = 1;
    else
        xmlIndentTreeOutput = 0;

    INFO(i18n("Parsing document"));

    xmlDocPtr doc = xmlParseFile( pat.latin1() );
    xsltTransformContextPtr ctxt;

    ctxt = xsltNewTransformContext(style_sheet, doc);
    if (ctxt == NULL)
        return parsed;

    INFO(i18n("Applying stylesheet"));
    TQValueVector<const char *> p = params;
    p.append( NULL );
    xmlDocPtr res = xsltApplyStylesheet(style_sheet, doc, const_cast<const char **>(&p[0]));
    xmlFreeDoc(doc);
    if (res != NULL) {
        xmlOutputBufferPtr outp = xmlOutputBufferCreateIO(writeToQString, (xmlOutputCloseCallback)closeQString, &parsed, 0);
        outp->written = 0;
        INFO(i18n("Writing document"));
        xsltSaveResultTo ( outp, res, style_sheet );
        xmlOutputBufferFlush(outp);
        xmlFreeDoc(res);
    }
    xsltFreeStylesheet(style_sheet);

    if (parsed.isEmpty())
	parsed = " "; // avoid error message
    return parsed;
}

/*
xmlParserInputPtr meinExternalEntityLoader(const char *URL, const char *ID,
					   xmlParserCtxtPtr ctxt) {
    xmlParserInputPtr ret = NULL;

    // fprintf(stderr, "loading %s %s %s\n", URL, ID, ctxt->directory);

    if (URL == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
            ctxt->sax->warning(ctxt,
                    "failed to load external entity \"%s\"\n", ID);
        return(NULL);
    }
    if (!qstrcmp(ID, "-//OASIS//DTD DocBook XML V4.1.2//EN"))
        URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";
    if (!qstrcmp(ID, "-//OASIS//DTD XML DocBook V4.1.2//EN"))
	URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";

    TQString file;
    if (TDEStandardDirs::exists( TQDir::currentDirPath() + "/" + URL ) )
        file = TQDir::currentDirPath() + "/" + URL;
    else
        file = locate("dtd", URL);

    ret = xmlNewInputFromFile(ctxt, file.latin1());
    if (ret == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
            ctxt->sax->warning(ctxt,

                "failed to load external entity \"%s\"\n", URL);
    }
    return(ret);
}
*/

TQString splitOut(const TQString &parsed, int index)
{
    int start_index = index + 1;
    while (parsed.at(start_index - 1) != '>') start_index++;

    int inside = 0;

    TQString filedata;

    while (true) {
        int endindex = parsed.find("</FILENAME>", index);
        int startindex = parsed.find("<FILENAME ", index) + 1;

//        kdDebug() << "FILENAME " << startindex << " " << endindex << " " << inside << " " << parsed.mid(startindex + 18, 15)<< " " << parsed.length() << endl;

        if (startindex > 0) {
            if (startindex < endindex) {
                //              kdDebug() << "finding another" << endl;
                index = startindex + 8;
                inside++;
            } else {
                index = endindex + 8;
                inside--;
            }
        } else {
            inside--;
            index = endindex + 1;
        }

        if (inside == 0) {
            filedata = parsed.mid(start_index, endindex - start_index);
            break;
        }

    }

    index = filedata.find("<FILENAME ");

    if (index > 0) {
        int endindex = filedata.findRev("</FILENAME>");
        while (filedata.at(endindex) != '>') endindex++;
        endindex++;
        filedata = filedata.left(index) + filedata.mid(endindex);
    }

    // filedata.replace(TQRegExp(">"), "\n>");
    return filedata;
}

void fillInstance(TDEInstance &ins, const TQString &srcdir) {
    TQString catalogs;

    if ( srcdir.isEmpty() ) {
        catalogs += ins.dirs()->findResource("data", "ksgmltools2/customization/catalog.xml");
        catalogs += ':';
        catalogs += ins.dirs()->findResource("data", "ksgmltools2/docbook/xml-dtd-4.2/catalog.xml");
        ins.dirs()->addResourceType("dtd", TDEStandardDirs::kde_default("data") + "ksgmltools2");
    } else {
        catalogs += srcdir +"/customization/catalog.xml:" + srcdir + "/docbook/xml-dtd-4.2/catalog.xml";
        ins.dirs()->addResourceDir("dtd", srcdir);
    }

    xmlLoadCatalogs(catalogs.latin1());
}

extern "C" void *init_kbzip2filter();

static TQIODevice *getBZip2device(const TQString &fileName )
{
    TQFile * f = new TQFile( fileName );
    KLibFactory * factory = static_cast<KLibFactory*>(init_kbzip2filter());
    KFilterBase * base = static_cast<KFilterBase*>( factory->create(0, "bzip2" ) );

    if ( base )
    {
        base->setDevice(f, true);
        return new KFilterDev(base, true);
    }
    return 0;
}

bool saveToCache( const TQString &contents, const TQString &filename )
{
    TQIODevice *fd = ::getBZip2device(filename);
    if ( !fd )
        return false;

    if (!fd->open(IO_WriteOnly))
    {
       delete fd;
       return false;
    }

    fd->writeBlock( contents.utf8() );
    fd->close();
    delete fd;
    return true;
}

static bool readCache( const TQString &filename,
                       const TQString &cache, TQString &output)
{
    kdDebug( 7119 ) << "verifyCache " << filename << " " << cache << endl;
    if ( !compareTimeStamps( filename, cache ) )
        return false;
    if ( !compareTimeStamps( locate( "dtd", "customization/tde-chunk.xsl"), cache ) )
        return false;

    kdDebug( 7119 ) << "create filter" << endl;
    TQIODevice *fd = ::getBZip2device(cache);
    if ( !fd )
        return false;

    if (!fd->open(IO_ReadOnly))
    {
       delete fd;
       TQFile::remove(cache);
       return false;
    }

    kdDebug( 7119 ) << "reading" << endl;

    char buffer[32000];
    int n;
    TQCString text;
    // Also end loop in case of error, when -1 is returned
    while ( ( n = fd->readBlock(buffer, 31900) ) > 0)
    {
        buffer[n] = 0;
        text += buffer;
    }
    kdDebug( 7119 ) << "read " << text.length() << endl;
    fd->close();

    output = TQString::fromUtf8( text );
    delete fd;

    if (n == -1)
        return false;

    kdDebug( 7119 ) << "finished " << endl;

    return true;
}

TQString lookForCache( const TQString &filename )
{
    kdDebug() << "lookForCache " << filename << endl;
    assert( filename.endsWith( ".docbook" ) );
    assert( filename.at( 0 ) == '/' );

    TQString cache = filename.left( filename.length() - 7 );
    TQString output;
    if ( readCache( filename, cache + "cache.bz2", output) )
        return output;
    if ( readCache( filename,
                    locateLocal( "cache",
                                 "tdeio_help" + cache +
                                 "cache.bz2" ), output ) )
        return output;

    return TQString::null;
}

bool compareTimeStamps( const TQString &older, const TQString &newer )
{
    TQFileInfo _older( older );
    TQFileInfo _newer( newer );
    assert( _older.exists() );
    if ( !_newer.exists() )
        return false;
    return ( _newer.lastModified() > _older.lastModified() );
}

TQCString fromUnicode( const TQString &data )
{
    TQTextCodec *locale = TQTextCodec::codecForLocale();
    TQCString result;
    char buffer[30000];
    uint buffer_len = 0;
    uint len = 0;
    uint offset = 0;
    const int part_len = 5000;

    TQString part;

    while ( offset < data.length() )
    {
        part = data.mid( offset, part_len );
        TQCString test = locale->fromUnicode( part );
        if ( locale->toUnicode( test ) == part ) {
            result += test;
            offset += part_len;
            continue;
        }
        len = part.length();
        buffer_len = 0;
        for ( uint i = 0; i < len; i++ ) {
            TQCString test = locale->fromUnicode( part.mid( i, 1 ) );
            if ( locale->toUnicode( test ) == part.mid( i, 1 ) ) {
                if (buffer_len + test.length() + 1 > sizeof(buffer))
                   break;
                strcpy( buffer + buffer_len, test.data() );
                buffer_len += test.length();
            } else {
                TQString res;
                res.sprintf( "&#%d;", TQChar(part.at( i )).unicode() );
                test = locale->fromUnicode( res );
                if (buffer_len + test.length() + 1 > sizeof(buffer))
                   break;
                strcpy( buffer + buffer_len, test.data() );
                buffer_len += test.length();
            }
        }
        result += TQCString( buffer, buffer_len + 1);
        offset += part_len;
    }
    return result;
}

void replaceCharsetHeader( TQString &output )
{
    TQString name = TQTextCodec::codecForLocale()->name();
    name.replace( TQString( "ISO " ), "iso-" );
    output.replace( TQString( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" ),
                    TQString( "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\">" ).arg( name ) );
}
