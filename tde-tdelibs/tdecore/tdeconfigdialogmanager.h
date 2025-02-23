/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben+tdelibs at meyerhome dot net)
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
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
#ifndef TDECONFIGDIALOGMANAGER_H
#define TDECONFIGDIALOGMANAGER_H

#include <tqobject.h>
#include <tqptrlist.h>
#include "tdelibs_export.h"

class TDEConfigSkeleton;
class TDEConfigSkeletonItem;
class TQWidget;
class TQSqlPropertyMap;

/**
 * @short Provides a means of automatically retrieving,
 * saving and resetting TDEConfigSkeleton based settings in a dialog.
 *
 * The TDEConfigDialogManager class provides a means of automatically
 * retrieving, saving and resetting basic settings.
 * It also can emit signals when settings have been changed
 * (settings were saved) or modified (the user changes a checkbox
 * from on to off).
 *
 * The names of the widgets to be managed have to correspond to the names of the
 * configuration entries in the TDEConfigSkeleton object plus an additional
 * "kcfg_" prefix. For example a widget named "kcfg_MyOption" would be
 * associated to the configuration entry "MyOption".
 *
 * TDEConfigDialogManager uses the TQSqlPropertyMap class to determine if it can do
 * anything to a widget.  Note that TDEConfigDialogManager doesn't  require a
 * database, it simply uses the functionality that is built into the
 * TQSqlPropertyMap class.  New widgets can be added to the map using
 * TQSqlPropertyMap::installDefaultMap().  Note that you can't just add any
 * class.  The class must have a matching TQ_PROPERTY(...) macro defined.
 *
 * For example (note that KColorButton is already added and it doesn't need to
 * manually added):
 *
 * kcolorbutton.h defines the following property:
 * \code
 * TQ_PROPERTY( TQColor color READ color WRITE setColor )
 * \endcode
 *
 * To add KColorButton the following code would be inserted in the main.
 *
 * \code
 * kapp->installKDEPropertyMap();
 * TQSqlPropertyMap *map = TQSqlPropertyMap::defaultMap();
 * map->insert("KColorButton", "color");
 * \endcode
 *
 * If you add a new widget to the TQSqlPropertyMap and wish to be notified when
 * it is modified you should add its signal using addWidgetChangedSignal().

 * @since 3.2
 * @author Benjamin C Meyer <ben+tdelibs at meyerhome dot net>
 * @author Waldo Bastian <bastian@kde.org>
 */
class TDECORE_EXPORT TDEConfigDialogManager : public TQObject {

TQ_OBJECT

signals:
  /**
   * One or more of the settings have been saved (such as when the user
   * clicks on the Apply button).  This is only emitted by updateSettings()
   * whenever one or more setting were changed and consequently saved.
   */
  void settingsChanged();

  /**
   * TODO: Verify
   * One or more of the settings have been changed.
   * @param widget - The widget group (pass in via addWidget()) that
   * contains the one or more modified setting.
   * @see settingsChanged()
   */
  void settingsChanged( TQWidget *widget );

  /**
   * If retrieveSettings() was told to track changes then if
   * any known setting was changed this signal will be emitted.  Note
   * that a settings can be modified several times and might go back to the
   * original saved state. hasChanged() will tell you if anything has
   * actually changed from the saved values.
   */
  void widgetModified();


public:

  /**
   * Constructor.
   * @param parent  Dialog widget to manage
   * @param conf Object that contains settings
   * @param name - Object name.
   */
   TDEConfigDialogManager(TQWidget *parent, TDEConfigSkeleton *conf, const char *name=0);

  /**
   * Destructor.
   */
  ~TDEConfigDialogManager();

  /**
   * Add additional widgets to manage
   * @param widget Additional widget to manage, inlcuding all its children
   */
  void addWidget(TQWidget *widget);

  /**
   * Returns whether the current state of the known widgets are
   * different from the state in the config object.
   */
  bool hasChanged();

  /**
   * Returns whether the current state of the known widgets are
   * the same as the default state in the config object.
   */
  bool isDefault();

public slots:
  /**
   * Traverse the specified widgets, saving the settings of all known
   * widgets in the settings object.
   *
   * Example use: User clicks Ok or Apply button in a configure dialog.
   */
  void updateSettings();

  /**
   * Traverse the specified widgets, sets the state of all known
   * widgets according to the state in the settings object.
   *
   * Example use: Initialisation of dialog.
   * Example use: User clicks Reset button in a configure dialog.
   */
  void updateWidgets();

  /**
   * Traverse the specified widgets, sets the state of all known
   * widgets according to the default state in the settings object.
   *
   * Example use: User clicks Defaults button in a configure dialog.
   */
  void updateWidgetsDefault();

protected:

  /**
   * @param trackChanges - If any changes by the widgets should be tracked
   * set true.  This causes the emitting the modified() signal when
   * something changes.
   * TODO: @return bool - True if any setting was changed from the default.
   */
  void init(bool trackChanges);

  /**
   * Recursive function that finds all known children.
   * Goes through the children of widget and if any are known and not being
   * ignored, stores them in currentGroup.  Also checks if the widget
   * should be disabled because it is set immutable.
   * @param widget - Parent of the children to look at.
   * @param trackChanges - If true then tracks any changes to the children of
   * widget that are known.
   * @return bool - If a widget was set to something other then its default.
   */
  bool parseChildren(const TQWidget *widget, bool trackChanges);

  /**
   * Set a property
   */
  void setProperty(TQWidget *w, const TQVariant &v);

  /**
   * Retrieve a property
   */
  TQVariant property(TQWidget *w);

  /**
   * Setup secondary widget properties
   */
  void setupWidget(TQWidget *widget, TDEConfigSkeletonItem *item);

protected:
  /**
   * TDEConfigSkeleton object used to store settings
   */
  TDEConfigSkeleton *m_conf;

  /**
   * Dialog being managed
   */
  TQWidget *m_dialog;

  /**
   * Pointer to the property map for easy access.
   */
  TQSqlPropertyMap *propertyMap;

  /**
   * Map of the classes and the signals that they emit when changed.
   */
  TQMap<TQString, TQCString> changedMap;

private:
  class Private;
  /**
   * TDEConfigDialogManager Private class.
   */
  Private *d;

};

#endif // TDECONFIGDIALOGMANAGER_H

