/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqlayout.h>
#include <tqstring.h>
#include <tqlabel.h>

#include <tdelistview.h>
#include <tdelocale.h>
#include <tdemessagebox.h>

#include "engine.h"
#include "provider.h"

#include "providerdialog.h"
#include "providerdialog.moc"

using namespace KNS;

class ProviderItem : public TDEListViewItem
{
  public:
    ProviderItem( TDEListView *parent, Provider *provider ) :
      TDEListViewItem( parent ), mProvider( provider )
    {
      setText( 0, provider->name() );
    }

    Provider *provider() { return mProvider; }

  private:
    Provider *mProvider;
};

ProviderDialog::ProviderDialog( Engine *engine, TQWidget *parent ) :
  KDialogBase( Plain, i18n("Hot New Stuff Providers"), Ok | Cancel, Cancel,
               parent, 0, false, true ),
  mEngine( engine )
{
  TQFrame *topPage = plainPage();

  TQBoxLayout *topLayout = new TQVBoxLayout( topPage );

  TQLabel *description = new TQLabel( i18n("Please select one of the providers listed below:"), topPage );
  topLayout->addWidget( description );

  mListView = new TDEListView( topPage );
  mListView->addColumn( i18n("Name") );
  topLayout->addWidget( mListView );
}

void ProviderDialog::clear()
{
  mListView->clear();
}

void ProviderDialog::addProvider( Provider *provider )
{
  new ProviderItem( mListView, provider );
  if ( mListView->childCount() == 1 ) {
    mListView->setSelected(mListView->firstChild(), true);
  } else if (mListView->childCount() > 1) {
    mListView->setSelected(mListView->firstChild(), false);
  }
}

void ProviderDialog::slotOk()
{
  ProviderItem *item = static_cast<ProviderItem *>( mListView->selectedItem() );
  if ( !item ) {
    KMessageBox::error( this, i18n("No provider selected.") );
    return;
  }

  mEngine->requestMetaInformation( item->provider() );

  accept();
}
