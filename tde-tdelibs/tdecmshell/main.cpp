/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2004 Frans Englich <frans.englich@telia.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <iostream>

#include <tqcstring.h>
#include <tqfile.h> 

#include <dcopclient.h>
#include <qxembed.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdecmoduleinfo.h>
#include <tdecmoduleloader.h>
#include <tdecmoduleproxy.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kservice.h>
#include <kservicegroup.h>
#include <tdestartupinfo.h>
#include <twin.h>
#include <tdeglobal.h>

#include "main.h"
#include "main.moc"

using namespace std;

KService::List m_modules;

static TDECmdLineOptions options[] =
{
    { "list", I18N_NOOP("List all possible modules"), 0},
    { "+module", I18N_NOOP("Configuration module to open"), 0 },
    { "lang <language>", I18N_NOOP("Specify a particular language"), 0 },
    { "embed <id>", I18N_NOOP("Embeds the module with buttons in window with id <id>"), 0 },
    { "embed-proxy <id>", I18N_NOOP("Embeds the module without buttons in window with id <id>"), 0 },
    { "silent", I18N_NOOP("Do not display main window"), 0 },
    TDECmdLineLastOption
};

static void listModules(const TQString &baseGroup)
{

  KServiceGroup::Ptr group = KServiceGroup::group(baseGroup);

  if (!group || !group->isValid())
      return;

  KServiceGroup::List list = group->entries(true, true);

  for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
  {
     KSycocaEntry *p = (*it);
     if (p->isType(KST_KService))
     {
        KService *s = static_cast<KService*>(p);
        if (!kapp->authorizeControlModule(s->menuId()))
           continue;
        m_modules.append(s);
     }
     else if (p->isType(KST_KServiceGroup))
        listModules(p->entryPath());
  }
}

static KService::Ptr locateModule(const TQCString& module)
{
    TQString path = TQFile::decodeName(module);

    if (!path.endsWith(".desktop"))
        path += ".desktop";

    KService::Ptr service = KService::serviceByStorageId( path );
    if (!service)
    {
        kdWarning(780) << "Could not find module '" << module << "'." << endl;
        return 0;
    }

    // avoid finding random non-TDE applications
    if ( module.left( 4 ) != "tde-" && service->library().isEmpty() )
        return locateModule( "tde-" + module );

    if(!TDECModuleLoader::testModule( module ))
    {
        kdDebug(780) << "According to \"" << module << "\"'s test function, it should Not be loaded." << endl;
        return 0;
    }

    return service;
}

bool KCMShell::isRunning()
{
    if( dcopClient()->appId() == m_dcopName )
        return false; // We are the one and only.

    kdDebug(780) << "tdecmshell with modules '" << 
        m_dcopName << "' is already running." << endl;

    dcopClient()->attach(); // Reregister as anonymous
    dcopClient()->setNotifications(true);

    TQByteArray data;
    TQDataStream str( data, IO_WriteOnly );
    str << kapp->startupId();
    TQCString replyType;
    TQByteArray replyData;
    if (!dcopClient()->call(m_dcopName, "dialog", "activate(TQCString)", 
                data, replyType, replyData))
    {
        kdDebug(780) << "Calling DCOP function dialog::activate() failed." << endl;
        return false; // Error, we have to do it ourselves.
    }

    return true;
}

KCMShellMultiDialog::KCMShellMultiDialog( int dialogFace, const TQString& caption,
        TQWidget *parent, const char *name, bool modal)
    : KCMultiDialog( dialogFace, caption, parent, name, modal ),
        DCOPObject("dialog")
{
}

void KCMShellMultiDialog::activate( TQCString asn_id )
{
    kdDebug(780) << k_funcinfo << endl;

    TDEStartupInfo::setNewStartupId( this, asn_id );
}

void KCMShell::setDCOPName(const TQCString &dcopName, bool rootMode )
{
    m_dcopName = "tdecmshell_";
    if( rootMode )
        m_dcopName += "rootMode_";

    m_dcopName += dcopName;
    
    dcopClient()->registerAs(m_dcopName, false);
}

void KCMShell::waitForExit()
{
    kdDebug(780) << k_funcinfo << endl;

    connect(dcopClient(), TQ_SIGNAL(applicationRemoved(const TQCString&)),
            TQ_SLOT( appExit(const TQCString&) ));
    exec();
}

void KCMShell::appExit(const TQCString &appId)
{
    kdDebug(780) << k_funcinfo << endl;

    if( appId == m_dcopName )
    {
        kdDebug(780) << "'" << appId << "' closed, dereferencing." << endl;
        deref();
    }
}

