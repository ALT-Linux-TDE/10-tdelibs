/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
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

#include <config.h>
#include "kjavaappletserver.h"
#include "kjavaappletcontext.h"
#include "kjavaprocess.h"
#include "kjavadownloader.h"

#include <kdebug.h>
#include <tdeconfig.h>
#include <tdelocale.h>
#include <tdeparts/browserextension.h>
#include <tdeapplication.h>
#include <kstandarddirs.h>

#include <tdeio/job.h>
#include <tdeio/tdeprotocolmanager.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>
#include <kssl.h>

#include <tqtimer.h>
#include <tqguardedptr.h>
#include <tqvaluelist.h>
#include <tqptrlist.h>
#include <tqdir.h>
#include <tqeventloop.h>
#include <tqapplication.h>
#include <tqlabel.h>
#include <tqdialog.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqregexp.h>

#include <stdlib.h>
#include <assert.h>

#define KJAS_CREATE_CONTEXT    (char)1
#define KJAS_DESTROY_CONTEXT   (char)2
#define KJAS_CREATE_APPLET     (char)3
#define KJAS_DESTROY_APPLET    (char)4
#define KJAS_START_APPLET      (char)5
#define KJAS_STOP_APPLET       (char)6
#define KJAS_INIT_APPLET       (char)7
#define KJAS_SHOW_DOCUMENT     (char)8
#define KJAS_SHOW_URLINFRAME   (char)9
#define KJAS_SHOW_STATUS       (char)10
#define KJAS_RESIZE_APPLET     (char)11
#define KJAS_GET_URLDATA       (char)12
#define KJAS_URLDATA           (char)13
#define KJAS_SHUTDOWN_SERVER   (char)14
#define KJAS_JAVASCRIPT_EVENT   (char)15
#define KJAS_GET_MEMBER        (char)16
#define KJAS_CALL_MEMBER       (char)17
#define KJAS_PUT_MEMBER        (char)18
#define KJAS_DEREF_OBJECT      (char)19
#define KJAS_AUDIOCLIP_PLAY    (char)20
#define KJAS_AUDIOCLIP_LOOP    (char)21
#define KJAS_AUDIOCLIP_STOP    (char)22
#define KJAS_APPLET_STATE      (char)23
#define KJAS_APPLET_FAILED     (char)24
#define KJAS_DATA_COMMAND      (char)25
#define KJAS_PUT_URLDATA       (char)26
#define KJAS_PUT_DATA          (char)27
#define KJAS_SECURITY_CONFIRM  (char)28
#define KJAS_SHOW_CONSOLE      (char)29


class JSStackFrame;

typedef TQMap< int, KJavaTDEIOJob* > TDEIOJobMap;
typedef TQMap< int, JSStackFrame* > JSStack;

class JSStackFrame {
public:
    JSStackFrame(JSStack & stack, TQStringList & a)
    : jsstack(stack), args(a), ticket(counter++), ready(false), exit (false) {
        jsstack.insert( ticket, this );
    }
    ~JSStackFrame() {
        jsstack.erase( ticket );
    }
    JSStack & jsstack;
    TQStringList & args;
    int ticket;
    bool ready : 1;
    bool exit : 1;
    static int counter;
};

int JSStackFrame::counter = 0;

class KJavaAppletServerPrivate
{
friend class KJavaAppletServer;
private:
   KJavaAppletServerPrivate() : kssl( 0L ) {}
   ~KJavaAppletServerPrivate() {
       delete kssl;
   }
   int counter;
   TQMap< int, TQGuardedPtr<KJavaAppletContext> > contexts;
   TQString appletLabel;
   JSStack jsstack;
   TDEIOJobMap kiojobs;
   bool javaProcessFailed;
   bool useKIO;
   KSSL * kssl;
   //int locked_context;
   //TQValueList<TQByteArray> java_requests;
};

static KJavaAppletServer* self = 0;

