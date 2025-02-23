/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001-2002 Rolf Magnus <ramagnus@kde.org>
 *  Copyright (C) 2001-2002 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation version 2.0.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 *  $Id$
 */

#include <assert.h>

#include <tqshared.h>
#include <tqdict.h>

#include <ktrader.h>
#include <kstaticdeleter.h>
#include <tdeparts/componentfactory.h>
#include <kuserprofile.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kdatastream.h> // needed for serialization of bool
#include <tdelocale.h>
#include <tdeio/global.h>

#include "tdefilemetainfo.h"

// shared data of a KFileMetaInfoItem
class KFileMetaInfoItem::Data : public TQShared
{
public:
    Data( const KFileMimeTypeInfo::ItemInfo* mti, const TQString& _key,
          const TQVariant& _value )
        : TQShared(),
          mimeTypeInfo( mti ),
          key( _key ),
          value( _value ),
          dirty( false ),
          added( false ),
          removed( false )
    {}

    // we use this one for the streaming operators
    Data() : mimeTypeInfo( 0L )
    {}

    ~Data()
    {
        if ( this == null ) // only the null item owns its mimeTypeInfo
            delete mimeTypeInfo;
    }

    const KFileMimeTypeInfo::ItemInfo*  mimeTypeInfo;
    // mimeTypeInfo has the key, too, but only for non-variable ones
    TQString                             key;
    TQVariant                            value;
    bool                                dirty    :1;
    bool                                added    :1;
    bool                                removed  :1;

    static Data* null;
    static Data* makeNull();
};

//this is our null data
KFileMetaInfoItem::Data* KFileMetaInfoItem::Data::null = 0L;
static KStaticDeleter<KFileMetaInfoItem::Data> sd_KFileMetaInfoItemData;

KFileMetaInfoItem::Data* KFileMetaInfoItem::Data::makeNull()
{
    if (!null)
    {
        // We deliberately do not reset "null" after it has been destroyed!
        // Otherwise we will run into problems later in ~KFileMetaInfoItem
        // where the d-pointer is compared against null.

        KFileMimeTypeInfo::ItemInfo* info = new KFileMimeTypeInfo::ItemInfo();
        null = new Data(info, TQString::null, TQVariant());
        sd_KFileMetaInfoItemData.setObject( null );
    }
    return null;
}

KFileMetaInfoItem::KFileMetaInfoItem( const KFileMimeTypeInfo::ItemInfo* mti,
                                      const TQString& key, const TQVariant& value )
    : d( new Data( mti, key, value ) )
{
}

KFileMetaInfoItem::KFileMetaInfoItem( const KFileMetaInfoItem& item )
{
    // operator= does everything that's necessary
    d = Data::makeNull();
    *this = item;
}

KFileMetaInfoItem::KFileMetaInfoItem()
{
    d = Data::makeNull();
}

KFileMetaInfoItem::~KFileMetaInfoItem()
{
    deref();
}

const KFileMetaInfoItem& KFileMetaInfoItem::operator=
                                              (const KFileMetaInfoItem & item )
{
    if (d != item.d)
    {
        // first deref the old one
        deref();
        d = item.d;
        // and now ref the new one
        ref();
    }

    return *this;
}

bool KFileMetaInfoItem::setValue( const TQVariant& value )
{
    // We don't call makeNull here since it isn't necassery, see deref()
    if ( d == Data::null ) return false;

    if ( ! (d->mimeTypeInfo->attributes() & KFileMimeTypeInfo::Modifiable ) ||
         ! (value.canCast(d->mimeTypeInfo->type())))
    {
        kdDebug(7033) << "setting the value of " << key() << "failed\n";
        return false;
    }

//    kdDebug(7033) << key() << ".setValue()\n";

    if ( d->value == value )
        return true;

    d->dirty = true;
    d->value = value;
    // If we don't cast (and test for canCast in the above if), TQVariant is
    // very picky about types (e.g. TQString vs. TQCString or int vs. uint)
    d->value.cast(d->mimeTypeInfo->type());

    return true;
}

bool KFileMetaInfoItem::isRemoved() const
{
    return d->removed;
}

TQString KFileMetaInfoItem::key() const
{
    return d->key;
}

TQString KFileMetaInfoItem::translatedKey() const
{
    // are we a variable key?
    if (d->mimeTypeInfo->key().isNull())
    {
        // then try if we have luck with i18n()
        return i18n(d->key.utf8());
    }

    return d->mimeTypeInfo->translatedKey();
}

const TQVariant& KFileMetaInfoItem::value() const
{
    return d->value;
}

TQString KFileMetaInfoItem::string( bool mangle ) const
{
    return d->mimeTypeInfo->string(d->value, mangle);
}

TQVariant::Type KFileMetaInfoItem::type() const
{
    return d->mimeTypeInfo->type();
}

uint KFileMetaInfoItem::unit() const
{
    return d->mimeTypeInfo->unit();
}

bool KFileMetaInfoItem::isModified() const
{
    return d->dirty;
}

TQString KFileMetaInfoItem::prefix() const
{
    return d->mimeTypeInfo->prefix();
}

TQString KFileMetaInfoItem::suffix() const
{
    return d->mimeTypeInfo->suffix();
}

uint KFileMetaInfoItem::hint() const
{
    return d->mimeTypeInfo->hint();
}

uint KFileMetaInfoItem::attributes() const
{
    return d->mimeTypeInfo->attributes();
}

bool KFileMetaInfoItem::isEditable() const
{
    return d->mimeTypeInfo->attributes() & KFileMimeTypeInfo::Modifiable;
}

bool KFileMetaInfoItem::isValid() const
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    return d != Data::null;
}

void KFileMetaInfoItem::setAdded()
{
    d->added = true;
}

void KFileMetaInfoItem::setRemoved()
{
    d->removed = true;
}

void KFileMetaInfoItem::ref()
{
    if (d != Data::null) d->ref();
}

void KFileMetaInfoItem::deref()
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    if ((d != Data::null) && d->deref())
    {
//        kdDebug(7033) << "item " << d->key
//                      << " is finally deleted\n";
        delete d;
        d = 0;
    }
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

// shared data of a KFileMetaInfo
class KFileMetaInfo::Data : public TQShared
{
public:
    Data(const KURL& _url, uint _what)
        : TQShared(),
          url(_url),
          what(_what),
          mimeTypeInfo( 0L )
    {}

    // wee use this one for the streaming operators
    Data() {};

    KURL                              url;
    uint                              what;
    TQMap<TQString, KFileMetaInfoGroup> groups;
    const KFileMimeTypeInfo*          mimeTypeInfo;
    TQStringList                       removedGroups;

