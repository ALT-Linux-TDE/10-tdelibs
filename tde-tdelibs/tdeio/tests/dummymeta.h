#ifndef DUMMYMETA_H
#define DUMMYMETA_H

#include <tdefilemetainfo.h>

class KFileMetaInfo;

class DummyMeta : public KFilePlugin
{
    TQ_OBJECT
    
public:
    DummyMeta( TQObject *parent, const char *name, const TQStringList &args );
    ~DummyMeta() {}

    virtual bool readInfo( KFileMetaInfo::Internal& info );

};

#endif
