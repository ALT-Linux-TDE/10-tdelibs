/* This file is part of the KDE project
   Copyright (c) 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqdict.h>
#include <tqfile.h>
#include <tqtextstream.h>

#include <kdirwatch.h>
#include <kstaticdeleter.h>
#include <kdebug.h>
#include <ksimpleconfig.h>

#include "ksambashare.h"

class KSambaSharePrivate
{
public:
  KSambaSharePrivate();
  
  bool readSmbConf();
  bool findSmbConf();
  bool load();
  
  TQDict<bool> sharedPaths;
  TQString smbConf;
};

KSambaSharePrivate::KSambaSharePrivate() 
{
    load();
}  


#define FILESHARECONF "/etc/security/fileshare.conf"

bool KSambaSharePrivate::load() {
  if (!findSmbConf())
      return false;
      
  return readSmbConf();
}

/**
 * Try to find the samba config file path
 * First tries the tdeconfig, then checks
 * several well-known paths
 * @return wether a smb.conf was found.
 **/
bool KSambaSharePrivate::findSmbConf() {
  KSimpleConfig config(TQString::fromLatin1(FILESHARECONF),true);
  smbConf = config.readEntry("SMBCONF");

  if ( TQFile::exists(smbConf) )
    return true;

  if ( TQFile::exists("/etc/samba/smb.conf") )
    smbConf = "/etc/samba/smb.conf";
  else
  if ( TQFile::exists("/etc/smb.conf") )
    smbConf = "/etc/smb.conf";
  else
  if ( TQFile::exists("/usr/local/samba/lib/smb.conf") )
    smbConf = "/usr/local/samba/lib/smb.conf";
  else
  if ( TQFile::exists("/usr/samba/lib/smb.conf") )
    smbConf = "/usr/samba/lib/smb.conf";
  else
  if ( TQFile::exists("/usr/lib/smb.conf") )
    smbConf = "/usr/lib/smb.conf";
  else
  if ( TQFile::exists("/usr/local/lib/smb.conf") )
    smbConf = "/usr/local/lib/smb.conf";
  else {
    kdDebug(7000) << "KSambaShare: Could not found smb.conf!" << endl;
    return false;
  }
      
  return true;
}


/**
 * Reads all path= entries from the smb.conf file
 * and fills the sharedPaths dict with the values
 */
bool KSambaSharePrivate::readSmbConf() {
  TQFile f(smbConf);

  kdDebug(7000) << "KSambaShare::readSmbConf " << smbConf << endl;
  
  if (!f.open(IO_ReadOnly)) {
    kdError() << "KSambaShare: Could not open " << smbConf << endl;
    return false;
  }
  
  sharedPaths.clear();

  TQTextStream s(&f);

  bool continuedLine = false; // is true if the line before ended with a backslash
  TQString completeLine;

  while (!s.eof())
  {
    TQString currentLine = s.readLine().stripWhiteSpace();

    if (continuedLine) {
      completeLine += currentLine;
      continuedLine = false;
    }      
    else
      completeLine = currentLine;

    // is the line continued in the next line ?
    if ( completeLine[completeLine.length()-1] == '\\' )
    {
      continuedLine = true;
      // remove the ending backslash
      completeLine.truncate( completeLine.length()-1 ); 
      continue;
    }
    
    // comments or empty lines
    if (completeLine.isEmpty() ||
        '#' == completeLine[0] ||
        ';' == completeLine[0])
    {
      continue;
    }

    // parameter
    int i = completeLine.find('=');

    if (i>-1)
    {
      TQString name = completeLine.left(i).stripWhiteSpace().lower();
      TQString value = completeLine.mid(i+1).stripWhiteSpace();

      if (name == TDEGlobal::staticQString("path")) {
        // Handle quotation marks
        if ( value[0] == '"' )
          value.remove(0,1);
         
        if ( value[value.length()-1] == '"' )
          value.truncate(value.length()-1);        
        
        // Normalize path
        if ( value[value.length()-1] != '/' )
             value += '/';
             
        bool b = true;             
        sharedPaths.insert(value,&b);
        kdDebug(7000) << "KSambaShare: Found path: " << value << endl;
      }
    }
  }

  f.close();

  return true;  

}

KSambaShare::KSambaShare() {
  d = new KSambaSharePrivate();
  if (TQFile::exists(d->smbConf)) {
    KDirWatch::self()->addFile(d->smbConf);
    KDirWatch::self()->addFile(FILESHARECONF);
    connect(KDirWatch::self(), TQ_SIGNAL(dirty (const TQString&)),this,
   	        TQ_SLOT(slotFileChange(const TQString&)));
  } 
}

KSambaShare::~KSambaShare() {
  if (TQFile::exists(d->smbConf)) {
        KDirWatch::self()->removeFile(d->smbConf);
        KDirWatch::self()->removeFile(FILESHARECONF);
  }
  delete d;
}

TQString KSambaShare::smbConfPath() const {
  return d->smbConf;
}

bool KSambaShare::isDirectoryShared( const TQString & path ) const {
  TQString fixedPath = path;
  if ( path[path.length()-1] != '/' )
       fixedPath += '/';
  
  return d->sharedPaths.find(fixedPath) != 0;
}

TQStringList KSambaShare::sharedDirectories() const {
  TQStringList result;
  TQDictIterator<bool> it(d->sharedPaths);
  for( ; it.current(); ++it )
      result << it.currentKey();
      
  return result;       
}

void KSambaShare::slotFileChange( const TQString & path ) {
  if (path == d->smbConf)
     d->readSmbConf();
  else
  if (path == FILESHARECONF)
     d->load();
              
  emit changed();     
}

KSambaShare* KSambaShare::_instance = 0L; 
static KStaticDeleter<KSambaShare> ksdSambaShare;

KSambaShare* KSambaShare::instance() {
  if (! _instance ) 
      _instance = ksdSambaShare.setObject(_instance, new KSambaShare());
      
  return _instance;      
}

#include "ksambashare.moc"