KJavaAppletServer::KJavaAppletServer()
{
    d = new KJavaAppletServerPrivate;
    process = new KJavaProcess();

    connect( process, TQ_SIGNAL(received(const TQByteArray&)),
             this,    TQ_SLOT(slotJavaRequest(const TQByteArray&)) );

    setupJava( process );

    if( process->startJava() ) {
        d->appletLabel = i18n( "Loading Applet" );
        d->javaProcessFailed = false;
    }
    else {
        d->appletLabel = i18n( "Error: java executable not found" );
        d->javaProcessFailed = true;
    }
}

KJavaAppletServer::~KJavaAppletServer()
{
    quit();

    delete process;
    delete d;
}

TQString KJavaAppletServer::getAppletLabel()
{
    if( self )
        return self->appletLabel();
    else
        return TQString::null;
}

TQString KJavaAppletServer::appletLabel()
{
    return d->appletLabel;
}

KJavaAppletServer* KJavaAppletServer::allocateJavaServer()
{
   if( self == 0 )
   {
      self = new KJavaAppletServer();
      self->d->counter = 0;
   }

   ++(self->d->counter);
   return self;
}

void KJavaAppletServer::freeJavaServer()
{
    --(self->d->counter);

    if( self->d->counter == 0 )
    {
        //instead of immediately quitting here, set a timer to kill us
        //if there are still no servers- give us one minute
        //this is to prevent repeated loading and unloading of the jvm
        TDEConfig config( "konquerorrc", true );
        config.setGroup( "Java/JavaScript Settings" );
        if( config.readBoolEntry( "ShutdownAppletServer", true )  )
        {
            const int value = config.readNumEntry( "AppletServerTimeout", 60 );
            TQTimer::singleShot( value*1000, self, TQ_SLOT( checkShutdown() ) );
        }
    }
}

void KJavaAppletServer::checkShutdown()
{
    if( self->d->counter == 0 )
    {
        delete self;
        self = 0;
    }
}

void KJavaAppletServer::setupJava( KJavaProcess *p )
{
    TDEConfig config ( "konquerorrc", true );
    config.setGroup( "Java/JavaScript Settings" );

    TQString jvm_path = "java";

    TQString jPath = config.readPathEntry( "JavaPath" );
    if ( !jPath.isEmpty() && jPath != "java" )
    {
        // Cut off trailing slash if any
        if( jPath[jPath.length()-1] == '/' )
            jPath.remove(jPath.length()-1, 1);

        TQDir dir( jPath );
        if( dir.exists( "bin/java" ) )
        {
            jvm_path = jPath + "/bin/java";
        }
        else if (dir.exists( "/jre/bin/java" ) )
        {
            jvm_path = jPath + "/jre/bin/java";
        }
        else if( TQFile::exists(jPath) )
        {
            //check here to see if they entered the whole path the java exe
            jvm_path = jPath;
        }
    }

    //check to see if jvm_path is valid and set d->appletLabel accordingly
    p->setJVMPath( jvm_path );

    // Prepare classpath variable
    TQString kjava_class = locate("data", "kjava/kjava.jar");
    kdDebug(6100) << "kjava_class = " << kjava_class << endl;
    if( kjava_class.isNull() ) // Should not happen
        return;

    TQDir dir( kjava_class );
    dir.cdUp();
    kdDebug(6100) << "dir = " << dir.absPath() << endl;

    const TQStringList entries = dir.entryList( "*.jar" );
    kdDebug(6100) << "entries = " << entries.join( ":" ) << endl;

    TQString classes;
    {
        TQStringList::ConstIterator it = entries.begin();
	const TQStringList::ConstIterator itEnd = entries.end();
        for( ; it != itEnd; ++it )
        {
            if( !classes.isEmpty() )
                classes += ":";
            classes += dir.absFilePath( *it );
        }
    }
    p->setClasspath( classes );

    // Fix all the extra arguments
    const TQString extraArgs = config.readEntry( "JavaArgs" );
    p->setExtraArgs( extraArgs );

    if( config.readBoolEntry( "UseSecurityManager", true ) )
    {
        TQString class_file = locate( "data", "kjava/kjava.policy" );
        p->setSystemProperty( "java.security.policy", class_file );

        p->setSystemProperty( "java.security.manager",
                              "org.kde.kjas.server.KJASSecurityManager" );
    }

    d->useKIO = config.readBoolEntry( "UseKio", false);
    if( d->useKIO )
    {
        p->setSystemProperty( "kjas.useKio", TQString::null );
    }

    //check for http proxies...
    if( KProtocolManager::useProxy() )
    {
        // only proxyForURL honors automatic proxy scripts
        // we do not know the applet url here so we just use a dummy url
        // this is a workaround for now
        // FIXME
        const KURL dummyURL( "http://www.kde.org/" );
        const TQString httpProxy = KProtocolManager::proxyForURL(dummyURL);
        kdDebug(6100) << "httpProxy is " << httpProxy << endl;

        const KURL url( httpProxy );
        p->setSystemProperty( "http.proxyHost", url.host() );
        p->setSystemProperty( "http.proxyPort", TQString::number( url.port() ) );
    }

    //set the main class to run
    p->setMainClass( "org.kde.kjas.server.Main" );
}

