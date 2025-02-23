/* This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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
/*
 * KAuthIcon - an icon which shows whether privileges are in effect
 */

#include <unistd.h> // For getuid

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqtimer.h>

#include <tdelocale.h>

#include "kauthicon.h"

/* XPM */
static const char * const lock_xpm[] = {
"22 22 5 1",
"       c None",
".      c #808080",
"+      c #000000",
"@      c #FFFFFF",
"#      c #C0C0C0",
"                      ",
"                      ",
"                      ",
"                      ",
"        .+++.         ",
"        .@@@.+        ",
"      ..@+++@..       ",
"      +@+...+@+       ",
"      +@+.  +@+.      ",
"      +@+.  +@+.      ",
"     +++++++++++      ",
"     +#########+.     ",
"     +#.......#+.     ",
"     +#@@@@@@@#+.     ",
"     +#.......#+.     ",
"     +#########+.     ",
"     +++++++++++.     ",
"      ...........     ",
"                      ",
"                      ",
"                      ",
"                      "};

/* XPM */
static const char * const openlock_xpm[] = {
"22 22 5 1",
"       c None",
".      c #808080",
"+      c #000000",
"@      c #FFFFFF",
"#      c #C0C0C0",
"                      ",
"                      ",
"        .+++.         ",
"        .@@@.+        ",
"      ..@+++@..       ",
"      +@+...+@+       ",
"      +@+.  +@+.      ",
"      +@+.  +@+.      ",
"      +++.  +@+.      ",
"       ...  +@+.      ",
"            +@+.      ",
"     +++++++++++      ",
"     +#########+.     ",
"     +#.......#+.     ",
"     +#@@@@@@@#+.     ",
"     +#.......#+.     ",
"     +#########+.     ",
"     +++++++++++.     ",
"      ...........     ",
"                      ",
"                      ",
"                      "};

KAuthIcon::KAuthIcon(TQWidget *parent, const char *name)
  : TQWidget(parent, name),
   lockPM( const_cast< const char** >( lock_xpm)),
   openLockPM( const_cast< const char** >(openlock_xpm))
{
  lockText = i18n("Editing disabled");
  openLockText = i18n("Editing enabled");

  lockBox = new TQLabel(this);
  lockBox->setFrameStyle(TQFrame::WinPanel|TQFrame::Raised);
  lockBox->setPixmap(lockPM);
  lockBox->setFixedSize(lockBox->sizeHint());

  lockLabel = new TQLabel(this);
  lockLabel->setFrameStyle(TQFrame::NoFrame);

  // set fixed size of this frame to whichever phrase is longer
  if (lockLabel->fontMetrics().boundingRect(lockText).width() >
      lockLabel->fontMetrics().boundingRect(openLockText).width())
    lockLabel->setText(lockText);
  else
    lockLabel->setText(openLockText);
  lockLabel->setAlignment(AlignCenter);
  lockLabel->setMinimumSize(lockLabel->sizeHint());
  lockLabel->setText(lockText);

  layout = new TQHBoxLayout(this);

  layout->addWidget(lockBox, 0, AlignLeft|AlignVCenter);
  layout->addSpacing(5);
  layout->addWidget(lockLabel, 0, AlignRight|AlignVCenter);

  layout->activate();
  resize(sizeHint());
}

KAuthIcon::~KAuthIcon()
{
}


TQSize KAuthIcon::sizeHint() const
{
  return layout->minimumSize();
}


/************************************************************************/

KRootPermsIcon::KRootPermsIcon(TQWidget *parent, const char *name)
  : KAuthIcon(parent, name)
{
  updateStatus();
}


KRootPermsIcon::~KRootPermsIcon()
{
}

void KRootPermsIcon::updateStatus()
{
  const bool newRoot = (geteuid() == 0);
  lockBox->setPixmap(newRoot ? openLockPM : lockPM);
  lockLabel->setText(newRoot ? openLockText : lockText);
  update();
  if (root != newRoot) {
    root = newRoot;
    emit authChanged(newRoot);
  }
}

/************************************************************************/

KWritePermsIcon::KWritePermsIcon(const TQString & fileName,
				 TQWidget *parent, const char *name)
  : KAuthIcon(parent, name)
{
  fi.setFile(fileName);
  updateStatus();
}


KWritePermsIcon::~KWritePermsIcon()
{
}

void KWritePermsIcon::updateStatus()
{
  bool newwrite;
  newwrite = fi.isWritable();
  lockBox->setPixmap(newwrite ? openLockPM : lockPM);
  lockLabel->setText(newwrite ? openLockText : lockText);
  update();
  if (writable != newwrite) {
    writable = newwrite;
    emit authChanged(newwrite);
  }
}

void KAuthIcon::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KRootPermsIcon::virtual_hook( int id, void* data )
{ KAuthIcon::virtual_hook( id, data ); }

void KWritePermsIcon::virtual_hook( int id, void* data )
{ KAuthIcon::virtual_hook( id, data ); }

#include "kauthicon.moc"
