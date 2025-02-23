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
#ifndef TDECONFIGDIALOG_H
#define TDECONFIGDIALOG_H

class TDEConfig;
class TDEConfigSkeleton;
class TDEConfigDialogManager;
#include <kdialogbase.h>
#include <tqasciidict.h>

/**
 * \short Standard %KDE configuration dialog class
 *
 * The TDEConfigDialog class provides an easy and uniform means of displaying
 * a settings dialog using KDialogBase, TDEConfigDialogManager and a
 * TDEConfigSkeleton derived settings class.
 *
 * TDEConfigDialog handles the enabling and disabling of buttons, creation
 * of the dialog, and deletion of the widgets.  Because of
 * TDEConfigDialogManager, this class also manages: restoring
 * the settings, reseting them to the default values, and saving them. This
 * requires that the names of the widgets corresponding to configuration entries
 * have to have the same name plus an additional "kcfg_" prefix. For example the
 * widget named "kcfg_MyOption" would be associated with the configuration entry
 * "MyOption".
 *
 * Here is an example usage of TDEConfigDialog:
 *
 * \code
 * void KCoolApp::showSettings(){
 *   if(TDEConfigDialog::showDialog("settings"))
 *     return;
 *   TDEConfigDialog *dialog = new TDEConfigDialog(this, "settings", MySettings::self(), KDialogBase::IconList);
 *   dialog->addPage(new General(0, "General"), i18n("General") );
 *   dialog->addPage(new Appearance(0, "Style"), i18n("Appearance") );
 *   connect(dialog, TQ_SIGNAL(settingsChanged()), mainWidget, TQ_SLOT(loadSettings()));
 *   connect(dialog, TQ_SIGNAL(settingsChanged()), this, TQ_SLOT(loadSettings()));
 *   dialog->show();
 * }
 * \endcode
 *
 * Other than the above code, each class that has settings in the dialog should
 * have a loadSettings() type slot to read settings and perform any
 * necessary changes.
 *
 * Please note that using the setMainWidget method inherited from KDialogBase
 * currently yields broken behaviour at runtime; use @ref addPage() instead.
 *
 * @see TDEConfigSkeleton
 * @author Waldo Bastian <bastian@kde.org>
 * @since 3.2
 */
class TDEUI_EXPORT TDEConfigDialog : public KDialogBase {
TQ_OBJECT

signals:
  /**
   * A widget in the dialog was modified.
   */
  void widgetModified();

  /**
   * One or more of the settings have been permanently changed such as if
   * the user clicked on the Apply or Ok button.
   */
  void settingsChanged();

  /**
   * One or more of the settings have been permanently changed such as if
   * the user clicked on the Apply or Ok button.
   * This signal is useful when using TDEConfigDialog to configure
   * items in a list.  When emitted the main class would then know what
   * item in the list was actually changed.
   * @param dialogName the name of the dialog.
   */
  void settingsChanged(const char *dialogName);

public:
  /**
   * @param parent - The parent of this object.  Even though the class
   * deletes itself the parent should be set so the dialog can be centered
   * with the application on the screen.
   *
   * @param name - The name of this object.  The name is used in determining if
   * there can be more than one dialog at a time.  Use names such as:
   * "Font Settings" or "Color Settings" and not just "Settings" in
   * applications where there is more than one dialog.
   *
   * @param dialogType - Type used in creating the dialog.  See KDialogBase
   *
   * @param config - Config object containing settings.
   *
   * @param dialogButtons - Buttons that should show up on the dialog.
   *
   * @param defaultButton default button that is choosen by hitting the enter key.
   *
   * @param modal - Whether the dialog should be modal. To prevent more than one
   * non-modal settings dialog from showing the static function showDialog() can be
   * used in determining if the settings dialog already exists before creating
   * a new TDEConfigDialog object.
   */
  // KDE4: Add the "separator" parameter as in KDialogBase
  //       Make "dialogType" an int
  TDEConfigDialog( TQWidget *parent, const char *name,
                 TDEConfigSkeleton *config,
                 DialogType dialogType = IconList,
                 int dialogButtons = Default|Ok|Apply|Cancel|Help,
                 ButtonCode defaultButton = Ok,
                 bool modal=false );

