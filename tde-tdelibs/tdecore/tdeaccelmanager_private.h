/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias Hoelzer-Kluepfel <mhk@kde.org>

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


#ifndef __TDEACCELMANAGER_PRIVATE_H__
#define __TDEACCELMANAGER_PRIVATE_H__


#include <tqstring.h>
#include <tqmemarray.h>
#include <tqvaluelist.h>
#include <tqobject.h>

class TQWidgetStack;

/**
 * A string class handling accelerators.
 *
 * This class contains a string and knowledge about accelerators.
 * It keeps a list weights, telling how valuable each character
 * would be as an accelerator.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
*/

class TDEAccelString
{
public:

  TDEAccelString() : m_pureText(), m_accel(-1) {}
  TDEAccelString(const TQString &input, int initalWeight=-1);

  void calculateWeights(int initialWeight);

  const TQString &pure() const { return m_pureText; }
  TQString accelerated() const;

  int accel() const { return m_accel; }
  void setAccel(int accel) { m_accel = accel; }

  int originalAccel() const { return m_orig_accel; }
  TQString originalText() const { return m_origText; }

  TQChar accelerator() const;

  int maxWeight(int &index, const TQString &used);

  bool operator == (const TDEAccelString &c) const { return m_pureText == c.m_pureText && m_accel == c.m_accel && m_orig_accel == c.m_orig_accel; }


private:

  int stripAccelerator(TQString &input);

  void dump();

  TQString        m_pureText,  m_origText;
  int            m_accel, m_orig_accel;
  TQMemArray<int> m_weight;

};


typedef TQValueList<TDEAccelString> TDEAccelStringList;


/**
 * This class encapsulates the algorithm finding the 'best'
 * distribution of accelerators in a hierarchy of widgets.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
*/

class TDEAccelManagerAlgorithm
{
public:

  /// Constants used in the algorithm
  enum {
    /// Default control weight
    DEFAULT_WEIGHT = 50,
    /// Additional weight for first character in string
    FIRST_CHARACTER_EXTRA_WEIGHT = 50,
    /// Additional weight for the beginning of a word
    WORD_BEGINNING_EXTRA_WEIGHT = 50,
    /// Additional weight for the dialog buttons (large, we basically never want these reassigned)
    DIALOG_BUTTON_EXTRA_WEIGHT = 300,
    /// Additional weight for a 'wanted' accelerator
    WANTED_ACCEL_EXTRA_WEIGHT = 150,
    /// Default weight for an 'action' widget (ie, pushbuttons)
    ACTION_ELEMENT_WEIGHT = 50,
    /// Default weight for group boxes (low priority)
    GROUP_BOX_WEIGHT = -2000,
    /// Default weight for menu titles
    MENU_TITLE_WEIGHT = 250,
    /// Additional weight for KDE standard accelerators
    STANDARD_ACCEL = 300
  };

  /// Method to call to find the best distribution of accelerators.
  static void findAccelerators(TDEAccelStringList &result, TQString &used);

};


class TQPopupMenu;


/**
 * This class manages a popup menu. It will notice if entries have been
 * added or changed, and will recalculate the accelerators accordingly.
 *
 * This is necessary for dynamic menus like for example in kicker.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
*/

class TDEPopupAccelManager : public TQObject
{
  TQ_OBJECT

public:

  static void manage(TQPopupMenu *popup);


protected:

  TDEPopupAccelManager(TQPopupMenu *popup);


private slots:

  void aboutToShow();


private:

  void calculateAccelerators();

  void findMenuEntries(TDEAccelStringList &list);
  void setMenuEntries(const TDEAccelStringList &list);

  TQPopupMenu       *m_popup;
  TDEAccelStringList m_entries;
  int              m_count;

};


class QWidgetStackAccelManager : public TQObject
{
  TQ_OBJECT

public:

  static void manage(TQWidgetStack *popup);


protected:

  QWidgetStackAccelManager(TQWidgetStack *popup);


private slots:

  void aboutToShow(TQWidget *);
    bool eventFilter ( TQObject * watched, TQEvent * e );

private:

  void calculateAccelerators();

  TQWidgetStack     *m_stack;
  TDEAccelStringList m_entries;

};


#endif