void KJavaAppletServer::createContext( int contextId, KJavaAppletContext* context )
{
//    kdDebug(6100) << "createContext: " << contextId << endl;
    if ( d->javaProcessFailed ) return;

    d->contexts.insert( contextId, context );

    TQStringList args;
    args.append( TQString::number( contextId ) );
    process->send( KJAS_CREATE_CONTEXT, args );
}

void KJavaAppletServer::destroyContext( int contextId )
{
//    kdDebug(6100) << "destroyContext: " << contextId << endl;
    if ( d->javaProcessFailed ) return;
    d->contexts.remove( contextId );

    TQStringList args;
    args.append( TQString::number( contextId ) );
    process->send( KJAS_DESTROY_CONTEXT, args );
}

bool KJavaAppletServer::createApplet( int contextId, int appletId,
                             const TQString & name, const TQString & clazzName,
                             const TQString & baseURL, const TQString & user,
                             const TQString & password, const TQString & authname,
                             const TQString & codeBase, const TQString & jarFile,
                             TQSize size, const TQMap<TQString,TQString>& params,
                             const TQString & windowTitle )
{
//    kdDebug(6100) << "createApplet: contextId = " << contextId     << endl
//              << "              appletId  = " << appletId      << endl
//              << "              name      = " << name          << endl
//              << "              clazzName = " << clazzName     << endl
//              << "              baseURL   = " << baseURL       << endl
//              << "              codeBase  = " << codeBase      << endl
//              << "              jarFile   = " << jarFile       << endl
//              << "              width     = " << size.width()  << endl
//              << "              height    = " << size.height() << endl;

    if ( d->javaProcessFailed ) return false;

    TQStringList args;
    args.append( TQString::number( contextId ) );
    args.append( TQString::number( appletId ) );

    //it's ok if these are empty strings, I take care of it later...
    args.append( name );
    args.append( clazzName );
    args.append( baseURL );
    args.append( user );
    args.append( password );
    args.append( authname );
    args.append( codeBase );
    args.append( jarFile );

    args.append( TQString::number( size.width() ) );
    args.append( TQString::number( size.height() ) );

    args.append( windowTitle );

    //add on the number of parameter pairs...
    const int num = params.count();
    const TQString num_params = TQString("%1").arg( num, 8 );
    args.append( num_params );

    TQMap< TQString, TQString >::ConstIterator it = params.begin();
    const TQMap< TQString, TQString >::ConstIterator itEnd = params.end();

    for( ; it != itEnd; ++it )
    {
        args.append( it.key() );
        args.append( it.data() );
    }

    process->send( KJAS_CREATE_APPLET, args );

    return true;
}

void KJavaAppletServer::initApplet( int contextId, int appletId )
{
    if ( d->javaProcessFailed ) return;
    TQStringList args;
    args.append( TQString::number( contextId ) );
    args.append( TQString::number( appletId ) );

    process->send( KJAS_INIT_APPLET, args );
}

void KJavaAppletServer::destroyApplet( int contextId, int appletId )
{
    if ( d->javaProcessFailed ) return;
    TQStringList args;
    args.append( TQString::number(contextId) );
    args.append( TQString::number(appletId) );

    process->send( KJAS_DESTROY_APPLET, args );
}

