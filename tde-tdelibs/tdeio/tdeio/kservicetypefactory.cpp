/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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
 **/

#include "kservicetypefactory.h"
#include "tdesycoca.h"
#include "tdesycocatype.h"
#include "tdesycocadict.h"
#include "kservicetype.h"
#include "kmimetype.h"
#include "kuserprofile.h"

#include <tdeapplication.h>
#include <kdebug.h>
#include <assert.h>
#include <kstringhandler.h>
#include <tqfile.h>

KServiceTypeFactory::KServiceTypeFactory()
 : KSycocaFactory( KST_KServiceTypeFactory )
{
   _self = this;
   m_fastPatternOffset = 0;
   m_otherPatternOffset = 0;
   if (m_str)
   {
      // Read Header
      TQ_INT32 i,n;
      (*m_str) >> i;
      m_fastPatternOffset = i;
      (*m_str) >> i;
      m_otherPatternOffset = i;
      (*m_str) >> n;

      if (n > 1024)
      {
         KSycoca::flagError();
      }
      else
      {
         TQString str;
         for(;n;n--)
         {
            KSycocaEntry::read(*m_str, str);
            (*m_str) >> i;
            m_propertyTypeDict.insert(str, i);
         }
      }
   }
}


KServiceTypeFactory::~KServiceTypeFactory()
{
  _self = 0L;
  KServiceTypeProfile::clear();
}

KServiceTypeFactory * KServiceTypeFactory::self()
{
  if (!_self)
    _self = new KServiceTypeFactory();
  return _self;
}

KServiceType * KServiceTypeFactory::findServiceTypeByName(const TQString &_name)
{
   if (!m_sycocaDict) return 0L; // Error!
   assert (!KSycoca::self()->isBuilding());
   int offset = m_sycocaDict->find_string( _name );
   if (!offset) return 0; // Not found
   KServiceType * newServiceType = createEntry(offset);

   // Check whether the dictionary was right.
   if (newServiceType && (newServiceType->name() != _name))
   {
     // No it wasn't...
     delete newServiceType;
     newServiceType = 0; // Not found
   }
   return newServiceType;
}

TQVariant::Type KServiceTypeFactory::findPropertyTypeByName(const TQString &_name)
{
   if (!m_sycocaDict)
      return TQVariant::Invalid; // Error!

   assert (!KSycoca::self()->isBuilding());

   TQMapConstIterator<TQString,int> it = m_propertyTypeDict.find(_name);
   if (it != m_propertyTypeDict.end()) {
     return (TQVariant::Type)it.data();
   }

   return TQVariant::Invalid;
}

KMimeType * KServiceTypeFactory::findFromPattern(const TQString &_filename, TQString *match)
{
   // Assume we're NOT building a database
   if (!m_str) return 0;

   // Get stream to the header
   TQDataStream *str = m_str;

   str->device()->at( m_fastPatternOffset );

   TQ_INT32 nrOfEntries;
   (*str) >> nrOfEntries;
   TQ_INT32 entrySize;
   (*str) >> entrySize;

   TQ_INT32 fastOffset =  str->device()->at( );

   TQ_INT32 matchingOffset = 0;

   // Let's go for a binary search in the "fast" pattern index
   TQ_INT32 left = 0;
   TQ_INT32 right = nrOfEntries - 1;
   TQ_INT32 middle;
   // Extract extension
   int lastDot = _filename.findRev('.');
   int ext_len = _filename.length() - lastDot - 1;
   if (lastDot != -1 && ext_len <= 4) // if no '.', skip the extension lookup
   {
      TQString extension = _filename.right( ext_len );
      extension = extension.leftJustify(4);

      TQString pattern;
      while (left <= right) {
         middle = (left + right) / 2;
         // read pattern at position "middle"
         str->device()->at( middle * entrySize + fastOffset );
         KSycocaEntry::read(*str, pattern);
         int cmp = pattern.compare( extension );
         if (cmp < 0)
            left = middle + 1;
         else if (cmp == 0) // found
         {
            (*str) >> matchingOffset;
            // don't return newServiceType - there may be an "other" pattern that
            // matches best this file, like *.tar.bz
            if (match)
                *match = "*."+pattern.stripWhiteSpace();
            break; // but get out of the fast patterns
         }
         else
            right = middle - 1;
      }
   }

   // Now try the "other" Pattern table
   if ( m_patterns.isEmpty() ) {
      str->device()->at( m_otherPatternOffset );

      TQString pattern;
      TQ_INT32 mimetypeOffset;

      while (true)
      {
         KSycocaEntry::read(*str, pattern);
         if (pattern.isEmpty()) // end of list
            break;
         (*str) >> mimetypeOffset;
         m_patterns.push_back( pattern );
         m_pattern_offsets.push_back( mimetypeOffset );
      }
   }

   assert( m_patterns.size() == m_pattern_offsets.size() );

   TQStringList::const_iterator it = m_patterns.begin();
   TQStringList::const_iterator end = m_patterns.end();
   TQValueVector<TQ_INT32>::const_iterator it_offset = m_pattern_offsets.begin();

  for ( ; it != end; ++it, ++it_offset )
   {
      if ( KStringHandler::matchFileName( _filename, *it ) )
      {
         if ( !matchingOffset || !(*it).endsWith( "*" ) ) // *.html wins over Makefile.*
         {
             matchingOffset = *it_offset;
             if (match)
                *match = *it;
             break;
         }
      }
   }

   if ( matchingOffset ) {
      KServiceType *newServiceType = createEntry( matchingOffset );
      assert (newServiceType && newServiceType->isType( KST_KMimeType ));
      return (KMimeType *) newServiceType;
   }
   else
      return 0;
}

