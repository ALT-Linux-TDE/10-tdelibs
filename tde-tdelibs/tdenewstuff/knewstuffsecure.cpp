/***************************************************************************
                          knewstuffsecure.cpp  -  description
                             -------------------
    begin                : Tue Jun 22 12:19:55 2004
    copyright          : (C) 2004, 2005 by Andras Mantia <amantia@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; version 2 of the License.  *
 *                                                                         *
 ***************************************************************************/
//qt includes
#include <tqfileinfo.h>

//kde includes
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeio/netaccess.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <ktempdir.h>

//app includes
#include "engine.h"
#include "knewstuffsecure.h"
#include "security.h"

using namespace KNS;

TDENewStuffSecure::TDENewStuffSecure(const TQString &type,  TQWidget *parentWidget)
 : TDENewStuff(type, parentWidget)
{
  m_tempDir = 0L;
  connect(engine(), TQ_SIGNAL(uploadFinished(bool)), TQ_SLOT(slotUploadFinished(bool)));
}


TDENewStuffSecure::~TDENewStuffSecure()
{
  removeTempDirectory();
}

bool TDENewStuffSecure::install(const TQString &fileName)
{
  bool ok = true;

  removeTempDirectory();
  m_tempDir = new KTempDir();
  m_tempDir->setAutoDelete(true);
  KTar tar(fileName, "application/x-gzip");
  if (tar.open(IO_ReadOnly))
  {
      const KArchiveDirectory *directory = tar.directory();
      directory->copyTo(m_tempDir->name(), true);
      m_tarName = "";
      TQStringList entries = directory->entries();
      for (TQStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
      {
        if (*it != "signature" && *it != "md5sum")
        {
          m_tarName = *it;
          break;
        }
      }
      tar.close();
      if (m_tarName.isEmpty())
        ok = false;
      else
      {
         m_tarName.prepend(m_tempDir->name());
         connect(Security::ref(), TQ_SIGNAL(validityResult(int)), this, TQ_SLOT(slotValidated(int)));
         Security::ref()->checkValidity(m_tarName);
      }
  } else
      ok = false;
  if (!ok)
    KMessageBox::error(parentWidget(), i18n("There was an error with the downloaded resource tarball file. Possible causes are damaged archive or invalid directory structure in the archive."), i18n("Resource Installation Error"));
  return ok;
}

void TDENewStuffSecure::slotValidated(int result)
{
   TQString errorString;
   TQString signatureStr;
   bool valid = true;
   if (result == -1)
   {
     errorString ="<br>-    " +  i18n("No keys were found.");
     valid = false;
   } else
   if (result == 0)
   {
       errorString ="<br>-    " +  i18n("The validation failed for unknown reason.");
       valid = false;
   } else
   {
      KeyStruct key = Security::ref()->signatureKey();
      if (!(result & Security::MD5_OK ))
      {
          errorString = "<br>-    " + i18n("The MD5SUM check failed, the archive might be broken.");
          valid = false;
      }
      if (result & Security::SIGNED_BAD)
      {
          errorString += "<br>-    " + i18n("The signature is bad, the archive might be broken or altered.");
          valid = false;
      }
      if (result & Security::SIGNED_OK)
      {
         if (result & Security::TRUSTED)
         {
            kdDebug() << "Signed and trusted " << endl;
         } else
         {
            errorString += "<br>-    " + i18n("The signature is valid, but untrusted.");
            valid = false;
         }
      }
      if (result & Security::UNKNOWN)
      {
          errorString += "<br>-    " + i18n("The signature is unknown.");
          valid = false;
      } else
      {
          signatureStr = i18n("The resource was signed with key <i>0x%1</i>, belonging to <i>%2 &lt;%3&gt;</i>.").arg(key.id.right(8)).arg(key.name).arg(key.mail);
      }
   }
  if (!valid)
  {
      signatureStr.prepend( "<br>");
      if (KMessageBox::warningContinueCancel(parentWidget(), i18n("<qt>There is a problem with the resource file you have downloaded. The errors are :<b>%1</b><br>%2<br><br>Installation of the resource is <b>not recommended</b>.<br><br>Do you want to proceed with the installation?</qt>").arg(errorString).arg(signatureStr), i18n("Problematic Resource File")) == KMessageBox::Continue)
          valid = true;
  } else
    KMessageBox::information(parentWidget(), i18n("<qt>%1<br><br>Press OK to install it.</qt>").arg(signatureStr), i18n("Valid Resource"), "Show Valid Signature Information");
  if (valid)
  {
     installResource();
     emit installFinished();
  } else
  {
    TDEConfig *cfg = TDEGlobal::config();
    cfg->deleteGroup("TDENewStuffStatus");
    cfg->setGroup("TDENewStuffStatus");
    for (TQMap<TQString, TQString>::ConstIterator it = m_installedResources.constBegin(); it != m_installedResources.constEnd(); ++it)
    {
      cfg->writeEntry(it.key(), it.data());
    }
    cfg->sync();
  }
  removeTempDirectory();
  disconnect(Security::ref(), TQ_SIGNAL(validityResult(int)), this, TQ_SLOT(slotValidated(int)));
}

void TDENewStuffSecure::downloadResource()
{
  TDEConfig *cfg = TDEGlobal::config();
  m_installedResources = cfg->entryMap("TDENewStuffStatus");
  engine()->ignoreInstallResult(true);
  TDENewStuff::download();
}

bool TDENewStuffSecure::createUploadFile(const TQString &fileName)
{
  Q_UNUSED(fileName);
  return true; 
}

void TDENewStuffSecure::uploadResource(const TQString& fileName)
{
  connect(Security::ref(), TQ_SIGNAL(fileSigned(int)), this, TQ_SLOT(slotFileSigned(int)));
  removeTempDirectory();
  m_tempDir = new KTempDir();
  m_tempDir->setAutoDelete(true);
  TQFileInfo f(fileName);
  m_signedFileName = m_tempDir->name() + "/" + f.fileName();
  TDEIO::NetAccess::file_copy(KURL::fromPathOrURL(fileName), KURL::fromPathOrURL(m_signedFileName), -1, true);
  Security::ref()->signFile(m_signedFileName);
}

void TDENewStuffSecure::slotFileSigned(int result)
{
  if (result == 0)
  {
    KMessageBox::error(parentWidget(), i18n("The signing failed for unknown reason."));    
  } else
  {
    if (result & Security::BAD_PASSPHRASE)
    {
      if (KMessageBox::warningContinueCancel(parentWidget(), i18n("There are no keys usable for signing or you did not entered the correct passphrase.\nProceed without signing the resource?")) == KMessageBox::Cancel)
      {
        disconnect(Security::ref(), TQ_SIGNAL(fileSigned(int)), this, TQ_SLOT(slotFileSigned(int)));
        removeTempDirectory();
        return;    
      }
    } 
    KTar tar(m_signedFileName + ".signed", "application/x-gzip");
    tar.open(IO_WriteOnly);
    TQStringList files;
    files << m_signedFileName;
    files << m_tempDir->name() + "/md5sum";
    files << m_tempDir->name() + "/signature";
  
    for (TQStringList::Iterator it_f = files.begin(); it_f != files.end(); ++it_f)
    {
      TQFile file(*it_f);
      file.open(IO_ReadOnly);
      TQByteArray bArray = file.readAll();
      tar.writeFile(TQFileInfo(file).fileName(), "user", "group", bArray.size(), bArray.data());
      file.close();
    }
    tar.close();
    TDEIO::NetAccess::file_move(KURL::fromPathOrURL(m_signedFileName + ".signed"), KURL::fromPathOrURL(m_signedFileName), -1, true);
    TDENewStuff::upload(m_signedFileName, TQString::null);
    disconnect(Security::ref(), TQ_SIGNAL(fileSigned(int)), this, TQ_SLOT(slotFileSigned(int)));
  }
}

void TDENewStuffSecure::slotUploadFinished(bool result)
{
  Q_UNUSED(result);
  removeTempDirectory();
}

void TDENewStuffSecure::removeTempDirectory()
{
  if (m_tempDir)
  {
    TDEIO::NetAccess::del(KURL().fromPathOrURL(m_tempDir->name()), parentWidget());
    delete m_tempDir;
    m_tempDir = 0L;
  }
}

#include "knewstuffsecure.moc"