void KJavaAppletServer::startApplet( int contextId, int appletId )
{
    if ( d->javaProcessFailed ) return;
    TQStringList args;
    args.append( TQString::number(contextId) );
    args.append( TQString::number(appletId) );

    process->send( KJAS_START_APPLET, args );
}

void KJavaAppletServer::stopApplet( int contextId, int appletId )
{
    if ( d->javaProcessFailed ) return;
    TQStringList args;
    args.append( TQString::number(contextId) );
    args.append( TQString::number(appletId) );

    process->send( KJAS_STOP_APPLET, args );
}

void KJavaAppletServer::showConsole() {
    if ( d->javaProcessFailed ) return;
    TQStringList args;
    process->send( KJAS_SHOW_CONSOLE, args );
}

void KJavaAppletServer::sendURLData( int loaderID, int code, const TQByteArray& data )
{
    TQStringList args;
    args.append( TQString::number(loaderID) );
    args.append( TQString::number(code) );

    process->send( KJAS_URLDATA, args, data );
}

void KJavaAppletServer::removeDataJob( int loaderID )
{
    const TDEIOJobMap::iterator it = d->kiojobs.find( loaderID );
    if (it != d->kiojobs.end()) {
        it.data()->deleteLater();
        d->kiojobs.erase( it );
    }
}

void KJavaAppletServer::quit()
{
    const TQStringList args;

    process->send( KJAS_SHUTDOWN_SERVER, args );
    process->flushBuffers();
    process->wait( 10 );
}