static void setIcon(TQWidget *w, const TQString &iconName)
{
    TQPixmap icon = DesktopIcon(iconName);
    TQPixmap miniIcon = SmallIcon(iconName);
    w->setIcon( icon ); //standard X11
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
    KWin::setIcons(w->winId(), icon, miniIcon );
#endif
}

extern "C" TDE_EXPORT int kdemain(int _argc, char *_argv[])
{
    TDEAboutData aboutData( "tdecmshell", I18N_NOOP("TDE Control Module"),
                          0,
                          I18N_NOOP("A tool to start single TDE control modules"),
                          TDEAboutData::License_GPL,
                          I18N_NOOP("(c) 1999-2004, The KDE Developers") );

    aboutData.addAuthor("Frans Englich", I18N_NOOP("Maintainer"), "frans.englich@kde.org");
    aboutData.addAuthor("Daniel Molkentin", 0, "molkentin@kde.org");
    aboutData.addAuthor("Matthias Hoelzer-Kluepfel",0, "hoelzer@kde.org");
    aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    aboutData.addAuthor("Waldo Bastian",0, "bastian@kde.org");
    
    TDEGlobal::locale()->setMainCatalogue("tdecmshell");

    TDECmdLineArgs::init(_argc, _argv, &aboutData);
    TDECmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KCMShell app;

    const TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

    const TQCString lang = args->getOption("lang");
    if( !lang.isNull() )
        TDEGlobal::locale()->setLanguage(lang);

    if (args->isSet("list"))
    {
        cout << static_cast<const char *>(i18n("The following modules are available:").local8Bit()) << endl;

        listModules( "Settings/" );

        int maxLen=0;

        for (KService::List::ConstIterator it = m_modules.begin(); it != m_modules.end(); ++it)
        {
            int len = (*it)->desktopEntryName().length();
            if (len > maxLen)
                maxLen = len;
        }

        TQStringList module_list;
        for (KService::List::ConstIterator it = m_modules.begin(); it != m_modules.end(); ++it)
        {
            module_list.append(TQString("%1 - %2")
                .arg((*it)->desktopEntryName().leftJustify(maxLen, ' '))
                .arg(!(*it)->comment().isEmpty() ? (*it)->comment() : i18n("No description available")));
        }
        module_list.sort();

        for (TQStringList::Iterator it=module_list.begin(); it!=module_list.end(); ++it)
        {
            cout << static_cast<const char *>((*it).local8Bit()) << endl;
        }
        return 0;
    }

    if (args->count() < 1)
    {
        args->usage();
        return -1;
    }

    TQCString dcopName;
    KService::List modules;
    for (int i = 0; i < args->count(); i++)
    {
        KService::Ptr service = locateModule(args->arg(i));
        if( service )
        {
            modules.append(service);
            if( !dcopName.isEmpty() )
                dcopName += "_";

            dcopName += args->arg(i);
        }
    }

    /* Check if this particular module combination is already running, but 
     * allow the same module to run when embedding(root mode) */
    app.setDCOPName(dcopName, 
            ( args->isSet( "embed-proxy" ) || args->isSet( "embed" )));
    if( app.isRunning() )
    {
        app.waitForExit();
        return 0;
    }

    KDialogBase::DialogType dtype = KDialogBase::Plain;               
    if ( modules.count() < 1 )
        return 0;
    else if( modules.count() > 1 )
        dtype = KDialogBase::IconList;

    bool idValid;
    int id;

    if ( args->isSet( "embed-proxy" ))
    {
        id = args->getOption( "embed-proxy" ).toInt(&idValid);    
        if( idValid )
        {
            TDECModuleProxy *module = new TDECModuleProxy( modules.first()->desktopEntryName() );
            module->realModule();
            QXEmbed::embedClientIntoWindow( module, id);
            app.exec();
            delete module;
        }
        else
            kdDebug(780) << "Supplied id '" << id << "' is not valid." << endl;

        return 0;

    }

    KCMShellMultiDialog *dlg = new KCMShellMultiDialog( dtype, 
            i18n("Configure - %1").arg(kapp->caption()), 0, "", true );

    for (KService::List::ConstIterator it = modules.begin(); it != modules.end(); ++it)
        dlg->addModule(TDECModuleInfo(*it));

    if ( args->isSet( "embed" ))
    {
        id = args->getOption( "embed" ).toInt(&idValid);    
        if( idValid )
        {
            QXEmbed::embedClientIntoWindow( dlg, id );
            dlg->exec();
            delete dlg;
        }
        else
            kdDebug(780) << "Supplied id '" << id << "' is not valid." << endl;

    }
    else
    {

        if (kapp->iconName() != kapp->name())
            setIcon(dlg, kapp->iconName());
        else if ( modules.count() == 1 )
            setIcon(dlg, TDECModuleInfo( modules.first()).icon());

        dlg->exec();
        delete dlg;
    }

    return 0;
}
