/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

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

// $Id$

#include "blockselectioninterface.h"
#include "blockselectiondcopinterface.h"
#include "document.h"

namespace KTextEditor
{

class PrivateBlockSelectionInterface
{
  public:
    PrivateBlockSelectionInterface() {interface = 0;}
    ~PrivateBlockSelectionInterface() {}
    BlockSelectionDCOPInterface *interface;
};

}

using namespace KTextEditor;

unsigned int BlockSelectionInterface::globalBlockSelectionInterfaceNumber = 0;

BlockSelectionInterface::BlockSelectionInterface()
{
  globalBlockSelectionInterfaceNumber++;
  myBlockSelectionInterfaceNumber = globalBlockSelectionInterfaceNumber++;
  TQString name = "BlockSelectionInterface#" + TQString::number(myBlockSelectionInterfaceNumber);

  d = new PrivateBlockSelectionInterface();
  d->interface = new BlockSelectionDCOPInterface(this, name.latin1());
}

BlockSelectionInterface::~BlockSelectionInterface()
{
  delete d->interface;
  delete d;
}

unsigned int BlockSelectionInterface::blockSelectionInterfaceNumber () const
{
  return myBlockSelectionInterfaceNumber;
}

void BlockSelectionInterface::setBlockSelectionInterfaceDCOPSuffix (const TQCString &suffix)
{
  d->interface->setObjId ("BlockSelectionInterface#"+suffix);
}

BlockSelectionInterface *KTextEditor::blockSelectionInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::BlockSelectionInterface*>(doc);
}