void KJavaAppletServer::slotJavaRequest( const TQByteArray& qb )
{
    // qb should be one command only without the length string,
    // we parse out the command and it's meaning here...
    TQString cmd;
    TQStringList args;
    int index = 0;
    const int qb_size = qb.size();

    //get the command code
    const char cmd_code = qb[ index++ ];
    ++index; //skip the next sep

    //get contextID
    TQString contextID;
    while( qb[index] != 0 && index < qb_size )
    {
        contextID += qb[ index++ ];
    }
    bool ok;
    const int ID_num = contextID.toInt( &ok ); // context id or tdeio job id
    /*if (d->locked_context > -1 &&
        ID_num != d->locked_context &&
        (cmd_code == KJAS_JAVASCRIPT_EVENT ||
         cmd_code == KJAS_APPLET_STATE ||
         cmd_code == KJAS_APPLET_FAILED))
    {
        / * Don't allow requests from other contexts if we're waiting
         * on a return value that can trigger JavaScript events
         * /
        d->java_requests.push_back(qb);
        return;
    }*/
    ++index; //skip the sep

    if (cmd_code == KJAS_PUT_DATA) {
        // rest of the data is for tdeio put
        if (ok) {
            TDEIOJobMap::iterator it = d->kiojobs.find( ID_num );
            if (ok && it != d->kiojobs.end()) {
                TQByteArray qba;
                qba.setRawData(qb.data() + index, qb.size() - index - 1);
                it.data()->data(qba);
                qba.resetRawData(qb.data() + index, qb.size() - index - 1);
            }
            kdDebug(6100) << "PutData(" << ID_num << ") size=" << qb.size() - index << endl;
        } else
            kdError(6100) << "PutData error " << ok << endl;
        return;
    }
    //now parse out the arguments
    while( index < qb_size )
    {
        int sep_pos = qb.find( 0, index );
        if (sep_pos < 0) {
            kdError(6100) << "Missing separation byte" << endl;
            sep_pos = qb_size;
        }
        //kdDebug(6100) << "KJavaAppletServer::slotJavaRequest: "<< TQString::fromLocal8Bit( qb.data() + index, sep_pos - index ) << endl;
        args.append( TQString::fromLocal8Bit( qb.data() + index, sep_pos - index ) );
        index = sep_pos + 1; //skip the sep
    }
    //here I should find the context and call the method directly
    //instead of emitting signals
    switch( cmd_code )
    {
        case KJAS_SHOW_DOCUMENT:
            cmd = TQString::fromLatin1( "showdocument" );
            break;

        case KJAS_SHOW_URLINFRAME:
            cmd = TQString::fromLatin1( "showurlinframe" );
            break;

        case KJAS_SHOW_STATUS:
            cmd = TQString::fromLatin1( "showstatus" );
            break;

        case KJAS_RESIZE_APPLET:
            cmd = TQString::fromLatin1( "resizeapplet" );
            break;

        case KJAS_GET_URLDATA:
            if (ok && !args.empty() ) {
                d->kiojobs.insert(ID_num, new KJavaDownloader(ID_num, args.first()));
                kdDebug(6100) << "GetURLData(" << ID_num << ") url=" << args.first() << endl;
            } else
                kdError(6100) << "GetURLData error " << ok << " args:" << args.size() << endl;
            return;
        case KJAS_PUT_URLDATA:
            if (ok && !args.empty()) {
                KJavaUploader* const job = new KJavaUploader(ID_num, args.first());
                d->kiojobs.insert(ID_num, job);
                job->start();
                kdDebug(6100) << "PutURLData(" << ID_num << ") url=" << args.first() << endl;
            } else
                kdError(6100) << "PutURLData error " << ok << " args:" << args.size() << endl;
            return;
        case KJAS_DATA_COMMAND:
            if (ok && !args.empty()) {
                const int cmd = args.first().toInt( &ok );
                TDEIOJobMap::iterator it = d->kiojobs.find( ID_num );
                if (ok && it != d->kiojobs.end())
                    it.data()->jobCommand( cmd );
                kdDebug(6100) << "TDEIO Data command: " << ID_num << " " << args.first() << endl;
            } else
                kdError(6100) << "TDEIO Data command error " << ok << " args:" << args.size() << endl;
            return;
        case KJAS_JAVASCRIPT_EVENT:
            cmd = TQString::fromLatin1( "JS_Event" );
            kdDebug(6100) << "Javascript request: "<< contextID
                          << " code: " << args[0] << endl;
            break;
        case KJAS_GET_MEMBER:
        case KJAS_PUT_MEMBER:
        case KJAS_CALL_MEMBER: {
            const int ticket = args[0].toInt();
            JSStack::iterator it = d->jsstack.find(ticket);
            if (it != d->jsstack.end()) {
                kdDebug(6100) << "slotJavaRequest: " << ticket << endl;
                args.pop_front();
                it.data()->args.operator=(args); // just in case ..
                it.data()->ready = true;
                it.data()->exit = true;
            } else
                kdDebug(6100) << "Error: Missed return member data" << endl;
            return;
        }
        case KJAS_AUDIOCLIP_PLAY:
            cmd = TQString::fromLatin1( "audioclip_play" );
            kdDebug(6100) << "Audio Play: url=" << args[0] << endl;
            break;
        case KJAS_AUDIOCLIP_LOOP:
            cmd = TQString::fromLatin1( "audioclip_loop" );
            kdDebug(6100) << "Audio Loop: url=" << args[0] << endl;
            break;
        case KJAS_AUDIOCLIP_STOP:
            cmd = TQString::fromLatin1( "audioclip_stop" );
            kdDebug(6100) << "Audio Stop: url=" << args[0] << endl;
            break;
        case KJAS_APPLET_STATE:
            kdDebug(6100) << "Applet State Notification for Applet " << args[0] << ". New state=" << args[1] << endl;
            cmd = TQString::fromLatin1( "AppletStateNotification" );
            break;
        case KJAS_APPLET_FAILED:
            kdDebug(6100) << "Applet " << args[0] << " Failed: " << args[1] << endl;
            cmd = TQString::fromLatin1( "AppletFailed" );
            break;
        case KJAS_SECURITY_CONFIRM: {
            if (KSSL::doesSSLWork() && !d->kssl)
                d->kssl = new KSSL;
            TQStringList sl;
            TQCString answer( "invalid" );

            if (!d->kssl) {
                answer = "nossl";
            } else if (args.size() > 2) {
                const int certsnr = args[1].toInt();
                TQString text;
                TQPtrList<KSSLCertificate> certs;
                certs.setAutoDelete( true );
                for (int i = certsnr; i >= 0; --i) {
                    KSSLCertificate * cert = KSSLCertificate::fromString(args[i+2].ascii());
                    if (cert) {
                        certs.prepend(cert);
                        if (cert->isSigner())
                            text += i18n("Signed by (validation: ");
                        else
                            text += i18n("Certificate (validation: ");
                        switch (cert->validate()) {
                            case KSSLCertificate::Ok:
                                text += i18n("Ok"); break;
                            case KSSLCertificate::NoCARoot:
                                text += i18n("NoCARoot"); break;
                            case KSSLCertificate::InvalidPurpose:
                                text += i18n("InvalidPurpose"); break;
                            case KSSLCertificate::PathLengthExceeded:
                                text += i18n("PathLengthExceeded"); break;
                            case KSSLCertificate::InvalidCA:
                                text += i18n("InvalidCA"); break;
                            case KSSLCertificate::Expired:
                                text += i18n("Expired"); break;
                            case KSSLCertificate::SelfSigned:
                                text += i18n("SelfSigned"); break;
                            case KSSLCertificate::ErrorReadingRoot:
                                text += i18n("ErrorReadingRoot"); break;
                            case KSSLCertificate::Revoked:
                                text += i18n("Revoked"); break;
                            case KSSLCertificate::Untrusted:
                                text += i18n("Untrusted"); break;
                            case KSSLCertificate::SignatureFailed:
                                text += i18n("SignatureFailed"); break;
                            case KSSLCertificate::Rejected:
                                text += i18n("Rejected"); break;
                            case KSSLCertificate::PrivateKeyFailed:
                                text += i18n("PrivateKeyFailed"); break;
                            case KSSLCertificate::InvalidHost:
                                text += i18n("InvalidHost"); break;
                            case KSSLCertificate::Unknown:
                            default:
                                text += i18n("Unknown"); break;
                        }
                        text += TQString(")\n");
                        TQString subject = cert->getSubject() + TQChar('\n');
                        TQRegExp reg(TQString("/[A-Z]+="));
                        int pos = 0;
                        while ((pos = subject.find(reg, pos)) > -1)
                            subject.replace(pos, 1, TQString("\n    "));
                        text += subject.mid(1);
                    }
                }
                kdDebug(6100) << "Security confirm " << args.first() << certs.count() << endl;
		if ( !certs.isEmpty() ) {
                    KSSLCertChain chain;
                    chain.setChain( certs );
                    if ( chain.isValid() )
                        answer = PermissionDialog( tqApp->activeWindow() ).exec( text, args[0] );
                }
            }
            sl.push_front( TQString(answer) );
            sl.push_front( TQString::number(ID_num) );
            process->send( KJAS_SECURITY_CONFIRM, sl );
            return;
        }
        default:
            return;
            break;
    }


    if( !ok )
    {
        kdError(6100) << "could not parse out contextID to call command on" << endl;
        return;
    }

    KJavaAppletContext* const context = d->contexts[ ID_num ];
    if( context )
        context->processCmd( cmd, args );
    else if (cmd != "AppletStateNotification")
        kdError(6100) << "no context object for this id" << endl;
}

