/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)

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

#ifndef _TDETOOLBARRADIOGROUP_H
#define _TDETOOLBARRADIOGROUP_H

#include <tqobject.h>

#include <tdelibs_export.h>

class TDEToolBar;
class TDEToolBarButtonList;
class TDEToolBarRadioGroupPrivate;

/*************************************************************************
 *                          TDEToolBarRadioGroup                                  *
 *************************************************************************/
 /**
  * @short Class for group of radio butons in toolbar.
  *
  * TDEToolBarRadioGroup is class for group of radio butons in toolbar.
  * Take toggle buttons which you already inserted into toolbar,
  * create TDEToolBarRadioGroup instance and add them here.
  * All buttons will emit signals toggled (bool) (or you can
  * use sitgnal toggled (int id) from toolbar). When one button is set
  * down, all others are unset. All buttons emit signals - those who
  * "go down" and those who "go up".
  *
  * @author Sven Radej <radej@kde.org>
  */
class TDEUI_EXPORT TDEToolBarRadioGroup : public TQObject
{
  TQ_OBJECT

public:
  /**
   * Constructor. Parent must be TDEToolBar .
   */
  TDEToolBarRadioGroup (TDEToolBar *_parent, const char *_name=0);
  /**
   * Destructor.
   */
  ~TDEToolBarRadioGroup ();

  /**
   * Adds button to group. Button cannot be unset by mouse clicks (you
   * must press some other button tounset this one)
   */
  void addButton (int id);

  /**
   * Removes button from group, making it again toggle button (i.e.
   * You can unset it with mouse).
   */
  void removeButton (int id);

public slots:
  /**
   * Internal - nothing for you here.
   */
  void slotToggled (int);

private:
  TDEToolBarButtonList *buttons;
  TDEToolBar *tb;

  TDEToolBarRadioGroupPrivate *d;
};

#endif
