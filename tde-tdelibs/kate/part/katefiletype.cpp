/* This file is part of the KDE libraries
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>

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

//BEGIN Includes
#include "katefiletype.h"
#include "katefiletype.moc"

#include "katedocument.h"
#include "kateconfig.h"
#include "kateview.h"
#include "katefactory.h"

#include <tdeconfig.h>
#include <kmimemagic.h>
#include <kmimetype.h>
#include <kmimetypechooser.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <tdelocale.h>
#include <tdepopupmenu.h>

#include <tqregexp.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqgroupbox.h>
#include <tqhbox.h>
#include <tqheader.h>
#include <tqhgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqtoolbutton.h>
#include <tqvbox.h>
#include <tqvgroupbox.h>
#include <tqwhatsthis.h>
#include <tqwidgetstack.h>

#define KATE_FT_HOWMANY 1024
//END Includes

//BEGIN KateFileTypeManager
KateFileTypeManager::KateFileTypeManager ()
{
  m_types.setAutoDelete (true);

  update ();
}

KateFileTypeManager::~KateFileTypeManager ()
{
}

//
// read the types from config file and update the internal list
//
void KateFileTypeManager::update ()
{
  TDEConfig config ("katefiletyperc", false, false);

  TQStringList g (config.groupList());
  g.sort ();

  m_types.clear ();
  for (uint z=0; z < g.count(); z++)
  {
    config.setGroup (g[z]);

    KateFileType *type = new KateFileType ();

    type->number = z;
    type->name = g[z];
    type->section = config.readEntry ("Section");
    type->wildcards = config.readListEntry ("Wildcards", ';');
    type->mimetypes = config.readListEntry ("Mimetypes", ';');
    type->priority = config.readNumEntry ("Priority");
    type->varLine = config.readEntry ("Variables");

    m_types.append (type);
  }
}

//
// save the given list to config file + update
//
void KateFileTypeManager::save (TQPtrList<KateFileType> *v)
{
  TDEConfig config ("katefiletyperc", false, false);

  TQStringList newg;
  for (uint z=0; z < v->count(); z++)
  {
    config.setGroup (v->at(z)->name);

    config.writeEntry ("Section", v->at(z)->section);
    config.writeEntry ("Wildcards", v->at(z)->wildcards, ';');
    config.writeEntry ("Mimetypes", v->at(z)->mimetypes, ';');
    config.writeEntry ("Priority", v->at(z)->priority);

    TQString varLine = v->at(z)->varLine;
    if (TQRegExp("kate:(.*)").search(varLine) < 0)
      varLine.prepend ("kate: ");

    config.writeEntry ("Variables", varLine);

    newg << v->at(z)->name;
  }

  TQStringList g (config.groupList());

  for (uint z=0; z < g.count(); z++)
  {
    if (newg.findIndex (g[z]) == -1)
      config.deleteGroup (g[z]);
  }

  config.sync ();

  update ();
}

int KateFileTypeManager::fileType (KateDocument *doc)
{
  kdDebug(13020)<<k_funcinfo<<endl;
  if (!doc)
    return -1;

  if (m_types.isEmpty())
    return -1;

  TQString fileName = doc->url().prettyURL();
  int length = doc->url().prettyURL().length();

  int result;

  // Try wildcards
  if ( ! fileName.isEmpty() )
  {
    static TQStringList commonSuffixes = TQStringList::split (";", ".orig;.new;~;.bak;.BAK");

    if ((result = wildcardsFind(fileName)) != -1)
      return result;

    TQString backupSuffix = KateDocumentConfig::global()->backupSuffix();
    if (fileName.endsWith(backupSuffix)) {
      if ((result = wildcardsFind(fileName.left(length - backupSuffix.length()))) != -1)
        return result;
    }

    for (TQStringList::Iterator it = commonSuffixes.begin(); it != commonSuffixes.end(); ++it) {
      if (*it != backupSuffix && fileName.endsWith(*it)) {
        if ((result = wildcardsFind(fileName.left(length - (*it).length()))) != -1)
          return result;
      }
    }
  }

  // Even try the document name, if the URL is empty
  // This is usefull if the document name is set for example by a plugin which
  // created the document
  else if ( (result = wildcardsFind(doc->docName())) != -1)
  {
    kdDebug(13020)<<"KateFiletype::filetype(): got type "<<result<<" using docName '"<<doc->docName()<<"'"<<endl;
    return result;
  }

  // Try content-based mimetype
  KMimeType::Ptr mt = doc->mimeTypeForContent();

  TQPtrList<KateFileType> types;

  for (uint z=0; z < m_types.count(); z++)
  {
    if (m_types.at(z)->mimetypes.findIndex (mt->name()) > -1)
      types.append (m_types.at(z));
  }

  if ( !types.isEmpty() )
  {
    int pri = -1;
    int hl = -1;

    for (KateFileType *type = types.first(); type != 0L; type = types.next())
    {
      if (type->priority > pri)
      {
        pri = type->priority;
        hl = type->number;
      }
    }

    return hl;
  }


  return -1;
}

int KateFileTypeManager::wildcardsFind (const TQString &fileName)
{
  TQPtrList<KateFileType> types;

  for (uint z=0; z < m_types.count(); z++)
  {
    for( TQStringList::Iterator it = m_types.at(z)->wildcards.begin(); it != m_types.at(z)->wildcards.end(); ++it )
    {
      // anders: we need to be sure to match the end of string, as eg a css file
      // would otherwise end up with the c hl
      TQRegExp re(*it, true, true);
      if ( ( re.search( fileName ) > -1 ) && ( re.matchedLength() == (int)fileName.length() ) )
        types.append (m_types.at(z));
    }
  }

  if ( !types.isEmpty() )
  {
    int pri = -1;
    int hl = -1;

    for (KateFileType *type = types.first(); type != 0L; type = types.next())
    {
      if (type->priority > pri)
      {
        pri = type->priority;
        hl = type->number;
      }
    }

    return hl;
  }

  return -1;
}

const KateFileType *KateFileTypeManager::fileType (uint number)
{
  if (number < m_types.count())
    return m_types.at(number);

  return 0;
}
//END KateFileTypeManager

//BEGIN KateFileTypeConfigTab
KateFileTypeConfigTab::KateFileTypeConfigTab( TQWidget *parent )
  : KateConfigPage( parent )
{
  m_types.setAutoDelete (true);
  m_lastType = 0;

  TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint() );

  // hl chooser
  TQHBox *hbHl = new TQHBox( this );
  layout->add (hbHl);
  hbHl->setSpacing( KDialog::spacingHint() );
  TQLabel *lHl = new TQLabel( i18n("&Filetype:"), hbHl );
  typeCombo = new TQComboBox( false, hbHl );
  lHl->setBuddy( typeCombo );
  connect( typeCombo, TQ_SIGNAL(activated(int)),
           this, TQ_SLOT(typeChanged(int)) );

  TQPushButton *btnnew = new TQPushButton( i18n("&New"), hbHl );
  connect( btnnew, TQ_SIGNAL(clicked()), this, TQ_SLOT(newType()) );

  btndel = new TQPushButton( i18n("&Delete"), hbHl );
  connect( btndel, TQ_SIGNAL(clicked()), this, TQ_SLOT(deleteType()) );

  gbProps = new TQGroupBox( 2, TQt::Horizontal, i18n("Properties"), this );
  layout->add (gbProps);

  // file & mime types
  TQLabel *lname = new TQLabel( i18n("N&ame:"), gbProps );
  name  = new TQLineEdit( gbProps );
  lname->setBuddy( name );

  // file & mime types
  TQLabel *lsec = new TQLabel( i18n("&Section:"), gbProps );
  section  = new TQLineEdit( gbProps );
  lsec->setBuddy( section );

  // file & mime types
  TQLabel *lvar = new TQLabel( i18n("&Variables:"), gbProps );
  varLine  = new TQLineEdit( gbProps );
  lvar->setBuddy( varLine );

  // file & mime types
  TQLabel *lFileExts = new TQLabel( i18n("File e&xtensions:"), gbProps );
  wildcards  = new TQLineEdit( gbProps );
  lFileExts->setBuddy( wildcards );

  TQLabel *lMimeTypes = new TQLabel( i18n("MIME &types:"), gbProps);
  TQHBox *hbMT = new TQHBox (gbProps);
  mimetypes = new TQLineEdit( hbMT );
  lMimeTypes->setBuddy( mimetypes );

  TQToolButton *btnMTW = new TQToolButton(hbMT);
  btnMTW->setIconSet(TQIconSet(SmallIcon("wizard")));
  connect(btnMTW, TQ_SIGNAL(clicked()), this, TQ_SLOT(showMTDlg()));

  TQLabel *lprio = new TQLabel( i18n("Prio&rity:"), gbProps);
  priority = new KIntNumInput( gbProps );
  lprio->setBuddy( priority );

  layout->addStretch();

  reload();

  connect( name, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
  connect( section, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
  connect( varLine, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
  connect( wildcards, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
  connect( mimetypes, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
  connect( priority, TQ_SIGNAL( valueChanged ( int ) ), this, TQ_SLOT( slotChanged() ) );

  TQWhatsThis::add( btnnew, i18n("Create a new file type.") );
  TQWhatsThis::add( btndel, i18n("Delete the current file type.") );
  TQWhatsThis::add( name, i18n(
      "The name of the filetype will be the text of the corresponding menu item.") );
  TQWhatsThis::add( section, i18n(
      "The section name is used to organize the file types in menus.") );
  TQWhatsThis::add( varLine, i18n(
      "<p>This string allows you to configure Kate's settings for the files "
      "selected by this mimetype using Kate variables. You can set almost any "
      "configuration option, such as highlight, indent-mode, encoding, etc.</p>"
      "<p>For a full list of known variables, see the manual.</p>") );
  TQWhatsThis::add( wildcards, i18n(
      "The wildcards mask allows you to select files by filename. A typical "
      "mask uses an asterisk and the file extension, for example "
      "<code>*.txt; *.text</code>. The string is a semicolon-separated list "
      "of masks.") );
  TQWhatsThis::add( mimetypes, i18n(
      "The mime type mask allows you to select files by mimetype. The string is "
      "a semicolon-separated list of mimetypes, for example "
      "<code>text/plain; text/english</code>.") );
  TQWhatsThis::add( btnMTW, i18n(
      "Displays a wizard that helps you easily select mimetypes.") );
  TQWhatsThis::add( priority, i18n(
      "Sets a priority for this file type. If more than one file type selects the same "
      "file, the one with the highest priority will be used." ) );
}

void KateFileTypeConfigTab::apply()
{
  if (!changed())
    return;

  save ();

  KateFactory::self()->fileTypeManager()->save(&m_types);
}

void KateFileTypeConfigTab::reload()
{
  m_types.clear();
  for (uint z=0; z < KateFactory::self()->fileTypeManager()->list()->count(); z++)
  {
    KateFileType *type = new KateFileType ();

    *type = *KateFactory::self()->fileTypeManager()->list()->at(z);

    m_types.append (type);
  }

  update ();
}

void KateFileTypeConfigTab::reset()
{
  reload ();
}

void KateFileTypeConfigTab::defaults()
{
  reload ();
}

void KateFileTypeConfigTab::update ()
{
  m_lastType = 0;

  typeCombo->clear ();

  for( uint i = 0; i < m_types.count(); i++) {
    if (m_types.at(i)->section.length() > 0)
      typeCombo->insertItem(m_types.at(i)->section + TQString ("/") + m_types.at(i)->name);
    else
      typeCombo->insertItem(m_types.at(i)->name);
  }

  typeCombo->setCurrentItem (0);

  typeChanged (0);

  typeCombo->setEnabled (typeCombo->count() > 0);
}

void KateFileTypeConfigTab::deleteType ()
{
  int type = typeCombo->currentItem ();

  if ((type > -1) && ((uint)type < m_types.count()))
  {
    m_types.remove (type);
    update ();
  }
}

void KateFileTypeConfigTab::newType ()
{
  TQString newN = i18n("New Filetype");

  for( uint i = 0; i < m_types.count(); i++) {
    if (m_types.at(i)->name == newN)
    {
      typeCombo->setCurrentItem (i);
      typeChanged (i);
      return;
    }
  }

  KateFileType *newT = new KateFileType ();
  newT->priority = 0;
  newT->name = newN;

  m_types.prepend (newT);

  update ();
}

void KateFileTypeConfigTab::save ()
{
  if (m_lastType)
  {
    m_lastType->name = name->text ();
    m_lastType->section = section->text ();
    m_lastType->varLine = varLine->text ();
    m_lastType->wildcards = TQStringList::split (";", wildcards->text ());
    m_lastType->mimetypes = TQStringList::split (";", mimetypes->text ());
    m_lastType->priority = priority->value();
  }
}

void KateFileTypeConfigTab::typeChanged (int type)
{
  save ();

  KateFileType *t = 0;

  if ((type > -1) && ((uint)type < m_types.count()))
    t = m_types.at(type);

  if (t)
  {
    gbProps->setTitle (i18n("Properties of %1").arg (typeCombo->currentText()));

    gbProps->setEnabled (true);
    btndel->setEnabled (true);

    name->setText(t->name);
    section->setText(t->section);
    varLine->setText(t->varLine);
    wildcards->setText(t->wildcards.join (";"));
    mimetypes->setText(t->mimetypes.join (";"));
    priority->setValue(t->priority);
  }
  else
  {
    gbProps->setTitle (i18n("Properties"));

    gbProps->setEnabled (false);
    btndel->setEnabled (false);

    name->clear();
    section->clear();
    varLine->clear();
    wildcards->clear();
    mimetypes->clear();
    priority->setValue(0);
  }

  m_lastType = t;
}

void KateFileTypeConfigTab::showMTDlg()
{

  TQString text = i18n("Select the MimeTypes you want for this file type.\nPlease note that this will automatically edit the associated file extensions as well.");
  TQStringList list = TQStringList::split( TQRegExp("\\s*;\\s*"), mimetypes->text() );
  KMimeTypeChooserDialog d( i18n("Select Mime Types"), text, list, "text", this );
  if ( d.exec() == KDialogBase::Accepted ) {
    // do some checking, warn user if mime types or patterns are removed.
    // if the lists are empty, and the fields not, warn.
    wildcards->setText( d.chooser()->patterns().join(";") );
    mimetypes->setText( d.chooser()->mimeTypes().join(";") );
  }
}
//END KateFileTypeConfigTab

//BEGIN KateViewFileTypeAction
void KateViewFileTypeAction::init()
{
  m_doc = 0;
  subMenus.setAutoDelete( true );

  popupMenu()->insertItem ( i18n("None"), this, TQ_SLOT(setType(int)), 0,  0);

  connect(popupMenu(),TQ_SIGNAL(aboutToShow()),this,TQ_SLOT(slotAboutToShow()));
}

void KateViewFileTypeAction::updateMenu (Kate::Document *doc)
{
  m_doc = (KateDocument *)doc;
}

void KateViewFileTypeAction::slotAboutToShow()
{
  KateDocument *doc=m_doc;
  int count = KateFactory::self()->fileTypeManager()->list()->count();

  for (int z=0; z<count; z++)
  {
    TQString hlName = KateFactory::self()->fileTypeManager()->list()->at(z)->name;
    TQString hlSection = KateFactory::self()->fileTypeManager()->list()->at(z)->section;

    if ( !hlSection.isEmpty() && (names.contains(hlName) < 1) )
    {
      if (subMenusName.contains(hlSection) < 1)
      {
        subMenusName << hlSection;
        TQPopupMenu *menu = new TQPopupMenu ();
        subMenus.append(menu);
        popupMenu()->insertItem (hlSection, menu);
      }

      int m = subMenusName.findIndex (hlSection);
      names << hlName;
      subMenus.at(m)->insertItem ( hlName, this, TQ_SLOT(setType(int)), 0,  z+1);
    }
    else if (names.contains(hlName) < 1)
    {
      names << hlName;
      popupMenu()->insertItem ( hlName, this, TQ_SLOT(setType(int)), 0,  z+1);
    }
  }

  if (!doc) return;

  for (uint i=0;i<subMenus.count();i++)
  {
    for (uint i2=0;i2<subMenus.at(i)->count();i2++)
      subMenus.at(i)->setItemChecked(subMenus.at(i)->idAt(i2),false);
  }
  popupMenu()->setItemChecked (0, false);

  if (doc->fileType() == -1)
    popupMenu()->setItemChecked (0, true);
  else
  {
    const KateFileType *t = 0;
    if ((t = KateFactory::self()->fileTypeManager()->fileType (doc->fileType())))
    {
      int i = subMenusName.findIndex (t->section);
      if (i >= 0 && subMenus.at(i))
        subMenus.at(i)->setItemChecked (doc->fileType()+1, true);
      else
        popupMenu()->setItemChecked (0, true);
    }
  }
}

void KateViewFileTypeAction::setType (int mode)
{
  KateDocument *doc=m_doc;

  if (doc)
    doc->updateFileType(mode-1, true);
}
//END KateViewFileTypeAction