    static Data* null;
    static Data* makeNull();

};

KFileMetaInfo::KFileMetaInfo( const TQString& path, const TQString& mimeType,
                              uint what )
{
    KURL u;

    u.setPath(path);
    init(u, mimeType, what);
}

KFileMetaInfo::KFileMetaInfo( const KURL& url, const TQString& mimeType,
                              uint what )
{
    init(url, mimeType, what);
}

void KFileMetaInfo::init( const KURL& url, const TQString& mimeType,
                          uint what )
{
    d = new Data( url, what );

    TQString mT;
    if (mimeType.isEmpty())
        mT = KMimeType::findByURL(url)->name();
    else
        mT = mimeType;

    // let's "share our property"
    KFileMetaInfo item(*this);

    //kdDebug() << k_funcinfo << mT << " " << url << endl;

    d->mimeTypeInfo = KFileMetaInfoProvider::self()->mimeTypeInfo( mT, url.protocol() );
    if ( d->mimeTypeInfo )
    {
        //kdDebug(7033) << "Found mimetype info for " << mT /* or protocol*/ << endl;
        KFilePlugin *p = plugin();
        Q_ASSERT( p );
        if ( p && !p->readInfo( item, what) )
        {
            deref();
            d = Data::makeNull();
        }
    }
    else
    {
//        kdDebug(7033) << "No mimetype info for " << mimeType << endl;
        deref();
        d = Data::makeNull();
    }
}

KFileMetaInfo::KFileMetaInfo( const KFileMetaInfo& original )
{
    // operator= does everything that's necessary
    d = Data::makeNull();
    *this = original;
}

KFileMetaInfo::KFileMetaInfo()
{
    d = Data::makeNull();
}

KFileMetaInfo::~KFileMetaInfo()
{
    deref();
}

TQStringList KFileMetaInfo::supportedGroups() const
{
    assert(isValid());
    return d->mimeTypeInfo->supportedGroups();
}

TQStringList KFileMetaInfo::supportedKeys() const
{
    assert(isValid());
    return d->mimeTypeInfo->supportedKeys();
}

TQStringList KFileMetaInfo::groups() const
{
    TQStringList list;
    TQMapConstIterator<TQString, KFileMetaInfoGroup> it = d->groups.begin();
    for ( ; it != d->groups.end(); ++it )
        list += (*it).name();

    return list;
}

TQStringList KFileMetaInfo::editableGroups() const
{
    TQStringList list;
    TQStringList supported = supportedGroups();
    TQStringList::ConstIterator it = supported.begin();
    for ( ; it != supported.end(); ++it ) {
        const KFileMimeTypeInfo::GroupInfo * groupInfo = d->mimeTypeInfo->groupInfo( *it );
        if ( groupInfo && groupInfo->attributes() &
             (KFileMimeTypeInfo::Addable | KFileMimeTypeInfo::Removable) )
            list.append( *it );
    }

    return list;
}

TQStringList KFileMetaInfo::preferredGroups() const
{
    assert(isValid());
    TQStringList list = groups();
    TQStringList newlist;
    TQStringList preferred = d->mimeTypeInfo->preferredGroups();
    TQStringList::Iterator pref;

    // move all keys from the preferred groups that are in our list to a new list
    for ( pref = preferred.begin(); pref != preferred.end(); ++pref )
    {
        TQStringList::Iterator group = list.find(*pref);
        if ( group != list.end() )
        {
             newlist.append( *group );
             list.remove(group);
        }
    }

    // now the old list only contains the non-preferred items, so we
    // add the remaining ones to newlist
    newlist += list;

    return newlist;
}

TQStringList KFileMetaInfo::preferredKeys() const
{
    TQStringList newlist;

    TQStringList list = preferredGroups();
    for (TQStringList::Iterator git = list.begin(); git != list.end(); ++git)
    {
        newlist += d->groups[*git].preferredKeys();
    }

    return newlist;
}

KFileMetaInfoGroup KFileMetaInfo::group(const TQString& key) const
{
    TQMapIterator<TQString,KFileMetaInfoGroup> it = d->groups.find( key );
    if ( it != d->groups.end() )
        return it.data();
    else
        return KFileMetaInfoGroup();
}

bool KFileMetaInfo::addGroup( const TQString& name )
{
    assert(isValid());
    if ( d->mimeTypeInfo->supportedGroups().contains(name) &&
         ! d->groups.contains(name) )
    {
        KFileMetaInfoGroup group( name, d->mimeTypeInfo );

        // add all the items that can't be added by the user later
        const KFileMimeTypeInfo::GroupInfo* ginfo = d->mimeTypeInfo->groupInfo(name);
        Q_ASSERT(ginfo);
        if (!ginfo) return false;

        TQStringList keys = ginfo->supportedKeys();
        for (TQStringList::Iterator it = keys.begin(); it != keys.end(); ++it)
        {
            const KFileMimeTypeInfo::ItemInfo* iteminfo = ginfo->itemInfo(*it);
            Q_ASSERT(ginfo);
            if (!iteminfo) return false;

            if ( !(iteminfo->attributes() & KFileMimeTypeInfo::Addable) &&
                  (iteminfo->attributes() & KFileMimeTypeInfo::Modifiable))
            {
                // append it now or never
                group.appendItem(iteminfo->key(), TQVariant());
            }

        }

        d->groups.insert(name, group);
        group.setAdded();
        return true;
    }

    return false;
}

bool KFileMetaInfo::removeGroup( const TQString& name )
{
    TQMapIterator<TQString, KFileMetaInfoGroup> it = d->groups.find(name);
    if ( (it==d->groups.end()) ||
        !((*it).attributes() & KFileMimeTypeInfo::Removable))
        return false;

    d->groups.remove(it);
    d->removedGroups.append(name);
    return true;
}

TQStringList KFileMetaInfo::removedGroups()
{
    return d->removedGroups;
}

const KFileMetaInfo& KFileMetaInfo::operator= (const KFileMetaInfo& info )
{
    if (d != info.d)
    {
        deref();
        // first deref the old one
        d = info.d;
        // and now ref the new one
        ref();
    }
    return *this;
}

bool KFileMetaInfo::isValid() const
{
    // We don't call makeNull here since it isn't necassery, see deref()
    return d != Data::null;
}

bool KFileMetaInfo::isEmpty() const
{
    for (TQMapIterator<TQString, KFileMetaInfoGroup> it = d->groups.begin();
         it!=d->groups.end(); ++it)
        if (!(*it).isEmpty())
            return false;
    return true;
}

