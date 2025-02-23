/* This file is part of the KDE libraries
   Copyright (C) 2001 - 2004 Anders Lund <anders@alweb.dk>

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

#include "kmimetypechooser.h"

#include <tdeconfig.h>
#include <kiconloader.h>
#include <tdelistview.h>
#include <tdelocale.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <krun.h>
#include <tdesycoca.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

//BEGIN KMimeTypeChooserPrivate
class KMimeTypeChooserPrivate
{
  public:
    TDEListView *lvMimeTypes;
    TQPushButton *btnEditMimeType;

    TQString defaultgroup;
    TQStringList groups;
    int visuals;
};
//END

//BEGIN KMimeTypeChooser
KMimeTypeChooser::KMimeTypeChooser( const TQString &text,
                              const TQStringList &selMimeTypes,
                              const TQString &defaultGroup,
                              const TQStringList &groupsToShow,
                              int visuals,
                              TQWidget *parent,
                              const char *name )
    : TQVBox( parent, name )
{
  d = new KMimeTypeChooserPrivate();
  d->lvMimeTypes = 0;
  d->btnEditMimeType = 0;
  d->defaultgroup = defaultGroup;
  d->groups = groupsToShow;
  d->visuals = visuals;

  setSpacing( KDialogBase::spacingHint() );

  if ( !text.isEmpty() )
  {
    new TQLabel( text, this );
  }

  d->lvMimeTypes = new TDEListView( this );

  d->lvMimeTypes->addColumn( i18n("Mime Type") );
//   d->lvMimeTypes->setColumnWidthMode( 0, TQListView::Manual );

  if ( visuals & Comments )
  {
    d->lvMimeTypes->addColumn( i18n("Comment") );
    d->lvMimeTypes->setColumnWidthMode( 1, TQListView::Manual );
  }
  if ( visuals & Patterns )
    d->lvMimeTypes->addColumn( i18n("Patterns") );

  d->lvMimeTypes->setRootIsDecorated( true );

  loadMimeTypes( selMimeTypes );

  if (visuals & KMimeTypeChooser::EditButton)
  {
    TQHBox *btns = new TQHBox( this );
    ((TQBoxLayout*)btns->layout())->addStretch(1);
    d->btnEditMimeType = new TQPushButton( i18n("&Edit..."), btns );

    connect( d->btnEditMimeType, TQ_SIGNAL(clicked()), this, TQ_SLOT(editMimeType()) );
    d->btnEditMimeType->setEnabled( false );
    connect( d->lvMimeTypes, TQ_SIGNAL( doubleClicked ( TQListViewItem * )),
             this, TQ_SLOT( editMimeType()));
    connect( d->lvMimeTypes, TQ_SIGNAL(currentChanged(TQListViewItem*)),
             this, TQ_SLOT(slotCurrentChanged(TQListViewItem*)) );

    TQWhatsThis::add( d->btnEditMimeType, i18n(
        "Click this button to display the familiar TDE mime type editor.") );
  }
}

KMimeTypeChooser::~KMimeTypeChooser()
{
  delete d;
}

void KMimeTypeChooser::loadMimeTypes( const TQStringList &_selectedMimeTypes )
{
  TQStringList selMimeTypes;

  if ( !_selectedMimeTypes.isEmpty() )
    selMimeTypes = _selectedMimeTypes;
  else
    selMimeTypes = mimeTypes();

  d->lvMimeTypes->clear();

  TQMap<TQString,TQListViewItem*> groups;
  // thanks to tdebase/kcontrol/filetypes/filetypesview
  KMimeType::List mimetypes = KMimeType::allMimeTypes();
  TQValueListIterator<KMimeType::Ptr> it(mimetypes.begin());

  TQListViewItem *groupItem;
  bool agroupisopen = false;
  TQListViewItem *idefault = 0; //open this, if all other fails
  TQListViewItem *firstChecked = 0; // make this one visible after the loop

  for (; it != mimetypes.end(); ++it)
  {
    TQString mimetype = (*it)->name();
    int index = mimetype.find("/");
    TQString maj = mimetype.left(index);

    if ( d->groups.count() && !d->groups.contains( maj ) )
      continue;

    TQString min = mimetype.right(mimetype.length() - (index+1));

    TQMapIterator<TQString,TQListViewItem*> mit = groups.find( maj );
    if ( mit == groups.end() )
    {
      groupItem = new TQListViewItem( d->lvMimeTypes, maj );
      groups.insert( maj, groupItem );
       if ( maj == d->defaultgroup )
         idefault = groupItem;
    }
    else
        groupItem = mit.data();

    TQCheckListItem *item = new TQCheckListItem( groupItem, min, TQCheckListItem::CheckBox );
    item->setPixmap( 0, SmallIcon( (*it)->icon(TQString::null,false) ) );

    int cl = 1;

    if ( d->visuals & Comments )
    {
      item->setText( cl, (*it)->comment(TQString::null, false) );
      cl++;
    }

    if ( d->visuals & Patterns )
      item->setText( cl, (*it)->patterns().join("; ") );

    if ( selMimeTypes.contains(mimetype) )
    {
      item->setOn( true );
      groupItem->setOpen( true );
      agroupisopen = true;
      if ( !firstChecked )
        firstChecked = item;
    }
  }

  if ( firstChecked )
    d->lvMimeTypes->ensureItemVisible( firstChecked );

  if ( !agroupisopen && idefault )
  {
    idefault->setOpen( true );
    d->lvMimeTypes->ensureItemVisible( idefault );
  }
}

void KMimeTypeChooser::editMimeType()
{
  if ( !(d->lvMimeTypes->currentItem() && (d->lvMimeTypes->currentItem())->parent()) )
    return;
  TQString mt = (d->lvMimeTypes->currentItem()->parent())->text( 0 ) + "/" + (d->lvMimeTypes->currentItem())->text( 0 );
  // thanks to libkonq/konq_operations.cpp
  connect( KSycoca::self(), TQ_SIGNAL(databaseChanged()),
           this, TQ_SLOT(slotSycocaDatabaseChanged()) );
  TQString keditfiletype = TQString::fromLatin1("keditfiletype");
  KRun::runCommand( keditfiletype
                    + " --parent " + TQString::number( (ulong)topLevelWidget()->winId())
                    + " " + TDEProcess::quote(mt),
                    keditfiletype, keditfiletype /*unused*/);
}

