#include "resource.h"

namespace Evolution {
    class DBWrapper;
}

namespace TDEABC {
    class ResourceEvolution : public Resource {
    public:
        ResourceEvolution( const TDEConfig* config );
        ~ResourceEvolution();

        bool doOpen();
        void doClose();
        Ticket* requestSaveTicket();
        bool load();
        bool save( Ticket* ticket );
        void removeAddressee( const Addressee& );
    private:
        Evolution::DBWrapper *mWrap;
        bool m_isOpen : 1;
    };
}
