#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <tqstring.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <xslt.h>
#include <tqfile.h>
#include <tqdir.h>
#include <tdecmdlineargs.h>
#include <tdelocale.h>
#include <tdeaboutdata.h>
#include <stdlib.h>
#include <kdebug.h>
#include <tqtextcodec.h>
#include <tqfileinfo.h>
#include <kprocess.h>
#include <tqvaluevector.h>

extern int xmlLoadExtDtdDefaultValue;

class MyPair {
public:
    TQString word;
    int base;};

typedef TQValueList<MyPair> PairList;

void parseEntry(PairList &list, xmlNodePtr cur, int base)
{
    if ( !cur )
        return;

    base += atoi( ( const char* )xmlGetProp(cur, ( const xmlChar* )"header") );
    if ( base > 10 ) // 10 is the maximum
        base = 10;

    /* We don't care what the top level element name is */
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {

        if ( cur->type == XML_TEXT_NODE ) {
            TQString words = TQString::fromUtf8( ( char* )cur->content );
            TQStringList wlist = TQStringList::split( ' ',  words.simplifyWhiteSpace() );
            for ( TQStringList::ConstIterator it = wlist.begin();
                  it != wlist.end(); ++it )
            {
                MyPair m;
                m.word = *it;
                m.base = base;
                list.append( m );
            }
        } else if ( !xmlStrcmp( cur->name, (const xmlChar *) "entry") )
            parseEntry( list, cur, base );

    	cur = cur->next;
    }

}

static TDECmdLineOptions options[] =
{
    { "stylesheet <xsl>",  I18N_NOOP( "Stylesheet to use" ), 0 },
    { "stdout", I18N_NOOP( "Output whole document to stdout" ), 0 },
    { "o", 0, 0 },
    { "output <file>", I18N_NOOP("Output whole document to file" ), 0 },
    { "htdig", I18N_NOOP( "Create a ht://dig compatible index" ), 0 },
    { "check", I18N_NOOP( "Check the document for validity" ), 0 },
    { "cache <file>", I18N_NOOP( "Create a cache file for the document" ), 0},
    { "srcdir <dir>", I18N_NOOP( "Set the srcdir, for tdelibs" ), 0},
    { "param <key>=<value>", I18N_NOOP( "Parameters to pass to the stylesheet" ), 0},
    { "+xml", I18N_NOOP("The file to transform"), 0},
    TDECmdLineLastOption // End of options.
};




