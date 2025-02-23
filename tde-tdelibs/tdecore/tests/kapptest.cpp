/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

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

#define private public
#include "tdeapplication.h"
#include <stdio.h>

#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdeglobalsettings.h>
#include <tdesycoca.h>

int
main(int argc, char *argv[])
{
   TDEAboutData about("kapptest", "kapptest", "version");
   TDECmdLineArgs::init(argc, argv, &about);

   TDEApplication a;

   KSycoca *s = KSycoca::self();
  
   tqWarning("s->language() %s", s->language().latin1());
   tqWarning("Offset of aIconName: %ld", (long) &(static_cast<TDEApplication *>(0)->aIconName));
}
