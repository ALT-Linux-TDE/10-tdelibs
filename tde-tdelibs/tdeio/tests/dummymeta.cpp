#include <kgenericfactory.h>

#include "dummymeta.h"

K_EXPORT_COMPONENT_FACTORY( dummymeta, KGenericFactory<DummyMeta> )

DummyMeta::DummyMeta( TQObject *parent, const char *name,
                      const TQStringList &preferredItems )
    : KFilePlugin( parent, name, preferredItems )
{
    tqDebug("---- DummyMeta::DummyMeta: got %i preferred items.", preferredItems.count());
}

bool DummyMeta::readInfo( KFileMetaInfo::Internal & info )
{
   tqDebug("#### DummyMeta:: readInfo: %s", info.path().latin1() );
   return 0L;
}

#include "dummymeta.moc"
