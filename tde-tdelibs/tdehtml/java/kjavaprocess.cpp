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

#include "kjavaprocess.h"

#include <kdebug.h>
#include <tdeio/tdeprotocolmanager.h>

#include <tqtextstream.h>
#include <tqmap.h>

#include <config.h>

#include <unistd.h>
#include <tqptrlist.h>

class KJavaProcessPrivate
{
friend class KJavaProcess;
private:
    TQString jvmPath;
    TQString classPath;
    TQString mainClass;
    TQString extraArgs;
    TQString classArgs;
    TQPtrList<TQByteArray> BufferList;
    TQMap<TQString, TQString> systemProps;
    bool processKilled;
};

KJavaProcess::KJavaProcess() : TDEProcess()
{
    d = new KJavaProcessPrivate;
    d->BufferList.setAutoDelete( true );
    d->processKilled = false;

    javaProcess = this; //new TDEProcess();

    connect( javaProcess, TQ_SIGNAL( wroteStdin( TDEProcess * ) ),
             this, TQ_SLOT( slotWroteData() ) );
    connect( javaProcess, TQ_SIGNAL( receivedStdout( int, int& ) ),
             this, TQ_SLOT( slotReceivedData(int, int&) ) );
    connect( javaProcess, TQ_SIGNAL( processExited (TDEProcess *) ),
             this, TQ_SLOT( slotExited (TDEProcess *) ) );

    d->jvmPath = "java";
    d->mainClass = "-help";
}

KJavaProcess::~KJavaProcess()
{
    if ( isRunning() )
    {
        kdDebug(6100) << "stopping java process" << endl;
        stopJava();
    }

    //delete javaProcess;
    delete d;
}

bool KJavaProcess::isRunning()
{
   return javaProcess->isRunning();
}

bool KJavaProcess::startJava()
{
   return invokeJVM();
}

void KJavaProcess::stopJava()
{
   killJVM();
}

void KJavaProcess::setJVMPath( const TQString& path )
{
   d->jvmPath = path;
}

void KJavaProcess::setClasspath( const TQString& classpath )
{
    d->classPath = classpath;
}

void KJavaProcess::setSystemProperty( const TQString& name,
                                      const TQString& value )
{
   d->systemProps.insert( name, value );
}

void KJavaProcess::setMainClass( const TQString& className )
{
   d->mainClass = className;
}

void KJavaProcess::setExtraArgs( const TQString& args )
{
   d->extraArgs = args;
}

void KJavaProcess::setClassArgs( const TQString& args )
{
   d->classArgs = args;
}

//Private Utility Functions used by the two send() methods
TQByteArray* KJavaProcess::addArgs( char cmd_code, const TQStringList& args )
{
    //the buffer to store stuff, etc.
    TQByteArray* const buff = new TQByteArray();
    TQTextOStream output( *buff );
    const char sep = 0;

    //make space for the command size: 8 characters...
    const TQCString space( "        " );
    output << space;

    //write command code
    output << cmd_code;

    //store the arguments...
    if( args.isEmpty() )
    {
        output << sep;
    }
    else
    {
        TQStringList::ConstIterator it = args.begin();
        const TQStringList::ConstIterator itEnd = args.end();
        for( ; it != itEnd; ++it )
        {
            if( !(*it).isEmpty() )
            {
                output << (*it).local8Bit();
            }
            output << sep;
        }
    }

    return buff;
}

void KJavaProcess::storeSize( TQByteArray* buff )
{
    const int size = buff->size() - 8;  //subtract out the length of the size_str
    const TQString size_str = TQString("%1").arg( size, 8 );
    kdDebug(6100) << "KJavaProcess::storeSize, size = " << size_str << endl;

    const char* size_ptr = size_str.latin1();
    for( int i = 0; i < 8; ++i )
        buff->at(i) = size_ptr[i];
}

void KJavaProcess::sendBuffer( TQByteArray* buff )
{
    d->BufferList.append( buff );
    if( d->BufferList.count() == 1)
    {
        popBuffer();
    }
}

void KJavaProcess::send( char cmd_code, const TQStringList& args )
{
    if( isRunning() )
    {
        TQByteArray* const buff = addArgs( cmd_code, args );
        storeSize( buff );
        kdDebug(6100) << "<KJavaProcess::send " << (int)cmd_code << endl;
        sendBuffer( buff );
    }
}

void KJavaProcess::send( char cmd_code, const TQStringList& args,
                         const TQByteArray& data )
{
    if( isRunning() )
    {
        kdDebug(6100) << "KJavaProcess::send, qbytearray is size = " << data.size() << endl;

        TQByteArray* const buff = addArgs( cmd_code, args );
        const int cur_size = buff->size();
        const int data_size = data.size();
        buff->resize( cur_size + data_size );
        memcpy( buff->data() + cur_size, data.data(), data_size );

        storeSize( buff );
        sendBuffer( buff );
    }
}

