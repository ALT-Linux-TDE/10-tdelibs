/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben+tdelibs at meyerhome dot net)
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2004 Michael Brade <brade@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */
#include "tdeconfigdialog.h"

#include <tdeconfigskeleton.h>
#include <tdeconfigdialogmanager.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <tqlayout.h>
#include <tqvbox.h>
#include <tqmap.h>

TQAsciiDict<TDEConfigDialog> TDEConfigDialog::openDialogs;

// This class is here purly so we don't break binary compatibility down the road.
class TDEConfigDialog::TDEConfigDialogPrivate
{
public:
  TDEConfigDialogPrivate(KDialogBase::DialogType t)
  : shown(false), type(t), manager(0) { }

  bool shown;
  KDialogBase::DialogType type;
  TDEConfigDialogManager *manager;
  TQMap<TQWidget *, TDEConfigDialogManager *> managerForPage;
};

TDEConfigDialog::TDEConfigDialog( TQWidget *parent, const char *name,
          TDEConfigSkeleton *config,
          DialogType dialogType,
          int dialogButtons,
          ButtonCode defaultButton,
          bool modal ) :
    KDialogBase( dialogType, (WFlags)TQt::WStyle_DialogBorder,
          parent, name, modal, i18n("Configure"), dialogButtons, defaultButton ),
    d(new TDEConfigDialogPrivate(dialogType))
{
  if ( name ) {
    openDialogs.insert(name, this);
  } else {
    TQCString genericName;
    genericName.sprintf("SettingsDialog-%p", this);
    openDialogs.insert(genericName, this);
    setName(genericName);
  }

  connect(this, TQ_SIGNAL(okClicked()), this, TQ_SLOT(updateSettings()));
  connect(this, TQ_SIGNAL(applyClicked()), this, TQ_SLOT(updateSettings()));
  connect(this, TQ_SIGNAL(applyClicked()), this, TQ_SLOT(updateButtons()));
  connect(this, TQ_SIGNAL(defaultClicked()), this, TQ_SLOT(updateWidgetsDefault()));
  connect(this, TQ_SIGNAL(defaultClicked()), this, TQ_SLOT(updateButtons()));

  d->manager = new TDEConfigDialogManager(this, config);
  setupManagerConnections(d->manager);

  enableButton(Apply, false);
}

TDEConfigDialog::~TDEConfigDialog()
{
  openDialogs.remove(name());
  delete d;
}

void TDEConfigDialog::addPage(TQWidget *page,
                                const TQString &itemName,
                                const TQString &pixmapName,
                                const TQString &header,
                                bool manage)
{
  addPageInternal(page, itemName, pixmapName, header);
  if(manage)
    d->manager->addWidget(page);
}

void TDEConfigDialog::addPage(TQWidget *page,
                                TDEConfigSkeleton *config,
                                const TQString &itemName,
                                const TQString &pixmapName,
                                const TQString &header)
{
  addPageInternal(page, itemName, pixmapName, header);
  d->managerForPage[page] = new TDEConfigDialogManager(page, config);
  setupManagerConnections(d->managerForPage[page]);
}

void TDEConfigDialog::addPageInternal(TQWidget *page,
                                        const TQString &itemName,
                                        const TQString &pixmapName,
                                        const TQString &header)
{
  if(d->shown)
  {
    kdDebug(240) << "TDEConfigDialog::addPage: can not add a page after the dialog has been shown.";
    return;
  }
  switch(d->type)
  {
    case TreeList:
    case IconList:
    case Tabbed: {
      TQVBox *frame = addVBoxPage(itemName, header, SmallIcon(pixmapName, 32));
      frame->setSpacing( 0 );
      frame->setMargin( 0 );
      page->reparent(((TQWidget*)frame), 0, TQPoint());
    }
    break;

    case Swallow:
    {
      page->reparent(this, 0, TQPoint());
      setMainWidget(page);
    }
    break;

    case Plain:
    {
      TQFrame *main = plainPage();
      TQVBoxLayout *topLayout = new TQVBoxLayout( main, 0, 0 );
      page->reparent(((TQWidget*)main), 0, TQPoint());
      topLayout->addWidget( page );
    }
    break;

    default:
      kdDebug(240) << "TDEConfigDialog::addpage: unknown type.";
  }
}

void TDEConfigDialog::setupManagerConnections(TDEConfigDialogManager *manager)
{
  connect(manager, TQ_SIGNAL(settingsChanged()), this, TQ_SLOT(settingsChangedSlot()));
  connect(manager, TQ_SIGNAL(widgetModified()), this, TQ_SLOT(updateButtons()));

  connect(this, TQ_SIGNAL(okClicked()), manager, TQ_SLOT(updateSettings()));
  connect(this, TQ_SIGNAL(applyClicked()), manager, TQ_SLOT(updateSettings()));
  connect(this, TQ_SIGNAL(defaultClicked()), manager, TQ_SLOT(updateWidgetsDefault()));
}

TDEConfigDialog* TDEConfigDialog::exists(const char* name)
{
  return openDialogs.find(name);
}

bool TDEConfigDialog::showDialog(const char* name)
{
  TDEConfigDialog *dialog = exists(name);
  if(dialog)
    dialog->show();
  return (dialog != NULL);
}

void TDEConfigDialog::updateButtons()
{
  static bool only_once = false;
  if (only_once) return;
  only_once = true;

  TQMap<TQWidget *, TDEConfigDialogManager *>::iterator it;

  bool has_changed = d->manager->hasChanged() || hasChanged();
  for (it = d->managerForPage.begin();
          it != d->managerForPage.end() && !has_changed;
          ++it)
  {
    has_changed |= (*it)->hasChanged();
  }

  enableButton(Apply, has_changed);

  bool is_default = d->manager->isDefault() && isDefault();
  for (it = d->managerForPage.begin();
          it != d->managerForPage.end() && is_default;
          ++it)
  {
    is_default &= (*it)->isDefault();
  }

  enableButton(Default, !is_default);

  emit widgetModified();
  only_once = false;
}

void TDEConfigDialog::settingsChangedSlot()
{
  // Update the buttons
  updateButtons();
  emit settingsChanged();
  emit settingsChanged(name());
}

void TDEConfigDialog::show()
{
  TQMap<TQWidget *, TDEConfigDialogManager *>::iterator it;

  updateWidgets();
  d->manager->updateWidgets();
  for (it = d->managerForPage.begin(); it != d->managerForPage.end(); ++it)
    (*it)->updateWidgets();

  bool has_changed = d->manager->hasChanged() || hasChanged();
  for (it = d->managerForPage.begin();
          it != d->managerForPage.end() && !has_changed;
          ++it)
  {
    has_changed |= (*it)->hasChanged();
  }

  enableButton(Apply, has_changed);

  bool is_default = d->manager->isDefault() && isDefault();
  for (it = d->managerForPage.begin();
          it != d->managerForPage.end() && is_default;
          ++it)
  {
    is_default &= (*it)->isDefault();
  }

  enableButton(Default, !is_default);
  d->shown = true;
  KDialogBase::show();
}

void TDEConfigDialog::updateSettings()
{
}

void TDEConfigDialog::updateWidgets()
{
}

void TDEConfigDialog::updateWidgetsDefault()
{
}


#include "tdeconfigdialog.moc"
