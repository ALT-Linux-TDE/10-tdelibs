/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */


#include <tdeaboutdata.h>
#include <kstandarddirs.h>
#include <tqfile.h>
#include <tqtextstream.h>

TQString
TDEAboutPerson::name() const
{
   return TQString::fromUtf8(mName);
}

TQString
TDEAboutPerson::task() const
{
   if (mTask && *mTask)
      return i18n(mTask);
   else
      return TQString::null;
}

TQString
TDEAboutPerson::emailAddress() const
{
   return TQString::fromUtf8(mEmailAddress);
}


TQString
TDEAboutPerson::webAddress() const
{
   return TQString::fromUtf8(mWebAddress);
}


TDEAboutTranslator::TDEAboutTranslator(const TQString & name,
                const TQString & emailAddress)
{
    mName=name;
    mEmail=emailAddress;
}

TQString TDEAboutTranslator::name() const
{
    return mName;
}

TQString TDEAboutTranslator::emailAddress() const
{
    return mEmail;
}

class TDEAboutDataPrivate
{
public:
    TDEAboutDataPrivate()
        : translatorName("_: NAME OF TRANSLATORS\nYour names")
        , translatorEmail("_: EMAIL OF TRANSLATORS\nYour emails")
        , productName(0)
        , programLogo(0)
        , customAuthorTextEnabled(false)
        , mTranslatedProgramName( 0 )
        {}
    ~TDEAboutDataPrivate()
        {
             delete programLogo;
             delete[] mTranslatedProgramName;
        }
    const char *translatorName;
    const char *translatorEmail;
    const char *productName;
    TQImage* programLogo;
    TQString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;
    const char *mTranslatedProgramName;
};

const char *TDEAboutData::defaultBugTracker = "http://bugs.trinitydesktop.org";

TDEAboutData::TDEAboutData( const char *appName,
                        const char *programName,
                        const char *version,
                        const char *shortDescription,
			int licenseType,
			const char *copyrightStatement,
			const char *text,
			const char *homePageAddress,
			const char *bugsEmailAddress
			) :
  mProgramName( programName ),
  mVersion( version ),
  mShortDescription( shortDescription ),
  mLicenseKey( licenseType ),
  mCopyrightStatement( copyrightStatement ),
  mOtherText( text ),
  mHomepageAddress( homePageAddress ),
  mBugEmailAddress( (bugsEmailAddress!=0)?bugsEmailAddress:defaultBugTracker ),
  mLicenseText (0)
{
   d = new TDEAboutDataPrivate;

   if( appName ) {
     const char *p = strrchr(appName, '/');
     if( p )
	 mAppName = p+1;
     else
	 mAppName = appName;
   } else
     mAppName = 0;
}

TDEAboutData::~TDEAboutData()
{
    if (mLicenseKey == License_File)
        delete [] mLicenseText;
    delete d;
}

void
TDEAboutData::addAuthor( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  mAuthorList.append(TDEAboutPerson(name,task,emailAddress,webAddress));
}

void
TDEAboutData::addCredit( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  mCreditList.append(TDEAboutPerson(name,task,emailAddress,webAddress));
}

void
TDEAboutData::setTranslator( const char *name, const char *emailAddress)
{
  d->translatorName=name;
  d->translatorEmail=emailAddress;
}

void
TDEAboutData::setLicenseText( const char *licenseText )
{
  mLicenseText = licenseText;
  mLicenseKey = License_Custom;
}

void
TDEAboutData::setLicenseTextFile( const TQString &file )
{
  mLicenseText = tqstrdup(TQFile::encodeName(file));
  mLicenseKey = License_File;
}

void
TDEAboutData::setAppName( const char *appName )
{
  mAppName = appName;
}

void
TDEAboutData::setProgramName( const char* programName )
{
  mProgramName = programName;
  translateInternalProgramName();
}

void
TDEAboutData::setVersion( const char* version )
{
  mVersion = version;
}

void
TDEAboutData::setShortDescription( const char *shortDescription )
{
  mShortDescription = shortDescription;
}

void
TDEAboutData::setLicense( LicenseKey licenseKey)
{
  mLicenseKey = licenseKey;
}

void
TDEAboutData::setCopyrightStatement( const char *copyrightStatement )
{
  mCopyrightStatement = copyrightStatement;
}

void
TDEAboutData::setOtherText( const char *otherText )
{
  mOtherText = otherText;
}

void
TDEAboutData::setHomepage( const char *homepage )
{
  mHomepageAddress = homepage;
}

void
TDEAboutData::setBugAddress( const char *bugAddress )
{
  mBugEmailAddress = bugAddress;
}

void
TDEAboutData::setProductName( const char *productName )
{
  d->productName = productName;
}

const char *
TDEAboutData::appName() const
{
   return mAppName;
}

const char *
TDEAboutData::productName() const
{
   if (d->productName)
      return d->productName;
   else
      return appName();
}

TQString
TDEAboutData::programName() const
{
   if (mProgramName && *mProgramName)
      return i18n(mProgramName);
   else
      return TQString::null;
}

const char*
TDEAboutData::internalProgramName() const
{
   if (d->mTranslatedProgramName)
      return d->mTranslatedProgramName;
   else
      return mProgramName;
}

// TDECrash should call as few things as possible and should avoid e.g. malloc()
// because it may deadlock. Since i18n() needs it, when TDELocale is available
// the i18n() call will be done here in advance.
void
TDEAboutData::translateInternalProgramName() const
{
  delete[] d->mTranslatedProgramName;
  d->mTranslatedProgramName = 0;
  if( TDEGlobal::locale() )
      d->mTranslatedProgramName = tqstrdup( programName().utf8());
}

