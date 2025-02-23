/*
   This file is part of the KDE libraries

   Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

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

#ifndef TDECMODULE_H
#define TDECMODULE_H

#include <tqwidget.h>

#include <tdelibs_export.h>

class TQStringList;

class TDEAboutData;
class TDEConfigDialogManager;
class TDEConfigSkeleton;
class TDECModulePrivate;
class TDEInstance;

/**
 * The base class for control center modules.
 *
 * Starting from KDE 2.0, control center modules are realized as shared
 * libraries that are loaded into the control center at runtime.
 *
 * The module in principle is a simple widget displaying the
 * item to be changed. The module has a very small interface.
 *
 * All the necessary glue logic and the GUI bells and whistles
 * are provided by the control center and must not concern
 * the module author.
 *
 * To write a config module, you have to create a library
 * that contains at one factory function like this:
 *
 * \code
 * #include <kgenericfactory.h>
 *
 * typedef KGenericFactory<YourTDECModule, TQWidget> YourTDECModuleFactory;
 * K_EXPORT_COMPONENT_FACTORY( yourLibName, YourTDECModuleFactory("name_of_the_po_file") );
 * \endcode
 *
 * The parameter "name_of_the_po_file" has to correspond with the messages target
 * that you created in your Makefile.am.
 *
 * See http://developer.kde.org/documentation/other/kcm_howto.html
 * for more detailed documentation.
 *
 * @author Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 */
class TDEUI_EXPORT TDECModule : public TQWidget
{
  TQ_OBJECT

public:

  /**
   * An enumeration type for the buttons used by this module.
   * You should only use Help, Default and Apply. The rest is obsolete.
   *
   * @see TDECModule::buttons @see TDECModule::setButtons
   */
  enum Button {Help=1, Default=2, Apply=16,
               Reset=4, /* obsolete, do not use! */
               Cancel=8, /* obsolete, do not use! */
               Ok=32, /* obsolete, do not use! */
  	       SysDefault=64 /* obsolete, do not use! */ };

  /*
   * Base class for all KControlModules.
   * Make sure you have a TQStringList argument in your
   * implementation.
   */
  TDECModule(TQWidget *parent=0, const char *name=0, const TQStringList &args=TQStringList() );

  TDECModule(TDEInstance *instance, TQWidget *parent=0, const TQStringList &args=TQStringList() );

  /*
   * Destroys the module.
   */
  ~TDECModule();

  /**
   * Load the configuration data into the module.
   *
   * The load method sets the user interface elements of the
   * module to reflect the current settings stored in the
   * configuration files.
   *
   * This method is invoked whenever the module should read its configuration
   * (most of the times from a config file) and update the user interface.
   * This happens when the user clicks the "Reset" button in the control
   * center, to undo all of his changes and restore the currently valid
   * settings. 
   *
   * If you use TDEConfigXT, loading is taken care of automatically and 
   * you do not need to do it manually. However, if you for some reason reimplement it and
   * also are using TDEConfigXT, you must call this function otherwise the loading of TDEConfigXT 
   * options will not work.
   *
   */
  virtual void load();
  // ### KDE 4: Call load() automatically through a single-shot timer
  //            from the constructor // and change documentation

  /**
   * Save the configuration data.
   *
   * The save method stores the config information as shown
   * in the user interface in the config files.
   *
   * If necessary, this method also updates the running system,
   * e.g. by restarting applications. This normally does not apply for
   * KSettings::Dialog modules where the updating is taken care of by
   * KSettings::Dispatcher.
   *
   * save is called when the user clicks "Apply" or "Ok".
   *
   * If you use TDEConfigXT, saving is taken care off automatically and 
   * you do not need to load manually. However, if you for some reason reimplement it and
   * also are using TDEConfigXT, you must call this function, otherwise the saving of TDEConfigXT 
   * options will not work. Call it at the very end of your reimplementation, to avoid
   * changed() signals getting emitted when you modify widgets.
   */
  virtual void save();

  /**
   * Sets the configuration to sensible default values.
   *
   * This method is called when the user clicks the "Default"
   * button. It should set the display to useful values.
   *
   * If you use TDEConfigXT, you do not have to reimplement this function since
   * the fetching and settings of default values is done automatically. However, if you 
   * reimplement and also are using TDEConfigXT, remember to call the base function at the 
   * very end of your reimplementation.
   */
  virtual void defaults();

  /**
   * Set the configuration to system default values.
   *
   * This method is called when the user clicks the "System-Default"
   * button. It should set the display to the system default values.
   *
   * @note The default behavior is to call defaults().
   */
  virtual void sysdefaults() { defaults(); }
  // KDE 4 deprecate

  /**
   * Return a quick-help text.
   *
   * This method is called when the module is docked.
   * The quick-help text should contain a short description of the module and
   * links to the module's help files. You can use QML formatting tags in the text.
   *
   * @note make sure the quick help text gets translated (use i18n()).
   */
  virtual TQString quickHelp() const;

