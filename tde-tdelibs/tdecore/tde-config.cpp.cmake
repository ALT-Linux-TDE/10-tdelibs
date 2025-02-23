#include <tdecmdlineargs.h>
#include <tdelocale.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <stdio.h>
#include <tdeaboutdata.h>
#include <config.h>
#include <tdeapplication.h>

static const char *description = I18N_NOOP("A little program to output installation paths");

static TDECmdLineOptions options[] =
{
    { "expandvars", I18N_NOOP("expand ${prefix} and ${exec_prefix} in output"), 0 },
    { "prefix",	   I18N_NOOP("Compiled in prefix for TDE libraries"), 0 },
    { "exec-prefix", I18N_NOOP("Compiled in exec_prefix for TDE libraries"), 0 },
    { "libsuffix", I18N_NOOP("Compiled in library path suffix"), 0 },
    { "localprefix", I18N_NOOP("Prefix in $HOME used to write files"), 0},
    { "version",   I18N_NOOP("Compiled in version string for TDE libraries"), 0 },
    { "types",     I18N_NOOP("Available TDE resource types"), 0 },
    { "path type", I18N_NOOP("Search path for resource type"), 0 },
    { "userpath type", I18N_NOOP("User path: desktop|autostart|trash|document"), 0 },
    { "install type", I18N_NOOP("Prefix to install resource files to"), 0},
    { 0,0,0 }
};

bool _expandvars = false;

TQString expandvars(const char *_input)
{
    TQString result = TQString::fromLatin1(_input);
    if (!_expandvars)
        return result;

    bool changed = false;
    int index = result.find("${prefix}");
    if (index >= 0) {
        result = result.replace(index, 9, "@CMAKE_INSTALL_PREFIX@");
        changed = true;
    }
    index = result.find("$(prefix)");
    if (index >= 0) {
        result = result.replace(index, 9, "@CMAKE_INSTALL_PREFIX@");
        changed = true;
    }
    index = result.find("${datadir}");
    if (index >= 0) {
        result = result.replace(index, 10, "@SHARE_INSTALL_PREFIX@");
        changed = true;
    }
    index = result.find("$(datadir)");
    if (index >= 0) {
        result = result.replace(index, 10, "@SHARE_INSTALL_PREFIX@");
        changed = true;
    }
    index = result.find("${exec_prefix}");
    if (index >= 0) {
        result = result.replace(index, 14, "@EXEC_INSTALL_PREFIX@");
        changed = true;
    }
    index = result.find("$(exec_prefix)");
    if (index >= 0) {
        result = result.replace(index, 14, "@EXEC_INSTALL_PREFIX@");
        changed = true;
    }
    index = result.find("${libdir}");
    if (index >= 0) {
        result = result.replace(index, 9, "@LIB_INSTALL_DIR@");
        changed = true;
    }
    index = result.find("$(libdir)");
    if (index >= 0) {
        result = result.replace(index, 9, "@LIB_INSTALL_DIR@");
        changed = true;
    }
    index = result.find("${includedir}");
    if (index >= 0) {
        result = result.replace(index, 20, "@INCLUDE_INSTALL_DIR@");
        changed = true;
    }
    index = result.find("$(includedir)");
    if (index >= 0) {
        result = result.replace(index, 20, "@INCLUDE_INSTALL_DIR@");
        changed = true;
    }
    index = result.find("${sysconfdir}");
    if (index >= 0) {
        result = result.replace(index, 13, "@SYSCONF_INSTALL_DIR@");
        changed = true;
    }
    index = result.find("$(sysconfdir)");
    if (index >= 0) {
        result = result.replace(index, 13, "@SYSCONF_INSTALL_DIR@");
        changed = true;
    }
    if (changed)
        return expandvars(result.latin1());
    else
        return result;
}

void printResult(const TQString &s)
{
    if (s.isEmpty())
        printf("\n");
    else
        printf("%s\n", s.local8Bit().data());
}

