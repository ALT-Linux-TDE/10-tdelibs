/*
 *  Copyright (C) 2002 David Faure   <faure@kde.org>
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

#ifndef _kruntest_h
#define _kruntest_h

#include <krun.h>

#include <tqwidget.h>

class testKRun : public KRun
{
  TQ_OBJECT
public:

  testKRun( const KURL& _url, mode_t _mode = 0,
            bool _is_local_file = false, bool _auto_delete = true )
    : KRun( _url, _mode, _is_local_file, _auto_delete ) {}

  virtual ~testKRun() {}

  virtual void foundMimeType( const TQString& _type );

};


class TQPushButton;
class Receiver : public TQWidget
{
  TQ_OBJECT
public:
  Receiver();
  ~Receiver() {}
public slots:
 void slotStart();
 void slotStop();
private:
 TQPushButton * start;
 TQPushButton * stop;

};

#endif