  /**
   * If the handbook's docPath string of the currently displayed content
   * (for example in tabbed control center modules) differs from the docPath string
   * of the currently displaying container, this method returns the relevant docPath string
   *
   * @returns a TQString containing the relevant handbook's docPath string or TQString::null 
   *          if no specific docPath string is required (i.e. the container docPath is sufficient)
   */
  virtual TQString handbookDocPath() const;
  
  /**
   * If a specific handbook section is relevant to the currently displayed content
   * (for example tabbed control center modules) this method returns the relevant section name
   *
   * @note this string must match the handbook HTML section name exactly
   * @returns a TQString containing the relevant handbook section name or TQString::null if no specific section is relevant
   */
  virtual TQString handbookSection() const;

  /**
   * This is generally only called for the KBugReport.
   * If you override you should  have it return a pointer to a constant.
   *
   *
   * @returns the TDEAboutData for this module
   */
  virtual const TDEAboutData *aboutData() const;

  /**
   * This sets the TDEAboutData returned by aboutData()
   * @since 3.3
   */
   void setAboutData( TDEAboutData* about );

  /**
   * Indicate which buttons will be used.
   *
   * The return value is a value or'ed together from
   * the Button enumeration type.
   *
   * @see TDECModule::setButtons
   */
  int buttons() const { return _btn; }

  /**
   * Get the RootOnly message for this module.
   *
   * When the module must be run as root, or acts differently
   * for root and a normal user, it is sometimes useful to
   * customize the message that appears at the top of the module
   * when used as a normal user. This function returns this
   * customized message. If none has been set, a default message
   * will be used.
   *
   * @see TDECModule::setRootOnlyMsg
   */
  TQString rootOnlyMsg() const;

  /**
   * Tell if KControl should show a RootOnly message when run as
   * a normal user.
   *
   * In some cases, the module don't want a RootOnly message to
   * appear (for example if it has already one). This function
   * tells KControl if a RootOnly message should be shown
   *
   * @see TDECModule::setUseRootOnlyMsg
   */
  bool useRootOnlyMsg() const;

  TDEInstance *instance() const;

  /**
   * @return a list of @ref TDEConfigDialogManager's in use, if any.
   * @since 3.4
   */
  const TQPtrList<TDEConfigDialogManager>& configs() const;

protected:
  /**
   * Adds a TDEConfigskeleton @p config to watch the widget @p widget
   *
   * This function is useful if you need to handle multiple configuration files.
   *
   * @since 3.3
   * @return a pointer to the TDEConfigDialogManager in use
   * @param config the TDEConfigSkeleton to use
   * @param widget the widget to watch
   */
  TDEConfigDialogManager* addConfig( TDEConfigSkeleton *config, TQWidget* widget );

  /**
   * Sets the quick help.
   *
   * @since 3.3
   */
  void setQuickHelp( const TQString& help );

signals:

  /**
   * Indicate that the state of the modules contents has changed.
   *
   * This signal is emitted whenever the state of the configuration
   * shown in the module changes. It allows the control center to
   * keep track of unsaved changes.
   */
  void changed(bool state);

  /**
   * Indicate that the module's quickhelp has changed.
   *
   * Emit this signal whenever the module's quickhelp changes.
   * Modules implemented as tabbed dialogs might want to implement
   * per-tab quickhelp for example.
   *
   */
  void quickHelpChanged();

protected slots:

  /**
   * Calling this slot is equivalent to emitting changed(true).
   * @since 3.3
   */
  void changed();

  /**
   * A managed widget was changed, the widget settings and the current
   * settings are compared and a corresponding changed() signal is emitted
   * @since 3.4
   */
   void widgetChanged();

protected:

  /**
   * Sets the buttons to display.
   *
   * Help: shows a "Help" button.
   * Default: shows a "Use Defaults" button
   * Apply: in kcontrol this will show an "Apply" and "Reset" button
   *        in tdecmshell this will show an "Ok", "Apply" and "Cancel" button
   *
   * If Apply is not specified, tdecmshell will show a "Close" button.
   *
   * @see TDECModule::buttons
   */
  void setButtons(int btn) { _btn = btn; }

  /**
   * Sets the RootOnly message.
   *
   * This message will be shown at the top of the module of the
   * corresponding desktop file contains the line X-TDE-RootOnly=true.
   * If no message is set, a default one will be used.
   *
   * @see TDECModule::rootOnlyMsg
   */
  void setRootOnlyMsg(const TQString& msg);

  /**
   * Change whether or not the RootOnly message should be shown.
   *
   * Following the value of @p on, the RootOnly message will be
   * shown or not.
   *
   * @see TDECModule::useRootOnlyMsg
   */
  void setUseRootOnlyMsg(bool on);

  /**
   * Returns the changed state of automatically managed widgets in this dialog
   * @since 3.5
   */
  bool managedWidgetChangeState() const;

  /**
   * Call this method when your manually managed widgets change state between
   * changed and not changed
   * @since 3.5
   */
  void unmanagedWidgetChangeState(bool);

private:

  int _btn;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  TDECModulePrivate *d;

  /**
   * Internal function for initialization of the class.
   */
  void init();

};

#endif //TDECMODULE_H