void KJavaAppletServer::endWaitForReturnData() {
    kdDebug(6100) << "KJavaAppletServer::endWaitForReturnData" << endl;
    killTimers();
    JSStack::iterator it = d->jsstack.begin();
    JSStack::iterator itEnd = d->jsstack.end();
    for (; it != itEnd; ++it)
        it.data()->exit = true;
}

void KJavaAppletServer::timerEvent(TQTimerEvent *) {
    endWaitForReturnData();
    kdDebug(6100) << "KJavaAppletServer::timerEvent timeout" << endl;
}

void KJavaAppletServer::waitForReturnData(JSStackFrame * frame) {
    kdDebug(6100) << ">KJavaAppletServer::waitForReturnData" << endl;
    killTimers();
    startTimer(15000);
    while (!frame->exit)
        kapp->eventLoop()->processEvents (TQEventLoop::AllEvents | TQEventLoop::WaitForMore);
    if (d->jsstack.size() <= 1)
        killTimers();
    kdDebug(6100) << "<KJavaAppletServer::waitForReturnData stacksize:" << d->jsstack.size() << endl;
}

bool KJavaAppletServer::getMember(TQStringList & args, TQStringList & ret_args) {
    JSStackFrame frame( d->jsstack, ret_args );
    args.push_front( TQString::number(frame.ticket) );

    process->send( KJAS_GET_MEMBER, args );
    waitForReturnData( &frame );

    return frame.ready;
}