void KMimeTypeChooser::slotCurrentChanged(TQListViewItem* i)
{
  if ( d->btnEditMimeType )
    d->btnEditMimeType->setEnabled( i->parent() );
}

void KMimeTypeChooser::slotSycocaDatabaseChanged()
{
  if ( KSycoca::self()->isChanged("mime") )
    loadMimeTypes();
}

TQStringList KMimeTypeChooser::mimeTypes() const
{
  TQStringList l;
  TQListViewItemIterator it( d->lvMimeTypes );
  for (; it.current(); ++it)
  {
    if ( it.current()->parent() && ((TQCheckListItem*)it.current())->isOn() )
      l << it.current()->parent()->text(0) + "/" + it.current()->text(0); // FIXME uncecked, should be Ok unless someone changes mimetypes during this!
  }
  return l;
}

TQStringList KMimeTypeChooser::patterns() const
{
  TQStringList l;
  KMimeType::Ptr p;
  TQString defMT = KMimeType::defaultMimeType();
  TQListViewItemIterator it( d->lvMimeTypes );
  for (; it.current(); ++it)
  {
    if ( it.current()->parent() && ((TQCheckListItem*)it.current())->isOn() )
    {
      p = KMimeType::mimeType( it.current()->parent()->text(0) + "/" + it.current()->text(0) );
      if ( p->name() != defMT )
        l += p->patterns();
    }
  }
  return l;
}
//END

//BEGIN KMimeTypeChooserDialog
KMimeTypeChooserDialog::KMimeTypeChooserDialog(
                         const TQString &caption,
                         const TQString& text,
                         const TQStringList &selMimeTypes,
                         const TQString &defaultGroup,
                         const TQStringList &groupsToShow,
                         int visuals,
                         TQWidget *parent, const char *name )
    : KDialogBase(parent, name, true, caption, Cancel|Ok, Ok)
{
  m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                  defaultGroup, groupsToShow, visuals,
                                  this, "chooser" );
  setMainWidget(m_chooser);

  TDEConfigGroup group( TDEGlobal::config(), "KMimeTypeChooserDialog");
  TQSize defaultSize( 400, 300 );
  resize( group.readSizeEntry("size", &defaultSize) );
}

KMimeTypeChooserDialog::KMimeTypeChooserDialog(
                         const TQString &caption,
                         const TQString& text,
                         const TQStringList &selMimeTypes,
                         const TQString &defaultGroup,
                         TQWidget *parent, const char *name )
    : KDialogBase(parent, name, true, caption, Cancel|Ok, Ok)
{
  m_chooser = new KMimeTypeChooser( text, selMimeTypes,
                                  defaultGroup, TQStringList(),
                                  KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns|KMimeTypeChooser::EditButton,
                                  this, "chooser" );
  setMainWidget(m_chooser);

  TDEConfigGroup group( TDEGlobal::config(), "KMimeTypeChooserDialog");
  TQSize defaultSize( 400, 300 );
  resize( group.readSizeEntry("size", &defaultSize) );
}


KMimeTypeChooserDialog::~KMimeTypeChooserDialog()
{
  TDEConfigGroup group( TDEGlobal::config(), "KMimeTypeChooserDialog");
  group.writeEntry("size", size());
}

//END KMimeTypeChooserDialog
#include "kmimetypechooser.moc"