TQImage
TDEAboutData::programLogo() const
{
    return d->programLogo ? (*d->programLogo) : TQImage();
}

void
TDEAboutData::setProgramLogo(const TQImage& image)
{
    if (!d->programLogo)
       d->programLogo = new TQImage( image );
    else
       *d->programLogo = image;
}

TQString
TDEAboutData::version() const
{
   return TQString::fromLatin1(mVersion);
}

TQString
TDEAboutData::shortDescription() const
{
   if (mShortDescription && *mShortDescription)
      return i18n(mShortDescription);
   else
      return TQString::null;
}

TQString
TDEAboutData::homepage() const
{
   return TQString::fromLatin1(mHomepageAddress);
}

TQString
TDEAboutData::bugAddress() const
{
   return TQString::fromLatin1(mBugEmailAddress);
}

const TQValueList<TDEAboutPerson>
TDEAboutData::authors() const
{
   return mAuthorList;
}

const TQValueList<TDEAboutPerson>
TDEAboutData::credits() const
{
   return mCreditList;
}

const TQValueList<TDEAboutTranslator>
TDEAboutData::translators() const
{
    TQValueList<TDEAboutTranslator> personList;

    if(d->translatorName == 0)
        return personList;

    TQStringList nameList;
    TQStringList emailList;

    TQString names = i18n(d->translatorName);
    if(names != TQString::fromUtf8(d->translatorName))
    {
        nameList = TQStringList::split(',',names);
    }


    if(d->translatorEmail)
    {
        TQString emails = i18n(d->translatorEmail);

        if(emails != TQString::fromUtf8(d->translatorEmail))
        {
            emailList = TQStringList::split(',',emails,true);
        }
    }


    TQStringList::Iterator nit;
    TQStringList::Iterator eit=emailList.begin();

    for(nit = nameList.begin(); nit != nameList.end(); ++nit)
    {
        TQString email;
        if(eit != emailList.end())
        {
            email=*eit;
            ++eit;
        }

        TQString name=*nit;

        personList.append(TDEAboutTranslator(name.stripWhiteSpace(), email.stripWhiteSpace()));
    }

    return personList;
}

TQString
TDEAboutData::aboutTranslationTeam()
{
    return i18n("replace this with information about your translation team",
            "<p>TDE is translated into many languages thanks to the work "
            "of the translation teams all over the world.</p>"
            "<p>For more information on TDE internationalization "
            "visit the <a href=\"https://wiki.trinitydesktop.org/"
            "TDE_Weblate_Translation_Workspace\">TDE Weblate "
            "Translation Workspace (TWTW)</a></p>"
            );
}

TQString
TDEAboutData::otherText() const
{
   if (mOtherText && *mOtherText)
      return i18n(mOtherText);
   else
      return TQString::null;
}


TQString
TDEAboutData::license() const
{
  TQString result;
  if (!copyrightStatement().isEmpty())
    result = copyrightStatement() + "\n\n";

  TQString l;
  TQString f;
  switch ( mLicenseKey )
  {
    case License_File:
       f = TQFile::decodeName(mLicenseText);
       break;
    case License_GPL_V2:
       l = "GPL v2";
       f = locate("data", "LICENSES/GPL_V2");
       break;
    case License_LGPL_V2:
       l = "LGPL v2";
       f = locate("data", "LICENSES/LGPL_V2");
       break;
    case License_GPL_V3:
       l = "GPL v3";
       f = locate("data", "LICENSES/GPL_V3");
       break;
    case License_LGPL_V3:
       l = "LGPL v3";
       f = locate("data", "LICENSES/LGPL_V3");
       break;
    case License_BSD:
       l = "BSD License";
       f = locate("data", "LICENSES/BSD");
       break;
    case License_Artistic:
       l = "Artistic License";
       f = locate("data", "LICENSES/ARTISTIC");
       break;
    case License_QPL_V1_0:
       l = "QPL v1.0";
       f = locate("data", "LICENSES/QPL_V1.0");
       break;
    case License_MIT:
       l = "MIT";
       f = locate("data", "LICENSES/MIT");
       break;
    case License_Custom:
       if (mLicenseText && *mLicenseText)
          return( i18n(mLicenseText) );
       // fall through
    default:
       result += i18n("No licensing terms for this program have been specified.\n"
                   "Please check the documentation or the source for any\n"
                   "licensing terms.\n");
       return result;
      }

  if (!l.isEmpty())
     result += i18n("This program is distributed under the terms of the %1.").arg( l );

  if (!f.isEmpty())
  {
     TQFile file(f);
     if (file.open(IO_ReadOnly))
     {
        result += '\n';
        result += '\n';
        TQTextStream str(&file);
        result += str.read();
     }
  }

  return result;
}

TQString
TDEAboutData::copyrightStatement() const
{
  if (mCopyrightStatement && *mCopyrightStatement)
     return i18n(mCopyrightStatement);
  else
     return TQString::null;
}

TQString
TDEAboutData::customAuthorPlainText() const
{
  return d->customAuthorPlainText;
}

TQString
TDEAboutData::customAuthorRichText() const
{
  return d->customAuthorRichText;
}

bool
TDEAboutData::customAuthorTextEnabled() const
{
  return d->customAuthorTextEnabled;
}
    
void
TDEAboutData::setCustomAuthorText(const TQString &plainText, const TQString &richText)
{
  d->customAuthorPlainText = plainText;
  d->customAuthorRichText = richText;

  d->customAuthorTextEnabled = true;
}
    
void
TDEAboutData::unsetCustomAuthorText()
{
  d->customAuthorPlainText = TQString::null;
  d->customAuthorRichText = TQString::null;

  d->customAuthorTextEnabled = false;
}