bool KJavaAppletServer::putMember( TQStringList & args ) {
    TQStringList ret_args;
    JSStackFrame frame( d->jsstack, ret_args );
    args.push_front( TQString::number(frame.ticket) );

    process->send( KJAS_PUT_MEMBER, args );
    waitForReturnData( &frame );

    return frame.ready && ret_args.count() > 0 && ret_args[0].toInt();
}

bool KJavaAppletServer::callMember(TQStringList & args, TQStringList & ret_args) {
    JSStackFrame frame( d->jsstack, ret_args );
    args.push_front( TQString::number(frame.ticket) );

    process->send( KJAS_CALL_MEMBER, args );
    waitForReturnData( &frame );

    return frame.ready;
}

void KJavaAppletServer::derefObject( TQStringList & args ) {
    process->send( KJAS_DEREF_OBJECT, args );
}

bool KJavaAppletServer::usingKIO() {
    return d->useKIO;
}


PermissionDialog::PermissionDialog( TQWidget* parent )
    : TQObject(parent), m_button("no")
{}

TQCString PermissionDialog::exec( const TQString & cert, const TQString & perm ) {
    TQGuardedPtr<TQDialog> dialog = new TQDialog( static_cast<TQWidget*>(parent()), "PermissionDialog");

    dialog->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)1, 0, 0, dialog->sizePolicy().hasHeightForWidth() ) );
    dialog->setModal( true );
    dialog->setCaption( i18n("Security Alert") );

    TQVBoxLayout* const dialogLayout = new TQVBoxLayout( dialog, 11, 6, "dialogLayout");

    dialogLayout->addWidget( new TQLabel( i18n("Do you grant Java applet with certificate(s):"), dialog ) );
    dialogLayout->addWidget( new TQLabel( cert, dialog, "message" ) );
    dialogLayout->addWidget( new TQLabel( i18n("the following permission"), dialog, "message" ) );
    dialogLayout->addWidget( new TQLabel( perm, dialog, "message" ) );
    TQSpacerItem* const spacer2 = new TQSpacerItem( 20, 40, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    dialogLayout->addItem( spacer2 );

    TQHBoxLayout* const buttonLayout = new TQHBoxLayout( 0, 0, 6, "buttonLayout");

    TQPushButton* const no = new TQPushButton( i18n("&No"), dialog, "no" );
    no->setDefault( true );
    buttonLayout->addWidget( no );

    TQPushButton* const reject = new TQPushButton( i18n("&Reject All"), dialog, "reject" );
    buttonLayout->addWidget( reject );

    TQPushButton* const yes = new TQPushButton( i18n("&Yes"), dialog, "yes" );
    buttonLayout->addWidget( yes );

    TQPushButton* const grant = new TQPushButton( i18n("&Grant All"), dialog, "grant" );
    buttonLayout->addWidget( grant );
    dialogLayout->addLayout( buttonLayout );
    dialog->resize( dialog->minimumSizeHint() );
    //clearWState( WState_Polished );

    connect( no, TQ_SIGNAL( clicked() ), this, TQ_SLOT( clicked() ) );
    connect( reject, TQ_SIGNAL( clicked() ), this, TQ_SLOT( clicked() ) );
    connect( yes, TQ_SIGNAL( clicked() ), this, TQ_SLOT( clicked() ) );
    connect( grant, TQ_SIGNAL( clicked() ), this, TQ_SLOT( clicked() ) );

    dialog->exec();
    delete dialog;

    return m_button;
}

PermissionDialog::~PermissionDialog()
{}

void PermissionDialog::clicked()
{
    m_button = sender()->name();
    static_cast<const TQWidget*>(sender())->parentWidget()->close();
}

#include "kjavaappletserver.moc"