int main(int argc, char **argv)
{
    TDELocale::setMainCatalogue("tdelibs");
    TDEAboutData about("tde-config", "tde-config", "1.0", description, TDEAboutData::License_GPL, "(C) 2000 Stephan Kulow");
    TDECmdLineArgs::init( argc, argv, &about);

    TDECmdLineArgs::addCmdLineOptions( options ); // Add my own options.

    TDEInstance a("tde-config");
    a.setConfigReadOnly(TRUE);
    (void)TDEGlobal::dirs(); // trigger the creation
    (void)TDEGlobal::config();

    // Get application specific arguments
    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

    _expandvars = args->isSet("expandvars");

    if (args->isSet("prefix"))
    {
        printResult(expandvars("@CMAKE_INSTALL_PREFIX@"));
        return 0;
    }

    if (args->isSet("exec-prefix"))
    {
        printResult(expandvars("@EXEC_INSTALL_PREFIX@"));
        return 0;
    }

    if (args->isSet("libsuffix"))
    {
        TQString tmp(KDELIBSUFF);
        tmp.remove('"');
        printResult(expandvars(tmp.local8Bit()));
        return 0;
    }

    if (args->isSet("localprefix"))
    {
        printResult(TDEGlobal::dirs()->localtdedir());
        return 0;
    }

    if (args->isSet("version"))
    {
        printf("%s\n", TDE_VERSION_STRING);
        return 0;
    }

    if (args->isSet("types"))
    {
        TQStringList types = TDEGlobal::dirs()->allTypes();
        types.sort();
        const char *helptexts[] = {
            "apps", I18N_NOOP("Applications menu (.desktop files)"),
            "cgi", I18N_NOOP("CGIs to run from kdehelp"),
            "config", I18N_NOOP("Configuration files"),
            "data", I18N_NOOP("Where applications store data"),
            "exe", I18N_NOOP("Executables in $prefix/bin"),
            "html", I18N_NOOP("HTML documentation"),
            "icon", I18N_NOOP("Icons"),
            "kcfg", I18N_NOOP("Configuration description files"),
            "lib", I18N_NOOP("Libraries"),
            "include", I18N_NOOP("Includes/Headers"),
            "locale", I18N_NOOP("Translation files for TDELocale"),
            "mime", I18N_NOOP("Mime types"),
            "module", I18N_NOOP("Loadable modules"),
            "tqtplugins", I18N_NOOP("TQt plugins"),
            "services", I18N_NOOP("Services"),
            "servicetypes", I18N_NOOP("Service types"),
            "sound", I18N_NOOP("Application sounds"),
            "templates", I18N_NOOP("Templates"),
            "wallpaper", I18N_NOOP("Wallpapers"),
            "xdgdata-apps", I18N_NOOP("XDG Application menu (.desktop files)"),
            "xdgdata-dirs", I18N_NOOP("XDG Menu descriptions (.directory files)"),
            "xdgconf-menu", I18N_NOOP("XDG Menu layout (.menu files)"),
            "cmake", I18N_NOOP("CMake import modules (.cmake files)"),
            "tmp", I18N_NOOP("Temporary files (specific for both current host and current user)"),
            "socket", I18N_NOOP("UNIX Sockets (specific for both current host and current user)"),
            0, 0
        };
        for (TQStringList::ConstIterator it = types.begin(); it != types.end(); ++it)
        {
            int index = 0;
            while (helptexts[index] && *it != helptexts[index]) {
                index += 2;
            }
            if (helptexts[index]) {
                printf("%s - %s\n", helptexts[index], i18n(helptexts[index+1]).local8Bit().data());
            } else {
                printf("%s", i18n("%1 - unknown type\n").arg(*it).local8Bit().data());
            }
        }
        return 0;
    }

    TQString type = args->getOption("path");
    if (!type.isEmpty())
    {
        printResult(TDEGlobal::dirs()->resourceDirs(type.latin1()).join(":"));
        return 0;
    }

    type = args->getOption("userpath");
    if (!type.isEmpty())
    {
        if ( type == "desktop" )
            printResult(TDEGlobalSettings::desktopPath());
        else if ( type == "autostart" )
            printResult(TDEGlobalSettings::autostartPath());
        else if ( type == "trash" )
            printResult(TDEGlobalSettings::trashPath());
        else if ( type == "document" )
            printResult(TDEGlobalSettings::documentPath());
        else
            fprintf(stderr, "%s", i18n("%1 - unknown type of userpath\n").arg(type).local8Bit().data() );
        return 0;
    }

    type = args->getOption("install");
    if (!type.isEmpty())
    {
        const char *installprefixes[] = {
            "apps",   "@APPS_INSTALL_DIR@",
            "config", "@CONFIG_INSTALL_DIR@",
            "kcfg",   "@KCFG_INSTALL_DIR@",
            "data",   "@DATA_INSTALL_DIR@",
            "exe",    "@BIN_INSTALL_DIR@",
            "html",   "@HTML_INSTALL_DIR@",
            "icon",   "@ICON_INSTALL_DIR@",
            "lib",    "@LIB_INSTALL_DIR@",
            "module", "@PLUGIN_INSTALL_DIR@",
            "tqtplugins", "@PLUGIN_INSTALL_DIR@/plugins",
            "locale", "@LOCALE_INSTALL_DIR@",
            "mime",   "@MIME_INSTALL_DIR@",
            "services", "@SERVICES_INSTALL_DIR@",
            "servicetypes", "@SERVICETYPES_INSTALL_DIR@",
            "sound", "@SOUND_INSTALL_DIR@",
            "templates", "@TEMPLATES_INSTALL_DIR@",
            "wallpaper", "@WALLPAPER_INSTALL_DIR@",
            "xdgconf-menu", "@XDG_MENU_INSTALL_DIR@",
            "xdgdata-apps", "@XDG_APPS_INSTALL_DIR@",
            "xdgdata-dirs", "@XDG_DIRECTORY_INSTALL_DIR@",
            "include", "@INCLUDE_INSTALL_DIR@",
            "cmake", "@CMAKE_INSTALL_DIR@",
            0, 0
        };
        int index = 0;
        while (installprefixes[index] && type != installprefixes[index]) {
            index += 2;
        }
        if (installprefixes[index]) {
            printResult(expandvars(installprefixes[index+1]));
        } else {
            printResult("NONE"); // no i18n here as for scripts
        }
    }
    return 0;
}
