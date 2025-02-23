/*
 *  Copyright (C) 2002, 2003 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kfilterdev.h"
#include "kfilterbase.h"
#include <unistd.h>
#include <limits.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <kdebug.h>
#include <kinstance.h>

void test_block( const TQString & fileName )
{
    TQIODevice * dev = KFilterDev::deviceForFile( fileName );
    if (!dev) { kdWarning() << "dev=0" << endl; return; }
    if ( !dev->open( IO_ReadOnly ) ) { kdWarning() << "open failed " << endl; return; }

    // This is what KGzipDev::readAll could do, if TQIODevice::readAll was virtual....

    TQByteArray array(1024);
    int n;
    while ( ( n = dev->readBlock( array.data(), array.size() ) ) )
    {
        kdDebug() << "readBlock returned " << n << endl << endl;
        // TQCString s(array,n+1); // Terminate with 0 before printing
        // printf("%s", s.data());

        kdDebug() << "dev.at = " << dev->at() << endl;
        //kdDebug() << "f.at = " << f.at() << endl;
    }
    dev->close();
    delete dev;
}

void test_block_write( const TQString & fileName )
{
    TQIODevice * dev = KFilterDev::deviceForFile( fileName );
    if (!dev) { kdWarning() << "dev=0" << endl; return; }
    if ( !dev->open( IO_WriteOnly ) ) { kdWarning() << "open failed " << endl; return; }

    TQCString s("hello\n");
    int ret = dev->writeBlock( s, s.size()-1 );
    kdDebug() << "writeBlock ret=" << ret << endl;
    //ret = dev->writeBlock( s, s.size()-1 );
    //kdDebug() << "writeBlock ret=" << ret << endl;
    dev->close();
    delete dev;
}

void test_getch( const TQString & fileName )
{
    TQIODevice * dev = KFilterDev::deviceForFile( fileName );
    if (!dev) { kdWarning() << "dev=0" << endl; return; }
    if ( !dev->open( IO_ReadOnly ) ) { kdWarning() << "open failed " << endl; return; }
    int ch;
    while ( ( ch = dev->getch() ) != -1 )
        printf("%c",ch);
    dev->close();
    delete dev;
}

void test_textstream(  const TQString & fileName )
{
    TQIODevice * dev = KFilterDev::deviceForFile( fileName );
    if (!dev) { kdWarning() << "dev=0" << endl; return; }
    if ( !dev->open( IO_ReadOnly ) ) { kdWarning() << "open failed " << endl; return; }
    TQTextStream ts( dev );
    printf("%s\n", ts.read().latin1());
    dev->close();
    delete dev;
}

int main()
{
    TDEInstance instance("kfiltertest");

    char currentdir[PATH_MAX+1];
    getcwd( currentdir, PATH_MAX );
    TQString pathgz = TQFile::decodeName(currentdir) + "/test.gz";
    TQString pathbz2 = TQFile::decodeName(currentdir) + "/test.bz2";
    TQString pathxz = TQFile::decodeName(currentdir) + "/test.xz";

    kdDebug() << " -- test_block_write gzip -- " << endl;
    test_block_write(pathgz);
    kdDebug() << " -- test_block_write bzip2 -- " << endl;
    test_block_write(pathbz2);
    kdDebug() << " -- test_block_write xz -- " << endl;
    test_block_write(pathxz);

    kdDebug() << " -- test_block gzip -- " << endl;
    test_block(pathgz);
    kdDebug() << " -- test_getch gzip -- " << endl;
    test_getch(pathgz);
    kdDebug() << " -- test_textstream gzip -- " << endl;
    test_textstream(pathgz);

    kdDebug() << " -- test_block bzip2 -- " << endl;
    test_block(pathbz2);
    kdDebug() << " -- test_getch bzip2 -- " << endl;
    test_getch(pathbz2);
    kdDebug() << " -- test_textstream bzip2 -- " << endl;
    test_textstream(pathbz2);

    kdDebug() << " -- test_block xz -- " << endl;
    test_block(pathxz);
    kdDebug() << " -- test_getch xz -- " << endl;
    test_getch(pathxz);
    kdDebug() << " -- test_textstream xz -- " << endl;
    test_textstream(pathxz);

    return 0;
}