bool KFileMetaInfo::applyChanges()
{
    return applyChanges( path() );
}

bool KFileMetaInfo::applyChanges( const TQString& path )
{
    bool doit = false;

//    kdDebug(7033) << "KFileMetaInfo::applyChanges()\n";

    // look up if we need to write to the file
    TQMapConstIterator<TQString, KFileMetaInfoGroup> it;
    for (it = d->groups.begin(); it!=d->groups.end() && !doit; ++it)
    {
        if ( (*it).isModified() )
            doit = true;

        else
        {
            TQStringList keys = it.data().keys();
            for (TQStringList::Iterator it2 = keys.begin(); it2!=keys.end(); ++it2)
            {
                if ( (*it)[*it2].isModified() )
                {
                    doit = true;
                    break;
                }
            }
        }
    }

    if (!doit)
    {
        kdDebug(7033) << "Don't need to write, nothing changed\n";
        return true;
    }

    KFilePlugin* p = plugin();
    if (!p) return false;

//    kdDebug(7033) << "Ok, trying to write the info\n";

    KURL savedURL = url();
    d->url = KURL();
    d->url.setPath( path );
    
    bool ret = p->writeInfo(*this);
    
    d->url = savedURL;
    return ret;
}

KFilePlugin * KFileMetaInfo::plugin() const
{
    assert(isValid());
    KFileMetaInfoProvider* prov = KFileMetaInfoProvider::self();
    return prov->plugin( d->mimeTypeInfo->mimeType(), d->url.protocol() );
}

TQString KFileMetaInfo::mimeType() const
{
    assert(isValid());
    return d->mimeTypeInfo->mimeType();
}

bool KFileMetaInfo::contains(const TQString& key) const
{
    TQStringList glist = groups();
    for (TQStringList::Iterator it = glist.begin(); it != glist.end(); ++it)
    {
        KFileMetaInfoGroup g = d->groups[*it];
        if (g.contains(key)) return true;
    }
    return false;
}

bool KFileMetaInfo::containsGroup(const TQString& key) const
{
    return groups().contains(key);
}

KFileMetaInfoItem KFileMetaInfo::item( const TQString& key) const
{
    TQStringList groups = preferredGroups();
    for (TQStringList::Iterator it = groups.begin(); it != groups.end(); ++it)
    {
        KFileMetaInfoItem i = d->groups[*it][key];
        if (i.isValid()) return i;
    }
    return KFileMetaInfoItem();
}

KFileMetaInfoItem KFileMetaInfo::item(const KFileMetaInfoItem::Hint hint) const
{
    TQStringList groups = preferredGroups();
    TQStringList::ConstIterator it;
    for (it = groups.begin(); it != groups.end(); ++it)
    {
        KFileMetaInfoItem i = d->groups[*it].item(hint);
        if (i.isValid()) return i;
    }
    return KFileMetaInfoItem();
}

KFileMetaInfoItem KFileMetaInfo::saveItem( const TQString& key,
                                           const TQString& preferredGroup,
                                           bool createGroup )
{
    assert(isValid());
    // try the preferred groups first
    if ( !preferredGroup.isEmpty() ) {
        TQMapIterator<TQString,KFileMetaInfoGroup> it =
            d->groups.find( preferredGroup );

        // try to create the preferred group, if necessary
        if ( it == d->groups.end() && createGroup ) {
            const KFileMimeTypeInfo::GroupInfo *groupInfo =
                d->mimeTypeInfo->groupInfo( preferredGroup );
            if ( groupInfo && groupInfo->supportedKeys().contains( key ) ) {
                if ( addGroup( preferredGroup ) )
                    it = d->groups.find( preferredGroup );
            }
        }

        if ( it != d->groups.end() ) {
            KFileMetaInfoItem item = it.data().addItem( key );
            if ( item.isValid() )
                return item;
        }
    }

    TQStringList groups = preferredGroups();

    KFileMetaInfoItem item;

    TQStringList::ConstIterator groupIt = groups.begin();
    for ( ; groupIt != groups.end(); ++groupIt )
    {
        TQMapIterator<TQString,KFileMetaInfoGroup> it = d->groups.find( *groupIt );
        if ( it != d->groups.end() )
        {
            KFileMetaInfoGroup group = it.data();
            item = findEditableItem( group, key );
            if ( item.isValid() )
                return item;
        }
        else // not existant -- try to create the group
        {
            const KFileMimeTypeInfo::GroupInfo *groupInfo =
                d->mimeTypeInfo->groupInfo( *groupIt );
            if ( groupInfo && groupInfo->supportedKeys().contains( key ) )
            {
                if ( addGroup( *groupIt ) )
                {
                    KFileMetaInfoGroup group = d->groups[*groupIt];
                    KFileMetaInfoItem item = group.addItem( key );
                    if ( item.isValid() )
                        return item;
//                     else ### add when removeGroup() is implemented :)
//                         removeGroup( *groupIt ); // couldn't add item -> remove
                }
            }
        }
    }

    // finally check for variable items

    return item;
}

KFileMetaInfoItem KFileMetaInfo::findEditableItem( KFileMetaInfoGroup& group,
                                                   const TQString& key )
{
    assert(isValid());
    KFileMetaInfoItem item = group.addItem( key );
    if ( item.isValid() && item.isEditable() )
         return item;

    if ( (d->mimeTypeInfo->groupInfo( group.name() )->attributes() & KFileMimeTypeInfo::Addable) )
        return item;

    return KFileMetaInfoItem();
}

KFileMetaInfoGroup KFileMetaInfo::appendGroup(const TQString& name)
{
    assert(isValid());
    if ( d->mimeTypeInfo->supportedGroups().contains(name) &&
         ! d->groups.contains(name) )
    {
        KFileMetaInfoGroup group( name, d->mimeTypeInfo );
        d->groups.insert(name, group);
        return group;
    }

    else {
        kdWarning(7033) << "Someone's trying to add a KFileMetaInfoGroup which is not supported or already existing: " << name << endl;
        return KFileMetaInfoGroup();
    }
}

TQString KFileMetaInfo::path() const
{
    return d->url.isLocalFile() ? d->url.path() : TQString::null;
}

KURL KFileMetaInfo::url() const
{
    return d->url;
}

void KFileMetaInfo::ref()
{
    if (d != Data::null) d->ref();

}

void KFileMetaInfo::deref()
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    if ((d != Data::null) && d->deref())
    {
//        kdDebug(7033) << "metainfo object for " << d->url.path << " is finally deleted\n";
        delete d;
        d = 0;
    }

}


KFileMetaInfo::Data* KFileMetaInfo::Data::null = 0L;
static KStaticDeleter<KFileMetaInfo::Data> sd_KFileMetaInfoData;