KMimeType::List KServiceTypeFactory::allMimeTypes()
{
   KMimeType::List result;
   KSycocaEntry::List list = allEntries();
   for( KSycocaEntry::List::Iterator it = list.begin();
        it != list.end();
        ++it)
   {
      KMimeType *newMimeType = dynamic_cast<KMimeType *>((*it).data());
      if (newMimeType)
         result.append( KMimeType::Ptr( newMimeType ) );
   }
   return result;
}

KServiceType::List KServiceTypeFactory::allServiceTypes()
{
   KServiceType::List result;
   KSycocaEntry::List list = allEntries();
   for( KSycocaEntry::List::Iterator it = list.begin();
        it != list.end();
        ++it)
   {
#ifndef TQ_WS_QWS
      KServiceType *newServiceType = dynamic_cast<KServiceType *>((*it).data());
#else //FIXME
      KServiceType *newServiceType = (KServiceType*)(*it).data();
#endif
      if (newServiceType)
         result.append( KServiceType::Ptr( newServiceType ) );
   }
   return result;
}

bool KServiceTypeFactory::checkMimeTypes()
{
   TQDataStream *str = KSycoca::self()->findFactory( factoryId() );
   if (!str) return false;

   // check if there are mimetypes/servicetypes
   return (m_beginEntryOffset != m_endEntryOffset);
}

KServiceType * KServiceTypeFactory::createEntry(int offset)
{
   KServiceType *newEntry = 0;
   KSycocaType type;
   TQDataStream *str = KSycoca::self()->findEntry(offset, type);
   if (!str) return 0;

   switch(type)
   {
     case KST_KServiceType:
        newEntry = new KServiceType(*str, offset);
        break;
     case KST_KMimeType:
        newEntry = new KMimeType(*str, offset);
        break;
     case KST_KFolderType:
        newEntry = new KFolderType(*str, offset);
        break;
     case KST_KDEDesktopMimeType:
        newEntry = new KDEDesktopMimeType(*str, offset);
        break;
     case KST_KExecMimeType:
        newEntry = new KExecMimeType(*str, offset);
        break;

     default:
        kdError(7011) << TQString(TQString("KServiceTypeFactory: unexpected object entry in KSycoca database (type = %1)").arg((int)type)) << endl;
        break;
   }
   if (newEntry && !newEntry->isValid())
   {
      kdError(7011) << "KServiceTypeFactory: corrupt object in KSycoca database!\n" << endl;
      delete newEntry;
      newEntry = 0;
   }
   return newEntry;
}

KServiceTypeFactory *KServiceTypeFactory::_self = 0;

void KServiceTypeFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }
