#ifndef KIDLTEST_H
#define KIDLTEST_H

#include <dcopobject.h>

class KIDLTest : virtual public DCOPObject
{
    K_DCOP
public:
    KIDLTest( const TQCString& id );

k_dcop:
    virtual TQString hello( const TQString& name );
};

#endif
