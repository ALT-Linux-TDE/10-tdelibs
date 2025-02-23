/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure   <faure@kde.org>
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

#include "kbuildservicetypefactory.h"
#include "tdesycoca.h"
#include "tdesycocadict.h"
#include "kresourcelist.h"

#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <kmessageboxwrapper.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <assert.h>
#include <kdesktopfile.h>

template class TQDict<KMimeType>;

KBuildServiceTypeFactory::KBuildServiceTypeFactory() :
  KServiceTypeFactory()
{
   // Read servicetypes first, since they might be needed to read mimetype properties
   m_resourceList = new KSycocaResourceList;
   m_resourceList->add("servicetypes", "*.desktop");
   m_resourceList->add("servicetypes", "*.kdelnk");
   m_resourceList->add( "mime", "*.desktop" );
   m_resourceList->add( "mime", "*.kdelnk" );
}

// return all service types for this factory
// i.e. first arguments to m_resourceList->add() above
TQStringList KBuildServiceTypeFactory::resourceTypes()
{
    return TQStringList() << "servicetypes" << "mime";
}

KBuildServiceTypeFactory::~KBuildServiceTypeFactory()
{
   delete m_resourceList;
}

KServiceType * KBuildServiceTypeFactory::findServiceTypeByName(const TQString &_name)
{
   assert (KSycoca::self()->isBuilding());
   // We're building a database - the service type must be in memory
   KSycocaEntry::Ptr * servType = (*m_entryDict)[ _name ];
   if (!servType)
      return 0;
   return (KServiceType *) ((KSycocaEntry*)*servType);
}


KSycocaEntry *
KBuildServiceTypeFactory::createEntry(const TQString &file, const char *resource)
{
  TQString name = file;
  int pos = name.findRev('/');
  if (pos != -1)
  {
     name = name.mid(pos+1);
  }

  if (name.isEmpty())
     return 0;

  KDesktopFile desktopFile(file, true, resource);

  if ( desktopFile.readBoolEntry( "Hidden", false ) == true )
      return 0;

  // TODO check Type field first
  TQString mime = desktopFile.readEntry( "MimeType" );
  TQString service = desktopFile.readEntry( "X-TDE-ServiceType" );

  if ( mime.isEmpty() && service.isEmpty() )
  {
    TQString tmp = TQString("The service/mime type config file\n%1\n"
                  "does not contain a ServiceType=...\nor MimeType=... entry").arg( file );
    kdWarning(7012) << tmp << endl;
    return 0;
  }

  KServiceType* e;
  if ( mime == "inode/directory" )
    e = new KFolderType( &desktopFile );
  else if ( (mime == "application/x-desktop")
         || (mime == "media/builtin-mydocuments")
         || (mime == "media/builtin-mycomputer")
         || (mime == "media/builtin-mynetworkplaces")
         || (mime == "media/builtin-printers")
         || (mime == "media/builtin-trash")
         || (mime == "media/builtin-webbrowser") )
    e = new KDEDesktopMimeType( &desktopFile );
  else if ( mime == "application/x-executable" || mime == "application/x-pie-executable" || mime == "application/x-shellscript" )
    e = new KExecMimeType( &desktopFile );
  else if ( !mime.isEmpty() )
    e = new KMimeType( &desktopFile );
  else
    e = new KServiceType( &desktopFile );

  if (e->isDeleted())
  {
    delete e;
    return 0;
  }

  if ( !(e->isValid()) )
  {
    kdWarning(7012) << "Invalid ServiceType : " << file << endl;
    delete e;
    return 0;
  }

  return e;
}

void
KBuildServiceTypeFactory::saveHeader(TQDataStream &str)
{
   KSycocaFactory::saveHeader(str);
   str << (TQ_INT32) m_fastPatternOffset;
   str << (TQ_INT32) m_otherPatternOffset;
   str << (TQ_INT32) m_propertyTypeDict.count();

   TQMapIterator<TQString, int> it;
   for (it = m_propertyTypeDict.begin(); it != m_propertyTypeDict.end(); ++it)
   {
     str << it.key() << (TQ_INT32)it.data();
   }

}

void
KBuildServiceTypeFactory::save(TQDataStream &str)
{
   KSycocaFactory::save(str);

   savePatternLists(str);

   int endOfFactoryData = str.device()->at();

   // Update header (pass #3)
   saveHeader(str);

   // Seek to end.
   str.device()->at(endOfFactoryData);
}

