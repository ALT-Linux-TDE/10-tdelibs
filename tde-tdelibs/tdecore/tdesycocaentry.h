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

#ifndef __tdesycocaentry_h__
#define __tdesycocaentry_h__

#include "tdesycocatype.h"

#include <tqstringlist.h>
#include <ksharedptr.h>
class TQDataStream;

/**
 * Base class for all Sycoca entries.
 * 
 * You can't create an instance of KSycocaEntry, but it provides
 * the common functionality for servicetypes and services.
 * 
 * @internal
 * @see http://developer.kde.org/documentation/library/kdetqt/trinityarch/tdesycoca.html 
 */
class TDECORE_EXPORT KSycocaEntry : public TDEShared
{
 
public:
   virtual bool isType(KSycocaType t) const { return (t == KST_KSycocaEntry); }
   virtual KSycocaType sycocaType() const { return KST_KSycocaEntry; }

public:
  typedef TDESharedPtr<KSycocaEntry> Ptr;
  typedef TQValueList<Ptr> List;
public: // KDoc seems to barf on those typedefs and generates no docs after them
   /**
    * Default constructor
    */
   KSycocaEntry(const TQString &path) : mOffset(0), m_bDeleted(false), mPath(path) { }

   /**
    * Safe demarshalling functions.
    */
   static void read( TQDataStream &s, TQString &str );
   static void read( TQDataStream &s, TQStringList &list );

   /**
    * @internal
    * Restores itself from a stream.
    */
   KSycocaEntry( TQDataStream &_str, int offset ) : 
              mOffset( offset ), m_bDeleted(false) 
   { 
     read(_str, mPath);
   }

   /**
    * @return the name of this entry
    */
   virtual TQString name() const = 0;

   /**
    * @return the path of this entry 
    * The path can be absolute or relative.
    * The corresponding factory should know relative to what.
    */
   TQString entryPath() const { return mPath; }
  
   /**
    * @return true if valid
    */
   virtual bool isValid() const = 0;
  
   /**
    * @return true if deleted
    */
   virtual bool isDeleted() const { return m_bDeleted; }

   /**
    * @internal
    * @return the position of the entry in the sycoca file
    */
   int offset() { return mOffset; }

   /**
    * @internal
    * Save ourselves to the database. Don't forget to call the parent class
    * first if you override this function.
    */
   virtual void save(TQDataStream &s)
     {
       mOffset = s.device()->at(); // store position in member variable
       s << (TQ_INT32) sycocaType() << mPath;
     }

   /**
    * @internal
    * Load ourselves from the database. Don't call the parent class!
    */
   virtual void load(TQDataStream &) = 0;

private:
   int mOffset;
protected:
   bool m_bDeleted;
   TQString mPath;   
protected:
   virtual void virtual_hook( int id, void* data );
};

#endif
