/***************************************************************************
                          security.cpp  -  description
                             -------------------
    begin                : Thu Jun 24 11:22:12 2004
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
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqstringlist.h>
#include <tqtimer.h>

 //kde includes
#include <kdebug.h>
#include <kinputdialog.h>
#include <tdelocale.h>
#include <kmdcodec.h>
#include <tdemessagebox.h>
#include <kpassdlg.h>
#include <kprocio.h>

 //app includes
#include "security.h"

using namespace KNS;

Security::Security()
{
   m_keysRead = false;
   m_gpgRunning = false;
   readKeys();
   readSecretKeys();
}


Security::~Security()
{
}

void Security::readKeys()
{
  if (m_gpgRunning)
  {
    TQTimer::singleShot(5, this, TQ_SLOT(readKeys()));
    return;
  }
  m_runMode = List;
  m_keys.clear();
  KProcIO *readProcess=new KProcIO();
  *readProcess << "gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--with-colon"<<"--list-keys";
  connect(readProcess, TQ_SIGNAL(processExited(TDEProcess *)), this, TQ_SLOT(slotProcessExited(TDEProcess *)));
  connect(readProcess, TQ_SIGNAL(readReady(KProcIO *)) ,this, TQ_SLOT(slotDataArrived(KProcIO *)));
  if (!readProcess->start(TDEProcess::NotifyOnExit, true))
    KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and retrieve the available keys. Make sure that <i>gpg</i> is installed, otherwise verification of downloaded resources will not be possible.</qt>"));
  else
    m_gpgRunning = true;
}

void Security::readSecretKeys()
{
  if (m_gpgRunning)
  {
    TQTimer::singleShot(5, this, TQ_SLOT(readSecretKeys()));
    return;
  }
  m_runMode = ListSecret;
  KProcIO *readProcess=new KProcIO();
  *readProcess << "gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--with-colon"<<"--list-secret-keys";
  connect(readProcess, TQ_SIGNAL(processExited(TDEProcess *)), this, TQ_SLOT(slotProcessExited(TDEProcess *)));
  connect(readProcess, TQ_SIGNAL(readReady(KProcIO *)) ,this, TQ_SLOT(slotDataArrived(KProcIO *)));
  if (readProcess->start(TDEProcess::NotifyOnExit, true))
    m_gpgRunning = true;  
}

void Security::slotProcessExited(TDEProcess *process)
{
  switch (m_runMode)
   {
     case ListSecret:
                  m_keysRead = true;
                  break;
     case Verify: emit validityResult(m_result);
                  break;
     case Sign:   emit fileSigned(m_result);
                  break;

   }
   m_gpgRunning = false;
   delete process;
}

void Security::slotDataArrived(KProcIO *procIO)
{
  TQString data;
  while (procIO->readln(data, true) != -1)
  {
     switch (m_runMode)
     {
        case List:
        case ListSecret:  
          if (data.startsWith("pub") || data.startsWith("sec"))
          {
              KeyStruct key;
              if (data.startsWith("pub"))
                key.secret = false;
              else
                key.secret = true;
              TQStringList line = TQStringList::split(":", data, true);
              key.id = line[4];
              TQString shortId = key.id.right(8);
              TQString trustStr = line[1];
              key.trusted = false;
              if (trustStr == "u" || trustStr == "f")
                  key.trusted = true;
              data = line[9];
              key.mail=data.section('<', -1, -1);
              key.mail.truncate(key.mail.length() - 1);
              key.name=data.section('<',0,0);
              if (key.name.find("(")!=-1)
                  key.name=key.name.section('(',0,0);
              m_keys[shortId] = key;
          }
          break;
       case Verify:
          data = TQString(data.section("]",1,-1)).stripWhiteSpace();
          if (data.startsWith("GOODSIG"))
          {
              m_result &= SIGNED_BAD_CLEAR;
              m_result |= SIGNED_OK;
              TQString id = data.section(" ", 1 , 1).right(8);
              if (!m_keys.contains(id))
              {
                  m_result |= UNKNOWN;
              } else
              {
                 m_signatureKey = m_keys[id];
              }
          } else
          if (data.startsWith("NO_PUBKEY"))
          {
              m_result &= SIGNED_BAD_CLEAR;
              m_result |= UNKNOWN;
          } else
          if (data.startsWith("BADSIG"))
          {
              m_result |= SIGNED_BAD;
              TQString id = data.section(" ", 1 , 1).right(8);
              if (!m_keys.contains(id))
              {
                  m_result |= UNKNOWN;
              } else
              {
                 m_signatureKey = m_keys[id];
              }
          } else
          if (data.startsWith("TRUST_ULTIMATE"))
          {
            m_result &= SIGNED_BAD_CLEAR;
            m_result |= TRUSTED;
          }
          break;

       case Sign:
         if (data.find("passphrase.enter") != -1)
         {
           TQString password;
           KeyStruct key = m_keys[m_secretKey];
           int result = KPasswordDialog::getPassword(password, i18n("<qt>Enter passphrase for key <b>0x%1</b>, belonging to<br><i>%2&lt;%3&gt;</i>:</qt>").arg(m_secretKey).arg(key.name).arg(key.mail));
           if (result == KPasswordDialog::Accepted)
           {
             procIO->writeStdin(password, true);
             password.fill(' ');
           }
           else
           {
             m_result |= BAD_PASSPHRASE;
             slotProcessExited(procIO);
             return;
           }
         } else
         if (data.find("BAD_PASSPHRASE") != -1)
         {
           m_result |= BAD_PASSPHRASE;
         }
         break;
     }
  }
}

void Security::checkValidity(const TQString& filename)
{
  m_fileName = filename;
  slotCheckValidity();
}  

void Security::slotCheckValidity()
{  
  if (!m_keysRead || m_gpgRunning)
  {
    TQTimer::singleShot(5, this, TQ_SLOT(slotCheckValidity()));
    return;
  }
  if (m_keys.count() == 0)
  {    
    emit validityResult(-1);
    return;
  }  

  m_result = 0;
  m_runMode = Verify;
  TQFileInfo f(m_fileName);
  //check the MD5 sum
  TQString md5sum;
  const char* c = "";
  KMD5 context(c);
  TQFile file(m_fileName);
  if (file.open(IO_ReadOnly))
  {
     context.reset();
     context.update(file);
     md5sum = context.hexDigest();
     file.close();
  }
  file.setName(f.dirPath() + "/md5sum");
  if (file.open(IO_ReadOnly))
  {
     TQString md5sum_file;
     file.readLine(md5sum_file, 50);
     if (!md5sum.isEmpty() && !md5sum_file.isEmpty() && md5sum_file.startsWith(md5sum))
       m_result |= MD5_OK;
     file.close();
  }
  m_result |= SIGNED_BAD;
  m_signatureKey.id = "";
  m_signatureKey.name = "";
  m_signatureKey.mail = "";
  m_signatureKey.trusted = false;

  //verify the signature
  KProcIO *verifyProcess=new KProcIO();
  *verifyProcess<<"gpg"<<"--no-secmem-warning"<<"--status-fd=2"<<"--command-fd=0"<<"--verify" << f.dirPath() + "/signature"<< m_fileName;
  connect(verifyProcess, TQ_SIGNAL(processExited(TDEProcess *)),this, TQ_SLOT(slotProcessExited(TDEProcess *)));
  connect(verifyProcess, TQ_SIGNAL(readReady(KProcIO *)),this, TQ_SLOT(slotDataArrived(KProcIO *)));
  if (verifyProcess->start(TDEProcess::NotifyOnExit,true))
      m_gpgRunning = true;
  else
  {
      KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and check the validity of the file. Make sure that <i>gpg</i> is installed, otherwise verification of downloaded resources will not be possible.</qt>"));
      emit validityResult(0);
      delete verifyProcess;
  }
}

void Security::signFile(const TQString &fileName)
{
  m_fileName = fileName;
  slotSignFile();
}

void Security::slotSignFile()
{
  if (!m_keysRead || m_gpgRunning)
  {
    TQTimer::singleShot(5, this, TQ_SLOT(slotSignFile()));
    return;
  }
  
  TQStringList secretKeys;
  for (TQMap<TQString, KeyStruct>::Iterator it = m_keys.begin(); it != m_keys.end(); ++it)
  {
    if (it.data().secret)
      secretKeys.append(it.key());
  }
  
  if (secretKeys.count() == 0)
  {    
    emit fileSigned(-1);
    return;
  }  
  
  m_result = 0;
  TQFileInfo f(m_fileName);

  //create the MD5 sum
  TQString md5sum;
  const char* c = "";
  KMD5 context(c);
  TQFile file(m_fileName);
  if (file.open(IO_ReadOnly))
  {
    context.reset();
    context.update(file);
    md5sum = context.hexDigest();
    file.close();
  }
  file.setName(f.dirPath() + "/md5sum");
  if (file.open(IO_WriteOnly))
  {
    TQTextStream stream(&file);
    stream << md5sum;
    m_result |= MD5_OK;
    file.close();
  }
  
  if (secretKeys.count() > 1)
  {
    bool ok;
    secretKeys = KInputDialog::getItemList(i18n("Select Signing Key"), i18n("Key used for signing:"), secretKeys, secretKeys[0], false, &ok);    
    if (ok)
      m_secretKey = secretKeys[0];
    else
    {
      emit fileSigned(0);
      return;
    }
  } else
    m_secretKey = secretKeys[0];

  //verify the signature
  KProcIO *signProcess=new KProcIO();
  *signProcess<<"gpg"<<"--no-secmem-warning"<<"--status-fd=2"<<"--command-fd=0"<<"--no-tty"<<"--detach-sign" << "-u" << m_secretKey << "-o" << f.dirPath() + "/signature" << m_fileName;
  connect(signProcess, TQ_SIGNAL(processExited(TDEProcess *)),this, TQ_SLOT(slotProcessExited(TDEProcess *)));
  connect(signProcess, TQ_SIGNAL(readReady(KProcIO *)),this, TQ_SLOT(slotDataArrived(KProcIO *)));
  m_runMode = Sign;
  if (signProcess->start(TDEProcess::NotifyOnExit,true))
    m_gpgRunning = true;
  else
  {
    KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and sign the file. Make sure that <i>gpg</i> is installed, otherwise signing of the resources will not be possible.</qt>"));
    emit fileSigned(0);
    delete signProcess;
  }
}

#include "security.moc"
