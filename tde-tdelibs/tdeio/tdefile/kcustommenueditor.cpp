/*  This file is part of the KDE libraries
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqhbox.h>
#include <tqregexp.h>
#include <tqimage.h>
#include <tqpushbutton.h>
#include <tqdir.h>

#include <kbuttonbox.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <tdelistview.h>
#include <kservice.h>
#include <kstandarddirs.h>
#include <tdeconfigbase.h>
#include <kopenwith.h>

#include "kcustommenueditor.h"

class KCustomMenuEditor::Item : public TQListViewItem
{
public:
   Item(TQListView *parent, KService::Ptr service)
     : TQListViewItem(parent),
       s(service)
   {
      init();
   }

   Item(TQListViewItem *parent, KService::Ptr service)
     : TQListViewItem(parent),
       s(service)
   {
      init();
   }

   void init()
   {
      TQString serviceName = s->name();

      // item names may contain ampersands. To avoid them being converted
      // to accelators, replace them with two ampersands.
      serviceName.replace("&", "&&");

      TQPixmap normal = TDEGlobal::instance()->iconLoader()->loadIcon(s->icon(), TDEIcon::Small,
                              0, TDEIcon::DefaultState, 0L, true);

      // make sure they are not larger than 16x16
      if (normal.width() > 16 || normal.height() > 16) {
          TQImage tmp = normal.convertToImage();
          tmp = tmp.smoothScale(16, 16);
          normal.convertFromImage(tmp);
      }
      setText(0, serviceName);
      setPixmap(0, normal);
   }

   KService::Ptr s;
};

class KCustomMenuEditor::KCustomMenuEditorPrivate
{
public:
    TQPushButton * pbRemove;
    TQPushButton * pbMoveUp;
    TQPushButton * pbMoveDown;
};

KCustomMenuEditor::KCustomMenuEditor(TQWidget *parent)
  : KDialogBase(parent, "custommenueditor", true, i18n("Menu Editor"), Ok|Cancel, Ok, true),
    m_listView(0)
{
    d = new KCustomMenuEditorPrivate;
   TQHBox *page = makeHBoxMainWidget();
   m_listView = new TDEListView(page);
   m_listView->addColumn(i18n("Menu"));
   m_listView->setFullWidth(true);
   m_listView->setSorting(-1);
   KButtonBox *buttonBox = new KButtonBox(page, TQt::Vertical);
   buttonBox->addButton(i18n("New..."), this, TQ_SLOT(slotNewItem()));
   d->pbRemove=buttonBox->addButton(i18n("Remove"), this, TQ_SLOT(slotRemoveItem()));
   d->pbMoveUp=buttonBox->addButton(i18n("Move Up"), this, TQ_SLOT(slotMoveUp()));
   d->pbMoveDown=buttonBox->addButton(i18n("Move Down"), this, TQ_SLOT(slotMoveDown()));
   buttonBox->layout();
   connect( m_listView, TQ_SIGNAL( selectionChanged () ), this, TQ_SLOT( refreshButton() ) );
   refreshButton();
}

KCustomMenuEditor::~KCustomMenuEditor()
{
    delete d;
    d=0;
}

void KCustomMenuEditor::refreshButton()
{
    TQListViewItem *item = m_listView->currentItem();
    d->pbRemove->setEnabled( item );
    d->pbMoveUp->setEnabled( item && item->itemAbove() );
    d->pbMoveDown->setEnabled( item && item->itemBelow() );
}

void
KCustomMenuEditor::load(TDEConfigBase *cfg)
{
   cfg->setGroup(TQString::null);
   int count = cfg->readNumEntry("NrOfItems");
   TQListViewItem *last = 0;
   for(int i = 0; i < count; i++)
   {
      TQString entry = cfg->readPathEntry(TQString("Item%1").arg(i+1));
      if (entry.isEmpty())
         continue;

      // Try KSycoca first.
      KService::Ptr menuItem = KService::serviceByDesktopPath( entry );
      if (!menuItem)
         menuItem = KService::serviceByDesktopName( entry );
      if (!menuItem)
         menuItem = new KService( entry );

      if (!menuItem->isValid())
         continue;

      TQListViewItem *item = new Item(m_listView, menuItem);
      item->moveItem(last);
      last = item;
   }
}

void
KCustomMenuEditor::save(TDEConfigBase *cfg)
{
   // First clear the whole config file.
   TQStringList groups = cfg->groupList();
   for(TQStringList::ConstIterator it = groups.begin();
      it != groups.end(); ++it)
   {
      cfg->deleteGroup(*it);
   }

   cfg->setGroup(TQString::null);
   Item * item = (Item *) m_listView->firstChild();
   int i = 0;
   while(item)
   {
      i++;
      TQString path = item->s->desktopEntryPath();
      if (TQDir::isRelativePath(path) || TQDir::isRelativePath(TDEGlobal::dirs()->relativeLocation("xdgdata-apps", path)))
         path = item->s->desktopEntryName();
      cfg->writePathEntry(TQString("Item%1").arg(i), path);
      item = (Item *) item->nextSibling();
   }
   cfg->writeEntry("NrOfItems", i);
}

void
KCustomMenuEditor::slotNewItem()
{
   TQListViewItem *item = m_listView->currentItem();

   KOpenWithDlg dlg(this);
   dlg.setSaveNewApplications(true);

   if (dlg.exec())
   {
      KService::Ptr s = dlg.service();
      if (s && s->isValid())
      {
         Item *newItem = new Item(m_listView, s);
         newItem->moveItem(item);
      }
      refreshButton();
   }
}

void
KCustomMenuEditor::slotRemoveItem()
{
   TQListViewItem *item = m_listView->currentItem();
   if (!item)
      return;

   delete item;
   refreshButton();
}

void
KCustomMenuEditor::slotMoveUp()
{
   TQListViewItem *item = m_listView->currentItem();
   if (!item)
      return;

   TQListViewItem *searchItem = m_listView->firstChild();
   while(searchItem)
   {
      TQListViewItem *next = searchItem->nextSibling();
      if (next == item)
      {
         searchItem->moveItem(item);
         break;
      }
      searchItem = next;
   }
   refreshButton();
}

void
KCustomMenuEditor::slotMoveDown()
{
   TQListViewItem *item = m_listView->currentItem();
   if (!item)
      return;

   TQListViewItem *after = item->nextSibling();
   if (!after)
      return;

   item->moveItem( after );
   refreshButton();
}

#include "kcustommenueditor.moc"