KFileMetaInfo::Data* KFileMetaInfo::Data::makeNull()
{
    if (!null)
        // We deliberately do not reset "null" after it has been destroyed!
        // Otherwise we will run into problems later in ~KFileMetaInfoItem
        // where the d-pointer is compared against null.
	null = sd_KFileMetaInfoData.setObject( new KFileMetaInfo::Data(KURL(), 0) );
    return null;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFilePlugin::KFilePlugin( TQObject *parent, const char *name,
                          const TQStringList& /*args*/)
    : TQObject( parent, name )
{
//    kdDebug(7033) << "loaded a plugin for " << name << endl;
}

KFilePlugin::~KFilePlugin()
{
//    kdDebug(7033) << "unloaded a plugin for " << name() << endl;
}

KFileMimeTypeInfo * KFilePlugin::addMimeTypeInfo( const TQString& mimeType )
{
    return KFileMetaInfoProvider::self()->addMimeTypeInfo( mimeType );
}

void KFilePlugin::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


KFileMimeTypeInfo::GroupInfo*  KFilePlugin::addGroupInfo(KFileMimeTypeInfo* info,
                  const TQString& key, const TQString& translatedKey) const
{
    return info->addGroupInfo(key, translatedKey);
}

void KFilePlugin::setAttributes(KFileMimeTypeInfo::GroupInfo* gi, uint attr) const
{
    gi->m_attr = attr;
}

void KFilePlugin::addVariableInfo(KFileMimeTypeInfo::GroupInfo* gi,
                                  TQVariant::Type type, uint attr) const
{
    gi->addVariableInfo(type, attr);
}

KFileMimeTypeInfo::ItemInfo* KFilePlugin::addItemInfo(KFileMimeTypeInfo::GroupInfo* gi,
                                                     const TQString& key,
                                                     const TQString& translatedKey,
                                                     TQVariant::Type type)
{
    return gi->addItemInfo(key, translatedKey, type);
}

void KFilePlugin::setAttributes(KFileMimeTypeInfo::ItemInfo* item, uint attr)
{
    item->m_attr = attr;
}

void KFilePlugin::setHint(KFileMimeTypeInfo::ItemInfo* item, uint hint)
{
    item->m_hint = hint;
}

void KFilePlugin::setUnit(KFileMimeTypeInfo::ItemInfo* item, uint unit)
{
    item->m_unit = unit;
    // set prefix and suffix
    switch (unit)
    {
        case KFileMimeTypeInfo::Seconds:
            item->m_suffix = i18n("s"); break;

        case KFileMimeTypeInfo::MilliSeconds:
            item->m_suffix = i18n("ms"); break;

        case KFileMimeTypeInfo::BitsPerSecond:
            item->m_suffix = i18n("bps"); break;

        case KFileMimeTypeInfo::Pixels:
            item->m_suffix = i18n("pixels"); break;

        case KFileMimeTypeInfo::Inches:
            item->m_suffix = i18n("in"); break;

        case KFileMimeTypeInfo::Centimeters:
            item->m_suffix = i18n("cm"); break;

        case KFileMimeTypeInfo::Bytes:
            item->m_suffix = i18n("B"); break;

        case KFileMimeTypeInfo::KiloBytes:
            item->m_suffix = i18n("KB"); break;

        case KFileMimeTypeInfo::FramesPerSecond:
            item->m_suffix = i18n("fps"); break;

        case KFileMimeTypeInfo::DotsPerInch:
            item->m_suffix = i18n("dpi"); break;

        case KFileMimeTypeInfo::BitsPerPixel:
            item->m_suffix = i18n("bpp"); break;

        case KFileMimeTypeInfo::Hertz:
            item->m_suffix = i18n("Hz"); break;

        case KFileMimeTypeInfo::Millimeters:
            item->m_suffix = i18n("mm");
    }
}

void KFilePlugin::setPrefix(KFileMimeTypeInfo::ItemInfo* item, const TQString& prefix)
{
    item->m_prefix = prefix;
}

void KFilePlugin::setSuffix(KFileMimeTypeInfo::ItemInfo* item, const TQString& suffix)
{
    item->m_suffix = suffix;
}

KFileMetaInfoGroup KFilePlugin::appendGroup(KFileMetaInfo& info, const TQString& key)
{
    return info.appendGroup(key);
}

void KFilePlugin::appendItem(KFileMetaInfoGroup& group, const TQString& key, TQVariant value)
{
    group.appendItem(key, value);
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KFileMetaInfoProvider * KFileMetaInfoProvider::s_self;
static KStaticDeleter<KFileMetaInfoProvider> sd;

KFileMetaInfoProvider * KFileMetaInfoProvider::self()
{
    if ( !s_self )
        s_self = sd.setObject( s_self, new KFileMetaInfoProvider() );

    return s_self;
}

KFileMetaInfoProvider::KFileMetaInfoProvider()
{
    m_plugins.setAutoDelete( true );
}

KFileMetaInfoProvider::~KFileMetaInfoProvider()
{
    m_plugins.clear();
    sd.setObject( 0 );
}

KFilePlugin* KFileMetaInfoProvider::loadPlugin( const TQString& mimeType, const TQString& protocol )
{
    //kdDebug() << "loadPlugin: mimeType=" << mimeType << " protocol=" << protocol << endl;
    // Currently the idea is: either the mimetype is set or the protocol, but not both.
    // We need PNG fileinfo, and trash: fileinfo, but not "PNG in the trash".
    TQString queryMimeType, query;
    if ( !mimeType.isEmpty() ) {
        query = "(not exist [X-TDE-Protocol])";
        queryMimeType = mimeType;
    } else {
        query = TQString::fromLatin1( "[X-TDE-Protocol] == '%1'" ).arg(protocol);
        // querying for a protocol: we have no mimetype, so we need to use KFilePlugin as one
        queryMimeType = "KFilePlugin";
        // hopefully using KFilePlugin as genericMimeType too isn't a problem
    }
    const TDETrader::OfferList offers = TDETrader::self()->query( queryMimeType, "KFilePlugin", query, TQString::null );
    if ( offers.isEmpty() )
        return 0;
    KService::Ptr service = *(offers.begin());
    Q_ASSERT( service && service->isValid() );
    if ( !service || !service->isValid() )
        return 0;

    KFilePlugin* plugin = KParts::ComponentFactory::createInstanceFromService<KFilePlugin>
                          ( service, this, mimeType.local8Bit() );
    if (!plugin)
        kdWarning(7033) << "error loading the plugin from " << service->desktopEntryPath() << endl;

    return plugin;
}

KFilePlugin* KFileMetaInfoProvider::loadAndRegisterPlugin( const TQString& mimeType, const TQString& protocol )
{
    Q_ASSERT( m_pendingMimetypeInfos.isEmpty() );
    m_pendingMimetypeInfos.clear();

    KFilePlugin* plugin = loadPlugin( mimeType, protocol );
    if ( !plugin ) {
        // No plugin found. Remember that, to save time.
        m_plugins.insert( protocol.isEmpty() ? mimeType : protocol, new CachedPluginInfo );
        return 0;
    }

    if ( !protocol.isEmpty() ) {
        // Protocol-metainfo: only one entry
        Q_ASSERT( m_pendingMimetypeInfos.count() == 1 );
        KFileMimeTypeInfo* info = m_pendingMimetypeInfos[ protocol ];
        Q_ASSERT( info );
        m_plugins.insert( protocol, new CachedPluginInfo( plugin, info, true ) );
    } else {
        // Mimetype-metainfo: the plugin can register itself for multiple mimetypes, remember them all
        bool first = true;
        TQDictIterator<KFileMimeTypeInfo> it( m_pendingMimetypeInfos );
        for( ; it.current(); ++it ) {
            KFileMimeTypeInfo* info = it.current();
            m_plugins.insert( it.currentKey(), new CachedPluginInfo( plugin, info, first ) );
            first = false;
        }
        // Hopefully the above includes the mimetype we asked for!
        if ( m_pendingMimetypeInfos.find( mimeType ) == 0 )
            kdWarning(7033) << plugin->className() << " was created for " << mimeType << " but doesn't call addMimeTypeInfo for it!" << endl;
    }
    m_pendingMimetypeInfos.clear();
    return plugin;
}

KFilePlugin * KFileMetaInfoProvider::plugin(const TQString& mimeType)
{
    return plugin( mimeType, TQString::null );
}

KFilePlugin * KFileMetaInfoProvider::plugin(const TQString& mimeType, const TQString& protocol)
{
    //kdDebug(7033) << "plugin() : looking for plugin for protocol=" << protocol << " mimeType=" << mimeType << endl;

    if ( !protocol.isEmpty() ) {
        CachedPluginInfo *cache = m_plugins.find( protocol );
        if ( cache && cache->plugin ) {
            return cache->plugin;
        }
        if ( !cache ) {
            KFilePlugin* plugin = loadAndRegisterPlugin( TQString::null, protocol );
            if ( plugin )
                return plugin;
        }
    }

    CachedPluginInfo *cache = m_plugins.find( mimeType );
    if ( cache ) {
        return cache->plugin;
    }

    KFilePlugin* plugin = loadAndRegisterPlugin( mimeType, TQString::null );

#if 0
    kdDebug(7033) << "currently loaded plugins:\n";

    TQDictIterator<CachedPluginInfo> it( m_plugins );
    for( ; it.current(); ++it ) {
        CachedPluginInfo* cache = it.current();
        kdDebug(7033)
            << it.currentKey() // mimetype or protocol
            << " : " << (cache->plugin ? cache->plugin->className() : "(no plugin)") << endl; // plugin
        // TODO print cache->mimeTypeInfo
    }
#endif

    return plugin;
}

TQStringList KFileMetaInfoProvider::preferredKeys( const TQString& mimeType ) const
{
    KService::Ptr service =
        KServiceTypeProfile::preferredService( mimeType, "KFilePlugin");

    if ( !service || !service->isValid() )
    {
//        kdDebug(7033) << "no valid service found\n";
        return TQStringList();
    }
    return service->property("PreferredItems").toStringList();
}

TQStringList KFileMetaInfoProvider::preferredGroups( const TQString& mimeType ) const
{
    KService::Ptr service =
        KServiceTypeProfile::preferredService( mimeType, "KFilePlugin");

    if ( !service || !service->isValid() )
    {
//        kdDebug(7033) << "no valid service found\n";
        return TQStringList();
    }
    return service->property("PreferredGroups").toStringList();
}

const KFileMimeTypeInfo * KFileMetaInfoProvider::mimeTypeInfo( const TQString& mimeType )
{
    return mimeTypeInfo( mimeType, TQString::null );
}

const KFileMimeTypeInfo * KFileMetaInfoProvider::mimeTypeInfo( const TQString& mimeType, const TQString& protocol )
{
    //kdDebug(7033) << "mimeTypeInfo() : looking for plugin for protocol=" << protocol << " mimeType=" << mimeType << endl;
    if ( !protocol.isEmpty() ) {
        CachedPluginInfo *cache = m_plugins.find( protocol );
        if ( cache && cache->mimeTypeInfo ) {
            return cache->mimeTypeInfo;
        }

        if ( !cache ) {
            loadAndRegisterPlugin( TQString::null, protocol );
            cache = m_plugins.find( protocol );
            if ( cache && cache->mimeTypeInfo ) {
                return cache->mimeTypeInfo;
            }
        }
    }

    CachedPluginInfo *cache = m_plugins.find( mimeType );
    if ( cache ) {
        return cache->mimeTypeInfo;
    }

    loadAndRegisterPlugin( mimeType, TQString::null );
    cache = m_plugins.find( mimeType );
    if ( cache ) {
        return cache->mimeTypeInfo;
    }
    return 0;
}

KFileMimeTypeInfo * KFileMetaInfoProvider::addMimeTypeInfo(
    const TQString& mimeType )
{

    KFileMimeTypeInfo *info = m_pendingMimetypeInfos.find( mimeType );
    Q_ASSERT( !info );
    if ( !info )
    {
        info = new KFileMimeTypeInfo( mimeType );
        m_pendingMimetypeInfos.insert( mimeType, info );
    }

    info->m_preferredKeys    = preferredKeys( mimeType );
    info->m_preferredGroups  = preferredGroups( mimeType );

    return info;
}

TQStringList KFileMetaInfoProvider::supportedMimeTypes() const
{
    TQStringList allMimeTypes;
    TQString tdefilePlugin = "KFilePlugin";

    TDETrader::OfferList offers = TDETrader::self()->query( "KFilePlugin" );
    TDETrader::OfferListIterator it = offers.begin();
    for ( ; it != offers.end(); ++it )
    {
        const TQStringList mimeTypes = (*it)->serviceTypes();
        TQStringList::ConstIterator it2 = mimeTypes.begin();
        for ( ; it2 != mimeTypes.end(); ++it2 )
            if ( allMimeTypes.find( *it2 ) == allMimeTypes.end() &&
                 *it2 != tdefilePlugin ) // also in serviceTypes()
                allMimeTypes.append( *it2 );
    }

    return allMimeTypes;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


// shared data of a KFileMetaInfoGroup
class KFileMetaInfoGroup::Data : public TQShared
{
public:
    Data(const TQString& _name)
        : TQShared(),
          name(_name),
          mimeTypeInfo(0L),
          dirty( false ),
          added( false )
    {}

    // we use this one for the streaming operators
    Data() : mimeTypeInfo(0L) {}
    ~Data() {
        if ( this == null )
            delete mimeTypeInfo;
    };

    TQString                             name;
    TQMap<TQString, KFileMetaInfoItem>    items;
    const KFileMimeTypeInfo*            mimeTypeInfo;
    TQStringList                         removedItems;
    bool                                dirty   :1;
    bool                                added   :1;

    static Data* null;
    static Data* makeNull();

};

KFileMetaInfoGroup::KFileMetaInfoGroup( const TQString& name,
                                        const KFileMimeTypeInfo* info )
    : d(new Data( name ) )
{
      d->mimeTypeInfo = info;
}

KFileMetaInfoGroup::KFileMetaInfoGroup( const KFileMetaInfoGroup& original )
{
    // operator= does everything that's necessary
    d = Data::makeNull();
    *this = original;
}

KFileMetaInfoGroup::KFileMetaInfoGroup()
{
    d = Data::makeNull();
}

KFileMetaInfoGroup::~KFileMetaInfoGroup()
{
    deref();
}

const KFileMetaInfoGroup& KFileMetaInfoGroup::operator= (const KFileMetaInfoGroup& info )
{
    if (d != info.d)
    {
        deref();
        // first deref the old one
        d = info.d;
        // and now ref the new one
        ref();
    }
    return *this;
}

bool KFileMetaInfoGroup::isValid() const
{
    // We don't call makeNull here since it isn't necassery, see deref()
    return d != Data::null;
}

bool KFileMetaInfoGroup::isEmpty() const
{
    return d->items.isEmpty();
}

TQStringList KFileMetaInfoGroup::preferredKeys() const
{
    assert(isValid());
    TQStringList list = keys();
    TQStringList newlist;
    TQStringList preferredKeys = d->mimeTypeInfo->preferredKeys();
    TQStringList::Iterator pref;
    TQStringList::Iterator begin = preferredKeys.begin();
    TQStringList::Iterator end   = preferredKeys.end();

    // move all keys from the preferred keys that are in our list to a new list
    for ( pref = begin; pref!=end; ++pref )
    {
        TQStringList::Iterator item = list.find(*pref);
        if ( item != list.end() )
        {
             newlist.append( *item );
             list.remove(item);
        }
    }

    // now the old list only contains the non-preferred items, so we
    // add the remaining ones to newlist
    newlist += list;

    return newlist;
}

TQStringList KFileMetaInfoGroup::keys() const
{
    if (d == Data::makeNull())
        kdWarning(7033) << "attempt to get the keys of "
                           "an invalid metainfo group";

    TQStringList list;

    // make a TQStringList with all available keys
    TQMapConstIterator<TQString, KFileMetaInfoItem> it;
    for (it = d->items.begin(); it!=d->items.end(); ++it)
    {
        list.append(it.data().key());
//        kdDebug(7033) << "Item " << it.data().key() << endl;
    }
    return list;
}

TQString KFileMetaInfoGroup::translatedName() const
{
    assert(isValid());
    return d->mimeTypeInfo->groupInfo(d->name)->translatedName();
}

TQStringList KFileMetaInfoGroup::supportedKeys() const
{
    assert(isValid());
    return d->mimeTypeInfo->groupInfo(d->name)->supportedKeys();
}

bool KFileMetaInfoGroup::supportsVariableKeys() const
{
    assert(isValid());
    return d->mimeTypeInfo->groupInfo(d->name)->supportsVariableKeys();
}

bool KFileMetaInfoGroup::contains( const TQString& key ) const
{
    return d->items.contains(key);
}

KFileMetaInfoItem KFileMetaInfoGroup::item( const TQString& key) const
{
    TQMapIterator<TQString,KFileMetaInfoItem> it = d->items.find( key );
    if ( it != d->items.end() )
        return it.data();

    return KFileMetaInfoItem();
}

KFileMetaInfoItem KFileMetaInfoGroup::item(uint hint) const
{
    TQMapIterator<TQString, KFileMetaInfoItem> it;

    for (it = d->items.begin(); it!=d->items.end(); ++it)
        if (it.data().hint() == hint)
            return it.data();

    return KFileMetaInfoItem();
}

TQString KFileMetaInfoGroup::name() const
{
    return d->name;
}

uint KFileMetaInfoGroup::attributes() const
{
    assert(isValid());
    return d->mimeTypeInfo->groupInfo(d->name)->attributes();
}

void KFileMetaInfoGroup::setAdded()
{
    d->added = true;
}

bool KFileMetaInfoGroup::isModified() const
{
    return d->dirty;
}

void KFileMetaInfoGroup::ref()
{
    if (d != Data::null) d->ref();

}

void KFileMetaInfoGroup::deref()
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    if ((d != Data::null) && d->deref())
    {
//        kdDebug(7033) << "metainfo group " << d->name
//                      << " is finally deleted\n";
        delete d;
        d = 0;
    }

}

KFileMetaInfoItem KFileMetaInfoGroup::addItem( const TQString& key )
{
    assert(isValid());
    TQMapIterator<TQString,KFileMetaInfoItem> it = d->items.find( key );
    if ( it != d->items.end() )
        return it.data();

    const KFileMimeTypeInfo::GroupInfo* ginfo = d->mimeTypeInfo->groupInfo(d->name);

    if ( !ginfo ) {
        Q_ASSERT( ginfo );
        return KFileMetaInfoItem();
    }

    const KFileMimeTypeInfo::ItemInfo* info = ginfo->itemInfo(key);

    if ( !info ) {
        Q_ASSERT( info );
        return KFileMetaInfoItem();
    }

    KFileMetaInfoItem item;

    if (info->isVariableItem())
        item = KFileMetaInfoItem(ginfo->variableItemInfo(), key, TQVariant());
    else
        item = KFileMetaInfoItem(info, key, TQVariant());

    d->items.insert(key, item);
    item.setAdded();           // mark as added
    d->dirty = true;           // mark ourself as dirty, too
    return item;
}

bool KFileMetaInfoGroup::removeItem( const TQString& key )
{
    if (!isValid())
    {
          kdDebug(7033) << "trying to remove an item from an invalid group\n";
          return false;
    }

    TQMapIterator<TQString, KFileMetaInfoItem> it = d->items.find(key);
    if ( it==d->items.end() )
    {
          kdDebug(7033) << "trying to remove the non existant item " << key << "\n";
          return false;
    }

    if (!((*it).attributes() & KFileMimeTypeInfo::Removable))
    {
        kdDebug(7033) << "trying to remove a non removable item\n";
        return false;
    }

    (*it).setRemoved();
    d->items.remove(it);
    d->removedItems.append(key);
    d->dirty = true;
    return true;
}

TQStringList KFileMetaInfoGroup::removedItems()
{
    return d->removedItems;
}

KFileMetaInfoItem KFileMetaInfoGroup::appendItem(const TQString& key,
                                                 const TQVariant& value)
{
    //KDE4 enforce (value.type() == d->mimeTypeInfo->type())
    assert(isValid());
    const KFileMimeTypeInfo::GroupInfo* ginfo = d->mimeTypeInfo->groupInfo(d->name);
    if ( !ginfo ) {
        kdWarning() << "Trying to append a Metadata item for a non-existant group:" << d->name << endl;
        return KFileMetaInfoItem();
    }
    const KFileMimeTypeInfo::ItemInfo* info = ginfo->itemInfo(key);
    if ( !info ) {
        kdWarning() << "Trying to append a Metadata item for an unknown key (no ItemInfo): " << key << endl;
        return KFileMetaInfoItem();
    }

    KFileMetaInfoItem item;

    if (info->key().isNull())
        item = KFileMetaInfoItem(ginfo->variableItemInfo(), key, value);
    else
        item = KFileMetaInfoItem(info, key, value);

    kdDebug(7033) << "KFileMetaInfogroup inserting a " << key << endl;

    d->items.insert(key, item);
    return item;
}

KFileMetaInfoGroup::Data* KFileMetaInfoGroup::Data::null = 0L;
static KStaticDeleter<KFileMetaInfoGroup::Data> sd_KFileMetaInfoGroupData;

KFileMetaInfoGroup::Data* KFileMetaInfoGroup::Data::makeNull()
{
    if (!null)
    {
        // We deliberately do not reset "null" after it has been destroyed!
        // Otherwise we will run into problems later in ~KFileMetaInfoItem
        // where the d-pointer is compared against null.
        null = new Data(TQString::null);
        null->mimeTypeInfo = new KFileMimeTypeInfo();
        sd_KFileMetaInfoGroupData.setObject( null );
    }
    return null;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFileMimeTypeInfo::KFileMimeTypeInfo( const TQString& mimeType )
    : m_mimeType( mimeType )
{
    m_groups.setAutoDelete( true );
}

KFileMimeTypeInfo::~KFileMimeTypeInfo()
{
}

const KFileMimeTypeInfo::GroupInfo * KFileMimeTypeInfo::groupInfo( const TQString& group ) const
{
    return m_groups.find( group );
}

KFileMimeTypeInfo::GroupInfo * KFileMimeTypeInfo::addGroupInfo(
                           const TQString& name, const TQString& translatedName )
{
    GroupInfo* group = new GroupInfo( name, translatedName );
    m_groups.insert(name, group);
    return group;
}

TQStringList KFileMimeTypeInfo::supportedGroups() const
{
    TQStringList list;
    TQDictIterator<GroupInfo> it( m_groups );
    for ( ; it.current(); ++it )
        list.append( it.current()->name() );

    return list;
}

TQStringList KFileMimeTypeInfo::translatedGroups() const
{
    TQStringList list;
    TQDictIterator<GroupInfo> it( m_groups );
    for ( ; it.current(); ++it )
        list.append( it.current()->translatedName() );

    return list;
}

TQStringList KFileMimeTypeInfo::supportedKeys() const
{
    // not really efficient, but not those are not large lists, probably.
    // maybe cache the result?
    TQStringList keys;
    TQStringList::ConstIterator lit;
    TQDictIterator<GroupInfo> it( m_groups );
    for ( ; it.current(); ++it ) { // need to nuke dupes
        TQStringList list = it.current()->supportedKeys();
        for ( lit = list.begin(); lit != list.end(); ++lit ) {
            if ( keys.find( *lit ) == keys.end() )
                keys.append( *lit );
        }
    }

    return keys;
}

TQValidator * KFileMimeTypeInfo::createValidator(const TQString& group,
                                                const TQString& key,
                                                TQObject *parent,
                                                const char *name) const
{
    KFilePlugin* plugin = KFileMetaInfoProvider::self()->plugin(m_mimeType);
    if (plugin) return plugin->createValidator(mimeType(), group, key,
                                               parent, name);
    return 0;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFileMimeTypeInfo::GroupInfo::GroupInfo( const TQString& name,
                                         const TQString& translatedName )
    : m_name( name ),
      m_translatedName( translatedName ),
      m_attr( 0 ),
      m_variableItemInfo( 0 )

{
    m_itemDict.setAutoDelete( true );
}

KFileMimeTypeInfo::GroupInfo::~GroupInfo()
{
    delete m_variableItemInfo;
} 

const KFileMimeTypeInfo::ItemInfo * KFileMimeTypeInfo::GroupInfo::itemInfo( const TQString& key ) const
{
    ItemInfo* item = m_itemDict.find( key );

    // if we the item isn't found and variable keys are supported, we need to
    // return the default variable key iteminfo.
    if (!item && m_variableItemInfo)
    {
        return m_variableItemInfo;
    }
    return item;
}

KFileMimeTypeInfo::ItemInfo* KFileMimeTypeInfo::GroupInfo::addItemInfo(
                  const TQString& key, const TQString& translatedKey,
                  TQVariant::Type type)
{
//    kdDebug(7034) << key << "(" << translatedKey << ") -> " << TQVariant::typeToName(type) << endl;

    ItemInfo* item = new ItemInfo(key, translatedKey, type);
    m_supportedKeys.append(key);
    m_itemDict.insert(key, item);
    return item;
}


void KFileMimeTypeInfo::GroupInfo::addVariableInfo( TQVariant::Type type,
                                                   uint attr )
{
    // just make sure that it's not already there
    delete m_variableItemInfo;
    m_variableItemInfo = new ItemInfo(TQString::null, TQString::null, type);
    m_variableItemInfo->m_attr = attr;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

TQString KFileMimeTypeInfo::ItemInfo::string(const TQVariant& value, bool mangle) const
{
    TQString s;

    switch (value.type())
    {
        case TQVariant::Invalid :
            return "---";

        case TQVariant::Bool :
            s = value.toBool() ? i18n("Yes") : i18n("No");
            break;

        case TQVariant::Int :
            if (unit() == KFileMimeTypeInfo::Seconds)
            {
              int seconds = value.toInt() % 60;
              int minutes = value.toInt() / 60 % 60;
              int hours   = value.toInt() / 3600;
              s = hours ? TQString().sprintf("%d:%02d:%02d",hours, minutes, seconds)
                        : TQString().sprintf("%02d:%02d", minutes, seconds);
              return s; // no suffix wanted
            }
            else if (unit() == KFileMimeTypeInfo::Bytes)
            {
                // convertSize already adds the correct suffix
                return TDEIO::convertSize(value.toInt());
            }
            else if (unit() == KFileMimeTypeInfo::KiloBytes)
            {
                // convertSizeFromKB already adds the correct suffix
                return TDEIO::convertSizeFromKB(value.toInt());
            }
            else
                s = TDEGlobal::locale()->formatNumber( value.toInt() , 0);
            break;

        case TQVariant::LongLong :
            s = TDEGlobal::locale()->formatNumber( value.toLongLong(), 0 );
            break;

	case TQVariant::ULongLong :
            if ( unit() == KFileMimeTypeInfo::Bytes )
                return TDEIO::convertSize( value.toULongLong() );
            else if ( unit() == KFileMimeTypeInfo::KiloBytes )
                return TDEIO::convertSizeFromKB( value.toULongLong() );
            else
                s = TDEGlobal::locale()->formatNumber( value.toULongLong(), 0 );
            break;

        case TQVariant::UInt :
            s = TDEGlobal::locale()->formatNumber( value.toUInt() , 0);
            break;

        case TQVariant::Double :
            s = TDEGlobal::locale()->formatNumber( value.toDouble(), 3);
            break;

        case TQVariant::Date :
            s = TDEGlobal::locale()->formatDate( value.toDate(), true );
            break;

        case TQVariant::Time :
            s = TDEGlobal::locale()->formatTime( value.toTime(), true );
            break;

        case TQVariant::DateTime :
            s = TDEGlobal::locale()->formatDateTime( value.toDateTime(),
                                                   true, true );
            break;

        case TQVariant::Size :
            s = TQString("%1 x %2").arg(value.toSize().width())
                                .arg(value.toSize().height());
            break;

        case TQVariant::Point :
            s = TQString("%1/%2").arg(value.toSize().width())
                                .arg(value.toSize().height());
            break;

        default:
            s = value.toString();
    }

    if (mangle && !s.isNull())
    {
        s.prepend(prefix());
        s.append(" " + suffix());
    }
    return s;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////



// stream operators

/* serialization of a KFileMetaInfoItem:
   first a bool that says if the items is valid, and if yes,
   all the elements of the Data
*/
TDEIO_EXPORT TQDataStream& operator <<(TQDataStream& s, const KFileMetaInfoItem& item )
{

     KFileMetaInfoItem::Data* d = item.d;

     // if the object is invalid, put only a char in the stream
     bool isValid = item.isValid();
     s << isValid;
     // ### what do about mimetypeInfo ?
     if (isValid)
         s << d->key
           << d->value
           << d->dirty
           << d->added
           << d->removed;

     return s;
}


TDEIO_EXPORT TQDataStream& operator >>(TQDataStream& s, KFileMetaInfoItem& item )
{
     bool isValid;
     s >> isValid;

     if (!isValid)
     {
         item = KFileMetaInfoItem();
         return s;
     }

     // we need a new object for our data
     item.deref();
     item.d = new KFileMetaInfoItem::Data();

     // ### what do about mimetypeInfo ?
     bool dirty, added, removed;
     s >> item.d->key
       >> item.d->value
       >> dirty
       >> added
       >> removed;
     item.d->dirty = dirty;
     item.d->added = added;
     item.d->removed = removed;

    return s;
}


// serialization of a KFileMetaInfoGroup
// we serialize the name of the mimetype here instead of the mimetype info
// on the other side, we can simply use this to ask the provider for the info
TDEIO_EXPORT TQDataStream& operator <<(TQDataStream& s, const KFileMetaInfoGroup& group )
{
    KFileMetaInfoGroup::Data* d = group.d;

    // if the object is invalid, put only a byte in the stream
    bool isValid = group.isValid();

    s << isValid;
    if (isValid)
    {
        s << d->name
          << d->items
          << d->mimeTypeInfo->mimeType();
    }
    return s;
}

TDEIO_EXPORT TQDataStream& operator >>(TQDataStream& s, KFileMetaInfoGroup& group )
{
    TQString mimeType;
    bool isValid;
    s >> isValid;

    // if it's invalid, there is not much to do
    if (!isValid)
    {
        group = KFileMetaInfoGroup();
        return s;
    }

    // we need a new object for our data
    group.deref();
    group.d = new KFileMetaInfoGroup::Data();

    s >> group.d->name
      >> group.d->items
      >> mimeType;

    group.d->mimeTypeInfo = KFileMetaInfoProvider::self()->mimeTypeInfo(mimeType);

    // we need to set the item info for the items here
    TQMapIterator<TQString, KFileMetaInfoItem> it = group.d->items.begin();
    for ( ; it != group.d->items.end(); ++it)
    {
        (*it).d->mimeTypeInfo = group.d->mimeTypeInfo->groupInfo(group.d->name)
                                  ->itemInfo((*it).key());
    }

    return s;
}

// serialization of a KFileMetaInfo object
// we serialize the name of the mimetype here instead of the mimetype info
// on the other side, we can simply use this to ask the provider for the info
TDEIO_EXPORT TQDataStream& operator <<(TQDataStream& s, const KFileMetaInfo& info )
{
    KFileMetaInfo::Data* d = info.d;

    // if the object is invalid, put only a byte that tells this
    bool isValid = info.isValid();

    s << isValid;
    if (isValid)
    {
        s << d->url
          << d->what
          << d->groups
          << d->mimeTypeInfo->mimeType();
    }
    return s;
}

TDEIO_EXPORT TQDataStream& operator >>(TQDataStream& s, KFileMetaInfo& info )
{
    TQString mimeType;
    bool isValid;
    s >> isValid;

    // if it's invalid, there is not much to do
    if (!isValid)
    {
        info = KFileMetaInfo();
        return s;
    }

    // we need a new object for our data
    info.deref();
    info.d = new KFileMetaInfo::Data();

    s >> info.d->url
      >> info.d->what
      >> info.d->groups
      >> mimeType;
    info.d->mimeTypeInfo = KFileMetaInfoProvider::self()->mimeTypeInfo(mimeType);

    return s;
}

#include "tdefilemetainfo.moc"