void KJavaProcess::popBuffer()
{
    TQByteArray* const buf = d->BufferList.first();
    if( buf )
    {
//        DEBUG stuff...
//	kdDebug(6100) << "Sending buffer to java, buffer = >>";
//        for( unsigned int i = 0; i < buf->size(); i++ )
//        {
//            if( buf->at(i) == (char)0 )
//                kdDebug(6100) << "<SEP>";
//            else if( buf->at(i) > 0 && buf->at(i) < 10 )
//                kdDebug(6100) << "<CMD " << (int) buf->at(i) << ">";
//            else
//                kdDebug(6100) << buf->at(i);
//        }
//        kdDebug(6100) << "<<" << endl;

        //write the data
        if ( !javaProcess->writeStdin( buf->data(),
                                       buf->size() ) )
        {
            kdError(6100) << "Could not write command" << endl;
        }
    }
}

void KJavaProcess::slotWroteData( )
{
    //do this here- we can't free the data until we know it went through
    d->BufferList.removeFirst();  //this should delete it since we setAutoDelete(true)
    kdDebug(6100) << "slotWroteData " << d->BufferList.count() << endl;

    if ( !d->BufferList.isEmpty() )
    {
        popBuffer();
    }
}


bool KJavaProcess::invokeJVM()
{
    
    *javaProcess << d->jvmPath;

    if( !d->classPath.isEmpty() )
    {
        *javaProcess << "-classpath";
        *javaProcess << d->classPath;
    }

    //set the system properties, iterate through the qmap of system properties
    TQMap<TQString,TQString>::ConstIterator it = d->systemProps.begin();
    const TQMap<TQString,TQString>::ConstIterator itEnd = d->systemProps.end();

    for( ; it != itEnd; ++it )
    {
        TQString currarg;

        if( !it.key().isEmpty() )
        {
            currarg = "-D" + it.key();
            if( !it.data().isEmpty() )
                currarg += "=" + it.data();
        }

        if( !currarg.isEmpty() )
            *javaProcess << currarg;
    }

    //load the extra user-defined arguments
    if( !d->extraArgs.isEmpty() )
    {
        // BUG HERE: if an argument contains space (-Dname="My name")
        // this parsing will fail. Need more sophisticated parsing -- use KShell?
        const TQStringList args = TQStringList::split( " ", d->extraArgs );
        TQStringList::ConstIterator it = args.begin();
        const TQStringList::ConstIterator itEnd = args.end();
        for ( ; it != itEnd; ++it )
            *javaProcess << *it;
    }

    *javaProcess << d->mainClass;

    if ( !d->classArgs.isNull() )
        *javaProcess << d->classArgs;

    kdDebug(6100) << "Invoking JVM now...with arguments = " << endl;
    TQString argStr;
    TQTextOStream stream( &argStr );
    const TQValueList<TQCString> args = javaProcess->args();
    tqCopy( args.begin(), args.end(), TQTextOStreamIterator<TQCString>( stream, " " ) );
    kdDebug(6100) << argStr << endl;

    TDEProcess::Communication flags =  (TDEProcess::Communication)
                                     (TDEProcess::Stdin | TDEProcess::Stdout |
                                      TDEProcess::NoRead);

    const bool rval = javaProcess->start( TDEProcess::NotifyOnExit, flags );
    if( rval )
        javaProcess->resume(); //start processing stdout on the java process
    else
        killJVM();

    return rval;
}

void KJavaProcess::killJVM()
{
   d->processKilled = true;
   disconnect( javaProcess, TQ_SIGNAL( receivedStdout( int, int& ) ),
               this, TQ_SLOT( slotReceivedData(int, int&) ) );
   javaProcess->kill();
}

void KJavaProcess::flushBuffers()
{
    while ( !d->BufferList.isEmpty() ) {
        if (innot)
            slotSendData(0);
        else
            d->BufferList.removeFirst();  //note: AutoDelete is true
    }
}

/*  In this method, read one command and send it to the d->appletServer
 *  then return, so we don't block the event handling
 */
void KJavaProcess::slotReceivedData( int fd, int& len )
{
    //read out the length of the message,
    //read the message and send it to the applet server
    char length[9] = { 0 };
    const int num_bytes = ::read( fd, length, 8 );
    if( !num_bytes )
    {
        len = 0;
        return;
    }
    if( num_bytes == -1 )
    {
        kdError(6100) << "could not read 8 characters for the message length!!!!" << endl;
        len = 0;
        return;
    }

    const TQString lengthstr( length );
    bool ok;
    const int num_len = lengthstr.toInt( &ok );
    if( !ok )
    {
        kdError(6100) << "could not parse length out of: " << lengthstr << endl;
        len = num_bytes;
        return;
    }

    //now parse out the rest of the message.
    char* const msg = new char[num_len];
    const int num_bytes_msg = ::read( fd, msg, num_len );
    if( num_bytes_msg == -1 || num_bytes_msg != num_len )
    {
        kdError(6100) << "could not read the msg, num_bytes_msg = " << num_bytes_msg << endl;
        delete[] msg;
        len = num_bytes;
        return;
    }

    TQByteArray qb;
    emit received( qb.duplicate( msg, num_len ) );
    delete[] msg;
    len = num_bytes + num_bytes_msg;
}

void KJavaProcess::slotExited( TDEProcess *process )
{
  if (process == javaProcess) {
    int status = -1;
    if (!d->processKilled) {
     status = javaProcess->exitStatus();
    }
    kdDebug(6100) << "jvm exited with status " << status << endl; 
    emit exited(status);
  }
}

#include "kjavaprocess.moc"
