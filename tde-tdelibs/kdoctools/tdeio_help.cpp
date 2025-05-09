#include <config.h>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <tqvaluelist.h>
#include <tqfileinfo.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <tqtextcodec.h>

#include <kdebug.h>
#include <kurl.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kinstance.h>

#include "tdeio_help.h"
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>
#include "xslt.h"

using namespace TDEIO;

TQString HelpProtocol::langLookup(const TQString &fname)
{
    TQStringList search;

    // assemble the local search paths
    const TQStringList localDoc = TDEGlobal::dirs()->resourceDirs("html") + TDEGlobal::dirs()->resourceDirs("html-bundle");

    TQStringList langs = TDEGlobal::locale()->languageList();
    langs.append( "en" );
    langs.remove( "C" );

    // this is kind of compat hack as we install our docs in en/ but the
    // default language is en_US
    for (TQStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
        if ( *it == "en_US" )
            *it = "en";

    // look up the different languages
    int ldCount = localDoc.count();
    for (int id=0; id < ldCount; id++)
    {
        TQStringList::ConstIterator lang;
        for (lang = langs.begin(); lang != langs.end(); ++lang)
            search.append(TQString("%1%2/%3").arg(localDoc[id], *lang, fname));
    }

    // try to locate the file
    TQStringList::Iterator it;
    for (it = search.begin(); it != search.end(); ++it)
    {
        kdDebug( 7119 ) << "Looking for help in: " << *it << endl;

        TQFileInfo info(*it);
        if (info.exists() && info.isFile() && info.isReadable())
            return *it;

        if ( ( *it ).right( 5 ) == ".html" )
        {
            TQString file = (*it).left((*it).findRev('/')) + "/index.docbook";
            kdDebug( 7119 ) << "Looking for help in: " << file << endl;
            info.setFile(file);
            if (info.exists() && info.isFile() && info.isReadable())
                return *it;
        }
    }


    return TQString::null;
}


TQString HelpProtocol::lookupFile(const TQString &fname, const TQString &query, bool &redirect)
{
    redirect = false;
    TQString result = langLookup(fname);
    if (result.isEmpty())
    {
        result = langLookup(fname+"/index.html");
        if (!result.isEmpty())
        {
            KURL red("help:/");
            red.setPath( fname+"/index.html" );
            red.setQuery( query );
            redirection(red);
            kdDebug( 7119 ) << "redirect to " << red.url() << endl;
            redirect = true;
        }
        else
        {
            const TQString helpNotFound("khelpcenter/helpnotfound/index.html");
            result = langLookup(helpNotFound);
            if (!result.isEmpty())
            {
                KURL red("help:/");
                red.setPath(helpNotFound);
                red.setQuery(query);
                redirection(red);
                kdDebug( 7119 ) << "redirect to " << red.url() << endl;
                redirect = true;
            }
            else
            {
                unicodeError(i18n("Sorry, there is no documentation available at all for %1." ).arg(fname));
                finished();
                return TQString::null;
            }
        }
    } else
        kdDebug( 7119 ) << "result " << result << endl;

    return result;
}


void HelpProtocol::unicodeError( const TQString &t )
{
   data(fromUnicode( TQString(
        "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\"></head>\n"
        "%2</html>" ).arg( TQTextCodec::codecForLocale()->name() ).arg( t ) ) );
}

HelpProtocol *slave = 0;

HelpProtocol::HelpProtocol( bool ghelp, const TQCString &pool, const TQCString &app )
  : SlaveBase( ghelp ? "ghelp" : "help", pool, app ), mGhelp( ghelp )
{
    slave = this;
}

void HelpProtocol::get( const KURL& url )
{
    kdDebug( 7119 ) << "get: path=" << url.path()
              << " query=" << url.query() << endl;

    bool redirect;
    TQString doc;
    doc = url.path();

    if ( !mGhelp ) {
        if (doc.at(0) != '/')
            doc = doc.prepend('/');

        if (doc.at(doc.length() - 1) == '/')
            doc += "index.html";
    }

    infoMessage(i18n("Looking up correct file"));

    if ( !mGhelp ) {
      doc = lookupFile(doc, url.query(), redirect);

      if (redirect)
      {
          finished();
          return;
      }
    }

    if (doc.isEmpty())
    {
        error( TDEIO::ERR_DOES_NOT_EXIST, url.url() );
        return;
    }

    mimeType("text/html");
    KURL target;
    target.setPath(doc);
    if (url.hasHTMLRef())
        target.setHTMLRef(url.htmlRef());

    kdDebug( 7119 ) << "target " << target.url() << endl;

    TQString file = target.path();
    
    if ( mGhelp ) {
      if ( file.right( 4 ) != ".xml" ) {
         get_file( target );
         return;
      }
    } else {
        TQString docbook_file = file.left(file.findRev('/')) + "/index.docbook";
        if (!TDEStandardDirs::exists(file)) {
            file = docbook_file;
        } else {
            TQFileInfo fi(file);
            if (fi.isDir()) {
                file = file + "/index.docbook";
            } else {
                if ( file.right( 5 ) != ".html" || !compareTimeStamps( file, docbook_file ) ) {
                    get_file( target );
                    return;
                } else
                    file = docbook_file;
            }
        }
    }

    infoMessage(i18n("Preparing document"));

    if ( mGhelp ) {
        TQString xsl = "customization/tde-nochunk.xsl";
        mParsed = transform(file, locate("dtd", xsl));

        kdDebug( 7119 ) << "parsed " << mParsed.length() << endl;

        if (mParsed.isEmpty()) {
            unicodeError( i18n( "The requested help file could not be parsed:<br>%1" ).arg( file ) );
        } else {
            int pos1 = mParsed.find( "charset=" );
            if ( pos1 > 0 ) {
              int pos2 = mParsed.find( '"', pos1 );
              if ( pos2 > 0 ) {
                mParsed.replace( pos1, pos2 - pos1, "charset=UTF-8" );
              }
            }
            data( mParsed.utf8() );
        }
    } else {

        kdDebug( 7119 ) << "look for cache for " << file << endl;

        mParsed = lookForCache( file );

        kdDebug( 7119 ) << "cached parsed " << mParsed.length() << endl;

        if ( mParsed.isEmpty() ) {
            mParsed = transform(file, locate("dtd", "customization/tde-chunk.xsl"));
            if ( !mParsed.isEmpty() ) {
                infoMessage( i18n( "Saving to cache" ) );
                TQString cache = file.left( file.length() - 7 );
                saveToCache( mParsed, locateLocal( "cache",
                                                        "tdeio_help" + cache +
                                                        "cache.bz2" ) );
            }
        } else infoMessage( i18n( "Using cached version" ) );

        kdDebug( 7119 ) << "parsed " << mParsed.length() << endl;

        if (mParsed.isEmpty()) {
            unicodeError( i18n( "The requested help file could not be parsed:<br>%1" ).arg( file ) );
        } else {
            TQString query = url.query(), anchor;

            // if we have a query, look if it contains an anchor
            if (!query.isEmpty())
                if (query.left(8) == "?anchor=") {
                    anchor = query.mid(8).lower();

			    KURL redirURL(url);

			    redirURL.setQuery(TQString::null);
			    redirURL.setHTMLRef(anchor);
			    redirection(redirURL);
			    finished();
			    return;
		    }
            if (anchor.isEmpty() && url.hasHTMLRef())
	        anchor = url.htmlRef();

            kdDebug( 7119 ) << "anchor: " << anchor << endl;

            if ( !anchor.isEmpty() )
            {
                int index = 0;
                while ( true ) {
                    index = mParsed.find( TQRegExp( "<a name=" ), index);
                    if ( index == -1 ) {
                        kdDebug( 7119 ) << "no anchor\n";
                        break; // use whatever is the target, most likely index.html
                    }

                    if ( mParsed.mid( index, 11 + anchor.length() ).lower() ==
                         TQString( "<a name=\"%1\">" ).arg( anchor ) )
                    {
                        index = mParsed.findRev( "<FILENAME filename=", index ) +
                                 strlen( "<FILENAME filename=\"" );
                        TQString filename=mParsed.mid( index, 2000 );
                        filename = filename.left( filename.find( '\"' ) );
                        TQString path = target.path();
                        path = path.left( path.findRev( '/' ) + 1) + filename;
                        kdDebug( 7119 ) << "anchor found in " << path <<endl;
                        target.setPath( path );
                        break;
                    }
                    index++;
                }
            }
            emitFile( target );
        }
    }

    finished();
}

void HelpProtocol::emitFile( const KURL& url )
{
    infoMessage(i18n("Looking up section"));

    TQString filename = url.path().mid(url.path().findRev('/') + 1);

    int index = mParsed.find(TQString("<FILENAME filename=\"%1\"").arg(filename));
    if (index == -1) {
        if ( filename == "index.html" ) {
            data( fromUnicode( mParsed ) );
            return;
        }

        unicodeError( i18n("Could not find filename %1 in %2.").arg(filename).arg( url.url() ) );
        return;
    }

    TQString filedata = splitOut(mParsed, index);
    replaceCharsetHeader( filedata );

    data( fromUnicode( filedata ) );
    data( TQByteArray() );
}

void HelpProtocol::mimetype( const KURL &)
{
    mimeType("text/html");
    finished();
}

// Copied from tdeio_file to avoid redirects

#define MAX_IPC_SIZE (1024*32)

void HelpProtocol::get_file( const KURL& url )
{
    kdDebug( 7119 ) << "get_file " << url.url() << endl;

    TQCString _path( TQFile::encodeName(url.path()));
    struct stat buff;
    if ( ::stat( _path.data(), &buff ) == -1 ) {
        if ( errno == EACCES )
           error( TDEIO::ERR_ACCESS_DENIED, url.path() );
        else
           error( TDEIO::ERR_DOES_NOT_EXIST, url.path() );
	return;
    }

    if ( S_ISDIR( buff.st_mode ) ) {
	error( TDEIO::ERR_IS_DIRECTORY, url.path() );
	return;
    }
    if ( S_ISFIFO( buff.st_mode ) || S_ISSOCK ( buff.st_mode ) ) {
	error( TDEIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
	return;
    }

    int fd = open( _path.data(), O_RDONLY);
    if ( fd < 0 ) {
	error( TDEIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
	return;
    }

    totalSize( buff.st_size );
    int processed_size = 0;

    char buffer[ MAX_IPC_SIZE ];
    TQByteArray array;

    while( 1 )
    {
       int n = ::read( fd, buffer, MAX_IPC_SIZE );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          error( TDEIO::ERR_COULD_NOT_READ, url.path());
          close(fd);
          return;
       }
       if (n == 0)
          break; // Finished

       array.setRawData(buffer, n);
       data( array );
       array.resetRawData(buffer, n);

       processed_size += n;
       processedSize( processed_size );
    }

    data( TQByteArray() );

    close( fd );

    processedSize( buff.st_size );

    finished();
}
