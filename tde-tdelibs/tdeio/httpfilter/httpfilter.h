/* 
   This file is part of the KDE libraries
   Copyright (c) 2002 Waldo Bastian <bastian@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _HTTPFILTER_H_
#define _HTTPFILTER_H_

#include <config.h>

#ifdef HAVE_LIBZ
#define DO_GZIP
#endif

#ifdef DO_GZIP
#include <zlib.h>
#endif

#include <tqobject.h>
#include <kmdcodec.h>

class HTTPFilterBase : public TQObject
{
    TQ_OBJECT
public:
    HTTPFilterBase();
    ~HTTPFilterBase();

    void chain(HTTPFilterBase *previous);

public slots:
    virtual void slotInput(const TQByteArray &d) = 0;
    
signals:
    void output(const TQByteArray &d);
    void error(int, const TQString &);

protected:
    HTTPFilterBase *last;
};

class HTTPFilterChain : public HTTPFilterBase
{
    TQ_OBJECT
public:
    HTTPFilterChain();

    void addFilter(HTTPFilterBase *filter);

public slots:
    void slotInput(const TQByteArray &d);

private:
    HTTPFilterBase *first;
};

class HTTPFilterMD5 : public HTTPFilterBase
{
    TQ_OBJECT
public:
    HTTPFilterMD5();

    TQString md5();

public slots:
    void slotInput(const TQByteArray &d);
    
private:
    KMD5 context;      
};


class HTTPFilterGZip : public HTTPFilterBase
{
    TQ_OBJECT
public:
    HTTPFilterGZip();
    ~HTTPFilterGZip();

public slots:
    void slotInput(const TQByteArray &d);
    
protected:
    int get_byte();
    int checkHeader();
#ifdef DO_GZIP
    z_stream zstr;
    bool bEof : 1;
    bool bHasHeader : 1;
    bool bHasFinished : 1;
    bool bPlainText : 1;
    bool bEatTrailer : 1;
    TQByteArray headerData;
    int iTrailer;
#endif
};

class HTTPFilterDeflate : public HTTPFilterGZip
{
    TQ_OBJECT
public:
    HTTPFilterDeflate();
};

#endif
