/*****************************************************************

Copyright (c) 2004 Waldo Bastian <bastian@kde.org>

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

******************************************************************
*/

#ifndef _TESTDCOP_H_
#define _TESTDCOP_H_

#include <tdeapplication.h>
#include <dcopclient.h>
#include <dcopobject.h>

#include <tqobject.h>
#include <tqtimer.h>

#include <stdio.h>

class MyDCOPObject : public TQObject, public DCOPObject
{
  TQ_OBJECT
public:
  MyDCOPObject(const TQCString &name, const TQCString &remoteName);
  bool process(const TQCString &fun, const TQByteArray &data,
	       TQCString& replyType, TQByteArray &replyData);
public slots:
  void slotTimeout();

private:
  TQCString m_remoteName;
  TQTimer m_timer;
};
#endif