int main(int argc, char **argv) {

    // xsltSetGenericDebugFunc(stderr, NULL);

    TDEAboutData aboutData( "meinproc", I18N_NOOP("XML-Translator" ),
	"$Revision$",
	I18N_NOOP("TDE Translator for XML"));

    TDECmdLineArgs::init(argc, argv, &aboutData);
    TDECmdLineArgs::addCmdLineOptions( options );

    TDELocale::setMainCatalogue("tdeio_help");
    TDEInstance ins("meinproc");
    TDEGlobal::locale();


    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
    if ( args->count() != 1 ) {
        args->usage();
        return ( 1 );
    }

    // Need to set SRCDIR before calling fillInstance
    TQString srcdir;
    if ( args->isSet( "srcdir" ) )
        srcdir = TQDir( TQFile::decodeName( args->getOption( "srcdir" ) ) ).absPath();
    fillInstance(ins,srcdir);

    LIBXML_TEST_VERSION

    TQString checkFilename = TQFile::decodeName(args->arg( 0 ));
    TQFileInfo checkFile(checkFilename);
    if (!checkFile.exists())
    {
        kdError() << "File '" << checkFilename << "' does not exist." << endl;
        return ( 2 );
    }
    if (!checkFile.isFile())
    {
        kdError() << "'" << checkFilename << "' is not a file." << endl;
        return ( 2 );
    }
    if (!checkFile.isReadable())
    {
        kdError() << "File '" << checkFilename << "' is not readable." << endl;
        return ( 2 );
    }

    if ( args->isSet( "check" ) ) {
#if !defined(PATH_MAX) && defined(__GLIBC__)
        char *pwd_buffer;
#else
        char pwd_buffer[PATH_MAX];
#endif
        TQFileInfo file( TQFile::decodeName(args->arg( 0 )) );
#if !defined(PATH_MAX) && defined(__GLIBC__)
        if ( !(pwd_buffer = getcwd( NULL, 0 ) ) )
#else
        if ( !getcwd( pwd_buffer, sizeof(pwd_buffer) - 1 ) )
#endif
	{
	     kdError() << "getcwd failed." << endl;
             return 2;
	}

        TQString catalogs;
        catalogs += locate( "dtd", "customization/catalog.xml" );
        catalogs += " ";
        catalogs += locate( "dtd", "docbook/xml-dtd-4.1.2/catalog.xml" );

        setenv( "XML_CATALOG_FILES", TQFile::encodeName( catalogs ).data(), 1);
        TQString exe;
#if defined( XMLLINT )
        exe = XMLLINT;
#endif
        if ( (::access( TQFile::encodeName( exe ), X_OK )!=0) ) {
            exe = TDEStandardDirs::findExe( "xmllint" );
            if (exe.isEmpty())
                exe = locate( "exe", "xmllint" );
        }
        if ( ::access( TQFile::encodeName( exe ), X_OK )==0 ) {
            chdir( TQFile::encodeName( file.dirPath( true ) ) );
            TQString cmd = exe;
            cmd += " --valid --noout ";
            cmd += TDEProcess::quote(file.fileName());
            cmd += " 2>&1";
            FILE *xmllint = popen( TQFile::encodeName( cmd ), "r");
            char buf[ 512 ];
            bool noout = true;
            unsigned int n;
            while ( ( n = fread(buf, 1, sizeof( buf ), xmllint ) ) ) {
                noout = false;
                buf[ n ] = '\0';
                fputs( buf, stderr );
            }
            pclose( xmllint );
            chdir( pwd_buffer );
            if ( !noout ) {
#if !defined(PATH_MAX) && defined(__GLIBC__)
                free( pwd_buffer );
#endif
                return 1;
            }
        } else {
            kdWarning() << "couldn't find xmllint" << endl;
        }
#if !defined(PATH_MAX) && defined(__GLIBC__)
        free( pwd_buffer );
#endif
    }

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;

    TQValueVector<const char *> params;
    if (args->isSet( "output" ) ) {
        params.append( tqstrdup( "outputFile" ) );
        params.append( tqstrdup( TQString(TQFile::decodeName( args->getOption( "output" ) )).latin1() ) );
    }
    {
        const QCStringList paramList = args->getOptionList( "param" );
        QCStringList::ConstIterator it = paramList.begin();
        QCStringList::ConstIterator end = paramList.end();
        for ( ; it != end; ++it ) {
            const TQCString tuple = *it;
            const int ch = tuple.find( '=' );
            if ( ch == -1 ) {
                kdError() << "Key-Value tuple '" << tuple << "' lacks a '='!" << endl;
                return( 2 );
            }
            params.append( tqstrdup( tuple.left( ch ) ) );
            params.append( tqstrdup( tuple.mid( ch + 1 ) )  );
        }
    }
    params.append( NULL );

    bool index = args->isSet( "htdig" );
    TQString tss = args->getOption( "stylesheet" );
    if ( tss.isEmpty() )
        tss =  "customization/tde-chunk.xsl";
    if ( index )
        tss = "customization/htdig_index.xsl" ;

    tss = locate( "dtd", tss );

    if ( index ) {
        xsltStylesheetPtr style_sheet =
            xsltParseStylesheetFile((const xmlChar *)tss.latin1());

        if (style_sheet != NULL) {

            xmlDocPtr doc = xmlParseFile( TQFile::encodeName( args->arg( 0 ) ) );

            xmlDocPtr res = xsltApplyStylesheet(style_sheet, doc, &params[0]);

            xmlFreeDoc(doc);
            xsltFreeStylesheet(style_sheet);
            if (res != NULL) {
                xmlNodePtr cur = xmlDocGetRootElement(res);
                if (!cur || xmlStrcmp(cur->name, (const xmlChar *) "entry")) {
                    fprintf(stderr,"document of the wrong type, root node != entry");
                    xmlFreeDoc(res);
                    return(1);
                }
                PairList list;
                parseEntry( list, cur, 0 );
                int wi = 0;
                for ( PairList::ConstIterator it = list.begin(); it != list.end();
                      ++it, ++wi )
                    fprintf( stdout, "w\t%s\t%d\t%d\n", ( *it ).word.utf8().data(),
                             1000*wi/(int)list.count(), ( *it ).base );

                xmlFreeDoc(res);
            } else {
                kdDebug() << "couldn't parse document " << args->arg( 0 ) << endl;
            }
        } else {
            kdDebug() << "couldn't parse style sheet " << tss << endl;
        }

    } else {
        TQString output = transform(args->arg( 0 ) , tss, params);
        if (output.isEmpty()) {
            fprintf(stderr, "unable to parse %s\n", args->arg( 0 ));
            return(1);
        }

        TQString cache = args->getOption( "cache" );
        if ( !cache.isEmpty() ) {
            if ( !saveToCache( output, cache ) ) {
                kdError() << TQString(i18n( "Could not write to cache file %1." ).arg( cache )) << endl;
            }
            goto end;
        }

        if (output.find( "<FILENAME " ) == -1 || args->isSet( "stdout" ) || args->isSet("output") )
        {
            TQFile file;
            if (args->isSet( "stdout" ) ) {
                file.open( IO_WriteOnly, stdout );
            } else {
                if (args->isSet( "output" ) )
                   file.setName( TQFile::decodeName(args->getOption( "output" )));
                else
                   file.setName( "index.html" );
                file.open(IO_WriteOnly);
            }
            replaceCharsetHeader( output );

            TQCString data = output.local8Bit();
            file.writeBlock(data.data(), data.length());
            file.close();
        } else {
            int index = 0;
            while (true) {
                index = output.find("<FILENAME ", index);
                if (index == -1)
                    break;
                int filename_index = index + strlen("<FILENAME filename=\"");

                TQString filename = output.mid(filename_index,
                                              output.find("\"", filename_index) -
                                              filename_index);

                TQString filedata = splitOut(output, index);
                TQFile file(filename);
                file.open(IO_WriteOnly);
                replaceCharsetHeader( filedata );
                TQCString data = fromUnicode( filedata );
                file.writeBlock(data.data(), data.length());
                file.close();

                index += 8;
            }
        }
    }
 end:
    xmlCleanupParser();
    xmlMemoryDump();
    return(0);
}

