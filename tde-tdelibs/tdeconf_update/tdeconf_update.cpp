/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <tqfile.h>
#include <tqtextstream.h>

#include <tdeconfig.h>
#include <ksimpleconfig.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <tdeaboutdata.h>
#include <kinstance.h>
#include <tdetempfile.h>

static TDECmdLineOptions options[] =
{
        { "debug", I18N_NOOP("Keep output results from scripts"), 0 },
	{ "check <update-file>", I18N_NOOP("Check whether config file itself requires updating"), 0 },
	{ "+[file]", I18N_NOOP("File to read update instructions from"), 0 },
        TDECmdLineLastOption
};

class KonfUpdate
{
public:
   KonfUpdate();
   ~KonfUpdate();
   TQStringList findUpdateFiles(bool dirtyOnly);

   TQTextStream &log();

   bool checkFile(const TQString &filename);
   void checkGotFile(const TQString &_file, const TQString &id);

   bool updateFile(const TQString &filename);

   void gotId(const TQString &_id);
   void gotFile(const TQString &_file);
   void gotGroup(const TQString &_group);
   void gotRemoveGroup(const TQString &_group);
   void gotKey(const TQString &_key);
   void gotRemoveKey(const TQString &_key);
   void gotAllKeys();
   void gotAllGroups();
   void gotOptions(const TQString &_options);
   void gotScript(const TQString &_script);
   void gotScriptArguments(const TQString &_arguments);
   void resetOptions();

   void copyGroup(TDEConfigBase *cfg1, const TQString &grp1, 
                  TDEConfigBase *cfg2, const TQString &grp2);

protected:
   TDEConfig *config;
   TQString currentFilename;
   bool skip;
   bool debug;
   TQString id;

   TQString oldFile;
   TQString newFile;
   TQString newFileName;
   TDEConfig *oldConfig1; // Config to read keys from.
   TDEConfig *oldConfig2; // Config to delete keys from.
   TDEConfig *newConfig;

   TQString oldGroup;
   TQString newGroup;
   TQString oldKey;
   TQString newKey;

   bool m_bCopy;
   bool m_bOverwrite;
   bool m_bUseConfigInfo;
   TQString m_arguments;
   TQTextStream *m_textStream;
   TQFile *m_file;
   TQString m_line;
   int m_lineCount;
};

KonfUpdate::KonfUpdate()
 : m_textStream(0), m_file(0)
{
   bool updateAll = false;
   oldConfig1 = 0;
   oldConfig2 = 0;
   newConfig = 0;

   config = new TDEConfig("tdeconf_updaterc");

   TQStringList updateFiles;
   TDECmdLineArgs *args=TDECmdLineArgs::parsedArgs();
   
   debug = args->isSet("debug");

   m_bUseConfigInfo = false;
   if (args->isSet("check"))
   {
      m_bUseConfigInfo = true;
      TQString file = locate("data", "tdeconf_update/"+TQFile::decodeName(args->getOption("check")));
      if (file.isEmpty())
      {
         tqWarning("File '%s' not found.", args->getOption("check").data());
         log() << "File '" << TQFile::decodeName(args->getOption("check")) << "' passed on command line not found" << endl;
         return;
      }
      updateFiles.append(file);
   }
   else if (args->count())
   {
      for(int i = 0; i < args->count(); i++)
      {
         KURL url = args->url(i);
         if (!url.isLocalFile())
            TDECmdLineArgs::usage(i18n("Only local files are supported."));
         updateFiles.append(url.path());
      }
   }
   else
   {
      if (config->readBoolEntry("autoUpdateDisabled", false))
         return;
      updateFiles = findUpdateFiles(true);
      updateAll = true;
   }

   for(TQStringList::ConstIterator it = updateFiles.begin();
       it != updateFiles.end();
       ++it)
   {
      TQString file = *it;
      updateFile(file);
   }

   config->setGroup(TQString::null);
   if (updateAll && !config->readBoolEntry("updateInfoAdded", false))
   {
       config->writeEntry("updateInfoAdded", true);
       updateFiles = findUpdateFiles(false);

       for(TQStringList::ConstIterator it = updateFiles.begin();
           it != updateFiles.end();
           ++it)
       {
           TQString file = *it;
           checkFile(file);
       }
       updateFiles.clear();
   }
}

KonfUpdate::~KonfUpdate()
{
   delete config;
   delete m_file;
   delete m_textStream;
}