  /**
   * Deconstructor, removes name from the list of open dialogs.
   * Deletes private class.
   * @see exists()
   */
  ~TDEConfigDialog();

  /**
   * Adds page to the dialog and to TDEConfigDialogManager.  When an
   * application is done adding pages show() should be called to
   * display the dialog.
   * Note that after you call show() you can not add any more pages
   * to the dialog.
   * @param page - Pointer to the page that is to be added to the dialog.
   * This object is reparented.
   * @param itemName - Name of the page.
   * @param pixmapName - Name of the pixmap that should be used if needed.
   * @param header - Header text use in the list modes. Ignored in Tabbed
   *        mode. If empty, the itemName text is used when needed.
   * @param manage - Whether TDEConfigDialogManager should manage the page or not.
   */
  // KDE4: Add a default value for itemName & pixmapName
  void addPage( TQWidget *page, const TQString &itemName,
                const TQString &pixmapName,
                const TQString &header=TQString::null,
                bool manage=true );

  /**
   * Adds page to the dialog that is managed by a custom TDEConfigDialogManager.
   * This is useful for dialogs that contain settings spread over more than
   * one configuration file and thus have/need more than one TDEConfigSkeleton.
   * When an application is done adding pages show() should be called to
   * display the dialog.
   * Note that after you call show() you can not add any more pages
   * to the dialog.
   * @param page - Pointer to the page that is to be added to the dialog.
   * This object is reparented.
   * @param config - Config object containing corresponding settings.
   * @param itemName - Name of the page.
   * @param pixmapName - Name of the pixmap that should be used if needed.
   * @param header - Header text use in the list modes. Ignored in Tabbed
   *        mode. If empty, the itemName text is used when needed.
   */
  // KDE4: Add a default value for itemName & pixmapName
  void addPage( TQWidget *page, TDEConfigSkeleton *config,
                const TQString &itemName,
                const TQString &pixmapName,
                const TQString &header=TQString::null );

  /**
   * See if a dialog with the name 'name' already exists.
   * @see showDialog()
   * @param name - Dialog name to look for.
   * @return Pointer to widget or NULL if it does not exist.
   */
  static TDEConfigDialog* exists( const char* name );

  /**
   * Attempts to show the dialog with the name 'name'.
   * @see exists()
   * @param name - The name of the dialog to show.
   * @return True if the dialog 'name' exists and was shown.
   */
  static bool showDialog( const char* name );

  /**
   * Show the dialog.
   */
  virtual void show();

protected slots:
  /**
   * Update the settings from the dialog.
   * Virtual function for custom additions.
   *
   * Example use: User clicks Ok or Apply button in a configure dialog.
   */
  virtual void updateSettings();

  /**
   * Update the dialog based on the settings.
   * Virtual function for custom additions.
   *
   * Example use: Initialisation of dialog.
   * Example use: User clicks Reset button in a configure dialog.
   */
  virtual void updateWidgets();

  /**
   * Update the dialog based on the default settings.
   * Virtual function for custom additions.
   *
   * Example use: User clicks Defaults button in a configure dialog.
   */
  virtual void updateWidgetsDefault();

protected:

  /**
   * Returns whether the current state of the dialog is
   * different from the current configutation.
   * Virtual function for custom additions.
   */
  virtual bool hasChanged() { return false; }

  /**
   * Returns whether the current state of the dialog is
   * the same as the default configuration.
   */
  virtual bool isDefault() { return true; }

protected slots:
  /**
   * Updates the Apply and Default buttons.
   */
  void updateButtons();

  /**
   * Some setting was changed. Emit the signal with the dialogs name
   */
  void settingsChangedSlot();

private:
  /**
   * Internal function with common addPage code.
   */
  void addPageInternal(TQWidget *page, const TQString &itemName,
                           const TQString &pixmapName, const TQString &header);

  /**
   * Sets the connections from a manager to the dialog (and the other
   * way round) up.
   */
  void setupManagerConnections(TDEConfigDialogManager *manager);

private:
  /**
   * The list of existing dialogs.
   */
  static TQAsciiDict<TDEConfigDialog> openDialogs;

  class TDEConfigDialogPrivate;
  /**
   * Private class.
   */
  TDEConfigDialogPrivate *d;
};

#endif //TDECONFIGDIALOG_H