void
KBuildServiceTypeFactory::savePatternLists(TQDataStream &str)
{
   // Store each patterns in one of the 2 string lists (for sorting)
   TQStringList fastPatterns;  // for *.a to *.abcd
   TQStringList otherPatterns; // for the rest (core.*, *.tar.bz2, *~) ...
   TQDict<KMimeType> dict;

   // For each mimetype in servicetypeFactory
   for(TQDictIterator<KSycocaEntry::Ptr> it ( *m_entryDict );
       it.current();
       ++it)
   {
      KSycocaEntry *entry = (*it.current());
      if ( entry->isType( KST_KMimeType ) )
      {
        KMimeType *mimeType = (KMimeType *) entry;
        TQStringList pat = mimeType->patterns();
        TQStringList::ConstIterator patit = pat.begin();
        for ( ; patit != pat.end() ; ++patit )
        {
           const TQString &pattern = *patit;
           if ( pattern.findRev('*') == 0
                && pattern.findRev('.') == 1
                && pattern.length() <= 6 )
              // it starts with "*.", has no other '*' and no other '.', and is max 6 chars
              // => fast patttern
              fastPatterns.append( pattern );
           else if (!pattern.isEmpty()) // some stupid mimetype files have "Patterns=;"
              otherPatterns.append( pattern );
           // Assumption : there is only one mimetype for that pattern
           // It doesn't really make sense otherwise, anyway.
           dict.replace( pattern, mimeType );
        }
      }
   }
   // Sort the list - the fast one, useless for the other one
   fastPatterns.sort();

   TQ_INT32 entrySize = 0;
   TQ_INT32 nrOfEntries = 0;

   m_fastPatternOffset = str.device()->at();

   // Write out fastPatternHeader (Pass #1)
   str.device()->at(m_fastPatternOffset);
   str << nrOfEntries;
   str << entrySize;

   // For each fast pattern
   TQStringList::ConstIterator it = fastPatterns.begin();
   for ( ; it != fastPatterns.end() ; ++it )
   {
     int start = str.device()->at();
     // Justify to 6 chars with spaces, so that the size remains constant
     // in the database file.
     TQString paddedPattern = (*it).leftJustify(6).right(4); // remove leading "*."
     //kdDebug(7021) << TQString("FAST : '%1' '%2'").arg(paddedPattern).arg(dict[(*it)]->name()) << endl;
     str << paddedPattern;
     str << dict[(*it)]->offset();
     entrySize = str.device()->at() - start;
     nrOfEntries++;
   }

   // store position
   m_otherPatternOffset = str.device()->at();

   // Write out fastPatternHeader (Pass #2)
   str.device()->at(m_fastPatternOffset);
   str << nrOfEntries;
   str << entrySize;

   // For the other patterns
   str.device()->at(m_otherPatternOffset);

   it = otherPatterns.begin();
   for ( ; it != otherPatterns.end() ; ++it )
   {
     //kdDebug(7021) << TQString("OTHER : '%1' '%2'").arg(*it).arg(dict[(*it)]->name()) << endl;
     str << (*it);
     str << dict[(*it)]->offset();
   }

   str << TQString(""); // end of list marker (has to be a string !)
}

void
KBuildServiceTypeFactory::addEntry(KSycocaEntry *newEntry, const char *resource)
{
   KServiceType * serviceType = (KServiceType *) newEntry;
   if ( (*m_entryDict)[ newEntry->name() ] )
   {
     // Already exists
     if (serviceType->desktopEntryPath().endsWith("kdelnk"))
        return; // Skip

     // Replace
     KSycocaFactory::removeEntry(newEntry);
   }
   KSycocaFactory::addEntry(newEntry, resource);


   const TQMap<TQString,TQVariant::Type>& pd = serviceType->propertyDefs();
   TQMap<TQString,TQVariant::Type>::ConstIterator pit = pd.begin();
   for( ; pit != pd.end(); ++pit )
   {
     if (!m_propertyTypeDict.contains(pit.key()))
       m_propertyTypeDict.insert(pit.key(), pit.data());
     else if (m_propertyTypeDict[pit.key()] != pit.data())
       kdWarning(7021) << "Property '"<< pit.key() << "' is defined multiple times ("<< serviceType->name() <<")" <<endl;
   }
}