TQTextStream &
KonfUpdate::log()
{
   if (!m_textStream)
   {
      TQString file = locateLocal("data", "tdeconf_update/log/update.log");
      m_file = new TQFile(file);
      if (m_file->open(IO_WriteOnly | IO_Append))
      {
        m_textStream = new TQTextStream(m_file);
      }
      else
      {
        // Error
        m_textStream = new TQTextStream(stderr, IO_WriteOnly);
      }
   }
   
   (*m_textStream) << TQDateTime::currentDateTime().toString( TQt::ISODate ) << " ";
   
   return *m_textStream;
}

TQStringList KonfUpdate::findUpdateFiles(bool dirtyOnly)
{
   TQStringList result;
   TQStringList list = TDEGlobal::dirs()->findAllResources("data", "tdeconf_update/*.upd", false, true);
   for(TQStringList::ConstIterator it = list.begin();
       it != list.end();
       ++it)
   {
      TQString file = *it;
      struct stat buff;
      if (stat( TQFile::encodeName(file), &buff) == 0)
      {
         int i = file.findRev('/');
         if (i != -1) 
            file = file.mid(i+1);
         config->setGroup(file);
         time_t ctime = config->readUnsignedLongNumEntry("ctime");
         time_t mtime = config->readUnsignedLongNumEntry("mtime");
         if (!dirtyOnly ||
             (ctime != buff.st_ctime) || (mtime != buff.st_mtime))
         {
            result.append(*it);
         }
      }
   }
   return result;
}

bool KonfUpdate::checkFile(const TQString &filename)
{
   currentFilename = filename;
   int i = currentFilename.findRev('/');
   if (i != -1) 
      currentFilename = currentFilename.mid(i+1);
   skip = true;
   TQFile file(filename);
   if (!file.open(IO_ReadOnly))
      return false;

   TQTextStream ts(&file);
   ts.setEncoding(TQTextStream::Latin1);
   int lineCount = 0;
   resetOptions();
   TQString id;
   while(!ts.atEnd())
   {
      TQString line = ts.readLine().stripWhiteSpace();
      lineCount++;
      if (line.isEmpty() || (line[0] == '#'))
         continue;
      if (line.startsWith("Id="))
         id = currentFilename+":"+line.mid(3);
      else if (line.startsWith("File="))
         checkGotFile(line.mid(5), id);
   }
  
   return true;
}

void KonfUpdate::checkGotFile(const TQString &_file, const TQString &id)
{
   TQString file;
   int i = _file.find(',');
   if (i == -1)
   {
      file = _file.stripWhiteSpace();
   }
   else
   {
      file = _file.mid(i+1).stripWhiteSpace();
   }

//   tqDebug("File %s, id %s", file.latin1(), id.latin1());

   KSimpleConfig cfg(file);
   cfg.setGroup("$Version");
   TQStringList ids = cfg.readListEntry("update_info");
   if (ids.contains(id))
       return;
   ids.append(id);
   cfg.writeEntry("update_info", ids);
}

/**
 * Syntax:
 * # Comment
 * Id=id
 * File=oldfile[,newfile]
 * AllGroups
 * Group=oldgroup[,newgroup]
 * RemoveGroup=oldgroup
 * Options=[copy,][overwrite,]
 * Key=oldkey[,newkey]
 * RemoveKey=ldkey
 * AllKeys
 * Keys= [Options](AllKeys|(Key|RemoveKey)*)
 * ScriptArguments=arguments
 * Script=scriptfile[,interpreter]
 *
 * Sequence:
 * (Id,(File(Group,Keys)*)*)*
 **/
