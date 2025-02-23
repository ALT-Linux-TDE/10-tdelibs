/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  
  This file is part of the KDE project
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef MODLOADER_H
#define MODLOADER_H

#include <tdecmodule.h>
#include <tdecmoduleinfo.h>

class TQWidget;
class KLibLoader;

/**
 * @ingroup tdecmodule
 * @brief Loads a KControl Module.
 *
 * TDECModuleLoader tries in several ways 
 * to locate and load a TDECModule. If loading fails a 
 * zero pointer is returned. \n
 * It is very unlikely TDECModuleLoader is what you want 
 * and @ref TDECModuleProxy suits your needs.
 * 
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
 * @author Frans Englich <frans.englich@telia.com>
 * @since 3.2
 * @internal
**/
class TDEUTILS_EXPORT TDECModuleLoader
{
  public:

    /**
     * Loads a @ref TDECModule. If loading fails a zero pointer is returned.
     * @param module what module to load
     * @param withFallback if true and loading failed a separate window
     * with the module may appear and a zero pointer is a returned
     * @param parent The parent widget
     * @param name The widget's name
     * @param args A list of arguments for the module to load
     *
     * @return a pointer to the loaded @ref TDECModule
     *
     * @deprecated use the function which explicitly states the error reporting
     * method
     */
    static TDECModule *loadModule(const TDECModuleInfo &module, bool withFallback=true, 
        TQWidget * parent = 0, const char * name = 0,
        const TQStringList & args = TQStringList() ) TDE_DEPRECATED;

    /**
     * Loads a @ref TDECModule. If loading fails a zero pointer is returned.
     * @param module what module to load
     * with the module may appear and a zero pointer is a returned
     * @param parent The parent widget
     * @param name The widget's name
     * @param args A list of arguments for the module to load
     *
     * @deprecated use the function which explicitly states the error reporting
     * method
     */
    static TDECModule *loadModule(const TQString &module, TQWidget *parent = 0,
        const char *name = 0, const TQStringList & args = TQStringList()) TDE_DEPRECATED;

    /**
     * Determines the way errors are reported
     */
    enum ErrorReporting {
      /** 
       * no error reporting is done 
       * */
      None = 0,
      /**
       * the error report is shown instead of the 
       * TDECModule that should have * been loaded 
       */
      Inline = 1,
      /** 
       * shows a dialog with the error report 
       */
      Dialog = 2,
      /** 
       * does both Inline and Dialog 
       */
      Both = 3
    };

    /**
     * Loads a @ref TDECModule. If loading fails a zero pointer is returned.
     * @param module what module to load
     * @param report see ErrorReporting
     * @param withFallback if true and loading failed a separate window
     * with the module may appear and a zero pointer is a returned
     * @param parent The parent widget
     * @param name The widget's name
     * @param args A list of arguments for the module to load
     *
     * @return a pointer to the loaded @ref TDECModule
     * @since 3.4
     */
    static TDECModule *loadModule(const TDECModuleInfo &module, ErrorReporting
        report, bool withFallback=true, TQWidget * parent = 0,
        const char * name = 0, const TQStringList & args = TQStringList() );

    /**
     * Loads a @ref TDECModule. If loading fails a zero pointer is returned.
     * @param module what module to load
     * @param report see ErrorReporting
     * with the module may appear and a zero pointer is a returned
     * @param parent The parent widget
     * @param name The widget's name
     * @param args A list of arguments for the module to load
     *
     * @return a pointer to the loaded @ref TDECModule
     * @since 3.4
     */
    static TDECModule *loadModule(const TQString &module, ErrorReporting
        report, TQWidget *parent = 0, const char *name = 0,
        const TQStringList & args = TQStringList());

    /**
     * Unloads the module's library
     * @param mod What module to unload for
     */
    static void unloadModule(const TDECModuleInfo &mod);

    /**
     * Display a message box explaining an error occured and possible
     * reasons to why.
     *
     * @deprecated Use a constructor with ErrorReporting set to Dialog to show a
     * message box like this function did.
     */
    static void showLastLoaderError(TQWidget *parent) TDE_DEPRECATED;


    /**
     * Checks whether an TDECModule should be shown by running its 
     * test function. If it is unsure whether a module should be shown, it should 
     * be made available, leaving the decision to the user.
     * If false is returned, the module should not be loaded in any interface.
     * 
     * A module declares it needs to be tested by having "X-TDE-Test-Module=true" in 
     * its desktop file. When that line exists, the following code must be available 
     * in the module's library:
     *
     * \code
     * extern "C"
     * {
     *    bool test_moduleName()
     *    {
     *      // Code testing for hardware/software presence.
     *      return true; // and the modue will be loaded.
     *    }
     *
     * }
     * \endcode
     *
     * where moduleName is the library name for the module.
     *
     * @param module the module to check
     * @returns true if the module should be loaded
     * @since 3.4
     */
    static bool testModule( const TQString& module );

    /**
     * Convenience function, essentially the same as above.
     *
     * @param module the module to check
     * @returns true if the module should be loaded
     * @since 3.4
     */
    static bool testModule( const TDECModuleInfo& module );

    /**
     * Returns a TDECModule containing the messages @p report and @p text.
     *
     * @param report the type of error reporting, see ErrorReporting
     * @param text the main message
     * @param details any additional details
     * @param parent The parent widget
     *
     * @since 3.4
     * @internal
     */
    static TDECModule* reportError( ErrorReporting report, const TQString & text,
        TQString details, TQWidget * parent );

  private:

    /**
     * Internal loader called by the public loaders.
     * @internal
     */
    static TDECModule* load(const TDECModuleInfo &mod, const TQString &libname, 
        KLibLoader *loader, ErrorReporting report, TQWidget * parent = 0,
        const char * name = 0, const TQStringList & args = TQStringList() );

};
#endif // MODLOADER_H

