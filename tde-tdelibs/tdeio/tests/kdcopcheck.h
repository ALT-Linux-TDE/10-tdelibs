#ifndef _BLA_H_
#define _BLA_H_

#include <kprocess.h>
#include <tqstring.h>
#include <tqobject.h>

class TestService : public TQObject
{
    TQ_OBJECT
public:
    TestService(const TQString &exec);

    int exec();

public slots:
    void newApp(const TQCString &appId);
    void endApp(const TQCString &appId);
    void appExit();
    void stop();

protected:  
    int result;
    TQString m_exec;
    TDEProcess proc;
};

#endif
