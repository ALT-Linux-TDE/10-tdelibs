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

#include <dcop_deadlock_test.h>
#include <dcopref.h>
#include <tqtimer.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

MyDCOPObject::MyDCOPObject(const TQCString &name, const TQCString &remoteName) 
: TQObject(0, name), DCOPObject(name), m_remoteName(remoteName)
{
  connect(&m_timer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotTimeout()));
}

bool MyDCOPObject::process(const TQCString &fun, const TQByteArray &data,
TQCString& replyType, TQByteArray &replyData)
{
  if (fun == "function(TQCString)") {
    TQDataStream args( data, IO_ReadOnly );
    args >>  m_remoteName;

    struct timeval tv;
    gettimeofday(&tv, 0);
tqWarning("%s: function('%s') %d:%06d", name(), m_remoteName.data(), tv.tv_sec % 100, tv.tv_usec);

    replyType = "TQString";
    TQDataStream reply( replyData, IO_WriteOnly );
    reply << TQString("Hey");
    m_timer.start(1000, true);
    return true;
  }
  return DCOPObject::process(fun, data, replyType, replyData);
}

void MyDCOPObject::slotTimeout()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
tqWarning("%s: slotTimeout() %d:%06d", name(), tv.tv_sec % 100, tv.tv_usec);

  m_timer.start(1000, true);
  TQString result;
  DCOPRef(m_remoteName, m_remoteName).call("function", TQCString(name())).get(result);
    gettimeofday(&tv, 0);
tqWarning("%s: Got result '%s' %d:%06d", name(), result.latin1(), tv.tv_sec % 100, tv.tv_usec);
}

int main(int argc, char **argv)
{
  TQCString myName = TDEApplication::dcopClient()->registerAs("testdcop", false);
  TDEApplication app(argc, argv, "testdcop");

  tqWarning("%d:I am '%s'", getpid(), app.dcopClient()->appId().data());
  
  if (myName == "testdcop")
  {
      system("./dcop_deadlock_test testdcop&");
  }

  TQCString remoteApp;
  if (argc == 2)
  {
      remoteApp = argv[1];
  }
  MyDCOPObject myObject(app.dcopClient()->appId(), remoteApp);

  if (!remoteApp.isEmpty())
     myObject.slotTimeout();
  app.exec();
}

#include "dcop_deadlock_test.moc"
