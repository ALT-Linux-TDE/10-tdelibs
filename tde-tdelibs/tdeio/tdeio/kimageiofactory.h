/*
* kimgio.h -- Declaration of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#ifndef SSK_KIMGIOFACTORY_H
#define SSK_KIMGIOFACTORY_H

#include "tdesycocafactory.h"
#include "kimageio.h"

class KImageIOFormat;
class KImageIOFormatList;

/** \internal */
class TDEIO_EXPORT KImageIOFormat : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KImageIOFormat, KSycocaEntry )

public:
  typedef TDESharedPtr<KImageIOFormat> Ptr;
  typedef TQValueList<Ptr> List;
public: // KDoc seems to barf on those typedefs and generates no docs after them
  /**
   * Read a KImageIOFormat description file
   */
  KImageIOFormat( const TQString & path);
  
  /**
   * @internal construct a ImageIOFormat from a stream
   */ 
  KImageIOFormat( TQDataStream& _str, int offset);

  virtual ~KImageIOFormat();

  virtual TQString name() const { return mType; }

  virtual bool isValid() const { return true; } 

  /**
   * @internal
   * Load the image format from a stream.
   */
  virtual void load(TQDataStream& ); 

  /**
   * @internal
   * Save the image format to a stream.
   */
  virtual void save(TQDataStream& );

  /**
   * @internal 
   * Calls image IO function
   */
  void callLibFunc( bool read, TQImageIO *);

public:  
  TQString mType;
  TQString mHeader;
  TQString mFlags;
  bool bRead;
  bool bWrite;
  TQStringList mSuffices;
  TQString mPattern;
  TQString mMimetype;
  TQString mLib;
  TQStringList rPaths;
  bool bLibLoaded;
  void (*mReadFunc)(TQImageIO *);
  void (*mWriteFunc)(TQImageIO *);
protected:
  virtual void virtual_hook( int id, void* data );
};

/** \internal */
class TDEIO_EXPORT KImageIOFormatList : public KImageIOFormat::List
{
public:
   KImageIOFormatList() { }
};


/** \internal */
class TDEIO_EXPORT KImageIOFactory : public KSycocaFactory
{
  friend class KImageIO;
  K_SYCOCAFACTORY( KST_KImageIO )
public:
  static KImageIOFactory *self() 
  { if (!_self) new KImageIOFactory(); return _self; }
  KImageIOFactory();
  virtual ~KImageIOFactory();

protected: // Internal stuff
  /**
   * @internal
   *
   * Load information from database
   */
  void load();

  /**
   * @internal Create pattern string
   **/
  TQString createPattern( KImageIO::Mode _mode);

  /**
   * @internal Not used.
   */
  virtual KSycocaEntry *createEntry(const TQString &, const char *)
    { return 0; }                                                    

  /**
   * @internal 
   */
  virtual KSycocaEntry *createEntry(int offset);

  /**
   * @internal Read an image
   **/
  static void readImage( TQImageIO *iio);

  /**
   * @internal Write an image
   **/
  static void writeImage( TQImageIO *iio);
  
protected:
  static KImageIOFactory *_self;  
  static KImageIOFormatList *formatList;
  TQString mReadPattern;
  TQString mWritePattern;
  TQStringList rPath;
protected:
    virtual void virtual_hook( int id, void* data );
};

#endif

