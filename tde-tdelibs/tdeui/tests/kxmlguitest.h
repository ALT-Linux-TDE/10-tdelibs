#ifndef KXMLGUITEST_H
#define KXMLGUITEST_H

#include <kxmlguiclient.h>
#include <tqobject.h>

class Client : public TQObject, public KXMLGUIClient
{
    TQ_OBJECT
public:
    Client() {}

    void setXMLFile( const TQString &f, bool merge = true ) { KXMLGUIClient::setXMLFile( f, merge ); }
    void setInstance( TDEInstance *inst ) { KXMLGUIClient::setInstance( inst ); }

public slots:
    void slotSec();
};
#endif
