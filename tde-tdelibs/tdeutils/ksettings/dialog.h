/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KSETTINGS_DIALOG_H
#define KSETTINGS_DIALOG_H

#include <tqobject.h>
#include <kservice.h>

template<class T> class TQValueList;
class KPluginInfo;
class KCMultiDialog;
class TDECModuleInfo;

namespace KSettings
{

/**
 * @ingroup main
 * @ingroup settings
 * @short Generic configuration dialog that even works over component boundaries
 *
 * For more information see \ref KSettings.
 *
 * This class aims to standardize the use of configuration dialogs in KDE
 * applications. Especially when using KParts and/or Plugins you face problems
 * creating a consistent config dialog.
 *
 * To show a configuration dialog you only have to call the show method and be
 * done with it. A code example:
 *
 * You initialize \p m_cfgdlg with
 * \code
 * m_cfgdlg = new Dialog( Dialog::Static, this );
 * \endcode
 * If you use a KPart that was not especially designed for your app you can use
 * the second constructor:
 * \code
 * TQStringList tdepartslist;
 * for( all my tdeparts )
 *   tdepartslist += m_mypart->instance().instanceName();
 * m_cfgdlg = new Dialog( tdepartslist, this );
 * \endcode
 * and the action for the config dialog is connected to the show slot:
 * \code
 * KStdAction::preferences( m_cfgdlg, TQ_SLOT( show() ), actionCollection() );
 * \endcode
 *
 * If you need to be informed when the config was changed and applied in the
 * dialog you might want to take a look at Dispatcher.
 *
 * For more information see \ref KSettings.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class TDEUTILS_EXPORT Dialog : public TQObject
{
    friend class PageNode;
    TQ_OBJECT
    public:
        /**
         * Tells the dialog whether the entries in the listview are all static
         * or whether it should add a Configure... button to select which parts
         * of the optional functionality should be active or not.
         */
        enum ContentInListView
        {
            /**
             * Static listview, while running no entries are added or deleted
             */
            Static,
            /**
             * Configurable listview. The user can select what functionality he
             * wants.
             */
            Configurable
        };

        /**
         * Construct a new Preferences Dialog for the application. It uses all
         * KCMs with X-TDE-ParentApp set to TDEGlobal::instance()->instanceName().
         *
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         * @param name         name
         */
        Dialog( TQWidget * parent = 0, const char * name = 0 );

        /**
         * Construct a new Preferences Dialog for the application. It uses all
         * KCMs with X-TDE-ParentApp set to TDEGlobal::instance()->instanceName().
         *
         * @param content      Select whether you want a static or configurable
         *                     config dialog.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         * @param name         name
         */
        Dialog( ContentInListView content = Static, TQWidget * parent = 0,
                const char * name = 0 );

        /**
         * Construct a new Preferences Dialog with the pages for the selected
         * instance names. For example if you want to have the configuration
         * pages for the kviewviewer KPart you would pass a
         * TQStringList consisting of only the name of the part "kviewviewer".
         *
         * @param components   A list of the names of the components that your
         *                     config dialog should merge the config pages in.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         * @param name         name
         */
        Dialog( const TQStringList & components, TQWidget * parent = 0,
                const char * name = 0 );

        /**
         * Construct a new Preferences Dialog with the pages for the selected
         * instance names. For example if you want to have the configuration
         * pages for the kviewviewer KPart you would pass a
         * TQStringList consisting of only the name of the part "kviewviewer".
         *
         * @param components   A list of the names of the components that your
         *                     config dialog should merge the config pages in.
         * @param content      Select whether you want a static or configurable
         *                     config dialog.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         * @param name         name
         */
        Dialog( const TQStringList & components, ContentInListView
                content, TQWidget * parent = 0, const char * name = 0 );

        ~Dialog();

        /**
         * If you use a Configurable dialog you need to pass KPluginInfo
         * objects that the dialog should configure.
         */
        void addPluginInfos( const TQValueList<KPluginInfo*> & plugininfos );

        KCMultiDialog * dialog();

    public slots:
        /**
         * Show the config dialog. The slot immediatly returns since the dialog
         * is non-modal.
         */
        void show();

    signals:
        /**
         * If you use the dialog in Configurable mode and want to be notified
         * when the user changes the plugin selections use this signal. It's
         * emitted if the selection has changed and the user pressed Apply or
         * Ok. In the slot you would then load and unload the plugins as
         * requested.
         */
        void pluginSelectionChanged();

    protected slots:
        void configureTree();
        void updateTreeList();

    private:
        /**
         * @internal
         * Check whether the plugin associated with this KCM is enabled.
         */
        bool isPluginForKCMEnabled( TDECModuleInfo * ) const;

        TQValueList<KService::Ptr> instanceServices() const;
        TQValueList<KService::Ptr> parentComponentsServices(
                const TQStringList & ) const;
        /**
         * @internal
         * Read the .setdlg file and add it to the groupmap
         */
        void parseGroupFile( const TQString & );

        /**
         * @internal
         * If this module is put into a TreeList hierarchy this will return a
         * list of the names of the parent modules.
         */
        TQStringList parentModuleNames( TDECModuleInfo * );

        /**
         * @internal
         * This method is called only once. The KCMultiDialog is not created
         * until it's really needed. So if some method needs to access d->dlg it
         * checks for 0 and if it's not created this method will do it.
         */
        void createDialogFromServices();

        class DialogPrivate;
        DialogPrivate * d;
};

}
#endif // KSETTINGS_DIALOG_H