bool KonfUpdate::updateFile(const TQString &filename)
{
   currentFilename = filename;
   int i = currentFilename.findRev('/');
   if (i != -1) 
       currentFilename = currentFilename.mid(i+1);
   skip = true;
   TQFile file(filename);
   if (!file.open(IO_ReadOnly))
      return false;

   log() << "Checking update-file '" << filename << "' for new updates" << endl; 

   TQTextStream ts(&file);
   ts.setEncoding(TQTextStream::Latin1);
   m_lineCount = 0;
   resetOptions();
   while(!ts.atEnd())
   {
      m_line = ts.readLine().stripWhiteSpace();
      m_lineCount++;
      if (m_line.isEmpty() || (m_line[0] == '#'))
         continue;
      if (m_line.startsWith("Id="))
         gotId(m_line.mid(3));
      else if (skip)
         continue;
      else if (m_line.startsWith("Options="))
         gotOptions(m_line.mid(8));
      else if (m_line.startsWith("File="))
         gotFile(m_line.mid(5));
      else if (m_line.startsWith("Group="))
         gotGroup(m_line.mid(6));
      else if (m_line.startsWith("RemoveGroup="))
      {
         gotRemoveGroup(m_line.mid(12));
         resetOptions();
      }
      else if (m_line.startsWith("Script="))
      {
         gotScript(m_line.mid(7));
         resetOptions();
      }
      else if (m_line.startsWith("ScriptArguments="))
         gotScriptArguments(m_line.mid(16));
      else if (m_line.startsWith("Key="))
      {
         gotKey(m_line.mid(4));
         resetOptions();
      }
      else if (m_line.startsWith("RemoveKey="))
      {
         gotRemoveKey(m_line.mid(10));
         resetOptions();
      }
      else if (m_line == "AllKeys")
      {
         gotAllKeys();
         resetOptions();
      }
      else if (m_line == "AllGroups")
      {
         gotAllGroups();
         resetOptions();
      }
      else
      {
         log() << currentFilename << ": parse error in line " << m_lineCount << " : '" << m_line << "'" << endl;
      }
   }
   // Flush.
   gotId(TQString::null);
  
   struct stat buff;
   stat( TQFile::encodeName(filename), &buff);
   config->setGroup(currentFilename);
   config->writeEntry("ctime", buff.st_ctime);
   config->writeEntry("mtime", buff.st_mtime);
   config->sync();
   return true;
}



void KonfUpdate::gotId(const TQString &_id)
{
   if (!id.isEmpty() && !skip)
   {
       config->setGroup(currentFilename);
       TQStringList ids = config->readListEntry("done");
       if (!ids.contains(id))
       {
          ids.append(id);
          config->writeEntry("done", ids);
          config->sync();
       }
   }

   // Flush pending changes
   gotFile(TQString::null);

   config->setGroup(currentFilename);
   TQStringList ids = config->readListEntry("done");
   if (!_id.isEmpty())
   {
       if (ids.contains(_id))
       {
          //tqDebug("Id '%s' was already in done-list", _id.latin1());
          if (!m_bUseConfigInfo)
          {
             skip = true;
             return;
          }
       }
       skip = false;
       id = _id;
       if (m_bUseConfigInfo)
          log() << currentFilename << ": Checking update '" << _id << "'" << endl;
       else
          log() << currentFilename << ": Found new update '" << _id << "'" << endl;
   }
}

void KonfUpdate::gotFile(const TQString &_file)
{
   // Reset group
   gotGroup(TQString::null);
 
   if (!oldFile.isEmpty())
   {
      // Close old file.
      delete oldConfig1;
      oldConfig1 = 0;

      oldConfig2->setGroup("$Version");
      TQStringList ids = oldConfig2->readListEntry("update_info");
      TQString cfg_id = currentFilename + ":" + id;
      if (!ids.contains(cfg_id) && !skip)
      {
         ids.append(cfg_id);
         oldConfig2->writeEntry("update_info", ids);
      }
      oldConfig2->sync();
      delete oldConfig2;
      oldConfig2 = 0;
      
      TQString file = locateLocal("config", oldFile);
      struct stat s_buf;
      if (stat(TQFile::encodeName(file), &s_buf) == 0)
      {
         if (s_buf.st_size == 0)
         {
            // Delete empty file.
            unlink(TQFile::encodeName(file));
         }   
      }

      oldFile = TQString::null;
   }
   if (!newFile.isEmpty())
   {
      // Close new file.
      newConfig->setGroup("$Version");
      TQStringList ids = newConfig->readListEntry("update_info");
      TQString cfg_id = currentFilename + ":" + id;
      if (!ids.contains(cfg_id) && !skip)
      {
         ids.append(cfg_id);
         newConfig->writeEntry("update_info", ids);
      }
      newConfig->sync();
      delete newConfig;
      newConfig = 0;

      newFile = TQString::null;
   }
   newConfig = 0; 

   int i = _file.find(',');
   if (i == -1)
   {
      oldFile = _file.stripWhiteSpace();
   }
   else
   {
      oldFile = _file.left(i).stripWhiteSpace();
      newFile = _file.mid(i+1).stripWhiteSpace();
      if (oldFile == newFile)
         newFile = TQString::null;
   }
   
   if (!oldFile.isEmpty())
   {
      oldConfig2 = new TDEConfig(oldFile, false, false);
      TQString cfg_id = currentFilename + ":" + id;
      oldConfig2->setGroup("$Version");
      TQStringList ids = oldConfig2->readListEntry("update_info");
      if (ids.contains(cfg_id))
      {
         skip = true;
         newFile = TQString::null;
         log() << currentFilename << ": Skipping update '" << id << "'" << endl;
      }

      if (!newFile.isEmpty())
      {
         newConfig = new TDEConfig(newFile, false, false);
         newConfig->setGroup("$Version");
         ids = newConfig->readListEntry("update_info");
         if (ids.contains(cfg_id))
         {
            skip = true;
            log() << currentFilename << ": Skipping update '" << id << "'" << endl;
         }
      }
      else
      {
         newConfig = oldConfig2;
      }

      oldConfig1 = new TDEConfig(oldFile, true, false);
   }
   else
   {
      newFile = TQString::null;
   }
   newFileName = newFile;
   if (newFileName.isEmpty())
      newFileName = oldFile;
}

