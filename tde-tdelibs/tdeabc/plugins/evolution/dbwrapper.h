#ifndef KABC_EVOLUTION_DB_WRAPPER
#define KABC_EVOLUTION_DB_WRAPPER

#include <db.h>

#include <tqstring.h>
#include <tqpair.h>

namespace Evolution {

    class DBWrapper;
    class DBIterator {
        friend class DBWrapper;
    public:
        DBIterator( DBWrapper* = 0l );
        ~DBIterator();

        DBIterator( const DBIterator& );
        DBIterator &operator=( const DBIterator& );

        TQString key()const;
        TQString value()const;

        TQString operator*();

        DBIterator &operator++();
        DBIterator &operator--();

        bool operator==( const DBIterator& );
        bool operator!=( const DBIterator& );
    private:
        struct Data;
        Data* data;
    };
    class DBWrapper {
    public:
        DBWrapper();
        ~DBWrapper();

        TQString lastError()const;

        bool open( const TQString& file, bool readOnly = false);
        bool save();
        DBIterator begin();
        DBIterator end();

        bool find( const TQString& key, TQString& value );
        bool add( const TQString& key,  const TQString& val );
        bool remove( const TQString& key );
    private:
        //  DBT element( const TQString& );
        struct Data;
        Data* data;

    };

}


#endif
