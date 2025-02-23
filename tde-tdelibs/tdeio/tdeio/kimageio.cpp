
/**
* kimgio.h -- Implementation of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* $Id$
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#include"config.h"

#include <tqdir.h>
#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <tqstring.h>
#include <tqregexp.h>
#include <tqvaluelist.h>

#include <ltdl.h>
#include "kimageio.h"
#include "kimageiofactory.h"
#include <tdelocale.h>
#include <klibloader.h>
#include <tdeglobal.h>
#include <kmimetype.h>
#include <tdesycocaentry.h>
#include <tdesycoca.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include <tqimage.h>

KImageIOFormat::KImageIOFormat( const TQString &path)
  : KSycocaEntry(path)
{
   bLibLoaded = false;
   mReadFunc = 0;
   mWriteFunc = 0;
   TDEConfig config(path, true, false);

   config.setGroup("Image Format");
   mType = config.readEntry("Type");
   mHeader = KURL::decode_string(config.readEntry("Header"), 4); // Latin1
   mFlags = config.readEntry("Flags");
   bRead = config.readBoolEntry("Read");
   bWrite = config.readBoolEntry("Write");
   mSuffices = config.readListEntry("Suffices");
   mPattern = config.readEntry("Name");
   mMimetype = config.readEntry("Mimetype");
   mLib = config.readPathEntry("Library");
   rPaths = config.readPathListEntry("rPaths");
}

KImageIOFormat::KImageIOFormat( TQDataStream& _str, int offset) :
	KSycocaEntry( _str, offset)
{
   bLibLoaded = false;
   mReadFunc = 0;
   mWriteFunc = 0;
   load( _str );
}

KImageIOFormat::~KImageIOFormat()
{
}

void
KImageIOFormat::load( TQDataStream& _str)
{
   TQ_INT8 iRead, iWrite;
   KSycocaEntry::read(_str, mType);
   KSycocaEntry::read(_str, mHeader);
   KSycocaEntry::read(_str, mFlags);
   _str >> iRead >> iWrite;
   KSycocaEntry::read(_str, mSuffices);
   KSycocaEntry::read(_str, mMimetype);
   KSycocaEntry::read(_str, mLib);
   KSycocaEntry::read(_str, mPattern);
   KSycocaEntry::read(_str, rPaths);
   bRead = (iRead != 0);
   bWrite = (iWrite != 0);
}

void
KImageIOFormat::save( TQDataStream& _str)
{
   KSycocaEntry::save( _str );
   TQ_INT8 iRead = bRead ? 1 : 0;
   TQ_INT8 iWrite = bWrite ? 1 : 0;

   _str << mType << mHeader << mFlags << iRead << iWrite
        << mSuffices << mMimetype << mLib << mPattern << rPaths;
}

void
KImageIOFormat::callLibFunc( bool read, TQImageIO *iio)
{
   if (!bLibLoaded)
   {
      if (mLib.isEmpty())
      {
         iio->setStatus(1); // Error
         return;
      }
      TQString libpath = KLibLoader::findLibrary(mLib.ascii());
      if ( libpath.isEmpty())
      {
         iio->setStatus(1); // Error
         return;
      }
      lt_dlhandle libhandle = lt_dlopen( TQFile::encodeName(libpath) );
      if (libhandle == 0) {
         iio->setStatus(1); // error
         kdWarning() << "KImageIOFormat::callLibFunc: couldn't dlopen " << mLib << "(" << lt_dlerror() << ")" << endl;
         return;
      }
      bLibLoaded = true;
      TQString funcName;
      if (bRead)
      {
         funcName = "kimgio_"+mType.lower()+"_read";
         lt_ptr func = lt_dlsym(libhandle, funcName.ascii());

         if (func == NULL) {
            iio->setStatus(1); // error
            kdWarning() << "couln't find " << funcName << " (" << lt_dlerror() << ")" << endl;
         }
         mReadFunc = (void (*)(TQImageIO *))func;
      }
      if (bWrite)
      {
         funcName = "kimgio_"+mType.lower()+"_write";
         lt_ptr func = lt_dlsym(libhandle, funcName.ascii());

         if (func == NULL) {
            iio->setStatus(1); // error
            kdWarning() << "couln't find " << funcName << " (" << lt_dlerror() << ")" << endl;
         }
         mWriteFunc = (void (*)(TQImageIO *))func;
      }

   }
   if (read)
      if (mReadFunc)
         mReadFunc(iio);
      else
         iio->setStatus(1); // Error
   else
      if (mWriteFunc)
         mWriteFunc(iio);
      else
         iio->setStatus(1); // Error
}


KImageIOFactory *KImageIOFactory::_self = 0;
KImageIOFormatList *KImageIOFactory::formatList = 0;

static KStaticDeleter<KImageIOFormatList> kiioflsd;

KImageIOFactory::KImageIOFactory() : KSycocaFactory( KST_KImageIO )
{
  _self = this;
  if (m_str)
  {
     // read from database
     KSycocaEntry::read(*m_str, mReadPattern);
     KSycocaEntry::read(*m_str, mWritePattern);
     KSycocaEntry::read(*m_str, rPath);
     if (!formatList)
     {
        kiioflsd.setObject( formatList, new KImageIOFormatList());
        lt_dlinit(); // Do this only once!
        // Add rPaths.
        for(TQStringList::Iterator it = rPath.begin();
            it != rPath.end(); ++it)
           lt_dladdsearchdir( TQFile::encodeName(*it) );
     }
     load();
  }
  else
    if (KSycoca::self()->isBuilding())
    {
      // Build database
      if (!formatList)
      {
        formatList = new KImageIOFormatList();
      }
    } else
    {
      // We have no database at all.. uh-oh
    }
}

TQString
KImageIOFactory::createPattern( KImageIO::Mode _mode)
{
  TQStringList patterns;
  TQString allPatterns;
  TQString wildCard("*.");
  TQString separator("|");
  for( KImageIOFormatList::ConstIterator it = formatList->begin();
       it != formatList->end();
       ++it )
  {
     KImageIOFormat *format = (*it);
     if (((_mode == KImageIO::Reading) && format->bRead) ||
         ((_mode == KImageIO::Writing) && format->bWrite))
     {
        TQString pattern;
        TQStringList suffices = format->mSuffices;
        for( TQStringList::ConstIterator it = suffices.begin();
             it != suffices.end();
             ++it)
        {
           if (!pattern.isEmpty())
              pattern += " ";
           pattern = pattern + wildCard+(*it);
           if (!allPatterns.isEmpty())
              allPatterns += " ";
           allPatterns = allPatterns + wildCard +(*it);
        }
        if (!pattern.isEmpty())
        {
           pattern = pattern + separator + format->mPattern;
           patterns.append(pattern);
        }
     }
  }
  allPatterns = allPatterns + separator + i18n("All Pictures");
  patterns.sort();
  patterns.prepend(allPatterns);

  TQString pattern = patterns.join(TQString::fromLatin1("\n"));
  return pattern;
}

void
KImageIOFactory::readImage( TQImageIO *iio)
{
   (void) self(); // Make sure we exist
   const char *fm = iio->format();
   if (!fm)
      fm = TQImageIO::imageFormat( iio->ioDevice());
   kdDebug() << "KImageIO: readImage() format = " <<  fm << endl;

   KImageIOFormat *format = 0;
   for( KImageIOFormatList::ConstIterator it = formatList->begin();
        it != formatList->end();
        ++it )
   {
      format = (*it);
      if (format->mType == fm)
         break;
   }
   if (!format || !format->bRead)
   {
      iio->setStatus(1); // error
      return;
   }

   format->callLibFunc( true, iio);
}

void
KImageIOFactory::writeImage( TQImageIO *iio)
{
   (void) self(); // Make sure we exist
   const char *fm = iio->format();
   if (!fm)
      fm = TQImageIO::imageFormat( iio->ioDevice());
   kdDebug () << "KImageIO: writeImage() format = "<<  fm << endl;

   KImageIOFormat *format = 0;
   for( KImageIOFormatList::ConstIterator it = formatList->begin();
        it != formatList->end();
        ++it )
   {
      format = (*it);
      if (format->mType == fm)
         break;
   }
   if (!format || !format->bWrite)
   {
      iio->setStatus(1); // error
      return;
   }

   format->callLibFunc( false, iio);
}

void
KImageIOFactory::load()
{
   KSycocaEntry::List list = allEntries();
   for( KSycocaEntry::List::Iterator it = list.begin();
        it != list.end();
        ++it)
   {
      KSycocaEntry *entry = static_cast<KSycocaEntry *>(*it);
      KImageIOFormat *format = static_cast<KImageIOFormat *>(entry);

      // Since Qt doesn't allow us to unregister image formats
      // we have to make sure not to add them a second time.
      // This typically happens when the sycoca database was updated
      // we need to reread it.
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
         KImageIOFormat *_format = (*it);
         if (format->mType == _format->mType)
         {
            // Already in list
            format = 0;
            break;
         }
      }
      if (!format)
         continue;
      if (!format->mHeader.isEmpty() && !format->mLib.isEmpty())
      {
         void (*readFunc)(TQImageIO *);
         void (*writeFunc)(TQImageIO *);
         if (format->bRead)
            readFunc = readImage;
         else
            readFunc = 0;
         if (format->bWrite)
            writeFunc = writeImage;
         else
            writeFunc = 0;
         TQImageIO::defineIOHandler( format->mType.ascii(),
                                    format->mHeader.ascii(),
                                    format->mFlags.ascii(),
                                    readFunc, writeFunc);
      }
      formatList->append( format );
   }
}

KImageIOFactory::~KImageIOFactory()
{
  _self = 0;

  // We would like to:
  // * Free all KImageIOFormats.
  // * Unload libs
  // * Remove Qt IO handlers.
  // But we can't remove IO handlers, so we better keep all KImageIOFormats
  // in memory so that we can make sure not register IO handlers again whenever
  // the sycoca database updates (Such event deletes this factory)
}

KSycocaEntry*
KImageIOFactory::createEntry(int offset)
{
   KImageIOFormat *format = 0;
   KSycocaType type;
   TQDataStream *str = KSycoca::self()->findEntry(offset, type);
   switch (type)
   {
     case KST_KImageIOFormat:
       format = new KImageIOFormat(*str, offset);
       break;
     default:
       return 0;
   }
   if (!format->isValid())
   {
      delete format;
      format = 0;
   }
   return format;
}

void KImageIO::registerFormats()
{
   (void) KImageIOFactory::self();
}

TQString
KImageIO::pattern(Mode _mode)
{
  if (_mode == Reading)
     return KImageIOFactory::self()->mReadPattern;
  else
     return KImageIOFactory::self()->mWritePattern;
}

bool KImageIO::canWrite(const TQString& type)
{
  KImageIOFormatList *formatList = KImageIOFactory::self()->formatList;

  if(formatList)
  {
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
          KImageIOFormat *format = (*it);
          if (format->mType == type)
              return format->bWrite;
      }
  }

  return false;
}

bool KImageIO::canRead(const TQString& type)
{
  KImageIOFormatList *formatList = KImageIOFactory::self()->formatList;

  if(formatList)
  {
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
          KImageIOFormat *format = (*it);
          if (format->mType == type)
              return format->bRead;
      }
  }

  return false;
}

TQStringList KImageIO::types(Mode _mode ) {
  KImageIOFormatList *formatList = KImageIOFactory::self()->formatList;
  TQStringList types;

  if(formatList)
  {
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
          KImageIOFormat *format = (*it);
          if (((_mode == Reading) && format->bRead) ||
              ((_mode == Writing) && format->bWrite))
              types.append(format->mType);
      }
  }

  return types;
}

TQString KImageIO::suffix(const TQString& type)
{
  KImageIOFormatList *formatList = KImageIOFactory::self()->formatList;

  if(formatList)
  {
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
          KImageIOFormat *format = (*it);
          if (format->mType == type)
              return format->mSuffices[0];
      }
  }

  return TQString::null;
}

TQString KImageIO::typeForMime(const TQString& mimeType)
{
  KImageIOFormatList *formatList = KImageIOFactory::self()->formatList;

  if(formatList)
  {
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
          KImageIOFormat *format = (*it);
          if (format->mMimetype == mimeType)
              return format->mType;
      }
  }

  return TQString::null;
}

TQString KImageIO::type(const TQString& filename)
{
  KImageIOFormatList *formatList = KImageIOFactory::self()->formatList;
  TQString suffix = filename;
  int dot = suffix.findRev('.');
  if (dot >= 0)
    suffix = suffix.mid(dot + 1);

  if(formatList)
  {
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
          KImageIOFormat *format = (*it);
          if (format->mSuffices.contains(suffix))
              return format->mType;
      }
  }

  return TQString::null;
}

TQStringList KImageIO::mimeTypes( Mode _mode )
{
  KImageIOFormatList *formatList = KImageIOFactory::self()->formatList;
  TQStringList mimeList;

  if(formatList)
  {
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
          KImageIOFormat *format = (*it);
          if (((_mode == Reading) && format->bRead) ||
              ((_mode == Writing) && format->bWrite))
            if ( !format->mMimetype.isEmpty() )
              mimeList.append ( format->mMimetype );
      }
  }

  return mimeList;
}

bool KImageIO::isSupported( const TQString& _mimeType, Mode _mode )
{
  KImageIOFormatList *formatList = KImageIOFactory::self()->formatList;

  if(formatList)
  {
      for( KImageIOFormatList::ConstIterator it = formatList->begin();
           it != formatList->end();
           ++it )
      {
          KImageIOFormat *format = (*it);
          if (format->mMimetype == _mimeType)
          {
              if (((_mode == Reading) && format->bRead) ||
                  ((_mode == Writing) && format->bWrite))
                  return true;
          }
      }
  }

  return false;
}

TQString KImageIO::mimeType( const TQString& _filename )
{
  return KMimeType::findByURL( KURL( _filename ) )->name();
}

void KImageIOFormat::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }

void KImageIOFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }

