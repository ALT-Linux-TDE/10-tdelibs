#ifndef _main_h
#define _main_h

#include <tqobject.h>
#include <tqstring.h>
#include <tqstrlist.h>
#include <tqtimer.h>

namespace TDEIO { class Job; }

class KIOExec : public TQObject
{
    TQ_OBJECT
public:
    KIOExec();

public slots:
    void slotResult( TDEIO::Job * );
    void slotRunApp();

protected:
    bool tempfiles;
    TQString suggestedFileName;
    int counter;
    int expectedCounter;
    TQString command;
    struct fileInfo {
       TQString path;
       KURL url;
       int time;
    };
    TQValueList<fileInfo> fileList;
};

#endif