void KonfUpdate::gotGroup(const TQString &_group)
{
   int i = _group.find(',');
   if (i == -1)
   {
      oldGroup = _group.stripWhiteSpace();
      newGroup = oldGroup;
   }
   else
   {
      oldGroup = _group.left(i).stripWhiteSpace();
      newGroup = _group.mid(i+1).stripWhiteSpace();
   }
}

void KonfUpdate::gotRemoveGroup(const TQString &_group)
{
   oldGroup = _group.stripWhiteSpace();

   if (!oldConfig1)
   {
      log() << currentFilename << ": !! RemoveGroup without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   if (!oldConfig1->hasGroup(oldGroup))
      return;
   // Delete group.
   oldConfig2->deleteGroup(oldGroup, true);
   log() << currentFilename << ": RemoveGroup removes group " << oldFile << ":" << oldGroup << endl;
}


void KonfUpdate::gotKey(const TQString &_key)
{
   int i = _key.find(',');
   if (i == -1)
   {
      oldKey = _key.stripWhiteSpace();
      newKey = oldKey;
   }
   else
   {
      oldKey = _key.left(i).stripWhiteSpace();
      newKey = _key.mid(i+1).stripWhiteSpace();
   }

   if (oldKey.isEmpty() || newKey.isEmpty())
   {
      log() << currentFilename << ": !! Key specifies invalid key in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }
   if (!oldConfig1)
   {
      log() << currentFilename << ": !! Key without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }
   oldConfig1->setGroup(oldGroup);
   if (!oldConfig1->hasKey(oldKey))
      return;
   TQString value = oldConfig1->readEntry(oldKey);
   newConfig->setGroup(newGroup);
   if (!m_bOverwrite && newConfig->hasKey(newKey))
   {
      log() << currentFilename << ": Skipping " << newFileName << ":" << newGroup << ":" << newKey << ", already exists."<< endl;
      return;
   }
   log() << currentFilename << ": Updating " << newFileName << ":" << newGroup << ":" << newKey << " to '" << value << "'" << endl;
   newConfig->writeEntry(newKey, value);

   if (m_bCopy)
      return; // Done.

   // Delete old entry
   if ((oldConfig2 == newConfig) && 
       (oldGroup == newGroup) &&
       (oldKey == newKey))
      return; // Don't delete!
   oldConfig2->setGroup(oldGroup);
   oldConfig2->deleteEntry(oldKey, false);
   log() << currentFilename << ": Removing " << oldFile << ":" << oldGroup << ":" << oldKey << ", moved." << endl;
   if (oldConfig2->deleteGroup(oldGroup, false)) { // Delete group if empty.
      log() << currentFilename << ": Removing empty group " << oldFile << ":" << oldGroup << endl;
   }
}

void KonfUpdate::gotRemoveKey(const TQString &_key)
{
   oldKey = _key.stripWhiteSpace();

   if (oldKey.isEmpty())
   {
      log() << currentFilename << ": !! RemoveKey specifies invalid key in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   if (!oldConfig1)
   {
      log() << currentFilename << ": !! Key without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   oldConfig1->setGroup(oldGroup);
   if (!oldConfig1->hasKey(oldKey))
      return;
   log() << currentFilename << ": RemoveKey removes " << oldFile << ":" << oldGroup << ":" << oldKey << endl;

   // Delete old entry
   oldConfig2->setGroup(oldGroup);
   oldConfig2->deleteEntry(oldKey, false);
   if (oldConfig2->deleteGroup(oldGroup, false)) { // Delete group if empty.
      log() << currentFilename << ": Removing empty group " << oldFile << ":" << oldGroup << endl;
   }
}

void KonfUpdate::gotAllKeys()
{
   if (!oldConfig1)
   {
      log() << currentFilename << ": !! AllKeys without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   TQMap<TQString, TQString> list = oldConfig1->entryMap(oldGroup);
   for(TQMap<TQString, TQString>::Iterator it = list.begin();
       it != list.end(); ++it)
   {
      gotKey(it.key());
   }
}

void KonfUpdate::gotAllGroups()
{
   if (!oldConfig1)
   {
      log() << currentFilename << ": !! AllGroups without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   TQStringList allGroups = oldConfig1->groupList();
   for(TQStringList::ConstIterator it = allGroups.begin();
       it != allGroups.end(); ++it)
   {
     oldGroup = *it;
     newGroup = oldGroup;
     gotAllKeys();
   }
}

void KonfUpdate::gotOptions(const TQString &_options)
{
   TQStringList options = TQStringList::split(',', _options);
   for(TQStringList::ConstIterator it = options.begin();
       it != options.end();
       ++it)
   {
       if ( (*it).lower().stripWhiteSpace() == "copy")
          m_bCopy = true;

       if ( (*it).lower().stripWhiteSpace() == "overwrite")
          m_bOverwrite = true;
   }
}

void KonfUpdate::copyGroup(TDEConfigBase *cfg1, const TQString &grp1, 
                           TDEConfigBase *cfg2, const TQString &grp2)
{
   cfg1->setGroup(grp1);
   cfg2->setGroup(grp2);
   TQMap<TQString, TQString> list = cfg1->entryMap(grp1);
   for(TQMap<TQString, TQString>::Iterator it = list.begin();
       it != list.end(); ++it)
   {
      cfg2->writeEntry(it.key(), cfg1->readEntry(it.key()));
   }
}

void KonfUpdate::gotScriptArguments(const TQString &_arguments)
{
   m_arguments = _arguments;
}

void KonfUpdate::gotScript(const TQString &_script)
{
   TQString script, interpreter;
   int i = _script.find(',');
   if (i == -1)
   {
      script = _script.stripWhiteSpace();
   }
   else
   {
      script = _script.left(i).stripWhiteSpace();
      interpreter = _script.mid(i+1).stripWhiteSpace();
   }


   if (script.isEmpty())
   {
      log() << currentFilename << ": !! Script fails to specify filename in line " << m_lineCount << " : '" << m_line << "'" << endl;
      skip = true;
      return;
   } 



   TQString path = locate("data","tdeconf_update/"+script);
   if (path.isEmpty())
   {
      if (interpreter.isEmpty())
         path = locate("lib", "tdeconf_update_bin/"+script);

      if (path.isEmpty())
      {
        log() << currentFilename << ": !! Script '" << script << "' not found in line " << m_lineCount << " : '" << m_line << "'" << endl;
        skip = true;
        return;
      }
   }

   if( !m_arguments.isNull())
      log() << currentFilename << ": Running script '" << script << "' with arguments '" << m_arguments << "'" << endl;
   else
      log() << currentFilename << ": Running script '" << script << "'" << endl;

   TQString cmd;
   if (interpreter.isEmpty())
      cmd = path;
   else
      cmd = interpreter + " " + path;

   if( !m_arguments.isNull())
   {
      cmd += ' ';
      cmd += m_arguments;
   }

   KTempFile tmp1;
   tmp1.setAutoDelete(true);
   KTempFile tmp2;
   tmp2.setAutoDelete(true);
   KTempFile tmp3;
   tmp3.setAutoDelete(true);

   int result;
   if (oldConfig1)
   {
       if (debug)
       {
           tmp1.setAutoDelete(false);
           log() << "Script input stored in " << tmp1.name() << endl;
       }
       KSimpleConfig cfg(tmp1.name());

       if (oldGroup.isEmpty())
       {
           // Write all entries to tmpFile;
           TQStringList grpList = oldConfig1->groupList();
           for(TQStringList::ConstIterator it = grpList.begin();
               it != grpList.end();
               ++it)
           {
               copyGroup(oldConfig1, *it, &cfg, *it);
           }
       }
       else 
       {
           copyGroup(oldConfig1, oldGroup, &cfg, TQString::null);
       }
       cfg.sync();
       result = system(TQFile::encodeName(TQString("%1 < %2 > %3 2> %4").arg(cmd, tmp1.name(), tmp2.name(), tmp3.name())));
   }
   else
   {
       // No config file
       result = system(TQFile::encodeName(TQString("%1 2> %2").arg(cmd, tmp3.name())));
   }

   // Copy script stderr to log file
   {
     TQFile output(tmp3.name());
     if (output.open(IO_ReadOnly))
     { 
       TQTextStream ts( &output );
       ts.setEncoding(TQTextStream::UnicodeUTF8);
       while(!ts.atEnd())
       {
         TQString line = ts.readLine();
         log() << "[Script] " << line << endl;
       }
     }
   }

   if (result)
   {
      log() << currentFilename << ": !! An error occured while running '" << cmd << "'" << endl;
      return;
   }

   if (!oldConfig1)
      return; // Nothing to merge

   if (debug)
   {
      tmp2.setAutoDelete(false);
      log() << "Script output stored in " << tmp2.name() << endl;
   }

   // Deleting old entries
   {
     TQString group = oldGroup;
     TQFile output(tmp2.name());
     if (output.open(IO_ReadOnly))
     { 
       TQTextStream ts( &output );
       ts.setEncoding(TQTextStream::UnicodeUTF8);
       while(!ts.atEnd())
       {
         TQString line = ts.readLine();
         if (line.startsWith("["))
         {
            int j = line.find(']')+1;
            if (j > 0)
               group = line.mid(1, j-2);
         }
         else if (line.startsWith("# DELETE "))
         {  
            TQString key = line.mid(9);
            if (key[0] == '[')
            {
               int j = key.find(']')+1;
               if (j > 0)
               {
                  group = key.mid(1,j-2);
                  key = key.mid(j);
               }
            }
            oldConfig2->setGroup(group);
            oldConfig2->deleteEntry(key, false);
            log() << currentFilename << ": Script removes " << oldFile << ":" << group << ":" << key << endl;
            if (oldConfig2->deleteGroup(group, false)) { // Delete group if empty.
               log() << currentFilename << ": Removing empty group " << oldFile << ":" << group << endl;
	    }
         }
         else if (line.startsWith("# DELETEGROUP"))
         {
            TQString key = line.mid(13).stripWhiteSpace();
            if (key[0] == '[')
            {
               int j = key.find(']')+1;
               if (j > 0)
               {
                  group = key.mid(1,j-2);
               }
            }
            if (oldConfig2->deleteGroup(group, true)) { // Delete group
               log() << currentFilename << ": Script removes group " << oldFile << ":" << group << endl;
	    }
          }
       }
     }
   }

   // Merging in new entries.
   m_bCopy = true;
   {
     TDEConfig *saveOldConfig1 = oldConfig1;
     TQString saveOldGroup = oldGroup;
     TQString saveNewGroup = newGroup;
     oldConfig1 = new TDEConfig(tmp2.name(), true, false);

     // For all groups...
     TQStringList grpList = oldConfig1->groupList();
     for(TQStringList::ConstIterator it = grpList.begin();
         it != grpList.end();
         ++it)
     {
        oldGroup = *it;
        if (oldGroup != "<default>")
        {
           newGroup = oldGroup;
        }
        gotAllKeys(); // Copy all keys
     }
     delete oldConfig1;
     oldConfig1 = saveOldConfig1;
     oldGroup = saveOldGroup;
     newGroup = saveNewGroup;
   }
}

void KonfUpdate::resetOptions()
{
   m_bCopy = false;
   m_bOverwrite = false;
   m_arguments = TQString::null;
}


extern "C" TDE_EXPORT int kdemain(int argc, char **argv)
{
   TDEAboutData aboutData("tdeconf_update", I18N_NOOP("KConf Update"),
                        "1.0.2",
                        I18N_NOOP("TDE Tool for updating user configuration files"),
                        TDEAboutData::License_GPL,
                        "(c) 2001, Waldo Bastian");

   aboutData.addAuthor("Waldo Bastian", 0, "bastian@kde.org");

   TDECmdLineArgs::init(argc, argv, &aboutData);
   TDECmdLineArgs::addCmdLineOptions(options);

   TDEInstance instance(&aboutData);

   KonfUpdate konfUpdate;

   return 0;
}
